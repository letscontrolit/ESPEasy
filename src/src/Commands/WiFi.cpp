#include "../Commands/WiFi.h"


#if FEATURE_WIFI

# include "../Commands/Common.h"

# include "../DataStructs/ESPEasy_EventStruct.h"

# include "../Globals/RTC.h"
# include "../Globals/ESPEasy_Scheduler.h"
# include "../Globals/SecuritySettings.h"
# include "../Globals/Settings.h"

# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/StringConverter.h"

# include "../../ESPEasy/net/wifi/ESPEasyWifi.h"
# include "../../ESPEasy/net/ESPEasyNetwork.h"


# define WIFI_MODE_MAX (WiFiMode_t)4

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
  if (!Settings.getNetworkEnabled(NETWORK_INDEX_WIFI_AP)) { return return_command_failed_flashstr(); }
  String TmpStr1;
  bool   APenable = true;

  // Command structure:
  // WifiAPMode               Start AP mode
  // WifiAPMode,<on|off|1|0>  Start/stop AP mode

  if (GetArgv(Line, TmpStr1, 2)) {
    TmpStr1.toLowerCase();

    if ((event->Par1 == 0) || (strcmp_P(TmpStr1.c_str(), PSTR("off")) == 0)) {
      APenable = false;
    } else if ((event->Par1 == 1) || (strcmp_P(TmpStr1.c_str(), PSTR("on")) == 0)) {
      APenable = true;
    } else {
      return return_incorrect_nr_arguments();
    }
  }

  if (APenable) {
    Scheduler.setNetworkInitTimer(10, NETWORK_INDEX_WIFI_AP);
  }
  else {
    Scheduler.setNetworkExitTimer(10, NETWORK_INDEX_WIFI_AP);
  }
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

# if FEATURE_OTA_FW_UPDATE_ESP_HOSTED_MCU
#  include "esp_hosted.h"
#  include "HttpClientLight.h"
#  include <WiFiClientSecureLightBearSSL.h>
#  include "../CustomBuild/Certificate_CA.h"
#  include "../Globals/ESPEasy_time.h"

// Perform OTA update on the esp-hosted-mcu firmware of the external WiFi module (typically ESP32-C6)
// Latest builds are available here:
// https://pioarduino.github.io/esp-hosted-mcu-firmware/
String Command_Wifi_OTA_hosted_mcu(
  struct EventStruct *event, const char *Line)
{

  bool updateSuccess = false;

  // Step 1: Verify ESP-Hosted is properly initialized
  if (!hostedIsInitialized()) {
    addLog(LOG_LEVEL_ERROR, F("ERROR: esp-hosted is not initialized. Did you call WiFi.STA.begin()?"));
    return return_command_failed_flashstr();
  }

  // Step 2: Check if an update is actually available
  if (!hostedHasUpdate()) {
    // esp-hosted is already the latest version - no update needed
    return return_command_failed_flashstr();
  }

  // Step 3: Ensure network connectivity is available
  if (!Network.isOnline()) {
    addLog(LOG_LEVEL_ERROR, F("ERROR: Network is not online! Did you call WiFi.STA.connect(ssid, password)?"));
    return return_command_failed_flashstr();
  }

  const String url = hostedGetUpdateURL();

  // Step 4: Begin the update process - display update URL
  addLog(LOG_LEVEL_INFO, concat(F("Updating esp-hosted co-processor from "), url));

  /*
  // Step 5: Create a secure network client for HTTPS communication
  BearSSL::WiFiClientSecure_light secureClient(4096, 4096);
  secureClient.setUtcTime_fcn(getUnixTime);
  secureClient.setCfgTime_fcn(get_build_unixtime);
  secureClient.setTrustAnchor(Tasmota_TA, Tasmota_TA_size);

  // Step 6: Configure client to skip certificate verification (insecure mode)
  secureClient.setInsecure();

  const String domain = url.substring(8, url.indexOf('/', 8));
  addLog(LOG_LEVEL_INFO, concat(F("Set domain hosting esp-hosted co-processor from "), domain));

  secureClient.setDomainName(domain.c_str());
  secureClient.setECDSA(true);
*/

  // Step 7: Initialize HTTP client and attempt to connect to update server
  HTTPClientLight https;
  https.setReuse(false);
  https.setAuthorization("");     // Clear Basic authorization
#  ifdef ESP32
//  https.setAuthorizationType(""); // Default type is "Basic"
#  endif

