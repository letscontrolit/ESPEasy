#ifndef GLOBALS_ESPEASYWIFIEVENT_H
#define GLOBALS_ESPEASYWIFIEVENT_H

#include "../../ESPEasy_common.h"

#include "../DataStructs/MAC_address.h"
#if FEATURE_ETHERNET
#include "../DataStructs/EthernetEventData.h"
#endif
#include "../DataStructs/WiFiEventData.h"


#include <Arduino.h>
#include <IPAddress.h>
#include <stdint.h>
#include <list>

#ifdef ESP32
# include <esp_event.h>
# include <WiFiGeneric.h>
# include <WiFiType.h>

#endif // ifdef ESP32

#ifdef ESP8266
# include <ESP8266WiFiGeneric.h>
# include <ESP8266WiFiType.h>
class IPAddress;

extern WiFiEventHandler stationConnectedHandler;
extern WiFiEventHandler stationDisconnectedHandler;
extern WiFiEventHandler stationGotIpHandler;
extern WiFiEventHandler stationModeDHCPTimeoutHandler;
extern WiFiEventHandler stationModeAuthModeChangeHandler;
extern WiFiEventHandler APModeStationConnectedHandler;
extern WiFiEventHandler APModeStationDisconnectedHandler;
#ifdef USES_ESPEASY_NOW
extern WiFiEventHandler APModeProbeRequestReceivedHandler;
extern std::list<WiFiEventSoftAPModeProbeRequestReceived> APModeProbeRequestReceived_list;
#endif
#endif // ifdef ESP8266

#ifdef ESP32
#ifdef USES_ESPEASY_NOW
extern std::list<system_event_ap_probe_req_rx_t> APModeProbeRequestReceived_list;
#endif
#endif

extern WiFiEventData_t WiFiEventData;


#endif // GLOBALS_ESPEASYWIFIEVENT_H
