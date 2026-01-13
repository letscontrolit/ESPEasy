#pragma once

#include "../../../ESPEasy_common.h"

#include "../DataTypes/NetworkIndex.h"

#include <vector>

namespace ESPEasy {
namespace net {


/*********************************************************************************************\
* NetworkDriverStruct
\*********************************************************************************************/
struct NetworkDriverStruct
{
  NetworkDriverStruct() = default;

  bool           onlySingleInstance    = true;
  bool           alwaysPresent         = false;
  bool           enabledOnFactoryReset = false;
  networkIndex_t fixedNetworkIndex     = INVALID_NETWORK_INDEX;


};

} // namespace net
} // namespace ESPEasy
