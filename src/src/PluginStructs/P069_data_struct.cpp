#include "../PluginStructs/P069_data_struct.h"

#ifdef USES_P069

#include "../Globals/I2Cdev.h"


# define LM75A_BASE_ADDRESS        0x48
# define LM75A_DEGREES_RESOLUTION  0.125f
# define LM75A_REG_ADDR_TEMP     0


P069_data_struct::P069_data_struct(uint8_t addr) :
  _i2c_device_address(addr) {}

float P069_data_struct::getTemperatureInDegrees() const
{
  // Go to temperature data register
  bool success = false;
  int16_t value = I2C_readS16_reg(_i2c_device_address, LM75A_REG_ADDR_TEMP, &success);
  if (!success)
  {
    // Transmission error
    return NAN;
  }

  // Shift data (left-aligned)
  value >>= 5;

  // Relocate negative bit (11th bit to 16th bit)
  if (value & 0x0400) // negative?
  {
    value |= 0xFC00;  // expand to 16 bit
  }

  // Real value can be calculated with sensor resolution
  const float real_result = static_cast<float>(value) * LM75A_DEGREES_RESOLUTION;

  return real_result;
}

#endif // ifdef USES_P069
