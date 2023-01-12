#include "../Helpers/PeriodicalActions.h"


#include "../../ESPEasy-Globals.h"

#include "../ControllerQueue/DelayQueueElements.h"
#include "../ControllerQueue/MQTT_queue_element.h"
#include "../DataStructs/TimingStats.h"
#include "../DataTypes/ESPEasy_plugin_functions.h"
#include "../ESPEasyCore/Controller.h"
#include "../ESPEasyCore/ESPEasyGPIO.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyWifi.h"
#include "../ESPEasyCore/ESPEasyRules.h"
#include "../ESPEasyCore/Serial.h"
#include "../Globals/ESPEasyWiFiEvent.h"
#if FEATURE_ETHERNET
#include "../Globals/ESPEasyEthEvent.h"
#endif
#include "../Globals/ESPEasy_Scheduler.h"
#include "../Globals/ESPEasy_time.h"
#include "../Globals/EventQueue.h"
#include "../Globals/MainLoopCommand.h"
#include "../Globals/MQTT.h"
#include "../Globals/NetworkState.h"
#include "../Globals/RTC.h"
#include "../Globals/SecuritySettings.h"
#include "../Globals/Services.h"
#include "../Globals/Settings.h"
#include "../Globals/Statistics.h"
#include "../Globals/WiFi_AP_Candidates.h"
#include "../Helpers/ESPEasyRTC.h"
#include "../Helpers/FS_Helper.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/Memory.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Networking.h"
#include "../Helpers/StringGenerator_System.h"
#include "../Helpers/StringGenerator_WiFi.h"
#include "../Helpers/StringProvider.h"

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
  if (NetworkConnected())
      Blynk_Run_c015();
  #endif
  #ifndef USE_RTOS_MULTITASKING
    web_server.handleClient();
  #endif
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
    addLog(LOG_LEVEL_INFO, F("SYS  : Reset 24h counters"));
  }

  if (Settings.ConnectionFailuresThreshold)
    if (WiFiEventData.connectionFailures > Settings.ConnectionFailuresThreshold)
      delayedReboot(60, ESPEasy_Scheduler::IntendedRebootReason_e::DelayedReboot);

  if (cmd_within_mainloop != 0)
  {
    switch (cmd_within_mainloop)
    {
      case CMD_WIFI_DISCONNECT:
        {
          WifiDisconnect();
          break;
        }
      case CMD_REBOOT:
        {
          reboot(ESPEasy_Scheduler::IntendedRebootReason_e::CommandReboot);
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
        String event;
        event.reserve(21);
        event += F("Clock#Time=");
        event += node_time.weekday_str();
        event += ',';
        event += node_time.getTimeString(':', false);

        // TD-er: Do not add to the eventQueue, but execute right now.
        rulesProcessing(event);
      }
    }
  }

//  unsigned long start = micros();
  String dummy;
  PluginCall(PLUGIN_ONCE_A_SECOND, 0, dummy);
//  unsigned long elapsed = micros() - start;


  // I2C Watchdog feed
  if (Settings.WDI2CAddress != 0)
  {
    Wire.beginTransmission(Settings.WDI2CAddress);
    Wire.write(0xA5);
    Wire.endTransmission();
  }

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
    String log;
    log.reserve(80);
    log = F("WD   : Uptime ");
    log += getUptimeMinutes();
    log += F(" ConnectFailures ");
    log += WiFiEventData.connectionFailures;
    log += F(" FreeMem ");
    log += FreeMem();
    bool logWiFiStatus = true;
    #if FEATURE_ETHERNET
    if(active_network_medium == NetworkMedium_t::Ethernet) {
      logWiFiStatus = false;
      log += F( " EthSpeedState ");
      log += getValue(LabelType::ETH_SPEED_STATE);
      log += F(" ETH status: ");
      log += EthEventData.ESPEasyEthStatusToString();
    }
    #endif // if FEATURE_ETHERNET
    if (logWiFiStatus) {
      log += F(" WiFiStatus ");
      log += ArduinoWifiStatusToString(WiFi.status());
      log += F(" ESPeasy internal wifi status: ");
      log += WiFiEventData.ESPeasyWifiStatusToString();
    }

//    log += F(" ListenInterval ");
//    log += WiFi.getListenInterval();
    addLogMove(LOG_LEVEL_INFO, log);
  }
  WiFi_AP_Candidates.purge_expired();
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
  if (!WiFiEventData.wifiConnectInProgress) {
    vcc = ESP.getVcc() / 1000.0f;
  }
#endif

  #if FEATURE_REPORTING
  ReportStatus();
  #endif // if FEATURE_REPORTING

}

#if FEATURE_MQTT


