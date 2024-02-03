#ifndef DATASTRUCTS_FACTORYDEFAULT_NETWORK_NVS_H
#define DATASTRUCTS_FACTORYDEFAULT_NETWORK_NVS_H

#include "../../ESPEasy_common.h"

#ifdef ESP32

# include "../Helpers/ESPEasy_NVS_Helper.h"

class FactoryDefault_Network_NVS {
public:

  bool applyToSettings_from_NVS(ESPEasy_NVS_Helper& preferences);

  void fromSettings_to_NVS(ESPEasy_NVS_Helper& preferences);

  void clear_from_NVS(ESPEasy_NVS_Helper& preferences);

private:

  // IP
  // GW
  // SN
  // DNS
  uint8_t IP_data[16]{};

# ifdef FEATURE_ETHERNET

  union {
    struct {
      int8_t  ETH_Phy_Addr;
      int8_t  ETH_Pin_mdc_cs;
      int8_t  ETH_Pin_mdio_irq;
      int8_t  ETH_Pin_power_rst;
      uint8_t ETH_Phy_Type;
      uint8_t ETH_Clock_Mode;
      uint8_t NetworkMedium;
      uint8_t unused;
    } bits;

    uint64_t ETH_HW_conf{};
  };


# endif // ifdef FEATURE_ETHERNET
};

#endif  // ifdef ESP32
#endif  // ifndef DATASTRUCTS_FACTORYDEFAULT_NETWORK_NVS_H
