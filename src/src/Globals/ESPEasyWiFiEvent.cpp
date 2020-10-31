#include "../Globals/ESPEasyWiFiEvent.h"

#include "../../ESPEasy_common.h"




#ifdef ESP8266
WiFiEventHandler stationConnectedHandler;
WiFiEventHandler stationDisconnectedHandler;
WiFiEventHandler stationGotIpHandler;
WiFiEventHandler stationModeDHCPTimeoutHandler;
WiFiEventHandler APModeStationConnectedHandler;
WiFiEventHandler APModeStationDisconnectedHandler;
#endif // ifdef ESP8266

WiFiEventData_t WiFiEventData;