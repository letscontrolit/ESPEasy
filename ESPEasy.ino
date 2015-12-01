/****************************************************************************************************************************\
 * Arduino project "ESP Easy" © Copyright www.esp8266.nu
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
 * Source Code     : https://sourceforge.net/projects/espeasy/
 * Support         : http://www.esp8266.nu
 * Discussion      : http://www.esp8266.nu/forum/
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
//   DHT11/22 humidity sensors
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
//   OLED I2C display with SSD1306 driver

//   Experimental/Preliminary:
//   =========================
//   Ser2Net server
//   Local Level Control to GPIO
//   PCA9685 16 channel I2C PWM driver

// ********************************************************************************
//   User specific configuration
// ********************************************************************************

// Set default configuration settings if you want (not mandatory)
// You can allways change these during runtime and save to eeprom
// After loading firmware, issue a 'reset' command to load the defaults.

#define DEFAULT_NAME        "newdevice"         // Enter your device friendly name
#define DEFAULT_SSID        "ssid"              // Enter your network SSID
#define DEFAULT_KEY         "wpakey"            // Enter your network WPA key
#define DEFAULT_SERVER      "192.168.0.8"       // Enter your Domoticz Server IP address
#define DEFAULT_PORT        8080                // Enter your Domoticz Server port value
#define DEFAULT_DELAY       60                  // Enter your Send delay in seconds
#define DEFAULT_AP_KEY      "configesp"         // Enter network WPA key for AP (config) mode
#define DEFAULT_PROTOCOL    1                   // Protocol used for controller communications
//   1 = Domoticz HTTP
//   2 = Domoticz MQTT
//   3 = Nodo Telnet
//   4 = ThingSpeak
//   5 = OpenHAB MQTT
//   6 = PiDome MQTT
//   7 = EmonCMS
#define UNIT                0

// ********************************************************************************
//   DO NOT CHANGE ANYTHING BELOW THIS LINE
// ********************************************************************************
#define ESP_PROJECT_PID           2015050101L
#define ESP_EASY
#define VERSION                             9
#define BUILD                              48
#define REBOOT_ON_MAX_CONNECTION_FAILURES  30
#define FEATURE_SPIFFS                  false

#define CPLUGIN_PROTOCOL_ADD                1
#define CPLUGIN_PROTOCOL_TEMPLATE           2
#define CPLUGIN_PROTOCOL_SEND               3
#define CPLUGIN_PROTOCOL_RECV               4

#define LOG_LEVEL_ERROR                     1
#define LOG_LEVEL_INFO                      2
#define LOG_LEVEL_DEBUG                     3
#define LOG_LEVEL_DEBUG_MORE                4

#define CMD_REBOOT                         89
#define CMD_WIFI_DISCONNECT               135

#define DEVICES_MAX                        64
#define TASKS_MAX                          12
#define VARS_PER_TASK                       4
#define PLUGIN_MAX                         64
#define PLUGIN_CONFIGVAR_MAX                8
#define PLUGIN_CONFIGFLOATVAR_MAX           4
#define PLUGIN_CONFIGLONGVAR_MAX            4
#define PLUGIN_EXTRACONFIGVAR_MAX          16
#define CPLUGIN_MAX                        16

#define DEVICE_TYPE_SINGLE                  1  // connected through 1 datapin
#define DEVICE_TYPE_I2C                     2  // connected through I2C
#define DEVICE_TYPE_ANALOG                  3  // tout pin
#define DEVICE_TYPE_DUAL                    4  // connected through 2 datapins

#define SENSOR_TYPE_SINGLE                  1
#define SENSOR_TYPE_TEMP_HUM                2
#define SENSOR_TYPE_TEMP_BARO               3
#define SENSOR_TYPE_SWITCH                 10
#define SENSOR_TYPE_DIMMER                 11
#define SENSOR_TYPE_LONG                   20

#define PLUGIN_INIT_ALL                     1
#define PLUGIN_INIT                         2
#define PLUGIN_READ                         3
#define PLUGIN_ONCE_A_SECOND                4
#define PLUGIN_TEN_PER_SECOND               5
#define PLUGIN_DEVICE_ADD                   6
#define PLUGIN_EVENTLIST_ADD                7
#define PLUGIN_WEBFORM_SAVE                 8
#define PLUGIN_WEBFORM_LOAD                 9
#define PLUGIN_WEBFORM_SHOW_VALUES         10
#define PLUGIN_GET_DEVICENAME              11
#define PLUGIN_GET_DEVICEVALUENAMES        12
#define PLUGIN_WRITE                       13
#define PLUGIN_EVENT_OUT                   14
#define PLUGIN_WEBFORM_SHOW_CONFIG         15
#define PLUGIN_SERIAL_IN                   16
#define PLUGIN_UDP_IN                      17

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h> 
#if FEATURE_SPIFFS
#include <FS.h>
#endif

Servo myservo1;
Servo myservo2;

// MQTT client
PubSubClient MQTTclient("");

// LCD
LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

// WebServer
ESP8266WebServer WebServer(80);

// syslog stuff
WiFiUDP portRX;
WiFiUDP portTX;

struct SecurityStruct
{
  char          WifiSSID[32];
  char          WifiKey[64];
  char          WifiAPKey[64];
  char          ControllerUser[26];
  char          ControllerPassword[64];
  char          Password[26];
} SecuritySettings;

struct SettingsStruct
{
  unsigned long PID;
  int           Version;
  byte          Unit;
  byte          Controller_IP[4];
  unsigned int  ControllerPort;
  byte          IP_Octet;
  char          _obsolete[64];
  unsigned long Delay;
  int8_t        Pin_i2c_sda;
  int8_t        Pin_i2c_scl;
  byte          Syslog_IP[4];
  unsigned int  UDPPort;
  byte          Protocol;
  byte          IP[4];
  byte          Gateway[4];
  byte          Subnet[4];
  char          Name[26];
  byte          SyslogLevel;
  byte          SerialLogLevel;
  byte          WebLogLevel;
  unsigned long BaudRate;
  unsigned long MessageDelay;
  byte          TaskDeviceNumber[TASKS_MAX];
  unsigned int  TaskDeviceID[TASKS_MAX];
  int8_t        TaskDevicePin1[TASKS_MAX];
  int8_t        TaskDevicePin2[TASKS_MAX];
  byte          TaskDevicePort[TASKS_MAX];
  boolean       TaskDevicePin1PullUp[TASKS_MAX];
  int16_t       TaskDevicePluginConfig[TASKS_MAX][PLUGIN_CONFIGVAR_MAX];
  boolean       TaskDevicePin1Inversed[TASKS_MAX];
  byte          deepSleep;
  char          MQTTpublish[81];
  char          MQTTsubscribe[81];
  boolean       CustomCSS;
  float         TaskDevicePluginConfigFloat[TASKS_MAX][PLUGIN_CONFIGFLOATVAR_MAX];
  long          TaskDevicePluginConfigLong[TASKS_MAX][PLUGIN_CONFIGLONGVAR_MAX];
} Settings;

struct ExtraTaskSettingsStruct
{
  byte    TaskIndex;
  char    TaskDeviceName[26];
  char    TaskDeviceFormula[VARS_PER_TASK][41];
  char    TaskDeviceValueNames[VARS_PER_TASK][26];
  long    TaskDevicePluginConfigLong[PLUGIN_EXTRACONFIGVAR_MAX];
} ExtraTaskSettings;

struct EventStruct
{
  byte TaskIndex;
  byte BaseVarIndex;
  int idx;
  byte sensorType;
  int Par1;
  int Par2;
  int Par3;
  byte OriginTaskIndex;
  String String1;
  String String2;
  byte *Data;
};

struct LogStruct
{
  unsigned long timeStamp;
  String Message;
} Logging[10];
int logcount = -1;

struct DeviceStruct
{
  byte Number;
  byte Type;
  byte VType;
  byte Ports;
  boolean PullUpOption;
  boolean InverseLogicOption;
  boolean FormulaOption;
  byte ValueCount;
  boolean Custom;
} Device[DEVICES_MAX + 1]; // 1 more because first device is empty device

struct ProtocolStruct
{
  byte Number;
  boolean usesMQTT;
  boolean usesAccount;
  boolean usesPassword;
  char Name[20];
} Protocol[CPLUGIN_MAX];

int deviceCount = -1;
int protocolCount = -1;

boolean printToWeb = false;
String printWebString = "";

float UserVar[VARS_PER_TASK * TASKS_MAX];

unsigned long timer;
unsigned long timer100ms;
unsigned long timer1s;
unsigned long timerwd;
unsigned int NC_Count = 0;
unsigned int C_Count = 0;
boolean AP_Mode = false;
byte cmd_within_mainloop = 0;
unsigned long connectionFailures;
unsigned long wdcounter = 0;

boolean WebLoggedIn = false;
int WebLoggedInTimer = 300;

boolean (*Plugin_ptr[PLUGIN_MAX])(byte, struct EventStruct*, String&);
byte Plugin_id[PLUGIN_MAX];

boolean (*CPlugin_ptr[PLUGIN_MAX])(byte, struct EventStruct*);
byte CPlugin_id[PLUGIN_MAX];

String dummyString = "";

boolean systemOK = false;

/*********************************************************************************************\
 * SETUP
\*********************************************************************************************/
void setup()
{
  Serial.begin(115200);

#if FEATURE_SPIFFS
  fileSystemCheck();
#endif

  emergencyReset();

  LoadSettings();
  ExtraTaskSettings.TaskIndex = 255; // make sure this is an unused nr to prevent cache load on boot

  // if different version, eeprom settings structure has changed. Full Reset needed
  // on a fresh ESP module eeprom values are set to 255. Version results into -1 (signed int)
  if (Settings.Version == VERSION && Settings.PID == ESP_PROJECT_PID)
  {
    systemOK = true;
  }
  else
  {
    // Direct Serial is allowed here, since this is only an emergency task.
    Serial.print("\nPID:");
    Serial.println(Settings.PID);
    Serial.print("Version:");
    Serial.println(Settings.Version);
    Serial.println(F("INIT : Incorrect PID or version!"));
    delay(1000);
    ResetFactory();
  }

  if (systemOK)
  {
    Serial.begin(Settings.BaudRate);
    String log = F("\nINIT : Booting Build nr:");
    log += BUILD;
    addLog(LOG_LEVEL_INFO, log);

    if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG_MORE)
      Serial.setDebugOutput(true);

    WifiAPconfig();
    WifiConnect();

    hardwareInit();
    PluginInit();
    CPluginInit();

    WebServerInit();

    // setup UDP
    if (Settings.UDPPort != 0)
      portRX.begin(Settings.UDPPort);

    // Setup LCD display
    lcd.init();                      // initialize the lcd
    lcd.backlight();
    lcd.print("ESP Easy");

    // Setup MQTT Client
    byte ProtocolIndex = getProtocolIndex(Settings.Protocol);
    if (Protocol[ProtocolIndex].usesMQTT)
      MQTTConnect();

    sendSysInfoUDP(3);

    log = F("INIT : Boot OK");
    addLog(LOG_LEVEL_INFO, log);

    if (Settings.deepSleep)
    {
      log = F("INIT : Deep sleep enabled");
      addLog(LOG_LEVEL_INFO, log);
    }

    byte bootMode = 0;
    if (readFromRTC(&bootMode))
    {
      if (bootMode == 1)
        log = F("INIT : Reboot from deepsleep");
      else
        log = F("INIT : Normal boot");
    }
    else
      log = F("INIT : RTC not read");

    addLog(LOG_LEVEL_INFO, log);

    saveToRTC(0);

    // Setup timers
    if (bootMode == 0)
      timer = millis() + 30000; // startup delay 30 sec
    else
      timer = millis() + 0; // no startup from deepsleep wake up

    timer100ms = millis() + 100; // timer for periodic actions 10 x per/sec
    timer1s = millis() + 1000; // timer for periodic actions once per/sec
    timerwd = millis() + 30000; // timer for watchdog once per 30 sec
  }
  else
  {
    Serial.println(F("Entered Rescue mode!"));
  }
}


