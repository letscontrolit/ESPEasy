#pragma once

#include "../../../ESPEasy_common.h"

#if FEATURE_WIFI

namespace ESPEasy {
namespace net {
namespace wifi {


enum class STA_connected_state {
  Idle,
  Connecting,
  Connected,
  Error_Not_Found,
  Error_Connect_Failed

};


} // namespace wifi
} // namespace net
} // namespace ESPEasy

#endif // if FEATURE_WIFI
