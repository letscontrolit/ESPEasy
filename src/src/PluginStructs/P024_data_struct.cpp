#include "../PluginStructs/P024_data_struct.h"

#ifdef USES_P024

P024_data_struct::P024_data_struct(uint8_t i2c_addr) : i2cAddress(i2c_addr) {}

float P024_data_struct::readTemperature(uint8_t reg)
{
  float temp = readRegister024(reg);

  temp *= .02f;
  temp -= 273.15f;
  return temp;
}

uint16_t P024_data_struct::readRegister024(uint8_t reg) {
  uint16_t ret;

  Wire.beginTransmission(i2cAddress);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(i2cAddress, (uint8_t)3);
  ret  = Wire.read();      // receive DATA
  ret |= Wire.read() << 8; // receive DATA
  Wire.read();
  return ret;
}

#endif // ifdef USES_P024
