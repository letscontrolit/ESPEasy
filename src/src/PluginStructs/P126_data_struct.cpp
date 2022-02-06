#include "../PluginStructs/P126_data_struct.h"

#ifdef USES_P126

#define P126_ADS1015_MUX_DIFF_01            (0x0000)
#define P126_ADS1015_MUX_DIFF_23            (0x3000)
#define P126_ADS1015_PGA_1_024V             (0x0600) //< +/-1.024V range = Gain 4
#define P126_ADS1015_CONFIG_CQUE_NONE       (0x0003) // Disable the comparator (default val)
#define P126_ADS1015_CONFIG_CLAT_NONLAT     (0x0000) // Non-latching (default val)
#define P126_ADS1015_CONFIG_CPOL_ACTVLOW    (0x0000) // Alert/Rdy active low   (default val)
#define P126_ADS1015_CONFIG_CMODE_TRAD      (0x0000) // Traditional comparator (default val)
#define P126_ADS1015_RATE_3300SPS           (0x00C0) // < 3300 samples per second
#define P126_ADS1015_CONV_MODE_SINGLE       (0x0100) // Single-shot mode (default)
#define P126_ADS1015_START_SINGLE_CONV      (0x8000) // Start a single conversion

P126_data_struct::P126_data_struct(uint8_t i2c_addr, uint8_t c1Cal, uint8_t c2Cal) : c1Calibre(c1Cal), c2Calibre(c2Cal), i2cAddress(i2c_addr) {}

bool P126_data_struct::read() {
  uint16_t config = P126_ADS1015_CONFIG_CQUE_NONE |
                    P126_ADS1015_CONFIG_CLAT_NONLAT |
                    P126_ADS1015_CONFIG_CPOL_ACTVLOW |
                    P126_ADS1015_CONFIG_CMODE_TRAD |
                    P126_ADS1015_RATE_3300SPS |
                    P126_ADS1015_CONV_MODE_SINGLE;

  config |= P126_ADS1015_PGA_1_024V;
  config |= P126_ADS1015_MUX_DIFF_01;
  config |= P126_ADS1015_START_SINGLE_CONV;

  Wire.beginTransmission(i2cAddress);
  Wire.write((uint8_t)(0x01));
  Wire.write((uint8_t)(config >> 8));
  Wire.write((uint8_t)(config & 0xFF));
  Wire.endTransmission();

  delay(9); // See https://github.com/letscontrolit/ESPEasy/issues/3159#issuecomment-660546091
  return readRegister126(0x00);
}

uint16_t P126_data_struct::readRegister126(uint8_t reg) {
  Wire.beginTransmission(i2cAddress);
  Wire.write((0x00));
  Wire.endTransmission();

  if (Wire.requestFrom(i2cAddress, (uint8_t)2) != 2) {
    return 0x8000;
  }
  return (Wire.read() << 8) | Wire.read();
}

float_t P126_data_struct::getCurrent(uint8_t canal)
{
  return 0.;
}

#endif // ifdef USES_P126
