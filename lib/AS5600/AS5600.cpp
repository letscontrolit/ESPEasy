//
//    FILE: AS56000.cpp
//  AUTHOR: Rob Tillaart
// VERSION: 0.6.1
// PURPOSE: Arduino library for AS5600 magnetic rotation meter
//    DATE: 2022-05-28
//     URL: https://github.com/RobTillaart/AS5600


#include "AS5600.h"


//  CONFIGURATION REGISTERS
const uint8_t AS5600_ZMCO = 0x00;
const uint8_t AS5600_ZPOS = 0x01;   //  + 0x02
const uint8_t AS5600_MPOS = 0x03;   //  + 0x04
const uint8_t AS5600_MANG = 0x05;   //  + 0x06
const uint8_t AS5600_CONF = 0x07;   //  + 0x08

//  CONFIGURATION BIT MASKS - byte level
const uint8_t AS5600_CONF_POWER_MODE    = 0x03;
const uint8_t AS5600_CONF_HYSTERESIS    = 0x0C;
const uint8_t AS5600_CONF_OUTPUT_MODE   = 0x30;
const uint8_t AS5600_CONF_PWM_FREQUENCY = 0xC0;
const uint8_t AS5600_CONF_SLOW_FILTER   = 0x03;
const uint8_t AS5600_CONF_FAST_FILTER   = 0x1C;
const uint8_t AS5600_CONF_WATCH_DOG     = 0x20;


//  UNKNOWN REGISTERS 0x09-0x0A

//  OUTPUT REGISTERS
const uint8_t AS5600_RAW_ANGLE = 0x0C;   //  + 0x0D
const uint8_t AS5600_ANGLE     = 0x0E;   //  + 0x0F

// I2C_ADDRESS REGISTERS (AS5600L)
const uint8_t AS5600_I2CADDR   = 0x20;
const uint8_t AS5600_I2CUPDT   = 0x21;

//  STATUS REGISTERS
const uint8_t AS5600_STATUS    = 0x0B;
const uint8_t AS5600_AGC       = 0x1A;
const uint8_t AS5600_MAGNITUDE = 0x1B;   //  + 0x1C
const uint8_t AS5600_BURN      = 0xFF;

//  STATUS BITS
const uint8_t AS5600_MAGNET_HIGH   = 0x08;
const uint8_t AS5600_MAGNET_LOW    = 0x10;
const uint8_t AS5600_MAGNET_DETECT = 0x20;


AS5600::AS5600(TwoWire *wire)
{
  _wire = wire;
}


bool AS5600::begin(uint8_t directionPin)
{
  _directionPin = directionPin;
  if (_directionPin != AS5600_SW_DIRECTION_PIN)
  {
    pinMode(_directionPin, OUTPUT);
  }
  setDirection(AS5600_CLOCK_WISE);

  if (! isConnected()) return false;
  return true;
}


bool AS5600::isConnected()
{
  _wire->beginTransmission(_address);
  return ( _wire->endTransmission() == 0);
}


uint8_t AS5600::getAddress()
{
  return _address;
}


/////////////////////////////////////////////////////////
//
//  CONFIGURATION REGISTERS + direction pin
//
void AS5600::setDirection(uint8_t direction)
{
  _direction = direction;
  if (_directionPin != AS5600_SW_DIRECTION_PIN)
  {
    digitalWrite(_directionPin, _direction);
  }
}


uint8_t AS5600::getDirection()
{
  if (_directionPin != AS5600_SW_DIRECTION_PIN)
  {
    _direction = digitalRead(_directionPin);
  }
  return _direction;
}


uint8_t AS5600::getZMCO()
{
  uint8_t value = readReg(AS5600_ZMCO);
  return value;
}


bool AS5600::setZPosition(uint16_t value)
{
  if (value > 0x0FFF) return false;
  writeReg2(AS5600_ZPOS, value);
  return true;
}


uint16_t AS5600::getZPosition()
{
  uint16_t value = readReg2(AS5600_ZPOS) & 0x0FFF;
  return value;
}


bool AS5600::setMPosition(uint16_t value)
{
  if (value > 0x0FFF) return false;
  writeReg2(AS5600_MPOS, value);
  return true;
}


