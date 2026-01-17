#include "../Helpers/PeriodicalActions.h"


#include "../../ESPEasy-Globals.h"

#include "../ControllerQueue/DelayQueueElements.h"
#include "../ControllerQueue/MQTT_queue_element.h"
#include "../DataStructs/TimingStats.h"
#include "../DataTypes/ESPEasy_plugin_functions.h"
#include "../ESPEasyCore/Controller.h"
#include "../ESPEasyCore/ESPEasyGPIO.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../../ESPEasy/net/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyRules.h"
#include "../ESPEasyCore/Serial.h"
#include "../Globals/ESPEasy_Scheduler.h"
#include "../Globals/ESPEasy_time.h"
#include "../Globals/EventQueue.h"
#include "../Globals/MainLoopCommand.h"
#include "../Globals/MQTT.h"
#include "../Globals/RTC.h"
#include "../Globals/Services.h"
#include "../Globals/Settings.h"
#include "../Globals/Statistics.h"
#include "../../ESPEasy/net/Globals/WiFi_AP_Candidates.h"
#include "../Helpers/ESPEasyRTC.h"
#include "../Helpers/FS_Helper.h"
#include "../Helpers/Hardware_temperature_sensor.h"
#include "../Helpers/Memory.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Networking.h"
#include "../Helpers/StringGenerator_System.h"
#include "../Helpers/StringGenerator_WiFi.h"
#include "../Helpers/StringProvider.h"

#include "../../ESPEasy/net/wifi/ESPEasyWifi.h"
#include "../../ESPEasy/net/Globals/ESPEasyWiFi.h"
#include "../../ESPEasy/net/Globals/ESPEasyWiFiEvent.h"
#include "../../ESPEasy/net/Globals/NetworkState.h"
#include "../../ESPEasy/net/Globals/NWPlugins.h"
#include "../../ESPEasy/net/wifi/WiFi_State.h"


#ifdef USES_C015
#include "../../ESPEasy_fdwdecl.h"
#endif



#define PLUGIN_ID_MQTT_IMPORT         37


/*********************************************************************************************\
 * Tasks that run 50 times per second
\*********************************************************************************************/

void run50TimesPerSecond() {
  String dummy;
  {
    // Do network calls first, so any needed checks or updates are done 
    // before any controller may need to use the network
#ifdef ESP32
    static const NetworkInterface *lastDefaultInterface = nullptr;
    NetworkInterface * currentDefaultInterface = Network.getDefaultInterface();
    if (nonDefaultNetworkInterface_gotIP || lastDefaultInterface != currentDefaultInterface) {
      nonDefaultNetworkInterface_gotIP = false;
      ESPEasy::net::NWPluginCall(NWPlugin::Function::NWPLUGIN_PRIORITY_ROUTE_CHANGED, 0, dummy);
      lastDefaultInterface = currentDefaultInterface;
    }
#endif

    START_TIMER;
    ESPEasy::net::NWPluginCall(NWPlugin::Function::NWPLUGIN_FIFTY_PER_SECOND, 0, dummy);
    STOP_TIMER(NWPLUGIN_CALL_50PS);
  }
  {
    ESPEasy::net::processNetworkEvents();
  }
  {
    START_TIMER;
    PluginCall(PLUGIN_FIFTY_PER_SECOND, 0, dummy);
    STOP_TIMER(PLUGIN_CALL_50PS);
  }
  {
    START_TIMER;
    CPluginCall(CPlugin::Function::CPLUGIN_FIFTY_PER_SECOND, 0, dummy);
    STOP_TIMER(CPLUGIN_CALL_50PS);
  }

  processNextEvent();
}

