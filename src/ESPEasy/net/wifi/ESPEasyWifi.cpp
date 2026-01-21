#include "../wifi/ESPEasyWifi.h"

#if FEATURE_WIFI

# include "../../../ESPEasy-Globals.h"
# include "../../../src/DataStructs/TimingStats.h"
# include "../../net/ESPEasyNetwork.h"
# include "../../../src/ESPEasyCore/ESPEasy_Log.h"
# include "../../../src/ESPEasyCore/Serial.h"
# include "../../../src/Globals/EventQueue.h"
# include "../../../src/Globals/Nodes.h"
# include "../../../src/Globals/RTC.h"
# include "../../../src/Globals/SecuritySettings.h"
# include "../../../src/Globals/Services.h"
# include "../../../src/Globals/Settings.h"
# include "../Globals/WiFi_AP_Candidates.h"
# include "../../../src/Helpers/ESPEasy_time_calc.h"
# include "../../../src/Helpers/ESPEasy_UnitOfMeasure.h"
# include "../../../src/Helpers/Hardware_defines.h"
# include "../../../src/Helpers/Misc.h"
# include "../../../src/Helpers/Networking.h"
# include "../../../src/Helpers/StringConverter.h"
# include "../../../src/Helpers/StringGenerator_WiFi.h"
# include "../../../src/Helpers/StringProvider.h"
# include "../Globals/ESPEasyWiFi.h"
# include "../Globals/ESPEasyWiFiEvent.h"
# include "../Globals/NetworkState.h"
# include "../wifi/ESPEasyWifi_abstracted.h"


# ifdef ESP32
#  include <WiFiGeneric.h>
#  include <esp_wifi.h> // Needed to call ESP-IDF functions like esp_wifi_....

#  ifndef ESP32P4
#   include <esp_phy_init.h>
#  else
#   include <esp_hosted.h>
extern "C" {
#   include "esp_hosted_transport_config.h"
}

#   include "port/esp/freertos/include/port_esp_hosted_host_config.h"
#  endif // ifndef ESP32P4

# endif // ifdef ESP32


