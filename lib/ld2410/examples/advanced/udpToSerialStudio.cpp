/*
 * Example sketch for reporting on readings from the LD2410 using 
 * whatever settings are currently configured.
 * 
 * The sketch assumes an ESP32 board with the LD2410 connected as Serial2 
 * on pins 16 & 17, the serial configuration for other boards may vary.
 * 
 * Program broadcasts [reporting data] on port 8090, and listens on 
 * port 8091 for commands or configuration requests, 
 * responses are sent to callers ip/port as discovered.
 * 
 * Where [reporting data], can be CSV-like, JSON with Short or Long attributes as
 * documented in [SerialStudio's page](https://github.com/Serial-Studio/Serial-Studio/wiki/Communication-Protocol)
 * 
 * WIFI_SSID and WIFI_PASS are double-quoted environment variables with related values, 
 * -- example: export PLATFORMIO_BUILD_FLAGS=-DWIFI_PASS='"ssid-password"' -DWIFI_SSID='"ssid-value"'
 * or 
 * -- example: export WIFI_PASS='"ssid-password"'
 * --          export WIFI_SSID='"ssid-value"'
 * 
 * Gates: 
 * - each gate is 0.75m or 30 inches
 *  0 to 9 gates = 6.75m or 22 feet ish
 * 
 * 
 */

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncUDP.h>
#include <ld2410.h>

#define RXD2 16 // 8
#define TXD2 17 // 9
#define MAX_COMMAND_TOKENS 32
#define SNAME "LD2410 Sensor 01"
#define SERIAL_STUDIO 1

#ifdef SERIAL_STUDIO
AsyncUDP udp;
#endif

const char* ssid           = WIFI_SSID;
const char* ssidPassword   = WIFI_PASS;
const uint16_t    sendPort = 8090;
const uint16_t  listenPort = 8091;
IPAddress ipSerialStudio(10,100,1,5);

uint16_t  remotePort = 8090;     // default value, will be overridden on reciept of udp request
IPAddress ipRemote(10,100,1,5);  // default value, will be overridden on reciept of udp request

ld2410 radar;
volatile bool udpFlag = false; // send for callback
uint32_t lastReading = 0;
uint32_t pos         = 0;
uint32_t pos1        = 0;
uint32_t pos2        = 0;

bool sending_enabled = true;
char buffer1[128];
char serialBuffer[256];
String command = "";
String output = "";

/*
 * Available Commands */
String availableCommands() {
    String sCmd = "";
    sCmd += "\nSupported commands:";
    sCmd += "\n\t( 1) help: this text."; 
    sCmd += "\n\t( 2) streamstart: start sending udp data to SerialStudio.";
    sCmd += "\n\t( 3) streamstop: stop sending to SerialStream.";
    sCmd += "\n\t( 4) read: read current values from the sensor";
    sCmd += "\n\t( 5) readconfig: read the configuration from the sensor";
    sCmd += "\n\t( 6) setmaxvalues <motion gate> <stationary gate> <inactivitytimer> (2-8) (0-65535)seconds";
    sCmd += "\n\t( 7) setsensitivity <gate> <motionsensitivity> <stationarysensitivity> (2-8|255) (0-100)";
    sCmd += "\n\t( 8) restart: restart the sensor";
    sCmd += "\n\t( 9) readversion: read firmware version";
    sCmd += "\n\t(10) factoryreset: factory reset the sensor";    
    sCmd += "\n\t(11) deviceinfo: LD2410 device info";    
    sCmd += "\n\t(12) reboot: reboot hosting micro-controller\n";    

  return sCmd;
}

/*
 * Command Processor 
 * - there are two ommands not implemented
 * - requestConfigurationModeBegin()
 * - requestConfigurationModeEnd()
 * Otherwise all commands are available as options
 */
