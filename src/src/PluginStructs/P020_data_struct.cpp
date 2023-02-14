#include "../PluginStructs/P020_data_struct.h"

#ifdef USES_P020

# include "../ESPEasyCore/Serial.h"
# include "../ESPEasyCore/ESPEasyNetwork.h"

# include "../Globals/EventQueue.h"

# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/Misc.h"

# define P020_RX_WAIT              PCONFIG(4)
# define P020_RX_BUFFER            PCONFIG(7)


P020_Task::P020_Task(taskIndex_t taskIndex) : _taskIndex(taskIndex) {
  serial_buffer.reserve(P020_DATAGRAM_MAX_SIZE);
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

    #ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS

    // See: https://github.com/espressif/arduino-esp32/pull/6676
    ser2netClient.setTimeout((CONTROLLER_CLIENTTIMEOUT_DFLT + 500) / 1000); // in seconds!!!!
    Client *pClient = &ser2netClient;
    pClient->setTimeout(CONTROLLER_CLIENTTIMEOUT_DFLT);
    #else // ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS
    ser2netClient.setTimeout(CONTROLLER_CLIENTTIMEOUT_DFLT);                // in msec as it should be!
    #endif // ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS

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
  serial_buffer = String();
  serial_buffer.reserve(P020_DATAGRAM_MAX_SIZE);
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
  int count      = ser2netClient.available();
  int bytes_read = 0;
  uint8_t net_buf[P020_DATAGRAM_MAX_SIZE];

  if (count > 0) {
    if (count > P020_DATAGRAM_MAX_SIZE) { count = P020_DATAGRAM_MAX_SIZE; }
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
  int RXWait  = P020_RX_WAIT;
  int timeOut = RXWait;

  do {
    if (ser2netSerial->available()) {
      if (serial_buffer.length() > static_cast<size_t>(P020_RX_BUFFER)) {
        # ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG, F("Ser2Net   : Error: Buffer overflow, discarded input."));
        # endif // ifndef BUILD_NO_DEBUG
        ser2netSerial->read();
      }
      else { serial_buffer += (char)ser2netSerial->read(); }
      timeOut = RXWait; // if serial received, reset timeout counter
    } else {
      if (timeOut <= 0) { break; }
      delay(1);
      --timeOut;
    }
  } while (true);

  if (serial_buffer.length() > 0) {
    if (ser2netClient.connected()) { // Only send out if a client is connected
      ser2netClient.print(serial_buffer);
    }
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

  NewLinePos = message.indexOf('\n', StartPos);

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
      case 0: { break; }
      case 1: { // Generic
        if (NewLinePos > StartPos) {
          eventString  = F("!Serial#");
          eventString += message.substring(StartPos, NewLinePos);
        }
        break;
      }
      case 2: {                          // RFLink
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
  eventQueue.add(_taskIndex, F("Client"), (connected ? 1 : 0));
}

#endif // ifdef USES_P020
