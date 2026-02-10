#pragma once

#include "../../../ESPEasy_common.h"

#if FEATURE_WIFI

# if defined(ESP8266)
  #  include <ESP8266WiFi.h>
# endif // if defined(ESP8266)
# if defined(ESP32)
  #  include <WiFi.h>
# endif // if defined(ESP32)

# include "../wifi/WiFiConnectionProtocol.h"
# include "../DataStructs/WiFi_AP_Candidate.h"

# include "../../../src/Helpers/LongTermTimer.h"
# include "../../../src/Helpers/KeyValueWriter.h"

# ifdef ESP32
#  define SOFTAP_STATION_COUNT  WiFi.AP.stationCount()
# endif
# ifdef ESP8266
#  define SOFTAP_STATION_COUNT  WiFi.softAPgetStationNum()
# endif


namespace ESPEasy {
namespace net {
namespace wifi {

# define WIFI_RECONNECT_WAIT                 30000  // in milliSeconds
# define WIFI_AP_OFF_TIMER_DURATION         300000  // in milliSeconds
# if FEATURE_CUSTOM_PROVISIONING
#  define WIFI_CONNECTION_CONSIDERED_STABLE   60000 // in milliSeconds
# else
#  define WIFI_CONNECTION_CONSIDERED_STABLE  60000  // in milliSeconds
# endif // if FEATURE_CUSTOM_PROVISIONING
# define WIFI_ALLOW_AP_AFTERBOOT_PERIOD     5       // in minutes
# define WIFI_SCAN_INTERVAL_AP_USED         125000  // in milliSeconds
# define WIFI_SCAN_INTERVAL_MINIMAL          60000  // in milliSeconds


bool WiFiConnected();

// void  WiFiConnectRelaxed();
bool prepareWiFi();
bool checkAndResetWiFi();
void resetWiFi();
void initWiFi();
void exitWiFi();
void loopWiFi();

# ifdef BOARD_HAS_SDIO_ESP_HOSTED

// ********************************************************************************
// ESP-Hosted-MCU
// ********************************************************************************
// Part of these ESP-Hosted-MCU related commands are original from Tasmota
// and parts are developed as a cooporation between ESPEasy and Tasmota.

enum class EspHostTypes {
  ESP_HOST,
  ESP_HOSTED

};

uint32_t GetHostFwVersion();
int32_t  GetHostedMCUFwVersion();
String   GetHostedFwVersion(EspHostTypes hostType);
String   GetHostedMCU();
void     HostedMCUStatus();

bool     write_WiFi_Hosted_MCU_pins(KeyValueWriter*writer);
bool     write_WiFi_Hosted_MCU_info(KeyValueWriter*writer);

# endif 

# if FEATURE_SET_WIFI_TX_PWR
void  SetWiFiTXpower();
void  SetWiFiTXpower(float dBm); // 0-20.5
void  SetWiFiTXpower(float dBm,
                     float rssi);
float GetWiFiTXpower();
# endif // if FEATURE_SET_WIFI_TX_PWR
float GetRSSIthreshold(float& maxTXpwr);

// Return some quality based on RSSI.
// <-97 => 0 , >-50 => 10
// -97 ... -50 => 1 ... 9
int                    GetRSSI_quality();
WiFiConnectionProtocol getConnectionProtocol();
# ifdef ESP32

// TSF time is 64-bit timer in usec, sent by the AP along with other packets.
// On tested access points, this seems to be the uptime in usec.
// Could be used among nodes connected to the same AP to increase time sync accuracy.
int64_t WiFi_get_TSF_time();
# endif // ifdef ESP32
void    WifiDisconnect();
bool    WiFiScanAllowed();
void    WiFiScan_log_to_serial();

void    setAPinternal(bool enable); // FIXME TD-er: Move to ESPEasyWifi_abstracted...

void    setUseStaticIP(bool enabled);
bool    WiFiUseStaticIP();
bool    wifiAPmodeActivelyUsed();
void    setupStaticIPconfig();
String  formatScanResult(int           i,
                         const String& separator);
String  formatScanResult(int           i,
                         const String& separator,
                         int32_t     & rssi);

void logConnectionStatus();


// ********************************************************************************
// Manage Wifi Modes
// ********************************************************************************
bool                       WifiIsAP(WiFiMode_t wifimode);
bool                       WifiIsSTA(WiFiMode_t wifimode);

const __FlashStringHelper* getWifiModeString(WiFiMode_t wifimode);

# if CONFIG_SOC_WIFI_SUPPORT_5G
const __FlashStringHelper* getWifiBandModeString(wifi_band_mode_t wifiBandMode);
# endif

bool                       setSTA(bool enable);
bool                       setAP(bool enable);
bool                       setSTA_AP(bool sta_enable,
                                     bool ap_enable);

bool                       setWifiMode(WiFiMode_t new_mode);


void                       WifiScan(bool    async,
                                    uint8_t channel = 0);


} // namespace wifi
} // namespace net
} // namespace ESPEasy
#endif // if FEATURE_WIFI
