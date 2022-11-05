#include "../ESPEasyCore/ESPEasyEthEvent.h"

#ifdef ESP32
# if FEATURE_ETHERNET
#  include <ETH.h>

#include "../ESPEasyCore/ESPEasyEth.h"
#include "../Globals/ESPEasyEthEvent.h"


// ********************************************************************************
// Functions called on events.
// Make sure not to call anything in these functions that result in delay() or yield()
// ********************************************************************************

#  include <WiFi.h>


#  if ESP_IDF_VERSION_MAJOR > 3
void EthEvent(WiFiEvent_t event, arduino_event_info_t info) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:

      if (ethPrepare()) {
        addLog(LOG_LEVEL_INFO, F("ETH event: Started"));
      } else {
        addLog(LOG_LEVEL_ERROR, F("ETH event: Could not prepare ETH!"));
      }
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      addLog(LOG_LEVEL_INFO, F("ETH event: Connected"));
      EthEventData.markConnected();
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      EthEventData.markGotIP();
      addLog(LOG_LEVEL_INFO,  F("ETH event: Got IP"));
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      addLog(LOG_LEVEL_ERROR, F("ETH event: Disconnected"));
      EthEventData.markDisconnect();
      break;
    case ARDUINO_EVENT_ETH_STOP:
      addLog(LOG_LEVEL_INFO, F("ETH event: Stopped"));
      break;
    #   if ESP_IDF_VERSION_MAJOR > 3
    case ARDUINO_EVENT_ETH_GOT_IP6:
    #   else // if ESP_IDF_VERSION_MAJOR > 3
    case ARDUINO_EVENT_GOT_IP6:
    #   endif // if ESP_IDF_VERSION_MAJOR > 3
      addLog(LOG_LEVEL_INFO, F("ETH event: Got IP6"));
      break;
    default:
    {
      break;
    }
  }
}

#  endif // if ESP_IDF_VERSION_MAJOR > 3

# endif // if FEATURE_ETHERNET

#endif // ifdef ESP32