String commandProcessor(String &cmdStr) {
  String sBuf = "\n";
  int iCmd = cmdStr.toInt();
  cmdStr.trim();

  if(cmdStr.equals("help") || iCmd == 1) 
  {
    sBuf += availableCommands();
  }
  else if(cmdStr.equals("streamstart") || iCmd == 2)  
  {
      sending_enabled = true;      
      sBuf += "\nSerialStudio UDP Stream Enabled. \n"; 
  } 
  else if(cmdStr.equals("streamstop") || iCmd == 3) 
  {
      sending_enabled = false;
      sBuf += "\nSerialStudio UDP Stream Disabled.\n";
  }
  else if(cmdStr.equals("read") || iCmd == 4) 
  {
    sBuf += "\nReading from sensor: ";
    if(radar.isConnected())
    {
      sBuf += "OK\n";
      if(radar.presenceDetected())
      {
        if(radar.stationaryTargetDetected())
        {
          sBuf += "Stationary target: ";
          sBuf += radar.stationaryTargetDistance();
          sBuf += " cm energy: ";
          sBuf += radar.stationaryTargetEnergy();
          sBuf += " dBZ\n";
        }
        if(radar.movingTargetDetected())
        {
          sBuf += "Moving target: ";
          sBuf += radar.movingTargetDistance();
          sBuf += " cm energy: ";
          sBuf += radar.movingTargetEnergy();
          sBuf += " dBZ\n";
        }
        if(!radar.stationaryTargetDetected() && !radar.movingTargetDetected()) {
          sBuf += "No Detection, in Idle Hold window of: ";
          sBuf += radar.cfgSensorIdleTimeInSeconds();
          sBuf += " seconds\n";
        }
      }
      else
      {
        sBuf += "\nnothing detected\n";
      }
    }
    else
    {
      sBuf += "failed to read\n";
    }
  }
  else if(cmdStr.equals("readconfig") || iCmd == 5) 
  {
    sBuf += "\nReading configuration from sensor: ";
    if(radar.requestCurrentConfiguration())
    {
      sBuf += "OK\n";
      sBuf += "Maximum gate ID: ";
      sBuf += radar.cfgMaxGate();
      sBuf += "\n";
      sBuf += "Maximum gate for moving targets: ";
      sBuf += radar.cfgMaxMovingGate();
      sBuf += "\n";
      sBuf += "Maximum gate for stationary targets: ";
      sBuf += radar.cfgMaxStationaryGate();
      sBuf += "\n";
      sBuf += "Idle time for targets: "; 
      sBuf += radar.cfgSensorIdleTimeInSeconds() ;
      sBuf += "s\n";
      sBuf += "Gate sensitivity\n";
      
      for(uint8_t gate = 0; gate < LD2410_MAX_GATES; gate++)
      {
        sBuf += "Gate ";
        sBuf += gate;
        sBuf += " moving targets: ";
        sBuf += radar.cfgMovingGateSensitivity(gate);
        sBuf += " dBZ stationary targets: ";
        sBuf += radar.cfgStationaryGateSensitivity(gate);
        sBuf += " dBZ\n";
      }
    }
    else
    {
      sBuf += "Failed\n";
    }
  }
  else if(cmdStr.startsWith("setmaxvalues") || iCmd == 6) 
  {
    uint8_t firstSpace = cmdStr.indexOf(' ');
    uint8_t secondSpace = cmdStr.indexOf(' ',firstSpace + 1);
    uint8_t thirdSpace = cmdStr.indexOf(' ',secondSpace + 1);

    uint8_t newMovingMaxDistance = (cmdStr.substring(firstSpace,secondSpace)).toInt();
    uint8_t newStationaryMaxDistance = (cmdStr.substring(secondSpace,thirdSpace)).toInt();
    uint16_t inactivityTimer = (cmdStr.substring(thirdSpace,cmdStr.length())).toInt();

    if(newMovingMaxDistance > 0 && newStationaryMaxDistance > 0 && newMovingMaxDistance <= 8 && newStationaryMaxDistance <= 8)
    {
      sBuf += "\nSetting max values to gate ";
      sBuf += newMovingMaxDistance;
      sBuf += " moving targets, gate ";
      sBuf += newStationaryMaxDistance;
      sBuf += " stationary targets, ";
      sBuf += inactivityTimer;
      sBuf += "s inactivity timer: ";
      if(radar.setMaxValues(newMovingMaxDistance, newStationaryMaxDistance, inactivityTimer))
      {
        sBuf += "OK, now restart to apply settings\n";
      }
      else
      {
        sBuf += "failed\n";
      }
    }
    else
    {
      sBuf += "Can't set distances to ";
      sBuf += newMovingMaxDistance;
      sBuf += " moving ";
      sBuf += newStationaryMaxDistance;
      sBuf += " stationary, try again\n";
    }
  }
  else if(cmdStr.startsWith("setsensitivity") || iCmd == 7) 
  {
    uint8_t firstSpace = cmdStr.indexOf(' ');
    uint8_t secondSpace = cmdStr.indexOf(' ',firstSpace + 1);
    uint8_t thirdSpace = cmdStr.indexOf(' ',secondSpace + 1);

    uint8_t gate = (cmdStr.substring(firstSpace,secondSpace)).toInt();
    uint8_t motionSensitivity = (cmdStr.substring(secondSpace,thirdSpace)).toInt();
    uint8_t stationarySensitivity = (cmdStr.substring(thirdSpace,cmdStr.length())).toInt();

    // Command method 1 -- limit gate to 0-8 -- set one gate set
    // Command method 2 -- limit gate to 255 -- set all gates to same sensitivity value
    if(motionSensitivity >= 0 && stationarySensitivity >= 0 && motionSensitivity <= 100 && stationarySensitivity <= 100)
    {
      sBuf += "\nSetting gate ";
      sBuf += gate;
      sBuf += " motion sensitivity to ";
      sBuf += motionSensitivity;
      sBuf += " dBZ & stationary sensitivity to ";
      sBuf += stationarySensitivity;
      sBuf += " dBZ: \n";
      if(radar.setGateSensitivityThreshold(gate, motionSensitivity, stationarySensitivity))
      {
        sBuf += "OK, now restart to apply settings\n";
      }
      else
      {
        sBuf += "failed\n";
      }
    }
    else
    {
      sBuf += "Can't set gate ";
      sBuf += gate;
      sBuf += " motion sensitivity to ";
      sBuf += motionSensitivity;
      sBuf += " dBZ & stationary sensitivity to ";
      sBuf += stationarySensitivity;
      sBuf += " dBZ, try again\n";
    }
  }
  else if(cmdStr.equals("restart") || iCmd ==8) 
  {
    if(radar.requestRestart())
    {
      delay(1500);
      if(radar.requestStartEngineeringMode()) {
        sBuf += "\nRestarting sensor: OK\n";
      }      
    }
    else
    {
      sBuf += "\nRestarting sensor: failed\n";
    }
  }
  else if(cmdStr.equals("readversion") || iCmd == 9) 
  {
    sBuf += "\nRequesting firmware version: ";
    if(radar.requestFirmwareVersion())
    {
      sBuf += radar.cmdFirmwareVersion();
    }
    else
    {
      sBuf += "Failed\n";
    }
  }
  else if(cmdStr.equals("factoryreset") || iCmd == 10) 
  {
    sBuf += "\nFactory resetting sensor: ";
    if(radar.requestFactoryReset())
    {
      sBuf += "OK, now restart sensor to take effect\n";
    }
    else
    {
      sBuf += "failed\n";
    }
  }
  else if(cmdStr.equals("deviceinfo") || iCmd == 11) 
  {
      sBuf += "\nLD2410 Device Information: \n";
      sBuf += "Data reporting mode: ";
      sBuf += (radar.isEngineeringMode() ? "Engineering Mode" : "Target Mode");
      sBuf += "\nCommunication protocol version: v";
      sBuf += radar.cmdProtocolVersion();
      sBuf += ".0\nCommunications Buffer Size: ";
      sBuf += radar.cmdCommunicationBufferSize();
      sBuf += " bytes\nDevice firmare version: ";
      sBuf += radar.cmdFirmwareVersion();
      sBuf += "\tEngineering retain data value: ";
      sBuf += radar.engRetainDataValue();
      sBuf += "\n";
  }
  else if(cmdStr.equals("reboot") || iCmd == 12) 
  {
    ESP.restart();
  }
  else
  {
    sBuf += "\nUnknown command: ";
    sBuf += cmdStr;
    sBuf += "\n";
  }

  cmdStr.clear();
  sBuf += "\n choose:> ";

  return sBuf;
}

