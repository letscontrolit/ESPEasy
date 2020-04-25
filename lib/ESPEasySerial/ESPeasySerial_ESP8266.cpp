#include <ESPeasySerial.h>
#include <ESPEasySoftwareSerial.h>

#define DETECT_BAUDATE_TIMEOUT     250


String ESPeasySerial::getLogString() const {
  String log;

  log.reserve(48);
  log = "ESPEasy serial: ";

  if (isI2Cserial()) {
    log += "I2C: addr:";
    log += String(_receivePin);
    log += " ch:";
    log += _transmitPin == 0 ? "A" : "B";
  } else {
    if (isSWserial()) {
      log += "SW";
    } else {
      log += "HW";
    }
    log += ": rx:";
    log += String(_receivePin);
    log += " tx:";
    log += String(_transmitPin);
  }
  log += " baud:";
  log += String(_baud);
  return log;
}

// ****************************************
// ESP8266 implementation wrapper
// ****************************************
#ifdef ESP8266
bool ESPeasySerial::_serial0_swap_active = false;
#endif // ifdef ESP8266

#if !defined(DISABLE_SOFTWARE_SERIAL) && defined(ESP8266)

ESPeasySerial::ESPeasySerial(int receivePin, int transmitPin, bool inverse_logic, unsigned int buffSize, bool forceSWserial)
  : _i2cserial(nullptr), _swserial(nullptr), _receivePin(receivePin), _transmitPin(transmitPin)
{
  _serialtype = ESPeasySerialType::getSerialType(receivePin, transmitPin);
  if (forceSWserial) {
    switch (_serialtype) {
      case ESPeasySerialType::serialtype::sc16is752:
        break;
      default:
        _serialtype = ESPeasySerialType::serialtype::software;
        break;
    }    
  }

  switch (_serialtype) {
    case ESPeasySerialType::serialtype::software:
    {
      _swserial = new ESPeasySoftwareSerial(receivePin, transmitPin, inverse_logic, buffSize);
      break;
    }
    case ESPeasySerialType::serialtype::sc16is752:
    {
      ESPEasySC16IS752_Serial::I2C_address addr     = static_cast<ESPEasySC16IS752_Serial::I2C_address>(receivePin);
      ESPEasySC16IS752_Serial::SC16IS752_channel ch = static_cast<ESPEasySC16IS752_Serial::SC16IS752_channel>(transmitPin);
      _i2cserial = new ESPEasySC16IS752_Serial(addr, ch);
      break;
    }
    default:

      if (isValid()) {
        getHW()->pins(transmitPin, receivePin);
      }
      break;
  }
}

ESPeasySerial::~ESPeasySerial() {
  end();

  if (_swserial != nullptr) {
    delete _swserial;
  }

  if (_i2cserial != nullptr) {
    delete _i2cserial;
  }
}

void ESPeasySerial::begin(unsigned long baud, SerialConfig config, SerialMode mode) {
  _baud = baud;

  if (isSWserial()) {
    if (_swserial != nullptr) {
      _swserial->begin(baud);
    }
  } else if (isI2Cserial()) {
    if (_i2cserial != nullptr) {
      _i2cserial->begin(baud);
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
# ifndef ARDUINO_ESP8266_RELEASE_2_3_0
    _swserial->end();
# endif // ifndef ARDUINO_ESP8266_RELEASE_2_3_0
    return;
  } else if (isI2Cserial()) {
    _i2cserial->end();
  } else {
    //  getHW()->end();
  }
}

HardwareSerial * ESPeasySerial::getHW() {
  switch (_serialtype) {
    case ESPeasySerialType::serialtype::serial0:
    case ESPeasySerialType::serialtype::serial0_swap: return &Serial;
    case ESPeasySerialType::serialtype::serial1:      return &Serial1;
    case ESPeasySerialType::serialtype::software:     break;
    case ESPeasySerialType::serialtype::sc16is752:    break;
    default: break;
  }
  return nullptr;
}

const HardwareSerial * ESPeasySerial::getHW() const {
  switch (_serialtype) {
    case ESPeasySerialType::serialtype::serial0:
    case ESPeasySerialType::serialtype::serial0_swap: return &Serial;
    case ESPeasySerialType::serialtype::serial1:      return &Serial1;
    case ESPeasySerialType::serialtype::software:     break;
    case ESPeasySerialType::serialtype::sc16is752:    break;
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
    case ESPeasySerialType::serialtype::sc16is752:    return _i2cserial != nullptr;
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
  } else if (isI2Cserial()) {
    return _i2cserial->peek();
  } else {
    return getHW()->peek();
  }
}

size_t ESPeasySerial::write(uint8_t val) {
  if (!isValid()) {
    return 0;
  }

  if (isSWserial()) {
    return _swserial->write(val);
  } else if (isI2Cserial()) {
    return _i2cserial->write(val);
  } else {
    return getHW()->write(val);
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

      if (written == 0) { return count; }
      count += written;
    }
    return count;
  } else if (isI2Cserial()) {
    return _i2cserial->write(buffer, size);
  } else {
    return getHW()->write(buffer, size);
  }
}

size_t ESPeasySerial::write(const char *buffer) {
  if (!buffer) { return 0; }
  return write(buffer, strlen(buffer));
}

