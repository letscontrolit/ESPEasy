#ifndef DATASTRUCTS_ETHERNETPARAMETERS_H
#define DATASTRUCTS_ETHERNETPARAMETERS_H

#include "../../ESPEasy_common.h"


// Is stored in settings
enum class EthClockMode_t : uint8_t {
  Ext_crystal_osc       = 0,
  Int_50MHz_GPIO_0      = 1,
  Int_50MHz_GPIO_16     = 2,
  Int_50MHz_GPIO_17_inv = 3
};

bool   isValid(EthClockMode_t clockMode);

const __FlashStringHelper * toString(EthClockMode_t clockMode);

bool isGpioUsedInETHClockMode(EthClockMode_t clockMode,
                              int8_t gpio);

// Is stored in settings
enum class EthPhyType_t : uint8_t {
#if CONFIG_ETH_USE_ESP32_EMAC
  LAN8720 = 0,
  TLK110  = 1,
#if ESP_IDF_VERSION_MAJOR > 3

  RTL8201 = 2,
#if ETH_TYPE_JL1101_SUPPORTED
  JL1101 = 3, 
#endif
  DP83848 = 4, 
  KSZ8041 = 5, 
  KSZ8081 = 6,
#endif
#endif
#if ESP_IDF_VERSION_MAJOR >= 5
#if CONFIG_ETH_SPI_ETHERNET_DM9051
  DM9051 = 10, 
#endif
#if CONFIG_ETH_SPI_ETHERNET_W5500
  W5500 = 11, 
#endif
#if CONFIG_ETH_SPI_ETHERNET_KSZ8851SNL
  KSZ8851 = 12,
#endif
#endif
  notSet = 127   // Might be processed in some code as int, uint8_t and int8_t
};

bool   isValid(EthPhyType_t phyType);

#if FEATURE_ETHERNET
#include <ETH.h>

bool isSPI_EthernetType(EthPhyType_t phyType);

// Convert to internal enum type as those enum values may not always be the same int value
eth_phy_type_t to_ESP_phy_type(EthPhyType_t phyType);
#endif

const __FlashStringHelper * toString(EthPhyType_t phyType);


#endif // DATASTRUCTS_ETHERNETPARAMETERS_H
