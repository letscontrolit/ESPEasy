#include "../Helpers/WiFi_AP_CandidatesList.h"

#ifdef ESP32
# include "../DataStructs/WiFi_AP_Candidates_NVS.h"
#endif

#include "../Globals/ESPEasyWiFiEvent.h"

#include "../../../src/ESPEasyCore/ESPEasy_Log.h"
#include "../../../src/Globals/RTC.h"
#include "../../../src/Globals/SecuritySettings.h"
#include "../../../src/Globals/Settings.h"
#include "../../../src/Helpers/Misc.h"
#include "../../../src/Helpers/StringConverter.h"

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

# define WIFI_CUSTOM_DEPLOYMENT_KEY_INDEX     3
# define WIFI_CUSTOM_SUPPORT_KEY_INDEX        4
# define WIFI_CREDENTIALS_FALLBACK_SSID_INDEX 5
# if FEATURE_STORE_CREDENTIALS_SEPARATE_FILE
#  define WIFI_CREDENTIALs_SEPARATE_FILE_FIRST_INDEX 6
#  define WIFI_CREDENTIALS_MAX_INDEX (WIFI_CREDENTIALs_SEPARATE_FILE_FIRST_INDEX + MAX_EXTRA_WIFI_CREDENTIALS_SEPARATE_FILE)
# else // if FEATURE_STORE_CREDENTIALS_SEPARATE_FILE
#  define WIFI_CREDENTIALS_MAX_INDEX 6
# endif // if FEATURE_STORE_CREDENTIALS_SEPARATE_FILE

static LongTermOnOffTimer _last_scan;


WiFi_AP_CandidatesList::WiFi_AP_CandidatesList() {
  _last_scan.clear();
  known.clear();
  candidates.clear();
  known_it = known.begin();
}

WiFi_AP_CandidatesList::~WiFi_AP_CandidatesList() {
  _last_scan.clear();
  candidates.clear();
  known.clear();
  scanned.clear();
  scanned_new.clear();
}

void WiFi_AP_CandidatesList::load_knownCredentials() {
  if (!_mustLoadCredentials && !known.empty()) { return; }
  _mustLoadCredentials = false;
  known.clear();
  candidates.clear();

  //  attemptsLeft = WiFi_CONNECT_ATTEMPTS;
  _addedKnownCandidate = false;

  //  addFromRTC();

  {
    // Add the known SSIDs
    uint8_t index = 1; // Index 0 is the "unset" value

    for (; index < WIFI_CREDENTIALS_MAX_INDEX; ++index) {
      String ssid;

      if (get_SSID(index, ssid)) {
        // Make sure emplace_back is not done on the 2nd heap
        # ifdef USE_SECOND_HEAP
        HeapSelectDram ephemeral;
        # endif // ifdef USE_SECOND_HEAP

        WiFi_AP_Candidate tmp_known(index, ssid);
        if (SettingsIndexMatchCustomCredentials(index)) {
          if (SettingsIndexMatchEmergencyFallback(index)) {
            tmp_known.bits.isEmergencyFallback = true;
          } else {
            tmp_known.bits.lowPriority = true;
          }
        }
        known.push_back(tmp_known);
        ++index;
      }
    }
  }
  loadCandidatesFromScanned();
  addFromRTC();
}

void WiFi_AP_CandidatesList::clearCache() {
  _mustLoadCredentials = true;
  known.clear();
  known_it = known.begin();
}

void WiFi_AP_CandidatesList::force_reload() {
  clearCache();
  RTC.clearLastWiFi(); // Invalidate the RTC WiFi data.
  candidates.clear();
  load_knownCredentials();
}

void WiFi_AP_CandidatesList::begin_scan(uint8_t channel) {
  _last_scan.setOn();
  _last_scan_channel = channel;
  candidates.clear();
  _addedKnownCandidate = false;
}

void WiFi_AP_CandidatesList::purge_expired() {
  for (auto it = scanned.begin(); it != scanned.end();) {
    if (it->expired()) {
      it = scanned.erase(it);
    } else {
      ++it;
    }
  }
}

