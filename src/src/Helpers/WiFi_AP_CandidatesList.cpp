#include "../Helpers/WiFi_AP_CandidatesList.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/RTC.h"
#include "../Globals/SecuritySettings.h"
#include "../Globals/Settings.h"
#include "../Helpers/Misc.h"


#if defined(ESP8266)
  # include <ESP8266WiFi.h>
#endif // if defined(ESP8266)
#if defined(ESP32)
  # include <WiFi.h>
#endif // if defined(ESP32)

#define WIFI_CUSTOM_DEPLOYMENT_KEY_INDEX     3
#define WIFI_CUSTOM_SUPPORT_KEY_INDEX        4
#define WIFI_CREDENTIALS_FALLBACK_SSID_INDEX 5

WiFi_AP_CandidatesList::WiFi_AP_CandidatesList() {
  known.clear();
  candidates.clear();
  known_it = known.begin();
}

WiFi_AP_CandidatesList::~WiFi_AP_CandidatesList() {
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
//  attemptsLeft = 1;
  _addedKnownCandidate = false;
//  addFromRTC();

  {
    // Add the known SSIDs
    String ssid;
    uint8_t   index = 1; // Index 0 is the "unset" value

    bool done = false;

    while (!done) {
      if (get_SSID(index, ssid)) {
        known.emplace_back(index, ssid);
        if (SettingsIndexMatchCustomCredentials(index)) {
          if (SettingsIndexMatchEmergencyFallback(index)) {
            known.back().isEmergencyFallback = true;
          } else {
            known.back().lowPriority = true;
          }
        }
        ++index;
      } else {
        if (SettingsIndexMatchCustomCredentials(index)) {
          ++index;
        } else {
          done = true;
        }
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
  loadCandidatesFromScanned();
}

void WiFi_AP_CandidatesList::begin_sync_scan() {
  candidates.clear();
  _addedKnownCandidate = false;
}

void WiFi_AP_CandidatesList::purge_expired() {
  for (auto it = scanned.begin(); it != scanned.end(); ) {
    if (it->expired()) {
      it = scanned.erase(it);
    } else {
      ++it;
    }
  }
}

#if !FEATURE_ESP8266_DIRECT_WIFI_SCAN
void WiFi_AP_CandidatesList::process_WiFiscan(uint8_t scancount) {
  // Append or update found APs from scan.
  for (uint8_t i = 0; i < scancount; ++i) {
    const WiFi_AP_Candidate tmp(i);

    scanned_new.push_back(tmp);
  }

  after_process_WiFiscan();
}
#endif

#ifdef ESP8266
#if FEATURE_ESP8266_DIRECT_WIFI_SCAN
void WiFi_AP_CandidatesList::process_WiFiscan(const bss_info& ap) {
  WiFi_AP_Candidate tmp(ap);
  scanned_new.push_back(tmp);
}
#endif
#endif

void WiFi_AP_CandidatesList::after_process_WiFiscan() {
  scanned_new.sort();
  scanned_new.unique();
  _mustLoadCredentials = true;
  WiFi.scanDelete();
  attemptsLeft = 1;
}

bool WiFi_AP_CandidatesList::getNext(bool scanAllowed) {
  load_knownCredentials();

  if (candidates.empty()) { 
    if (scanAllowed) {
      return false;
    }
    loadCandidatesFromScanned();
    attemptsLeft = 1;
    if (candidates.empty()) { return false; }
  }

  currentCandidate = candidates.front();
  bool mustPop = true;

  if (currentCandidate.isHidden) {
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
      if (currentCandidate.isHidden) {
        // We tried to connect to hidden SSIDs in 1 run, so pop all hidden candidates.
        for (auto cand_it = candidates.begin(); cand_it != candidates.end() && cand_it->isHidden; ) {
          cand_it = candidates.erase(cand_it);
        }
      } else {
        if (!candidates.empty()) {
          candidates.pop_front();
        }
      }

      known_it = known.begin();
      attemptsLeft = 1;
    } else {
      markAttempt();
    }
  }
  return currentCandidate.usable();
}

const WiFi_AP_Candidate& WiFi_AP_CandidatesList::getCurrent() const {
  return currentCandidate;
}

void WiFi_AP_CandidatesList::markAttempt() {
  if (attemptsLeft > 0) attemptsLeft--;
}

WiFi_AP_Candidate WiFi_AP_CandidatesList::getBestCandidate() const {
  for (auto it = candidates.begin(); it != candidates.end(); ++it) {
    if (it->rssi < -1) { return *it; }
  }
  return WiFi_AP_Candidate();
}

bool WiFi_AP_CandidatesList::hasKnownCredentials() {
  load_knownCredentials();
  return !known.empty();
}

bool WiFi_AP_CandidatesList::hasCandidates() const {
  return !candidates.empty();
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
        matchfound = true;
      }
    }
  }
  if (currentCandidate.usable()) {
    // Store in RTC
    RTC.lastWiFiChannel = currentCandidate.channel;
    currentCandidate.bssid.get(RTC.lastBSSID);
    RTC.lastWiFiSettingsIndex = currentCandidate.index;
  }

  candidates.clear();
  _addedKnownCandidate = false;
  addFromRTC(); // Store the current one from RTC as the first candidate for a reconnect.
}

