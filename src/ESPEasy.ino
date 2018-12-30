
#include <Arduino.h>

#ifdef CONTINUOUS_INTEGRATION
#pragma GCC diagnostic error "-Wall"
#else
#pragma GCC diagnostic warning "-Wall"
#endif

// Needed due to preprocessor issues.
#ifdef PLUGIN_SET_GENERIC_ESP32
  #ifndef ESP32
    #define ESP32
  #endif
#endif


/****************************************************************************************************************************\
 * Arduino project "ESP Easy" © Copyright www.letscontrolit.com
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * You received a copy of the GNU General Public License along with this program in file 'License.txt'.
 *
 * IDE download    : https://www.arduino.cc/en/Main/Software
 * ESP8266 Package : https://github.com/esp8266/Arduino
 *
 * Source Code     : https://github.com/ESP8266nu/ESPEasy
 * Support         : http://www.letscontrolit.com
 * Discussion      : http://www.letscontrolit.com/forum/
 *
 * Additional information about licensing can be found at : http://www.gnu.org/licenses
\*************************************************************************************************************************/

// This file incorporates work covered by the following copyright and permission notice:

/****************************************************************************************************************************\
* Arduino project "Nodo" © Copyright 2010..2015 Paul Tonkes
*
* This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
* You received a copy of the GNU General Public License along with this program in file 'License.txt'.
*
* Voor toelichting op de licentievoorwaarden zie    : http://www.gnu.org/licenses
* Uitgebreide documentatie is te vinden op          : http://www.nodo-domotica.nl
* Compiler voor deze programmacode te downloaden op : http://arduino.cc
\*************************************************************************************************************************/

//   Simple Arduino sketch for ESP module, supporting:
//   =================================================================================
//   Simple switch inputs and direct GPIO output control to drive relays, mosfets, etc
//   Analog input (ESP-7/12 only)
//   Pulse counters
//   Dallas OneWire DS18b20 temperature sensors
//   DHT11/22/12 humidity sensors
//   BMP085 I2C Barometric Pressure sensor
//   PCF8591 4 port Analog to Digital converter (I2C)
//   RFID Wiegand-26 reader
//   MCP23017 I2C IO Expanders
//   BH1750 I2C Luminosity sensor
//   Arduino Pro Mini with IO extender sketch, connected through I2C
//   LCD I2C display 4x20 chars
//   HC-SR04 Ultrasonic distance sensor
//   SI7021 I2C temperature/humidity sensors
//   TSL2561 I2C Luminosity sensor
//   TSOP4838 IR receiver
//   PN532 RFID reader
//   Sharp GP2Y10 dust sensor
//   PCF8574 I2C IO Expanders
//   PCA9685 I2C 16 channel PWM driver
//   OLED I2C display with SSD1306 driver
//   MLX90614 I2C IR temperature sensor
//   ADS1115 I2C ADC
//   INA219 I2C voltage/current sensor
//   BME280 I2C temp/hum/baro sensor
//   MSP5611 I2C temp/baro sensor
//   BMP280 I2C Barometric Pressure sensor
//   SHT1X temperature/humidity sensors
//   Ser2Net server

// Define globals before plugin sets to allow a personal override of the selected plugins
#include "ESPEasy-Globals.h"
#include "define_plugin_sets.h"
// Plugin helper needs the defined controller sets, thus include after 'define_plugin_sets.h'
#include "_CPlugin_Helper.h"

// Blynk_get prototype
boolean Blynk_get(const String& command, byte controllerIndex,float *data = NULL );

int firstEnabledBlynkController() {
  for (byte i = 0; i < CONTROLLER_MAX; ++i) {
    byte ProtocolIndex = getProtocolIndex(Settings.Protocol[i]);
    if (Protocol[ProtocolIndex].Number == 12 && Settings.ControllerEnabled[i]) {
      return i;
    }
  }
  return -1;
}

//void checkRAM( const __FlashStringHelper* flashString);

