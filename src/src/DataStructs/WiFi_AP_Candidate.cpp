#include "../DataStructs/WiFi_AP_Candidate.h"

#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/SecuritySettings.h"
#include "../Globals/Statistics.h"
#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/Misc.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringGenerator_WiFi.h"
#include "../../ESPEasy_common.h"

#if defined(ESP8266)
  # include <ESP8266WiFi.h>
#endif // if defined(ESP8266)
#if defined(ESP32)
  # include <WiFi.h>
#endif // if defined(ESP32)

#define WIFI_AP_CANDIDATE_MAX_AGE   300000  // 5 minutes in msec


WiFi_AP_Candidate::WiFi_AP_Candidate(uint8_t index_c, const String& ssid_c) :
  last_seen(0), rssi(0), channel(0), index(index_c), flags(0)
{
  const size_t ssid_length = ssid_c.length();

  if ((ssid_length == 0) || equals(ssid_c, F("ssid"))) {
    return;
  }

  if (ssid_length > 32) { return; }

  ssid = ssid_c;
}

WiFi_AP_Candidate::WiFi_AP_Candidate(uint8_t networkItem) : index(0), flags(0) {
  ssid    = WiFi.SSID(networkItem);
  rssi    = WiFi.RSSI(networkItem);
  channel = WiFi.channel(networkItem);
  bssid   = WiFi.BSSID(networkItem);
  enc_type = WiFi.encryptionType(networkItem);
  #ifdef ESP8266
  isHidden = WiFi.isHidden(networkItem);
  #endif // ifdef ESP8266
  #ifdef ESP32
  isHidden = ssid.isEmpty();
  wifi_ap_record_t* it = reinterpret_cast<wifi_ap_record_t*>(WiFi.getScanInfoByIndex(networkItem));
  if (it) {
    phy_11b = it->phy_11b;
    phy_11g = it->phy_11g;
    phy_11n = it->phy_11n;
    wps = it->wps;
    // FIXME TD-er: Maybe also add other info like 2nd channel, ftm and phy_lr support?
  }
  #endif // ifdef ESP32
  last_seen = millis();
}

#ifdef ESP8266
#if FEATURE_ESP8266_DIRECT_WIFI_SCAN
WiFi_AP_Candidate::WiFi_AP_Candidate(const bss_info& ap) :
  rssi(ap.rssi), channel(ap.channel), bssid(ap.bssid), 
  index(0), enc_type(0), isHidden(ap.is_hidden),
  phy_11b(ap.phy_11b), phy_11g(ap.phy_11g), phy_11n(ap.phy_11n),
  wps(ap.wps)
{
  last_seen = millis();

  switch(ap.authmode) {
    case AUTH_OPEN: enc_type = ENC_TYPE_NONE; break;
    case AUTH_WEP:  enc_type = ENC_TYPE_WEP; break;
    case AUTH_WPA_PSK: enc_type =  ENC_TYPE_TKIP; break;
    case AUTH_WPA2_PSK: enc_type =  ENC_TYPE_CCMP; break;
    case AUTH_WPA_WPA2_PSK: enc_type =  ENC_TYPE_AUTO; break;
    case AUTH_MAX: break;
  }

  char tmp[33]; //ssid can be up to 32chars, => plus null term
  const size_t ssid_len = std::min(static_cast<size_t>(ap.ssid_len), sizeof(ap.ssid));
  memcpy(tmp, ap.ssid, ssid_len);
  tmp[ssid_len] = 0; // nullterm marking end of string

  ssid = String(reinterpret_cast<const char*>(tmp));
}
#endif
#endif


bool WiFi_AP_Candidate::operator<(const WiFi_AP_Candidate& other) const {
  if (isEmergencyFallback != other.isEmergencyFallback) {
    return isEmergencyFallback;
  }
  if (lowPriority != other.lowPriority) {
    return !lowPriority;
  }
  // Prefer non hidden over hidden.
  if (isHidden != other.isHidden) {
    return !isHidden;
  }

  // RSSI values >= 0 are invalid
  if (rssi >= 0) { return false; }

  if (other.rssi >= 0) { return true; }

  // RSSI values are negative, so the larger value is the better one.
  return rssi > other.rssi;
}

bool WiFi_AP_Candidate::operator==(const WiFi_AP_Candidate& other) const {
  return bssid_match(other.bssid) && ssid.equals(other.ssid);// && key.equals(other.key);
}

bool WiFi_AP_Candidate::usable() const {
  // Allow for empty pass
  // if (key.isEmpty()) return false;
  if (isEmergencyFallback) {
    int allowedUptimeMinutes = 10;
    #ifdef CUSTOM_EMERGENCY_FALLBACK_ALLOW_MINUTES_UPTIME
    allowedUptimeMinutes = CUSTOM_EMERGENCY_FALLBACK_ALLOW_MINUTES_UPTIME;
    #endif
    if (getUptimeMinutes() > allowedUptimeMinutes || 
        !SecuritySettings.hasWiFiCredentials() || 
        WiFiEventData.performedClearWiFiCredentials ||
        lastBootCause != BOOT_CAUSE_COLD_BOOT) {
      return false;
    }
  }
  if (!isHidden && (ssid.isEmpty())) { return false; }
  return !expired();
}

bool WiFi_AP_Candidate::expired() const {
  if (last_seen == 0) {
    // Not set, so cannot expire
    return false;
  }
  return timePassedSince(last_seen) > WIFI_AP_CANDIDATE_MAX_AGE;
}

bool WiFi_AP_Candidate::allowQuickConnect() const {
  if (channel == 0) { return false; }
  return bssid_set();
}

bool WiFi_AP_Candidate::bssid_set() const {
  return !bssid.all_zero();
}

bool WiFi_AP_Candidate::bssid_match(const uint8_t bssid_c[6]) const {
  return bssid == bssid_c;
}

bool WiFi_AP_Candidate::bssid_match(const MAC_address& other) const {
  return bssid == other;
}

String WiFi_AP_Candidate::toString(const String& separator) const {
  String result = ssid;

  htmlEscape(result);
  if (isHidden) {
    result += F("#Hidden#");
  }
  result += separator;
  result += bssid.toString();
  result += separator;
  result += F("Ch:");
  result += channel;

  if (rssi == -1) {
    result += F(" (RTC) ");
  } else {
    result += F(" (");
    result += rssi;
    result += F("dBm) ");
  }

  result += encryption_type();
  if (phy_known()) {
    result += ' ';
    if (phy_11b) result += 'b';
    if (phy_11g) result += 'g';
    if (phy_11n) result += 'n';
  }
  return result;
}

String WiFi_AP_Candidate::encryption_type() const {
  return WiFi_encryptionType(enc_type);
}

bool WiFi_AP_Candidate::phy_known() const {
  return phy_11b || phy_11g || phy_11n;
}