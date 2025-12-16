#include "../Helpers/BusCmd_Handler_I2C.h"

/**
 * Constructor BusCmd_Handler_I2C
 */
BusCmd_Handler_I2C::BusCmd_Handler_I2C(uint8_t  i2cAddress,
                                       TwoWire *wire)
  : _i2cAddress(i2cAddress), _wire(wire)
{
  //
}

bool BusCmd_Handler_I2C::init() {
  return nullptr != _wire && _i2cAddress > 0 && _i2cAddress < 128;
}

uint8_t BusCmd_Handler_I2C::read8u() {
  return I2C_read8(_i2cAddress, &_ok);
}

uint16_t BusCmd_Handler_I2C::read16u() {
  return I2C_read16(_i2cAddress, &_ok);
}

uint32_t BusCmd_Handler_I2C::read24u() {
  return I2C_read24(_i2cAddress, &_ok);
}

uint32_t BusCmd_Handler_I2C::read32u() {
  return I2C_read32(_i2cAddress, &_ok);
}

uint8_t BusCmd_Handler_I2C::read8uREG(uint16_t reg,
                                      bool     wideReg) {
  return I2C_read8_reg(_i2cAddress, reg, &_ok);
}

uint16_t BusCmd_Handler_I2C::read16uREG(uint16_t reg,
                                        bool     wideReg) {
  return I2C_read16_reg(_i2cAddress, reg, &_ok);
}

uint32_t BusCmd_Handler_I2C::read24uREG(uint16_t reg,
                                        bool     wideReg) {
  return I2C_read24_reg(_i2cAddress, reg, &_ok);
}

uint32_t BusCmd_Handler_I2C::read32uREG(uint16_t reg,
                                        bool     wideReg) {
  return I2C_read32_reg(_i2cAddress, reg, &_ok);
}

std::vector<uint8_t>BusCmd_Handler_I2C::read8uB(uint32_t size) {
  std::vector<uint8_t> data_b;

  #ifdef ESP32

  if (size > I2C_BUFFER_LENGTH) {
    if (!_wire->setBufferSize(size)) { // 0 return on error
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_ERROR, strformat(F("I2C  : Error resizing buffer to size: %u"), size));
      # endif // ifndef BUILD_NO_DEBUG
      return data_b;
    }
  }
  #endif // ifdef ESP32
  data_b.reserve(size);

  _wire->requestFrom(_i2cAddress, size);

  for (uint8_t i = 0; i < size; ++i) {
    data_b.push_back(_wire->read());
  }
  #ifdef ESP32

  if (size > I2C_BUFFER_LENGTH) {
    _wire->setBufferSize(I2C_BUFFER_LENGTH); // Restore default
  }
  #endif // ifdef ESP32
  return data_b;
}

std::vector<uint16_t>BusCmd_Handler_I2C::read16uW(uint32_t size) {
  std::vector<uint16_t> data_w;

  #ifdef ESP32

  if ((size * 2) > I2C_BUFFER_LENGTH) {
    if (!_wire->setBufferSize(size * 2)) { // 0 return on error
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_ERROR, strformat(F("I2C  : Error resizing buffer to size: %u"), size * 2));
      # endif // ifndef BUILD_NO_DEBUG
      return data_w;
    }
  }
  #endif // ifdef ESP32
  data_w.reserve(size);

  _wire->requestFrom(_i2cAddress, size * 2);

  for (uint8_t i = 0; i < size; ++i) {
    data_w.push_back((_wire->read() << 8) | _wire->read());
  }
  #ifdef ESP32

  if ((size * 2) > I2C_BUFFER_LENGTH) {
    _wire->setBufferSize(I2C_BUFFER_LENGTH); // Restore default
  }
  #endif // ifdef ESP32
  return data_w;
}

