#include "../PluginStructs/P025_data_struct.h"

#ifdef USES_P025

# define P025_CONVERSION_REGISTER  0x00
# define P025_CONFIG_REGISTER      0x01

P025_data_struct::P025_data_struct(uint8_t i2c_addr, uint8_t pga, uint8_t mux) :
  _i2cAddress(i2c_addr)
{
  constexpr uint16_t defaultValue =
    (0x0003)    | // Disable the comparator (default val)
    (0x0000)    | // Non-latching (default val)
    (0x0000)    | // Alert/Rdy active low   (default val)
    (0x0000)    | // Traditional comparator (default val)
    (0x0080)    | // 128 samples per second (default)
    (0x0100)    | // Single-shot mode (default)
    (0x8000);     // Start a single conversion

  _configRegisterValue = defaultValue |
                         (static_cast<uint16_t>(pga) << 9) |
                         (static_cast<uint16_t>(mux) << 12);
}

bool P025_data_struct::read(int16_t& value) const {
  if (!waitReady025(5)) { return false; }

  if (!I2C_write16_reg(_i2cAddress, P025_CONFIG_REGISTER, _configRegisterValue)) {
    return false;
  }

  // See https://github.com/letscontrolit/ESPEasy/issues/3159#issuecomment-660546091
  if (!waitReady025(10)) { return false; }

  return readConversionRegister025(value);
}

bool P025_data_struct::readConversionRegister025(int16_t& value) const {
  bool is_ok = false;

  // Conversion register represents 2-complement format.
  value = (int16_t)I2C_read16_reg(_i2cAddress, P025_CONVERSION_REGISTER, &is_ok);
  return is_ok;
}

bool P025_data_struct::waitReady025(unsigned long timeout_ms) const {
  const unsigned long timeout = millis() + timeout_ms;

  while (!timeOutReached(timeout)) {
    bool is_ok = false;

    // bit15=0 performing a conversion   =1 not performing a conversion
    const bool ready = (I2C_read16_reg(_i2cAddress, P025_CONFIG_REGISTER, &is_ok) & 0x8000) != 0;

    if (!is_ok) { return false; }

    if (ready) { return true; }
    delay(1);
  }
  return false;
}

#endif // ifdef USES_P025