void scheduleNextMQTTdelayQueue() {
  if (MQTTDelayHandler != nullptr) {
    Scheduler.scheduleNextDelayQueue(ESPEasy_Scheduler::IntervalTimer_e::TIMER_MQTT_DELAY_QUEUE, MQTTDelayHandler->getNextScheduleTime());
  }
}

void schedule_all_MQTTimport_tasks() {
  controllerIndex_t ControllerIndex = firstEnabledMQTT_ControllerIndex();

  if (!validControllerIndex(ControllerIndex)) { return; }

  deviceIndex_t DeviceIndex = getDeviceIndex(PLUGIN_ID_MQTT_IMPORT); // Check if P037_MQTTimport is present in the build
  if (validDeviceIndex(DeviceIndex)) {
    for (taskIndex_t task = 0; task < TASKS_MAX; task++) {
      if ((Settings.TaskDeviceNumber[task] == PLUGIN_ID_MQTT_IMPORT) &&
          (Settings.TaskDeviceEnabled[task])) {
        // Schedule a call to each enabled MQTT import plugin to notify the broker connection state
        EventStruct event(task);
        event.Par1 = MQTTclient_connected ? 1 : 0;
        Scheduler.schedule_plugin_task_event_timer(DeviceIndex, PLUGIN_MQTT_CONNECTION_STATE, std::move(event));
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
  MQTT_queue_element *element(MQTTDelayHandler->getNext());

  if (element == nullptr) { return; }

  if (MQTTclient.publish(element->_topic.c_str(), element->_payload.c_str(), element->_retained)) {
    if (WiFiEventData.connectionFailures > 0) {
      --WiFiEventData.connectionFailures;
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
  Scheduler.setIntervalTimerOverride(ESPEasy_Scheduler::IntervalTimer_e::TIMER_MQTT, 10); // Make sure the MQTT is being processed as soon as possible.
  scheduleNextMQTTdelayQueue();
  STOP_TIMER(MQTT_DELAY_QUEUE);
}

void updateMQTTclient_connected() {
  if (MQTTclient_connected != MQTTclient.connected()) {
    MQTTclient_connected = !MQTTclient_connected;
    if (!MQTTclient_connected) {
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        String connectionError = F("MQTT : Connection lost, state: ");
        connectionError += getMQTT_state();
        addLogMove(LOG_LEVEL_ERROR, connectionError);
      }
      MQTTclient_must_send_LWT_connected = false;
    } else {
      // Now schedule all tasks using the MQTT controller.
      schedule_all_MQTTimport_tasks();
    }
    if (Settings.UseRules) {
      if (MQTTclient_connected) {
        eventQueue.add(F("MQTT#Connected"));
      } else {
        eventQueue.add(F("MQTT#Disconnected"));
      }
    }
  }
  if (!MQTTclient_connected) {
    // As suggested here: https://github.com/letscontrolit/ESPEasy/issues/1356
    if (timermqtt_interval < 30000) {
      timermqtt_interval += 5000;
    }
  } else {
    timermqtt_interval = 250;
  }
  Scheduler.setIntervalTimer(ESPEasy_Scheduler::IntervalTimer_e::TIMER_MQTT);
  scheduleNextMQTTdelayQueue();
}

void runPeriodicalMQTT() {
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
      MQTTclient.disconnect();
      updateMQTTclient_connected();
    }
  }
}

// FIXME TD-er: Must move to a more logical part of the code
controllerIndex_t firstEnabledMQTT_ControllerIndex() {
  for (controllerIndex_t i = 0; i < CONTROLLER_MAX; ++i) {
    protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(i);
    if (validProtocolIndex(ProtocolIndex)) {
      if (Protocol[ProtocolIndex].usesMQTT && Settings.ControllerEnabled[i]) {
        return i;
      }
    }
  }
  return INVALID_CONTROLLER_INDEX;
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
      MQTTclient.disconnect();
      updateMQTTclient_connected();
    }
#endif //if FEATURE_MQTT
    saveToRTC();
    delay(100); // Flush anything in the network buffers.
  }
  process_serialWriteBuffer();
}


void prepareShutdown(ESPEasy_Scheduler::IntendedRebootReason_e reason)
{
  WiFiEventData.intent_to_reboot = true;
#if FEATURE_MQTT
  runPeriodicalMQTT(); // Flush outstanding MQTT messages
#endif // if FEATURE_MQTT
  process_serialWriteBuffer();
  flushAndDisconnectAllClients();
  saveUserVarToRTC();
  setWifiMode(WIFI_OFF);
  ESPEASY_FS.end();
  delay(100); // give the node time to flush all before reboot or sleep
  node_time.now();
  Scheduler.markIntendedReboot(reason);
  saveToRTC();
}