uint16_t AS5600::getMPosition()
{
  uint16_t value = readReg2(AS5600_MPOS) & 0x0FFF;
  return value;
}


bool AS5600::setMaxAngle(uint16_t value)
{
  if (value > 0x0FFF) return false;
  writeReg2(AS5600_MANG, value);
  return true;
}


uint16_t AS5600::getMaxAngle()
{
  uint16_t value = readReg2(AS5600_MANG) & 0x0FFF;
  return value;
}


/////////////////////////////////////////////////////////
//
//  CONFIGURATION
//
bool AS5600::setConfigure(uint16_t value)
{
  if (value > 0x3FFF) return false;
  writeReg2(AS5600_CONF, value);
  return true;
}


uint16_t AS5600::getConfigure()
{
  uint16_t value = readReg2(AS5600_CONF) & 0x3FFF;
  return value;
}


//  details configure
bool AS5600::setPowerMode(uint8_t powerMode)
{
  if (powerMode > 3) return false;
  uint8_t value = readReg(AS5600_CONF + 1);
  value &= ~AS5600_CONF_POWER_MODE;
  value |= powerMode;
  writeReg(AS5600_CONF + 1, value);
  return true;
}


uint8_t AS5600::getPowerMode()
{
  return readReg(AS5600_CONF + 1) & 0x03;
}


bool AS5600::setHysteresis(uint8_t hysteresis)
{
  if (hysteresis > 3) return false;
  uint8_t value = readReg(AS5600_CONF + 1);
  value &= ~AS5600_CONF_HYSTERESIS;
  value |= (hysteresis << 2);
  writeReg(AS5600_CONF + 1, value);
  return true;
}


uint8_t AS5600::getHysteresis()
{
  return (readReg(AS5600_CONF + 1) >> 2) & 0x03;
}


bool AS5600::setOutputMode(uint8_t outputMode)
{
  if (outputMode > 2) return false;
  uint8_t value = readReg(AS5600_CONF + 1);
  value &= ~AS5600_CONF_OUTPUT_MODE;
  value |= (outputMode << 4);
  writeReg(AS5600_CONF + 1, value);
  return true;
}


uint8_t AS5600::getOutputMode()
{
  return (readReg(AS5600_CONF + 1) >> 4) & 0x03;
}


bool AS5600::setPWMFrequency(uint8_t pwmFreq)
{
  if (pwmFreq > 3) return false;
  uint8_t value = readReg(AS5600_CONF + 1);
  value &= ~AS5600_CONF_PWM_FREQUENCY;
  value |= (pwmFreq << 6);
  writeReg(AS5600_CONF + 1, value);
  return true;
}


uint8_t AS5600::getPWMFrequency()
{
  return (readReg(AS5600_CONF + 1) >> 6) & 0x03;
}


bool AS5600::setSlowFilter(uint8_t mask)
{
  if (mask > 3) return false;
  uint8_t value = readReg(AS5600_CONF);
  value &= ~AS5600_CONF_SLOW_FILTER;
  value |= mask;
  writeReg(AS5600_CONF, value);
  return true;
}


uint8_t AS5600::getSlowFilter()
{
  return readReg(AS5600_CONF) & 0x03;
}


bool AS5600::setFastFilter(uint8_t mask)
{
  if (mask > 7) return false;
  uint8_t value = readReg(AS5600_CONF);
  value &= ~AS5600_CONF_FAST_FILTER;
  value |= (mask << 2);
  writeReg(AS5600_CONF, value);
  return true;
}


uint8_t AS5600::getFastFilter()
{
  return (readReg(AS5600_CONF) >> 2) & 0x07;
}


bool AS5600::setWatchDog(uint8_t mask)
{
  if (mask > 1) return false;
  uint8_t value = readReg(AS5600_CONF);
  value &= ~AS5600_CONF_WATCH_DOG;
  value |= (mask << 5);
  writeReg(AS5600_CONF, value);
  return true;
}


uint8_t AS5600::getWatchDog()
{
  return (readReg(AS5600_CONF) >> 5) & 0x01;
}


