#ifndef HELPERS_WIFI_AP_CANDIDATESLIST_H
#define HELPERS_WIFI_AP_CANDIDATESLIST_H

#include "../DataStructs/WiFi_AP_Candidate.h"

#include <list>

struct WiFi_AP_CandidatesList {
  WiFi_AP_CandidatesList();

  // Load the known credentials from the settings
  void load_knownCredentials();

  void clearCache() {
    mustLoadCredentials = true;
  }

  // Add found WiFi access points to the list if they are possible candidates.
  void process_WiFiscan(uint8_t scancount);

  // Get the next candidate to connect
  // Return true when a valid next candidate was found.
  bool                     getNext();

  const WiFi_AP_Candidate& getCurrent() const;

  WiFi_AP_Candidate        getBestScanResult() const;

  bool                     hasKnownCredentials();

private:

  // Add item from WiFi scan.
  void add(uint8_t networkItem);

  void addFromRTC();

  void purge_unusable();

  // Load SSID and pass/key from the settings.
  bool get_SSID_key(byte    index,
                    String& ssid,
                    String& key) const;

  std::list<WiFi_AP_Candidate>candidates;

  std::list<WiFi_AP_Candidate>known;

  std::list<WiFi_AP_Candidate>::const_iterator known_it;

  WiFi_AP_Candidate currentCandidate;

  bool mustLoadCredentials = true;
};

#endif // ifndef HELPERS_WIFI_AP_CANDIDATESLIST_H
