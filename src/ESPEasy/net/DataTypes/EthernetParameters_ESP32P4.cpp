#include "../DataTypes/EthernetParameters.h"


#if CONFIG_IDF_TARGET_ESP32P4 && FEATURE_ETHERNET

# include <pins_arduino.h>

namespace ESPEasy {
namespace net {

bool isValid(EthClockMode_t clockMode) {
  switch (clockMode)
  {
    case EthClockMode_t::Default:
    case EthClockMode_t::Ext_crystal:
    case EthClockMode_t::Int_50MHz:
      /*
         case EthClockMode_t::Ext_crystal_GPIO_32:
         case EthClockMode_t::Ext_crystal_GPIO_44:
         case EthClockMode_t::Ext_crystal_GPIO_50:
         case EthClockMode_t::Int_50MHz_GPIO_32:
         case EthClockMode_t::Int_50MHz_GPIO_44:
         case EthClockMode_t::Int_50MHz_GPIO_50:
       */
      return true;

      // Do not use default: as this allows the compiler to detect any missing cases.
  }
  return false;
}

bool isGpioUsedInETHClockMode(EthClockMode_t clockMode,
                              int8_t         gpio) {
  switch (clockMode)
  {
    case EthClockMode_t::Default:
    case EthClockMode_t::Ext_crystal:
    case EthClockMode_t::Int_50MHz:           return gpio == ETH_RMII_CLK;

      /*
         case EthClockMode_t::Int_50MHz_GPIO_32:
         case EthClockMode_t::Ext_crystal_GPIO_32: return (gpio == 32);
         case EthClockMode_t::Int_50MHz_GPIO_44:
         case EthClockMode_t::Ext_crystal_GPIO_44: return (gpio == 44);
         case EthClockMode_t::Int_50MHz_GPIO_50:
         case EthClockMode_t::Ext_crystal_GPIO_50: return (gpio == 50);
       */
  }
  return false;
}

const __FlashStringHelper* toString(EthClockMode_t clockMode) {
  switch (clockMode)
  {
    case EthClockMode_t::Default:             return F("default");
    case EthClockMode_t::Ext_crystal:         return F("External crystal oscillator");
    case EthClockMode_t::Int_50MHz:           return F("50MHz APLL Output");

      /*
         case EthClockMode_t::Ext_crystal_GPIO_32:  return F("External crystal oscillator on GPIO-32");
         case EthClockMode_t::Ext_crystal_GPIO_44:  return F("External crystal oscillator on GPIO-44");
         case EthClockMode_t::Ext_crystal_GPIO_50:  return F("External crystal oscillator on GPIO-50");
         case EthClockMode_t::Int_50MHz_GPIO_32:    return F("50MHz APLL Output on GPIO-32");
         case EthClockMode_t::Int_50MHz_GPIO_44:    return F("50MHz APLL Output on GPIO-44");
         case EthClockMode_t::Int_50MHz_GPIO_50:    return F("50MHz APLL Output on GPIO-50");
       */

      // Do not use default: as this allows the compiler to detect any missing cases.
  }
  return F("Unknown");
}

} // namespace net
} // namespace ESPEasy

#endif // if CONFIG_IDF_TARGET_ESP32P4
