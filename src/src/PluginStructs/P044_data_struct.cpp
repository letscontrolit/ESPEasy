#include "../PluginStructs/P044_data_struct.h"

#ifdef USES_P044

#include "../ESPEasyCore/Serial.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"

#include "../Globals/EventQueue.h"

#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Misc.h"

#define P044_RX_WAIT              PCONFIG(0)


P044_Task::P044_Task() {
  clearBuffer();
}

P044_Task::~P044_Task() {
  stopServer();
}

bool P044_Task::serverActive(WiFiServer *server) {
#if defined(ESP8266)
  return nullptr != server && server->status() != CLOSED;
#elif defined(ESP32)
  return nullptr != server && *server;
#endif // if defined(ESP8266)
}

void P044_Task::startServer(uint16_t portnumber) {
  if ((gatewayPort == portnumber) && serverActive(P1GatewayServer)) {
    // server is already listening on this port
    return;
  }
  stopServer();
  gatewayPort     = portnumber;
  P1GatewayServer = new (std::nothrow) WiFiServer(portnumber);

  if ((nullptr != P1GatewayServer) && NetworkConnected()) {
    P1GatewayServer->begin();

    if (serverActive(P1GatewayServer)) {
      addLog(LOG_LEVEL_INFO, String(F("P1   : WiFi server started at port ")) + portnumber);
    } else {
      addLog(LOG_LEVEL_ERROR, String(F("P1   : WiFi server start failed at port ")) +
             portnumber + String(F(", retrying...")));
    }
  }
}

void P044_Task::checkServer() {
  if ((nullptr != P1GatewayServer) && !serverActive(P1GatewayServer) && NetworkConnected()) {
    P1GatewayServer->close();
    P1GatewayServer->begin();

    if (serverActive(P1GatewayServer)) {
      addLog(LOG_LEVEL_INFO, F("P1   : WiFi server started"));
    }
  }
}

void P044_Task::stopServer() {
  if (nullptr != P1GatewayServer) {
    if (P1GatewayClient) { P1GatewayClient.stop(); }
    clientConnected = false;
    P1GatewayServer->close();
    addLog(LOG_LEVEL_INFO, F("P1   : WiFi server closed"));
    delete P1GatewayServer;
    P1GatewayServer = nullptr;
  }
}

