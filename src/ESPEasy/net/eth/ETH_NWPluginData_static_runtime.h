#pragma once

#include "../../../ESPEasy_common.h"

#if FEATURE_ETHERNET
# include "../DataStructs/NWPluginData_static_runtime.h"

# include <ETH.h>

namespace ESPEasy {
namespace net {
namespace eth {


class ETH_NWPluginData_static_runtime
{
public:

  static ETHClass* getInterface(networkIndex_t networkIndex);

  static NWPluginData_static_runtime* getNWPluginData_static_runtime(networkIndex_t networkIndex);

  static ETHClass* init(networkIndex_t networkIndex);

  static void exit(networkIndex_t networkIndex);

private:

  static void onEvent(arduino_event_id_t   event,
                      arduino_event_info_t info);



}; // class ETH_NWPluginData_static_runtime

} // namespace eth
} // namespace net
} // namespace ESPEasy

#endif // if FEATURE_ETHERNET