namespace ESPEasy {
namespace net {
namespace wifi {

// ********************************************************************************
// WiFi state
// ********************************************************************************

/*
   WiFi STA states:
   1 STA off                 => ESPEASY_WIFI_DISCONNECTED
   2 STA connecting
   3 STA connected           => ESPEASY_WIFI_CONNECTED
   4 STA got IP              => ESPEASY_WIFI_GOT_IP
   5 STA connected && got IP => ESPEASY_WIFI_SERVICES_INITIALIZED

   N.B. the states are flags, meaning both "connected" and "got IP" must be set
        to be considered ESPEASY_WIFI_SERVICES_INITIALIZED

   The flag wifiConnectAttemptNeeded indicates whether a new connect attempt is needed.
   This is set to true when:
   - Security settings have been saved with AP mode enabled. FIXME TD-er, this may not be the best check.
   - WiFi connect timeout reached  &  No client is connected to the AP mode of the node.
   - Wifi is reset
   - WiFi setup page has been loaded with SSID/pass values.


   WiFi AP mode states:
   1 AP on                        => reset AP disable timer
   2 AP client connect/disconnect => reset AP disable timer
   3 AP off                       => AP disable timer = 0;

   AP mode will be disabled when both apply:
   - AP disable timer (timerAPoff) expired
   - No client is connected to the AP.

   AP mode will be enabled when at least one applies:
   - No valid WiFi settings
   - Start AP timer (timerAPstart) expired

   Start AP timer is set or cleared at:
   - Set timerAPstart when "valid WiFi connection" state is observed.
   - Disable timerAPstart when ESPEASY_WIFI_SERVICES_INITIALIZED wifi state is reached.

   For the first attempt to connect after a cold boot (RTC values are 0), a WiFi scan will be
   performed to find the strongest known SSID.
   This will set RTC.lastBSSID and RTC.lastWiFiChannel

   Quick reconnect (using BSSID/channel of last connection) when both apply:
   - If wifi_connect_attempt < 3
   - RTC.lastBSSID is known
   - RTC.lastWiFiChannel != 0

   Change of wifi settings when both apply:
   - "other" settings valid
   - (wifi_connect_attempt % 2) == 0

   Reset of wifi_connect_attempt to 0 when both apply:
   - connection successful
   - Connection stable (connected for > 5 minutes)

 */

// ********************************************************************************
// Check WiFi connected status
// This is basically the state machine to switch between states:
// - Initiate WiFi reconnect
// - Start/stop of AP mode
// ********************************************************************************
bool WiFiConnected() {
  if (!Settings.getNetworkEnabled(NETWORK_INDEX_WIFI_STA)) { return false; }
  static uint32_t lastCheckedTime = 0;

  const int32_t timePassed = timePassedSince(lastCheckedTime);

  if (lastCheckedTime != 0) {
    if (timePassed < 10) {
      // Rate limit time spent in WiFiConnected() to max. 100x per sec to process the rest of this function
      return ESPEasyWiFi.connected();
    }
    lastCheckedTime = millis();
  }

  ESPEasyWiFi.loop();
  return ESPEasyWiFi.connected();
}

// void WiFiConnectRelaxed() { AttemptWiFiConnect(); }

// ********************************************************************************
// Set Wifi config
// ********************************************************************************
bool prepareWiFi() {
  //  ESPEasyWiFi.setup();

  return true;
}

bool checkAndResetWiFi() { return true; }

void resetWiFi() {
  /*   //if (wifiAPmodeActivelyUsed()) return;
     if (WiFiEventData.lastWiFiResetMoment.isSet() && !WiFiEventData.lastWiFiResetMoment.timeoutReached(1000)) {
      // Don't reset WiFi too often
      return;
     }
     FeedSW_watchdog();
     WiFiEventData.clearAll();
     WifiDisconnect();

     // Send this log only after WifiDisconnect() or else sending to syslog may cause issues
     addLog(LOG_LEVEL_INFO, F("Reset WiFi."));

     //  setWifiMode(WIFI_OFF);

     initWiFi();
   */
}

void initWiFi() { ESPEasyWiFi.setup(); }

void exitWiFi() { ESPEasyWiFi.disable(); }

void loopWiFi() { ESPEasyWiFi.loop(); }

# ifdef BOARD_HAS_SDIO_ESP_HOSTED

// ********************************************************************************
// ESP-Hosted-MCU
// ********************************************************************************
// Part of these ESP-Hosted-MCU related commands are original from Tasmota
// and parts are developed as a cooporation between ESPEasy and Tasmota.
uint32_t GetHostFwVersion()
{
  const uint32_t host_version =
    (ESP_HOSTED_VERSION_MAJOR_1 << 16) |
    (ESP_HOSTED_VERSION_MINOR_1 << 8) |
    (ESP_HOSTED_VERSION_PATCH_1);

  return host_version;
}

int32_t GetHostedMCUFwVersion()
{
  static int hosted_version = -1;

  if (!esp_hosted_is_config_valid()) {
    return 0;
  }

  if (-1 == hosted_version) {
    hosted_version = 6;                                              // v0.0.6
    esp_hosted_coprocessor_fwver_t ver_info;
    esp_err_t err = esp_hosted_get_coprocessor_fwversion(&ver_info); // This takes almost 4 seconds on <v0.0.6

    if (err == ESP_OK) {
      hosted_version = ver_info.major1 << 16 | ver_info.minor1 << 8 | ver_info.patch1;
    } else {
      // We can not know exactly, as API was added after 0.0.6
      addLog(LOG_LEVEL_DEBUG, strformat(
               F("WiFi  : ESP-Hosted-MCU Error %d, hosted version 0.0.6 or older"), err));
    }
  }
  return hosted_version;
}

String GetHostedFwVersion(EspHostTypes hostType)
{
  const int32_t version = (hostType == EspHostTypes::ESP_HOSTED)
    ? GetHostedMCUFwVersion()
    : GetHostFwVersion();

  const uint16_t major1 = version >> 16;
  const uint8_t  minor1 = version >> 8;
  const uint8_t  patch1 = version;

  return strformat(F("%d.%d.%d"), major1, minor1, patch1);
}

String GetHostedMCU()
{
  // Function is not yet implemented in Arduino Core so emulate it here
  if (equals(F(CONFIG_ESP_HOSTED_IDF_SLAVE_TARGET), F("esp32c6"))) {
    return String("ESP32-C6");
  }
  return String("Unknown");
}

void HostedMCUStatus()
{
  // Execute after HostedMCU is init by WiFi.mode()
  static bool once_shown = false;

  if (once_shown) { return; }

  if (esp_hosted_is_config_valid()) {
    once_shown = true;
    char config[128] = { 0 };
    struct esp_hosted_transport_config *pconfig;

    if (ESP_TRANSPORT_OK == esp_hosted_transport_get_config(&pconfig)) {
      if (pconfig->transport_in_use == H_TRANSPORT_SDIO) {
        struct esp_hosted_sdio_config *psdio_config;

        if (ESP_TRANSPORT_OK == esp_hosted_sdio_get_config(&psdio_config)) {
          snprintf_P(config,
                     sizeof(config),
                     PSTR(" using GPIO%02d(CLK), GPIO%02d(CMD), GPIO%02d(D0), GPIO%02d(D1), GPIO%02d(D2), GPIO%02d(D3) and GPIO%02d(RST)"),
                     psdio_config->pin_clk.pin,
                     psdio_config->pin_cmd.pin,
                     psdio_config->pin_d0.pin,
                     psdio_config->pin_d1.pin,
                     psdio_config->pin_d2.pin,
                     psdio_config->pin_d3.pin,
                     psdio_config->pin_reset.pin);
        }
      }
    }
    addLog(LOG_LEVEL_INFO, strformat(
             F("WiFi  : ESP-Hosted-MCU %s v%s%s"),
             GetHostedMCU().c_str(),
             GetHostedFwVersion(EspHostTypes::ESP_HOSTED).c_str(),
             config));
  }
}

bool write_WiFi_Hosted_MCU_pins(KeyValueWriter*writer)
{
  if (!esp_hosted_is_config_valid()) { return false; }
  struct esp_hosted_transport_config *pconfig;

  if (ESP_TRANSPORT_OK != esp_hosted_transport_get_config(&pconfig)) { return false; }

  switch (pconfig->transport_in_use)
  {
    case H_TRANSPORT_NONE:
    case H_TRANSPORT_SPI_HD:
    case H_TRANSPORT_SPI:
    case H_TRANSPORT_UART:
      break;
    case H_TRANSPORT_SDIO:
    {
      struct esp_hosted_sdio_config *psdio_config;

      if (ESP_TRANSPORT_OK == esp_hosted_sdio_get_config(&psdio_config)) {
        if (writer->summaryValueOnly()) {
          KeyValueStruct kv(EMPTY_STRING);

          kv.appendValue(concat(F("CLK: "), psdio_config->pin_clk.pin));
          kv.appendValue(concat(F("CMD: "), psdio_config->pin_cmd.pin));
          kv.appendValue(concat(F("RST: "), psdio_config->pin_reset.pin));
          kv.appendValue(strformat(
                           F("D0..3: %d/%d/%d/%d"),
                           psdio_config->pin_d0.pin,
                           psdio_config->pin_d1.pin,
                           psdio_config->pin_d2.pin,
                           psdio_config->pin_d3.pin
                           ));

          writer->write(kv);
        } else {
          KeyValueStruct freq(F("SDIO Freq"), psdio_config->clock_freq_khz / 1000);
          freq.setUnit(UOM_MHz);
          writer->write(freq);
          writer->write({ F("SDIO D0"), psdio_config->pin_d0.pin });
          writer->write({ F("SDIO D1"), psdio_config->pin_d1.pin });
          writer->write({ F("SDIO D2"), psdio_config->pin_d2.pin });
          writer->write({ F("SDIO D3"), psdio_config->pin_d3.pin });
          writer->write({ F("SDIO CLK"), psdio_config->pin_clk.pin });
          writer->write({ F("SDIO CMD"), psdio_config->pin_cmd.pin });
          writer->write({ F("SDIO RST"), psdio_config->pin_reset.pin });
          // Hide TX/RX queue size for now as it is unclear what these mean
          //writer->write({ F("SDIO TX queue"), psdio_config->tx_queue_size });
          //writer->write({ F("SDIO RX queue"), psdio_config->rx_queue_size });
        }
        return true;
      }
      break;
    }

  }
  return false;
}

bool write_WiFi_Hosted_MCU_info(KeyValueWriter*writer)
{
  if (writer == nullptr) { return false; }

  if (writer->summaryValueOnly()) {
    KeyValueStruct kv(EMPTY_STRING);
    kv.appendValue(strformat(F("%s @ %s"), GetHostedMCU().c_str(), GetHostedFwVersion(EspHostTypes::ESP_HOSTED).c_str()));
    kv.appendValue(concat(F("MAC: "), WiFi.macAddress()));
    writer->write(kv);
  } else {
    writer->write({ F("ESP-Host Fw Version"), GetHostedFwVersion(EspHostTypes::ESP_HOST) });
    writer->write({ F("ESP-Hosted-MCU Fw Version"), GetHostedFwVersion(EspHostTypes::ESP_HOSTED) });
    writer->write({ F("ESP-Hosted-MCU Chip"), GetHostedMCU() });
    writer->write({
            F("MAC"),
            WiFi.macAddress(),
            KeyValueStruct::Format::PreFormatted });

  }
  return true;
}

# endif // ifdef BOARD_HAS_SDIO_ESP_HOSTED


// ********************************************************************************
// Configure WiFi TX power
// ********************************************************************************
# if FEATURE_SET_WIFI_TX_PWR

void SetWiFiTXpower() {
  SetWiFiTXpower(0); // Just some minimal value, will be adjusted in SetWiFiTXpower
}

void  SetWiFiTXpower(float dBm)             { doSetWiFiTXpower(dBm, WiFi.RSSI()); }

void  SetWiFiTXpower(float dBm, float rssi) { doSetWiFiTXpower(dBm, rssi); }

float GetWiFiTXpower()                      { return doGetWiFiTXpower(); }

# endif // if FEATURE_SET_WIFI_TX_PWR

float GetRSSIthreshold(float& maxTXpwr) {
  maxTXpwr = Settings.getWiFi_TX_power();
  return doGetRSSIthreshold(maxTXpwr);
}

int GetRSSI_quality() {
  long rssi = WiFi.RSSI();

  if (-50 < rssi) { return 10; }

  if (rssi <= -98) { return 0;  }
  rssi = rssi + 97; // Range 0..47 => 1..9
  return (rssi / 5) + 1;
}

WiFiConnectionProtocol getConnectionProtocol() { return doGetConnectionProtocol(); }

# ifdef ESP32

int64_t WiFi_get_TSF_time() {
  #  ifndef SOC_WIFI_SUPPORTED
  return 0;
  #  else
  return esp_wifi_get_tsf_time(WIFI_IF_STA);
  #  endif // ifndef SOC_WIFI_SUPPORTED
}

# endif // ifdef ESP32

// ********************************************************************************
// Disconnect from Wifi AP
// ********************************************************************************
void WifiDisconnect() { ESPEasyWiFi.disconnect(); }

// ********************************************************************************
// Scan WiFi network
// ********************************************************************************
bool WiFiScanAllowed() { return doWiFiScanAllowed(); }

// ********************************************************************************
// Scan all Wifi Access Points
// ********************************************************************************
void WiFiScan_log_to_serial()
{
  // Direct Serial is allowed here, since this function will only be called from serial input.
  serialPrintln(F("WIFI : SSID Scan start"));

  if (WiFi_AP_Candidates.scanComplete() <= 0) {
    WiFiMode_t cur_wifimode = WiFi.getMode();
    WifiScan(false);
    WiFi_AP_Candidates.process_WiFiscan();
    setWifiMode(cur_wifimode);
  }

  const int8_t scanCompleteStatus = WiFi_AP_Candidates.scanComplete();

  if (scanCompleteStatus <= 0) {
    serialPrintln(concat(F("WIFI : No networks found. Status: "), scanCompleteStatus));
  }
  else
  {
    serialPrint(F("WIFI : "));
    serialPrint(String(scanCompleteStatus));
    serialPrintln(F(" networks found"));

    int i = 0;

    for (auto it = WiFi_AP_Candidates.scanned_begin(); it != WiFi_AP_Candidates.scanned_end(); ++it)
    {
      ++i;

      // Print SSID and RSSI for each network found
      serialPrint(F("WIFI : "));
      serialPrint(String(i));
      serialPrint(": ");
      serialPrintln(it->toString());
      delay(10);
    }
  }
  serialPrintln("");
}

// Only internal scope
void setAPinternal(bool enable)   { doSetAPinternal(enable); }

void setUseStaticIP(bool enabled) { doSetUseStaticIP(enabled); }

bool WiFiUseStaticIP()            { return Settings.IP[0] != 0 && Settings.IP[0] != 255; }

bool wifiAPmodeActivelyUsed()
{
  if (!WifiIsAP(WiFi.getMode()) // || (!WiFiEventData.timerAPoff.isSet())
      ) {
    // AP not active or soon to be disabled in processDisableAPmode()
    return false;
  }
  return SOFTAP_STATION_COUNT != 0;

  // FIXME TD-er: is effectively checking for AP active enough or must really check for connected clients to prevent automatic wifi
  // reconnect?
}

void setupStaticIPconfig() {
  setUseStaticIP(WiFiUseStaticIP());

  if (!WiFiUseStaticIP()) { return; }
  const IPAddress ip(Settings.IP);
  const IPAddress gw(Settings.Gateway);
  const IPAddress subnet(Settings.Subnet);
  const IPAddress dns(Settings.DNS);

  //  WiFiEventData.dns0_cache = dns;

  WiFi.config(ip, gw, subnet, dns);
# ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLogMove(LOG_LEVEL_INFO, strformat(
                 F("IP   : Static IP : %s GW: %s SN: %s DNS: %s"),
                 formatIP(ip).c_str(),
                 formatIP(gw).c_str(),
                 formatIP(subnet).c_str(),
                 getValue(LabelType::DNS).c_str()));
  }
# endif // ifndef BUILD_NO_DEBUG
}

