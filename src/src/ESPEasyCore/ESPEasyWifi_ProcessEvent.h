#ifndef ESPEASYWIFI_PROCESSEVENT_H
#define ESPEASYWIFI_PROCESSEVENT_H

#include "../../ESPEasy_common.h"

void handle_unprocessedNetworkEvents();
void processDisconnect();
void processConnect();
void processGotIP();
void processDisconnectAPmode();
void processConnectAPmode();
void processDisableAPmode();
void processScanDone();
void markWiFi_services_initialized();

#ifdef HAS_ETHERNET
void processEthernetConnected();
void processEthernetDisconnected();
void processEthernetGotIP();
#endif

#endif //ESPEASYWIFI_PROCESSEVENT_H