#ifndef ESPEASYCORE_ESPEASYWIFI_PROCESSEVENT_H
#define ESPEASYCORE_ESPEASYWIFI_PROCESSEVENT_H

#include "../../ESPEasy_common.h"

void handle_unprocessedNetworkEvents();
void processDisconnect();
void processConnect();
void processGotIP();
#if FEATURE_USE_IPV6
void processGotIPv6();
#endif
void processDisconnectAPmode();
void processConnectAPmode();
void processDisableAPmode();
void processScanDone();

#endif // ifndef ESPEASYCORE_ESPEASYWIFI_PROCESSEVENT_H
