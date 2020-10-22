#ifndef WEBSERVER_WEBSERVER_ACCESSCONTROL_H
#define WEBSERVER_WEBSERVER_ACCESSCONTROL_H

#include "../WebServer/common.h"

#include <IPAddress.h>

// ********************************************************************************
// Allowed IP range check
// ********************************************************************************
#define ALL_ALLOWED            0
#define LOCAL_SUBNET_ALLOWED   1
#define ONLY_IP_RANGE_ALLOWED  2

boolean ipLessEqual(const IPAddress& ip, const IPAddress& high);

boolean ipInRange(const IPAddress& ip, const IPAddress& low, const IPAddress& high);

String describeAllowedIPrange();

bool getIPallowedRange(IPAddress& low, IPAddress& high);

bool clientIPinSubnet();

boolean clientIPallowed();

void clearAccessBlock();

// ********************************************************************************
// Add a IP Access Control select dropdown list
// ********************************************************************************
void addIPaccessControlSelect(const String& name, int choice);


#endif