/*
 * Accepts Serial chars and process chars as a command 
 * when the newline char is received
 */
void commandHandler() {
  if(Serial.available())
  {
    char typedCharacter = Serial.read();
    if(typedCharacter == '\n') {
        Serial.print( commandProcessor(command) );        
    } else {
      Serial.print(typedCharacter);
      if(typedCharacter != '\r') {  // effectively ignore CRs
        command += typedCharacter;
      }
    }
  }
}

#ifdef SERIAL_STUDIO
/*
 * Send data via UDP */
void sendToRequestor(String str, bool requestor = false) {
  if(requestor) {
    udp.connect(ipRemote,remotePort);
  }  else {
    udp.connect(ipSerialStudio,sendPort);
  }
  // Serial.printf("DEBUG: SizeOf(serialBuffer)=%d Length(str)=%d Contents:%s", sizeof(serialBuffer), str.length(), str.c_str());
  udp.print(str);
  return udp.close();
}
#endif

/*
 * CSV like Values for SerialStudio App - see test folder */
//              %1,2,3, 4, 5,6,7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43
/*LD2410 Sensor 01,0,0,62,43,0,0,50,15, 0, 0,50,15, 0, 0,40, 5,40,62,30, 9,40,45,20, 3,30,25,15, 6,30,18,15, 1,20,10,15, 2,20, 8,15, 7,20, 6*/
String buildWithAlarmSerialStudioCSV() {
  pos = snprintf(serialBuffer,sizeof(serialBuffer),"/*%s,%d,%d,%d,%d,%d,%d,",SNAME,radar.stationaryTargetDistance(),radar.detectionDistance(), radar.stationaryTargetEnergy(),radar.movingTargetDistance(), radar.detectionDistance(), radar.movingTargetEnergy());

  for(int x = 0; x < LD2410_MAX_GATES; ++x) {
    pos1 = snprintf(buffer1,sizeof(buffer1),"%d,%d,%d,%d,",  radar.cfgMovingGateSensitivity(x), radar.engMovingDistanceGateEnergy(x), radar.cfgStationaryGateSensitivity(x), radar.engStaticDistanceGateEnergy(x));  
    strcat(serialBuffer, buffer1);
    pos += pos1;
  }
  serialBuffer[--pos] = 0;
  strcat(serialBuffer, "*/\n");

  return String(serialBuffer);
}

