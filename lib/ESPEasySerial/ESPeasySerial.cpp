#include <ESPeasySerial.h>

#define DETECT_BAUDATE_TIMEOUT     250


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
    _swserial = new SoftwareSerial(receivePin, transmitPin, inverse_logic, buffSize);
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
  if (_serialtype == ESPeasySerialType::serialtype::serial0_swap) {
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
  if (isSWserial()) {
    _swserial->begin(baud);
    if (baud == 9600) {
      // Use interrupts for TX when baudrate is 9600.
      // See change in this commit:
      // https://github.com/plerup/espsoftwareserial/commit/7077979af30f12454b8e3165df8f41c73c5a2d24
      _swserial->enableIntTx(true);
    }
  } else {
    doHWbegin(baud, config, mode);
  }
}

void ESPeasySerial::end() {
  if (!isValid()) {
    return;
  }
  if (isSWserial()) {
#ifndef ARDUINO_ESP8266_RELEASE_2_3_0
    _swserial->end();
#endif
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
    case ESPeasySerialType::serialtype::serial0:      return !_serial0_swap_active;
    case ESPeasySerialType::serialtype::serial0_swap: return _serial0_swap_active;
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
    return _swserial->overflow();
  } else {
    return getHW()->hasOverrun();
  }
#endif
}



// *****************************
// HardwareSerial specific
// *****************************

void ESPeasySerial::swap(uint8_t tx_pin) {
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
    if (!isValid()) {
      return false;
    }
    if (baud > 0) {
      getHW()->begin(baud ? baud : 9600, config, mode, _transmitPin);
      return true;
    }
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
  }
#endif



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
  if (isValid()) {
    getHW()->pins(transmitPin, receivePin);
  }
}

ESPeasySerial::~ESPeasySerial() {
  end();
}

