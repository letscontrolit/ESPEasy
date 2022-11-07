#ifndef ESPEASY_ETH_EVENT_H
#define ESPEASY_ETH_EVENT_H

#include "../../ESPEasy_common.h"


// ********************************************************************************
// Functions called on Eth events.
// Make sure not to call anything in these functions that result in delay() or yield()
// ********************************************************************************
#ifdef ESP32
 #if ESP_IDF_VERSION_MAJOR > 3
  void EthEvent(WiFiEvent_t event, arduino_event_info_t info);
 #endif
#endif


#endif // ESPEASY_ETH_EVENT_H