bool P044_Task::hasClientConnected() {
  if ((nullptr != P1GatewayServer) && P1GatewayServer->hasClient())
  {
    if (P1GatewayClient) { P1GatewayClient.stop(); }
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

void P044_Task::discardClientIn() {
  // flush all data received from the WiFi gateway
  // as a P1 meter does not receive data
  while (P1GatewayClient.available()) {
    P1GatewayClient.read();
  }
}

void P044_Task::blinkLED() {
  blinkLEDStartTime = millis();
  digitalWrite(P044_STATUS_LED, 1);
}

void P044_Task::checkBlinkLED() {
  if ((blinkLEDStartTime > 0) && (timePassedSince(blinkLEDStartTime) >= 500)) {
    digitalWrite(P044_STATUS_LED, 0);
    blinkLEDStartTime = 0;
  }
}

void P044_Task::clearBuffer() {
  if (serial_buffer.length() > maxMessageSize) {
    maxMessageSize = _min(serial_buffer.length(), P044_DATAGRAM_MAX_SIZE);
  }

  serial_buffer = "";
  serial_buffer.reserve(maxMessageSize);
}

void P044_Task::addChar(char ch) {
  serial_buffer += ch;
}

/*  checkDatagram
    checks whether the P044_CHECKSUM of the data received from P1 matches the P044_CHECKSUM
    attached to the telegram
 */
bool P044_Task::checkDatagram() const {
  int endChar = serial_buffer.length() - 1;

  if (CRCcheck) {
    endChar -= P044_CHECKSUM_LENGTH;
  }

  if ((endChar < 0) || (serial_buffer[0] != P044_DATAGRAM_START_CHAR) ||
      (serial_buffer[endChar] != P044_DATAGRAM_END_CHAR)) { return false; }

  if (!CRCcheck) { return true; }

  const int checksumStartIndex = endChar + 1;

  #ifdef PLUGIN_044_DEBUG
    for (unsigned int cnt = 0; cnt < serial_buffer.length(); ++cnt) {
      serialPrint(serial_buffer.substring(cnt, 1));
    }
  #endif

  // calculate the CRC and check if it equals the hexadecimal one attached to the datagram
  unsigned int crc = CRC16(serial_buffer, checksumStartIndex);
  return strtoul(serial_buffer.substring(checksumStartIndex).c_str(), NULL, 16) == crc;
}

/*
   CRC16
      based on code written by Jan ten Hove
     https://github.com/jantenhove/P1-Meter-ESP8266
 */
unsigned int P044_Task::CRC16(const String& buf, int len)
{
  unsigned int crc = 0;

  for (int pos = 0; pos < len; pos++)
  {
    crc ^= static_cast<const unsigned int>(buf[pos]); // XOR byte into least sig. byte of crc

    for (int i = 8; i != 0; i--) {                    // Loop over each bit
      if ((crc & 0x0001) != 0) {                      // If the LSB is set
        crc >>= 1;                                    // Shift right and XOR 0xA001
        crc  ^= 0xA001;
      }
      else {                                          // Else LSB is not set
        crc >>= 1;                                    // Just shift right
      }
    }
  }

  return crc;
}

/*
   validP1char
       Checks if the character is valid as part of the P1 datagram contents and/or checksum.
       Returns false on a datagram start ('/'), end ('!') or invalid character
 */
bool P044_Task::validP1char(char ch) {
  if (((ch >= '0') && (ch <= '9')) || ((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')))
  {
    return true;
  }

  switch (ch) {
    case '.':
    case ' ':
    case '\\': // Single backslash, but escaped in C++
    case '\r':
    case '\n':
    case '(':
    case ')':
    case '-':
    case '*':
    case ':':
    case '_':
      return true;
  }
  return false;
}

void P044_Task::serialBegin(const ESPEasySerialPort port, int16_t rxPin, int16_t txPin,
                            unsigned long baud, byte config) {
  serialEnd();

  if (rxPin >= 0) {
    P1EasySerial = new (std::nothrow) ESPeasySerial(port, rxPin, txPin);

    if (nullptr != P1EasySerial) {
#if defined(ESP8266)
      P1EasySerial->begin(baud, (SerialConfig)config);
#elif defined(ESP32)
      P1EasySerial->begin(baud, config);
#endif // if defined(ESP8266)
      addLog(LOG_LEVEL_DEBUG, F("P1   : Serial opened"));
    }
  }
  state = ParserState::WAITING;
}

void P044_Task::serialEnd() {
  if (nullptr != P1EasySerial) {
    delete P1EasySerial;
    P1EasySerial = nullptr;
    addLog(LOG_LEVEL_DEBUG, F("P1   : Serial closed"));
  }
}

void P044_Task::handleSerialIn(struct EventStruct *event) {
  if (nullptr == P1EasySerial) { return; }
  int  RXWait  = P044_RX_WAIT;
  bool done    = false;
  int  timeOut = RXWait;

  do {
    if (P1EasySerial->available()) {
      digitalWrite(P044_STATUS_LED, 1);
      done = handleChar(P1EasySerial->read());
      digitalWrite(P044_STATUS_LED, 0);

      if (done) { break; }
      timeOut = RXWait; // if serial received, reset timeout counter
    } else {
      if (timeOut <= 0) { break; }
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

bool P044_Task::handleChar(char ch) {
  if (serial_buffer.length() >= P044_DATAGRAM_MAX_SIZE - 2) { // room for cr/lf
    addLog(LOG_LEVEL_DEBUG, F("P1   : Error: Buffer overflow, discarded input."));
    state = ParserState::WAITING;                             // reset
  }

  bool done    = false;
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
          state  = ParserState::CHECKSUM;
        } else {
          done = true;
        }
      } else if (ch == P044_DATAGRAM_START_CHAR) {
        addLog(LOG_LEVEL_DEBUG, F("P1   : Error: Start detected, discarded input."));
        state = ParserState::WAITING; // reset
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

    #ifdef PLUGIN_044_DEBUG
      serialPrint(F("faulty char>"));
      serialPrint(String(ch));
      serialPrintln("<");
    #endif
    state = ParserState::WAITING; // reset
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
    state = ParserState::WAITING; // prepare for next one
  }

  return done;
}

void P044_Task::discardSerialIn() {
  if (nullptr != P1EasySerial) {
    while (P1EasySerial->available()) {
      P1EasySerial->read();
    }
  }
  state = ParserState::WAITING;
}

bool P044_Task::isInit() const {
  return nullptr != P1GatewayServer && nullptr != P1EasySerial;
}

#endif
