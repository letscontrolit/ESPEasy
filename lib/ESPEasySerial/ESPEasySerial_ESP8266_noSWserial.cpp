#include <ESPeasySerial.h>

#if defined(DISABLE_SOFTWARE_SERIAL) && defined(ESP8266)
// ****************************************
// ESP8266 implementation wrapper
// No SoftwareSerial
// Only support HW serial on Serial 0 .. 1
// ****************************************
ESPeasySerial::ESPeasySerial(int receivePin, int transmitPin, bool inverse_logic, unsigned int buffSize)
    : _receivePin(receivePin), _transmitPin(transmitPin)
{
  _serialtype = ESPeasySerialType::getSerialType(receivePin, transmitPin);
  if (isValid(void)) {
    getHW(void)->pins(transmitPin, receivePin);
  }
}

ESPeasySerial::~ESPeasySerial(void) {
  end(void);
}

void ESPeasySerial::begin(unsigned long baud, SerialConfig config, SerialMode mode) {
  _baud = baud;
  if (_serialtype == ESPeasySerialType::serialtype::serial0_swap) {
    // Serial.swap(void) should only be called here and only once.
    if (!_serial0_swap_active) {
      Serial.begin(baud, config, mode, _transmitPin);
      Serial.swap(void);
      _serial0_swap_active = true;
      return;
    }
  }
  if (!isValid(void)) {
    _baud = 0;
    return;
  }
  doHWbegin(baud, config, mode);
}

void ESPeasySerial::end(void) {
  if (!isValid(void)) {
    return;
  }
  if (_serialtype == ESPeasySerialType::serialtype::serial0_swap) {
    if (_serial0_swap_active) {
      Serial.end(void);
      Serial.swap(void);
      _serial0_swap_active = false;
      return;
    }
  }
  getHW(void)->end(void);
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
    case ESPeasySerialType::serialtype::serial0:      return !_serial0_swap_active;
    case ESPeasySerialType::serialtype::serial0_swap: return _serial0_swap_active;
    case ESPeasySerialType::serialtype::serial1:      return true; // Must also check RX pin?
    case ESPeasySerialType::serialtype::software:     return false;
    default: break;
  }
  return false;
}



int ESPeasySerial::peek(void) {
  if (!isValid(void)) {
    return -1;
  }
  return getHW(void)->peek(void);
}

size_t ESPeasySerial::write(uint8_t byte) {
  if (!isValid(void)) {
    return 0;
  }
  return getHW(void)->write(byte);
}

size_t ESPeasySerial::write(const uint8_t *buffer, size_t size) {
  if (!isValid(void) || !buffer) {
    return 0;
  }
  return getHW(void)->write(buffer, size);
}

size_t ESPeasySerial::write(const char *buffer) {
  if (!buffer) return 0;
  return write(buffer, strlen(buffer));
}

int ESPeasySerial::read(void) {
  if (!isValid(void)) {
    return -1;
  }
  return getHW(void)->read(void);
}

size_t ESPeasySerial::readBytes(char* buffer, size_t size)  {
  if (!isValid(void) || !buffer) {
    return 0;
  }
  return getHW(void)->readBytes(buffer, size);
}

size_t ESPeasySerial::readBytes(uint8_t* buffer, size_t size)  {
  return readBytes((char*)buffer, size);
}

int ESPeasySerial::available(void) {
  if (!isValid(void)) {
    return 0;
  }
  return getHW(void)->available(void);
}

void ESPeasySerial::flush(void) {
  if (!isValid(void)) {
    return;
  }
  getHW(void)->flush(void);
}


bool ESPeasySerial::overflow(void) { return hasOverrun(void); }
bool ESPeasySerial::hasOverrun(void) {
  return false;
}



// *****************************
// HardwareSerial specific
// *****************************

void ESPeasySerial::swap(uint8_t tx_pin) {
  if (isValid(void)) {
    switch (_serialtype) {
      case ESPeasySerialType::serialtype::serial0:
      case ESPeasySerialType::serialtype::serial0_swap:
        // isValid(void) also checks for correct swap active state.
        _serial0_swap_active = !_serial0_swap_active;
        getHW(void)->swap(tx_pin);
        if (_serialtype == ESPeasySerialType::serialtype::serial0) {
          _serialtype = ESPeasySerialType::serialtype::serial0_swap;
        } else {
          _serialtype = ESPeasySerialType::serialtype::serial0;
        }
        break;
      default:
        return;
    }
  }
}

int ESPeasySerial::baudRate(void) {
  if (!isValid(void)) {
    return _baud;
  }
  return getHW(void)->baudRate(void);
}

void ESPeasySerial::setDebugOutput(bool enable) {
  if (!isValid(void)) {
    return;
  }
  getHW(void)->setDebugOutput(enable);
}

bool ESPeasySerial::isTxEnabled(void) {
  if (!isValid(void)) {
    return false;
  }
  return getHW(void)->isTxEnabled(void);
}

 bool ESPeasySerial::isRxEnabled(void) {
   if (!isValid(void)) {
     return false;
   }
   return getHW(void)->isRxEnabled(void);
 }

bool ESPeasySerial::hasRxError(void) {
#ifdef CORE_POST_2_5_0
  if (!isValid(void)) {
    return false;
  }
  return getHW(void)->hasRxError(void);
#else
  return false;
#endif
}

void ESPeasySerial::startDetectBaudrate(void) {
  if (!isValid(void)) {
    return;
  }
#ifndef ARDUINO_ESP8266_RELEASE_2_3_0
  getHW(void)->startDetectBaudrate(void);
#endif
}

unsigned long ESPeasySerial::testBaudrate(void) {
  if (!isValid(void)) {
    return 0;
  }
#ifndef ARDUINO_ESP8266_RELEASE_2_3_0
  return getHW(void)->testBaudrate(void);
#else
  return 0;
#endif
}

unsigned long ESPeasySerial::detectBaudrate(time_t timeoutMillis) {
  if (!isValid(void)) {
    return 0;
  }
#ifndef ARDUINO_ESP8266_RELEASE_2_3_0
  return getHW(void)->detectBaudrate(timeoutMillis);
#else
  return 0;
#endif
}


// *****************************
// SoftwareSerial specific
// *****************************


bool ESPeasySerial::listen(void) {
  if (isValid(void)) {
#ifndef ARDUINO_ESP8266_RELEASE_2_3_0
    return _swserial->listen(void);
#endif
  }
  return false;
}

#endif // DISABLE_SOFTWARE_SERIAL
