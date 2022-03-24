#ifndef DATASTRUCTS_ETHERNETPARAMETERS_H
#define DATASTRUCTS_ETHERNETPARAMETERS_H

#include <Arduino.h>

// Is stored in settings
enum class EthClockMode_t : uint8_t {
  Ext_crystal_osc       = 0,
  Int_50MHz_GPIO_0      = 1,
  Int_50MHz_GPIO_16     = 2,
  Int_50MHz_GPIO_17_inv = 3
};

bool   isValid(EthClockMode_t clockMode);

const __FlashStringHelper * toString(EthClockMode_t clockMode);


// Is stored in settings
enum class EthPhyType_t : uint8_t {
  LAN8710 = 0,
  TLK110  = 1
};

bool   isValid(EthPhyType_t phyType);

const __FlashStringHelper * toString(EthPhyType_t phyType);


#endif // DATASTRUCTS_ETHERNETPARAMETERS_H