void setup(void)
{
  delay(1000);

  // start console path
  Serial.begin(115200);
  delay(250);

  // start path to LD2410
  // radar.debug(Serial);
  Serial2.begin (256000, SERIAL_8N1, RXD2, TXD2); //UART for monitoring the radar rx, tx

#ifdef SERIAL_STUDIO
  // Start WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, ssidPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("WiFi connected with IP: ");
  Serial.println(WiFi.localIP());
  // 10.100.1.186

  if(udp.listen(listenPort)) {
    Serial.print(F("Client Listening on port: "));
    Serial.println(listenPort);
    udp.onPacket([](AsyncUDPPacket packet) {
      Serial.print("UDP Packet Type: ");
      Serial.print(packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast" : "Unicast");
      Serial.print(", From: ");
      Serial.print(packet.remoteIP());
      Serial.print(":");
      Serial.print(packet.remotePort());
      Serial.print(", To: ");
      Serial.print(packet.localIP());
      Serial.print(":");
      Serial.print(packet.localPort());
      Serial.print(", Length: ");
      Serial.print(packet.length()); 
      Serial.print(", Data: ");
      Serial.write(packet.data(), packet.length());
      Serial.println();

      // save path for response when using udp
      ipRemote = packet.remoteIP();
      remotePort = packet.remotePort();

      // Parse Commands -- executing inside a callback can be problematic
      // Using udpFlag to have loop handle it
      command = (const char*)packet.data(); 
      udpFlag=true;
    });
  }

  Serial.println(F("Client Initialized..."));
#endif

  // Start LD2410 Sensor
  if(radar.begin(Serial2))
  {
    Serial.println(F("Sensor Initialized..."));
    delay(500);
    radar.requestStartEngineeringMode();
  }
  else
  {
    Serial.println(F(" Sensor was not connected"));
  }

  Serial.println(F("setup() Complete..."));
  Serial.println( availableCommands() );
  Serial.print("\n choose> ");
}

void loop()
{
  radar.ld2410_loop();

  if(sending_enabled) {  
    if(radar.isConnected() && millis() - lastReading > 1000)  //Report every 1000ms
    {
      lastReading = millis();   
      #ifdef SERIAL_STUDIO
        sendToRequestor( buildWithAlarmSerialStudioCSV(), false );     
      #else
        if(Serial.available()) {
          Serial.print(  buildWithAlarmSerialStudioCSV() );
        }
      #endif
    }
  }
  #ifdef SERIAL_STUDIO
    if(udpFlag && (command.length() > 1)) { // handle cb request
      sendToRequestor(commandProcessor(command), true);
      udpFlag=false;
      command.clear();
    }
  #endif
  commandHandler();
}
