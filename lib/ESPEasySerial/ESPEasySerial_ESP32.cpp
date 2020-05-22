#include <ESPeasySerial.h>

// ****************************************
// ESP32 implementation wrapper
// Only support HW serial on Serial 0 .. 2
// ****************************************

#ifdef ESP32
ESPeasySerial::ESPeasySerial(int receivePin, int transmitPin, bool inverse_logic, int serialPort)
  : _receivePin(receivePin), _transmitPin(transmitPin), _inverse_logic(inverse_logic)
{
  switch (serialPort) {
    case 0: _serialtype = ESPeasySerialType::serialtype::serial0; break;
    case 1: _serialtype = ESPeasySerialType::serialtype::serial1; break;
    case 2: _serialtype = ESPeasySerialType::serialtype::serial2; break;
    default:
      _serialtype = ESPeasySerialType::getSerialType(receivePin, transmitPin);
  }

  switch (_serialtype) {
    case ESPeasySerialType::serialtype::sc16is752:
    {
      ESPEasySC16IS752_Serial::I2C_address addr     = static_cast<ESPEasySC16IS752_Serial::I2C_address>(receivePin);
      ESPEasySC16IS752_Serial::SC16IS752_channel ch = static_cast<ESPEasySC16IS752_Serial::SC16IS752_channel>(transmitPin);
      _i2cserial = new ESPEasySC16IS752_Serial(addr, ch);
      break;
    }
    default:
      break;
  }

}

ESPeasySerial::~ESPeasySerial() {
  end();
  if (_i2cserial != nullptr) {
    delete _i2cserial;
  }
}

void ESPeasySerial::begin(unsigned long baud, uint32_t config
                          , int8_t rxPin, int8_t txPin, bool invert, unsigned long timeout_ms) {
  _baud = baud;

  if (rxPin != -1) { _receivePin = rxPin; }

  if (txPin != -1) { _transmitPin = txPin; }

  if (invert) { _inverse_logic = true; }

  if (!isValid()) {
    _baud = 0;
    return;
  }

  if (isI2Cserial()) {
    if (_i2cserial != nullptr) {
      _i2cserial->begin(baud);
    }
  } else {

    // Make sure the extra bit is set for the config. The config differs between ESP32 and ESP82xx
    config = config | 0x8000000;

    if (isValid()) {
      // Timeout added for 1.0.1
      // See: https://github.com/espressif/arduino-esp32/commit/233d31bed22211e8c85f82bcf2492977604bbc78
      // getHW()->begin(baud, config, _receivePin, _transmitPin, invert, timeout_ms);
      getHW()->begin(baud, config, _receivePin, _transmitPin, _inverse_logic);
    }
  }
}

void ESPeasySerial::end() {
  if (!isValid()) {
    return;
  }
  if (isI2Cserial()) {
    _i2cserial->end();
  } else {
    getHW()->end();
  }
}

HardwareSerial * ESPeasySerial::getHW() {
  switch (_serialtype) {
    case ESPeasySerialType::serialtype::serial0: return &Serial;
    case ESPeasySerialType::serialtype::serial1: return &Serial1;
    case ESPeasySerialType::serialtype::serial2: return &Serial2;

    default: break;
  }
  return nullptr;
}

const HardwareSerial * ESPeasySerial::getHW() const {
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
    case ESPeasySerialType::serialtype::sc16is752:    
      return _i2cserial != nullptr;

    // FIXME TD-er: Must perform proper check for GPIO pins here.
    default: break;
  }
  return false;
}

int ESPeasySerial::peek(void) {
  if (!isValid()) {
    return -1;
  }
  if (isI2Cserial()) {
    return _i2cserial->peek();
  }
  return getHW()->peek();
}

size_t ESPeasySerial::write(uint8_t val) {
  if (!isValid()) {
    return 0;
  }
  if (isI2Cserial()) {
    return _i2cserial->write(val);
  }
  return getHW()->write(val);
}

size_t ESPeasySerial::write(const uint8_t *buffer, size_t size) {
  if (!isValid() || !buffer) {
    return 0;
  }
  if (isI2Cserial()) {
    return _i2cserial->write(buffer, size);
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
  if (isI2Cserial()) {
    return _i2cserial->read();
  }
  return getHW()->read();
}

int ESPeasySerial::available(void) {
  if (!isValid()) {
    return 0;
  }
  if (isI2Cserial()) {
    return _i2cserial->available();
  }
  return getHW()->available();
}

void ESPeasySerial::flush(void) {
  if (!isValid()) {
    return;
  }
  if (isI2Cserial()) {
    return _i2cserial->flush();
  }
  getHW()->flush();
}

int ESPeasySerial::baudRate(void) {
  if (!isValid()) {
    return 0;
  }
  if (isI2Cserial()) {
    return _baud;
  }
  return getHW()->baudRate();
}

// Not supported in ESP32, since only HW serial is used.
// Function included since it is used in some libraries.
bool ESPeasySerial::listen() {
  return true;
}

#endif // ESP32