/*********************************************************************************************\
 * SETUP
\*********************************************************************************************/
void setup()
{
#ifdef ESP8266_DISABLE_EXTRA4K
  disable_extra4k_at_link_time();
#endif
  WiFi.persistent(false); // Do not use SDK storage of SSID/WPA parameters
  WiFi.setAutoReconnect(false);
  setWifiMode(WIFI_OFF);
  lowestFreeStack = getFreeStackWatermark();
  lowestRAM = FreeMem();

  Plugin_id.resize(PLUGIN_MAX);
  Task_id_to_Plugin_id.resize(TASKS_MAX);

  checkRAM(F("setup"));
  #if defined(ESP32)
    for(byte x = 0; x < 16; x++)
      ledChannelPin[x] = -1;
  #endif

  Serial.begin(115200);
  // serialPrint("\n\n\nBOOOTTT\n\n\n");

  initLog();

#if defined(ESP32)
  WiFi.onEvent(WiFiEvent);
#else
  // WiFi event handlers
  stationConnectedHandler = WiFi.onStationModeConnected(onConnected);
	stationDisconnectedHandler = WiFi.onStationModeDisconnected(onDisconnect);
	stationGotIpHandler = WiFi.onStationModeGotIP(onGotIP);
  APModeStationConnectedHandler = WiFi.onSoftAPModeStationConnected(onConnectedAPmode);
  APModeStationDisconnectedHandler = WiFi.onSoftAPModeStationDisconnected(onDisonnectedAPmode);
#endif

  if (SpiffsSectors() < 32)
  {
    serialPrintln(F("\nNo (or too small) SPIFFS area..\nSystem Halted\nPlease reflash with 128k SPIFFS minimum!"));
    while (true)
      delay(1);
  }

  emergencyReset();

  String log = F("\n\n\rINIT : Booting version: ");
  log += BUILD_GIT;
  log += " (";
  log += getSystemLibraryString();
  log += ')';
  addLog(LOG_LEVEL_INFO, log);


  //warm boot
  if (readFromRTC())
  {
    RTC.bootFailedCount++;
    RTC.bootCounter++;
    readUserVarFromRTC();

    if (RTC.deepSleepState == 1)
    {
      log = F("INIT : Rebooted from deepsleep #");
      lastBootCause=BOOT_CAUSE_DEEP_SLEEP;
    }
    else
      log = F("INIT : Warm boot #");

    log += RTC.bootCounter;

  }
  //cold boot (RTC memory empty)
  else
  {
    initRTC();

    // cold boot situation
    if (lastBootCause == BOOT_CAUSE_MANUAL_REBOOT) // only set this if not set earlier during boot stage.
      lastBootCause = BOOT_CAUSE_COLD_BOOT;
    log = F("INIT : Cold Boot");
  }
  log += F(" - Restart Reason: ");
  log += getResetReasonString();

  RTC.deepSleepState=0;
  saveToRTC();

  addLog(LOG_LEVEL_INFO, log);

  fileSystemCheck();
  progMemMD5check();
  LoadSettings();
  Settings.UseRTOSMultitasking = false; // For now, disable it, we experience heap corruption.
  if (RTC.bootFailedCount > 10 && RTC.bootCounter > 10) {
    byte toDisable = RTC.bootFailedCount - 10;
    toDisable = disablePlugin(toDisable);
    if (toDisable != 0) {
      toDisable = disableController(toDisable);
    }
    if (toDisable != 0) {
      toDisable = disableNotification(toDisable);
    }
  }

//  setWifiMode(WIFI_STA);
  checkRuleSets();

  // if different version, eeprom settings structure has changed. Full Reset needed
  // on a fresh ESP module eeprom values are set to 255. Version results into -1 (signed int)
  if (Settings.Version != VERSION || Settings.PID != ESP_PROJECT_PID)
  {
    // Direct Serial is allowed here, since this is only an emergency task.
    serialPrint(F("\nPID:"));
    serialPrintln(String(Settings.PID));
    serialPrint(F("Version:"));
    serialPrintln(String(Settings.Version));
    serialPrintln(F("INIT : Incorrect PID or version!"));
    delay(1000);
    ResetFactory();
  }

  if (Settings.UseSerial)
  {
    //make sure previous serial buffers are flushed before resetting baudrate
    Serial.flush();
    Serial.begin(Settings.BaudRate);
//    Serial.setDebugOutput(true);
  }

  if (Settings.Build != BUILD)
    BuildFixes();


  log = F("INIT : Free RAM:");
  log += FreeMem();
  addLog(LOG_LEVEL_INFO, log);

  if (Settings.UseSerial && Settings.SerialLogLevel >= LOG_LEVEL_DEBUG_MORE)
    Serial.setDebugOutput(true);

  checkRAM(F("hardwareInit"));
  hardwareInit();

  timermqtt_interval = 250; // Interval for checking MQTT
  timerAwakeFromDeepSleep = millis();
  if (Settings.UseRules && isDeepSleepEnabled())
  {
    String event = F("System#NoSleep=");
    event += Settings.deepSleep;
    rulesProcessing(event);
  }

  PluginInit();
  CPluginInit();
  NPluginInit();
  log = F("INFO : Plugins: ");
  log += deviceCount + 1;
  log += getPluginDescriptionString();
  log += " (";
  log += getSystemLibraryString();
  log += ')';
  addLog(LOG_LEVEL_INFO, log);

  if (deviceCount + 1 >= PLUGIN_MAX) {
    addLog(LOG_LEVEL_ERROR, F("Programming error! - Increase PLUGIN_MAX"));
  }

  if (Settings.UseRules)
  {
    String event = F("System#Wake");
    rulesProcessing(event);
  }

  if (!selectValidWiFiSettings()) {
    wifiSetup = true;
  }
/*
  // FIXME TD-er:
  // Async scanning for wifi doesn't work yet like it should.
  // So no selection of strongest network yet.
  if (selectValidWiFiSettings()) {
    WifiScanAsync();
  }
*/
  WiFiConnectRelaxed();

  #ifdef FEATURE_REPORTING
  ReportStatus();
  #endif

  #ifdef FEATURE_ARDUINO_OTA
  ArduinoOTAInit();
  #endif

  // setup UDP
  if (Settings.UDPPort != 0)
    portUDP.begin(Settings.UDPPort);

  sendSysInfoUDP(3);

  if (Settings.UseNTP)
    initTime();

#if FEATURE_ADC_VCC
  vcc = ESP.getVcc() / 1000.0;
#endif

  if (Settings.UseRules)
  {
    String event = F("System#Boot");
    rulesProcessing(event);
  }

  writeDefaultCSS();

  UseRTOSMultitasking = Settings.UseRTOSMultitasking;
  #ifdef USE_RTOS_MULTITASKING
    if(UseRTOSMultitasking){
      log = F("RTOS : Launching tasks");
      addLog(LOG_LEVEL_INFO, log);
      xTaskCreatePinnedToCore(RTOS_TaskServers, "RTOS_TaskServers", 16384, NULL, 1, NULL, 1);
      xTaskCreatePinnedToCore(RTOS_TaskSerial, "RTOS_TaskSerial", 8192, NULL, 1, NULL, 1);
      xTaskCreatePinnedToCore(RTOS_Task10ps, "RTOS_Task10ps", 8192, NULL, 1, NULL, 1);
      xTaskCreatePinnedToCore(
                    RTOS_HandleSchedule,   /* Function to implement the task */
                    "RTOS_HandleSchedule", /* Name of the task */
                    16384,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    1,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    1);         /* Core where the task should run */
    }
  #endif

//  #ifndef ESP32
//  connectionCheck.attach(30, connectionCheckHandler);
//  #endif

  // Start the interval timers at N msec from now.
  // Make sure to start them at some time after eachother,
  // since they will keep running at the same interval.
  setIntervalTimerOverride(TIMER_20MSEC,  5); // timer for periodic actions 50 x per/sec
  setIntervalTimerOverride(TIMER_100MSEC, 66); // timer for periodic actions 10 x per/sec
  setIntervalTimerOverride(TIMER_1SEC,    777); // timer for periodic actions once per/sec
  setIntervalTimerOverride(TIMER_30SEC,   1333); // timer for watchdog once per 30 sec
  setIntervalTimerOverride(TIMER_MQTT,    88); // timer for interaction with MQTT
  setIntervalTimerOverride(TIMER_STATISTICS, 2222);
}