/*********************************************************************************************\
 * Tasks that run 10 times per second
\*********************************************************************************************/
void run10TimesPerSecond() {
  String dummy;
  //@giig19767g: WARNING: Monitor10xSec must run before PLUGIN_TEN_PER_SECOND
  {
    START_TIMER;
    GPIO_Monitor10xSec();
    STOP_TIMER(PLUGIN_CALL_10PSU);
  }
  {
    START_TIMER;
    ESPEasy::net::NWPluginCall(NWPlugin::Function::NWPLUGIN_TEN_PER_SECOND, 0, dummy);
    STOP_TIMER(NWPLUGIN_CALL_10PS);
  }
  {
    START_TIMER;
    PluginCall(PLUGIN_TEN_PER_SECOND, 0, dummy);
    STOP_TIMER(PLUGIN_CALL_10PS);
  }
  {
    START_TIMER;
//    PluginCall(PLUGIN_UNCONDITIONAL_POLL, 0, dummyString);
    PluginCall(PLUGIN_MONITOR, 0, dummy);
    STOP_TIMER(PLUGIN_CALL_10PSU);
  }
  {
    START_TIMER;
    CPluginCall(CPlugin::Function::CPLUGIN_TEN_PER_SECOND, 0, dummy);
    STOP_TIMER(CPLUGIN_CALL_10PS);
  }
  
  #ifdef USES_C015
  if (ESPEasy::net::NetworkConnected()) {
    Blynk_Run_c015();
  }
  #endif
  if (!UseRTOSMultitasking && 
    (ESPEasy::net::NetworkConnected() || ESPEasy::net::wifi::wifiAPmodeActivelyUsed())) {
    // FIXME TD-er: What about client connected via AP?
    START_TIMER
    web_server.handleClient();
    STOP_TIMER(WEBSERVER_HANDLE_CLIENT);
  }
}


/*********************************************************************************************\
 * Tasks each second
\*********************************************************************************************/
void runOncePerSecond()
{
  START_TIMER;
  updateLogLevelCache();
  dailyResetCounter++;
  if (dailyResetCounter > 86400) // 1 day elapsed... //86400
  {
    RTC.flashDayCounter=0;
    saveToRTC();
    dailyResetCounter=0;
    #ifndef LIMIT_BUILD_SIZE
    addLog(LOG_LEVEL_INFO, F("SYS  : Reset 24h counters"));
    #endif
  }

  if (Settings.ConnectionFailuresThreshold) {
    auto data = ESPEasy::net::getDefaultRoute_NWPluginData_static_runtime();
    if (data && data->getConnectionFailures() > Settings.ConnectionFailuresThreshold)
      delayedReboot(60, IntendedRebootReason_e::DelayedReboot);
  }
  if (cmd_within_mainloop != 0)
  {
    switch (cmd_within_mainloop)
    {
      case CMD_WIFI_DISCONNECT:
        {
          ESPEasy::net::wifi::WifiDisconnect();
          break;
        }
      case CMD_REBOOT:
        {
          reboot(IntendedRebootReason_e::CommandReboot);
          break;
        }
    }
    cmd_within_mainloop = 0;
  }
  // clock events
  if (node_time.reportNewMinute()) {
    String dummy;
    PluginCall(PLUGIN_CLOCK_IN, 0, dummy);
    if (Settings.UseRules)
    {
      // FIXME TD-er: What to do when the system time is not (yet) present?
      if (node_time.systemTimePresent()) {
        // TD-er: Do not add to the eventQueue, but execute right now.
        const String event = strformat(
          F("Clock#Time=%s,%s"), 
          node_time.weekday_str().c_str(),
          node_time.getTimeString(':', false).c_str());
        rulesProcessing(event);
      }
    }
  }

//  unsigned long start = micros();
  String dummy;
  PluginCall(PLUGIN_ONCE_A_SECOND, 0, dummy);
//  unsigned long elapsed = micros() - start;

#if FEATURE_NETWORK_STATS
  for (ESPEasy::net::networkIndex_t x = 0; x < NETWORK_MAX; x++) {
    if (Settings.getNetworkEnabled(x)) {
      EventStruct tempEvent;
      tempEvent.NetworkIndex = x;
      ESPEasy::net::NWPluginCall(NWPlugin::Function::NWPLUGIN_RECORD_STATS, &tempEvent);
    }
  }
#endif

  // I2C Watchdog feed
  if (Settings.WDI2CAddress != 0)
  {
    #if FEATURE_I2C_MULTIPLE
    I2CSelectHighClockSpeed(Settings.getI2CInterfaceWDT()); // Select bus
    #endif // if FEATURE_I2C_MULTIPLE
    I2C_write8(Settings.WDI2CAddress, 0xA5);
  }

  #if FEATURE_MDNS
  #ifdef ESP8266
  // Allow MDNS processing
  if (ESPEasy::net::NetworkConnected()) {
    MDNS.announce();
  }
  #endif
  #endif // if FEATURE_MDNS

  #if FEATURE_INTERNAL_TEMPERATURE && defined(ESP32_CLASSIC)
  getInternalTemperature(); // Just read the value every second to hopefully get a valid next reading on original ESP32
  #endif // if FEATURE_INTERNAL_TEMPERATURE && defined(ESP32_CLASSIC)

  checkResetFactoryPin();
  STOP_TIMER(PLUGIN_CALL_1PS);
}