/////////////////////////////////////////////////////////
//
//  OUTPUT REGISTERS
//
uint16_t AS5600::rawAngle()
{
  int16_t value = readReg2(AS5600_RAW_ANGLE) & 0x0FFF;
  if (_offset > 0) value = (value + _offset) & 0x0FFF;

  if ((_directionPin == AS5600_SW_DIRECTION_PIN) &&
      (_direction == AS5600_COUNTERCLOCK_WISE))
  {
    value = (4096 - value) & 0x0FFF;
  }
  return value;
}


uint16_t AS5600::readAngle()
{
  uint16_t value = readReg2(AS5600_ANGLE) & 0x0FFF;
  if (_offset > 0) value = (value + _offset) & 0x0FFF;

  if ((_directionPin == AS5600_SW_DIRECTION_PIN) &&
      (_direction == AS5600_COUNTERCLOCK_WISE))
  {
    value = (4096 - value) & 0x0FFF;
  }
  return value;
}


bool AS5600::setOffset(float degrees)
{
  //  expect loss of precision.
  if (abs(degrees) > 36000) return false;
  bool neg = (degrees < 0);
  if (neg) degrees = -degrees;

  uint16_t offset = round(degrees * AS5600_DEGREES_TO_RAW);
  offset &= 4095;
  if (neg) offset = 4096 - offset;
  _offset = offset;
  return true;
}


float AS5600::getOffset()
{
  return _offset * AS5600_RAW_TO_DEGREES;
}


bool AS5600::increaseOffset(float degrees)
{
  //  add offset to existing offset in degrees.
  return setOffset((_offset * AS5600_RAW_TO_DEGREES) + degrees);
}


/////////////////////////////////////////////////////////
//
//  STATUS REGISTERS
//
uint8_t AS5600::readStatus()
{
  uint8_t value = readReg(AS5600_STATUS);
  return value;
}


uint8_t AS5600::readAGC()
{
  uint8_t value = readReg(AS5600_AGC);
  return value;
}


uint16_t AS5600::readMagnitude()
{
  uint16_t value = readReg2(AS5600_MAGNITUDE) & 0x0FFF;
  return value;
}


bool AS5600::detectMagnet()
{
  return (readStatus() & AS5600_MAGNET_DETECT) > 0;
}


bool AS5600::magnetTooStrong()
{
  return (readStatus() & AS5600_MAGNET_HIGH) > 0;
}


bool AS5600::magnetTooWeak()
{
  return (readStatus() & AS5600_MAGNET_LOW) > 0;
}


/////////////////////////////////////////////////////////
//
//  BURN COMMANDS
//
//  DO NOT UNCOMMENT - USE AT OWN RISK - READ DATASHEET
//
//  void AS5600::burnAngle()
//  {
//    writeReg(AS5600_BURN, x0x80);
//  }
//
//
//  See https://github.com/RobTillaart/AS5600/issues/38
//  void AS5600::burnSetting()
//  {
//    writeReg(AS5600_BURN, 0x40);
//    delay(5);
//    writeReg(AS5600_BURN, 0x01);
//    writeReg(AS5600_BURN, 0x11);
//    writeReg(AS5600_BURN, 0x10);
//    delay(5);
//  }


float AS5600::getAngularSpeed(uint8_t mode)
{
  uint32_t now     = micros();
  int      angle   = readAngle();
  uint32_t deltaT  = now - _lastMeasurement;
  int      deltaA  = angle - _lastAngle;

  //  assumption is that there is no more than 180° rotation
  //  between two consecutive measurements.
  //  => at least two measurements per rotation (preferred 4).
  if (deltaA >  2048) deltaA -= 4096;
  if (deltaA < -2048) deltaA += 4096;
  float    speed   = (deltaA * 1e6) / deltaT;

  //  remember last time & angle
  _lastMeasurement = now;
  _lastAngle       = angle;

  //  return radians, RPM or degrees.
  if (mode == AS5600_MODE_RADIANS)
  {
    return speed * AS5600_RAW_TO_RADIANS;
  }
  if (mode == AS5600_MODE_RPM)
  {
    return speed * AS5600_RAW_TO_RPM;
  }
  //  default return degrees
  return speed * AS5600_RAW_TO_DEGREES;
}


