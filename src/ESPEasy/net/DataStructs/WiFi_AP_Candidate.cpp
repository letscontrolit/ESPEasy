#include "../DataStructs/WiFi_AP_Candidate.h"

#include "../../../ESPEasy_common.h"

#include "../Globals/ESPEasyWiFiEvent.h"
#include "../../../src/Globals/SecuritySettings.h"
#include "../../../src/Globals/Statistics.h"
#include "../../../src/Helpers/ESPEasy_time_calc.h"
#include "../../../src/Helpers/Misc.h"
#include "../../../src/Helpers/StringConverter.h"
#include "../../../src/Helpers/StringGenerator_WiFi.h"

#if defined(ESP8266)
  # include <ESP8266WiFi.h>
#endif // if defined(ESP8266)
#if defined(ESP32)
  # include <WiFi.h>
#endif // if defined(ESP32)


#if FEATURE_WIFI

namespace ESPEasy {
namespace net {
namespace wifi {





WiFi_AP_Candidate::WiFi_AP_Candidate() :
# ifdef ESP32
#  if ESP_IDF_VERSION_MAJOR >= 5
  country({
        .cc     = "01",
        .schan  = 1,
        .nchan  = 14,
        .policy = WIFI_COUNTRY_POLICY_AUTO,
      }),
#  endif // if ESP_IDF_VERSION_MAJOR >= 5
# endif // ifdef ESP32
  last_seen(0), rssi(0), channel(0), index(0), enc_type(0)
{
  _allBits = 0u;
}

WiFi_AP_Candidate::WiFi_AP_Candidate(const WiFi_AP_Candidate& other)
  : ssid(other.ssid),
  last_seen(other.last_seen),
  bssid(other.bssid),
  rssi(other.rssi),
  channel(other.channel),
  index(other.index),
  enc_type(other.enc_type)
{
  _allBits = other._allBits;
  # ifdef ESP32
  #  if ESP_IDF_VERSION_MAJOR >= 5
  memcpy(&this->country, &other.country, sizeof(wifi_country_t));
  #  endif
  # endif // ifdef ESP32
}

WiFi_AP_Candidate::WiFi_AP_Candidate(uint8_t index_c, const String& ssid_c) :
# ifdef ESP32
#  if ESP_IDF_VERSION_MAJOR >= 5
  country({
        .cc     = "01",
        .schan  = 1,
        .nchan  = 14,
        .policy = WIFI_COUNTRY_POLICY_AUTO,
      }),
#  endif // if ESP_IDF_VERSION_MAJOR >= 5
# endif // ifdef ESP32
  last_seen(0), rssi(0), channel(0), index(index_c), enc_type(0)
{
  _allBits = 0u;

  const size_t ssid_length = ssid_c.length();

  if ((ssid_length == 0) || equals(ssid_c, F("ssid"))) {
    return;
  }

  if (ssid_length > 32) { return; }

  ssid = ssid_c;
}

WiFi_AP_Candidate::WiFi_AP_Candidate(uint8_t networkItem) : index(0) {
  // Need to make sure the phy isn't known as we can't get this information from the AP
  // See: https://github.com/letscontrolit/ESPEasy/issues/4996
  // Not sure why this makes any difference as the flags should already have been set to 0.
  _allBits = 0u;

  ssid     = WiFi.SSID(networkItem);
  rssi     = WiFi.RSSI(networkItem);
  channel  = WiFi.channel(networkItem);
  bssid    = WiFi.BSSID(networkItem);
  enc_type = WiFi.encryptionType(networkItem);
  # ifdef ESP8266
  bits.isHidden = WiFi.isHidden(networkItem);
  #  ifdef CORE_POST_3_0_0
  const bss_info *it = reinterpret_cast<const bss_info *>(WiFi.getScanInfoByIndex(networkItem));

  if (it) {
    bits.phy_11b = it->phy_11b;
    bits.phy_11g = it->phy_11g;
    bits.phy_11n = it->phy_11n;
    bits.wps     = it->wps;
  }
  #  endif // ifdef CORE_POST_3_0_0
  # endif // ifdef ESP8266
  # ifdef ESP32
  bits.isHidden = ssid.isEmpty();
  wifi_ap_record_t *it = reinterpret_cast<wifi_ap_record_t *>(WiFi.getScanInfoByIndex(networkItem));

  if (it) {
    bits.phy_11b = it->phy_11b;
    bits.phy_11g = it->phy_11g;
    bits.phy_11n = it->phy_11n;
    bits.phy_lr  = it->phy_lr;
#  if ESP_IDF_VERSION_MAJOR >= 5
    bits.phy_11ax      = it->phy_11ax;
    bits.ftm_initiator = it->ftm_initiator;
    bits.ftm_responder = it->ftm_responder;
#   if CONFIG_SOC_WIFI_SUPPORT_5G
    bits.phy_11a  = it->phy_11a;
    bits.phy_11ac = it->phy_11ac;
#   endif // if CONFIG_SOC_WIFI_SUPPORT_5G
#  endif // if ESP_IDF_VERSION_MAJOR >= 5
    bits.wps = it->wps;

    // FIXME TD-er: Maybe also add other info like 2nd channel, ftm and phy_lr support?
#  if ESP_IDF_VERSION_MAJOR >= 5
    memcpy(&country, &(it->country), sizeof(wifi_country_t));
    bandwidth = it->bandwidth;
#  endif // if ESP_IDF_VERSION_MAJOR >= 5
  }
  # endif // ifdef ESP32
  last_seen = millis();
}


bool WiFi_AP_Candidate::operator<(const WiFi_AP_Candidate& other) const {
  if (bits.isEmergencyFallback != other.bits.isEmergencyFallback) {
    return bits.isEmergencyFallback;
  }

  if (bits.lowPriority != other.bits.lowPriority) {
    return !bits.lowPriority;
  }

  // Prefer non hidden over hidden.
  if (bits.isHidden != other.bits.isHidden) {
    return !bits.isHidden;
  }

  // RSSI values >= 0 are invalid
  if (rssi >= 0) { return false; }

  if (other.rssi >= 0) { return true; }

  // RSSI values are negative, so the larger value is the better one.
  return rssi > other.rssi;
}

WiFi_AP_Candidate& WiFi_AP_Candidate::operator=(const WiFi_AP_Candidate& other)
{
  ssid      = other.ssid;
  last_seen = other.last_seen;
  bssid     = other.bssid;
  rssi      = other.rssi;
  channel   = other.channel;
  index     = other.index;
  enc_type  = other.enc_type;
  _allBits  = other._allBits;
  # ifdef ESP32
  #  if ESP_IDF_VERSION_MAJOR >= 5
  memcpy(&this->country, &other.country, sizeof(wifi_country_t));
  bandwidth = other.bandwidth;
  #  endif // if ESP_IDF_VERSION_MAJOR >= 5
  # endif // ifdef ESP32

  return *this;
}

bool WiFi_AP_Candidate::usable() const {
  // Allow for empty pass
  // if (key.isEmpty()) return false;
  if (bits.isEmergencyFallback) {
    int allowedUptimeMinutes = 10;
    # ifdef CUSTOM_EMERGENCY_FALLBACK_ALLOW_MINUTES_UPTIME
    allowedUptimeMinutes = CUSTOM_EMERGENCY_FALLBACK_ALLOW_MINUTES_UPTIME;
    # endif // ifdef CUSTOM_EMERGENCY_FALLBACK_ALLOW_MINUTES_UPTIME

    if ((getUptimeMinutes() > allowedUptimeMinutes) ||
        !SecuritySettings.hasWiFiCredentials() ||
//        WiFiEventData.performedClearWiFiCredentials ||
        (lastBootCause != BOOT_CAUSE_COLD_BOOT)) {
      return false;
    }
  }

  if (!bits.isHidden && (ssid.isEmpty())) { return false; }
  if (!fromScan()) {
    // Not from a scan, thus usable
    return true;
  }

  return channel != 0 && !expired();
}

bool WiFi_AP_Candidate::fromScan() const
{
  return last_seen != 0;
}

bool WiFi_AP_Candidate::expired() const {
  if (last_seen == 0) {
    // Not set, so cannot expire
    return false;
  }
  return timePassedSince(last_seen) > WIFI_AP_CANDIDATE_MAX_AGE;
}

String WiFi_AP_Candidate::toString(const String& separator) const {
  String result = ssid;

  htmlEscape(result);

  if (bits.isHidden) {
    result += F("#Hidden#");
  }
  result += strformat(
    F("%s%s%sCh:%u"),
    separator.c_str(),
    bssid.toString().c_str(),
    separator.c_str(),
    channel);

# if CONFIG_SOC_WIFI_SUPPORT_5G

  if (channel < 36) {
    result += F(" 2.4 GHz");
  } else {
    result += F(" 5 GHz");
  }
# endif // if CONFIG_SOC_WIFI_SUPPORT_5G

  if (rssi == -1) {
    result += F(" (RTC) ");
  } else {
    result += strformat(F(" (%ddBm) "), rssi);
  }

  result += encryption_type();

# ifdef ESP32
#  if ESP_IDF_VERSION_MAJOR >= 5

  // Country code string
  if ((country.cc[0] != '\0') && (country.cc[1] != '\0')) {
    result += strformat(F(" '%c%c'"), country.cc[0], country.cc[1]);

    switch (country.cc[2])
    {
      case 'O': // Outdoor
      case 'I': // Indoor
      case 'X': // "non-country"
        result += strformat(F("(%c)"), country.cc[2]);
        break;
    }
  }

  if (country.nchan > 0) {
    result += strformat(F(" ch: %d..%d"), country.schan, country.schan + country.nchan - 1);
  }
#  endif // if ESP_IDF_VERSION_MAJOR >= 5
# endif // ifdef ESP32

  if (phy_known()) {
    String phy_str;

    // Order of items is kinda in order of age, oldest first.

    // first 2.4 GHz version
    // Later called "Wi-Fi 1"
    if (bits.phy_11b) { phy_str += 'b'; }

    // 2nd 2.4 GHz version to adopt the same 54 Mbps bandwidth as used in 802.11a
    // Later called "Wi-Fi 3"
    if (bits.phy_11g) { phy_str += 'g'; }

    // 3rd iteration for 2.4 GHz
    // Also used for 5 GHz to make it even more confusing
    // To top the naming confusion even worse, this is later called "Wi-Fi 4"
    if (bits.phy_11n) { phy_str += 'n'; }
# ifdef ESP32

    // Add 5 GHz WiFi types, oldest first.

    // 802.11a was one of the first WiFi standards, using 5 GHz
    // Later called "Wi-Fi 2"
    if (bits.phy_11a) { phy_str += F("/a"); }

    // 802.11ac was the significantly faster version for 5 GHz WiFi.
    // Later called "Wi-Fi 5"
    if (bits.phy_11ac) { phy_str += F("/ac"); }

    // AX is WiFi-6, on both frequencies
    if (bits.phy_11ax) { phy_str += F("/ax"); }

    if (bits.phy_lr) { phy_str += F("/lr"); }

    if (bits.ftm_initiator) { phy_str += F("/FTM_i"); }

    if (bits.ftm_responder) { phy_str += F("/FTM_r"); }

# endif // ifdef ESP32

    if (phy_str.length()) {
# ifdef ESP32

      switch (bandwidth)
      {
        case WIFI_BW_HT20: break;
        case WIFI_BW_HT40:   phy_str += F(" 40 MHz");
          break;
        case WIFI_BW80:      phy_str += F(" 80 MHz");
          break;
        case WIFI_BW160:     phy_str += F(" 160 MHz");
          break;
        case WIFI_BW80_BW80: phy_str += F(" 80+80 MHz");
          break;

        default:
          break;
      }
# endif // ifdef ESP32
      result += strformat(F(" (%s)"), phy_str.c_str());
    }
  }
  return result;
}

String WiFi_AP_Candidate::encryption_type() const {
  return WiFi_encryptionType(enc_type);
}

} // namespace wifi
} // namespace net
} // namespace ESPEasy

#endif // if FEATURE_WIFI
