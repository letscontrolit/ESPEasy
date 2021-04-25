#include "../Helpers/WiFi_AP_CandidatesList.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/RTC.h"
#include "../Globals/SecuritySettings.h"
#include "../Globals/Settings.h"
#include "../../ESPEasy_common.h"
#include "../../ESPEasy_fdwdecl.h"

#define WIFI_CUSTOM_DEPLOYMENT_KEY_INDEX     3
#define WIFI_CUSTOM_SUPPORT_KEY_INDEX        4
#define WIFI_CREDENTIALS_FALLBACK_SSID_INDEX 5

WiFi_AP_CandidatesList::WiFi_AP_CandidatesList() {
  known.clear();
  candidates.clear();
  known_it = known.begin();
  load_knownCredentials();
}

void WiFi_AP_CandidatesList::load_knownCredentials() {
  if (!_mustLoadCredentials) { return; }
  _mustLoadCredentials = false;
  known.clear();
  candidates.clear();

  {
    // Add the known SSIDs
    String ssid, key;
    byte   index = 1; // Index 0 is the "unset" value

    bool done = false;

    while (!done) {
      if (get_SSID_key(index, ssid, key)) {
        known.emplace_back(index, ssid, key);
        if (index == WIFI_CUSTOM_DEPLOYMENT_KEY_INDEX || 
            index == WIFI_CUSTOM_SUPPORT_KEY_INDEX) {
          known.back().lowPriority = true;
        } else if (index == WIFI_CREDENTIALS_FALLBACK_SSID_INDEX) {
          known.back().isEmergencyFallback = true;
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

void WiFi_AP_CandidatesList::process_WiFiscan(uint8_t scancount) {
  // Append or update found APs from scan.
  for (uint8_t i = 0; i < scancount; ++i) {
    const WiFi_AP_Candidate tmp(i);
    // Remove previous scan result if present
    for (auto it = scanned.begin(); it != scanned.end(); ) {
      if (tmp == *it || it->expired()) {
        it = scanned.erase(it);
      } else {
        ++it;
      }
    }
//    if (Settings.IncludeHiddenSSID() || !tmp.isHidden) {
      scanned.push_back(tmp);
      #ifndef BUILD_NO_DEBUG
      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        String log = F("WiFi : Scan result: ");
        log += tmp.toString();
        addLog(LOG_LEVEL_DEBUG, log);
      }
      #endif // ifndef BUILD_NO_DEBUG
//    }
  }
  scanned.sort();
  loadCandidatesFromScanned();
  WiFi.scanDelete();
}

bool WiFi_AP_CandidatesList::getNext(bool scanAllowed) {
  load_knownCredentials();

  if (candidates.empty()) { 
    if (scanAllowed) {
      return false;
    }
    loadCandidatesFromScanned();
    if (candidates.empty()) { return false; }
  }

  bool mustPop = true;

  currentCandidate = candidates.front();

  if (currentCandidate.isHidden) {
    // Iterate over the known credentials to try them all
    // Hidden SSID stations do not broadcast their SSID, so we must fill it in ourselves.
    if (known_it != known.end()) {
      currentCandidate.ssid  = known_it->ssid;
      currentCandidate.key   = known_it->key;
      currentCandidate.index = known_it->index;
      ++known_it;
    }

    if (known_it != known.end()) {
      mustPop = false;
    }
  }


  if (currentCandidate.usable()) {
    // Store in RTC
    RTC.lastWiFiChannel = currentCandidate.channel;

    for (byte i = 0; i < 6; ++i) {
      RTC.lastBSSID[i] = currentCandidate.bssid[i];
    }
    RTC.lastWiFiSettingsIndex = currentCandidate.index;
  }


  if (mustPop) {
    known_it = known.begin();
    if (!candidates.empty()) {
      candidates.pop_front();
    }
  }
  return currentCandidate.usable();
}

const WiFi_AP_Candidate& WiFi_AP_CandidatesList::getCurrent() const {
  return currentCandidate;
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
  candidates.clear();
  addFromRTC(); // Store the current one from RTC as the first candidate for a reconnect.
}

int8_t WiFi_AP_CandidatesList::scanComplete() const {
  size_t found = 0;
  for (auto scan = scanned.begin(); scan != scanned.end(); ++scan) {
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
          WIFI_CREDENTIALS_FALLBACK_SSID_INDEX == index);
}

void WiFi_AP_CandidatesList::loadCandidatesFromScanned() {
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
            tmp.key   = kn_it->key;
            tmp.index = kn_it->index;
            tmp.lowPriority = kn_it->lowPriority;
            tmp.isEmergencyFallback = kn_it->isEmergencyFallback;

            if (tmp.usable()) {
              candidates.push_back(tmp);

              // Check all knowns as we may have several AP's with the same SSID and different passwords.
            }
          }
        }
      }
      ++scan;
    }
  }
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    const WiFi_AP_Candidate bestCandidate = getBestCandidate();
    if (bestCandidate.usable()) {
      String log = F("WiFi : Best AP candidate: ");
      log += bestCandidate.toString();
      addLog(LOG_LEVEL_INFO, log);
    }
  }
  candidates.sort();
  addFromRTC();
  purge_unusable();
}

void WiFi_AP_CandidatesList::addFromRTC() {
  if (!RTC.lastWiFi_set()) { return; }

  if (SettingsIndexMatchCustomCredentials(RTC.lastWiFiSettingsIndex)) 
  { 
    return;
  }

  String ssid, key;

  if (!get_SSID_key(RTC.lastWiFiSettingsIndex, ssid, key)) {
    return;
  }

  WiFi_AP_Candidate fromRTC(RTC.lastWiFiSettingsIndex, ssid, key);
  fromRTC.setBSSID(RTC.lastBSSID);
  fromRTC.channel = RTC.lastWiFiChannel;

  if (candidates.size() > 0 && candidates.front().ssid.equals(fromRTC.ssid)) {
    // Front candidate was already from RTC.
    candidates.pop_front();
  }

  // See if we may have a better candidate for the current network, with a significant better RSSI.
  auto bestMatch = candidates.end();
  auto lastUsed  = candidates.end();
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
  candidates.sort();
  candidates.unique();
}

bool WiFi_AP_CandidatesList::get_SSID_key(byte index, String& ssid, String& key) const {
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
