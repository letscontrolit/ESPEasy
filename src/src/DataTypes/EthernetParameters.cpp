#include "EthernetParameters.h"

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

      // Do not use default: as this allows the compiler to detect any missing cases.
  }
  return false;
}

const __FlashStringHelper * toString(EthPhyType_t phyType) {
  switch (phyType) {
    case EthPhyType_t::LAN8710: return F("LAN8710");
    case EthPhyType_t::TLK110:  return F("TLK110");

      // Do not use default: as this allows the compiler to detect any missing cases.
  }
  return F("Unknown");
}
