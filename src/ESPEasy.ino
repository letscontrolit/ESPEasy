#ifdef CONTINUOUS_INTEGRATION
#pragma GCC diagnostic error "-Wall"
#else
#pragma GCC diagnostic warning "-Wall"
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
//   Simple switch inputs and direct GPIO output control to drive relais, mosfets, etc
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

#include "ESPEasy-Globals.h"

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


/*********************************************************************************************\
 * SETUP
\*********************************************************************************************/
void setup()
{

  lowestRAM = FreeMem();

  Serial.begin(115200);
  // Serial.print("\n\n\nBOOOTTT\n\n\n");

  initLog();


  if (SpiffsSectors() < 32)
  {
    Serial.println(F("\nNo (or too small) SPIFFS area..\nSystem Halted\nPlease reflash with 128k SPIFFS minimum!"));
    while (true)
      delay(1);
  }

  emergencyReset();

  String log = F("\n\n\rINIT : Booting version: ");
  log += BUILD_GIT;
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

  RTC.deepSleepState=0;
  saveToRTC();

  addLog(LOG_LEVEL_INFO, log);




  fileSystemCheck();
  LoadSettings();

  checkRuleSets();
  if (strcasecmp(SecuritySettings.WifiSSID, "ssid") == 0)
    wifiSetup = true;

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
  }

  if (Settings.Build != BUILD)
    BuildFixes();


  log = F("INIT : Free RAM:");
  log += FreeMem();
  addLog(LOG_LEVEL_INFO, log);

  if (Settings.UseSerial && Settings.SerialLogLevel >= LOG_LEVEL_DEBUG_MORE)
    Serial.setDebugOutput(true);

  hardwareInit();

  //After booting, we want all the tasks to run without delaying more than neccesary.
  //Plugins that need an initial startup delay need to overwrite their initial timerSensor value in PLUGIN_INIT
  //They should also check if we returned from deep sleep so that they can skip the delay in that case.
  for (byte x = 0; x < TASKS_MAX; x++)
    if (Settings.TaskDeviceTimer[x] !=0)
      timerSensor[x] = millis() + (x * Settings.MessageDelay);

  timer100ms = 0; // timer for periodic actions 10 x per/sec
  timer1s = 0; // timer for periodic actions once per/sec
  timerwd = 0; // timer for watchdog once per 30 sec
  timermqtt = 0; // Timer for the MQTT keep alive loop.
  timermqtt_interval = 250; // Interval for checking MQTT
  timerAwakeFromDeepSleep = millis();

  PluginInit();
  CPluginInit();
  NPluginInit();
  if (Settings.UseRules)
  {
    String event = F("System#Wake");
    rulesProcessing(event);
  }

  WiFi.persistent(false); // Do not use SDK storage of SSID/WPA parameters
  WifiAPconfig();

  if (Settings.deepSleep)
  {
    //only one attempt in deepsleep, to conserve battery
    if (!WifiConnect(1))
    {
        if (Settings.deepSleepOnFail)
        {
          addLog(LOG_LEVEL_ERROR, F("SLEEP: Connection failed, going back to sleep."));
          deepSleep(Settings.Delay);
        }
    }
  }
  else
    WiFiConnectRelaxed();

  #ifdef FEATURE_REPORTING
  ReportStatus();
  #endif

  WebServerInit();

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

  // Start DNS, only used if the ESP has no valid WiFi config
  // It will reply with it's own address on all DNS requests
  // (captive portal concept)
  if (wifiSetup)
    dnsServer.start(DNS_PORT, "*", apIP);

  if (Settings.UseRules)
  {
    String event = F("System#Boot");
    rulesProcessing(event);
  }

  writeDefaultCSS();

}

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

