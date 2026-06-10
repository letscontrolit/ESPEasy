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


enum class WiFi_STA_State_e
{
  // WiFi radio is off and no new attempt should be made (e.g. low power mode or Ethernet active)
  Disabled,

  // Error state, some action failed
  TimeOut,

  // State from where we decide to start scanning or connecting
  Idle,

  // Not allowed yet to continue (re)connecting.
  // For example when the interface is a fallback interface and not yet allowed to start
  // Or when in setup mode and no need to connect.
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

const __FlashStringHelper* toString(WiFi_STA_State_e state);


} // namespace wifi
} // namespace net
} // namespace ESPEasy
#endif // if FEATURE_WIFI
