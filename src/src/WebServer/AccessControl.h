#ifndef WEBSERVER_WEBSERVER_ACCESSCONTROL_H
#define WEBSERVER_WEBSERVER_ACCESSCONTROL_H

#include "../WebServer/common.h"

#include <IPAddress.h>

// ********************************************************************************
// Allowed IP range check
// ********************************************************************************
bool clientIPinSubnetDefaultNetwork();

bool clientIPallowed();

void clearAccessBlock();

// ********************************************************************************
// Add a IP Access Control select dropdown list
// ********************************************************************************
void addIPaccessControlSelect(const String& name, int choice);


#endif