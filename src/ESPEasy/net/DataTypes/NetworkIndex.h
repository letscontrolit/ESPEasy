#pragma once

#include "../../../ESPEasy_common.h"

#define NETWORK_INDEX_WIFI_STA  0 // Always the first network index
#define NETWORK_INDEX_WIFI_AP   1 // Always the 2nd network index


namespace ESPEasy {
namespace net {

typedef uint8_t networkIndex_t;
typedef uint16_t networkStatsVarIndex_t;


extern const networkIndex_t INVALID_NETWORK_INDEX;
extern const networkStatsVarIndex_t INVALID_NETWORK_STATS_VAR_INDEX;

} // namespace net
} // namespace ESPEasy
