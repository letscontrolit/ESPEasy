/****************************************************************************************************************************\
 * Arduino project "ESP Easy" © Copyright www.esp8266.nu
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * You received a copy of the GNU General Public License along with this program in file 'COPYING.TXT'.
 *
 * Additional information about licensing can be found at : http://www.gnu.org/licenses
\*************************************************************************************************************************/

// Simple Arduino sketch for ESP module, supporting:
//   Dallas OneWire DS18b20 temperature sensor
//   DHT11/22 humidity sensor
//   BH1750 I2C Lux sensor
//   BMP085 I2C Barometric Pressure sensor
//   RFID Wiegand-26 reader
//   MCP23017 I2C IO Expander
//   Analog input (ESP-7/12 only)
//   LCD I2C display 4x20 chars
//   Pulse counter
//   Simple switch input

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
                                                // 1 = Domoticz HTTP
                                                // 2 = Domoticz MQTT
#define UNIT                1

// ********************************************************************************
//   DO NOT CHANGE ANYTHING BELOW THIS LINE
// ********************************************************************************

// variables used
// 1 Dallas
// 2-3 DHT
// 4-5 BMP085
// 6 Lux
// 7 Analog
// 8 RFID
// 9 Pulsecounter
// 10 DomoticzSend Serial in
// 11 Switch

// sensor types
//  1 = single value, general purpose (Dallas, LUX, counters, etc)
//  2 = temp + hum (DHT)
//  3 = temp + hum + baro (BMP085)
// 10 = switch

#define ESP_PROJECT_PID   2015050101L
#define ESP_EASY
#define VERSION           1
#define BUILD             9

#define REBOOT_ON_MAX_CONNECTION_FAILURES  30

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
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

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
  unsigned int  Dallas;
  unsigned int  DHT;
  byte          DHTType;
  unsigned int  BMP;
  unsigned int  LUX;
  unsigned int  RFID;
  unsigned int  Analog;
  unsigned int  Pulse1;
  byte          BoardType;
  int8_t        Pin_i2c_sda;
  int8_t        Pin_i2c_scl;
  int8_t        Pin_wired_in_1;
  int8_t        Pin_wired_in_2;
  int8_t        Pin_wired_out_1;
  int8_t        Pin_wired_out_2;
  byte          Syslog_IP[4];
  unsigned int  UDPPort;
  unsigned int  Switch1;
  byte          Protocol;
  byte          IP[4];
  byte          Gateway[4];
  byte          Subnet[4];
  byte          Debug;
  char          Name[26];
} Settings;

struct LogStruct
{
  unsigned long timeStamp;
  char Message[80];
}Logging[10];
int logcount=-1;

boolean printToWeb = false;
String printWebString = "";

float UserVar[15];
unsigned long timer;
unsigned long timer100ms;
unsigned long timer1s;
unsigned long timerwd;
unsigned int NC_Count = 0;
unsigned int C_Count = 0;
boolean AP_Mode = false;
char ap_ssid[20];
boolean cmd_disconnect = false;
unsigned long connectionFailures;
unsigned long wdcounter=0;

unsigned long pulseCounter1=0;
byte switch1state=0;

void setup()
{
  Serial.begin(9600);
  Serial.print(F("\nINIT : Booting Build nr:"));
  Serial.println(BUILD);

  EEPROM.begin(1024);
   
  LoadSettings();
  // if different version, eeprom settings structure has changed. Full Reset needed
  // on a fresh ESP module eeprom values are set to 255. Version results into -1 (signed int)
  Serial.println(Settings.PID);
  Serial.println(Settings.Version);
  if (Settings.Version != VERSION || Settings.PID != ESP_PROJECT_PID)
  {
    Serial.println(F("INIT : Incorrect PID or version!"));
    delay(10000);
    ResetFactory();
  }

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
  MQTTConnect();

  syslog((char*)"Boot");
  sendSysInfoUDP(3);
  Serial.println(F("INIT : Boot OK"));
  addLog((char*)"Boot");
}

void loop()
{
  WebServer.handleClient();

  MQTTclient.loop();
  
  if (Serial.available())
    serial();
  
  checkUDP();  
   
  if (cmd_disconnect == true)
  {
    cmd_disconnect = false;
    WifiDisconnect();
  }

  // Watchdog trigger
  if (millis() > timerwd)
  {
    wdcounter++;
    timerwd = millis() + 30000;
    char str[40];
    str[0]=0;
    Serial.print("WD   : ");
    sprintf(str,"Uptime %u ConnectFailures %u FreeMem %u", wdcounter/2, connectionFailures, FreeMem());
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
  }

  // Check sensors and send data to controller when sensor timer has elapsed
  if (millis() > timer)
  {
    timer = millis() + Settings.Delay * 1000;
    SensorSend();
  }

  if(connectionFailures > REBOOT_ON_MAX_CONNECTION_FAILURES)
  {
    Serial.println(F("Too many connection failures"));
    delayedReboot(60);
  }
  
  if (Settings.Switch1 != 0)
  {
    byte state1 = digitalRead(Settings.Pin_wired_in_1);
    if (state1 != switch1state)
      {
        switch1state = state1;
        UserVar[11 - 1] = state1;
        sendData(10, Settings.Switch1, 11);
        delay(100);
      }
  }
  delay(10);
}

void inputCheck()
{
  if (Settings.RFID > 0)
  {
    unsigned long rfid_id = rfid();
    if (rfid_id > 0)
    {
      Serial.print("RFID : Tag : ");
      Serial.println(rfid_id);
      UserVar[8 - 1] = rfid_id;
      sendData(1, Settings.RFID, 8);
    }
  }
}

void SensorSend()
{
  if (Settings.Dallas > 0)
  {
    dallas(1, 1); // read ds18b20 on wiredout 1, store to var 1
    sendData(1, Settings.Dallas, 1);
  }

  if (Settings.DHT > 0)
  {
    dht(Settings.DHTType, 2, 2); // read dht on wiredout 2, store to var 2 (and 3)
    if (!isnan(UserVar[2-1]) && (UserVar[3-1] > 0))
      sendData(2, Settings.DHT, 2);
  }

  if (Settings.BMP > 0)
  {
    bmp085(4);  // read bmp085 (i2c) and store to vars 4 and 5
    sendData(3, Settings.BMP, 4);
  }

  if (Settings.LUX > 0)
  {
    lux(6);       // read BH1750 LUX sensor and store to var 6
    sendData(1, Settings.LUX, 6);
  }

  if (Settings.Analog > 0)
  {
    analog(7);       // read ADC and store to var 7
    sendData(1, Settings.Analog, 7);
  }

  if (Settings.Pulse1 > 0)
  {
    float value=0;
    if (Domoticz_getData(Settings.Pulse1, &value))
    {
      Serial.print(F("Current Value:"));
      Serial.println(value);
      Serial.print(F("Delta Value:"));
      Serial.println(pulseCounter1);
      value=(value + pulseCounter1)*100;
      pulseCounter1=0;
      Serial.print(F("New Value:"));
      Serial.println(value);
      UserVar[9 - 1] = value;  // store pulsecount to var 9
      sendData(1, Settings.Pulse1, 9);
    }
  }

}


