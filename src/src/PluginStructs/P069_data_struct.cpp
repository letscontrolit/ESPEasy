#include "P069_data_struct.h"

#ifdef USES_P069

#include "../Globals/I2Cdev.h"


# define LM75A_BASE_ADDRESS        0x48
# define LM75A_DEGREES_RESOLUTION  0.125f
# define LM75A_REG_ADDR_TEMP     0


P069_data_struct::P069_data_struct(bool A0_value, bool A1_value, bool A2_value)
{
  _i2c_device_address = LM75A_BASE_ADDRESS;

  if (A0_value) {
    _i2c_device_address += 1;
  }

  if (A1_value) {
    _i2c_device_address += 2;
  }

  if (A2_value) {
    _i2c_device_address += 4;
  }
}

P069_data_struct::P069_data_struct(uint8_t addr)
{
  _i2c_device_address = addr;
}

void P069_data_struct::setAddress(uint8_t addr)
{
  _i2c_device_address = addr;
}

float P069_data_struct::getTemperatureInDegrees() const
{
  float   real_result = NAN;
  int16_t value       = 0;

  // Go to temperature data register
  Wire.beginTransmission(_i2c_device_address);
  Wire.write(LM75A_REG_ADDR_TEMP);

  if (Wire.endTransmission())
  {
    // Transmission error
    return real_result;
  }

  // Get content
  Wire.requestFrom(_i2c_device_address, (uint8_t)2);

  if (Wire.available() == 2)
  {
    value = (Wire.read() << 8) | Wire.read();
  }
  else
  {
    // Can't read temperature
    return real_result;
  }

  // Shift data (left-aligned)
  value >>= 5;

  // Relocate negative bit (11th bit to 16th bit)
  if (value & 0x0400) // negative?
  {
    value |= 0xFC00;  // expand to 16 bit
  }

  // Real value can be calculated with sensor resolution
  real_result = (float)value * LM75A_DEGREES_RESOLUTION;

  return real_result;
}

#endif // ifdef USES_P069