void ESPeasySerial::begin(unsigned long baud, SerialConfig config, SerialMode mode) {
  _baud = baud;
  if (_serialtype == ESPeasySerialType::serialtype::serial0_swap) {
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
    case ESPeasySerialType::serialtype::serial0:      return !_serial0_swap_active;
    case ESPeasySerialType::serialtype::serial0_swap: return _serial0_swap_active;
    case ESPeasySerialType::serialtype::serial1:      return true; // Must also check RX pin?
    case ESPeasySerialType::serialtype::software:     return false;
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

size_t ESPeasySerial::write(uint8_t byte) {
  if (!isValid()) {
    return 0;
  }
  return getHW()->write(byte);
}

size_t ESPeasySerial::write(const uint8_t *buffer, size_t size) {
  if (!isValid() || !buffer) {
    return 0;
  }
  return getHW()->write(buffer, size);
}

size_t ESPeasySerial::write(const char *buffer) {
  if (!buffer) return 0;
  return write(buffer, strlen(buffer));
}

int ESPeasySerial::read(void) {
  if (!isValid()) {
    return -1;
  }
  return getHW()->read();
}

size_t ESPeasySerial::readBytes(char* buffer, size_t size)  {
  if (!isValid() || !buffer) {
    return 0;
  }
  return getHW()->readBytes(buffer, size);
}

size_t ESPeasySerial::readBytes(uint8_t* buffer, size_t size)  {
  return readBytes((char*)buffer, size);
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


bool ESPeasySerial::overflow() { return hasOverrun(); }
bool ESPeasySerial::hasOverrun(void) {
  return false;
}



// *****************************
// HardwareSerial specific
// *****************************

void ESPeasySerial::swap(uint8_t tx_pin) {
  if (isValid()) {
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

#ifdef CORE_2_5_0
bool ESPeasySerial::hasRxError(void) {
  if (!isValid()) {
    return false;
  }
  return getHW()->hasRxError();
}
#endif

void ESPeasySerial::startDetectBaudrate() {
  if (!isValid()) {
    return;
  }
#ifndef ARDUINO_ESP8266_RELEASE_2_3_0
  getHW()->startDetectBaudrate();
#endif
}

unsigned long ESPeasySerial::testBaudrate() {
  if (!isValid()) {
    return 0;
  }
#ifndef ARDUINO_ESP8266_RELEASE_2_3_0
  return getHW()->testBaudrate();
#else
  return 0;
#endif
}

unsigned long ESPeasySerial::detectBaudrate(time_t timeoutMillis) {
  if (!isValid()) {
    return 0;
  }
#ifndef ARDUINO_ESP8266_RELEASE_2_3_0
  return getHW()->detectBaudrate(timeoutMillis);
#else
  return 0;
#endif
}


// *****************************
// SoftwareSerial specific
// *****************************


bool ESPeasySerial::listen() {
  if (isValid()) {
#ifndef ARDUINO_ESP8266_RELEASE_2_3_0
    return _swserial->listen();
#endif
  }
  return false;
}

#endif // DISABLE_SOFTWARE_SERIAL


// ****************************************
// ESP32 implementation wrapper
// Only support HW serial on Serial 0 .. 2
// ****************************************

#ifdef ESP32
ESPeasySerial::ESPeasySerial(int receivePin, int transmitPin, bool inverse_logic, int serialPort)
    : _receivePin(receivePin), _transmitPin(transmitPin), _inverse_logic(inverse_logic)
{
  switch (serialPort) {
    case 0: _serialtype = ESPeasySerialType::serialtype::serial0;
    case 1: _serialtype = ESPeasySerialType::serialtype::serial1;
    case 2: _serialtype = ESPeasySerialType::serialtype::serial2;
    default:
      _serialtype = ESPeasySerialType::getSerialType(receivePin, transmitPin);
  }
}

ESPeasySerial::~ESPeasySerial() {
  end();
}

void ESPeasySerial::begin(unsigned long baud, uint32_t config
         , int8_t rxPin, int8_t txPin, bool invert, unsigned long timeout_ms) {
  _baud = baud;
  if (rxPin != -1) _receivePin = rxPin;
  if (txPin != -1) _transmitPin = txPin;
  if (invert) _inverse_logic = true;
  if (!isValid()) {
    _baud = 0;
    return;
  }
  // Make sure the extra bit is set for the config. The config differs between ESP32 and ESP82xx
  config = config | 0x8000000;

  // Timeout added for 1.0.1
  // See: https://github.com/espressif/arduino-esp32/commit/233d31bed22211e8c85f82bcf2492977604bbc78
//  getHW()->begin(baud, config, _receivePin, _transmitPin, invert, timeout_ms);
  getHW()->begin(baud, config, _receivePin, _transmitPin, _inverse_logic);
}

void ESPeasySerial::end() {
  if (!isValid()) {
    return;
  }
  getHW()->end();
}

HardwareSerial* ESPeasySerial::getHW() {
  switch (_serialtype) {
    case ESPeasySerialType::serialtype::serial0: return &Serial;
    case ESPeasySerialType::serialtype::serial1: return &Serial1;
    case ESPeasySerialType::serialtype::serial2: return &Serial2;

    default: break;
  }
  return nullptr;
}

const HardwareSerial* ESPeasySerial::getHW() const {
  switch (_serialtype) {
    case ESPeasySerialType::serialtype::serial0: return &Serial;
    case ESPeasySerialType::serialtype::serial1: return &Serial1;
    case ESPeasySerialType::serialtype::serial2: return &Serial2;
    default: break;
  }
  return nullptr;
}

bool ESPeasySerial::isValid() const {
  switch (_serialtype) {
    case ESPeasySerialType::serialtype::serial0:
    case ESPeasySerialType::serialtype::serial2:
      return true;
    case ESPeasySerialType::serialtype::serial1:
      return _transmitPin != -1 && _receivePin != -1;
      // FIXME TD-er: Must perform proper check for GPIO pins here.
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

size_t ESPeasySerial::write(uint8_t byte) {
  if (!isValid()) {
    return 0;
  }
  return getHW()->write(byte);
}

size_t ESPeasySerial::write(const uint8_t *buffer, size_t size) {
  if (!isValid() || !buffer) {
    return 0;
  }
  return getHW()->write(buffer, size);
}

size_t ESPeasySerial::write(const char *buffer) {
  if (!buffer) return 0;
  return write(buffer, strlen(buffer));
}

int ESPeasySerial::read(void) {
  if (!isValid()) {
    return -1;
  }
  return getHW()->read();
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

int ESPeasySerial::baudRate(void) {
  if (!isValid()) {
    return 0;
  }
  return getHW()->baudRate();
}


// Not supported in ESP32, since only HW serial is used.
// Function included since it is used in some libraries.
bool ESPeasySerial::listen() {
  return true;
}


#endif // ESP32
