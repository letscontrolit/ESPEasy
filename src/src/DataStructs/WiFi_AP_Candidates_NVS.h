#ifndef DATASTRUCTS_WIFI_AP_CANDIDATES_NVS_H
#define DATASTRUCTS_WIFI_AP_CANDIDATES_NVS_H

#include "../../ESPEasy_common.h"

#ifdef ESP32
#include "../DataStructs/WiFi_AP_Candidate.h"

class WiFi_AP_Candidates_NVS {
public:

  static bool loadCandidate_from_NVS(WiFi_AP_Candidate& candidate);

  static void currentConnection_to_NVS(const WiFi_AP_Candidate& candidate);

  static void clear_from_NVS();
};

#endif
#endif