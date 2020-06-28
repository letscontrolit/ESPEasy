#ifdef USES_P044
//#################################### Plugin 044: P1WifiGateway ########################################
//
//  based on P020 Ser2Net, extended by Ronald Leenes romix/-at-/macuser.nl
//
//  designed for combo
//    Wemos D1 mini (see http://wemos.cc) and
//    P1 wifi gateway shield (see http://www.esp8266thingies.nl for print design and kits)
//    See also http://domoticx.com/p1-poort-slimme-meter-hardware/
//#######################################################################################################

#include "_Plugin_Helper.h"

#define PLUGIN_044
#define PLUGIN_ID_044         44
#define PLUGIN_NAME_044       "Communication - P1 Wifi Gateway"

#ifndef PLUGIN_044_DEBUG
  #define PLUGIN_044_DEBUG                 false // extra logging in serial out
#endif

#define P044_STATUS_LED                    12
#define P044_CHECKSUM_LENGTH               4
#define P044_DATAGRAM_START_CHAR           '/'
#define P044_DATAGRAM_END_CHAR             '!'
#define P044_DATAGRAM_MAX_SIZE             1024

#define P044_WIFI_SERVER_PORT     ExtraTaskSettings.TaskDevicePluginConfigLong[0]
#define P044_BAUDRATE             ExtraTaskSettings.TaskDevicePluginConfigLong[1]
#define P044_RX_WAIT              PCONFIG(0)
#define P044_SERIAL_CONFIG        PCONFIG(1)
#define P044_RESET_TARGET_PIN     CONFIG_PIN1
 

struct P044_Task : public PluginTaskData_base {

  enum class ParserState : byte {
    WAITING,
    READING,
    CHECKSUM
  };

  P044_Task() {
    clearBuffer();
  }

  ~P044_Task() {
    stopServer();
  }

  inline static bool serverActive(WiFiServer *server) {
#if defined(ESP8266)
    return nullptr != server && server->status() != CLOSED;
#elif defined(ESP32)
    return nullptr != server && *server;
#endif
  }

  void startServer(uint16_t portnumber) {
    if (gatewayPort == portnumber && serverActive(P1GatewayServer)) {
      // server is already listening on this port
      return;
    }
    stopServer();
    gatewayPort = portnumber;
    P1GatewayServer = new WiFiServer(portnumber);
    if (nullptr != P1GatewayServer && NetworkConnected()) {
      P1GatewayServer->begin();
      if(serverActive(P1GatewayServer)) {
        addLog(LOG_LEVEL_INFO, String(F("P1   : WiFi server started at port ")) + portnumber);
      } else {
        addLog(LOG_LEVEL_ERROR, String(F("P1   : WiFi server start failed at port ")) +
            portnumber + String(F(", retrying...")));
      }
    }
  }

  void checkServer() {
    if (nullptr != P1GatewayServer && !serverActive(P1GatewayServer) && NetworkConnected()) {
      P1GatewayServer->close();
      P1GatewayServer->begin();
      if(serverActive(P1GatewayServer)) {
        addLog(LOG_LEVEL_INFO, F("P1   : WiFi server started"));
      }
    }
  }

  void stopServer() {
    if (nullptr != P1GatewayServer) {
      if (P1GatewayClient) P1GatewayClient.stop();
      clientConnected = false;
      P1GatewayServer->close();
      addLog(LOG_LEVEL_INFO, F("P1   : WiFi server closed"));
      delete P1GatewayServer;
      P1GatewayServer = nullptr;
    }
  }

  bool hasClientConnected() {
    if (nullptr != P1GatewayServer && P1GatewayServer->hasClient())
    {
      if (P1GatewayClient) P1GatewayClient.stop();
      P1GatewayClient = P1GatewayServer->available();
      P1GatewayClient.setTimeout(CONTROLLER_CLIENTTIMEOUT_DFLT);
      addLog(LOG_LEVEL_INFO, F("P1   : Client connected!"));
    }

    if (P1GatewayClient.connected())
    {
      clientConnected = true;
    }
    else
    {
      if (clientConnected) // there was a client connected before...
      {
        clientConnected = false;
        addLog(LOG_LEVEL_INFO, F("P1   : Client disconnected!"));
      }
    }
    return clientConnected;
  }