#ifdef USE_RTOS_MULTITASKING
void RTOS_TaskServers( void * parameter )
{
 while (true){
  delay(100);
  WebServer.handleClient();
  checkUDP();
 }
}

void RTOS_TaskSerial( void * parameter )
{
 while (true){
    delay(100);
    if (Settings.UseSerial)
    if (Serial.available())
      if (!PluginCall(PLUGIN_SERIAL_IN, 0, dummyString))
        serial();
 }
}

void RTOS_Task10ps( void * parameter )
{
 while (true){
    delay(100);
    run10TimesPerSecond();
 }
}

void RTOS_HandleSchedule( void * parameter )
{
 while (true){
    handle_schedule();
 }
}

#endif

int firstEnabledMQTTController() {
  for (byte i = 0; i < CONTROLLER_MAX; ++i) {
    byte ProtocolIndex = getProtocolIndex(Settings.Protocol[i]);
    if (Protocol[ProtocolIndex].usesMQTT && Settings.ControllerEnabled[i]) {
      return i;
    }
  }
  return -1;
}

bool getControllerProtocolDisplayName(byte ProtocolIndex, byte parameterIdx, String& protoDisplayName) {
  EventStruct tmpEvent;
  tmpEvent.idx=parameterIdx;
  return CPluginCall(ProtocolIndex, CPLUGIN_GET_PROTOCOL_DISPLAY_NAME, &tmpEvent, protoDisplayName);
}

