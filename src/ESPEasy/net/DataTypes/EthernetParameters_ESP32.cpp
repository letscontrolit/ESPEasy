#include "../DataTypes/EthernetParameters.h"


#if CONFIG_IDF_TARGET_ESP32 && FEATURE_ETHERNET

namespace ESPEasy {
namespace net {

bool isValid(EthClockMode_t clockMode) {
  switch (clockMode)
  {
    case  EthClockMode_t::Ext_crystal_osc:
    case  EthClockMode_t::Int_50MHz_GPIO_0:
    case  EthClockMode_t::Int_50MHz_GPIO_16:
    case  EthClockMode_t::Int_50MHz_GPIO_17_inv:
      return true;

      // Do not use default: as this allows the compiler to detect any missing cases.
  }
  return false;
}

bool isGpioUsedInETHClockMode(EthClockMode_t clockMode,
                              int8_t         gpio) {
  if (((clockMode == EthClockMode_t::Int_50MHz_GPIO_0)      && (gpio == 0)) ||
      ((clockMode == EthClockMode_t::Int_50MHz_GPIO_16)     && (gpio == 16)) ||
      ((clockMode == EthClockMode_t::Int_50MHz_GPIO_17_inv) && (gpio == 17))) {
    return true;
  }
  return false;
}

const __FlashStringHelper* toString(EthClockMode_t clockMode) {
  switch (clockMode)
  {
    case  EthClockMode_t::Ext_crystal_osc:       return F("External crystal oscillator");
    case  EthClockMode_t::Int_50MHz_GPIO_0:      return F("50MHz APLL Output on GPIO0");
    case  EthClockMode_t::Int_50MHz_GPIO_16:     return F("50MHz APLL Output on GPIO16");
    case  EthClockMode_t::Int_50MHz_GPIO_17_inv: return F("50MHz APLL Inverted Output on GPIO17");

      // Do not use default: as this allows the compiler to detect any missing cases.
  }
  return F("Unknown");
}

} // namespace net
} // namespace ESPEasy

#endif // if CONFIG_IDF_TARGET_ESP32