  void discardClientIn() {
    // flush all data received from the WiFi gateway
    // as a P1 meter does not receive data
    while(P1GatewayClient.available()) {
      P1GatewayClient.read();
    }
  }

  void blinkLED() {
    blinkLEDStartTime = millis();
    digitalWrite(P044_STATUS_LED, 1);
  }

  void checkBlinkLED() {
    if (blinkLEDStartTime > 0 && timePassedSince(blinkLEDStartTime) >= 500) {
      digitalWrite(P044_STATUS_LED, 0);
      blinkLEDStartTime = 0;
    }
  }

  void clearBuffer() {
    serial_buffer = "";
    serial_buffer.reserve(P044_DATAGRAM_MAX_SIZE);
  }

  void addChar(char ch) {
    serial_buffer += ch;
  }

  /*  checkDatagram
      checks whether the P044_CHECKSUM of the data received from P1 matches the P044_CHECKSUM
      attached to the telegram
  */
  bool checkDatagram() const {
    int endChar = serial_buffer.length() - 1;
    if (CRCcheck) {
      endChar -= P044_CHECKSUM_LENGTH;
    }
    if (endChar < 0 || serial_buffer[0] != P044_DATAGRAM_START_CHAR ||
        serial_buffer[endChar] != P044_DATAGRAM_END_CHAR) return false;

    if (!CRCcheck) return true;

    const int checksumStartIndex = endChar + 1;
    if (PLUGIN_044_DEBUG) {
      for (unsigned int cnt = 0; cnt < serial_buffer.length(); ++cnt)
        serialPrint(serial_buffer.substring(cnt, 1));
    }

    // calculate the CRC and check if it equals the hexadecimal one attached to the datagram
    unsigned int crc = CRC16(serial_buffer, checksumStartIndex);
    return (strtoul(serial_buffer.substring(checksumStartIndex).c_str(), NULL, 16) == crc);
  }

  /*
     CRC16
        based on code written by Jan ten Hove
       https://github.com/jantenhove/P1-Meter-ESP8266
  */
  static unsigned int CRC16(const String& buf, int len)
  {
    unsigned int crc = 0;
    for (int pos = 0; pos < len; pos++)
    {
      crc ^= static_cast<const unsigned int>(buf[pos]);    // XOR byte into least sig. byte of crc

      for (int i = 8; i != 0; i--) {    // Loop over each bit
        if ((crc & 0x0001) != 0) {      // If the LSB is set
          crc >>= 1;                    // Shift right and XOR 0xA001
          crc ^= 0xA001;
        }
        else                            // Else LSB is not set
          crc >>= 1;                    // Just shift right
      }
    }

    return crc;
  }

  /*
     validP1char
         Checks if the character is valid as part of the P1 datagram contents and/or checksum.
         Returns false on a datagram start ('/'), end ('!') or invalid character
  */
  static bool validP1char(char ch) {
    if ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
    {
      return true;
    }
    switch (ch) {
      case '.':
      case ' ':
      case '\\':  // Single backslash, but escaped in C++
      case '\r':
      case '\n':
      case '(':
      case ')':
      case '-':
      case '*':
      case ':':
        return true;
    }
    return false;
  }

  void serialBegin(int16_t rxPin, int16_t txPin,
                   unsigned long baud, byte config) {
    serialEnd();
    if (rxPin >= 0) {
      P1EasySerial = new ESPeasySerial(rxPin, txPin);
      if (nullptr != P1EasySerial) {
#if defined(ESP8266)
        P1EasySerial->begin(baud, (SerialConfig)config);
#elif defined(ESP32)
        P1EasySerial->begin(baud, config);
#endif
        addLog(LOG_LEVEL_DEBUG, F("P1   : Serial opened"));
      }
    }
    state = ParserState::WAITING;
  }
  
  void serialEnd() {
    if (nullptr != P1EasySerial) {
      delete P1EasySerial;
      P1EasySerial = nullptr;
      addLog(LOG_LEVEL_DEBUG, F("P1   : Serial closed"));
    }
  }