void updateLoopStats() {
  ++loopCounter;
  ++loopCounter_full;
  if (lastLoopStart == 0) {
    lastLoopStart = micros();
    return;
  }
  const long usecSince = usecPassedSince(lastLoopStart);
  miscStats[LOOP_STATS].add(usecSince);

  loop_usec_duration_total += usecSince;
  lastLoopStart = micros();
  if (usecSince <= 0 || usecSince > 10000000)
    return; // No loop should take > 10 sec.
  if (shortestLoop > static_cast<unsigned long>(usecSince)) {
    shortestLoop = usecSince;
    loopCounterMax = 30 * 1000000 / usecSince;
  }
  if (longestLoop < static_cast<unsigned long>(usecSince))
    longestLoop = usecSince;
}

void updateLoopStats_30sec(byte loglevel) {
  loopCounterLast = loopCounter;
  loopCounter = 0;
  if (loopCounterLast > loopCounterMax)
    loopCounterMax = loopCounterLast;

  msecTimerHandler.updateIdleTimeStats();

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
    log += F(" countFindPluginId: ");
    log += countFindPluginId;
    addLog(loglevel, log);
  }
  countFindPluginId = 0;
  loop_usec_duration_total = 0;
  loopCounter_full = 1;
}

float getCPUload() {
  return 100.0 - msecTimerHandler.getIdleTimePct();
}

int getLoopCountPerSec() {
  return loopCounterLast / 30;
}




