#include "../PluginStructs/P060_data_struct.h"


#ifdef USES_P060

P060_data_struct::P060_data_struct(uint8_t i2c_addr) : i2cAddress(i2c_addr) {}

void P060_data_struct::overSampleRead()
{
  Oversampling.add(readMCP3221());
}

float P060_data_struct::getValue()
{
  float value{};
  if (!Oversampling.get(value)) {
    value = readMCP3221();
  }
  return value;
}

uint16_t P060_data_struct::readMCP3221()
{
  uint16_t value;

  Wire.requestFrom(i2cAddress, (uint8_t)2);

  if (Wire.available() == 2)
  {
    value = (Wire.read() << 8) | Wire.read();
  }
  else {
    value = 9999;
  }

  return value;
}

#endif // ifdef USES_P060
