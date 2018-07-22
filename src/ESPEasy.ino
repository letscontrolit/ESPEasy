
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
  WiFi.persistent(false); // Do not use SDK storage of SSID/WPA parameters
  WiFi.setAutoReconnect(false);
  setWifiMode(WIFI_OFF);

  Plugin_id.resize(PLUGIN_MAX);
  Task_id_to_Plugin_id.resize(TASKS_MAX);

  checkRAM(F("setup"));
  #if defined(ESP32)
    for(byte x = 0; x < 16; x++)
      ledChannelPin[x] = -1;
  #endif

  lowestRAM = FreeMem();

  Serial.begin(115200);
  // Serial.print("\n\n\nBOOOTTT\n\n\n");

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
    Serial.println(F("\nNo (or too small) SPIFFS area..\nSystem Halted\nPlease reflash with 128k SPIFFS minimum!"));
    while (true)
      delay(1);
  }

  emergencyReset();

  String log = F("\n\n\rINIT : Booting version: ");
  log += BUILD_GIT;
  log += F(" (");
  log += getSystemLibraryString();
  log += F(")");
  addLog(LOG_LEVEL_INFO, log);



  //warm boot
  if (readFromRTC())
  {
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

//  setWifiMode(WIFI_STA);
  checkRuleSets();

  ExtraTaskSettings.TaskIndex = 255; // make sure this is an unused nr to prevent cache load on boot

  // if different version, eeprom settings structure has changed. Full Reset needed
  // on a fresh ESP module eeprom values are set to 255. Version results into -1 (signed int)
  if (Settings.Version != VERSION || Settings.PID != ESP_PROJECT_PID)
  {
    // Direct Serial is allowed here, since this is only an emergency task.
    Serial.print(F("\nPID:"));
    Serial.println(Settings.PID);
    Serial.print(F("Version:"));
    Serial.println(Settings.Version);
    Serial.println(F("INIT : Incorrect PID or version!"));
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

  PluginInit();
  CPluginInit();
  NPluginInit();
  log = F("INFO : Plugins: ");
  log += deviceCount + 1;
  log += getPluginDescriptionString();
  log += F(" (");
  log += getSystemLibraryString();
  log += F(")");
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
      xTaskCreatePinnedToCore(RTOS_TaskServers, "RTOS_TaskServers", 8192, NULL, 1, NULL, 1);
      xTaskCreatePinnedToCore(RTOS_TaskSerial, "RTOS_TaskSerial", 8192, NULL, 1, NULL, 1);
      xTaskCreatePinnedToCore(RTOS_Task10ps, "RTOS_Task10ps", 8192, NULL, 1, NULL, 1);
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
  return CPlugin_ptr[ProtocolIndex](CPLUGIN_GET_PROTOCOL_DISPLAY_NAME, &tmpEvent, protoDisplayName);
}

void updateLoopStats() {
  ++loopCounter;
  ++loopCounter_full;
  if (lastLoopStart == 0) {
    lastLoopStart = micros();
    return;
  }
  const long usecSince = usecPassedSince(lastLoopStart);
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
  if(MainLoopCall_ptr)
      MainLoopCall_ptr();

  updateLoopStats();

  if (wifiSetupConnect)
  {
    // try to connect for setup wizard
    WiFiConnectRelaxed();
    wifiSetupConnect = false;
  }
  if (wifiStatus != ESPEASY_WIFI_SERVICES_INITIALIZED) {
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

  bool firstLoopWiFiConnected = wifiStatus == ESPEASY_WIFI_SERVICES_INITIALIZED && firstLoop;
  if (firstLoopWiFiConnected) {
     firstLoop = false;
     timerAwakeFromDeepSleep = millis(); // Allow to run for "awake" number of seconds, now we have wifi.
   }

  // Deep sleep mode, just run all tasks one (more) time and go back to sleep as fast as possible
  if ((firstLoopWiFiConnected || readyForSleep()) && isDeepSleepEnabled())
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
    handle_schedule();
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

    deepSleep(Settings.Delay);
    //deepsleep will never return, its a special kind of reboot
  }
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

void updateMQTTclient_connected() {
  if (MQTTclient_connected != MQTTclient.connected()) {
    MQTTclient_connected = !MQTTclient_connected;
    if (!MQTTclient_connected)
      addLog(LOG_LEVEL_ERROR, F("MQTT : Connection lost"));
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
    PluginCall(PLUGIN_UNCONDITIONAL_POLL, 0, dummyString);
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
    String log = F("SYS  : Reset 24h counters");
    addLog(LOG_LEVEL_INFO, log);
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
          #if defined(ESP8266)
            ESP.reset();
          #endif
          #if defined(ESP32)
            ESP.restart();
          #endif
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
    Serial.print(F("Plugin calls: 50 ps:"));
    Serial.print(elapsed50ps);
    Serial.print(F(" uS, 10 ps:"));
    Serial.print(elapsed10ps);
    Serial.print(F(" uS, 10 psU:"));
    Serial.print(elapsed10psU);
    Serial.print(F(" uS, 1 ps:"));
    Serial.print(elapsed);
    Serial.println(F(" uS"));
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
  logStatistics(loglevel, true);
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

    boolean success = false;
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
          String spreValue = String(preValue[varNr]);
          String formula = ExtraTaskSettings.TaskDeviceFormula[varNr];
          float value = UserVar[varIndex + varNr];
          float result = 0;
          String svalue = String(value);
          formula.replace(F("%pvalue%"), spreValue);
          formula.replace(F("%value%"), svalue);
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
  yield();

  //prevent recursion!
  if (runningBackgroundTasks)
  {
    return;
  }
  runningBackgroundTasks=true;

  #if defined(ESP8266)
    tcpCleanup();
  #endif

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
    yield();
    ArduinoOTA.handle();
  }

  #endif

  yield();

  statusLED(false);

  runningBackgroundTasks=false;
}
