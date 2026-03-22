#pragma once

#include "../../../ESPEasy_common.h"

#if FEATURE_WIFI

# include "../wifi/WiFiConnectionProtocol.h"
# include "../wifi/WiFi_STA_connected_state.h"
# include "../wifi/WiFi_State.h"


# if defined(ESP8266)
#  include "../wifi/ESPEasyWiFi_STA_Event_ESP8266.h"
#  include <ESP8266WiFi.h>
# endif // if defined(ESP8266)
# if defined(ESP32)
#  include "../wifi/ESPEasyWiFi_STA_Event_ESP32.h"
#  include <WiFi.h>
#  include <WiFiType.h>
# endif // if defined(ESP32)

namespace ESPEasy {
namespace net {
namespace wifi {


// Call before starting WiFi
bool WiFi_pre_setup();

// Call before setting WiFi into STA mode
bool WiFi_pre_STA_setup();


void doWiFiDisconnect();

// ********************************************************************************
// Manage Wifi Modes
// ********************************************************************************
bool                       doWifiIsAP(WiFiMode_t wifimode);
bool                       doWifiIsSTA(WiFiMode_t wifimode);

const __FlashStringHelper* doGetWifiModeString(WiFiMode_t wifimode);

# if CONFIG_SOC_WIFI_SUPPORT_5G
const __FlashStringHelper* doGetWifiBandModeString(wifi_band_mode_t wifiBandMode);
# endif

bool                       doSetSTA(bool enable);
bool                       doSetAP(bool enable);
bool                       doSetSTA_AP(bool sta_enable,
                                       bool ap_enable);

bool                       doSetWifiMode(WiFiMode_t new_mode);


void                       doWifiScan(bool    async,
                                      uint8_t channel = 0);


bool doWiFiScanAllowed();

// Only internal scope
void doSetAPinternal(bool enable);

// ********************************************************************************
// Event handlers
// ********************************************************************************

WiFiConnectionProtocol doGetConnectionProtocol();
float                  doGetRSSIthreshold(float& maxTXpwr);

// ********************************************************************************
// Configure WiFi TX power
// ********************************************************************************

# if FEATURE_SET_WIFI_TX_PWR

// Actually set the TX power using platform specific calls.
void doSetWiFiTXpower(float& dBm);


void doSetWiFiTXpower();

void doSetWiFiTXpower(float dBm,
                      float rssi);

float doGetWiFiTXpower();

# endif // if FEATURE_SET_WIFI_TX_PWR


void doSetConnectionSpeed();
# if CONFIG_SOC_WIFI_SUPPORT_5G
void doSetConnectionSpeed(bool             ForceWiFi_bg_mode,
                          wifi_band_mode_t WiFi_band_mode);
# else // if CONFIG_SOC_WIFI_SUPPORT_5G
void doSetConnectionSpeed(bool ForceWiFi_bg_mode);
# endif // if CONFIG_SOC_WIFI_SUPPORT_5G

void doSetWiFiNoneSleep();
void doSetWiFiEcoPowerMode();
void doSetWiFiDefaultPowerMode();

void doSetWiFiCountryPolicyManual();


void doSetUseStaticIP(bool enabled);

} // namespace wifi
} // namespace net
} // namespace ESPEasy


#endif // if FEATURE_WIFI
