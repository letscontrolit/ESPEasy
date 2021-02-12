#include "../Helpers/WiFi_AP_CandidatesList.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/RTC.h"
#include "../Globals/SecuritySettings.h"
#include "../Globals/Settings.h"

WiFi_AP_CandidatesList::WiFi_AP_CandidatesList() {
  known_it = known.begin();
  load_knownCredentials();
}

void WiFi_AP_CandidatesList::load_knownCredentials() {
  if (!mustLoadCredentials) { return; }
  mustLoadCredentials = false;
  known.clear();
  candidates.clear();
  addFromRTC();

  {
    // Add the known SSIDs
    String ssid, key;
    byte   index = 1; // Index 0 is the "unset" value

    while (get_SSID_key(index, ssid, key)) {
      known.emplace_back(index, ssid, key);
      ++index;
    }
  }
  known_it = known.begin();
  purge_unusable();
}

void WiFi_AP_CandidatesList::clearCache() {
  mustLoadCredentials = true;
  known.clear();
  known_it = known.begin();
}


void WiFi_AP_CandidatesList::force_reload() {
  clearCache();
  RTC.clearLastWiFi(); // Invalidate the RTC WiFi data.
  process_WiFiscan(WiFi.scanComplete());
}

void WiFi_AP_CandidatesList::process_WiFiscan(uint8_t scancount) {
  load_knownCredentials();
  candidates.clear();

  known_it = known.begin();

  // Now try to merge the known SSIDs, or add a new one if it is a hidden SSID
  for (uint8_t i = 0; i < scancount; ++i) {
    add(i);
  }
  addFromRTC();
  purge_unusable();

#ifndef BUILD_NO_DEBUG

  for (auto it = candidates.begin(); it != candidates.end(); ++it) {
    String log = F("WIFI  : Scan result: ");
    log += it->toString();
    addLog(LOG_LEVEL_INFO, log);
  }
#endif // ifndef BUILD_NO_DEBUG
}

bool WiFi_AP_CandidatesList::getNext() {
  if (candidates.empty()) { return false; }

  load_knownCredentials();

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
  return true;
}

const WiFi_AP_Candidate& WiFi_AP_CandidatesList::getCurrent() const {
  return currentCandidate;
}

WiFi_AP_Candidate WiFi_AP_CandidatesList::getBestScanResult() const {
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

void WiFi_AP_CandidatesList::add(uint8_t networkItem) {
  WiFi_AP_Candidate tmp(networkItem);

  if (tmp.isHidden && Settings.IncludeHiddenSSID()) {
    candidates.push_back(tmp);
    return;
  }

  if (tmp.ssid.length() == 0) { return; }

  for (auto it = known.begin(); it != known.end(); ++it) {
    if (it->ssid.equals(tmp.ssid)) {
      tmp.key   = it->key;
      tmp.index = it->index;

      if (tmp.usable()) {
        candidates.push_back(tmp);

        // Do not return as we may have several AP's with the same SSID and different passwords.
      }
    }
  }
}

void WiFi_AP_CandidatesList::addFromRTC() {
  if (!RTC.lastWiFi_set()) { return; }

  String ssid, key;

  if (!get_SSID_key(RTC.lastWiFiSettingsIndex, ssid, key)) {
    return;
  }

  candidates.emplace_front(RTC.lastWiFiSettingsIndex, ssid, key);
  candidates.front().setBSSID(RTC.lastBSSID);
  candidates.front().rssi = -1; // Set to best possible RSSI so it is tried first.
  candidates.front().channel = RTC.lastWiFiChannel;

  if (!candidates.front().usable() || !candidates.front().allowQuickConnect()) {
    candidates.pop_front();
    return;
  }

  // This is not taken from a scan, so no idea of the used encryption.
  // Try to find a matching BSSID to get the encryption.
  for (auto it = candidates.begin(); it != candidates.end(); ++it) {
    if ((it->rssi != -1) && candidates.front() == *it) {
      candidates.front().enc_type = it->enc_type;
      return;
    }
  }
  if (currentCandidate == candidates.front()) {
    candidates.front().enc_type = currentCandidate.enc_type;
  }
}

void WiFi_AP_CandidatesList::purge_unusable() {
  for (auto it = known.begin(); it != known.end();) {
    if (it->usable()) {
      ++it;
    } else {
      it = known.erase(it);
    }
  }
  known.sort();
  known.unique();

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
      return true;
    case 2:
      ssid = SecuritySettings.WifiSSID2;
      key  = SecuritySettings.WifiKey2;
      return true;
  }

  // TODO TD-er: Read other credentials from extra file.
  return false;
}
