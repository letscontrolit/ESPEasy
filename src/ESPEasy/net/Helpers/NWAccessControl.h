#pragma once

#include <IPAddress.h>

namespace ESPEasy {
namespace net {

// ********************************************************************************
// Allowed IP range check
// ********************************************************************************
#define ALL_ALLOWED            0
#define LOCAL_SUBNET_ALLOWED   1
#define ONLY_IP_RANGE_ALLOWED  2

bool ipInAllowedSubnet(const IPAddress& ip);

String describeAllowedIPrange();

bool getIPallowedRange(IPAddress& low, IPAddress& high);


} // namespace net
} // namespace ESPEasy