int8_t WiFi_AP_CandidatesList::scanComplete() const {
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
  if (found > 0) {    
    return found;
  }
  const int8_t scanCompleteStatus = WiFi.scanComplete();
  if (scanCompleteStatus <= 0) {
    return scanCompleteStatus;
  }
  return 0;
}

bool WiFi_AP_CandidatesList::SettingsIndexMatchCustomCredentials(uint8_t index)
{
  return (WIFI_CUSTOM_DEPLOYMENT_KEY_INDEX     == index ||
          WIFI_CUSTOM_SUPPORT_KEY_INDEX        == index ||
          SettingsIndexMatchEmergencyFallback(index));
}

bool WiFi_AP_CandidatesList::SettingsIndexMatchEmergencyFallback(uint8_t index)
{
  return (WIFI_CREDENTIALS_FALLBACK_SSID_INDEX == index);
}


void WiFi_AP_CandidatesList::loadCandidatesFromScanned() {
  if (scanned_new.size() > 0) {
    // We have new scans to process.
    #ifdef USE_SECOND_HEAP
    HeapSelectIram ephemeral;
    // TD-er: Disabled for now as it is suspect for crashes
    #endif
    purge_expired();
    for (auto scan = scanned_new.begin(); scan != scanned_new.end();) {
      #ifndef BUILD_NO_DEBUG
      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        String log = F("WiFi : Scan result: ");
        log += scan->toString();
        addLogMove(LOG_LEVEL_DEBUG, log);
      }
      #endif // ifndef BUILD_NO_DEBUG

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
      if (scan->isHidden) {
        if (Settings.IncludeHiddenSSID()) {
          if (SecuritySettings.hasWiFiCredentials()) {
            candidates.push_back(*scan);
          }
        }
      } else if (scan->ssid.length() > 0) {
        for (auto kn_it = known.begin(); kn_it != known.end(); ++kn_it) {
          if (scan->ssid.equals(kn_it->ssid)) {
            WiFi_AP_Candidate tmp = *scan;
            tmp.index = kn_it->index;
            tmp.lowPriority = kn_it->lowPriority;
            tmp.isEmergencyFallback = kn_it->isEmergencyFallback;

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
      String log = F("WiFi : Best AP candidate: ");
      log += bestCandidate.toString();
      addLogMove(LOG_LEVEL_INFO, log);
    }
  }
  #endif
  candidates.sort();
  candidates.unique();
  addFromRTC();
  purge_unusable();
}

void WiFi_AP_CandidatesList::addFromRTC() {
  if (!Settings.UseLastWiFiFromRTC() || !RTC.lastWiFi_set()) { return; }

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

  if (candidates.size() > 0 && candidates.front().ssid.equals(fromRTC.ssid)) {
    // Front candidate was already from RTC.
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
    if (lastUsed == candidates.end() || (bestMatch->rssi > (lastUsed->rssi + 10))) {
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
      if ((it->rssi != -1) && candidates.front() == *it) {
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
  switch (index) {
    case 1:
      ssid = SecuritySettings.WifiSSID;
      key  = SecuritySettings.WifiKey;
      break;
    case 2:
      ssid = SecuritySettings.WifiSSID2;
      key  = SecuritySettings.WifiKey2;
      break;
    case WIFI_CUSTOM_DEPLOYMENT_KEY_INDEX:
      #if !defined(CUSTOM_DEPLOYMENT_SSID) || !defined(CUSTOM_DEPLOYMENT_KEY)
      return false;
      #else
      ssid = F(CUSTOM_DEPLOYMENT_SSID);
      key  = F(CUSTOM_DEPLOYMENT_KEY);
      #endif
      break;
    case WIFI_CUSTOM_SUPPORT_KEY_INDEX:
      #if !defined(CUSTOM_SUPPORT_SSID) || !defined(CUSTOM_SUPPORT_KEY)
      return false;
      #else
      ssid = F(CUSTOM_SUPPORT_SSID);
      key  = F(CUSTOM_SUPPORT_KEY);
      #endif
      break;
    case WIFI_CREDENTIALS_FALLBACK_SSID_INDEX:
    {
      #if !defined(CUSTOM_EMERGENCY_FALLBACK_SSID) || !defined(CUSTOM_EMERGENCY_FALLBACK_KEY)
      return false;
      #else
      int allowedUptimeMinutes = 10;
      #ifdef CUSTOM_EMERGENCY_FALLBACK_ALLOW_MINUTES_UPTIME
      allowedUptimeMinutes = CUSTOM_EMERGENCY_FALLBACK_ALLOW_MINUTES_UPTIME;
      #endif
      if (getUptimeMinutes() < allowedUptimeMinutes && SecuritySettings.hasWiFiCredentials()) {
        ssid = F(CUSTOM_EMERGENCY_FALLBACK_SSID);
        key  = F(CUSTOM_EMERGENCY_FALLBACK_KEY);
      } else {
        return false;
      }
      #endif
      break;
    }
    default:
      return false;
  }

  // TODO TD-er: Read other credentials from extra file.



  // Spaces are allowed in both SSID and pass phrase, so make sure to not trim the ssid and key.
  return true;
}

bool WiFi_AP_CandidatesList::get_SSID(uint8_t index, String& ssid)
{
  String key;
  return get_SSID_key(index, ssid, key);
}

String WiFi_AP_CandidatesList::get_key(uint8_t index)
{
  String ssid, key;
  if (get_SSID_key(index, ssid, key))
    return key;
  return EMPTY_STRING;
}