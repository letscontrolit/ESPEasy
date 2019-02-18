#include <ESPeasySerial.h>
#include <ESPEasySoftwareSerial.h>

#define DETECT_BAUDATE_TIMEOUT     250


String ESPeasySerial::getLogString() const {
  String log;
  log.reserve(48);
  log = "ESPEasy serial: ";
  if (isSWserial()) {
    log += "SW";
  } else {
    log += "HW";
  }
  log += ": rx:";
  log += String(_receivePin);
  log += " tx:";
  log += String(_transmitPin);
  log += " baud:";
  log += String(_baud);
  return log;
}

// ****************************************
// ESP8266 implementation wrapper
// ****************************************
#ifdef ESP8266
bool ESPeasySerial::_serial0_swap_active = false;
#endif

#if !defined(DISABLE_SOFTWARE_SERIAL) && defined(ESP8266)

ESPeasySerial::ESPeasySerial(int receivePin, int transmitPin, bool inverse_logic, unsigned int buffSize)
    : _swserial(nullptr), _receivePin(receivePin), _transmitPin(transmitPin)
{
  _serialtype = ESPeasySerialType::getSerialType(receivePin, transmitPin);
  if (isSWserial()) {
    _swserial = new ESPeasySoftwareSerial(receivePin, transmitPin, inverse_logic, buffSize);
  } else {
    getHW()->pins(transmitPin, receivePin);
  }
}

ESPeasySerial::~ESPeasySerial() {
  end();
  if (_swserial != nullptr) {
    delete _swserial;
  }
}

void ESPeasySerial::begin(unsigned long baud, SerialConfig config, SerialMode mode) {
  _baud = baud;
  if (isSWserial()) {
    if (_swserial != nullptr) {
      _swserial->begin(baud);
    }
  } else {
    doHWbegin(baud, config, mode);
  }
}

void ESPeasySerial::end() {
  if (!isValid()) {
    return;
  }
  flush();
  if (isSWserial()) {
#ifndef ARDUINO_ESP8266_RELEASE_2_3_0
    _swserial->end();
#endif
    return;
  } else {
  //  getHW()->end();
  }
}


HardwareSerial* ESPeasySerial::getHW() {
  switch (_serialtype) {
    case ESPeasySerialType::serialtype::serial0:
    case ESPeasySerialType::serialtype::serial0_swap: return &Serial;
    case ESPeasySerialType::serialtype::serial1:      return &Serial1;
    case ESPeasySerialType::serialtype::software:     break;
    default: break;
  }
  return nullptr;
}

const HardwareSerial* ESPeasySerial::getHW() const {
  switch (_serialtype) {
    case ESPeasySerialType::serialtype::serial0:
    case ESPeasySerialType::serialtype::serial0_swap: return &Serial;
    case ESPeasySerialType::serialtype::serial1:      return &Serial1;
    case ESPeasySerialType::serialtype::software:     break;
    default: break;
  }
  return nullptr;
}

bool ESPeasySerial::isValid() const {
  switch (_serialtype) {
    case ESPeasySerialType::serialtype::serial0:
    case ESPeasySerialType::serialtype::serial0_swap:
    case ESPeasySerialType::serialtype::serial1:      return true; // Must also check RX pin?
    case ESPeasySerialType::serialtype::software:     return _swserial != nullptr;
    default: break;
  }
  return false;
}



int ESPeasySerial::peek(void) {
  if (!isValid()) {
    return -1;
  }
  if (isSWserial()) {
    return _swserial->peek();
  } else {
    return getHW()->peek();
  }
}

size_t ESPeasySerial::write(uint8_t byte) {
  if (!isValid()) {
    return 0;
  }
  if (isSWserial()) {
    return _swserial->write(byte);
  } else {
    return getHW()->write(byte);
  }
}

size_t ESPeasySerial::write(const uint8_t *buffer, size_t size) {
  if (!isValid() || !buffer) {
    return 0;
  }
  if (isSWserial()) {
    // Not implemented in SoftwareSerial
    size_t count = 0;
    for (size_t i = 0; i < size; ++i) {
      size_t written = _swserial->write(buffer[i]);
      if (written == 0) return count;
      count += written;
    }
    return count;
  } else {
    return getHW()->write(buffer, size);
  }
}

size_t ESPeasySerial::write(const char *buffer) {
  if (!buffer) return 0;
  return write(buffer, strlen(buffer));
}

int ESPeasySerial::read(void) {
  if (!isValid()) {
    return -1;
  }
  if (isSWserial()) {
    return _swserial->read();
  } else {
    return getHW()->read();
  }
}

size_t ESPeasySerial::readBytes(char* buffer, size_t size)  {
  if (!isValid() || !buffer) {
    return 0;
  }
  if (isSWserial()) {
    // Not implemented in SoftwareSerial
    size_t count = 0;
    for (size_t i = 0; i < size; ++i) {
      int read = _swserial->read();
      if (read < 0) return count;
      buffer[i] = static_cast<char>(read & 0xFF);
      ++count;
    }
    return count;
  } else {
    return getHW()->readBytes(buffer, size);
  }
}