std::vector<uint8_t>BusCmd_Handler_I2C::read8uBREG(uint16_t reg,
                                                   uint32_t size,
                                                   bool     wideReg) {
  std::vector<uint8_t> data_b;

  #ifdef ESP32

  if (size > I2C_BUFFER_LENGTH) {
    if (!_wire->setBufferSize(size)) { // 0 return on error
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_ERROR, strformat(F("I2C  : Error resizing buffer to size: %u"), size));
      # endif // ifndef BUILD_NO_DEBUG
      return data_b;
    }
  }
  #endif // ifdef ESP32
  data_b.reserve(size);

  if (I2C_write8(_i2cAddress, reg) && (_wire->requestFrom(_i2cAddress, size) == size)) {
    for (uint8_t i = 0; i < size; ++i) {
      data_b.push_back(_wire->read());
    }
  }
  #ifdef ESP32

  if (size > I2C_BUFFER_LENGTH) {
    _wire->setBufferSize(I2C_BUFFER_LENGTH); // Restore default
  }
  #endif // ifdef ESP32
  return data_b;
}

std::vector<uint16_t>BusCmd_Handler_I2C::read16uWREG(uint16_t reg,
                                                     uint32_t size,
                                                     bool     wideReg) {
  std::vector<uint16_t> data_w;

  #ifdef ESP32

  if ((size * 2) > I2C_BUFFER_LENGTH) {
    if (!_wire->setBufferSize(size * 2)) { // 0 return on error
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_ERROR, strformat(F("I2C  : Error resizing buffer to size: %u"), size * 2));
      # endif // ifndef BUILD_NO_DEBUG
      return data_w;
    }
  }
  #endif // ifdef ESP32
  data_w.reserve(size);

  if (I2C_write8(_i2cAddress, reg) && (_wire->requestFrom(_i2cAddress, size * 2) == size * 2)) {
    for (uint8_t i = 0; i < size; ++i) {
      data_w.push_back((_wire->read() << 8) | _wire->read());
    }
  }
  #ifdef ESP32

  if ((size * 2) > I2C_BUFFER_LENGTH) {
    _wire->setBufferSize(I2C_BUFFER_LENGTH); // Restore default
  }
  #endif // ifdef ESP32
  return data_w;
}

#if FEATURE_BUSCMD_STRING
String BusCmd_Handler_I2C::readString(uint32_t len) {
  String result;

  # ifdef ESP32

  if (len > I2C_BUFFER_LENGTH) {
    if (!_wire->setBufferSize(len)) { // 0 return on error
      #  ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_ERROR, strformat(F("I2C  : Error resizing buffer to size: %u"), len));
      #  endif // ifndef BUILD_NO_DEBUG
      return result;
    }
  }
  # endif // ifdef ESP32
  result.reserve(len);

  # ifdef ESP32
  const size_t maxBytesPerChunk = len;
  # endif // ifdef ESP32
  # ifdef ESP8266
  constexpr size_t maxBytesPerChunk = 128; // Chuncked in 32 byte blocks
  # endif // ifdef ESP8266
  size_t index = 0;

  while (index < len) {
    const size_t bytesRemaining = len - index;
    const size_t bytesThisChunk = min(bytesRemaining, maxBytesPerChunk);
    const size_t endIndex       = index + bytesThisChunk;
    const bool   isLastChunk    = (bytesRemaining <= maxBytesPerChunk);
    _wire->requestFrom(_i2cAddress, bytesThisChunk, isLastChunk);

    for (; index < endIndex; ++index) {
      # ifndef BUILD_NO_DEBUG

      if (_wire->available() < 1) {
        addLog(LOG_LEVEL_ERROR, strformat(F("I2C  : Error reading data, got just: %u from %u requested bytes"), index, bytesThisChunk));
        break;
      }
      # endif // ifndef BUILD_NO_DEBUG
      const uint8_t c = _wire->read();

      if (c != 0) {
        result += static_cast<char>(c);
      }
    }
  }

  # ifdef ESP32

  if (len > I2C_BUFFER_LENGTH) {
    _wire->setBufferSize(I2C_BUFFER_LENGTH); // Restore default
  }
  # endif // ifdef ESP32
  return result;
}