/*********************************************************************************************\
 * MAIN LOOP
\*********************************************************************************************/
void loop()
{
  loopCounter++;

  if (wifiSetupConnect)
  {
    // try to connect for setup wizard
    WifiConnect(1);
    wifiSetupConnect = false;
  }

  // Deep sleep mode, just run all tasks one time and go back to sleep as fast as possible
  if (firstLoop && isDeepSleepEnabled())
  {
      // Setup MQTT Client
      // Controller index is forced to the first enabled MQTT controller.
      // This is normally done via frequent checks, but there's no time for in deepsleep.
      int enabledMqttController = firstEnabledMQTTController();
      if (enabledMqttController >= 0) {
        MQTTConnect(enabledMqttController);
      }
      // Now run all frequent tasks
      run50TimesPerSecond();
      run10TimesPerSecond();
      runEach30Seconds();
      runOncePerSecond();
  }
  //normal mode, run each task when its time
  else
  {

    if (timeOutReached(timer20ms))
      run50TimesPerSecond();

    if (timeOutReached(timer100ms))
      run10TimesPerSecond();

    if (timeOutReached(timerwd))
      runEach30Seconds();

    if (timeOutReached(timer1s))
      runOncePerSecond();

    if (timeOutReached(timermqtt)) {
      // MQTT_KEEPALIVE = 15 seconds.
      timermqtt = millis() + timermqtt_interval;
      //dont do this in backgroundtasks(), otherwise causes crashes. (https://github.com/letscontrolit/ESPEasy/issues/683)
      int enabledMqttController = firstEnabledMQTTController();
      if (enabledMqttController >= 0) {
        if (!MQTTclient.loop()) {
          if (!MQTTCheck(enabledMqttController)) {
            // Check failed, no need to retry it immediately.
            if (timermqtt_interval < 2000)
              timermqtt_interval += 250;
          } else {
            timermqtt_interval = 250;
          }
        }
      }
    }
  }

  backgroundtasks();

  if (readyForSleep()){
    if (Settings.UseRules)
    {
      String event = F("System#Sleep");
      rulesProcessing(event);
    }

    deepSleep(Settings.Delay);
    //deepsleep will never return, its a special kind of reboot
  }
  firstLoop = false;
}


/*********************************************************************************************\
 * Tasks that run 50 times per second
\*********************************************************************************************/

void run50TimesPerSecond()
{
  timer20ms = millis() + 20;
  PluginCall(PLUGIN_FIFTY_PER_SECOND, 0, dummyString);

  // statusLED(false);
}

/*********************************************************************************************\
 * Tasks that run 10 times per second
\*********************************************************************************************/
void run10TimesPerSecond()
{
  start = micros();
  timer100ms = millis() + 100;
  PluginCall(PLUGIN_TEN_PER_SECOND, 0, dummyString);
  if (Settings.UseRules && eventBuffer.length() > 0)
  {
    rulesProcessing(eventBuffer);
    eventBuffer = "";
  }
  elapsed = micros() - start;
  WebServer.handleClient();
}


/*********************************************************************************************\
 * Tasks each second
\*********************************************************************************************/
void runOncePerSecond()
{
  dailyResetCounter++;
  if (dailyResetCounter > 86400) // 1 day elapsed... //86400
  {
    RTC.flashDayCounter=0;
    saveToRTC();
    dailyResetCounter=0;
    String log = F("SYS  : Reset 24h counters");
    addLog(LOG_LEVEL_INFO, log);
  }

  timer1s = millis() + 1000;

  checkSensors();

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
          ESP.reset();
          break;
        }
    }
    cmd_within_mainloop = 0;
  }
  WifiCheck();

  // clock events
  if (Settings.UseNTP)
    checkTime();

  unsigned long timer = micros();
  PluginCall(PLUGIN_ONCE_A_SECOND, 0, dummyString);

  checkSystemTimers();

  if (Settings.UseRules)
    rulesTimers();

  timer = micros() - timer;

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

  if (Settings.SerialLogLevel == 5)
  {
    Serial.print(F("10 ps:"));
    Serial.print(elapsed);
    Serial.print(F(" uS  1 ps:"));
    Serial.println(timer);
  }

  if (timerAPoff != 0 && timeOutReached(timerAPoff))
  {
    timerAPoff = 0;
    WifiAPMode(false);
  }
}

/*********************************************************************************************\
 * Tasks each 30 seconds
\*********************************************************************************************/
void runEach30Seconds()
{
  wdcounter++;
  timerwd = millis() + 30000;
  char str[60];
  str[0] = 0;
  sprintf_P(str, PSTR("Uptime %u ConnectFailures %u FreeMem %u"), wdcounter / 2, connectionFailures, FreeMem());
  String log = F("WD   : ");
  log += str;
  addLog(LOG_LEVEL_INFO, log);
  sendSysInfoUDP(1);
  refreshNodeList();
  int enabledMqttController = firstEnabledMQTTController();
  if (enabledMqttController >= 0) {
    MQTTCheck(enabledMqttController);
  }
  if (Settings.UseSSDP)
    SSDP_update();
#if FEATURE_ADC_VCC
  vcc = ESP.getVcc() / 1000.0;
#endif
  loopCounterLast = loopCounter;
  loopCounter = 0;
  if (loopCounterLast > loopCounterMax)
    loopCounterMax = loopCounterLast;

  #ifdef FEATURE_REPORTING
  ReportStatus();
  #endif

}


