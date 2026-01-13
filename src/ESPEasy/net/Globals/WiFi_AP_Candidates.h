#pragma once

#include "../Helpers/WiFi_AP_CandidatesList.h"

#if FEATURE_WIFI

namespace ESPEasy {
namespace net {
namespace wifi {


extern WiFi_AP_CandidatesList WiFi_AP_Candidates;

} // namespace wifi
} // namespace net
} // namespace ESPEasy

#endif // if FEATURE_WIFI
