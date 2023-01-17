#include "../PluginStructs/P020_data_struct.h"

#ifdef USES_P020

# include "../ESPEasyCore/Serial.h"
# include "../ESPEasyCore/ESPEasyNetwork.h"

# include "../Globals/EventQueue.h"

# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/Misc.h"

P020_Task::P020_Task(struct EventStruct *event) : _taskIndex(event->TaskIndex) {
  clearBuffer();

  if (P020_GET_LED_ENABLED) {
    _ledPin = P020_LED_PIN; // Default pin (12) is already initialized in P020_Task
  }
  _ledEnabled  = P020_GET_LED_ENABLED == 1;
  _ledInverted = P020_GET_LED_INVERTED == 1;
  _space       = static_cast<char>(P020_REPLACE_SPACE);
  _newline     = static_cast<char>(P020_REPLACE_NEWLINE);
}

P020_Task::~P020_Task() {
  if (ser2netServer != nullptr) {
    delete ser2netServer;
    ser2netServer = nullptr;
  }

  if (ser2netSerial != nullptr) {
    delete ser2netSerial;
    ser2netSerial = nullptr;
  }
}

bool P020_Task::serverActive(WiFiServer *server) {
# if defined(ESP8266)
  return nullptr != server && server->status() != CLOSED;
# elif defined(ESP32)
  return nullptr != server && *server;
# endif // if defined(ESP8266)
}

void P020_Task::startServer(uint16_t portnumber) {
  if ((gatewayPort == portnumber) && serverActive(ser2netServer)) {
    // server is already listening on this port
    return;
  }
  stopServer();
  gatewayPort   = portnumber;
  ser2netServer = new (std::nothrow) WiFiServer(portnumber);

  if ((nullptr != ser2netServer) && NetworkConnected()) {
    ser2netServer->begin();

    if (serverActive(ser2netServer)) {
      addLog(LOG_LEVEL_INFO, String(F("Ser2Net  : WiFi server started at port ")) + portnumber);
    } else {
      addLog(LOG_LEVEL_ERROR, String(F("Ser2Net   : WiFi server start failed at port ")) +
             portnumber + String(F(", retrying...")));
    }
  }
}

void P020_Task::checkServer() {
  if ((nullptr != ser2netServer) && !serverActive(ser2netServer) && NetworkConnected()) {
    ser2netServer->close();
    ser2netServer->begin();

    if (serverActive(ser2netServer)) {
      addLog(LOG_LEVEL_INFO, F("Ser2net   : WiFi server started"));
    }
  }
}

void P020_Task::stopServer() {
  if (nullptr != ser2netServer) {
    if (ser2netClient) { ser2netClient.stop(); }
    clientConnected = false;
    ser2netServer->close();
    addLog(LOG_LEVEL_INFO, F("Ser2net   : WiFi server closed"));
    delete ser2netServer;
    ser2netServer = nullptr;
  }
}

bool P020_Task::hasClientConnected() {
  if ((nullptr != ser2netServer) && ser2netServer->hasClient())
  {
    if (ser2netClient) { ser2netClient.stop(); }
    ser2netClient = ser2netServer->available();

    # ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS

    // See: https://github.com/espressif/arduino-esp32/pull/6676
    ser2netClient.setTimeout((CONTROLLER_CLIENTTIMEOUT_DFLT + 500) / 1000); // in seconds!!!!
    Client *pClient = &ser2netClient;
    pClient->setTimeout(CONTROLLER_CLIENTTIMEOUT_DFLT);
    # else // ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS
    ser2netClient.setTimeout(CONTROLLER_CLIENTTIMEOUT_DFLT); // in msec as it should be!
    # endif // ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS

    sendConnectedEvent(true);
    addLog(LOG_LEVEL_INFO, F("Ser2Net   : Client connected!"));
  }

  if (ser2netClient.connected())
  {
    clientConnected = true;
  }
  else
  {
    if (clientConnected) // there was a client connected before...
    {
      clientConnected = false;
      sendConnectedEvent(false);
      addLog(LOG_LEVEL_INFO, F("Ser2net   : Client disconnected!"));
    }
  }
  return clientConnected;
}

void P020_Task::discardClientIn() {
  // flush all data received from the WiFi gateway
  // as a P1 meter does not receive data
  while (ser2netClient.available()) {
    ser2netClient.read();
  }
}