void WiFi_AP_CandidatesList::process_WiFiscan() {
  //  if (_last_scan.isOn()) {
  // Append or update found APs from scan.
  int scancount = WiFi.scanComplete();

  if (scancount > 0) {
    for (int i = 0; i < scancount; ++i) {
      const WiFi_AP_Candidate tmp(i);

      scanned_new.push_back(tmp);
    }

    after_process_WiFiscan();
  }

  //  }
}

void WiFi_AP_CandidatesList::after_process_WiFiscan() {
  _last_scan.setOff();
  scanned_new.sort();
  scanned_new.unique();
  _mustLoadCredentials = true;
  load_knownCredentials();
  WiFi.scanDelete();
  attemptsLeft = WiFi_CONNECT_ATTEMPTS;
}

bool WiFi_AP_CandidatesList::getNext(bool scanAllowed) {
  load_knownCredentials();

  if (candidates.empty()) {
    return false;
  }

  currentCandidate = candidates.front();
  bool mustPop = true;

  if (currentCandidate.bits.isHidden) {
    // Iterate over the known credentials to try them all
    // Hidden SSID stations do not broadcast their SSID, so we must fill it in ourselves.
    if (known_it != known.end()) {
      currentCandidate.ssid  = known_it->ssid;
      currentCandidate.index = known_it->index;
      ++known_it;
    }

    if (known_it != known.end()) {
      mustPop = false;
    }
  }

  if (mustPop) {
    if (attemptsLeft == 0) {
      if (currentCandidate.bits.isHidden && !Settings.HiddenSSID_SlowConnectPerBSSID()) {
        // We tried to connect to hidden SSIDs in 1 run, so pop all hidden candidates.
        for (auto cand_it = candidates.begin(); cand_it != candidates.end() && cand_it->bits.isHidden;) {
          cand_it = candidates.erase(cand_it);
        }
      } else {
        if (!candidates.empty()) {
          candidates.pop_front();
        }
      }

      known_it     = known.begin();
      attemptsLeft = WiFi_CONNECT_ATTEMPTS;
    } else {
      markAttempt();
    }
  }
  return currentCandidate.usable();
}

const WiFi_AP_Candidate& WiFi_AP_CandidatesList::getCurrent() const {
  return currentCandidate;
}

void              WiFi_AP_CandidatesList::markAttempt() { if (attemptsLeft > 0) { attemptsLeft--; }
}

WiFi_AP_Candidate WiFi_AP_CandidatesList::getBestCandidate() const {
  for (auto it = candidates.begin(); it != candidates.end(); ++it) {
    if (it->rssi < -1 && it->usable()) { return *it; }
  }
  return WiFi_AP_Candidate();
}

bool WiFi_AP_CandidatesList::hasCandidateCredentials() {
  load_knownCredentials();
  return !known.empty();
}

bool WiFi_AP_CandidatesList::hasCandidates() const {
//  return !candidates.empty();
  return getBestCandidate().usable();
}

void WiFi_AP_CandidatesList::markCurrentConnectionStable() {
  clearCache();

  if (currentCandidate.enc_type == 0) {
    bool matchfound = false;

    for (auto it = candidates.begin(); !matchfound && it != candidates.end(); ++it) {
      if (currentCandidate == *it) {
        // We may have gotten the enc_type of the active used candidate
        // Make sure to store the enc type before clearing the candidates list
        currentCandidate.enc_type = it->enc_type;
        matchfound                = true;
      }
    }
  }

  if (currentCandidate.usable()) {
    // Store in RTC
    RTC.lastWiFiChannel = currentCandidate.channel;
    currentCandidate.bssid.get(RTC.lastBSSID);
    RTC.lastWiFiSettingsIndex = currentCandidate.index;
# ifdef ESP32

    if (Settings.UseLastWiFiFromRTC()) {
      WiFi_AP_Candidates_NVS::currentConnection_to_NVS(currentCandidate);
    }
    else {
      WiFi_AP_Candidates_NVS::clear_from_NVS();
    }
# endif // ifdef ESP32
  }

  candidates.clear();
  _addedKnownCandidate = false;
  addFromRTC(); // Store the current one from RTC as the first candidate for a reconnect.
}

