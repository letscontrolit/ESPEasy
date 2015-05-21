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

// ********************************************************************************
//   User specific configuration
// ********************************************************************************

// Set default configuration settings if you want (not mandatory)
// You can allways change these during runtime and save to eeprom
// After loading firmware, issue a 'reset' command to load the defaults.

#define DEFAULT_SSID       "ssid"              // Enter your network SSID
#define DEFAULT_KEY        "wpakey"            // Enter your network WPA key
#define DEFAULT_SERVER     "192.168.0.8"       // Enter your Domoticz Server IP address
#define DEFAULT_PORT       8080                // Enter your Domoticz Server port value
#define DEFAULT_DELAY      60                  // Enter your Send delay in seconds
#define DEFAULT_AP_KEY     "configesp"         // Enter network WPA key for AP (config) mode
#define DEFAULT_DALLAS_IDX 0                   // Enter IDX of your virtual temperature device
#define DEFAULT_DHT_IDX    0                   // Enter IDX of your virtual Humidity device
#define DEFAULT_DHT_TYPE   11                  // Enter Type of your virtual Humidity device
#define DEFAULT_BMP_IDX    0                   // Enter IDX of your virtual Barometric Pressure device
#define DEFAULT_LUX_IDX    0                   // Enter IDX of your virtual LUX device
#define DEFAULT_RFID_IDX   0                   // Enter IDX of your virtual RFID device
#define DEFAULT_ANALOG_IDX 0                   // Enter IDX of your virtual Analog device
#define DEFAULT_PULSE1_IDX 0                   // Enter IDX of your virtual Pulse counter
#define UNIT               1

// ********************************************************************************
//   DO NOT CHANGE ANYTHING BELOW THIS LINE
// ********************************************************************************

#define ESP_PROJECT_PID   2015050101L
#define VERSION           1
#define BUILD             5

#define UDP_LISTEN_PORT   65500
#define REBOOT_ON_MAX_CONNECTION_FAILURES  30

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <Wire.h>
#include <stdio.h>

ESP8266WebServer server(80);

// syslog stuff
char packetBuffer[255];
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
} Settings;

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

unsigned long pulseCounter1=0;

void setup()
{
  Serial.begin(19200);
  Serial.print("\nINIT : Booting Build nr:");
  Serial.println(BUILD);

  EEPROM.begin(4096);
   
  LoadSettings();
  // if different version, eeprom settings structure has changed. Full Reset needed
  // on a fresh ESP module eeprom values are set to 255. Version results into -1 (signed int)
  Serial.println(Settings.PID);
  Serial.println(Settings.Version);
  if (Settings.Version != VERSION || Settings.PID != ESP_PROJECT_PID)
  {
    Serial.println("INIT : Incorrect PID or version!");
    delay(10000);
    ResetFactory();
  }

  // configure hardware pins according to eeprom settings.
  if (Settings.Pin_i2c_sda != -1)
    {
      Serial.println("INIT : I2C");
      Wire.begin(Settings.Pin_i2c_sda, Settings.Pin_i2c_scl);
    }

  if (Settings.Pin_wired_in_1 != -1)
    {
      Serial.println("INIT : Input 1");
      pinMode(Settings.Pin_wired_in_1, INPUT_PULLUP);
    }
    
  if (Settings.Pin_wired_in_2 != -1)
    {
      Serial.println("INIT : Input 2");
      pinMode(Settings.Pin_wired_in_2, INPUT_PULLUP);
    }

  if (Settings.Pin_wired_out_1 != -1)
    {
      Serial.println("INIT : Output 1");
      pinMode(Settings.Pin_wired_out_1, OUTPUT);
    }

  if (Settings.Pin_wired_out_2 != -1)
    {
      Serial.println("INIT : Output 2");
      pinMode(Settings.Pin_wired_out_2, OUTPUT);
    }

  // create unique AP SSID
  ap_ssid[0] = 0;
  strcpy(ap_ssid, "ESP_");
  sprintf(ap_ssid, "%s%u", ap_ssid, Settings.Unit);
  // setup ssid for AP Mode when needed
  WiFi.softAP(ap_ssid, Settings.WifiAPKey);
  // We start in STA mode
  WiFi.mode(WIFI_STA);
  
  server.on("/", handle_root);
  server.on("/config", handle_config);
  server.on("/devices", handle_devices);
  server.on("/hardware", handle_hardware);
  server.on("/json.htm", handle_json);
  server.begin();

  // Syslog
  portRX.begin(UDP_LISTEN_PORT);
  
  timer = millis() + 30000; // startup delay 30 sec
  timer100ms = millis() + 100; // timer for periodic actions 10 x per/sec
  timer1s = millis() + 1000; // timer for periodic actions once per/sec
  timerwd = millis() + 30000; // timer for watchdog once per 30 sec

  if (Settings.RFID > 0)
    rfidinit(Settings.Pin_wired_in_1, Settings.Pin_wired_in_2);

  if (Settings.Pulse1 > 0)
    pulseinit(Settings.Pin_wired_in_1);

  // if a SSID is configured, try to connect to Wifi network
  if (strcasecmp(Settings.WifiSSID, "ssid") != 0)
    {
      WifiConnect();
      // check if we got an ip address.
      IPAddress ip = WiFi.localIP();
      if (ip[0]==0) // dhcp issue ?
        {
          Serial.println("No IP!");
          delayedReboot(60);
        }
    }
  else
    {
      // Start Access Point for first config steps...
      AP_Mode = true;
      WifiAPMode(true);
    }
     
  Serial.println("INIT : Boot OK");
  syslog((char*)"Boot");
}

