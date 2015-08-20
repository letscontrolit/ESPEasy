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
//   LCD I2C display 4x20 chars
//   Pulse counters
//   Simple switch inputs
//   Direct GPIO output control to drive relais, mosfets, etc

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
#define DEFAULT_DALLAS_IDX  0                   // Enter IDX of your virtual temperature device
#define DEFAULT_DHT_IDX     0                   // Enter IDX of your virtual Humidity device
#define DEFAULT_DHT_TYPE    11                  // Enter Type of your virtual Humidity device
#define DEFAULT_BMP_IDX     0                   // Enter IDX of your virtual Barometric Pressure device
#define DEFAULT_LUX_IDX     0                   // Enter IDX of your virtual LUX device
#define DEFAULT_RFID_IDX    0                   // Enter IDX of your virtual RFID device
#define DEFAULT_ANALOG_IDX  0                   // Enter IDX of your virtual Analog device
#define DEFAULT_PULSE1_IDX  0                   // Enter IDX of your virtual Pulse counter
#define DEFAULT_SWITCH1_IDX 0                   // Enter IDX of your switch device
#define DEFAULT_PROTOCOL    1                   // Protocol used for controller communications
                                                //   1 = Domoticz HTTP
                                                //   2 = Domoticz MQTT
                                                //   3 = Nodo Telnet
#define UNIT                0

// ********************************************************************************
//   DO NOT CHANGE ANYTHING BELOW THIS LINE
// ********************************************************************************

// sensor types
//  1 = single value, general purpose (Dallas, LUX, counters, etc)
//  2 = temp + hum (DHT)
//  3 = temp + hum + baro (BMP085)
// 10 = switch

#define ESP_PROJECT_PID   2015050101L
#define ESP_EASY
#define VERSION           4
#define BUILD            14
#define REBOOT_ON_MAX_CONNECTION_FAILURES  30

#define LOG_LEVEL_ERROR      1
#define LOG_LEVEL_INFO       2
#define LOG_LEVEL_DEBUG      3
#define LOG_LEVEL_DEBUG_MORE 4

#define CMD_REBOOT                      89
#define CMD_WIFI_DISCONNECT             135

#define DEVICE_DS18B20        1
#define DEVICE_DHT11          2
#define DEVICE_DHT22          3
#define DEVICE_BMP085         4
#define DEVICE_BH1750         5
#define DEVICE_ANALOG         6
#define DEVICE_RFID           7
#define DEVICE_PULSE          8
#define DEVICE_SWITCH         9

#define DEVICES_MAX 10
#define TASKS_MAX 8

#define DEVICE_TYPE_SINGLE    1  // connected through 1 datapin
#define DEVICE_TYPE_I2C       2  // connected through I2C
#define DEVICE_TYPE_ANALOG    3  // tout pin
#define DEVICE_TYPE_DUAL      4  // connected through 2 datapins

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

// MQTT client
PubSubClient MQTTclient("");

// LCD
LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

// WebServer
ESP8266WebServer WebServer(80);

// syslog stuff
WiFiUDP portRX;
WiFiUDP portTX;

struct SettingsStruct
{
  unsigned long PID;
  int           Version;
  byte          Unit;
  char          WifiSSID[26];
  char          WifiKey[26];
  byte          Controller_IP[4];
  unsigned int  ControllerPort;
  byte          IP_Octet;
  char          WifiAPKey[26];
  unsigned long Delay;
  unsigned int  _Dallas;
  unsigned int  _DHT;
  byte          _DHTType;
  unsigned int  _BMP;
  unsigned int  _LUX;
  unsigned int  _RFID;
  unsigned int  _Analog;
  unsigned int  _Pulse1;
  byte          _BoardType;
  int8_t        Pin_i2c_sda;
  int8_t        Pin_i2c_scl;
  int8_t        _Pin_wired_in_1;
  int8_t        _Pin_wired_in_2;
  int8_t        _Pin_wired_out_1;
  int8_t        _Pin_wired_out_2;
  byte          Syslog_IP[4];
  unsigned int  UDPPort;
  unsigned int  Switch1;
  byte          Protocol;
  byte          IP[4];
  byte          Gateway[4];
  byte          Subnet[4];
  byte          Debug;
  char          Name[26];
  byte          SyslogLevel;
  byte          SerialLogLevel;
  byte          WebLogLevel;
  unsigned long BaudRate;
  char          ControllerUser[26];
  char          ControllerPassword[26];
  char          Password[26];
  unsigned long MessageDelay;
  byte          TaskDeviceNumber[TASKS_MAX];
  unsigned int  TaskDeviceID[TASKS_MAX];
  int8_t        TaskDevicePin1[TASKS_MAX];
  int8_t        TaskDevicePin2[TASKS_MAX];
} Settings;

struct LogStruct
{
  unsigned long timeStamp;
  char Message[80];
} Logging[10];
int logcount = -1;

struct DeviceStruct
{
  byte Number;
  char Name[26];
  byte Type;
} Device[DEVICES_MAX];

boolean printToWeb = false;
String printWebString = "";

float UserVar[2 * TASKS_MAX];
unsigned long pulseCounter[TASKS_MAX];
unsigned long pulseTotalCounter[TASKS_MAX];
byte switchstate[TASKS_MAX];

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