int8_t WiFi_AP_CandidatesList::scanComplete() const {
  //  if (!_last_scan.isOn() ||
  //      (_last_scan.getLastOnDuration_ms() > WIFI_AP_CANDIDATE_MAX_AGE))  return -3;

  const int8_t scanCompleteStatus = WiFi.scanComplete();

  if (scanCompleteStatus == -1) {
    // Still scanning
    return scanCompleteStatus;
  }

  size_t found = 0;

  for (auto scan = scanned.begin(); scan != scanned.end(); ++scan) {
    if (!scan->expired()) {
      ++found;
    }
  }

  for (auto scan = scanned_new.begin(); scan != scanned_new.end(); ++scan) {
    if (!scan->expired()) {
      ++found;
    }
  }

  if (found == 0) {
    if (scanCompleteStatus == -2) {
      // Not triggered
      return scanCompleteStatus;
    }
  }
  return found;
}

bool WiFi_AP_CandidatesList::SettingsIndexMatchCustomCredentials(uint8_t index)
{
  return WIFI_CUSTOM_DEPLOYMENT_KEY_INDEX     == index ||
         WIFI_CUSTOM_SUPPORT_KEY_INDEX        == index ||
         SettingsIndexMatchEmergencyFallback(index);
}

bool WiFi_AP_CandidatesList::SettingsIndexMatchEmergencyFallback(uint8_t index) { return WIFI_CREDENTIALS_FALLBACK_SSID_INDEX == index; }

# if FEATURE_STORE_CREDENTIALS_SEPARATE_FILE

bool WiFi_AP_CandidatesList::SettingsIndexMatchCredentialsSeparateFile(uint8_t index)
{
  return index >= WIFI_CREDENTIALs_SEPARATE_FILE_FIRST_INDEX &&
         index < WIFI_CREDENTIALS_MAX_INDEX;
}

# endif // if FEATURE_STORE_CREDENTIALS_SEPARATE_FILE

void WiFi_AP_CandidatesList::loadCandidatesFromScanned() {
  // Make sure list operations are not done on the 2nd heap
  # ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  # endif // ifdef USE_SECOND_HEAP

  if (scanned_new.size() > 0) {
    // We have new scans to process.
    purge_expired();

    for (auto scan = scanned_new.begin(); scan != scanned_new.end();) {
      # ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        addLogMove(LOG_LEVEL_DEBUG, concat(F("WiFi : Scan result: "), scan->toString()));
      }
      # endif // ifndef BUILD_NO_DEBUG

      // Check to see if it is already present, if so, remove existing one.
      for (auto tmp = scanned.begin(); tmp != scanned.end();) {
        if (*tmp == *scan) {
          tmp = scanned.erase(tmp);
        } else {
          ++tmp;
        }
      }

      // We copy instead of move, to make sure it is stored on the 2nd heap.
      scanned.push_back(*scan);
      scan = scanned_new.erase(scan);
    }
    scanned.sort();
    scanned.unique();
  }

  if (candidates.size() > 1) {
    // Do not mess with the current candidates order if > 1 present
    return;
  }

  // Purge unusable from known list.
  for (auto it = known.begin(); it != known.end();) {
    if (it->usable()) {
      ++it;
    } else {
      it = known.erase(it);
    }
  }
  known.sort();
  known.unique();
  known_it = known.begin();

  for (auto scan = scanned.begin(); scan != scanned.end();) {
    if (scan->expired()) {
      scan = scanned.erase(scan);
    } else {
      if (scan->bits.isHidden) {
        if (Settings.IncludeHiddenSSID()) {
          if (WiFi_AP_CandidatesList::hasWiFiCredentials()) {
            candidates.push_back(*scan);
          }
        }
      } else if (scan->ssid.length() > 0) {
        for (auto kn_it = known.begin(); kn_it != known.end(); ++kn_it) {
          if (scan->ssid.equals(kn_it->ssid)) {
            WiFi_AP_Candidate tmp(*scan);
            tmp.index                    = kn_it->index;
            tmp.bits.lowPriority         = kn_it->bits.lowPriority;
            tmp.bits.isEmergencyFallback = kn_it->bits.isEmergencyFallback;

            if (tmp.usable()) {
              candidates.push_back(tmp);
              _addedKnownCandidate = true;

              // Check all knowns as we may have several AP's with the same SSID and different passwords.
            }
          }
        }
      }
      ++scan;
    }
  }
  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    const WiFi_AP_Candidate bestCandidate = getBestCandidate();

    if (bestCandidate.usable()) {
      addLogMove(LOG_LEVEL_INFO, concat(F("WiFi : Best AP candidate: "), bestCandidate.toString()));
    }
  }
  # endif // ifndef BUILD_NO_DEBUG
  candidates.sort();
  candidates.unique();
  addFromRTC();
  purge_unusable();
}