  int httpCode = 0;

  if (!https.begin(url, nullptr)) {
    //    auto tls_last_error = secureClient.getLastError();
    //    addLog(LOG_LEVEL_ERROR, strformat(F("ERROR: HTTP begin failed! Error-code: %d"), tls_last_error));
    addLog(LOG_LEVEL_ERROR, F("ERROR: HTTP begin failed!"));

    goto finish_ota;
  }

  // Step 8: Send HTTP GET request to download the firmware
  httpCode = https.GET();

  if (httpCode == HTTP_CODE_OK) {
    // Step 9: Get the size of the firmware file to download
    int len = https.getSize();

    if (len < 0) {
      addLog(LOG_LEVEL_ERROR, F("ERROR: Update size not received!"));
      https.end();
      goto finish_ota;
    }

    // Step 10: Get stream pointer for reading firmware data
    NetworkClient *stream = https.getStreamPtr();

    // Step 11: Initialize the ESP-Hosted update process
    addLog(LOG_LEVEL_INFO, F("Beginning update process..."));

    // FIXME TD-er: Block actual update to see how far we get with making connection
//          goto finish_ota;

    if (!hostedBeginUpdate()) {
      addLog(LOG_LEVEL_ERROR, F("ERROR: esp-hosted update start failed!"));
      https.end();
      goto finish_ota;
    }

    // Step 12: Allocate buffer for firmware data transfer (2KB chunks)
#  define HOSTED_OTA_BUF_SIZE 2048
    uint8_t *buff = (uint8_t *)malloc(HOSTED_OTA_BUF_SIZE);

    if (!buff) {
      addLog(LOG_LEVEL_ERROR, F("ERROR: Could not allocate OTA buffer!"));
      https.end();
      goto finish_ota;
    }

    // Step 13: Download and write firmware data in chunks
    while (https.connected() && len > 0) {
      size_t size = stream->available();

      if (size > 0) {
        // Show progress indicator
        Serial.print(".");

        // Limit chunk size to buffer capacity
        if (size > HOSTED_OTA_BUF_SIZE) {
          size = HOSTED_OTA_BUF_SIZE;
        }

        // Prevent reading more data than expected
        if (size > len) {
          addLog(LOG_LEVEL_ERROR, strformat(F("ERROR: Update received extra bytes: %lu!"), (unsigned long)size - len));
          break;
        }

        // Read firmware data chunk into buffer
        int readLen = stream->readBytes(buff, size);
        len -= readLen;

        // Write the chunk to ESP-Hosted co-processor
        if (!hostedWriteUpdate(buff, readLen)) {
          addLog(LOG_LEVEL_ERROR, F("ERROR: esp-hosted update write failed!"));
          break;
        }

        // Step 14: Check if entire firmware has been downloaded
        if (len == 0) {
          Serial.println();

          // Finalize the update process
          addLog(LOG_LEVEL_INFO, F("Finalizing update process..."));

          if (!hostedEndUpdate()) {
            addLog(LOG_LEVEL_ERROR, F("ERROR: esp-hosted update end failed!"));
            break;
          }

          // Activate the new firmware
          addLog(LOG_LEVEL_INFO, F("Activating new firmware..."));

          if (!hostedActivateUpdate()) {
            addLog(LOG_LEVEL_ERROR, F("ERROR: esp-hosted update activate failed!"));
            break;
          }

          // Update completed successfully
          updateSuccess = true;
          addLog(LOG_LEVEL_INFO, F("SUCCESS: esp-hosted co-processor updated!"));
          break;
        }
      }

      // Small delay to prevent overwhelming the system
      delay(1);
    }

    // Step 15: Clean up allocated buffer
    free(buff);
    Serial.println();
  } else if (httpCode == HTTP_CODE_NOT_FOUND) {
    addLog(LOG_LEVEL_ERROR, F("ERROR: Update file not found!"));
  } else {
    addLog(LOG_LEVEL_ERROR, strformat(F("ERROR: HTTP request failed with code %d!"), httpCode));
  }

  // Step 16: Close HTTP connection
  https.end();

finish_ota:

  // Step 17: Clean up network client
  //  delete client;

  if (updateSuccess) { return return_command_success_flashstr(); }
  return return_command_failed_flashstr();
}

# endif // ifdef ESP32P4


#endif // if FEATURE_WIFI
