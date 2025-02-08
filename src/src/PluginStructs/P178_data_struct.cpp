
#include "../PluginStructs/P178_data_struct.h"

#ifdef USES_P178

P178_data_struct::P178_data_struct(uint8_t address, uint16_t freq) : _address(address)
{
  // Reset
  I2C_write8_reg(_address, 0xFB, 0xFB);
  setFrequency(freq);
  update();
}

bool P178_data_struct::enablePin(uint8_t pin, bool enable)
{
  if (pin == LU9685_ALL_PINS) {
    constexpr uint32_t mask = LU9685_ALL_PIN_ENABLE_MASK;
    _enabledPin = enable ? mask : 0;
    return update();
  }

  if (pin >= LU9685_MAX_PINS) {
    return false;
  }
  bitWrite(_enabledPin, pin, enable);
  return setAngle(pin, _data[pin]);
}

bool P178_data_struct::setAngle(uint8_t pin, int angle)
{
  if ((pin != LU9685_ALL_PINS) && (pin >= LU9685_MAX_PINS)) {
    return false;
  }

  setAngle_internal(pin, angle);

  if (pin == LU9685_ALL_PINS) {
    return update();
  }
  return I2C_write8_reg(_address, pin, getAngle(pin));
}

bool P178_data_struct::setAngle(
  uint8_t pin,
  int     angle,
  bool    enable)
{
  if ((pin != LU9685_ALL_PINS) && (pin >= LU9685_MAX_PINS)) {
    return false;
  }

  if (pin == LU9685_ALL_PINS) {
    constexpr uint32_t mask = LU9685_ALL_PIN_ENABLE_MASK;
    _enabledPin = enable ? mask : 0;
  } else {
    bitWrite(_enabledPin, pin, enable);
  }
  return setAngle(pin, constrain(angle, 0, LU9685_MAX_ANGLE));
}

bool P178_data_struct::setRange(uint8_t startPin, uint32_t startVarIndex, int nrPins)
{
  uint8_t endPin = LU9685_MAX_PINS;

  if (startPin == LU9685_ALL_PINS) {
    startPin = 0;
    nrPins   = -1;
  }

  if (startPin < LU9685_MAX_PINS) {
    if ((nrPins > 0) && (nrPins < LU9685_MAX_PINS) && ((startPin + nrPins) < LU9685_MAX_PINS)) {
      endPin = startPin + nrPins;
    }

    for (uint8_t pin = startPin; pin < endPin; ++pin) {
      const int angle = getCustomFloatVar(startVarIndex, -1);
      setAngle_internal(pin, angle);
      ++startVarIndex;
    }
    return update();
  }
  return false;
}

bool P178_data_struct::update()
{
  // Write all 20 values in a single I2C call
  Wire.beginTransmission(_address);
  Wire.write(0xFD);

  for (uint8_t pin = 0; pin < LU9685_MAX_PINS; ++pin) {
    Wire.write(getAngle(pin));
  }
  return Wire.endTransmission() == 0;
}

void P178_data_struct::setFrequency(uint16_t freq)
{
  I2C_write16_reg(
    _address,
    0xFC,
    constrain(freq, LU9685_MIN_FREQUENCY, LU9685_MAX_FREQUENCY));
}

String P178_data_struct::logPrefix() {
  return concat(formatToHex(_address, F("LU9685 0x"), 2),  F(": "));
}

String P178_data_struct::logPrefix(const __FlashStringHelper *poststr)
{
  return concat(logPrefix(), poststr);
}

void P178_data_struct::setAngle_internal(uint8_t pin, int angle)
{
  if (pin < LU9685_MAX_PINS) {
    if ((angle < 0) || (angle > LU9685_MAX_ANGLE)) {
      bitClear(_enabledPin, pin);
    } else {
      // Do not change the enabled state here
      _data[pin] = angle;
    }
  } else if (pin == LU9685_ALL_PINS) {
    for (uint8_t i = 0; i < LU9685_MAX_PINS; ++i) {
      setAngle_internal(i, angle);
    }
  }
}

uint8_t P178_data_struct::getAngle(uint8_t pin) const
{
  if ((pin < LU9685_MAX_PINS) && bitRead(_enabledPin, pin)) {
    return constrain(_data[pin], 0, LU9685_MAX_ANGLE);
  }
  return LU9685_OUTPUT_DISABLED;
}

#endif // ifdef USES_P178