void WiFi_AP_CandidatesList::addFromRTC() {
  if (!Settings.UseLastWiFiFromRTC()) { return; }

  if (!RTC.lastWiFi_set()) {
    # ifdef ESP32

    // Try to load from NVS and store in RTC
    WiFi_AP_Candidate fromNVS;

    if (WiFi_AP_Candidates_NVS::loadCandidate_from_NVS(fromNVS)) {
      RTC.lastWiFiChannel = currentCandidate.channel;
      currentCandidate.bssid.get(RTC.lastBSSID);
      RTC.lastWiFiSettingsIndex = currentCandidate.index;
    } else {
      return;
    }
    # else // ifdef ESP32
    return;
    # endif // ifdef ESP32
  }

  if (SettingsIndexMatchCustomCredentials(RTC.lastWiFiSettingsIndex))
  {
    return;
  }

  String ssid;

  if (!get_SSID(RTC.lastWiFiSettingsIndex, ssid)) {
    return;
  }

  WiFi_AP_Candidate fromRTC(RTC.lastWiFiSettingsIndex, ssid);
  fromRTC.bssid   = RTC.lastBSSID;
  fromRTC.channel = RTC.lastWiFiChannel;

  if (!fromRTC.usable()) {
    return;
  }

  if ((candidates.size() > 0) && candidates.front().ssid.equals(fromRTC.ssid)) {
    // Front candidate was already from RTC.
    // Copy all flags, before removing it from list
    fromRTC = candidates.front();
    fromRTC.index = RTC.lastWiFiSettingsIndex;
    candidates.pop_front();
  }

  // See if we may have a better candidate for the current network, with a significant better RSSI.
  auto bestMatch = candidates.end();
  auto lastUsed  = bestMatch;

  for (auto it = candidates.begin(); lastUsed == candidates.end() && it != candidates.end(); ++it) {
    if (it->usable() && it->ssid.equals(fromRTC.ssid)) {
      const bool foundLastUsed = fromRTC.bssid_match(it->bssid);

      if (foundLastUsed) {
        lastUsed = it;
      } else if (bestMatch == candidates.end()) {
        bestMatch = it;
      }
    }
  }
  bool matchAdded = false;

  if (bestMatch != candidates.end()) {
    // Found a best match, possibly better than the last used.
    if ((lastUsed == candidates.end()) || (bestMatch->rssi > (lastUsed->rssi + 10))) {
      // Last used was not found or
      // Other candidate has significant better RSSI
      matchAdded = true;
      candidates.push_front(*bestMatch);
    }
  } else if (lastUsed != candidates.end()) {
    matchAdded = true;
    candidates.push_front(*lastUsed);
  }

  if (!matchAdded) {
    candidates.push_front(fromRTC);

    // This is not taken from a scan, so no idea of the used encryption.
    // Try to find a matching BSSID to get the encryption.
    for (auto it = candidates.begin(); it != candidates.end(); ++it) {
      if ((it->rssi != -1) && (candidates.front() == *it)) {
        candidates.front().enc_type = it->enc_type;
        return;
      }
    }
  }

  candidates.front().rssi = -1; // Set to best possible RSSI so it is tried first.

  if (!candidates.front().usable() || !candidates.front().allowQuickConnect()) {
    candidates.pop_front();
    return;
  }

  if (currentCandidate == candidates.front()) {
    candidates.front().enc_type = currentCandidate.enc_type;
  }
}

void WiFi_AP_CandidatesList::purge_unusable() {
  for (auto it = candidates.begin(); it != candidates.end();) {
    if (it->usable()) {
      ++it;
    } else {
      it = candidates.erase(it);
    }
  }

  if (candidates.size() > 1) {
    candidates.sort();
    candidates.unique();
  }
}