/*********************************************************************************************\
 * Tasks each 30 seconds
\*********************************************************************************************/
void runEach30Seconds()
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAMtoLog();
  #endif
  wdcounter++;
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {  
    auto data = ESPEasy::net::getDefaultRoute_NWPluginData_static_runtime();
    if (!data) {
      addLogMove(LOG_LEVEL_INFO, strformat(
        F("WD   : Uptime %d  FreeMem %u"),
        getUptimeMinutes(),
        FreeMem()));
    } else {
      String log = strformat(
        F("WD   : Uptime %d  ConnectFailures %u FreeMem %u"),
        getUptimeMinutes(),
        data->getConnectionFailures(),
        FreeMem());
      bool logWiFiStatus = true;
      #if FEATURE_ETHERNET
      if(active_network_medium == ESPEasy::net::NetworkMedium_t::Ethernet) {
        logWiFiStatus = false;
        log += F( " EthSpeedState ");
        log += getValue(LabelType::ETH_SPEED_STATE);
//        log += F(" ETH status: ");
//        log += EthEventData.ESPEasyEthStatusToString();
      }
      #endif // if FEATURE_ETHERNET
      if (logWiFiStatus) {
        log += strformat(
          F(" WiFiStatus: %s ESPeasy internal wifi status: %s (%s)"),
          ArduinoWifiStatusToString(WiFi.status()).c_str(),
          FsP(ESPEasy::net::wifi::toString(ESPEasyWiFi.getState())),
          data->statusToString().c_str());
      }
  //    log += F(" ListenInterval ");
  //    log += WiFi.getListenInterval();
      addLogMove(LOG_LEVEL_INFO, log);
#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  //    addLogMove(LOG_LEVEL_INFO,  ESPEASY_SERIAL_CONSOLE_PORT.getLogString());
#endif
    }
  }
  ESPEasy::net::wifi::WiFi_AP_Candidates.purge_expired();
  #if FEATURE_ESPEASY_P2P
  sendSysInfoUDP(1);
  refreshNodeList();
  #endif

  // sending $stats to homie controller
  CPluginCall(CPlugin::Function::CPLUGIN_INTERVAL, 0);

  #if defined(ESP8266)
  #if FEATURE_SSDP
  if (Settings.UseSSDP)
    SSDP_update();

  #endif // if FEATURE_SSDP
  #endif
#if FEATURE_ADC_VCC
//  if (!WiFiEventData.wifiConnectInProgress) {
    vcc = ESP.getVcc() / 1000.0f;
//  }
#endif

  #if FEATURE_REPORTING
  ReportStatus();
  #endif // if FEATURE_REPORTING

}

#if FEATURE_MQTT


void scheduleNextMQTTdelayQueue() {
  if (MQTTDelayHandler != nullptr) {
    Scheduler.scheduleNextDelayQueue(SchedulerIntervalTimer_e::TIMER_MQTT_DELAY_QUEUE, MQTTDelayHandler->getNextScheduleTime());
  }
}

void schedule_all_MQTTimport_tasks() {
  constexpr pluginID_t PLUGIN_MQTT_IMPORT(PLUGIN_ID_MQTT_IMPORT);

  deviceIndex_t DeviceIndex = getDeviceIndex(PLUGIN_MQTT_IMPORT); // Check if P037_MQTTimport is present in the build
  if (validDeviceIndex(DeviceIndex)) {
    for (taskIndex_t task = 0; task < TASKS_MAX; task++) {
      if ((Settings.getPluginID_for_task(task) == PLUGIN_MQTT_IMPORT) &&
          (Settings.TaskDeviceEnabled[task])) {
        // Schedule a call to each enabled MQTT import plugin to notify the broker connection state
        EventStruct event(task);
        event.Par1 = MQTTclient_connected ? 1 : 0;
        Scheduler.schedule_plugin_task_event_timer(
          task,
          PLUGIN_MQTT_CONNECTION_STATE, 
          std::move(event));
      }
    }
  }
}

