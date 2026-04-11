#pragma once

#include "../../../ESPEasy_common.h"

#if FEATURE_ETHERNET

# include <ETH.h>

namespace ESPEasy {
namespace net {


// Is stored in settings
# ifdef ESP32P4

// Clock pin can only be GPIO 32 / 44 / 50 (default)
enum class EthClockMode_t : uint8_t {
  Default     = 0,
  Ext_crystal = 1,
  Int_50MHz   = 2,

  /*
     Ext_crystal_GPIO_32   = (1 | (32 << 2)),
     Ext_crystal_GPIO_44   = (1 | (44 << 2)),
     Ext_crystal_GPIO_50   = (1 | (50 << 2)),
     Int_50MHz_GPIO_32     = (2 | (32 << 2)),
     Int_50MHz_GPIO_44     = (2 | (44 << 2)),
     Int_50MHz_GPIO_50     = (2 | (50 << 2)),
   */

};

# endif // ifdef ESP32P4
# if CONFIG_IDF_TARGET_ESP32
enum class EthClockMode_t : uint8_t {
  Ext_crystal_osc       = 0,
  Int_50MHz_GPIO_0      = 1,
  Int_50MHz_GPIO_16     = 2,
  Int_50MHz_GPIO_17_inv = 3

};

# endif // if CONFIG_IDF_TARGET_ESP32

# if CONFIG_ETH_USE_ESP32_EMAC && FEATURE_ETHERNET

bool                       isValid(EthClockMode_t clockMode);

const __FlashStringHelper* toString(EthClockMode_t clockMode);

bool                       isGpioUsedInETHClockMode(EthClockMode_t clockMode,
                                                    int8_t         gpio);
# endif // if CONFIG_ETH_USE_ESP32_EMAC && FEATURE_ETHERNET

// Is stored in settings
enum class EthPhyType_t : uint8_t {
# if CONFIG_ETH_USE_ESP32_EMAC && FEATURE_ETHERNET
  LAN8720 = 0,
  TLK110  = 1,
  RTL8201 = 2,
#  if ETH_TYPE_JL1101_SUPPORTED
  JL1101 = 3,
#  endif
  DP83848 = 4,
  KSZ8041 = 5,
  KSZ8081 = 6,
# if ETH_PHY_LAN867X_SUPPORTED
  LAN867X = 7,
# endif
# endif // if CONFIG_ETH_USE_ESP32_EMAC && FEATURE_ETHERNET
# if ESP_IDF_VERSION_MAJOR >= 5
#  if CONFIG_ETH_SPI_ETHERNET_DM9051
  DM9051 = 10,
#  endif
#  if CONFIG_ETH_SPI_ETHERNET_W5500
  W5500 = 11,
#  endif
#  if CONFIG_ETH_SPI_ETHERNET_KSZ8851SNL
  KSZ8851 = 12,
#  endif
# endif // if ESP_IDF_VERSION_MAJOR >= 5
  notSet = 127 // Might be processed in some code as int, uint8_t and int8_t

};

bool                       isValid(EthPhyType_t phyType);

bool                       isSPI_EthernetType(EthPhyType_t phyType);
bool                       isRMII_EthernetType(EthPhyType_t phyType);

// Convert to internal enum type as those enum values may not always be the same int value
eth_phy_type_t             to_ESP_phy_type(EthPhyType_t phyType);

const __FlashStringHelper* toString(EthPhyType_t phyType);

} // namespace net
} // namespace ESPEasy

#endif // if FEATURE_ETHERNET