unsigned long start = 0;
unsigned long elapsed = 0;

/*********************************************************************************************\
 * MAIN LOOP
\*********************************************************************************************/
void loop()
{
  if (Serial.available())
  {
    if(!PluginCall(PLUGIN_SERIAL_IN, 0, dummyString))
      serial();
  }
  
  if (systemOK)
  {
    checkUDP();

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

    // Watchdog trigger
    if (millis() > timerwd)
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
      MQTTCheck();
    }

    // Perform regular checks, 10 times/sec
    if (millis() > timer100ms)
    {
      start = micros();
      timer100ms = millis() + 100;
      PluginCall(PLUGIN_TEN_PER_SECOND, 0, dummyString);
      elapsed = micros() - start;
    }

    // Perform regular checks, 1 time/sec
    if (millis() > timer1s)
    {
      unsigned long timer = micros();
      PluginCall(PLUGIN_ONCE_A_SECOND, 0, dummyString);
      timer = micros() - timer;
      
      timer1s = millis() + 1000;
      WifiCheck();

      if (SecuritySettings.Password[0] != 0)
      {
        if (WebLoggedIn)
          WebLoggedInTimer++;
        if (WebLoggedInTimer > 300)
          WebLoggedIn = false;
      }
      if (Settings.SerialLogLevel == 5)
      {
        Serial.print("10 ps:");
        Serial.print(elapsed);
        Serial.print(" uS  1 ps:");
        Serial.print(timer);
        Serial.println(" uS");
      }
    }

    // Check sensors and send data to controller when sensor timer has elapsed
    if (millis() > timer)
    {
      timer = millis() + Settings.Delay * 1000;
      SensorSend();
      if (Settings.deepSleep)
      {
        saveToRTC(1);
        String log = F("Enter deep sleep...");
        addLog(LOG_LEVEL_INFO, log);
        ESP.deepSleep(Settings.Delay * 1000000, WAKE_RF_DEFAULT); // Sleep for set delay
      }
    }

    if (connectionFailures > REBOOT_ON_MAX_CONNECTION_FAILURES)
      delayedReboot(60);

    backgroundtasks();
  }
  else
    delay(1);
}