/////////////////////////////////////////////////////////
//
//  POSITION cumulative
//
int32_t AS5600::getCumulativePosition()
{
  int16_t value = readReg2(AS5600_ANGLE) & 0x0FFF;
  if (_error != AS5600_OK) return _position;

  //  whole rotation CW?
  //  less than half a circle
  if ((_lastPosition > 2048) && ( value < (_lastPosition - 2048)))
  {
    _position = _position + 4096 - _lastPosition + value;
  }
  //  whole rotation CCW?
  //  less than half a circle
  else if ((value > 2048) && ( _lastPosition < (value - 2048)))
  {
    _position = _position - 4096 - _lastPosition + value;
  }
  else _position = _position - _lastPosition + value;
  _lastPosition = value;

  return _position;
}


int32_t AS5600::getRevolutions()
{
  int32_t p = _position >> 12;  //  divide by 4096
  return p;
  // if (p < 0) p++;
  // return p;
}


int32_t AS5600::resetPosition(int32_t position)
{
  int32_t old = _position;
  _position = position;
  return old;
}


int32_t AS5600::resetCumulativePosition(int32_t position)
{
  _lastPosition = readReg2(AS5600_RAW_ANGLE) & 0x0FFF;
  int32_t old = _position;
  _position = position;
  return old;
}


int AS5600::lastError()
{
  int value = _error;
  _error = AS5600_OK;
  return value;
}


/////////////////////////////////////////////////////////
//
//  PROTECTED AS5600
//
uint8_t AS5600::readReg(uint8_t reg)
{
  _error = AS5600_OK;
  _wire->beginTransmission(_address);
  _wire->write(reg);
  if (_wire->endTransmission() != 0)
  {
    _error = AS5600_ERROR_I2C_READ_0;
    return 0;
  }
  uint8_t n = _wire->requestFrom(_address, (uint8_t)1);
  if (n != 1)
  {
    _error = AS5600_ERROR_I2C_READ_1;
    return 0;
  }
  uint8_t _data = _wire->read();
  return _data;
}


uint16_t AS5600::readReg2(uint8_t reg)
{
  _error = AS5600_OK;
  _wire->beginTransmission(_address);
  _wire->write(reg);
  if (_wire->endTransmission() != 0)
  {
    _error = AS5600_ERROR_I2C_READ_2;
    return 0;
  }
  uint8_t n = _wire->requestFrom(_address, (uint8_t)2);
  if (n != 2)
  {
    _error = AS5600_ERROR_I2C_READ_3;
    return 0;
  }
  uint16_t _data = _wire->read();
  _data <<= 8;
  _data += _wire->read();
  return _data;
}


uint8_t AS5600::writeReg(uint8_t reg, uint8_t value)
{
  _error = AS5600_OK;
  _wire->beginTransmission(_address);
  _wire->write(reg);
  _wire->write(value);
  if (_wire->endTransmission() != 0)
  {
    _error = AS5600_ERROR_I2C_WRITE_0;
  }
  return _error;
}


uint8_t AS5600::writeReg2(uint8_t reg, uint16_t value)
{
  _error = AS5600_OK;
  _wire->beginTransmission(_address);
  _wire->write(reg);
  _wire->write(value >> 8);
  _wire->write(value & 0xFF);
  if (_wire->endTransmission() != 0)
  {
    _error = AS5600_ERROR_I2C_WRITE_0;
  }
  return _error;
}


/////////////////////////////////////////////////////////////////////////////
//
//  AS5600L
//
AS5600L::AS5600L(uint8_t address, TwoWire *wire) : AS5600(wire)
{
  _address = address;;   //  0x40 = default address AS5600L.
}


bool AS5600L::setAddress(uint8_t address)
{
  //  skip reserved I2C addresses
  if ((address < 8) || (address > 119)) return false;

  //  note address need to be shifted 1 bit.
  writeReg(AS5600_I2CADDR, address << 1);
  writeReg(AS5600_I2CUPDT, address << 1);

  //  remember new address.
  _address = address;
  return true;
}


bool AS5600L::setI2CUPDT(uint8_t address)
{
  //  skip reserved I2C addresses
  if ((address < 8) || (address > 119)) return false;
  writeReg(AS5600_I2CUPDT, address << 1);
  return true;
}


uint8_t AS5600L::getI2CUPDT()
{
  return (readReg(AS5600_I2CUPDT) >> 1);
}


//  -- END OF FILE --


