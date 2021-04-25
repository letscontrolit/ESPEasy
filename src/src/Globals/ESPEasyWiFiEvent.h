#ifndef GLOBALS_ESPEASYWIFIEVENT_H
#define GLOBALS_ESPEASYWIFIEVENT_H

#include <Arduino.h>
#include <IPAddress.h>
#include <stdint.h>

#include "../../ESPEasy_common.h"

#include "../DataStructs/WiFiEventData.h"
#ifdef HAS_ETHERNET
#include "../DataStructs/EthernetEventData.h"
#endif


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
#endif // ifdef ESP8266


extern WiFiEventData_t WiFiEventData;

#ifdef HAS_ETHERNET
extern EthernetEventData_t EthEventData;
#endif

#ifdef ESP32
extern WiFiEventId_t wm_event_id;
#endif // ifdef ESP32


#endif // GLOBALS_ESPEASYWIFIEVENT_H
