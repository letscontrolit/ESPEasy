#ifdef ESP8266  // Needed for precompile issues.
#include <ESPeasySoftwareSerial.h>

bool ESPeasySoftwareSerial::_serial0_swap_active = false;

ESPeasySoftwareSerial::ESPeasySoftwareSerial(int receivePin, int transmitPin, bool inverse_logic, unsigned int buffSize)
    : _swserial(nullptr), _receivePin(receivePin), _transmitPin(transmitPin)
{
  _serialtype = ESPeasySerialType::getSerialType(receivePin, transmitPin);
  if (isSWserial()) {
    _swserial = new SoftwareSerial(receivePin, transmitPin, inverse_logic, buffSize);
  } else {
    getHW()->pins(transmitPin, receivePin);
  }
}

ESPeasySoftwareSerial::~ESPeasySoftwareSerial() {
  end();
  if (_swserial != nullptr) {
    delete _swserial;
  }
}

HardwareSerial* ESPeasySoftwareSerial::getHW() {
  switch (_serialtype) {
    case ESPeasySerialType::serialtype::serial0:
    case ESPeasySerialType::serialtype::serial0_swap:
      return &Serial;
    case ESPeasySerialType::serialtype::serial1:
      return &Serial1;
    case ESPeasySerialType::serialtype::software:
      break;
  }
  return nullptr;
}

const HardwareSerial* ESPeasySoftwareSerial::getHW() const {
  switch (_serialtype) {
    case ESPeasySerialType::serialtype::serial0:
    case ESPeasySerialType::serialtype::serial0_swap:
      return &Serial;
    case ESPeasySerialType::serialtype::serial1:
      return &Serial1;
    case ESPeasySerialType::serialtype::software:
      break;
  }
  return nullptr;
}

bool ESPeasySoftwareSerial::isValid() const {
  switch (_serialtype) {
    case ESPeasySerialType::serialtype::serial0:      return !_serial0_swap_active;
    case ESPeasySerialType::serialtype::serial0_swap: return _serial0_swap_active;
    case ESPeasySerialType::serialtype::serial1:      return true; // Must also check RX pin?
    case ESPeasySerialType::serialtype::software:     return _swserial != nullptr;
  }
  return false;
}

void ESPeasySoftwareSerial::begin(unsigned long baud, SerialConfig config, SerialMode mode, uint8_t tx_pin) {
  _baud = baud;
  if (_serialtype == ESPeasySerialType::serialtype::serial0_swap) {
    // Serial.swap() should only be called here and only once.
    if (!_serial0_swap_active) {
      Serial.begin(baud, config, mode, tx_pin);
      Serial.swap();
      _serial0_swap_active = true;
      return;
    }
  }
  if (!isValid()) {
    _baud = 0;
    return;
  }
  if (isSWserial()) {
    _swserial->begin(baud);
  } else {
    getHW()->begin(baud, config, mode, tx_pin);
  }
}

void ESPeasySoftwareSerial::end() {
  if (!isValid()) {
    return;
  }
  if (isSWserial()) {
    _swserial->end();
    return;
  } else {
    if (_serialtype == ESPeasySerialType::serialtype::serial0_swap) {
      if (_serial0_swap_active) {
        Serial.end();
        Serial.swap();
        _serial0_swap_active = false;
        return;
      }
    }
    getHW()->end();
  }
}

int ESPeasySoftwareSerial::peek(void) {
  if (!isValid()) {
    return -1;
  }
  if (isSWserial()) {
    return _swserial->peek();
  } else {
    return getHW()->peek();
  }
}

size_t ESPeasySoftwareSerial::write(uint8_t byte) {
  if (!isValid()) {
    return 0;
  }
  if (isSWserial()) {
    return _swserial->write(byte);
  } else {
    return getHW()->write(byte);
  }
}