// ********************************************************************************
// Formatting WiFi related strings
// ********************************************************************************
String formatScanResult(int i, const String& separator) {
  int32_t rssi = 0;

  return formatScanResult(i, separator, rssi);
}

String formatScanResult(int i, const String& separator, int32_t& rssi) {
  WiFi_AP_Candidate tmp(i);

  rssi = tmp.rssi;
  return tmp.toString(separator);
}

void logConnectionStatus() {
  static unsigned long lastLog = 0;

  if ((lastLog != 0) && (timePassedSince(lastLog) < 1000)) {
    return;
  }
  lastLog = millis();
# ifndef BUILD_NO_DEBUG
  #  ifdef ESP8266
  const uint8_t arduino_corelib_wifistatus = WiFi.status();
  const uint8_t sdk_wifistatus             = wifi_station_get_connect_status();

  if ((arduino_corelib_wifistatus == WL_CONNECTED) != (sdk_wifistatus == STATION_GOT_IP)) {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log = F("WiFi : SDK station status differs from Arduino status. SDK-status: ");
      log += SDKwifiStatusToString(sdk_wifistatus);
      log += F(" Arduino status: ");
      log += ArduinoWifiStatusToString(arduino_corelib_wifistatus);
      addLogMove(LOG_LEVEL_ERROR, log);
    }
  }
  #  endif // ifdef ESP8266

  /*
     if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLogMove(LOG_LEVEL_INFO, strformat(
                   F("WIFI : Arduino wifi status: %s ESPeasy internal wifi status: %s"),
                   ArduinoWifiStatusToString(WiFi.status()).c_str(),
                   WiFiEventData.ESPeasyWifiStatusToString().c_str()));
     }
   */