/*********************************************************************************************\
 * MAIN LOOP
\*********************************************************************************************/
void loop()
{
  /*
  //FIXME TD-er: No idea what this does.
  if(MainLoopCall_ptr)
      MainLoopCall_ptr();
  */

  updateLoopStats();

  if (wifiSetupConnect)
  {
    // try to connect for setup wizard
    WiFiConnectRelaxed();
    wifiSetupConnect = false;
  }
  if (wifiStatus != ESPEASY_WIFI_SERVICES_INITIALIZED || unprocessedWifiEvents()) {
    // WiFi connection is not yet available, so introduce some extra delays to
    // help the background tasks managing wifi connections
    delay(1);
    if (wifiStatus >= ESPEASY_WIFI_CONNECTED) processConnect();
    if (wifiStatus >= ESPEASY_WIFI_GOT_IP) processGotIP();
    if (wifiStatus == ESPEASY_WIFI_DISCONNECTED) processDisconnect();
  } else if (!WiFiConnected()) {
    // Somehow the WiFi has entered a limbo state.
    // FIXME TD-er: This may happen on WiFi config with AP_STA mode active.
//    addLog(LOG_LEVEL_ERROR, F("Wifi status out sync"));
//    resetWiFi();
  }
  if (!processedConnectAPmode) processConnectAPmode();
  if (!processedDisconnectAPmode) processDisconnectAPmode();
  if (!processedScanDone) processScanDone();

  bool firstLoopConnectionsEstablished = checkConnectionsEstablished() && firstLoop;
  if (firstLoopConnectionsEstablished) {
     firstLoop = false;
     timerAwakeFromDeepSleep = millis(); // Allow to run for "awake" number of seconds, now we have wifi.
     // schedule_all_task_device_timers(); Disabled for now, since we are now using queues for controllers.
     if (Settings.UseRules && isDeepSleepEnabled())
     {
        String event = F("System#NoSleep=");
        event += Settings.deepSleep;
        rulesProcessing(event);
     }


     RTC.bootFailedCount = 0;
     saveToRTC();
  }

  // Deep sleep mode, just run all tasks one (more) time and go back to sleep as fast as possible
  if ((firstLoopConnectionsEstablished || readyForSleep()) && isDeepSleepEnabled())
  {
      runPeriodicalMQTT();
      // Now run all frequent tasks
      run50TimesPerSecond();
      run10TimesPerSecond();
      runEach30Seconds();
      runOncePerSecond();
  }
  //normal mode, run each task when its time
  else
  {
    if (!UseRTOSMultitasking) {
      // On ESP32 the schedule is executed on the 2nd core.
      handle_schedule();
    }
  }

  backgroundtasks();

  if (readyForSleep()){
    if (Settings.UseRules)
    {
      String event = F("System#Sleep");
      rulesProcessing(event);
    }
    // Flush outstanding MQTT messages
    runPeriodicalMQTT();
    flushAndDisconnectAllClients();

    deepSleep(Settings.Delay);
    //deepsleep will never return, its a special kind of reboot
  }
}

bool checkConnectionsEstablished() {
  if (wifiStatus != ESPEASY_WIFI_SERVICES_INITIALIZED) return false;
  if (firstEnabledMQTTController() >= 0) {
    // There should be a MQTT connection.
    return MQTTclient_connected;
  }
  return true;
}

void flushAndDisconnectAllClients() {
  if (MQTTclient.connected()) {
    MQTTclient.disconnect();
    updateMQTTclient_connected();
  }
  /// FIXME TD-er: add call to all controllers (delay queue) to flush all data.
}

