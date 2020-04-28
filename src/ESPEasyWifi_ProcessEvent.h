#ifndef ESPEASYWIFI_PROCESSEVENT_H
#define ESPEASYWIFI_PROCESSEVENT_H

bool unprocessedWifiEvents();
void handle_unprocessedWiFiEvents();
void processDisconnect();
void processConnect();
void processGotIP();
void processDisconnectAPmode();
void processConnectAPmode();
void processDisableAPmode();
void processScanDone();
void markWiFi_services_initialized();

#endif //ESPEASYWIFI_PROCESSEVENT_H