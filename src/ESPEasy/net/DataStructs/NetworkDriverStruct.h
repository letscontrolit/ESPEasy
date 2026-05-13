#pragma once

#include "../../../ESPEasy_common.h"

#include "../DataTypes/NetworkIndex.h"


namespace ESPEasy {
namespace net {


/*********************************************************************************************\
* NetworkDriverStruct
\*********************************************************************************************/
struct NetworkDriverStruct
{
  NetworkDriverStruct() {
    onlySingleInstance = 1;
  }

  union {
    struct {
      uint8_t onlySingleInstance                  : 1; // Default true
      uint8_t alwaysPresent                       : 1;
      uint8_t enabledOnFactoryReset               : 1;
      uint8_t _unusedNetworkDriverStructBits0_03  : 1;
      uint8_t _unusedNetworkDriverStructBits0_04  : 1;
      uint8_t _unusedNetworkDriverStructBits0_05  : 1;
      uint8_t _unusedNetworkDriverStructBits0_06  : 1;
      uint8_t _unusedNetworkDriverStructBits0_07  : 1;

    };

    uint8_t _networkDriverStructBits0{};

  };

  networkIndex_t fixedNetworkIndex = INVALID_NETWORK_INDEX;

  uint16_t _networkDriverStructPadding{};

};

} // namespace net
} // namespace ESPEasy