unsigned long pulseCounter1 = 0;
byte switch1state = 0;

boolean WebLoggedIn = false;
int WebLoggedInTimer = 300;

void setup()
{
  EEPROM.begin(1024);
  
  LoadSettings();

  // if different version, eeprom settings structure has changed. Full Reset needed
  // on a fresh ESP module eeprom values are set to 255. Version results into -1 (signed int)
  if (Settings.Version != VERSION || Settings.PID != ESP_PROJECT_PID)
  {
    Serial.begin(9600);                                                           // Initialiseer de seriele poort
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

  Device_Init();
  hardwareInit();

  WifiAPconfig();
  WifiConnect();

  WebServerInit();

  // setup UDP
  if (Settings.UDPPort != 0)
    portRX.begin(Settings.UDPPort);

  // Setup timers
  timer = millis() + 30000; // startup delay 30 sec
  timer100ms = millis() + 100; // timer for periodic actions 10 x per/sec
  timer1s = millis() + 1000; // timer for periodic actions once per/sec
  timerwd = millis() + 30000; // timer for watchdog once per 30 sec

  // Setup LCD display
  lcd.init();                      // initialize the lcd
  lcd.backlight();
  lcd.print("ESP Easy");

  // Setup MQTT Client
  if (Settings.Protocol == 2)
    MQTTConnect();

  syslog((char*)"Boot");
  sendSysInfoUDP(3);
  Serial.println(F("INIT : Boot OK"));
  addLog(LOG_LEVEL_INFO,(char*)"Boot");
}

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
  }

  // Perform regular checks, 10 times/sec
  if (millis() > timer100ms)
  {
    timer100ms = millis() + 100;
    inputCheck();
  }

  // Perform regular checks, 1 time/sec
  if (millis() > timer1s)
  {
    timer1s = millis() + 1000;
    WifiCheck();

    if (Settings.Password[0] != 0)
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
  }

  if (connectionFailures > REBOOT_ON_MAX_CONNECTION_FAILURES)
  {
    Serial.println(F("Too many connection failures"));
    delayedReboot(60);
  }

  backgroundtasks();   

}

void inputCheck()
{
  // Handle switches
  for (byte x=0; x < TASKS_MAX; x++)
  {
    if(Settings.TaskDeviceNumber[x] == DEVICE_SWITCH && Settings.TaskDeviceID[x] != 0)
    {
      byte state = digitalRead(Settings.TaskDevicePin1[x]);
      if (state != switchstate[x])
      {
        switchstate[x] = state;
        UserVar[(x*2+1) - 1] = state;
        sendData(10, Settings.TaskDeviceID[x], x*2+1);
        delay(100);
      }
    }
  }

  // Handle rfid
  for (byte x=0; x < TASKS_MAX; x++)
  {
    if(Settings.TaskDeviceNumber[x] == DEVICE_RFID && Settings.TaskDeviceID[x] != 0)
    {
    unsigned long rfid_id = rfid();
    if (rfid_id > 0)
      {
        Serial.print("RFID : Tag : ");
        Serial.println(rfid_id);
        UserVar[(x*2+1) - 1] = rfid_id;
        sendData(1, Settings.TaskDeviceID[x], x*2+1);
      }
    }
  }

}

void SensorSend()
{
  for (byte x=0; x < TASKS_MAX; x++)
  {
    if (Settings.TaskDeviceID[x] != 0)
    {
      switch(Settings.TaskDeviceNumber[x])
      {
        case DEVICE_DS18B20:
          dallas(Settings.TaskDevicePin1[x], x*2+1);
          sendData(1, Settings.TaskDeviceID[x], x*2+1);
          break;
          
        case DEVICE_DHT11:
          dht(11, Settings.TaskDevicePin1[x], x*2+1);
          if (!isnan(UserVar[(x*2+1) - 1]) && (UserVar[(x*2+2) - 1] > 0))
            sendData(2,Settings.TaskDeviceID[x], x*2+1);
          break;

        case DEVICE_DHT22:
          dht(22, Settings.TaskDevicePin1[x], 2);
          if (!isnan(UserVar[(x*2+1) - 1]) && (UserVar[(x*2+2) - 1] > 0))
            sendData(2, Settings.TaskDeviceID[x], x*2+1);
          break;

        case DEVICE_BMP085:
          bmp085(x*2+1);
          if ((UserVar[(x*2+2) - 1] >= 300) && (UserVar[(x*2+2) - 1] <= 1100))
            sendData(3, Settings.TaskDeviceID[x], x*2+1);
          break;

        case DEVICE_BH1750:
          lux(x*2+1);       // read BH1750 LUX sensor and store to var 6
          sendData(1, Settings.TaskDeviceID[x], x*2+1);
          break;

        case DEVICE_ANALOG:
          analog(x*2+1);       // read ADC and store to var 7
          sendData(1, Settings.TaskDeviceID[x], x*2+1);
          break;

        case DEVICE_PULSE:
          UserVar[(x*2+1) - 1] = pulseCounter[x];
          sendData(1, Settings.TaskDeviceID[x], x*2+1);
          pulseCounter[x] = 0;
          break;
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