String BusCmd_Handler_I2C::readStringREG(uint16_t reg,
                                         uint32_t len,
                                         bool     wideReg) {
  String result;

  # ifdef ESP32

  if (len > I2C_BUFFER_LENGTH) {
    if (!_wire->setBufferSize(len)) { // 0 return on error
      #  ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_ERROR, strformat(F("I2C  : Error resizing buffer to size: %u"), len));
      #  endif // ifndef BUILD_NO_DEBUG
      return result;
    }
  }
  # endif // ifdef ESP32

  result.reserve(len);
  _wire->requestFrom(_i2cAddress, len);

  if (I2C_write8(_i2cAddress, reg) && (_wire->requestFrom(_i2cAddress, len) == len)) {
    for (uint8_t i = 0; i < len; ++i) {
      result += static_cast<char>(_wire->read());
    }
  }
  # ifdef ESP32

  if (len > I2C_BUFFER_LENGTH) {
    _wire->setBufferSize(I2C_BUFFER_LENGTH); // Restore default
  }
  # endif // ifdef ESP32
  return result;
}

#endif // if FEATURE_BUSCMD_STRING

bool BusCmd_Handler_I2C::write8u(uint8_t data) {
  return I2C_write8(_i2cAddress, data);
}

bool BusCmd_Handler_I2C::write16u(uint16_t data) {
  return I2C_write16(_i2cAddress, data);
}

bool BusCmd_Handler_I2C::write24u(uint32_t data) {
  return I2C_write24(_i2cAddress, data);
}

bool BusCmd_Handler_I2C::write32u(uint32_t data) {
  return I2C_write32(_i2cAddress, data);
}

bool BusCmd_Handler_I2C::write8uREG(uint16_t reg,
                                    uint8_t  data,
                                    bool     wideReg) {
  return I2C_write8_reg(_i2cAddress, reg, data);
}

bool BusCmd_Handler_I2C::write16uREG(uint16_t reg,
                                     uint16_t data,
                                     bool     wideReg) {
  return I2C_write16_reg(_i2cAddress, reg, data);
}

bool BusCmd_Handler_I2C::write24uREG(uint16_t reg,
                                     uint32_t data,
                                     bool     wideReg) {
  return I2C_write24_reg(_i2cAddress, reg, data);
}

bool BusCmd_Handler_I2C::write32uREG(uint16_t reg,
                                     uint32_t data,
                                     bool     wideReg) {
  return I2C_write32_reg(_i2cAddress, reg, data);
}

uint32_t BusCmd_Handler_I2C::write8uB(std::vector<uint8_t>data) {
  const size_t size = data.size();

  #ifdef ESP32

  if (size > I2C_BUFFER_LENGTH) {
    if (!_wire->setBufferSize(size)) { // 0 return on error
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_ERROR, strformat(F("I2C  : Error resizing buffer to size: %u"), size));
      # endif // ifndef BUILD_NO_DEBUG
      return 0;
    }
  }
  #endif // ifdef ESP32
  _wire->beginTransmission(_i2cAddress);

  for (size_t itb = 0; itb < size; ++itb) {
    _wire->write(data[itb]);
  }
  #ifdef ESP32

  if (size > I2C_BUFFER_LENGTH) {
    _wire->setBufferSize(I2C_BUFFER_LENGTH); // Restore default
  }
  #endif // ifdef ESP32
  return _wire->endTransmission() == 0;
}

uint32_t BusCmd_Handler_I2C::write16uW(std::vector<uint16_t>data) {
  const size_t size = data.size();

  #ifdef ESP32

  if ((size * 2) > I2C_BUFFER_LENGTH) {
    if (!_wire->setBufferSize(size * 2)) { // 0 return on error
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_ERROR, strformat(F("I2C  : Error resizing buffer to size: %u"), size * 2));
      # endif // ifndef BUILD_NO_DEBUG
      return 0;
    }
  }
  #endif // ifdef ESP32
  _wire->beginTransmission(_i2cAddress);

  for (size_t itw = 0; itw < size; ++itw) {
    _wire->write((uint8_t)(data[itw] << 8));
    _wire->write((uint8_t)(data[itw] & 0xFF));
  }
  #ifdef ESP32

  if ((size * 2) > I2C_BUFFER_LENGTH) {
    _wire->setBufferSize(I2C_BUFFER_LENGTH); // Restore default
  }
  #endif // ifdef ESP32
  return _wire->endTransmission() == 0;
}

