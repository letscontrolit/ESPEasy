#include <ESPeasySerial.h>

#if defined(DISABLE_SOFTWARE_SERIAL) && defined(ESP8266)

// ****************************************
// ESP8266 implementation wrapper
// No SoftwareSerial
// Only support HW serial on Serial 0 .. 1
// ****************************************
ESPeasySerial::ESPeasySerial(ESPEasySerialPort port, int receivePin, int transmitPin, bool inverse_logic, unsigned int buffSize)
  : _receivePin(receivePin), _transmitPin(transmitPin)
{
  _serialtype = ESPeasySerialType::getSerialType(port, receivePin, transmitPin);

  if (isValid()) {
    getHW()->pins(transmitPin, receivePin);
  }
}

ESPeasySerial::~ESPeasySerial() {
  end();
}

void ESPeasySerial::begin(unsigned long baud, SerialConfig config, SerialMode mode) {
  _baud = baud;

  if (_serialtype == ESPEasySerialPort::serial0_swap) {
    // Serial.swap() should only be called here and only once.
    if (!_serial0_swap_active) {
      Serial.begin(baud, config, mode, _transmitPin);
      Serial.swap();
      _serial0_swap_active = true;
      return;
    }
  }

  if (!isValid()) {
    _baud = 0;
    return;
  }
  doHWbegin(baud, config, mode);
}

void ESPeasySerial::end() {
  if (!isValid()) {
    return;
  }

  if (_serialtype == ESPEasySerialPort::serial0_swap) {
    if (_serial0_swap_active) {
      Serial.end();
      Serial.swap();
      _serial0_swap_active = false;
      return;
    }
  }
  getHW()->end();
}

HardwareSerial * ESPeasySerial::getHW() {
  switch (_serialtype) {
    case ESPEasySerialPort::serial0:
    case ESPEasySerialPort::serial0_swap: return &Serial;
    case ESPEasySerialPort::serial1:      return &Serial1;
    case ESPEasySerialPort::software:     break;
    default: break;
  }
  return nullptr;
}

const HardwareSerial * ESPeasySerial::getHW() const {
  switch (_serialtype) {
    case ESPEasySerialPort::serial0:
    case ESPEasySerialPort::serial0_swap: return &Serial;
    case ESPEasySerialPort::serial1:      return &Serial1;
    case ESPEasySerialPort::software:     break;
    default: break;
  }
  return nullptr;
}

bool ESPeasySerial::isValid() const {
  switch (_serialtype) {
    case ESPEasySerialPort::serial0:      return !_serial0_swap_active;
    case ESPEasySerialPort::serial0_swap: return _serial0_swap_active;
    case ESPEasySerialPort::serial1:      return true; // Must also check RX pin?
    case ESPEasySerialPort::software:     return false;
    default: break;
  }
  return false;
}

int ESPeasySerial::peek(void) {
  if (!isValid()) {
    return -1;
  }
  return getHW()->peek();
}

size_t ESPeasySerial::write(uint8_t val) {
  if (!isValid()) {
    return 0;
  }
  return getHW()->write(val);
}

size_t ESPeasySerial::write(const uint8_t *buffer, size_t size) {
  if (!isValid() || !buffer) {
    return 0;
  }
  return getHW()->write(buffer, size);
}

size_t ESPeasySerial::write(const char *buffer) {
  if (!buffer) { return 0; }
  return write(buffer, strlen(buffer));
}

int ESPeasySerial::read(void) {
  if (!isValid()) {
    return -1;
  }
  return getHW()->read();
}

size_t ESPeasySerial::readBytes(char *buffer, size_t size)  {
  if (!isValid() || !buffer) {
    return 0;
  }
  return getHW()->readBytes(buffer, size);
}

size_t ESPeasySerial::readBytes(uint8_t *buffer, size_t size)  {
  return readBytes((char *)buffer, size);
}

int ESPeasySerial::available(void) {
  if (!isValid()) {
    return 0;
  }
  return getHW()->available();
}

void ESPeasySerial::flush(void) {
  if (!isValid()) {
    return;
  }
  getHW()->flush();
}

bool ESPeasySerial::overflow() {
  return hasOverrun();
}

bool ESPeasySerial::hasOverrun(void) {
  return false;
}

// *****************************
// HardwareSerial specific
// *****************************

void ESPeasySerial::swap(uint8_t tx_pin) {
  if (isValid()) {
    switch (_serialtype) {
      case ESPEasySerialPort::serial0:
      case ESPEasySerialPort::serial0_swap:

        // isValid() also checks for correct swap active state.
        _serial0_swap_active = !_serial0_swap_active;
        getHW()->swap(tx_pin);

        if (_serialtype == ESPEasySerialPort::serial0) {
          _serialtype = ESPEasySerialPort::serial0_swap;
        } else {
          _serialtype = ESPEasySerialPort::serial0;
        }
        break;
      default:
        return;
    }
  }
}

int ESPeasySerial::baudRate(void) {
  if (!isValid()) {
    return _baud;
  }
  return getHW()->baudRate();
}

void ESPeasySerial::setDebugOutput(bool enable) {
  if (!isValid()) {
    return;
  }
  getHW()->setDebugOutput(enable);
}

bool ESPeasySerial::isTxEnabled(void) {
  if (!isValid()) {
    return false;
  }
  return getHW()->isTxEnabled();
}

bool ESPeasySerial::isRxEnabled(void) {
  if (!isValid()) {
    return false;
  }
  return getHW()->isRxEnabled();
}

bool ESPeasySerial::hasRxError(void) {
# ifdef CORE_POST_2_5_0

  if (!isValid()) {
    return false;
  }
  return getHW()->hasRxError();
# else // ifdef CORE_POST_2_5_0
  return false;
# endif // ifdef CORE_POST_2_5_0
}

void ESPeasySerial::startDetectBaudrate() {
  if (!isValid()) {
    return;
  }
# ifndef ARDUINO_ESP8266_RELEASE_2_3_0
  getHW()->startDetectBaudrate();
# endif // ifndef ARDUINO_ESP8266_RELEASE_2_3_0
}

unsigned long ESPeasySerial::testBaudrate() {
  if (!isValid()) {
    return 0;
  }
# ifndef ARDUINO_ESP8266_RELEASE_2_3_0
  return getHW()->testBaudrate();
# else // ifndef ARDUINO_ESP8266_RELEASE_2_3_0
  return 0;
# endif // ifndef ARDUINO_ESP8266_RELEASE_2_3_0
}

unsigned long ESPeasySerial::detectBaudrate(time_t timeoutMillis) {
  if (!isValid()) {
    return 0;
  }
# ifndef ARDUINO_ESP8266_RELEASE_2_3_0
  return getHW()->detectBaudrate(timeoutMillis);
# else // ifndef ARDUINO_ESP8266_RELEASE_2_3_0
  return 0;
# endif // ifndef ARDUINO_ESP8266_RELEASE_2_3_0
}

// *****************************
// SoftwareSerial specific
// *****************************


bool ESPeasySerial::listen() {
  if (isValid()) {
# ifndef ARDUINO_ESP8266_RELEASE_2_3_0
    return _swserial->listen();
# endif // ifndef ARDUINO_ESP8266_RELEASE_2_3_0
  }
  return false;
}

#endif // DISABLE_SOFTWARE_SERIAL
