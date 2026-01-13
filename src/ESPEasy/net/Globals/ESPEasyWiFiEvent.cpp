#include "../Globals/ESPEasyWiFiEvent.h"

#include "../../../ESPEasy_common.h"

#if FEATURE_WIFI

# ifdef ESP8266
WiFiEventHandler stationConnectedHandler;
WiFiEventHandler stationDisconnectedHandler;
WiFiEventHandler stationGotIpHandler;
WiFiEventHandler stationModeDHCPTimeoutHandler;
WiFiEventHandler stationModeAuthModeChangeHandler;
WiFiEventHandler APModeStationConnectedHandler;
WiFiEventHandler APModeStationDisconnectedHandler;
# endif // ifdef ESP8266


#endif // if FEATURE_WIFI
