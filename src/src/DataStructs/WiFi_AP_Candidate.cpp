#include "../DataStructs/WiFi_AP_Candidate.h"

#include "../Helpers/StringConverter.h"
#include "../Helpers/StringGenerator_WiFi.h"

WiFi_AP_Candidate::WiFi_AP_Candidate(byte index_c, const String& ssid_c, const String& pass) :
  rssi(0), channel(0), index(index_c), isHidden(false)
{
  const size_t ssid_length = ssid_c.length();

  if ((ssid_length == 0) || ssid_c.equals(F("ssid"))) {
    return;
  }

  if (ssid_length > 32) { return; }

  if (pass.length() > 64) { return; }

  ssid = ssid_c;
  key  = pass;
}

WiFi_AP_Candidate::WiFi_AP_Candidate(uint8_t networkItem) : index(0) {
  ssid    = WiFi.SSID(networkItem);
  rssi    = WiFi.RSSI(networkItem);
  channel = WiFi.channel(networkItem);
  setBSSID(WiFi.BSSID(networkItem));
  enc_type = WiFi.encryptionType(networkItem);
  #ifdef ESP8266
  isHidden = WiFi.isHidden(networkItem);
  #endif // ifdef ESP8266
  #ifdef ESP32
  isHidden = ssid.length() == 0;
  #endif // ifdef ESP32
}

WiFi_AP_Candidate::WiFi_AP_Candidate() {}

bool WiFi_AP_Candidate::operator<(const WiFi_AP_Candidate& other) const {
  // RSSI values >= 0 are invalid
  if (rssi >= 0) { return false; }

  if (other.rssi >= 0) { return true; }

  // RSSI values are negative, so the larger value is the better one.
  return rssi > other.rssi;
}

bool WiFi_AP_Candidate::operator==(const WiFi_AP_Candidate& other) const {
  return bssid_match(other.bssid) && ssid.equals(other.ssid) && key.equals(other.key);
}

WiFi_AP_Candidate& WiFi_AP_Candidate::operator=(const WiFi_AP_Candidate& other) {
  if (this != &other) { // not a self-assignment
    ssid    = other.ssid;
    key     = other.key;
    rssi    = other.rssi;
    channel = other.channel;
    setBSSID(other.bssid);
    isHidden = other.isHidden;
    index    = other.index;
    enc_type = other.enc_type;
  }
  return *this;
}

void WiFi_AP_Candidate::setBSSID(const uint8_t *bssid_c) {
  for (byte i = 0; i < 6; ++i) {
    bssid[i] = *(bssid_c + i);
  }
}

bool WiFi_AP_Candidate::usable() const {
  // Allow for empty pass
  // if (key.length() == 0) return false;
  if (!isHidden && (ssid.length() == 0)) { return false; }
  return true;
}

bool WiFi_AP_Candidate::allowQuickConnect() const {
  if (channel == 0) { return false; }
  return bssid_set();
}

bool WiFi_AP_Candidate::bssid_set() const {
  for (byte i = 0; i < 6; ++i) {
    if (bssid[i] != 0) { return true; }
  }
  return false;
}

bool WiFi_AP_Candidate::bssid_match(const uint8_t *bssid_c) const {
  for (byte i = 0; i < 6; ++i) {
    if (bssid[i] != bssid_c[i]) { return false; }
  }
  return true;
}

String WiFi_AP_Candidate::toString(const String& separator) const {
  String result = ssid;

  htmlEscape(result);
  if (isHidden) {
    result += F("#Hidden#");
  }
  result += separator;
  result += formatMAC(bssid);
  result += separator;
  result += F("Ch:");
  result += channel;

  if (rssi == -1) {
    result += F(" (RTC) ");
  } else {
    result += " (";
    result += rssi;
    result += F("dBm) ");
  }

  result += encryption_type();
  return result;
}

String WiFi_AP_Candidate::encryption_type() const {
  return WiFi_encryptionType(enc_type);
}