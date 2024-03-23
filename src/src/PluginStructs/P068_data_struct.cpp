#include "../PluginStructs/P068_data_struct.h"

#ifdef USES_P068

# include "../Helpers/CRC_functions.h"

// ==============================================
// P068_SHT3X LIBRARY - SHT3X.cpp
// =============================================
P068_SHT3X::P068_SHT3X(uint8_t addr) : _i2c_device_address(addr)
{
  // Set to periodic mode
  I2C_write8_reg(
    _i2c_device_address,
    0x20, // periodic 0.5mps
    0x32  // repeatability high
    );
}

void P068_SHT3X::readFromSensor()
{
  I2C_write8_reg(
    _i2c_device_address,
    0xE0, // fetch data command
    0x00
    );

  // FIXME TD-er: Currently the I2Cdev::readBytes does not support writing 2 bytes before reading.
  Wire.requestFrom(_i2c_device_address, (uint8_t)6);

  if (Wire.available() == 6)
  {
    uint16_t data[6];

    for (uint8_t i = 0; i < 6u; ++i) {
      data[i] = Wire.read();
    }

    // TODO: check CRC (data[2] and data[5])
    if (calc_CRC8(data[0], data[1], data[2]) &&
        calc_CRC8(data[3], data[4], data[5]))
    {
      tmp = ((((data[0] << 8) | data[1]) * 175.0f) / 65535.0f) - 45.0f;
      hum = ((((data[3] << 8) | data[4]) * 100.0f) / 65535.0f);

      // Humidity temperature compensation borrowed from P028 BME280
      if (!essentiallyZero(tmpOff)) {
        const float last_dew_temp_val = compute_dew_point_temp(tmp + (tmpOff / 2.0f), hum);
        tmp += tmpOff;
        hum  = compute_humidity_from_dewpoint(tmp, last_dew_temp_val);
      }
    }
  }
  else
  {
    tmp = NAN;
    hum = NAN;

    // Set to periodic mode
    I2C_write8_reg(
      _i2c_device_address,
      0x20, // periodic 0.5mps
      0x32  // repeatability high
      );
  }
}

#endif // ifdef USES_P068
