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
                              int8_t         gpio) {
  if (((clockMode == EthClockMode_t::Int_50MHz_GPIO_0)      && (gpio == 0)) ||
      ((clockMode == EthClockMode_t::Int_50MHz_GPIO_16)     && (gpio == 16)) ||
      ((clockMode == EthClockMode_t::Int_50MHz_GPIO_17_inv) && (gpio == 17))) {
    return true;
  }
  return false;
}

const __FlashStringHelper* toString(EthClockMode_t clockMode) {
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
#if CONFIG_ETH_USE_ESP32_EMAC
    case EthPhyType_t::LAN8720:
    case EthPhyType_t::TLK110:
# if ESP_IDF_VERSION_MAJOR > 3
    case EthPhyType_t::RTL8201:
#if ETH_TYPE_JL1101_SUPPORTED
    case EthPhyType_t::JL1101:
#endif
    case EthPhyType_t::DP83848:
    case EthPhyType_t::KSZ8041:
    case EthPhyType_t::KSZ8081:
# endif // if ESP_IDF_VERSION_MAJOR > 3
      return true;
#endif // if CONFIG_ETH_USE_ESP32_EMAC

#if ESP_IDF_VERSION_MAJOR >= 5
# if CONFIG_ETH_SPI_ETHERNET_DM9051
    case EthPhyType_t::DM9051: return true;
# endif // if CONFIG_ETH_SPI_ETHERNET_DM9051
# if CONFIG_ETH_SPI_ETHERNET_W5500
    case EthPhyType_t::W5500:  return true;
# endif // if CONFIG_ETH_SPI_ETHERNET_W5500
# if CONFIG_ETH_SPI_ETHERNET_KSZ8851SNL
    case EthPhyType_t::KSZ8851: return true;
# endif // if CONFIG_ETH_SPI_ETHERNET_KSZ8851SNL
#endif // if ESP_IDF_VERSION_MAJOR >= 5
    case EthPhyType_t::notSet:
      break;
  }
  return false;
}

#if FEATURE_ETHERNET
bool isSPI_EthernetType(EthPhyType_t phyType) {
# if ESP_IDF_VERSION_MAJOR >= 5
  return
#  if CONFIG_ETH_SPI_ETHERNET_DM9051
    phyType ==  EthPhyType_t::DM9051 ||
#  endif // if CONFIG_ETH_SPI_ETHERNET_DM9051
#  if CONFIG_ETH_SPI_ETHERNET_W5500
    phyType ==  EthPhyType_t::W5500 ||
#  endif // if CONFIG_ETH_SPI_ETHERNET_W5500
#  if CONFIG_ETH_SPI_ETHERNET_KSZ8851SNL
    phyType ==  EthPhyType_t::KSZ8851 ||
#  endif // if CONFIG_ETH_SPI_ETHERNET_KSZ8851SNL
    false;
# else // if ESP_IDF_VERSION_MAJOR >= 5
  return false;
# endif // if ESP_IDF_VERSION_MAJOR >= 5
}

eth_phy_type_t to_ESP_phy_type(EthPhyType_t phyType)
{
  switch (phyType) {
# if CONFIG_ETH_USE_ESP32_EMAC
    case EthPhyType_t::LAN8720:  return ETH_PHY_LAN8720;
    case EthPhyType_t::TLK110:   return ETH_PHY_TLK110;
#  if ESP_IDF_VERSION_MAJOR > 3
    case EthPhyType_t::RTL8201:  return ETH_PHY_RTL8201;
#   if ETH_TYPE_JL1101_SUPPORTED
    case EthPhyType_t::JL1101:   return ETH_PHY_JL1101;
#   endif
    case EthPhyType_t::DP83848:  return ETH_PHY_DP83848;
    case EthPhyType_t::KSZ8041:  return ETH_PHY_KSZ8041;
    case EthPhyType_t::KSZ8081:  return ETH_PHY_KSZ8081;
#  endif // if ESP_IDF_VERSION_MAJOR > 3
# endif // if CONFIG_ETH_USE_ESP32_EMAC

# if ESP_IDF_VERSION_MAJOR >= 5
#  if CONFIG_ETH_SPI_ETHERNET_DM9051
    case EthPhyType_t::DM9051:   return ETH_PHY_DM9051;
#  endif // if CONFIG_ETH_SPI_ETHERNET_DM9051
#  if CONFIG_ETH_SPI_ETHERNET_W5500
    case EthPhyType_t::W5500:   return ETH_PHY_W5500;
#  endif // if CONFIG_ETH_SPI_ETHERNET_W5500
#  if CONFIG_ETH_SPI_ETHERNET_KSZ8851SNL
    case EthPhyType_t::KSZ8851:   return ETH_PHY_KSZ8851;
#  endif // if CONFIG_ETH_SPI_ETHERNET_KSZ8851SNL
# endif // if ESP_IDF_VERSION_MAJOR >= 5
    case EthPhyType_t::notSet:
      break;
  }
  return ETH_PHY_MAX;
}

#endif // if FEATURE_ETHERNET


const __FlashStringHelper* toString(EthPhyType_t phyType) {
  switch (phyType) {
#if CONFIG_ETH_USE_ESP32_EMAC
    case EthPhyType_t::LAN8720:  return F("LAN8710/LAN8720");
    case EthPhyType_t::TLK110:   return F("TLK110");
# if ESP_IDF_VERSION_MAJOR > 3
    case EthPhyType_t::RTL8201:  return F("RTL8201");
#if ETH_TYPE_JL1101_SUPPORTED
    case EthPhyType_t::JL1101:   return F("JL1101");
#endif
    case EthPhyType_t::DP83848:  return F("DP83848");
    case EthPhyType_t::KSZ8041:  return F("KSZ8041");
    case EthPhyType_t::KSZ8081:  return F("KSZ8081");
# endif // if ESP_IDF_VERSION_MAJOR > 3
#endif // if CONFIG_ETH_USE_ESP32_EMAC

#if ESP_IDF_VERSION_MAJOR >= 5
# if CONFIG_ETH_SPI_ETHERNET_DM9051
    case EthPhyType_t::DM9051:   return F("DM9051(SPI)");
# endif // if CONFIG_ETH_SPI_ETHERNET_DM9051
# if CONFIG_ETH_SPI_ETHERNET_W5500
    case EthPhyType_t::W5500:   return F("W5500(SPI)");
# endif // if CONFIG_ETH_SPI_ETHERNET_W5500
# if CONFIG_ETH_SPI_ETHERNET_KSZ8851SNL
    case EthPhyType_t::KSZ8851:   return F("KSZ8851(SPI)");
# endif // if CONFIG_ETH_SPI_ETHERNET_KSZ8851SNL
#endif // if ESP_IDF_VERSION_MAJOR >= 5
    case EthPhyType_t::notSet:
      break;

      // Do not use default: as this allows the compiler to detect any missing cases.
  }
  return F("- None -");
}
