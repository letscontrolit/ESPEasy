#include "../Globals/ESPEasyWiFiEvent.h"

#include "../../ESPEasy_common.h"


#ifdef ESP8266
WiFiEventHandler stationConnectedHandler;
WiFiEventHandler stationDisconnectedHandler;
WiFiEventHandler stationGotIpHandler;
WiFiEventHandler stationModeDHCPTimeoutHandler;
WiFiEventHandler stationModeAuthModeChangeHandler;
WiFiEventHandler APModeStationConnectedHandler;
WiFiEventHandler APModeStationDisconnectedHandler;
#ifdef USES_ESPEASY_NOW
WiFiEventHandler APModeProbeRequestReceivedHandler;

std::list<WiFiEventSoftAPModeProbeRequestReceived> APModeProbeRequestReceived_list;
#endif
#endif // ifdef ESP8266

#ifdef ESP32
#ifdef USES_ESPEASY_NOW
std::list<system_event_ap_probe_req_rx_t> APModeProbeRequestReceived_list;
#endif
#endif

WiFiEventData_t WiFiEventData;
