#include "../DataStructs/FactoryDefault_WiFi_NVS.h"

#ifdef ESP32

# include "../Globals/Settings.h"
# include "../Globals/SecuritySettings.h"
# include "../Helpers/StringConverter.h"

// Max. 15 char keys for ESPEasy Factory Default marked keys
# define FACTORY_DEFAULT_NVS_SSID1_KEY      "WIFI_SSID1"
# define FACTORY_DEFAULT_NVS_WPA_PASS1_KEY  "WIFI_PASS1"
# define FACTORY_DEFAULT_NVS_SSID2_KEY      "WIFI_SSID2"
# define FACTORY_DEFAULT_NVS_WPA_PASS2_KEY  "WIFI_PASS2"
# define FACTORY_DEFAULT_NVS_AP_PASS_KEY    "WIFI_AP_PASS"
# define FACTORY_DEFAULT_NVS_WIFI_FLAGS_KEY "WIFI_Flags"


void FactoryDefault_WiFi_NVS::fromSettings() {
  bits.IncludeHiddenSSID              = Settings.IncludeHiddenSSID();
  bits.ApDontForceSetup               = Settings.ApDontForceSetup();
  bits.DoNotStartAP                   = Settings.DoNotStartAP();
  bits.ForceWiFi_bg_mode              = Settings.ForceWiFi_bg_mode();
  bits.WiFiRestart_connection_lost    = Settings.WiFiRestart_connection_lost();
  bits.WifiNoneSleep                  = Settings.WifiNoneSleep();
  bits.gratuitousARP                  = Settings.gratuitousARP();
  bits.UseMaxTXpowerForSending        = Settings.UseMaxTXpowerForSending();
  bits.UseLastWiFiFromRTC             = Settings.UseLastWiFiFromRTC();
  bits.WaitWiFiConnect                = Settings.WaitWiFiConnect();
  bits.SDK_WiFi_autoreconnect         = Settings.SDK_WiFi_autoreconnect();
  bits.HiddenSSID_SlowConnectPerBSSID = Settings.HiddenSSID_SlowConnectPerBSSID();
}

void FactoryDefault_WiFi_NVS::applyToSettings() const {
  Settings.IncludeHiddenSSID(bits.IncludeHiddenSSID);
  Settings.ApDontForceSetup(bits.ApDontForceSetup);
  Settings.DoNotStartAP(bits.DoNotStartAP);
  Settings.ForceWiFi_bg_mode(bits.ForceWiFi_bg_mode);
  Settings.WiFiRestart_connection_lost(bits.WiFiRestart_connection_lost);
  Settings.WifiNoneSleep(bits.WifiNoneSleep);
  Settings.gratuitousARP(bits.gratuitousARP);
  Settings.UseMaxTXpowerForSending(bits.UseMaxTXpowerForSending);
  Settings.UseLastWiFiFromRTC(bits.UseLastWiFiFromRTC);
  Settings.WaitWiFiConnect(bits.WaitWiFiConnect);
  Settings.SDK_WiFi_autoreconnect(bits.SDK_WiFi_autoreconnect);
  Settings.HiddenSSID_SlowConnectPerBSSID(bits.HiddenSSID_SlowConnectPerBSSID);
}

bool FactoryDefault_WiFi_NVS::applyToSettings_from_NVS(ESPEasy_NVS_Helper& preferences) {
  String tmp;

  if (preferences.getPreference(F(FACTORY_DEFAULT_NVS_SSID1_KEY), tmp)) {
    safe_strncpy(SecuritySettings.WifiSSID, tmp, sizeof(SecuritySettings.WifiSSID));
  }

  if (preferences.getPreference(F(FACTORY_DEFAULT_NVS_WPA_PASS1_KEY), tmp)) {
    safe_strncpy(SecuritySettings.WifiKey, tmp, sizeof(SecuritySettings.WifiKey));
  }

  if (preferences.getPreference(F(FACTORY_DEFAULT_NVS_SSID2_KEY), tmp)) {
    safe_strncpy(SecuritySettings.WifiSSID2, tmp, sizeof(SecuritySettings.WifiSSID2));
  }

  if (preferences.getPreference(F(FACTORY_DEFAULT_NVS_WPA_PASS2_KEY), tmp)) {
    safe_strncpy(SecuritySettings.WifiKey2, tmp, sizeof(SecuritySettings.WifiKey2));
  }

  if (preferences.getPreference(F(FACTORY_DEFAULT_NVS_AP_PASS_KEY), tmp)) {
    safe_strncpy(SecuritySettings.WifiAPKey, tmp, sizeof(SecuritySettings.WifiAPKey));
  }


  if (!preferences.getPreference(F(FACTORY_DEFAULT_NVS_WIFI_FLAGS_KEY), data)) {
    return false;
  }

  applyToSettings();
  return true;
}

void FactoryDefault_WiFi_NVS::fromSettings_to_NVS(ESPEasy_NVS_Helper& preferences) {
  fromSettings();
  preferences.setPreference(F(FACTORY_DEFAULT_NVS_WIFI_FLAGS_KEY), data);

  // Store WiFi credentials
  preferences.setPreference(F(FACTORY_DEFAULT_NVS_SSID1_KEY),      String(SecuritySettings.WifiSSID));
  preferences.setPreference(F(FACTORY_DEFAULT_NVS_WPA_PASS1_KEY),  String(SecuritySettings.WifiKey));
  preferences.setPreference(F(FACTORY_DEFAULT_NVS_SSID2_KEY),      String(SecuritySettings.WifiSSID2));
  preferences.setPreference(F(FACTORY_DEFAULT_NVS_WPA_PASS2_KEY),  String(SecuritySettings.WifiKey2));
  preferences.setPreference(F(FACTORY_DEFAULT_NVS_AP_PASS_KEY),    String(SecuritySettings.WifiAPKey));
}

void FactoryDefault_WiFi_NVS::clear_from_NVS(ESPEasy_NVS_Helper& preferences) {
  preferences.remove(F(FACTORY_DEFAULT_NVS_SSID1_KEY));
  preferences.remove(F(FACTORY_DEFAULT_NVS_WPA_PASS1_KEY));
  preferences.remove(F(FACTORY_DEFAULT_NVS_SSID2_KEY));
  preferences.remove(F(FACTORY_DEFAULT_NVS_WPA_PASS2_KEY));
  preferences.remove(F(FACTORY_DEFAULT_NVS_AP_PASS_KEY));
  preferences.remove(F(FACTORY_DEFAULT_NVS_WIFI_FLAGS_KEY));
}

#endif // ifdef ESP32