/*********************************************************************************************\
 * Check sensor timers
\*********************************************************************************************/
void checkSensors()
{
  bool isDeepSleep = isDeepSleepEnabled();
  //check all the devices and only run the sendtask if its time, or we if we used deep sleep mode
  for (byte x = 0; x < TASKS_MAX; x++)
  {
    if (
        (Settings.TaskDeviceTimer[x] != 0) &&
        (isDeepSleep || timeOutReached(timerSensor[x]))
    )
    {
      timerSensor[x] = millis() + Settings.TaskDeviceTimer[x] * 1000;
      if (timerSensor[x] == 0) // small fix if result is 0, else timer will be stopped...
        timerSensor[x] = 1;
      SensorSendTask(x);
    }
  }
  saveUserVarToRTC();
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
      sendData(&TempEvent);
    }
  }
}


/*********************************************************************************************\
 * set global system timer
\*********************************************************************************************/
void setSystemTimer(unsigned long timer, byte plugin, byte Par1, byte Par2, byte Par3)
{
  // plugin number and par1 form a unique key that can be used to restart a timer
  // first check if a timer is not already running for this request
  boolean reUse = false;
  for (byte x = 0; x < SYSTEM_TIMER_MAX; x++)
    if (systemTimers[x].timer != 0)
    {
      if ((systemTimers[x].plugin == plugin) && (systemTimers[x].Par1 == Par1))
      {
        systemTimers[x].timer = millis() + timer;
        reUse = true;
        break;
      }
    }

  if (!reUse)
  {
    // find a new free timer slot...
    for (byte x = 0; x < SYSTEM_TIMER_MAX; x++)
      if (systemTimers[x].timer == 0)
      {
        systemTimers[x].timer = millis() + timer;
        systemTimers[x].plugin = plugin;
        systemTimers[x].Par1 = Par1;
        systemTimers[x].Par2 = Par2;
        systemTimers[x].Par3 = Par3;
        break;
      }
  }
}


//EDWIN: this function seems to be unused?
/*********************************************************************************************\
 * set global system command timer
\*********************************************************************************************/
void setSystemCMDTimer(unsigned long timer, String& action)
{
  for (byte x = 0; x < SYSTEM_CMD_TIMER_MAX; x++)
    if (systemCMDTimers[x].timer == 0)
    {
      systemCMDTimers[x].timer = millis() + timer;
      systemCMDTimers[x].action = action;
      break;
    }
}


/*********************************************************************************************\
 * check global system timers
\*********************************************************************************************/
void checkSystemTimers()
{
  for (byte x = 0; x < SYSTEM_TIMER_MAX; x++)
    if (systemTimers[x].timer != 0)
    {
      if (timeOutReached(systemTimers[x].timer))
      {
        struct EventStruct TempEvent;
        TempEvent.Par1 = systemTimers[x].Par1;
        TempEvent.Par2 = systemTimers[x].Par2;
        TempEvent.Par3 = systemTimers[x].Par3;
        for (byte y = 0; y < PLUGIN_MAX; y++)
          if (Plugin_id[y] == systemTimers[x].plugin)
            Plugin_ptr[y](PLUGIN_TIMER_IN, &TempEvent, dummyString);
        systemTimers[x].timer = 0;
      }
    }

  for (byte x = 0; x < SYSTEM_CMD_TIMER_MAX; x++)
    if (systemCMDTimers[x].timer != 0)
      if (timeOutReached(systemCMDTimers[x].timer))
      {
        struct EventStruct TempEvent;
        parseCommandString(&TempEvent, systemCMDTimers[x].action);
        if (!PluginCall(PLUGIN_WRITE, &TempEvent, systemCMDTimers[x].action))
          ExecuteCommand(VALUE_SOURCE_SYSTEM, systemCMDTimers[x].action.c_str());
        systemCMDTimers[x].timer = 0;
        systemCMDTimers[x].action = "";
      }
}


/*********************************************************************************************\
 * run background tasks
\*********************************************************************************************/
bool runningBackgroundTasks=false;
void backgroundtasks()
{
  //always start with a yield
  yield();

  //prevent recursion!
  if (runningBackgroundTasks)
  {
    return;
  }
  runningBackgroundTasks=true;

  tcpCleanup();

  if (Settings.UseSerial)
    if (Serial.available())
      if (!PluginCall(PLUGIN_SERIAL_IN, 0, dummyString))
        serial();

  // process DNS, only used if the ESP has no valid WiFi config
  if (wifiSetup)
    dnsServer.processNextRequest();

  WebServer.handleClient();

  checkUDP();

  #ifdef FEATURE_ARDUINO_OTA
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
