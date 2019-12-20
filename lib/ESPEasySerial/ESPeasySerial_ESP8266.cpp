#include <ESPeasySerial.h>
#include <ESPEasySoftwareSerial.h>

#define DETECT_BAUDATE_TIMEOUT     250


String ESPeasySerial::getLogString(void) const {
  String log;
  log.reserve(48);
  log = "ESPEasy serial: ";
  if (isSWserial(void)) {
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
  if (isSWserial(void)) {
    _swserial = new ESPeasySoftwareSerial(receivePin, transmitPin, inverse_logic, buffSize);
  } else if (isValid(void)) {
    getHW(void)->pins(transmitPin, receivePin);
  }
}

ESPeasySerial::~ESPeasySerial(void) {
  end(void);
  if (_swserial != nullptr) {
    delete _swserial;
  }
}

void ESPeasySerial::begin(unsigned long baud, SerialConfig config, SerialMode mode) {
  _baud = baud;
  if (isSWserial(void)) {
    if (_swserial != nullptr) {
      _swserial->begin(baud);
    }
  } else {
    doHWbegin(baud, config, mode);
  }
}

void ESPeasySerial::end(void) {
  if (!isValid(void)) {
    return;
  }
  flush(void);
  if (isSWserial(void)) {
#ifndef ARDUINO_ESP8266_RELEASE_2_3_0
    _swserial->end(void);
#endif
    return;
  } else {
  //  getHW(void)->end(void);
  }
}


HardwareSerial* ESPeasySerial::getHW(void) {
  switch (_serialtype) {
    case ESPeasySerialType::serialtype::serial0:
    case ESPeasySerialType::serialtype::serial0_swap: return &Serial;
    case ESPeasySerialType::serialtype::serial1:      return &Serial1;
    case ESPeasySerialType::serialtype::software:     break;
    default: break;
  }
  return nullptr;
}

const HardwareSerial* ESPeasySerial::getHW(void) const {
  switch (_serialtype) {
    case ESPeasySerialType::serialtype::serial0:
    case ESPeasySerialType::serialtype::serial0_swap: return &Serial;
    case ESPeasySerialType::serialtype::serial1:      return &Serial1;
    case ESPeasySerialType::serialtype::software:     break;
    default: break;
  }
  return nullptr;
}

bool ESPeasySerial::isValid(void) const {
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
  if (!isValid(void)) {
    return -1;
  }
  if (isSWserial(void)) {
    return _swserial->peek(void);
  } else {
    return getHW(void)->peek(void);
  }
}

size_t ESPeasySerial::write(uint8_t byte) {
  if (!isValid(void)) {
    return 0;
  }
  if (isSWserial(void)) {
    return _swserial->write(byte);
  } else {
    return getHW(void)->write(byte);
  }
}

size_t ESPeasySerial::write(const uint8_t *buffer, size_t size) {
  if (!isValid(void) || !buffer) {
    return 0;
  }
  if (isSWserial(void)) {
    // Not implemented in SoftwareSerial
    size_t count = 0;
    for (size_t i = 0; i < size; ++i) {
      size_t written = _swserial->write(buffer[i]);
      if (written == 0) return count;
      count += written;
    }
    return count;
  } else {
    return getHW(void)->write(buffer, size);
  }
}

size_t ESPeasySerial::write(const char *buffer) {
  if (!buffer) return 0;
  return write(buffer, strlen(buffer));
}

int ESPeasySerial::read(void) {
  if (!isValid(void)) {
    return -1;
  }
  if (isSWserial(void)) {
    return _swserial->read(void);
  } else {
    return getHW(void)->read(void);
  }
}

size_t ESPeasySerial::readBytes(char* buffer, size_t size)  {
  if (!isValid(void) || !buffer) {
    return 0;
  }
  if (isSWserial(void)) {
    // Not implemented in SoftwareSerial
    size_t count = 0;
    for (size_t i = 0; i < size; ++i) {
      int read = _swserial->read(void);
      if (read < 0) return count;
      buffer[i] = static_cast<char>(read & 0xFF);
      ++count;
    }
    return count;
  } else {
    return getHW(void)->readBytes(buffer, size);
  }
}

size_t ESPeasySerial::readBytes(uint8_t* buffer, size_t size)  {
  return readBytes((char*)buffer, size);
}

int ESPeasySerial::available(void) {
  if (!isValid(void)) {
    return 0;
  }
  if (isSWserial(void)) {
    return _swserial->available(void);
  } else {
    return getHW(void)->available(void);
  }
}

void ESPeasySerial::flush(void) {
  if (!isValid(void)) {
    return;
  }
  if (isSWserial(void)) {
    _swserial->flush(void);
  } else {
    getHW(void)->flush(void);
  }
}


bool ESPeasySerial::overflow(void) { return hasOverrun(void); }
bool ESPeasySerial::hasOverrun(void) {
  if (!isValid(void)) {
    return false;
  }
#ifdef CORE_PRE_2_4_2
  return false;
#else
  if (isSWserial(void)) {
    return false;
    //return _swserial->overflow(void);
  } else {
    return getHW(void)->hasOverrun(void);
  }
#endif
}



// *****************************
// HardwareSerial specific
// *****************************

void ESPeasySerial::swap(uint8_t tx_pin) {
  if (getHW(void) == nullptr) return;
  getHW(void)->swap(tx_pin);
}

int ESPeasySerial::baudRate(void) {
  if (!isValid(void) || isSWserial(void)) {
    return _baud;
  }
  return getHW(void)->baudRate(void);
}

void ESPeasySerial::setDebugOutput(bool enable) {
  if (!isValid(void) || isSWserial(void)) {
    return;
  }
  getHW(void)->setDebugOutput(enable);
}

bool ESPeasySerial::isTxEnabled(void) {
  if (!isValid(void) || isSWserial(void)) {
    return false;
  }
  return getHW(void)->isTxEnabled(void);
}

 bool ESPeasySerial::isRxEnabled(void) {
   if (!isValid(void) || isSWserial(void)) {
     return false;
   }
   return getHW(void)->isRxEnabled(void);
 }

bool ESPeasySerial::hasRxError(void) {
#ifdef CORE_POST_2_5_0
  if (!isValid(void) || isSWserial(void)) {
    return false;
  }
  return getHW(void)->hasRxError(void);
#else
  return false;
#endif
}

void ESPeasySerial::startDetectBaudrate(void) {
  if (!isValid(void) || isSWserial(void)) {
    return;
  }
#ifdef CORE_PRE_2_4_2
  return;
#else
  getHW(void)->startDetectBaudrate(void);
#endif
}

unsigned long ESPeasySerial::testBaudrate(void) {
  if (!isValid(void) || isSWserial(void)) {
    return 0;
  }
#ifdef CORE_PRE_2_4_2
  return 0;
#else
  return getHW(void)->testBaudrate(void);
#endif
}

unsigned long ESPeasySerial::detectBaudrate(time_t timeoutMillis) {
  if (!isValid(void) || isSWserial(void)) {
    return 0;
  }
#ifdef CORE_PRE_2_4_2
  return 0;
#else
  return getHW(void)->detectBaudrate(timeoutMillis);
#endif
}


// *****************************
// SoftwareSerial specific
// *****************************


bool ESPeasySerial::listen(void) {
  if (isValid(void) && isSWserial(void)) {
#ifndef ARDUINO_ESP8266_RELEASE_2_3_0
    return _swserial->listen(void);
#endif
  }
  return false;
}

bool ESPeasySerial::isListening(void) {
  if (isValid(void) && isSWserial(void)) {
#ifndef ARDUINO_ESP8266_RELEASE_2_3_0
    return _swserial->isListening(void);
#endif
  }
  return false;
}

bool ESPeasySerial::stopListening(void) {
  if (isValid(void) && isSWserial(void)) {
#ifndef ARDUINO_ESP8266_RELEASE_2_3_0
    return _swserial->stopListening(void);
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
    if (getHW(void) == nullptr) return false;
    getHW(void)->begin(baud > 0 ? baud : 9600, config, mode, _transmitPin);
    getHW(void)->pins(_transmitPin, _receivePin);
    return isValid(void);
/*  // Detect baudrate gives crashes, so disabled for now.
    startDetectBaudrate(void);
    unsigned long detectedBaudRate = detectBaudrate(DETECT_BAUDATE_TIMEOUT);
    if(detectedBaudRate > 0) {
        delay(100); // Give some time...
        getHW(void)->begin(detectedBaudRate, config, mode, _transmitPin);
        _baud = detectedBaudRate;
        return true;
    } else {
      return false;
    }
*/
  }
#endif
