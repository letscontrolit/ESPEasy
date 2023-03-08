#include "../PluginStructs/P025_data_struct.h"

#ifdef USES_P025

# define P025_CONVERSION_REGISTER  0x00
# define P025_CONFIG_REGISTER      0x01

P025_data_struct::P025_data_struct(uint8_t i2c_addr, uint8_t _pga, uint8_t _mux) :
  pga(_pga), mux(_mux), i2cAddress(i2c_addr) {}

int16_t P025_data_struct::read() const {
  uint16_t config = (0x0003)    | // Disable the comparator (default val)
                    (0x0000)    | // Non-latching (default val)
                    (0x0000)    | // Alert/Rdy active low   (default val)
                    (0x0000)    | // Traditional comparator (default val)
                    (0x0080)    | // 128 samples per second (default)
                    (0x0100);     // Single-shot mode (default)

  config |= static_cast<uint16_t>(pga) << 9;
  config |= static_cast<uint16_t>(mux) << 12;
  config |= (0x8000); // Start a single conversion

  I2C_write16_reg(i2cAddress, P025_CONFIG_REGISTER, config);


  // See https://github.com/letscontrolit/ESPEasy/issues/3159#issuecomment-660546091
  const unsigned long timeout = millis() + 10;

  while (!timeOutReached(timeout) && !isReady025()) {
    delay(1);
  }

  // Conversion register represents 2-complement format.
  return (int16_t)readConversionRegister025();
}

uint16_t P025_data_struct::readConversionRegister025() const {
  bool is_ok                         = false;
  const uint16_t wConversionRegister = I2C_read16_reg(i2cAddress, P025_CONVERSION_REGISTER, &is_ok);

  if (!is_ok) { return 0u; }
  return wConversionRegister;
}

bool P025_data_struct::isReady025() const {
  bool is_ok                = false;
  const uint16_t wConfigReg = I2C_read16_reg(i2cAddress, P025_CONFIG_REGISTER, &is_ok);

  if (!is_ok) { return false; }

  // bit15=0 performing a conversion   =1 not performing a conversion
  return (wConfigReg & 0x8000) == 0x8000;
}

#endif // ifdef USES_P025
