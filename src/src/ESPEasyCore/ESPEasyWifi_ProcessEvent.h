#ifndef ESPEASYCORE_ESPEASYWIFI_PROCESSEVENT_H
#define ESPEASYCORE_ESPEASYWIFI_PROCESSEVENT_H

#include "../../ESPEasy_common.h"

void handle_unprocessedNetworkEvents();
void processDisconnect();
void processConnect();
void processGotIP();
void processDisconnectAPmode();
void processConnectAPmode();
void processDisableAPmode();
void processScanDone();

#endif // ifndef ESPEASYCORE_ESPEASYWIFI_PROCESSEVENT_H