size_t ESPeasySerial::readBytes(uint8_t* buffer, size_t size)  {
  return readBytes((char*)buffer, size);
}

int ESPeasySerial::available(void) {
  if (!isValid()) {
    return 0;
  }
  if (isSWserial()) {
    return _swserial->available();
  } else {
    return getHW()->available();
  }
}

void ESPeasySerial::flush(void) {
  if (!isValid()) {
    return;
  }
  if (isSWserial()) {
    _swserial->flush();
  } else {
    getHW()->flush();
  }
}


bool ESPeasySerial::overflow() { return hasOverrun(); }
bool ESPeasySerial::hasOverrun(void) {
  if (!isValid()) {
    return false;
  }
#if defined(ARDUINO_ESP8266_RELEASE_2_4_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_1)
  return false;
#else
  if (isSWserial()) {
    return false;
    //return _swserial->overflow();
  } else {
    return getHW()->hasOverrun();
  }
#endif
}



// *****************************
// HardwareSerial specific
// *****************************

void ESPeasySerial::swap(uint8_t tx_pin) {
  if (getHW() == nullptr) return;
  getHW()->swap(tx_pin);
}

int ESPeasySerial::baudRate(void) {
  if (!isValid() || isSWserial()) {
    return _baud;
  }
  return getHW()->baudRate();
}

void ESPeasySerial::setDebugOutput(bool enable) {
  if (!isValid() || isSWserial()) {
    return;
  }
  getHW()->setDebugOutput(enable);
}

bool ESPeasySerial::isTxEnabled(void) {
  if (!isValid() || isSWserial()) {
    return false;
  }
  return getHW()->isTxEnabled();
}

 bool ESPeasySerial::isRxEnabled(void) {
   if (!isValid() || isSWserial()) {
     return false;
   }
   return getHW()->isRxEnabled();
 }

#ifdef CORE_2_5_0
bool ESPeasySerial::hasRxError(void) {
  if (!isValid() || isSWserial()) {
    return false;
  }
  return getHW()->hasRxError();
}
#endif

void ESPeasySerial::startDetectBaudrate() {
  if (!isValid() || isSWserial()) {
    return;
  }
#if defined(ARDUINO_ESP8266_RELEASE_2_4_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_1)
  return;
#else
  getHW()->startDetectBaudrate();
#endif
}

unsigned long ESPeasySerial::testBaudrate() {
  if (!isValid() || isSWserial()) {
    return 0;
  }
#if defined(ARDUINO_ESP8266_RELEASE_2_4_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_1)
  return 0;
#else
  return getHW()->testBaudrate();
#endif
}

unsigned long ESPeasySerial::detectBaudrate(time_t timeoutMillis) {
  if (!isValid() || isSWserial()) {
    return 0;
  }
#if defined(ARDUINO_ESP8266_RELEASE_2_4_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_1)
  return 0;
#else
  return getHW()->detectBaudrate(timeoutMillis);
#endif
}


// *****************************
// SoftwareSerial specific
// *****************************


bool ESPeasySerial::listen() {
  if (isValid() && isSWserial()) {
#ifndef ARDUINO_ESP8266_RELEASE_2_3_0
    return _swserial->listen();
#endif
  }
  return false;
}

bool ESPeasySerial::isListening() {
  if (isValid() && isSWserial()) {
#ifndef ARDUINO_ESP8266_RELEASE_2_3_0
    return _swserial->isListening();
#endif
  }
  return false;
}

bool ESPeasySerial::stopListening() {
  if (isValid() && isSWserial()) {
#ifndef ARDUINO_ESP8266_RELEASE_2_3_0
    return _swserial->stopListening();
#endif
  }
  return false;
}
#endif // ESP8266


#ifdef ESP8266
  // ****************************************
  // ESP8266 implementation wrapper
  // Shared functions for HW serial
  // ****************************************
  bool ESPeasySerial::doHWbegin(unsigned long baud, SerialConfig config, SerialMode mode) {
    if (getHW() == nullptr) return false;
    getHW()->begin(baud > 0 ? baud : 9600, config, mode, _transmitPin);
    getHW()->pins(_transmitPin, _receivePin);
    return isValid();
/*  // Detect baudrate gives crashes, so disabled for now.
    startDetectBaudrate();
    unsigned long detectedBaudRate = detectBaudrate(DETECT_BAUDATE_TIMEOUT);
    if(detectedBaudRate > 0) {
        delay(100); // Give some time...
        getHW()->begin(detectedBaudRate, config, mode, _transmitPin);
        _baud = detectedBaudRate;
        return true;
    } else {
      return false;
    }
*/
  }
#endif