void P020_Task::clearBuffer() {
  serial_buffer    = String();
  _maxDataGramSize = serial_processing == P020_Events::P1WiFiGateway
                    ? P020_P1_DATAGRAM_MAX_SIZE
                    : P020_DATAGRAM_MAX_SIZE;
  serial_buffer.reserve(_maxDataGramSize);
}

void P020_Task::serialBegin(const ESPEasySerialPort port, int16_t rxPin, int16_t txPin, unsigned long baud, uint8_t config) {
  serialEnd();

  if (rxPin >= 0) {
    ser2netSerial = new (std::nothrow) ESPeasySerial(port, rxPin, txPin);

    if (nullptr != ser2netSerial) {
      # if defined(ESP8266)
      ser2netSerial->begin(baud, (SerialConfig)config);
      # elif defined(ESP32)
      ser2netSerial->begin(baud, config);
      # endif // if defined(ESP8266)
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG, F("Ser2net   : Serial opened"));
      # endif // ifndef BUILD_NO_DEBUG
    }
  }
}

void P020_Task::serialEnd() {
  if (nullptr != ser2netSerial) {
    delete ser2netSerial;
    clearBuffer();
    ser2netSerial = nullptr;
    # ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_DEBUG, F("Ser2net   : Serial closed"));
    # endif // ifndef BUILD_NO_DEBUG
  }
}

void P020_Task::handleClientIn(struct EventStruct *event) {
  size_t  count      = ser2netClient.available();
  size_t  bytes_read = 0;
  uint8_t net_buf[_maxDataGramSize];

  if (count > 0) {
    if (count > _maxDataGramSize) { count = _maxDataGramSize; }
    bytes_read = ser2netClient.read(net_buf, count);
    ser2netSerial->write(net_buf, bytes_read);
    ser2netSerial->flush();             // Waits for the transmission of outgoing serial data to

    while (ser2netClient.available()) { // flush overflow data if available
      ser2netClient.read();
    }
  }
}

void P020_Task::handleSerialIn(struct EventStruct *event) {
  if (nullptr == ser2netSerial) { return; }
  int  RXWait  = P020_RX_WAIT;
  int  timeOut = RXWait;
  bool done    = false;

  do {
    if (ser2netSerial->available()) {
      if ((serial_processing != P020_Events::P1WiFiGateway) // P1 handling without this check
          && (serial_buffer.length() > static_cast<size_t>(P020_RX_BUFFER))) {
        # ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG, F("Ser2Net   : Error: Buffer overflow, discarded input."));
        # endif // ifndef BUILD_NO_DEBUG
        ser2netSerial->read();
      }
      else {
        if (_ledEnabled) {
          digitalWrite(_ledPin, _ledInverted ? 0 : 1);
        }

        if (serial_processing == P020_Events::P1WiFiGateway) {
          done = handleP1Char(static_cast<char>(ser2netSerial->read()));
        } else {
          addChar(static_cast<char>(ser2netSerial->read()));
        }

        if (_ledEnabled) {
          digitalWrite(_ledPin, _ledInverted ? 1 : 0);
        }
      }

      if (done) { break; }
      timeOut = RXWait; // if serial received, reset timeout counter
    } else {
      if (timeOut <= 0) { break; }
      delay(1);
      --timeOut;
    }
  } while (true);

  if (serial_buffer.length() > 0) {
    if (ser2netClient.connected()) { // Only send out if a client is connected
      // FIXME tonhuisman: Disable extra check for now as it reportedly doesn't work as intended
      // if ((serial_processing == P020_Events::P1WiFiGateway) && !serial_buffer.endsWith(F("\r\n"))) {
      //   serial_buffer += F("\r\n");
      // }
      ser2netClient.print(serial_buffer);
    }

    blinkLED();

    rulesEngine(serial_buffer);
    ser2netClient.flush();
    clearBuffer();
    # ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_DEBUG, F("Ser2Net   : data send!"));
    # endif // ifndef BUILD_NO_DEBUG
  } // done
}

void P020_Task::discardSerialIn() {
  if (nullptr != ser2netSerial) {
    while (ser2netSerial->available()) {
      ser2netSerial->read();
    }
  }
}