size_t ESPeasySoftwareSerial::write(const uint8_t *buffer, size_t size) {
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

size_t ESPeasySoftwareSerial::write(const char *buffer) {
  if (!buffer) return 0;
  return write(buffer, strlen(buffer));
}

int ESPeasySoftwareSerial::read(void) {
  if (!isValid()) {
    return -1;
  }
  if (isSWserial()) {
    return _swserial->read();
  } else {
    return getHW()->read();
  }
}

size_t ESPeasySoftwareSerial::readBytes(char* buffer, size_t size)  {
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

size_t ESPeasySoftwareSerial::readBytes(uint8_t* buffer, size_t size)  {
  return readBytes((char*)buffer, size);
}

int ESPeasySoftwareSerial::available(void) {
  if (!isValid()) {
    return 0;
  }
  if (isSWserial()) {
    return _swserial->available();
  } else {
    return getHW()->available();
  }
}

void ESPeasySoftwareSerial::flush(void) {
  if (!isValid()) {
    return;
  }
  if (isSWserial()) {
    _swserial->flush();
  } else {
    getHW()->flush();
  }
}


bool ESPeasySoftwareSerial::overflow() { return hasOverrun(); }
bool ESPeasySoftwareSerial::hasOverrun(void) {
  if (!isValid()) {
    return false;
  }
  if (isSWserial()) {
    return _swserial->overflow();
  } else {
    return getHW()->hasOverrun();
  }
}



// *****************************
// HardwareSerial specific
// *****************************

void ESPeasySoftwareSerial::swap() {
  swap(1);
}

void ESPeasySoftwareSerial::swap(uint8_t tx_pin) {
  if (isValid() && !isSWserial()) {
    switch (_serialtype) {
      case ESPeasySerialType::serialtype::serial0:
      case ESPeasySerialType::serialtype::serial0_swap:
        // isValid() also checks for correct swap active state.
        _serial0_swap_active = !_serial0_swap_active;
        getHW()->swap(tx_pin);
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

int ESPeasySoftwareSerial::baudRate(void) {
  if (!isValid() || isSWserial()) {
    return _baud;
  }
  return getHW()->baudRate();
}

void ESPeasySoftwareSerial::setDebugOutput(bool enable) {
  if (!isValid() || isSWserial()) {
    return;
  }
  getHW()->setDebugOutput(enable);
}

bool ESPeasySoftwareSerial::isTxEnabled(void) {
  if (!isValid() || isSWserial()) {
    return false;
  }
  return getHW()->isTxEnabled();
}

 bool ESPeasySoftwareSerial::isRxEnabled(void) {
   if (!isValid() || isSWserial()) {
     return false;
   }
   return getHW()->isRxEnabled();
 }

#ifdef CORE_2_5_0
bool ESPeasySoftwareSerial::hasRxError(void) {
  if (!isValid() || isSWserial()) {
    return false;
  }
  return getHW()->hasRxError();
}
#endif

void ESPeasySoftwareSerial::startDetectBaudrate() {
  if (!isValid() || isSWserial()) {
    return;
  }
  getHW()->startDetectBaudrate();
}

unsigned long ESPeasySoftwareSerial::testBaudrate() {
  if (!isValid() || isSWserial()) {
    return 0;
  }
  return getHW()->testBaudrate();
}

unsigned long ESPeasySoftwareSerial::detectBaudrate(time_t timeoutMillis) {
  if (!isValid() || isSWserial()) {
    return 0;
  }
  return getHW()->detectBaudrate(timeoutMillis);
}


// *****************************
// SoftwareSerial specific
// *****************************


bool ESPeasySoftwareSerial::listen() {
  if (isValid() && isSWserial()) {
    return _swserial->listen();
  }
  return false;
}

bool ESPeasySoftwareSerial::isListening() {
  if (isValid() && isSWserial()) {
    return _swserial->isListening();
  }
  return false;
}

bool ESPeasySoftwareSerial::stopListening() {
  if (isValid() && isSWserial()) {
    return _swserial->stopListening();
  }
  return false;
}



#endif // ESP8266
