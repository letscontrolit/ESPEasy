#include "../Helpers/StringGenerator_WiFi.h"

#include "../Globals/ESPEasyWiFiEvent.h"

String WiFi_encryptionType(byte encryptionType) {
    String result;
switch (encryptionType) {
  #ifdef ESP32
    case WIFI_AUTH_OPEN: result            += F("open"); break;
    case WIFI_AUTH_WEP:  result            += F("WEP"); break;
    case WIFI_AUTH_WPA_PSK: result         += F("WPA/PSK"); break;
    case WIFI_AUTH_WPA2_PSK: result        += F("WPA2/PSK"); break;
    case WIFI_AUTH_WPA_WPA2_PSK: result    += F("WPA/WPA2/PSK"); break;
    case WIFI_AUTH_WPA2_ENTERPRISE: result += F("WPA2 Enterprise"); break;
  #else // ifdef ESP32
    case ENC_TYPE_WEP: result  += F("WEP"); break;
    case ENC_TYPE_TKIP: result += F("WPA/PSK"); break;
    case ENC_TYPE_CCMP: result += F("WPA2/PSK"); break;
    case ENC_TYPE_NONE: result += F("open"); break;
    case ENC_TYPE_AUTO: result += F("WPA/WPA2/PSK"); break;
  #endif // ifdef ESP32
    default:
      break;
  }
  return result;
}

#ifndef ESP32
String SDKwifiStatusToString(uint8_t sdk_wifistatus) {
  #ifdef LIMIT_BUILD_SIZE
  return String(sdk_wifistatus);
  #else
  switch (sdk_wifistatus) {
    case STATION_IDLE:           return F("STATION_IDLE");
    case STATION_CONNECTING:     return F("STATION_CONNECTING");
    case STATION_WRONG_PASSWORD: return F("STATION_WRONG_PASSWORD");
    case STATION_NO_AP_FOUND:    return F("STATION_NO_AP_FOUND");
    case STATION_CONNECT_FAIL:   return F("STATION_CONNECT_FAIL");
    case STATION_GOT_IP:         return F("STATION_GOT_IP");
  }
  return getUnknownString();
  #endif
}

#endif // ifndef ESP32

String ArduinoWifiStatusToString(uint8_t arduino_corelib_wifistatus) {
  #ifdef LIMIT_BUILD_SIZE
  return String(arduino_corelib_wifistatus);
  #else
  String log;

  switch (arduino_corelib_wifistatus) {
    case WL_NO_SHIELD:       log += F("WL_NO_SHIELD"); break;
    case WL_IDLE_STATUS:     log += F("WL_IDLE_STATUS"); break;
    case WL_NO_SSID_AVAIL:   log += F("WL_NO_SSID_AVAIL"); break;
    case WL_SCAN_COMPLETED:  log += F("WL_SCAN_COMPLETED"); break;
    case WL_CONNECTED:       log += F("WL_CONNECTED"); break;
    case WL_CONNECT_FAILED:  log += F("WL_CONNECT_FAILED"); break;
    case WL_CONNECTION_LOST: log += F("WL_CONNECTION_LOST"); break;
    case WL_DISCONNECTED:    log += F("WL_DISCONNECTED"); break;
    default:  log                += arduino_corelib_wifistatus; break;
  }
  return log;
  #endif
}


String getLastDisconnectReason() {
  String reason = "(";

  reason += WiFiEventData.lastDisconnectReason;
  reason += F(") ");

  #ifndef LIMIT_BUILD_SIZE
  switch (WiFiEventData.lastDisconnectReason) {
    case WIFI_DISCONNECT_REASON_UNSPECIFIED:                reason += F("Unspecified");              break;
    case WIFI_DISCONNECT_REASON_AUTH_EXPIRE:                reason += F("Auth expire");              break;
    case WIFI_DISCONNECT_REASON_AUTH_LEAVE:                 reason += F("Auth leave");               break;
    case WIFI_DISCONNECT_REASON_ASSOC_EXPIRE:               reason += F("Assoc expire");             break;
    case WIFI_DISCONNECT_REASON_ASSOC_TOOMANY:              reason += F("Assoc toomany");            break;
    case WIFI_DISCONNECT_REASON_NOT_AUTHED:                 reason += F("Not authed");               break;
    case WIFI_DISCONNECT_REASON_NOT_ASSOCED:                reason += F("Not assoced");              break;
    case WIFI_DISCONNECT_REASON_ASSOC_LEAVE:                reason += F("Assoc leave");              break;
    case WIFI_DISCONNECT_REASON_ASSOC_NOT_AUTHED:           reason += F("Assoc not authed");         break;
    case WIFI_DISCONNECT_REASON_DISASSOC_PWRCAP_BAD:        reason += F("Disassoc pwrcap bad");      break;
    case WIFI_DISCONNECT_REASON_DISASSOC_SUPCHAN_BAD:       reason += F("Disassoc supchan bad");     break;
    case WIFI_DISCONNECT_REASON_IE_INVALID:                 reason += F("IE invalid");               break;
    case WIFI_DISCONNECT_REASON_MIC_FAILURE:                reason += F("Mic failure");              break;
    case WIFI_DISCONNECT_REASON_4WAY_HANDSHAKE_TIMEOUT:     reason += F("4way handshake timeout");   break;
    case WIFI_DISCONNECT_REASON_GROUP_KEY_UPDATE_TIMEOUT:   reason += F("Group key update timeout"); break;
    case WIFI_DISCONNECT_REASON_IE_IN_4WAY_DIFFERS:         reason += F("IE in 4way differs");       break;
    case WIFI_DISCONNECT_REASON_GROUP_CIPHER_INVALID:       reason += F("Group cipher invalid");     break;
    case WIFI_DISCONNECT_REASON_PAIRWISE_CIPHER_INVALID:    reason += F("Pairwise cipher invalid");  break;
    case WIFI_DISCONNECT_REASON_AKMP_INVALID:               reason += F("AKMP invalid");             break;
    case WIFI_DISCONNECT_REASON_UNSUPP_RSN_IE_VERSION:      reason += F("Unsupp RSN IE version");    break;
    case WIFI_DISCONNECT_REASON_INVALID_RSN_IE_CAP:         reason += F("Invalid RSN IE cap");       break;
    case WIFI_DISCONNECT_REASON_802_1X_AUTH_FAILED:         reason += F("802 1X auth failed");       break;
    case WIFI_DISCONNECT_REASON_CIPHER_SUITE_REJECTED:      reason += F("Cipher suite rejected");    break;
    case WIFI_DISCONNECT_REASON_BEACON_TIMEOUT:             reason += F("Beacon timeout");           break;
    case WIFI_DISCONNECT_REASON_NO_AP_FOUND:                reason += F("No AP found");              break;
    case WIFI_DISCONNECT_REASON_AUTH_FAIL:                  reason += F("Auth fail");                break;
    case WIFI_DISCONNECT_REASON_ASSOC_FAIL:                 reason += F("Assoc fail");               break;
    case WIFI_DISCONNECT_REASON_HANDSHAKE_TIMEOUT:          reason += F("Handshake timeout");        break;
    default:  reason                                               += getUnknownString();       break;
  }
  #endif
  return reason;
}
