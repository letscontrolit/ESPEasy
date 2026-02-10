#pragma once

#include "../../../ESPEasy_common.h"

#if FEATURE_WIFI


namespace ESPEasy {
namespace net {
namespace wifi {


/*

   State machine:

   Start at boot: Disabled

   Disabled:
   When AP or STA is needed -> OFF


   OFF:
   When STA is needed -> IdleWaiting


   IdleWaiting:
   After waiting period:
    If candidate present -> start STA_Connecting
    Else if AP enabled (and client connected to AP) -> Start STA_AP_Scanning
    Else -> start STA_Scanning

   STA_Scanning:
   When finished:
    If known candidate present -> IdleWaiting
    else if no known candidate present -> start AP_Fallback -> IdleWaiting

   STA_Connecting:
   When failed -> STA_Reconnecting
   Else -> STA_Connected

   STA_Reconnecting:
   When failed -> IdleWaiting
   Else -> STA_Connected


   STA_Connected:
   If disconnected -> OFF
   If connected for X minutes -> mark as stable


 */


enum class WiFiState_e
{
  // WiFi radio is off and no new attempt should be made (e.g. low power mode or Ethernet active)
  Disabled,

  // WiFi radio off, to continue, everything needs to be (re)initialized
  WiFiOFF,

  // Only running in AP mode
  // Typically this is only used when STA is off and AP Auto Start is checked
  // TODO TD-er: Must implement this.
  AP_only,

  // Fallback mode which is started when connecting to AP was not possible
  AP_Fallback,

  // WiFi was in some kind of error state or needs waiting period
  IdleWaiting,

  // STA mode + scanning
  STA_Scanning,

  // STA+AP mode + scanning,
  // needs some careful handling to prevent disconnecting the connected stations
  STA_AP_Scanning,

  // Connecting to an AP
  STA_Connecting,

  // Reconnecting to an AP
  // May need to handle some specific disconnect reasons differently from connecting for the first time.
  STA_Reconnecting,

  // Connected to an AP
  STA_Connected

};

const __FlashStringHelper* toString(WiFiState_e state);


} // namespace wifi
} // namespace net
} // namespace ESPEasy
#endif // if FEATURE_WIFI