int ESPeasySerial::read(void) {
  if (!isValid()) {
    return -1;
  }

  if (isSWserial()) {
    return _swserial->read();
  } else if (isI2Cserial()) {
    return _i2cserial->read();
  } else {
    return getHW()->read();
  }
}

size_t ESPeasySerial::readBytes(char *buffer, size_t size)  {
  if (!isValid() || !buffer) {
    return 0;
  }

  if (isSWserial()) {
    // Not implemented in SoftwareSerial
    size_t count = 0;

    for (size_t i = 0; i < size; ++i) {
      int read = _swserial->read();

      if (read < 0) { return count; }
      buffer[i] = static_cast<char>(read & 0xFF);
      ++count;
    }
    return count;
  } else if (isI2Cserial()) {
    return _i2cserial->readBytes(buffer, size);
  } else {
    return getHW()->readBytes(buffer, size);
  }
}

size_t ESPeasySerial::readBytes(uint8_t *buffer, size_t size)  {
  return readBytes((char *)buffer, size);
}

int ESPeasySerial::available(void) {
  if (!isValid()) {
    return 0;
  }

  if (isSWserial()) {
    return _swserial->available();
  } else if (isI2Cserial()) {
    return _i2cserial->available();
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
  } else if (isI2Cserial()) {
    return _i2cserial->flush();
  } else {
    getHW()->flush();
  }
}

bool ESPeasySerial::overflow() {
  return hasOverrun();
}

bool ESPeasySerial::hasOverrun(void) {
  if (!isValid()) {
    return false;
  }
# ifdef CORE_PRE_2_4_2
  return false;
# else // ifdef CORE_PRE_2_4_2

  if (isSWserial() || isI2Cserial()) {
    return false;

    // return _swserial->overflow();
  } else {
    return getHW()->hasOverrun();
  }
# endif // ifdef CORE_PRE_2_4_2
}

// *****************************
// HardwareSerial specific
// *****************************

void ESPeasySerial::swap(uint8_t tx_pin) {
  if (getHW() == nullptr) { return; }
  getHW()->swap(tx_pin);
}

int ESPeasySerial::baudRate(void) {
  if (!isValid() || isSWserial() || isI2Cserial()) {
    return _baud;
  }
  return getHW()->baudRate();
}

void ESPeasySerial::setDebugOutput(bool enable) {
  if (!isValid() || isSWserial() || isI2Cserial()) {
    return;
  }
  getHW()->setDebugOutput(enable);
}

bool ESPeasySerial::isTxEnabled(void) {
  if (!isValid() || isSWserial() || isI2Cserial()) {
    return false;
  }
  return getHW()->isTxEnabled();
}

bool ESPeasySerial::isRxEnabled(void) {
  if (!isValid() || isSWserial() || isI2Cserial()) {
    return false;
  }
  return getHW()->isRxEnabled();
}

bool ESPeasySerial::hasRxError(void) {
# ifdef CORE_POST_2_5_0

  if (!isValid() || isSWserial() || isI2Cserial()) {
    return false;
  }
  return getHW()->hasRxError();
# else // ifdef CORE_POST_2_5_0
  return false;
# endif // ifdef CORE_POST_2_5_0
}

void ESPeasySerial::startDetectBaudrate() {
  if (!isValid() || isSWserial() || isI2Cserial()) {
    return;
  }
# ifdef CORE_PRE_2_4_2
  return;
# else // ifdef CORE_PRE_2_4_2
  getHW()->startDetectBaudrate();
# endif // ifdef CORE_PRE_2_4_2
}

unsigned long ESPeasySerial::testBaudrate() {
  if (!isValid() || isSWserial() || isI2Cserial()) {
    return 0;
  }
# ifdef CORE_PRE_2_4_2
  return 0;
# else // ifdef CORE_PRE_2_4_2
  return getHW()->testBaudrate();
# endif // ifdef CORE_PRE_2_4_2
}

unsigned long ESPeasySerial::detectBaudrate(time_t timeoutMillis) {
  if (!isValid() || isSWserial() || isI2Cserial()) {
    return 0;
  }
# ifdef CORE_PRE_2_4_2
  return 0;
# else // ifdef CORE_PRE_2_4_2
  return getHW()->detectBaudrate(timeoutMillis);
# endif // ifdef CORE_PRE_2_4_2
}

// *****************************
// SoftwareSerial specific
// *****************************


bool ESPeasySerial::listen() {
  if (isValid() && isSWserial()) {
# ifndef ARDUINO_ESP8266_RELEASE_2_3_0
    return _swserial->listen();
# endif // ifndef ARDUINO_ESP8266_RELEASE_2_3_0
  }
  return false;
}

bool ESPeasySerial::isListening() {
  if (isValid() && isSWserial()) {
# ifndef ARDUINO_ESP8266_RELEASE_2_3_0
    return _swserial->isListening();
# endif // ifndef ARDUINO_ESP8266_RELEASE_2_3_0
  }
  return false;
}

bool ESPeasySerial::stopListening() {
  if (isValid() && isSWserial()) {
# ifndef ARDUINO_ESP8266_RELEASE_2_3_0
    return _swserial->stopListening();
# endif // ifndef ARDUINO_ESP8266_RELEASE_2_3_0
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
  if (getHW() == nullptr) { return false; }
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

#endif // ifdef ESP8266
