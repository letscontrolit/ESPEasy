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

struct FactoryDefault_WiFi_NVS_securityPrefs {
  FactoryDefault_WiFi_NVS_securityPrefs(const __FlashStringHelper *pref,
                                        char                      *dest,
                                        size_t                     size)
    : _pref(pref), _dest(dest), _size(size) {}

  const __FlashStringHelper *_pref;
  char                      *_dest;
  size_t                     _size;
};

const FactoryDefault_WiFi_NVS_securityPrefs _WiFi_NVS_securityPrefs_values[] = {
  { F(FACTORY_DEFAULT_NVS_SSID1_KEY),     SecuritySettings.WifiSSID,  sizeof(SecuritySettings.WifiSSID)  },
  { F(FACTORY_DEFAULT_NVS_WPA_PASS1_KEY), SecuritySettings.WifiKey,   sizeof(SecuritySettings.WifiKey)   },
  { F(FACTORY_DEFAULT_NVS_SSID2_KEY),     SecuritySettings.WifiSSID2, sizeof(SecuritySettings.WifiSSID2) },
  { F(FACTORY_DEFAULT_NVS_WPA_PASS2_KEY), SecuritySettings.WifiKey2,  sizeof(SecuritySettings.WifiKey2)  },
  { F(FACTORY_DEFAULT_NVS_AP_PASS_KEY),   SecuritySettings.WifiAPKey, sizeof(SecuritySettings.WifiAPKey) }
};


bool FactoryDefault_WiFi_NVS::applyToSettings_from_NVS(ESPEasy_NVS_Helper& preferences) {
  String tmp;
  constexpr unsigned nr__WiFi_NVS_securityPrefs_values = NR_ELEMENTS(_WiFi_NVS_securityPrefs_values);

  for (unsigned i = 0; i < nr__WiFi_NVS_securityPrefs_values; ++i) {
    if (preferences.getPreference(_WiFi_NVS_securityPrefs_values[i]._pref, tmp)) {
      safe_strncpy(_WiFi_NVS_securityPrefs_values[i]._dest, tmp, _WiFi_NVS_securityPrefs_values[i]._size);
    }
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
  constexpr unsigned nr__WiFi_NVS_securityPrefs_values = NR_ELEMENTS(_WiFi_NVS_securityPrefs_values);

  for (unsigned i = 0; i < nr__WiFi_NVS_securityPrefs_values; ++i) {
    preferences.setPreference(_WiFi_NVS_securityPrefs_values[i]._pref, String(_WiFi_NVS_securityPrefs_values[i]._dest));
  }
}

void FactoryDefault_WiFi_NVS::clear_from_NVS(ESPEasy_NVS_Helper& preferences) {
  constexpr unsigned nr__WiFi_NVS_securityPrefs_values = NR_ELEMENTS(_WiFi_NVS_securityPrefs_values);

  for (unsigned i = 0; i < nr__WiFi_NVS_securityPrefs_values; ++i) {
    preferences.remove(_WiFi_NVS_securityPrefs_values[i]._pref);
  }
  preferences.remove(F(FACTORY_DEFAULT_NVS_WIFI_FLAGS_KEY));
}

#endif // ifdef ESP32