// We can also use the rules engine for local control!
void P020_Task::rulesEngine(const String& message) {
  if (!Settings.UseRules || message.isEmpty()) { return; }
  int NewLinePos    = 0;
  uint16_t StartPos = 0;

  NewLinePos = handleMultiLine ? message.indexOf('\n', StartPos) : message.length();

  do {
    if (NewLinePos < 0) {
      NewLinePos = message.length();
    }

    String eventString;

    if ((NewLinePos - StartPos) + 10 > 12) {
      eventString.reserve((NewLinePos - StartPos) + 10); // Include the prefix
    }

    // Remove preceeding CR also
    if ((message[NewLinePos] == '\n') && (message[NewLinePos - 1] == '\r')) {
      NewLinePos--;
    }

    switch (serial_processing) {
      case P020_Events::None: { break; }
      case P020_Events::Generic: { // Generic
        if (NewLinePos > StartPos) {
          eventString  = F("!Serial#");
          eventString += message.substring(StartPos, NewLinePos);
        }
        break;
      }
      case P020_Events::RFLink: {        // RFLink
        StartPos += 6;                   // RFLink, strip 20;xx; from incoming message

        if (message.substring(StartPos, NewLinePos)
            .startsWith(F("ESPEASY"))) { // Special treatment for gpio values, strip unneeded parts...
          StartPos   += 8;               // Strip "ESPEASY;"
          eventString = F("RFLink#");
        } else {
          eventString = F("!RFLink#");   // default event as it comes in, literal match needed in rules, using '!'
        }

        if (NewLinePos > StartPos) {
          eventString += message.substring(StartPos, NewLinePos);
        }
        break;
      }
      case P020_Events::P1WiFiGateway: // P1 WiFi Gateway
        eventString  = getTaskDeviceName(_taskIndex);
        eventString += F("#Data");

        if (_P1EventData) {
          eventString += '=';
          eventString += message; // Include entire message, may cause memory overflow!
        }
        break;
    } // switch

    // Skip CR/LF
    StartPos = NewLinePos; // Continue after what was already handled

    while (StartPos < message.length() && (message[StartPos] == '\n' || message[StartPos] == '\r')) {
      StartPos++;
    }

    if (!eventString.isEmpty()) {
      eventQueue.add(eventString);
    }
    NewLinePos = message.indexOf('\n', StartPos);

    if (handleMultiLine && (NewLinePos < 0)) {
      NewLinePos = message.length();
    }
  } while (handleMultiLine && NewLinePos > StartPos);
}

bool P020_Task::isInit() const {
  return nullptr != ser2netServer && nullptr != ser2netSerial;
}

void P020_Task::sendConnectedEvent(bool connected)
{
  if (Settings.UseRules)
  {
    String RuleEvent;
    RuleEvent += getTaskDeviceName(_taskIndex);
    RuleEvent += '#';
    RuleEvent += F("Client");
    RuleEvent += '=';
    RuleEvent += (connected ? 1 : 0);
    eventQueue.addMove(std::move(RuleEvent));
  }
}

void P020_Task::blinkLED() {
  if (_ledEnabled) {
    _blinkLEDStartTime = millis();
    digitalWrite(_ledPin, _ledInverted ? 0 : 1);
  }
}

void P020_Task::checkBlinkLED() {
  if (_ledEnabled && (_blinkLEDStartTime > 0) && (timePassedSince(_blinkLEDStartTime) >= 500)) {
    digitalWrite(_ledPin, _ledInverted ? 1 : 0);
    _blinkLEDStartTime = 0;
  }
}

void P020_Task::addChar(char ch) {
  if ((ch == 0x20) && (_space > 0)) { ch = _space; }

  if (_newline > 0) {
    if (ch == '\n') { ch = _newline; }

    if (ch == '\r') { return; } // Ignore CR if LF is replaced
  }

  serial_buffer += ch;
}

/*  checkDatagram
    checks whether the P020_CHECKSUM of the data received from P1 matches the P020_CHECKSUM
    attached to the telegram
 */
