#include "../Commands/WiFi.h"

#include "../../ESPEasy_common.h"

#include "../Commands/Common.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

#include "../ESPEasyCore/ESPEasyWifi.h"
#include "../ESPEasyCore/Serial.h"

#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/RTC.h"
#include "../Globals/Settings.h"

#include "../Helpers/StringConverter.h"


#define WIFI_MODE_MAX (WiFiMode_t)4


String Command_Wifi_SSID(struct EventStruct *event, const char *Line)
{
  return Command_GetORSetString(event, F("Wifi SSID:"),
                                Line,
                                SecuritySettings.WifiSSID,
                                sizeof(SecuritySettings.WifiSSID),
                                1);
}

String Command_Wifi_Key(struct EventStruct *event, const char *Line)
{
  return Command_GetORSetString(event, F("Wifi Key:"),
                                Line,
                                SecuritySettings.WifiKey,
                                sizeof(SecuritySettings.WifiKey),
                                1);
}

String Command_Wifi_SSID2(struct EventStruct *event, const char *Line)
{
  return Command_GetORSetString(event, F("Wifi2 SSID:"),
                                Line,
                                SecuritySettings.WifiSSID2,
                                sizeof(SecuritySettings.WifiSSID2),
                                1);
}

String Command_Wifi_Key2(struct EventStruct *event, const char *Line)
{
  return Command_GetORSetString(event, F("Wifi2 Key:"),
                                Line,
                                SecuritySettings.WifiKey2,
                                sizeof(SecuritySettings.WifiKey2),
                                1);
}

const __FlashStringHelper * Command_Wifi_Scan(struct EventStruct *event, const char *Line)
{
  WiFiScan_log_to_serial();
  return return_command_success();
}

const __FlashStringHelper * Command_Wifi_Connect(struct EventStruct *event, const char *Line)
{
  WiFiEventData.wifiConnectAttemptNeeded = true;
  return return_command_success();
}

const __FlashStringHelper * Command_Wifi_Disconnect(struct EventStruct *event, const char *Line)
{
  RTC.clearLastWiFi(); // Force a WiFi scan
  WifiDisconnect();

  return return_command_success();
}

const __FlashStringHelper * Command_Wifi_APMode(struct EventStruct *event, const char *Line)
{
  setAP(true);
  return return_command_success();
}

const __FlashStringHelper * Command_Wifi_STAMode(struct EventStruct *event, const char *Line)
{
  setSTA(true);
  return return_command_success();
}

String Command_Wifi_Mode(struct EventStruct *event, const char *Line)
{
  String TmpStr1;

  if (GetArgv(Line, TmpStr1, 2)) {
    WiFiMode_t mode = WIFI_MODE_MAX;

    if (event->Par1 > 0 && event->Par1 < WIFI_MODE_MAX) {
      mode = static_cast<WiFiMode_t>(event->Par1 - 1);
    } else {
      TmpStr1.toLowerCase();

      if (strcmp_P(TmpStr1.c_str(), PSTR("off")) == 0) { mode = WIFI_OFF; }
      else if (strcmp_P(TmpStr1.c_str(), PSTR("sta")) == 0) { mode = WIFI_STA; }
      else if (strcmp_P(TmpStr1.c_str(), PSTR("ap")) == 0) { mode = WIFI_AP; }
      else if (strcmp_P(TmpStr1.c_str(), PSTR("ap+sta")) == 0) { mode = WIFI_AP_STA; }
    }

    if ((mode >= WIFI_OFF) && (mode < WIFI_MODE_MAX)) {
      setWifiMode(mode);
    } else {
      return return_result(event, F("Wifi Mode: invalid arguments"));
    }
  } else {
    return return_result(event, concat(F("WiFi Mode:"),  getWifiModeString(WiFi.getMode())));
  }
  return return_command_success_str();
}

const __FlashStringHelper * Command_Wifi_AllowAP(struct EventStruct *event, const char* Line)
{
  Settings.DoNotStartAP(false);
  return return_command_success();
}

// FIXME: TD-er This is not an erase, but actually storing the current settings
// in the wifi settings of the core library
const __FlashStringHelper * Command_WiFi_Erase(struct EventStruct *event, const char *Line)
{
  WiFi.persistent(true);  // use SDK storage of SSID/WPA parameters
  WifiDisconnect();       // this will store empty ssid/wpa into sdk storage
  WiFi.persistent(false); // Do not use SDK storage of SSID/WPA parameters
  return return_command_success();
}
