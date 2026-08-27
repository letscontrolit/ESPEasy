#pragma once

#include "../../../ESPEasy_common.h"

#include "../DataStructs/WiFi_AP_Candidate.h"


#define WiFi_CONNECT_ATTEMPTS  1

#if FEATURE_WIFI

namespace ESPEasy {
namespace net {
namespace wifi {


struct WiFi_AP_CandidatesList {
  WiFi_AP_CandidatesList();

  ~WiFi_AP_CandidatesList();

  // Load the known credentials from the settings
  void load_knownCredentials();

  void clearCache();

  // Called after WiFi credentials have changed.
  void force_reload();

  void begin_scan(uint8_t channel = 0);

  void purge_expired();

  // Add found WiFi access points to the list if they are possible candidates.
  void process_WiFiscan();

  void after_process_WiFiscan();

  // Get the next candidate to connect
  // Return true when a valid next candidate was found.
  bool                     getNext(bool scanAllowed);

  const WiFi_AP_Candidate& getCurrent() const;

  // Decrease attemptsLeft
  void                     markAttempt();

  WiFi_AP_Candidate        getBestCandidate() const;

  bool                     hasCandidateCredentials();

  bool                     hasCandidates() const;

  bool                     hasScanned() const { return scanned.size() != 0; }

  // Make sure the current connection (from RTC) is set as first next candidate.
  // This will force a reconnect to the current AP if connection is lost.
  void                             markCurrentConnectionStable();

  bool                             addedKnownCandidate() const { return _addedKnownCandidate; }

  // Return number of found access points
  // @retval -1: WIFI_SCAN_RUNNING
  // @retval -2: WIFI_SCAN_FAILED
  // @retval -3: No scan performed (recently)
  int8_t                           scanComplete() const;

  WiFi_AP_Candidate_const_iterator scanned_begin() const {
    return scanned.begin();
  }

  WiFi_AP_Candidate_const_iterator scanned_end() const {
    return scanned.end();
  }

  static bool SettingsIndexMatchCustomCredentials(uint8_t index);

  static bool SettingsIndexMatchEmergencyFallback(uint8_t index);
#if FEATURE_STORE_CREDENTIALS_SEPARATE_FILE
  static bool SettingsIndexMatchCredentialsSeparateFile(uint8_t index);
#endif

private:

  // Pick the possible
  void        loadCandidatesFromScanned();

  void        addFromRTC();

  void        purge_unusable();

  // Load SSID and pass/key from the settings.
  static bool get_SSID_key(uint8_t index,
                           String& ssid,
                           String& key);

  static bool hasWiFiCredentials();

public:

  static bool   get_SSID(uint8_t index,
                         String& ssid);

  static String get_key(uint8_t index);

private:

  // Known credentials supplying the following:
  // - Credentials index
  // - Priority
  // - SSID
  // - Key
  WiFi_AP_Candidate_List_t known;

  // Unprocessed found from latest scan
  WiFi_AP_Candidate_List_t scanned_new;

  // Processed found via one or more scans
  // When processing scanned_new, matching elements from a previous scan (in 'scanned')
  // will be updated with new information, like channel, RSSI, supported protocols, etc.
  WiFi_AP_Candidate_List_t scanned;

  // Sorted list of possible APs to connect to. Matching known and scanned.
  // First element in the candidates list is the best candidate to try first.
  // Failed connect attempts will be removed from the candidates list.
  WiFi_AP_Candidate_List_t candidates;

  WiFi_AP_Candidate_const_iterator known_it;

  WiFi_AP_Candidate currentCandidate;

  bool _mustLoadCredentials = true;
  bool _addedKnownCandidate = false;
  uint8_t _last_scan_channel{};

public:

  int attemptsLeft = WiFi_CONNECT_ATTEMPTS;

};

} // namespace wifi
} // namespace net
} // namespace ESPEasy

#endif // if FEATURE_WIFI