uint32_t BusCmd_Handler_I2C::write8uBREG(uint16_t            reg,
                                         std::vector<uint8_t>data,
                                         bool                wideReg) {
  const size_t size = data.size();

  #ifdef ESP32

  if (size > I2C_BUFFER_LENGTH) {
    if (!_wire->setBufferSize(size)) { // 0 return on error
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_ERROR, strformat(F("I2C  : Error resizing buffer to size: %u"), size));
      # endif // ifndef BUILD_NO_DEBUG
      return 0;
    }
  }
  #endif // ifdef ESP32
  _wire->beginTransmission(_i2cAddress);
  _wire->write((uint8_t)reg);

  for (size_t itb = 0; itb < size; ++itb) {
    _wire->write(data[itb]);
  }
  #ifdef ESP32

  if (size > I2C_BUFFER_LENGTH) {
    _wire->setBufferSize(I2C_BUFFER_LENGTH); // Restore default
  }
  #endif // ifdef ESP32
  return _wire->endTransmission() == 0;
}

uint32_t BusCmd_Handler_I2C::write16uWREG(uint16_t             reg,
                                          std::vector<uint16_t>data,
                                          bool                 wideReg) {
  const size_t size = data.size();

  #ifdef ESP32

  if ((size * 2) > I2C_BUFFER_LENGTH) {
    if (!_wire->setBufferSize(size * 2)) { // 0 return on error
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_ERROR, strformat(F("I2C  : Error resizing buffer to size: %u"), size * 2));
      # endif // ifndef BUILD_NO_DEBUG
      return 0;
    }
  }
  #endif // ifdef ESP32
  _wire->beginTransmission(_i2cAddress);
  _wire->write((uint8_t)reg);

  for (size_t itw = 0; itw < size; ++itw) {
    _wire->write((uint8_t)(data[itw] << 8));
    _wire->write((uint8_t)(data[itw] & 0xFF));
  }
  #ifdef ESP32

  if ((size * 2) > I2C_BUFFER_LENGTH) {
    _wire->setBufferSize(I2C_BUFFER_LENGTH); // Restore default
  }
  #endif // ifdef ESP32
  return _wire->endTransmission() == 0;
}

#if FEATURE_BUSCMD_STRING
uint32_t BusCmd_Handler_I2C::writeString(const String& data) {
  const size_t len = data.length();

  # ifdef ESP32

  if (len > I2C_BUFFER_LENGTH) {
    if (!_wire->setBufferSize(len)) { // 0 return on error
      #  ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_ERROR, strformat(F("I2C  : Error resizing buffer to size: %u"), len));
      #  endif // ifndef BUILD_NO_DEBUG
      return 0;
    }
  }
  # endif // ifdef ESP32
  _wire->beginTransmission(_i2cAddress);

  for (size_t its = 0; its < len; ++its) {
    _wire->write(data[its]);
  }
  # ifdef ESP32

  if (len > I2C_BUFFER_LENGTH) {
    _wire->setBufferSize(I2C_BUFFER_LENGTH); // Restore default
  }
  # endif // ifdef ESP32
  return _wire->endTransmission() == 0;
}

uint32_t BusCmd_Handler_I2C::writeStringReg(uint16_t      reg,
                                            const String& data,
                                            bool          wideReg) {
  const size_t len = data.length();

  # ifdef ESP32

  if (len > I2C_BUFFER_LENGTH) {
    if (!_wire->setBufferSize(len)) { // 0 return on error
      #  ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_ERROR, strformat(F("I2C  : Error resizing buffer to size: %u"), len));
      #  endif // ifndef BUILD_NO_DEBUG
      return 0;
    }
  }
  # endif // ifdef ESP32
  _wire->beginTransmission(_i2cAddress);
  _wire->write((uint8_t)reg);

  for (size_t its = 0; its < len; ++its) {
    _wire->write(data[its]);
  }
  # ifdef ESP32

  if (len > I2C_BUFFER_LENGTH) {
    _wire->setBufferSize(I2C_BUFFER_LENGTH); // Restore default
  }
  # endif // ifdef ESP32
  return _wire->endTransmission() == 0;
}

#endif // if FEATURE_BUSCMD_STRING
