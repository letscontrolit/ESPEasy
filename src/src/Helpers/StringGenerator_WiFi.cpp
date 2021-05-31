#include "../Helpers/StringGenerator_WiFi.h"

#include "../Globals/ESPEasyWiFiEvent.h"

const __FlashStringHelper * WiFi_encryptionType(byte encryptionType) {
switch (encryptionType) {
  #ifdef ESP32
    case WIFI_AUTH_OPEN:             return F("open"); 
    case WIFI_AUTH_WEP:              return F("WEP"); 
    case WIFI_AUTH_WPA_PSK:          return F("WPA/PSK"); 
    case WIFI_AUTH_WPA2_PSK:         return F("WPA2/PSK"); 
    case WIFI_AUTH_WPA_WPA2_PSK:     return F("WPA/WPA2/PSK"); 
    case WIFI_AUTH_WPA2_ENTERPRISE:  return F("WPA2 Enterprise"); 
  #else // ifdef ESP32
    case ENC_TYPE_WEP:   return F("WEP"); 
    case ENC_TYPE_TKIP:  return F("WPA/PSK"); 
    case ENC_TYPE_CCMP:  return F("WPA2/PSK"); 
    case ENC_TYPE_NONE:  return F("open"); 
    case ENC_TYPE_AUTO:  return F("WPA/WPA2/PSK"); 
  #endif // ifdef ESP32
    default:
	break;
      
  }
  return F("Unknown");
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
  return F("Unknown");
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


const __FlashStringHelper * getLastDisconnectReason(WiFiDisconnectReason reason) {
  switch (reason) {
    case WIFI_DISCONNECT_REASON_UNSPECIFIED:                return F("Unspecified");              
    case WIFI_DISCONNECT_REASON_AUTH_EXPIRE:                return F("Auth expire");              
    case WIFI_DISCONNECT_REASON_AUTH_LEAVE:                 return F("Auth leave");               
    case WIFI_DISCONNECT_REASON_ASSOC_EXPIRE:               return F("Assoc expire");             
    case WIFI_DISCONNECT_REASON_ASSOC_TOOMANY:              return F("Assoc toomany");            
    case WIFI_DISCONNECT_REASON_NOT_AUTHED:                 return F("Not authed");               
    case WIFI_DISCONNECT_REASON_NOT_ASSOCED:                return F("Not assoced");              
    case WIFI_DISCONNECT_REASON_ASSOC_LEAVE:                return F("Assoc leave");              
    case WIFI_DISCONNECT_REASON_ASSOC_NOT_AUTHED:           return F("Assoc not authed");         
    case WIFI_DISCONNECT_REASON_DISASSOC_PWRCAP_BAD:        return F("Disassoc pwrcap bad");      
    case WIFI_DISCONNECT_REASON_DISASSOC_SUPCHAN_BAD:       return F("Disassoc supchan bad");     
    case WIFI_DISCONNECT_REASON_IE_INVALID:                 return F("IE invalid");               
    case WIFI_DISCONNECT_REASON_MIC_FAILURE:                return F("Mic failure");              
    case WIFI_DISCONNECT_REASON_4WAY_HANDSHAKE_TIMEOUT:     return F("4way handshake timeout");   
    case WIFI_DISCONNECT_REASON_GROUP_KEY_UPDATE_TIMEOUT:   return F("Group key update timeout"); 
    case WIFI_DISCONNECT_REASON_IE_IN_4WAY_DIFFERS:         return F("IE in 4way differs");       
    case WIFI_DISCONNECT_REASON_GROUP_CIPHER_INVALID:       return F("Group cipher invalid");     
    case WIFI_DISCONNECT_REASON_PAIRWISE_CIPHER_INVALID:    return F("Pairwise cipher invalid");  
    case WIFI_DISCONNECT_REASON_AKMP_INVALID:               return F("AKMP invalid");             
    case WIFI_DISCONNECT_REASON_UNSUPP_RSN_IE_VERSION:      return F("Unsupp RSN IE version");    
    case WIFI_DISCONNECT_REASON_INVALID_RSN_IE_CAP:         return F("Invalid RSN IE cap");       
    case WIFI_DISCONNECT_REASON_802_1X_AUTH_FAILED:         return F("802 1X auth failed");       
    case WIFI_DISCONNECT_REASON_CIPHER_SUITE_REJECTED:      return F("Cipher suite rejected");    
    case WIFI_DISCONNECT_REASON_BEACON_TIMEOUT:             return F("Beacon timeout");           
    case WIFI_DISCONNECT_REASON_NO_AP_FOUND:                return F("No AP found");              
    case WIFI_DISCONNECT_REASON_AUTH_FAIL:                  return F("Auth fail");                
    case WIFI_DISCONNECT_REASON_ASSOC_FAIL:                 return F("Assoc fail");               
    case WIFI_DISCONNECT_REASON_HANDSHAKE_TIMEOUT:          return F("Handshake timeout");        
    default:  return F("Unknown");
  }
}

String getLastDisconnectReason() {
  String reason = "(";

  reason += WiFiEventData.lastDisconnectReason;
  reason += F(") ");

  #ifndef LIMIT_BUILD_SIZE
  reason += getLastDisconnectReason(WiFiEventData.lastDisconnectReason);
  #endif
  return reason;
}