  void handleSerialIn(struct EventStruct *event) {
    if (nullptr == P1EasySerial) return;
    int RXWait = P044_RX_WAIT;
    bool done = false;
    int timeOut = RXWait;
    do {
      if (P1EasySerial->available()) {
        digitalWrite(P044_STATUS_LED, 1);
        done = handleChar(P1EasySerial->read());
        digitalWrite(P044_STATUS_LED, 0);
        if (done) break;
        timeOut = RXWait; // if serial received, reset timeout counter
      } else {
        if (timeOut <= 0) break;
        delay(1);
        --timeOut;
      }
    } while (true);

    if (done) {
      P1GatewayClient.print(serial_buffer);
      P1GatewayClient.flush();

      addLog(LOG_LEVEL_DEBUG, F("P1   : data send!"));
      blinkLED();

      if (Settings.UseRules)
      {
        LoadTaskSettings(event->TaskIndex);
        String eventString = getTaskDeviceName(event->TaskIndex);
        eventString += F("#Data");
        eventQueue.add(eventString);
      }
    } // done
  }

  bool handleChar(char ch) {
    if (serial_buffer.length() >= P044_DATAGRAM_MAX_SIZE - 2) { // room for cr/lf
      addLog(LOG_LEVEL_DEBUG, F("P1   : Error: Buffer overflow, discarded input."));
      state = ParserState::WAITING;    // reset
    }
    
    bool done = false;
    bool invalid = false;
    switch (state) {
      case ParserState::WAITING:
        if (ch == P044_DATAGRAM_START_CHAR)  {
          clearBuffer();
          addChar(ch);
          state = ParserState::READING;
        } // else ignore data
        break;
      case ParserState::READING:
        if (validP1char(ch)) {
          addChar(ch);
        } else if (ch == P044_DATAGRAM_END_CHAR) {
          addChar(ch);
          if (CRCcheck) {
            checkI = 0;
            state = ParserState::CHECKSUM;
          } else {
            done = true;
          }
        } else if (ch == P044_DATAGRAM_START_CHAR) {
          addLog(LOG_LEVEL_DEBUG, F("P1   : Error: Start detected, discarded input."));
          state = ParserState::WAITING;    // reset
          return handleChar(ch);
        } else {
          invalid = true;
        }
        break;
      case ParserState::CHECKSUM:
        if (validP1char(ch)) {
          addChar(ch);
          ++checkI;
          if (checkI == P044_CHECKSUM_LENGTH) {
            done = true;
          }
        } else {
          invalid = true;
        }
        break;
    } // switch
    
    if (invalid) {
      // input is not a datagram char
      addLog(LOG_LEVEL_DEBUG, F("P1   : Error: DATA corrupt, discarded input."));
      if (PLUGIN_044_DEBUG) {
        serialPrint(F("faulty char>"));
        serialPrint(String(ch));
        serialPrintln("<");
      }
      state = ParserState::WAITING;    // reset
    }

    if (done) {
      done = checkDatagram();
      if (done) {
        // add the cr/lf pair to the datagram ahead of reading both
        // from serial as the datagram has already been validated
        addChar('\r');
        addChar('\n');
      } else if (CRCcheck) {
        addLog(LOG_LEVEL_DEBUG, F("P1   : Error: Invalid CRC, dropped data"));
      } else {
        addLog(LOG_LEVEL_DEBUG, F("P1   : Error: Invalid datagram, dropped data"));
      }
      state = ParserState::WAITING;    // prepare for next one
    }

    return done;
  }
  
  void discardSerialIn() {
    if (nullptr != P1EasySerial) {
      while (P1EasySerial->available()) {
        P1EasySerial->read();
      }
    }
    state = ParserState::WAITING;
	}

  bool isInit() const {
  	return nullptr != P1GatewayServer && nullptr != P1EasySerial;
  }

  inline static P044_Task *init(taskIndex_t taskIndex) {
    initPluginTaskData(taskIndex, new P044_Task());
    return static_cast<P044_Task *>(getPluginTaskData(taskIndex));
  }

  inline static P044_Task *get(taskIndex_t taskIndex) {
    P044_Task * task = static_cast<P044_Task *>(getPluginTaskData(taskIndex));
    return (nullptr != task && task->isInit()) ? task : nullptr;
  }

  WiFiServer *P1GatewayServer = nullptr;
  uint16_t gatewayPort = 0;
  WiFiClient P1GatewayClient;
  bool clientConnected = false;
  String serial_buffer;
  ParserState state = ParserState::WAITING;
  int checkI = 0;
  boolean CRCcheck = false;
  ESPeasySerial *P1EasySerial = nullptr;
  unsigned long blinkLEDStartTime = 0;
};

