#include "../DataStructs/WiFi_AP_Candidates_NVS.h"

#ifdef ESP32
# include "../Helpers/ESPEasy_NVS_Helper.h"

# define WIFI_AP_CANDIDATE_NVS_KEY "WIFICANDIDATE"

struct WiFi_AP_Candidates_NVS_data_t {
  union {
    struct {
      uint8_t BSSID[6];
      uint8_t lastWiFiChannel;
      uint8_t lastWiFiSettingsIndex;
    } APdata;

    uint64_t rawdata{};
  };
};

bool WiFi_AP_Candidates_NVS::loadCandidate_from_NVS(WiFi_AP_Candidate& candidate)
{
  WiFi_AP_Candidates_NVS_data_t fromNVS;

  {
    ESPEasy_NVS_Helper preferences;

    if (!preferences.begin(F(WIFI_CONNECTION_NVS_NAMESPACE))) {
      return false;
    }

    if (!preferences.getPreference(F(WIFI_AP_CANDIDATE_NVS_KEY), fromNVS.rawdata)) {
      return false;
    }
  }
  candidate.bssid.set(fromNVS.APdata.BSSID);
  candidate.channel = fromNVS.APdata.lastWiFiChannel;
  candidate.index   = fromNVS.APdata.lastWiFiSettingsIndex;
  return true;
}

void WiFi_AP_Candidates_NVS::currentConnection_to_NVS(const WiFi_AP_Candidate& candidate)
{
  ESPEasy_NVS_Helper preferences;

  preferences.begin(F(WIFI_CONNECTION_NVS_NAMESPACE));
  WiFi_AP_Candidates_NVS_data_t toNVS;

  candidate.bssid.get(toNVS.APdata.BSSID);
  toNVS.APdata.lastWiFiChannel       = candidate.channel;
  toNVS.APdata.lastWiFiSettingsIndex = candidate.index;

  preferences.setPreference(F(WIFI_AP_CANDIDATE_NVS_KEY), toNVS.rawdata);
}

void WiFi_AP_Candidates_NVS::clear_from_NVS()
{
  ESPEasy_NVS_Helper preferences;

  if (preferences.begin(F(WIFI_CONNECTION_NVS_NAMESPACE))) {
    preferences.remove(F(WIFI_AP_CANDIDATE_NVS_KEY));
  }
}

#endif // ifdef ESP32
