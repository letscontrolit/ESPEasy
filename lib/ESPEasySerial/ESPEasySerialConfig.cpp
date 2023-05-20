#include "ESPEasySerialConfig.h"


#if USES_I2C_SC16IS752

bool ESPEasySerialConfig::getI2C_SC16IS752_Parameters(
  ESPEasySC16IS752_Serial::I2C_address      & addr,
  ESPEasySC16IS752_Serial::SC16IS752_channel& ch) const
{
  if ((receivePin >= 0x48) && (receivePin <= 0x57) && ((transmitPin >= 0) && (transmitPin < 2))) {
    addr = static_cast<ESPEasySC16IS752_Serial::I2C_address>(receivePin);
    ch   = static_cast<ESPEasySC16IS752_Serial::SC16IS752_channel>(transmitPin);
    return true;
  }
  return false;
}

#endif // if USES_I2C_SC16IS752
