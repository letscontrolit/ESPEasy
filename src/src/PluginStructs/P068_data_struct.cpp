#include "../PluginStructs/P068_data_struct.h"

#ifdef USES_P068

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

    data[0] = Wire.read();
    data[1] = Wire.read();
    data[2] = Wire.read();
    data[3] = Wire.read();
    data[4] = Wire.read();
    data[5] = Wire.read();

    // TODO: check CRC (data[2] and data[5])
    if (CRC8(data[0], data[1], data[2]) &&
        CRC8(data[3], data[4], data[5]))
    {
      tmp = ((((data[0] << 8) | data[1]) * 175.0f) / 65535.0f) - 45.0f;
      hum = ((((data[3] << 8) | data[4]) * 100.0f) / 65535.0f);

      // Humidity temperature compensation borrowed from P028 BME280
      if (!essentiallyZero(tmpOff)) {
        float last_dew_temp_val = compute_dew_point_temp(tmp + (tmpOff / 2.0f), hum);
        hum = compute_humidity_from_dewpoint(tmp + tmpOff, last_dew_temp_val);
        tmp = tmp + tmpOff;
      }
    }
  }
  else
  {
    tmp = NAN;
    hum = NAN;

    // Set to periodic mode
    Wire.beginTransmission(_i2c_device_address);
    Wire.write(0x20); // periodic 0.5mps
    Wire.write(0x32); // repeatability high
    Wire.endTransmission();
  }
}

// FIXME TD-er: Try to make some collection of used CRC algorithms
// See http://reveng.sourceforge.net/crc-catalogue/1-15.htm#crc.cat.crc-8-dvb-s2
bool P068_SHT3X::CRC8(uint8_t MSB, uint8_t LSB, uint8_t CRC)
{
  /*
   *	Name           : CRC-8
   * Polynomial     : 0x31 (x8 + x5 + x4 + 1)
   * Initialization : 0xFF
   * Reflect input  : False
   * Reflect output : False
   * Final          : XOR 0x00
   *	Example        : CRC8( 0xBE, 0xEF, 0x92) should be true
   */
  uint8_t crc = 0xFF;

  for (uint8_t bytenr = 0; bytenr < 2; ++bytenr) {
    crc ^= (bytenr == 0) ? MSB : LSB;

    for (uint8_t i = 0; i < 8; ++i) {
      crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
    }
  }
  return crc == CRC;
}

#endif // ifdef USES_P068