bool WiFi_AP_CandidatesList::get_SSID_key(uint8_t index, String& ssid, String& key)  {
  ssid.clear();
  key.clear();

  switch (index)
  {
    case 1:
      ssid = SecuritySettings.getSSID(SecurityStruct::WiFiCredentialsSlot::first);
      key  = SecuritySettings.WifiKey;
      break;
    case 2:
      ssid = SecuritySettings.getSSID(SecurityStruct::WiFiCredentialsSlot::second);
      key  = SecuritySettings.WifiKey2;
      break;
    case WIFI_CUSTOM_DEPLOYMENT_KEY_INDEX:
      # if !defined(CUSTOM_DEPLOYMENT_SSID) || !defined(CUSTOM_DEPLOYMENT_KEY)
      return false;
      # else
      ssid = F(CUSTOM_DEPLOYMENT_SSID);
      key  = F(CUSTOM_DEPLOYMENT_KEY);
      # endif // if !defined(CUSTOM_DEPLOYMENT_SSID) || !defined(CUSTOM_DEPLOYMENT_KEY)
      break;
    case WIFI_CUSTOM_SUPPORT_KEY_INDEX:
      # if !defined(CUSTOM_SUPPORT_SSID) || !defined(CUSTOM_SUPPORT_KEY)
      return false;
      # else
      ssid = F(CUSTOM_SUPPORT_SSID);
      key  = F(CUSTOM_SUPPORT_KEY);
      # endif // if !defined(CUSTOM_SUPPORT_SSID) || !defined(CUSTOM_SUPPORT_KEY)
      break;
    case WIFI_CREDENTIALS_FALLBACK_SSID_INDEX:
    {
      # if !defined(CUSTOM_EMERGENCY_FALLBACK_SSID) || !defined(CUSTOM_EMERGENCY_FALLBACK_KEY)
      return false;
      # else
      int allowedUptimeMinutes = 10;
      #  ifdef CUSTOM_EMERGENCY_FALLBACK_ALLOW_MINUTES_UPTIME
      allowedUptimeMinutes = CUSTOM_EMERGENCY_FALLBACK_ALLOW_MINUTES_UPTIME;
      #  endif

      if ((getUptimeMinutes() < allowedUptimeMinutes) && SecuritySettings.hasWiFiCredentials()) {
        ssid = F(CUSTOM_EMERGENCY_FALLBACK_SSID);
        key  = F(CUSTOM_EMERGENCY_FALLBACK_KEY);
      } else {
        return false;
      }
      # endif // if !defined(CUSTOM_EMERGENCY_FALLBACK_SSID) || !defined(CUSTOM_EMERGENCY_FALLBACK_KEY)
      break;
    }
    default: break;
  }

# if FEATURE_STORE_CREDENTIALS_SEPARATE_FILE

  if (SettingsIndexMatchCredentialsSeparateFile(index))
  {
    return SecuritySettings_deviceSpecific.getWiFiCredentials(
      index - WIFI_CREDENTIALs_SEPARATE_FILE_FIRST_INDEX,
      ssid,
      key);
  }
# endif // if FEATURE_STORE_CREDENTIALS_SEPARATE_FILE

  // Spaces are allowed in both SSID and pass phrase, so make sure to not trim the ssid and key.
  return !ssid.isEmpty() && key.length() >= 8;
}

bool WiFi_AP_CandidatesList::hasWiFiCredentials()
{
  uint8_t index = 1; // Index 0 is the "unset" value

  for (; index < WIFI_CREDENTIALS_MAX_INDEX; ++index) {
    String ssid;
    if (get_SSID(index, ssid)) return true;
  }
  return false;
}

bool WiFi_AP_CandidatesList::get_SSID(uint8_t index, String& ssid)
{
  String key;

  return get_SSID_key(index, ssid, key);
}

String WiFi_AP_CandidatesList::get_key(uint8_t index)
{
  String ssid, key;

  if (get_SSID_key(index, ssid, key)) {
    return key;
  }
  return EMPTY_STRING;
}

} // namespace wifi
} // namespace net
} // namespace ESPEasy

#endif // if FEATURE_WIFI