bool P020_Task::checkDatagram() const {
  int endChar = serial_buffer.length() - 1;

  if (_CRCcheck) {
    endChar -= P020_CHECKSUM_LENGTH;
  }

  if ((endChar < 0) || (serial_buffer[0] != P020_DATAGRAM_START_CHAR) ||
      (serial_buffer[endChar] != P020_DATAGRAM_END_CHAR)) {
    return false;
  }

  if (!_CRCcheck) {
    return true;
  }

  const int checksumStartIndex = endChar + 1;

  # if PLUGIN_020_DEBUG

  for (unsigned int cnt = 0; cnt < serial_buffer.length(); ++cnt) {
    serialPrint(serial_buffer.substring(cnt, 1));
  }
  # endif // if PLUGIN_020_DEBUG

  // calculate the CRC and check if it equals the hexadecimal one attached to the datagram
  unsigned int crc = CRC16(serial_buffer, checksumStartIndex);
  return strtoul(serial_buffer.substring(checksumStartIndex).c_str(), nullptr, 16) == crc;
}

/*
   CRC16
      based on code written by Jan ten Hove
     https://github.com/jantenhove/P1-Meter-ESP8266
 */
unsigned int P020_Task::CRC16(const String& buf, int len) {
  unsigned int crc = 0;

  for (int pos = 0; pos < len; pos++) {
    crc ^= static_cast<const unsigned int>(buf[pos]); // XOR byte into least sig. byte of crc

    for (int i = 8; i != 0; i--) {                    // Loop over each bit
      if ((crc & 0x0001) != 0) {                      // If the LSB is set
        crc >>= 1;                                    // Shift right and XOR 0xA001
        crc  ^= 0xA001;
      } else {                                        // Else LSB is not set
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
bool P020_Task::validP1char(char ch) {
  return
    isAlphaNumeric(ch) ||
    ch == '.' ||
    ch == ' ' ||
    ch == '\\' || // Single backslash, but escaped in C++
    ch == '\r' ||
    ch == '\n' ||
    ch == '(' ||
    ch == ')' ||
    ch == '-' ||
    ch == '*' ||
    ch == ':' ||
    ch == '_';
}

bool P020_Task::handleP1Char(char ch) {
  if (serial_buffer.length() >= _maxDataGramSize - 2) { // room for cr/lf
    # ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_DEBUG, F("P1   : Error: Buffer overflow, discarded input."));
    # endif // ifndef BUILD_NO_DEBUG
    state = ParserState::WAITING; // reset
  }

  bool done    = false;
  bool invalid = false;

  switch (state) {
    case ParserState::WAITING:

      if (ch == P020_DATAGRAM_START_CHAR)  {
        clearBuffer();
        addChar(ch);
        state = ParserState::READING;
      } // else ignore data
      break;
    case ParserState::READING:

      if (validP1char(ch)) {
        addChar(ch);
      } else if (ch == P020_DATAGRAM_END_CHAR) {
        addChar(ch);

        if (_CRCcheck) {
          checkI = 0;
          state  = ParserState::CHECKSUM;
        } else {
          done = true;
        }
      } else if (ch == P020_DATAGRAM_START_CHAR) {
        # ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG, F("P1   : Error: Start detected, discarded input."));
        # endif // ifndef BUILD_NO_DEBUG
        state = ParserState::WAITING; // reset
        return handleP1Char(ch);
      } else {
        invalid = true;
      }
      break;
    case ParserState::CHECKSUM:

      if (validP1char(ch)) {
        addChar(ch);
        ++checkI;

        if (checkI == P020_CHECKSUM_LENGTH) {
          done = true;
        }
      } else {
        invalid = true;
      }
      break;
  } // switch

  if (invalid) {
    // input is not a datagram char
    # ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_DEBUG, F("P1   : Error: DATA corrupt, discarded input."));
    # endif // ifndef BUILD_NO_DEBUG

    # if PLUGIN_020_DEBUG
    serialPrint(F("faulty char>"));
    serialPrint(String(ch));
    serialPrintln("<");
    # endif // if PLUGIN_020_DEBUG
    state = ParserState::WAITING; // reset
  }

  if (done) {
    done = checkDatagram();

    if (done) {
      // add the cr/lf pair to the datagram ahead of reading both
      // from serial as the datagram has already been validated
      addChar('\r');
      addChar('\n');
    } else if (_CRCcheck) {
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG, F("P1   : Error: Invalid CRC, dropped data"));
      # endif // ifndef BUILD_NO_DEBUG
    } else {
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG, F("P1   : Error: Invalid datagram, dropped data"));
      # endif // ifndef BUILD_NO_DEBUG
    }
    state = ParserState::WAITING; // prepare for next one
  }

  return done;
}

#endif // ifdef USES_P020