# endif // ifndef BUILD_NO_DEBUG
}

bool                       WifiIsAP(WiFiMode_t wifimode)          { return doWifiIsAP(wifimode); }

bool                       WifiIsSTA(WiFiMode_t wifimode)         { return doWifiIsSTA(wifimode); }

const __FlashStringHelper* getWifiModeString(WiFiMode_t wifimode) { return doGetWifiModeString(wifimode); }

# if CONFIG_SOC_WIFI_SUPPORT_5G

const __FlashStringHelper* getWifiBandModeString(wifi_band_mode_t wifiBandMode) { return doGetWifiBandModeString(wifiBandMode); }

# endif // if CONFIG_SOC_WIFI_SUPPORT_5G

bool setSTA(bool enable) { return doSetSTA(enable); }

bool setAP(bool enable)  { return doSetAP(enable); }

bool setSTA_AP(bool sta_enable,
               bool ap_enable) { return doSetSTA_AP(sta_enable, ap_enable); }

bool setWifiMode(WiFiMode_t new_mode) {
  const bool res = doSetWifiMode(new_mode);

# ifdef ESP32P4

  if (new_mode != WIFI_OFF) {
    ESPEasy::net::wifi::HostedMCUStatus();
  }
# endif // ifdef ESP32P4
  return res;
}

void WifiScan(bool    async,
              uint8_t channel) { doWifiScan(async, channel); }


} // namespace wifi
} // namespace net
} // namespace ESPEasy

#endif // if FEATURE_WIFI