void processMQTTdelayQueue() {
  if (MQTTDelayHandler == nullptr) {
    return;
  }
  runPeriodicalMQTT(); // Update MQTT connected state.
  if (!MQTTclient_connected) {
    scheduleNextMQTTdelayQueue();
    return;
  }

  START_TIMER;
  MQTT_queue_element *element(static_cast<MQTT_queue_element *>(MQTTDelayHandler->getNext()));

  if (element == nullptr) { return; }

  bool handled = false;

  if (element->_call_PLUGIN_PROCESS_CONTROLLER_DATA) {
    struct EventStruct TempEvent(element->_taskIndex);
    String dummy;

    // FIXME TD-er: Do we need anything from the element in the event?
//    TempEvent.String1 = element->_topic;
//    TempEvent.String2 = element->_payload;
    if (PluginCall(PLUGIN_PROCESS_CONTROLLER_DATA, &TempEvent, dummy)) {
      handled = true;
      MQTTDelayHandler->markProcessed(true);
    } else {
      MQTTDelayHandler->markProcessed(false);
    }
  } else
  if (!handled) {
    if (MQTTclient.publish(element->_topic.c_str(), element->_payload.c_str(), element->_retained)) {
      auto data = ESPEasy::net::getDefaultRoute_NWPluginData_static_runtime();
      if (data) {
        data->markPublishSuccess();
      }
      MQTTDelayHandler->markProcessed(true);
    } else {
      MQTTDelayHandler->markProcessed(false);
#ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        String log = F("MQTT : process MQTT queue not published, ");
        log += MQTTDelayHandler->sendQueue.size();
        log += F(" items left in queue");
        addLogMove(LOG_LEVEL_DEBUG, log);
      }
#endif // ifndef BUILD_NO_DEBUG
    }
  }
  Scheduler.setIntervalTimerOverride(SchedulerIntervalTimer_e::TIMER_MQTT, 10); // Make sure the MQTT is being processed as soon as possible.
  scheduleNextMQTTdelayQueue();
  STOP_TIMER(MQTT_DELAY_QUEUE);
}

void updateMQTTclient_connected() {
  const bool actual_MQTTclient_connected = MQTTclient.connected();
  if (MQTTclient_connected != actual_MQTTclient_connected) {
    MQTTclient_connected = actual_MQTTclient_connected;
    MQTTclient_connected_stats.set(actual_MQTTclient_connected);
    if (!MQTTclient_connected) {
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        String connectionError = F("MQTT : Connection lost, state: ");
        connectionError += getMQTT_state();
#ifndef BUILD_NO_DEBUG
        auto duration_ms = MQTTclient_connected_stats.getLastOnDuration_ms();
        if (duration_ms > 0) {
          connectionError += concat(F(" Connected duration: "), format_msec_duration_HMS(duration_ms));
          connectionError += concat(F(" (successful) Reconnect Count: "), MQTTclient_connected_stats.getCycleCount());
        }
#endif
        addLogMove(LOG_LEVEL_ERROR, connectionError);
      }
      MQTTclient_must_send_LWT_connected = false;
    }
    if (Settings.UseRules) {
      if (MQTTclient_connected) {
        eventQueue.add(F("MQTT#Connected"));
      } else {
        eventQueue.add(F("MQTT#Disconnected"));
      }
    }
    // Now schedule all tasks using the MQTT Import plugin.
    schedule_all_MQTTimport_tasks();
  }
  if (!MQTTclient_connected) {
    // As suggested here: https://github.com/letscontrolit/ESPEasy/issues/1356
    if (timermqtt_interval < 30000) {
      timermqtt_interval += 500;
    }
  } else {
    timermqtt_interval = 100;
  }
  Scheduler.setIntervalTimer(SchedulerIntervalTimer_e::TIMER_MQTT);
  scheduleNextMQTTdelayQueue();
  #if FEATURE_MQTT_CONNECT_BACKGROUND
  MQTTConnectInBackground(CONTROLLER_MAX, true); // Report state
  #endif // if FEATURE_MQTT_CONNECT_BACKGROUND
}

void runPeriodicalMQTT() {
  START_TIMER
  // MQTT_KEEPALIVE = 15 seconds.
  if (!NetworkConnected(10)) {
    updateMQTTclient_connected();
    return;
  }
  //dont do this in backgroundtasks(), otherwise causes crashes. (https://github.com/letscontrolit/ESPEasy/issues/683)
  controllerIndex_t enabledMqttController = firstEnabledMQTT_ControllerIndex();
  if (validControllerIndex(enabledMqttController)) {
    if (!MQTTclient.loop()) {
      updateMQTTclient_connected();
      if (MQTTCheck(enabledMqttController)) {
        updateMQTTclient_connected();
      }
    }
  } else {
    if (MQTTclient.connected()) {
      #if FEATURE_MQTT_CONNECT_BACKGROUND
      if (MQTT_task_data.taskHandle) {
        vTaskDelete(MQTT_task_data.taskHandle);
        MQTT_task_data.taskHandle = NULL;
      }
      MQTT_task_data.status = MQTT_connect_status_e::Disconnected;
      #endif // if FEATURE_MQTT_CONNECT_BACKGROUND
      MQTTclient.disconnect();
      updateMQTTclient_connected();
    }
  }
  STOP_TIMER(PERIODICAL_MQTT);
}


