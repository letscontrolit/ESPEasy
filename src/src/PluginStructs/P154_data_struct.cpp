#include "../PluginStructs/P154_data_struct.h"

#if defined(USES_P154) || defined(USES_P172)

# define P154_BMP3_CHIP_ID     0x50
# define P154_BMP390_CHIP_ID   0x60


P154_data_struct::P154_data_struct(struct EventStruct *event) :
  i2cAddress(P154_I2C_ADDR),
  elevation(P154_ALTITUDE),
  csPin(PIN(0))
{}

bool P154_data_struct::begin(bool _i2cMode)
{
  i2cMode = _i2cMode;

  if (i2cMode && !bmp.begin_I2C(i2cAddress)) {
    return false;
  }

  if (!i2cMode && !bmp.begin_SPI(csPin)) {
    return false;
  }

  // Set up oversampling and filter initialization
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);

  // Perform a few reads, so the IIR filter does have a 'seed'
  for (int i = 0; i < 5; ++i) {
    bmp.performReading();
  }
  initialized = true;

  return true;
}

bool P154_data_struct::read(float& temp, float& pressure)
{
  if (!bmp.performReading()) {
    return false;
  }

  temp     = bmp.temperature;
  pressure = bmp.pressure / 100.0f; // hPa

  if (elevation != 0) {
    pressure = pressureElevation(pressure, elevation);
  }

  return true;
}

bool P154_data_struct::webformLoad(struct EventStruct *event,
                                   bool                _i2cMode)
{
  if (_i2cMode) {
    addRowLabel(F("Detected Sensor Type"));
    const uint32_t chipID = I2C_read8_reg(P154_I2C_ADDR, 0);

    if (chipID == P154_BMP3_CHIP_ID) {
      addHtml(F("BMP38x"));
    } else if (chipID == P154_BMP390_CHIP_ID) {
      addHtml(F("BMP390"));
    } else {
      addHtmlInt(chipID);
    }
  }

  addFormNumericBox(F("Altitude"), F("elev"), P154_ALTITUDE);
  addUnit('m');
  return true;
}

bool P154_data_struct::webformSave(struct EventStruct *event)
{
  P154_I2C_ADDR = getFormItemInt(F("i2c_addr"));
  P154_ALTITUDE = getFormItemInt(F("elev"));
  return true;
}

#endif // if defined(USES_P154) || defined(USES_P172)
