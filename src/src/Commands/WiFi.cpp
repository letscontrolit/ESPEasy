#include "../Commands/WiFi.h"

#include "../../ESPEasy_common.h"

#if FEATURE_WIFI

#include "../Commands/Common.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

#include "../ESPEasyCore/Serial.h"

#include "../../ESPEasy/net/Globals/ESPEasyWiFiEvent.h"
#include "../Globals/RTC.h"
#include "../Globals/SecuritySettings.h"
#include "../Globals/Settings.h"

#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/StringConverter.h"

#include "../../ESPEasy/net/wifi/ESPEasyWifi.h"



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

String Command_Wifi_HiddenSSID(struct EventStruct *event, const char *Line)
{
  bool   includeHiddenSSID = Settings.IncludeHiddenSSID();
  String result            = Command_GetORSetBool(event, F("Include Hidden SSID:"),
                                                  Line,
                                                  (bool *)&includeHiddenSSID,
                                                  1);

  if (Settings.IncludeHiddenSSID() != includeHiddenSSID) { // Update if changed
    Settings.IncludeHiddenSSID(includeHiddenSSID);
  }
  return result;
}

const __FlashStringHelper* Command_Wifi_Scan(struct EventStruct *event, const char *Line)
{
  ESPEasy::net::wifi::WiFiScan_log_to_serial();
  return return_command_success_flashstr();
}

const __FlashStringHelper* Command_Wifi_Connect(struct EventStruct *event, const char *Line)
{
//  WiFiEventData.wifiConnectAttemptNeeded = true;
  return return_command_success_flashstr();
}

const __FlashStringHelper* Command_Wifi_Disconnect(struct EventStruct *event, const char *Line)
{
  RTC.clearLastWiFi(); // Force a WiFi scan
  ESPEasy::net::wifi::WifiDisconnect();

  return return_command_success_flashstr();
}

const __FlashStringHelper* Command_Wifi_APMode(struct EventStruct *event, const char *Line)
{
   ESPEasy::net::wifi::setAPinternal(true);
  return return_command_success_flashstr();
}

const __FlashStringHelper* Command_Wifi_STAMode(struct EventStruct *event, const char *Line)
{
   ESPEasy::net::wifi::setSTA(true);
  return return_command_success_flashstr();
}

String Command_Wifi_Mode(struct EventStruct *event, const char *Line)
{
  String TmpStr1;

  if (GetArgv(Line, TmpStr1, 2)) {
    WiFiMode_t mode = WIFI_MODE_MAX;

    if ((event->Par1 > 0) && (event->Par1 < WIFI_MODE_MAX)) {
      mode = static_cast<WiFiMode_t>(event->Par1 - 1);
    } else {
      TmpStr1.toLowerCase();

      if (strcmp_P(TmpStr1.c_str(), PSTR("off")) == 0) { mode = WIFI_OFF; }
      else if (strcmp_P(TmpStr1.c_str(), PSTR("sta")) == 0) { mode = WIFI_STA; }
      else if (strcmp_P(TmpStr1.c_str(), PSTR("ap")) == 0) { mode = WIFI_AP; }
      else if (strcmp_P(TmpStr1.c_str(), PSTR("ap+sta")) == 0) { mode = WIFI_AP_STA; }
    }

    if ((mode >= WIFI_OFF) && (mode < WIFI_MODE_MAX)) {
      ESPEasy::net::wifi::setWifiMode(mode);
    } else {
      return return_result(event, F("Wifi Mode: invalid arguments"));
    }
  } else {
    return return_result(event, concat(F("WiFi Mode:"),  ESPEasy::net::wifi::getWifiModeString(WiFi.getMode())));
  }
  return return_command_success();
}

const __FlashStringHelper* Command_Wifi_AllowAP(struct EventStruct *event, const char *Line)
{
  Settings.DoNotStartAPfallback_ConnectFail(false);
  return return_command_success_flashstr();
}

// FIXME: TD-er This is not an erase, but actually storing the current settings
// in the wifi settings of the core library
const __FlashStringHelper* Command_WiFi_Erase(struct EventStruct *event, const char *Line)
{
  return Erase_WiFi_Calibration() 
    ? return_command_success_flashstr()
    : return_command_failed_flashstr();
}


#if FEATURE_OTA_FW_UPDATE_ESP_HOSTED_MCU
#  include "esp_hosted.h"

// Perform OTA update on the esp-hosted-mcu firmware of the external WiFi module (typically ESP32-C6)
// Latest builds are available here: 
// https://pioarduino.github.io/esp-hosted-mcu-firmware/
String Command_Wifi_OTA_hosted_mcu(
  struct EventStruct *event, const char *Line)
{

  String url;

  if (GetArgv(Line, url, 2)) {
    if (!url.startsWith(F("http://"))) {
      String str(concat(F("OTA update esp-hosted-mcu not HTTP url: "), url));
      addLog(LOG_LEVEL_ERROR, str);
      return str;
    }

    if (!url.endsWith(F("network_adapter.bin"))) {
      String str(concat(F("OTA update esp-hosted-mcu not correct file: "), url));
      addLog(LOG_LEVEL_ERROR, str);
      return str;
    }
    addLog(LOG_LEVEL_INFO, concat(F("OTA update esp-hosted-mcu: "), url));

// FIXME TD-er: Must implement new API. See https://github.com/espressif/esp-hosted-mcu/blob/main/examples/host_performs_slave_ota/main/main.c
// Current implementation does not work, so will disable for now...
/*
    esp_err_t err = esp_hosted_slave_ota(url.c_str());

    if (err != ESP_OK) {
      String str(strformat(F("Failed to start OTA update: %s"), esp_err_to_name(err)));
      addLog(LOG_LEVEL_ERROR, str);
      return str;
    }
*/
  }
  return return_command_success_flashstr();
}

# endif // ifdef ESP32P4


#endif