void SensorSend()
{
  for (byte x = 0; x < TASKS_MAX; x++)
  {
    if (Settings.TaskDeviceID[x] != 0)
    {
      byte varIndex = x * VARS_PER_TASK;
      boolean success = false;
      byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[x]);
      LoadTaskSettings(x);

      struct EventStruct TempEvent;
      TempEvent.TaskIndex = x;
      TempEvent.BaseVarIndex = varIndex;
      TempEvent.idx = Settings.TaskDeviceID[x];
      TempEvent.sensorType = Device[DeviceIndex].VType;

      float preValue[VARS_PER_TASK]; // store values before change, in case we need it in the formula
      for (byte varNr = 0; varNr < VARS_PER_TASK; varNr++)
        preValue[varNr] = UserVar[varIndex + varNr];

      success = PluginCall(PLUGIN_READ, &TempEvent, dummyString);

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
            formula.replace("%pvalue%", spreValue);
            formula.replace("%value%", svalue);
            char TmpStr[26];
            formula.toCharArray(TmpStr, 25);
            byte error = Calculate(TmpStr, &result);
            if (error == 0)
              UserVar[varIndex + varNr] = result;
          }
        }
        sendData(&TempEvent);
      }
    }
  }
}

void backgroundtasks()
{
  WebServer.handleClient();
  MQTTclient.loop();
  delay(10);
}

