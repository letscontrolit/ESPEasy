#include "../PluginStructs/P025_data_struct.h"

#ifdef USES_P025

P025_data_struct::P025_data_struct(uint8_t i2c_addr, uint8_t _pga, uint8_t _mux) : pga(_pga), mux(_mux), i2cAddress(i2c_addr) {}

int16_t P025_data_struct::read() {
  uint16_t config = (0x0003)    | // Disable the comparator (default val)
                    (0x0000)    | // Non-latching (default val)
                    (0x0000)    | // Alert/Rdy active low   (default val)
                    (0x0000)    | // Traditional comparator (default val)
                    (0x0080)    | // 128 samples per second (default)
                    (0x0100);     // Single-shot mode (default)

  config |= static_cast<uint16_t>(pga) << 9;
  config |= static_cast<uint16_t>(mux) << 12;
  config |= (0x8000); // Start a single conversion

  Wire.beginTransmission(i2cAddress);
  Wire.write((uint8_t)(0x01));   // access to config register
  Wire.write((uint8_t)(config >> 8));
  Wire.write((uint8_t)(config & 0xFF));
  Wire.endTransmission();

  while (isReady025()==false) delay(1);
//  delay(9); // See https://github.com/letscontrolit/ESPEasy/issues/3159#issuecomment-660546091
  return readConversionRegister025();
}

uint8_t P025_data_struct::getMux()
{
  return (mux);
}

uint16_t P025_data_struct::readConversionRegister025() {
  uint16_t wConversionRegister=0x0000;

  Wire.beginTransmission(i2cAddress);
  Wire.write((0x00));    // access to conversion register
  Wire.endTransmission();


  if (Wire.requestFrom(i2cAddress, (uint8_t)2) == 2) {
    wConversionRegister=Wire.read() << 8;
    wConversionRegister+=Wire.read();
  }
  return wConversionRegister;
}



bool P025_data_struct::isReady025() {
  uint16_t wConfigReg=0x0000;

  Wire.beginTransmission(i2cAddress);
  Wire.write((0x01));    // access to conversion register
  Wire.endTransmission();

  if (Wire.requestFrom(i2cAddress, (uint8_t)2) == 2) {
    wConfigReg=(Wire.read() << 8);
    wConfigReg+=Wire.read();
    if ((wConfigReg & 0x8000)==0x8000)  // bit15=0 performing a conversion   =1 not performing a conversion
      return true;
    else
      return false;
  }
  else
    return false;
}

#endif // ifdef USES_P025
