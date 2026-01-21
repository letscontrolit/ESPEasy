#pragma once

#include "../../../ESPEasy_common.h"

#ifdef ESP32
# include "../DataStructs/WiFi_AP_Candidate.h"

# if FEATURE_WIFI

namespace ESPEasy {
namespace net {
namespace wifi {


class WiFi_AP_Candidates_NVS
{
public:

  static bool loadCandidate_from_NVS(WiFi_AP_Candidate& candidate);

  static void currentConnection_to_NVS(const WiFi_AP_Candidate& candidate);

  static void clear_from_NVS();
};

} // namespace wifi
} // namespace net
} // namespace ESPEasy

# endif // if FEATURE_WIFI

#endif // ifdef ESP32
