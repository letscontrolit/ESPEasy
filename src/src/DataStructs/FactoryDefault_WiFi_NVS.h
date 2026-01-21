#ifndef DATASTRUCTS_FACTORYDEFAULT_WIFI_NVS_H
#define DATASTRUCTS_FACTORYDEFAULT_WIFI_NVS_H


#include "../../ESPEasy_common.h"

#ifdef ESP32

# include "../Helpers/ESPEasy_NVS_Helper.h"


class FactoryDefault_WiFi_NVS {
private:

  void fromSettings();

  void applyToSettings() const;

public:

  bool applyToSettings_from_NVS(ESPEasy_NVS_Helper& preferences);

  void fromSettings_to_NVS(ESPEasy_NVS_Helper& preferences);

  void clear_from_NVS(ESPEasy_NVS_Helper& preferences);

private:

  union {
    struct {
      uint64_t IncludeHiddenSSID              : 1;
      uint64_t ApCaptivePortal_inv            : 1;  // Was ApDontForceSetup, so store inverted to remain compatible with existing settings
      uint64_t DoNotStartAPfallback_ConnectFail                   : 1;
      uint64_t ForceWiFi_bg_mode              : 1;
      uint64_t WiFiRestart_connection_lost    : 1;
      uint64_t WifiNoneSleep                  : 1;
      uint64_t gratuitousARP                  : 1;
      uint64_t UseMaxTXpowerForSending        : 1;
      uint64_t UseLastWiFiFromRTC             : 1;
      uint64_t WaitWiFiConnect                : 1;
      uint64_t SDK_WiFi_autoreconnect         : 1;
      uint64_t HiddenSSID_SlowConnectPerBSSID : 1;
      uint64_t EnableIPv6                     : 1;
      uint64_t PassiveWiFiScan                : 1;

      uint64_t unused : 50;
    } bits;

    uint64_t data{};
  };
};


#endif // ifdef ESP32


#endif // ifndef DATASTRUCTS_FACTORYDEFAULT_WIFI_NVS_H