boolean Plugin_044(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_044;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].Custom = true;
        Device[deviceCount].TimerOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_044);
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        LoadTaskSettings(event->TaskIndex);
      	addFormNumericBox(F("TCP Port"), F("p044_port"), P044_WIFI_SERVER_PORT, 0);
      	addFormNumericBox(F("Baud Rate"), F("p044_baud"), P044_BAUDRATE, 0);

        byte serialConfChoice = serialHelper_convertOldSerialConfig(P044_SERIAL_CONFIG);
        serialHelper_serialconfig_webformLoad(event, serialConfChoice);

        // FIXME TD-er: Why isn't this using the normal pin selection functions?
      	addFormPinSelect(F("Reset target after boot"), F("taskdevicepin1"), P044_RESET_TARGET_PIN);

      	addFormNumericBox(F("RX Receive Timeout (mSec)"), F("p044_rxwait"), P044_RX_WAIT, 0);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        LoadTaskSettings(event->TaskIndex);
        P044_WIFI_SERVER_PORT = getFormItemInt(F("p044_port"));
        P044_BAUDRATE = getFormItemInt(F("p044_baud"));
        P044_RX_WAIT = getFormItemInt(F("p044_rxwait"));
        P044_SERIAL_CONFIG = serialHelper_serialconfig_webformSave();

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        pinMode(P044_STATUS_LED, OUTPUT);
        digitalWrite(P044_STATUS_LED, 0);

        LoadTaskSettings(event->TaskIndex);
        if ((P044_WIFI_SERVER_PORT == 0) || (P044_BAUDRATE == 0)) {
          clearPluginTaskData(event->TaskIndex);
          break;
        }

        // try to reuse to keep webserver running
        P044_Task *task = P044_Task::get(event->TaskIndex);
        if (nullptr == task) {
          task = P044_Task::init(event->TaskIndex);
        }
        if (nullptr == task) {
          break;
        }

        int rxPin;
        int txPin;
        ESPeasySerialType::getSerialTypePins(ESPeasySerialType::serial0, rxPin, txPin);
        byte serialconfig = serialHelper_convertOldSerialConfig(P044_SERIAL_CONFIG);
        task->serialBegin(rxPin, txPin, P044_BAUDRATE, serialconfig);
        task->startServer(P044_WIFI_SERVER_PORT);

        if (!task->isInit()) {
          clearPluginTaskData(event->TaskIndex);
          break;
        }

        if (P044_RESET_TARGET_PIN != -1) {
          pinMode(P044_RESET_TARGET_PIN, OUTPUT);
          digitalWrite(P044_RESET_TARGET_PIN, LOW);
          delay(500);
          digitalWrite(P044_RESET_TARGET_PIN, HIGH);
          pinMode(P044_RESET_TARGET_PIN, INPUT_PULLUP);
        }

        task->blinkLED();
        if (P044_BAUDRATE == 115200) {
          addLog(LOG_LEVEL_DEBUG, F("P1   : DSMR version 4 meter, CRC on"));
          task->CRCcheck = true;
        } else {
          addLog(LOG_LEVEL_DEBUG, F("P1   : DSMR version 4 meter, CRC off"));
          task->CRCcheck = false;
        }

        success = true;
        break;
      }

    case PLUGIN_EXIT:
      {
        clearPluginTaskData(event->TaskIndex);
        success = true;
        break;
      }

    case PLUGIN_ONCE_A_SECOND:
      {
        P044_Task *task = P044_Task::get(event->TaskIndex);
        if (nullptr == task) {
          break;
        }
        task->checkServer();
        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        P044_Task *task = P044_Task::get(event->TaskIndex);
        if (nullptr == task) {
          break;
        }
        if (task->hasClientConnected()) {
          task->discardClientIn();
        }
        task->checkBlinkLED();
        success = true;
        break;
      }

    case PLUGIN_SERIAL_IN:
      {
        P044_Task *task = P044_Task::get(event->TaskIndex);
        if (nullptr == task) {
          break;
        }
        if (task->hasClientConnected()) {
          task->handleSerialIn(event);
        } else {
          task->discardSerialIn();
        }
        success = true;
        break;
      }

  }
  return success;
}
#endif // USES_P044
