#include "../PluginStructs/P154_data_struct.h"

#ifdef USES_P154

P154_data_struct::P154_data_struct(struct EventStruct *event) :
  i2cAddress(P154_I2C_ADDR)
{}

bool P154_data_struct::begin()
{
  if (!bmp.begin_I2C(i2cAddress)) {
    return false;
  }

  // Set up oversampling and filter initialization
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);

  return true;
}

bool P154_data_struct::read(float& temp, float& pressure)
{
  if (!bmp.performReading()) {
    return false;
  }

  temp     = bmp.temperature;
  pressure = bmp.pressure / 100.0f; // hPa

  return true;
}

bool P154_data_struct::webformSave(struct EventStruct *event)
{
  P154_I2C_ADDR = getFormItemInt(F("i2c_addr"));
  return true;
}

#endif // ifdef USES_P154
