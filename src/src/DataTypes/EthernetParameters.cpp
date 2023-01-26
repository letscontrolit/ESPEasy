#include "../DataTypes/EthernetParameters.h"

bool isValid(EthClockMode_t clockMode) {
  switch (clockMode) {
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
                              int8_t gpio) {
  if (((clockMode == EthClockMode_t::Int_50MHz_GPIO_0)      && (gpio == 0)) ||
      ((clockMode == EthClockMode_t::Int_50MHz_GPIO_16)     && (gpio == 16)) ||
      ((clockMode == EthClockMode_t::Int_50MHz_GPIO_17_inv) && (gpio == 17))) {
    return true;
  }
  return false;
}

const __FlashStringHelper * toString(EthClockMode_t clockMode) {
  switch (clockMode) {
    case  EthClockMode_t::Ext_crystal_osc:       return F("External crystal oscillator");
    case  EthClockMode_t::Int_50MHz_GPIO_0:      return F("50MHz APLL Output on GPIO0");
    case  EthClockMode_t::Int_50MHz_GPIO_16:     return F("50MHz APLL Output on GPIO16");
    case  EthClockMode_t::Int_50MHz_GPIO_17_inv: return F("50MHz APLL Inverted Output on GPIO17");

      // Do not use default: as this allows the compiler to detect any missing cases.
  }
  return F("Unknown");
}

bool isValid(EthPhyType_t phyType) {
  switch (phyType) {
    case EthPhyType_t::LAN8710:
    case EthPhyType_t::TLK110:
      return true;
    case EthPhyType_t::RTL8201:
    case EthPhyType_t::DP83848:
    case EthPhyType_t::DM9051:
    #if ESP_IDF_VERSION_MAJOR > 3
      return true; // FIXME TD-er: Must check if supported per IDF version
    #else 
      return false;
    #endif

      // Do not use default: as this allows the compiler to detect any missing cases.
  }
  return false;
}

const __FlashStringHelper * toString(EthPhyType_t phyType) {
  switch (phyType) {
    case EthPhyType_t::LAN8710: return F("LAN8710/LAN8720");
    case EthPhyType_t::TLK110:  return F("TLK110");
    case EthPhyType_t::RTL8201: return F("RTL8201");
    case EthPhyType_t::DP83848: return F("DP83848");
    case EthPhyType_t::DM9051:  return F("DM9051");

      // Do not use default: as this allows the compiler to detect any missing cases.
  }
  return F("Unknown");
}
