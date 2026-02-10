#ifndef DATASTRUCTS_GPIOFACTORYSETTINGSSTRUCT_H
#define DATASTRUCTS_GPIOFACTORYSETTINGSSTRUCT_H

#include "../DataTypes/DeviceModel.h"
#include "../../ESPEasy/net/DataTypes/NetworkMedium.h"
#if FEATURE_ETHERNET
#include "../../ESPEasy/net/DataTypes/EthernetParameters.h"
#endif

struct GpioFactorySettingsStruct {
  GpioFactorySettingsStruct(DeviceModel model = DeviceModel::DeviceModel_default);

  int8_t          button[4];
  int8_t          relais[4];
  int8_t          status_led;
  int8_t          i2c_sda;
  int8_t          i2c_scl;
  int8_t          eth_phyaddr;
#if FEATURE_ETHERNET
  ESPEasy::net::EthPhyType_t    eth_phytype;
#else
  uint8_t         eth_phytype{};
#endif
  int8_t          eth_mdc;
  int8_t          eth_mdio;
  int8_t          eth_power;
#if FEATURE_ETHERNET && CONFIG_ETH_USE_ESP32_EMAC
  ESPEasy::net::EthClockMode_t  eth_clock_mode;
#else
  uint8_t  eth_clock_mode{};
#endif
  ESPEasy::net::NetworkMedium_t network_medium;
};


#endif // DATASTRUCTS_GPIOFACTORYSETTINGSSTRUCT_H