void runPeriodicalMQTT() {
  // MQTT_KEEPALIVE = 15 seconds.
  if (!WiFiConnected(10)) {
    updateMQTTclient_connected();
    return;
  }
  //dont do this in backgroundtasks(), otherwise causes crashes. (https://github.com/letscontrolit/ESPEasy/issues/683)
  int enabledMqttController = firstEnabledMQTTController();
  if (enabledMqttController >= 0) {
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

String getMQTT_state() {
  switch (MQTTclient.state()) {
    case MQTT_CONNECTION_TIMEOUT     : return F("Connection timeout");
    case MQTT_CONNECTION_LOST        : return F("Connection lost");
    case MQTT_CONNECT_FAILED         : return F("Connect failed");
    case MQTT_DISCONNECTED           : return F("Disconnected");
    case MQTT_CONNECTED              : return F("Connected");
    case MQTT_CONNECT_BAD_PROTOCOL   : return F("Connect bad protocol");
    case MQTT_CONNECT_BAD_CLIENT_ID  : return F("Connect bad client_id");
    case MQTT_CONNECT_UNAVAILABLE    : return F("Connect unavailable");
    case MQTT_CONNECT_BAD_CREDENTIALS: return F("Connect bad credentials");
    case MQTT_CONNECT_UNAUTHORIZED   : return F("Connect unauthorized");
    default: return "";
  }
}

void updateMQTTclient_connected() {
  if (MQTTclient_connected != MQTTclient.connected()) {
    MQTTclient_connected = !MQTTclient_connected;
    if (!MQTTclient_connected) {
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        String connectionError = F("MQTT : Connection lost, state: ");
        connectionError += getMQTT_state();
        addLog(LOG_LEVEL_ERROR, connectionError);
      }
    } else {
      schedule_all_tasks_using_MQTT_controller();
    }
    if (Settings.UseRules) {
      String event = MQTTclient_connected ? F("MQTT#Connected") : F("MQTT#Disconnected");
      rulesProcessing(event);
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
  setIntervalTimer(TIMER_MQTT);
}

/*********************************************************************************************\
 * Tasks that run 50 times per second
\*********************************************************************************************/

void run50TimesPerSecond() {
  START_TIMER;
  PluginCall(PLUGIN_FIFTY_PER_SECOND, 0, dummyString);
  STOP_TIMER(PLUGIN_CALL_50PS);
}

/*********************************************************************************************\
 * Tasks that run 10 times per second
\*********************************************************************************************/
void run10TimesPerSecond() {
  {
    START_TIMER;
    PluginCall(PLUGIN_TEN_PER_SECOND, 0, dummyString);
    STOP_TIMER(PLUGIN_CALL_10PS);
  }
  {
    START_TIMER;
//    PluginCall(PLUGIN_UNCONDITIONAL_POLL, 0, dummyString);
    PluginCall(PLUGIN_MONITOR, 0, dummyString);
    STOP_TIMER(PLUGIN_CALL_10PSU);
  }
  if (Settings.UseRules && eventBuffer.length() > 0)
  {
    rulesProcessing(eventBuffer);
    eventBuffer = "";
  }
  #ifndef USE_RTOS_MULTITASKING
    WebServer.handleClient();
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
    if (connectionFailures > Settings.ConnectionFailuresThreshold)
      delayedReboot(60);

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
          reboot();
          break;
        }
    }
    cmd_within_mainloop = 0;
  }
  WifiCheck();

  // clock events
  if (Settings.UseNTP)
    checkTime();

//  unsigned long start = micros();
  PluginCall(PLUGIN_ONCE_A_SECOND, 0, dummyString);
//  unsigned long elapsed = micros() - start;

  if (Settings.UseRules)
    rulesTimers();


  if (SecuritySettings.Password[0] != 0)
  {
    if (WebLoggedIn)
      WebLoggedInTimer++;
    if (WebLoggedInTimer > 300)
      WebLoggedIn = false;
  }

  // I2C Watchdog feed
  if (Settings.WDI2CAddress != 0)
  {
    Wire.beginTransmission(Settings.WDI2CAddress);
    Wire.write(0xA5);
    Wire.endTransmission();
  }

/*
  if (Settings.SerialLogLevel == LOG_LEVEL_DEBUG_DEV)
  {
    serialPrint(F("Plugin calls: 50 ps:"));
    serialPrint(elapsed50ps);
    serialPrint(F(" uS, 10 ps:"));
    serialPrint(elapsed10ps);
    serialPrint(F(" uS, 10 psU:"));
    serialPrint(elapsed10psU);
    serialPrint(F(" uS, 1 ps:"));
    serialPrint(elapsed);
    serialPrintln(F(" uS"));
    elapsed50ps=0;
    elapsed10ps=0;
    elapsed10psU=0;
  }
  */
  checkResetFactoryPin();
  STOP_TIMER(PLUGIN_CALL_1PS);
}

void logTimerStatistics() {
  byte loglevel = LOG_LEVEL_DEBUG;
  updateLoopStats_30sec(loglevel);
//  logStatistics(loglevel, true);
  if (loglevelActiveFor(loglevel)) {
    String queueLog = F("Scheduler stats: (called/tasks/max_length/idle%) ");
    queueLog += msecTimerHandler.getQueueStats();
    addLog(loglevel, queueLog);
  }
}

/*********************************************************************************************\
 * Tasks each 30 seconds
\*********************************************************************************************/
void runEach30Seconds()
{
   extern void checkRAMtoLog();
  checkRAMtoLog();
  wdcounter++;
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    log.reserve(60);
    log = F("WD   : Uptime ");
    log += wdcounter / 2;
    log += F(" ConnectFailures ");
    log += connectionFailures;
    log += F(" FreeMem ");
    log += FreeMem();
    addLog(LOG_LEVEL_INFO, log);
  }
  sendSysInfoUDP(1);
  refreshNodeList();

  #if defined(ESP8266)
  if (Settings.UseSSDP)
    SSDP_update();
  #endif