#endif //if FEATURE_MQTT



void logTimerStatistics() {
# ifndef BUILD_NO_DEBUG
  const uint8_t loglevel = LOG_LEVEL_DEBUG;
#else
  const uint8_t loglevel = LOG_LEVEL_NONE;
#endif
  updateLoopStats_30sec(loglevel);
#ifndef BUILD_NO_DEBUG
//  logStatistics(loglevel, true);
  if (loglevelActiveFor(loglevel)) {
    String queueLog = F("Scheduler stats: (called/tasks/max_length/idle%) ");
    queueLog += Scheduler.getQueueStats();
    addLogMove(loglevel, queueLog);
  }
#endif
}

void updateLoopStats_30sec(uint8_t loglevel) {
  loopCounterLast = loopCounter;
  loopCounter = 0;
  if (loopCounterLast > loopCounterMax)
    loopCounterMax = loopCounterLast;

  Scheduler.updateIdleTimeStats();

#ifndef BUILD_NO_DEBUG
  if (loglevelActiveFor(loglevel)) {
    String log = F("LoopStats: shortestLoop: ");
    log += shortestLoop;
    log += F(" longestLoop: ");
    log += longestLoop;
    log += F(" avgLoopDuration: ");
    log += loop_usec_duration_total / loopCounter_full;
    log += F(" loopCounterMax: ");
    log += loopCounterMax;
    log += F(" loopCounterLast: ");
    log += loopCounterLast;
    addLogMove(loglevel, log);
  }
#endif
  loop_usec_duration_total = 0;
  loopCounter_full = 1;
}


/********************************************************************************************\
   Clean up all before going to sleep or reboot.
 \*********************************************************************************************/
void flushAndDisconnectAllClients() {
  if (anyControllerEnabled()) {
#if FEATURE_MQTT
    bool mqttControllerEnabled = validControllerIndex(firstEnabledMQTT_ControllerIndex());
#endif //if FEATURE_MQTT
    unsigned long timer = millis() + 1000;
    while (!timeOutReached(timer)) {
      // call to all controllers (delay queue) to flush all data.
      CPluginCall(CPlugin::Function::CPLUGIN_FLUSH, 0);
#if FEATURE_MQTT      
      if (mqttControllerEnabled && MQTTclient.connected()) {
        MQTTclient.loop();
      }
#endif //if FEATURE_MQTT
    }
#if FEATURE_MQTT
    if (mqttControllerEnabled && MQTTclient.connected()) {
      #if FEATURE_MQTT_CONNECT_BACKGROUND
      if (MQTT_task_data.taskHandle) {
        vTaskDelete(MQTT_task_data.taskHandle);
        MQTT_task_data.taskHandle = NULL;
      }
      MQTT_task_data.status = MQTT_connect_status_e::Disconnected;
      #endif // if FEATURE_MQTT_CONNECT_BACKGROUND
      MQTTclient.disconnect();
      updateMQTTclient_connected();
    }
#endif //if FEATURE_MQTT
    saveToRTC();
    delay(100); // Flush anything in the network buffers.
  }
  process_serialWriteBuffer();
}


void prepareShutdown(IntendedRebootReason_e reason)
{
//  WiFiEventData.intent_to_reboot = true;
#if FEATURE_MQTT
  runPeriodicalMQTT(); // Flush outstanding MQTT messages
#endif // if FEATURE_MQTT
  process_serialWriteBuffer();
  flushAndDisconnectAllClients();
  saveUserVarToRTC();
  CPluginCall(CPlugin::Function::CPLUGIN_EXIT_ALL, 0);
  ESPEasy::net::NWPluginCall(NWPlugin::Function::NWPLUGIN_EXIT_ALL, 0);
//  ESPEasy::net::wifi::setWifiMode(WIFI_OFF);
  ESPEASY_FS.end();
  process_serialWriteBuffer();
  delay(100); // give the node time to flush all before reboot or sleep
  node_time.now_();
  Scheduler.markIntendedReboot(reason);
  saveToRTC();
}


