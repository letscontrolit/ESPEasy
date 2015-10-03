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

// Simple Arduino sketch for ESP module, supporting:
//   Dallas OneWire DS18b20 temperature sensors
//   DHT11/22 humidity sensors
//   BH1750 I2C Lux sensor
//   BMP085 I2C Barometric Pressure sensor
//   RFID Wiegand-26 reader
//   MCP23017 I2C IO Expanders
//   Analog input (ESP-7/12 only)
//   PCF8591 4 port Analog to Digital converter (I2C)
//   HC-SR04 Ultrasonic distance sensor
//   LCD I2C display 4x20 chars
//   Pulse counters
//   Simple switch inputs
//   Direct GPIO output control to drive relais, mosfets, etc
//   Arduino Pro Mini with IO extender sketch, connected through I2C

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
#define UNIT                0

// ********************************************************************************
//   DO NOT CHANGE ANYTHING BELOW THIS LINE
// ********************************************************************************
#define ESP_PROJECT_PID           2015050101L
#define ESP_EASY
#define VERSION                             9
#define BUILD                              29
#define REBOOT_ON_MAX_CONNECTION_FAILURES  30
#define FEATURE_SPIFFS                  false

#define PROTOCOL_DOMOTICZ_HTTP              1
#define PROTOCOL_DOMOTICZ_MQTT              2
#define PROTOCOL_NODO_TELNET                3
#define PROTOCOL_THINGSPEAK                 4
#define PROTOCOL_OPENHAB_MQTT               5
#define PROTOCOL_PIDOME_MQTT                6

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
#define PLUGIN_EXTRACONFIGVAR_MAX          16

#define DEVICE_TYPE_SINGLE                  1  // connected through 1 datapin
#define DEVICE_TYPE_I2C                     2  // connected through I2C
#define DEVICE_TYPE_ANALOG                  3  // tout pin
#define DEVICE_TYPE_DUAL                    4  // connected through 2 datapins

#define SENSOR_TYPE_SINGLE                  1
#define SENSOR_TYPE_TEMP_HUM                2
#define SENSOR_TYPE_TEMP_BARO               3
#define SENSOR_TYPE_SWITCH                 10
#define SENSOR_TYPE_DIMMER                 11

#define PLUGIN_INIT_ALL                     1
#define PLUGIN_INIT                         2
#define PLUGIN_READ                         3
#define PLUGIN_ONCE_A_SECOND                4
#define PLUGIN_TEN_PER_SECOND               5
#define PLUGIN_DEVICE_ADD                   6
#define PLUGIN_EVENTLIST_ADD                7
#define PLUGIN_WEBFORM_SAVE                 8
#define PLUGIN_WEBFORM_LOAD                 9
#define PLUGIN_WEBFORM_VALUES              10
#define PLUGIN_GET_DEVICENAME              11
#define PLUGIN_GET_DEVICEVALUENAMES        12
#define PLUGIN_WRITE                       13
#define PLUGIN_EVENT_OUT                   14

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#if FEATURE_SPIFFS
  #include <FS.h>
#endif

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
  char          ControllerPassword[26];
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
} Settings;

struct ExtraTaskSettingsStruct
{
  byte    TaskIndex;
  char    TaskDeviceName[26];
  char    TaskDeviceFormula[VARS_PER_TASK][41];
  char    TaskDeviceValueNames[VARS_PER_TASK][26];
  long    TaskDevicePluginConfig[PLUGIN_EXTRACONFIGVAR_MAX];
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
} Device[DEVICES_MAX + 1]; // 1 more because first device is empty device

int deviceCount = -1;

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

String dummyString = "";

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
  if (Settings.Version != VERSION || Settings.PID != ESP_PROJECT_PID)
  {
    Serial.println(Settings.PID);
    Serial.println(Settings.Version);
    Serial.println(F("INIT : Incorrect PID or version!"));
    delay(10000);
    ResetFactory();
  }

  Serial.begin(Settings.BaudRate);
  Serial.print(F("\nINIT : Booting Build nr:"));
  Serial.println(BUILD);

  if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG_MORE)
    Serial.setDebugOutput(true);

  WifiAPconfig();
  WifiConnect();

  hardwareInit();
  PluginInit();

  WebServerInit();

  // setup UDP
  if (Settings.UDPPort != 0)
    portRX.begin(Settings.UDPPort);

  // Setup LCD display
  lcd.init();                      // initialize the lcd
  lcd.backlight();
  lcd.print("ESP Easy");

  // Setup MQTT Client
  if ((Settings.Protocol == PROTOCOL_DOMOTICZ_MQTT) || (Settings.Protocol == PROTOCOL_OPENHAB_MQTT) || (Settings.Protocol == PROTOCOL_PIDOME_MQTT))
    MQTTConnect();

  sendSysInfoUDP(3);
  Serial.println(F("INIT : Boot OK"));
  addLog(LOG_LEVEL_INFO, (char*)"Boot");

  if (Settings.deepSleep)
    Serial.println(F("INIT : Deep sleep enabled"));

  byte bootMode = 0;
  if (readFromRTC(&bootMode))
  {
    if (bootMode == 1)
      Serial.println(F("INIT : Reboot from deepsleep"));
    else
      Serial.println(F("INIT : Normal boot"));
  }
  else
    Serial.println(F("INIT : RTC not read"));

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


/*********************************************************************************************\
 * MAIN LOOP
\*********************************************************************************************/
void loop()
{
  if (Serial.available())
    serial();

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
    char str[40];
    str[0] = 0;
    Serial.print("WD   : ");
    sprintf_P(str, PSTR("Uptime %u ConnectFailures %u FreeMem %u"), wdcounter / 2, connectionFailures, FreeMem());
    Serial.println(str);
    syslog(str);
    sendSysInfoUDP(1);
    refreshNodeList();
    MQTTCheck();
  }

  // Perform regular checks, 10 times/sec
  if (millis() > timer100ms)
  {
    timer100ms = millis() + 100;
    PluginCall(PLUGIN_TEN_PER_SECOND, 0, dummyString);
  }

  // Perform regular checks, 1 time/sec
  if (millis() > timer1s)
  {
    PluginCall(PLUGIN_ONCE_A_SECOND, 0, dummyString);

    timer1s = millis() + 1000;
    WifiCheck();

    if (SecuritySettings.Password[0] != 0)
    {
      if (WebLoggedIn)
        WebLoggedInTimer++;
      if (WebLoggedInTimer > 300)
        WebLoggedIn = false;
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
      Serial.println(F("Enter deep sleep..."));
      ESP.deepSleep(Settings.Delay * 1000000, WAKE_RF_DEFAULT); // Sleep for set delay
    }
  }

  if (connectionFailures > REBOOT_ON_MAX_CONNECTION_FAILURES)
    delayedReboot(60);

  backgroundtasks();

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
            String spreValue= String(preValue[varNr]);
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