#if FEATURE_ADC_VCC
  vcc = ESP.getVcc() / 1000.0;
#endif

  #ifdef FEATURE_REPORTING
  ReportStatus();
  #endif

}


/*********************************************************************************************\
 * send all sensordata
\*********************************************************************************************/
// void SensorSendAll()
// {
//   for (byte x = 0; x < TASKS_MAX; x++)
//   {
//     SensorSendTask(x);
//   }
// }


/*********************************************************************************************\
 * send specific sensor task data
\*********************************************************************************************/
void SensorSendTask(byte TaskIndex)
{
  checkRAM(F("SensorSendTask"));
  if (Settings.TaskDeviceEnabled[TaskIndex])
  {
    byte varIndex = TaskIndex * VARS_PER_TASK;

    bool success = false;
    byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[TaskIndex]);
    LoadTaskSettings(TaskIndex);

    struct EventStruct TempEvent;
    TempEvent.TaskIndex = TaskIndex;
    TempEvent.BaseVarIndex = varIndex;
    // TempEvent.idx = Settings.TaskDeviceID[TaskIndex]; todo check
    TempEvent.sensorType = Device[DeviceIndex].VType;

    float preValue[VARS_PER_TASK]; // store values before change, in case we need it in the formula
    for (byte varNr = 0; varNr < VARS_PER_TASK; varNr++)
      preValue[varNr] = UserVar[varIndex + varNr];

    if(Settings.TaskDeviceDataFeed[TaskIndex] == 0)  // only read local connected sensorsfeeds
      success = PluginCall(PLUGIN_READ, &TempEvent, dummyString);
    else
      success = true;

    if (success)
    {
      START_TIMER;
      for (byte varNr = 0; varNr < VARS_PER_TASK; varNr++)
      {
        if (ExtraTaskSettings.TaskDeviceFormula[varNr][0] != 0)
        {
          String formula = ExtraTaskSettings.TaskDeviceFormula[varNr];
          formula.replace(F("%pvalue%"), String(preValue[varNr]));
          formula.replace(F("%value%"), String(UserVar[varIndex + varNr]));
          float result = 0;
          byte error = Calculate(formula.c_str(), &result);
          if (error == 0)
            UserVar[varIndex + varNr] = result;
        }
      }
      STOP_TIMER(COMPUTE_FORMULA_STATS);
      sendData(&TempEvent);
    }
  }
}


/*********************************************************************************************\
 * run background tasks
\*********************************************************************************************/
bool runningBackgroundTasks=false;
void backgroundtasks()
{
  //checkRAM(F("backgroundtasks"));
  //always start with a yield
  delay(0);
/*
  // Remove this watchdog feed for now.
  // See https://github.com/letscontrolit/ESPEasy/issues/1722#issuecomment-419659193

  #ifdef ESP32
  // Have to find a similar function to call ESP32's esp_task_wdt_feed();
  #else
  ESP.wdtFeed();
  #endif
*/

  //prevent recursion!
  if (runningBackgroundTasks)
  {
    return;
  }
  START_TIMER
  runningBackgroundTasks=true;

  #if defined(ESP8266)
    tcpCleanup();
  #endif
  process_serialWriteBuffer();
  if(!UseRTOSMultitasking){
    if (Settings.UseSerial)
      if (Serial.available())
        if (!PluginCall(PLUGIN_SERIAL_IN, 0, dummyString))
          serial();
    WebServer.handleClient();
    checkUDP();
  }

  // process DNS, only used if the ESP has no valid WiFi config
  if (dnsServerActive)
    dnsServer.processNextRequest();

  #ifdef FEATURE_ARDUINO_OTA
  if(Settings.ArduinoOTAEnable)
    ArduinoOTA.handle();

  //once OTA is triggered, only handle that and dont do other stuff. (otherwise it fails)
  while (ArduinoOTAtriggered)
  {
    delay(0);
    ArduinoOTA.handle();
  }

  #endif

  delay(0);

  statusLED(false);

  runningBackgroundTasks=false;
  STOP_TIMER(BACKGROUND_TASKS);
}