void loop()
{
  yield();
  server.handleClient();

  if (Serial.available())
    serial();
    
  // upd events    
  int packetSize = portRX.parsePacket();
  if (packetSize)
    {
      Serial.print("UDP  : " );
      int len = portRX.read(packetBuffer, 255);
      if (len > 0) packetBuffer[len] = 0;
        {
          Serial.println(packetBuffer);
          ExecuteCommand(packetBuffer);
        }
    }
    
  if (cmd_disconnect == true)
  {
    cmd_disconnect = false;
    WifiDisconnect();
  }

  // Watchdog trigger
  if (millis() > timerwd)
  {
    timerwd = millis() + 30000;
    char str[40];
    str[0]=0;
    sprintf(str,"WD %u CF %u FM %u", millis()/1000, connectionFailures, FreeMem());
    Serial.println(str);
    syslog(str);
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
    Serial.println("Too many connection failures");
    delayedReboot(60);
  }
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
      Domoticz_sendData(1, Settings.RFID, 8);
    }
  }
}

void SensorSend()
{
  if (Settings.Dallas > 0)
  {
    dallas(1, 1); // read ds18b20 on wiredout 1, store to var 1
    Domoticz_sendData(1, Settings.Dallas, 1);
  }

  if (Settings.DHT > 0)
  {
    dht(Settings.DHTType, 2, 2); // readd dht on wiredout 2, store to var 2 (and 3)
    Domoticz_sendData(2, Settings.DHT, 2);
  }

  if (Settings.BMP > 0)
  {
    bmp085(4);  // read bmp085 (i2c) and store to vars 4 and 5
    Domoticz_sendData(3, Settings.BMP, 4);
  }

  if (Settings.LUX > 0)
  {
    lux(6);       // read BH1750 LUX sensor and store to var 6
    Domoticz_sendData(1, Settings.LUX, 6);
  }

  if (Settings.Analog > 0)
  {
    analog(7);       // read ADC and store to var 7
    Domoticz_sendData(1, Settings.Analog, 7);
  }

  if (Settings.Pulse1 > 0)
  {
    float value=0;
    if (Domoticz_getData(Settings.Pulse1, &value))
    {
      Serial.print("Current Value:");
      Serial.println(value);
      Serial.print("Delta Value:");
      Serial.println(pulseCounter1);
      value=(value + pulseCounter1)*100;
      pulseCounter1=0;
      Serial.print("New Value:");
      Serial.println(value);
      UserVar[9 - 1] = value;  // store pulsecount to var 9
      Domoticz_sendData(1, Settings.Pulse1, 9);
    }
  }

}

void syslog(char *message)
{
  if (Settings.Syslog_IP[0] != 0)
    {
      IPAddress broadcastIP(Settings.Syslog_IP[0],Settings.Syslog_IP[1],Settings.Syslog_IP[2],Settings.Syslog_IP[3]);
      portTX.beginPacket(broadcastIP,514);
      char str[80];
      str[0]=0;
      sprintf(str,"<7>ESP Unit: %u : %s",Settings.Unit,message);
      Serial.print("SYSLG: ");
      Serial.println(str);
      portTX.write(str);
      portTX.endPacket();
    }
}

void delayedReboot(int rebootDelay)
  {
    while (rebootDelay !=0 )
    {
      Serial.print("Delayed Reset ");
      Serial.println(rebootDelay);
      rebootDelay--;
      delay(1000);
    }
    ESP.reset();
  }

