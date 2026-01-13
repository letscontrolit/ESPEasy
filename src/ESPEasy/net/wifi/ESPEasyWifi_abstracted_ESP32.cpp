#include "../wifi/ESPEasyWifi_abstracted.h"

#if FEATURE_WIFI
# ifdef ESP32

#  include "../../../src/DataStructs/TimingStats.h"
#  include "../../../src/Globals/EventQueue.h"
#  include "../../../src/Globals/Services.h"
#  include "../../../src/Globals/Settings.h"
#  include "../Globals/WiFi_AP_Candidates.h"
#  include "../../../src/Helpers/StringConverter.h"

#  include "../../net/ESPEasyNetwork.h" // Needed for NetworkCreateRFCCompliantHostname, WiFi code should not include network code
#  include "../Globals/ESPEasyWiFiEvent.h"
#  include "../wifi/ESPEasyWiFi_STA_Event_ESP32.h"

#  include <WiFiGeneric.h>
#  include <esp_wifi.h> // Needed to call ESP-IDF functions like esp_wifi_....

#  ifndef ESP32P4
#   include <esp_phy_init.h>
#  endif


namespace ESPEasy {
namespace net {
namespace wifi {

bool WiFi_pre_setup() {
  if (!ESPEasyWiFi_STA_EventHandler::initialized()) { return false; }

  //  registerWiFiEventHandler();
  WiFi.persistent(false);

  return doSetSTA_AP(false, false);
}

bool WiFi_pre_STA_setup()
{
  if (!doSetSTA(true)) { return false; }

  WiFi.setAutoReconnect(false);
  delay(10);
  return true;
}

void doWiFiDisconnect() {
  uint8_t retry = 3;
  while (!WiFi.disconnect(Settings.WiFiRestart_connection_lost()) && retry) {
    --retry;
    delay(100);
  }
/*
  {
    const IPAddress ip;
    const IPAddress gw;
    const IPAddress subnet;
    const IPAddress dns;
    WiFi.config(ip, gw, subnet, dns);
  }
*/
}

bool doWifiIsAP(WiFiMode_t wifimode)  { return (wifimode == WIFI_MODE_AP) || (wifimode == WIFI_MODE_APSTA); }

bool doWifiIsSTA(WiFiMode_t wifimode) { return (wifimode == WIFI_MODE_STA) || (wifimode == WIFI_MODE_APSTA); }

bool doSetWifiMode(WiFiMode_t new_mode)
{
  if (!Settings.getNetworkEnabled(NETWORK_INDEX_WIFI_AP)) {
    if (new_mode == WIFI_MODE_AP) new_mode = WIFI_OFF;
    if (new_mode == WIFI_MODE_APSTA) new_mode = WIFI_MODE_STA;
  }
  if (!Settings.getNetworkEnabled(NETWORK_INDEX_WIFI_STA)) {
    if (new_mode == WIFI_MODE_STA) new_mode = WIFI_OFF;
    if (new_mode == WIFI_MODE_APSTA) new_mode = WIFI_MODE_AP;
  }

  const WiFiMode_t cur_mode = WiFi.getMode();

  // Made this static flag an int as ESP8266 and ESP32 differ in the "not set" values
  static int8_t processing_wifi_mode = -1;

  if (cur_mode == new_mode) {
    if (cur_mode != WIFI_OFF) {
      //      registerWiFiEventHandler();
    }
    return true;
  }

  if (processing_wifi_mode == static_cast<int8_t>(new_mode)) {
    // Prevent loops
    return true;
  }
  processing_wifi_mode = static_cast<int8_t>(new_mode);


  if (cur_mode == WIFI_OFF) {
    // Needs to be set while WiFi is off
    WiFi.hostname(NetworkCreateRFCCompliantHostname());

    //    WiFiEventData.markWiFiTurnOn();
  }

  if (new_mode != WIFI_OFF) {
    #  ifdef ESP8266

    // See: https://github.com/esp8266/Arduino/issues/6172#issuecomment-500457407
    WiFi.forceSleepWake(); // Make sure WiFi is really active.
    #  endif // ifdef ESP8266
    delay(100);
  } else {
    doWiFiDisconnect();

    //    delay(100);
    //processDisconnect();

    //    WiFiEventData.clear_processed_flags();
  }

  addLog(LOG_LEVEL_INFO, concat(F("WIFI : Set WiFi to "), doGetWifiModeString(new_mode)));

  # if FEATURE_DNS_SERVER
  if (!doWifiIsAP(new_mode)) {
    if (dnsServerActive) {
      dnsServerActive = false;
      dnsServer.stop();
    }
  }
  #endif

  int retry = 2;

  while (!WiFi.mode(new_mode) && retry > 0) {
    delay(100);
    --retry;
  }
  retry = 2;

  while (WiFi.getMode() != new_mode && retry > 0) {
    addLog(LOG_LEVEL_INFO, F("WIFI : mode not yet set"));
    delay(100);
    --retry;
  }

  if (WiFi.getMode() != new_mode) {
    WiFi.mode(WIFI_OFF);
    delay(100);
    addLog(LOG_LEVEL_ERROR, F("WIFI : Cannot set mode!!!!!"));
    return false;
  }


  if (new_mode == WIFI_OFF) {

    // FIXME TD-er: Is this correct to mark Turn ON ????
    //    WiFiEventData.markWiFiTurnOn();

    // Needs to be set while WiFi is off
    WiFi.hostname(NetworkCreateRFCCompliantHostname());
    delay(100);
#  ifndef ESP32P4
    esp_wifi_set_ps(WIFI_PS_NONE);
#  endif

    //    esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
    delay(1);
    #  ifdef ESP32
    WiFi.STA.end();
    #  endif
  } else {

    /*
        if (cur_mode == WIFI_OFF) {
          registerWiFiEventHandler();
        }
     */

    // Only set power mode when AP is not enabled
    // When AP is enabled, the sleep mode is already set to WIFI_NONE_SLEEP
    if (!doWifiIsAP(new_mode)) {
      if (Settings.WifiNoneSleep()) {
        doSetWiFiNoneSleep();
      } else if (Settings.EcoPowerMode()) {
        doSetWiFiEcoPowerMode();
      } else {
        // Default
        doSetWiFiDefaultPowerMode();
      }
    }
#  if FEATURE_SET_WIFI_TX_PWR
    doSetWiFiTXpower();
#  endif

    if (doWifiIsSTA(new_mode)) {
      //      WiFi.setAutoConnect(Settings.SDK_WiFi_autoreconnect());
      WiFi.setAutoReconnect(Settings.SDK_WiFi_autoreconnect());
    }
    delay(100); // Must allow for some time to init.
  }
  const bool new_mode_AP_enabled =  doWifiIsAP(new_mode);

  if (doWifiIsAP(cur_mode) && !new_mode_AP_enabled) {
    eventQueue.add(F("WiFi#APmodeDisabled"));
  }

  if (doWifiIsAP(cur_mode) != new_mode_AP_enabled) {
    // Mode has changed
    doSetAPinternal(new_mode_AP_enabled);
  }
  #  if FEATURE_MDNS
  #   ifdef ESP8266

  // notifyAPChange() is not present in the ESP32 MDNSResponder
  MDNS.notifyAPChange();
  #   endif // ifdef ESP8266
  #  endif // if FEATURE_MDNS
  return true;
}

void doWifiScan(bool async, uint8_t channel) {
  doSetSTA(true);

  if (!doWiFiScanAllowed()) {
    return;
  }

#  if CONFIG_SOC_WIFI_SUPPORT_5G
  const wifi_band_mode_t current_wifi_band_mode = WiFi.getBandMode();
  WiFi.setBandMode(Settings.WiFi_band_mode());
#  endif // if CONFIG_SOC_WIFI_SUPPORT_5G

  // TD-er: Don't run async scan on ESP32.
  // Since IDF 4.4 it seems like the active channel may be messed up when running async scan
  // Perform a disconnect after scanning.
  // See: https://github.com/letscontrolit/ESPEasy/pull/3579#issuecomment-967021347
  // async = false;

  if (Settings.IncludeHiddenSSID()) {
    doSetWiFiCountryPolicyManual();
  }

  START_TIMER;
//  WiFiEventData.lastScanMoment.setNow();
  #  ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    if (channel == 0) {
      addLog(LOG_LEVEL_INFO, F("WiFi : Start network scan all channels"));
    } else {
      addLogMove(LOG_LEVEL_INFO, strformat(F("WiFi : Start network scan ch: %d "), channel));
    }
  }
  #  endif // ifndef BUILD_NO_DEBUG
  bool show_hidden = true;
//  WiFiEventData.lastGetScanMoment.setNow();
//  WiFiEventData.lastScanChannel = channel;

  unsigned int nrScans = 1 + (async ? 0 : Settings.ConnectFailRetryCount);

  while (nrScans > 0) {
    WiFi_AP_Candidates.begin_scan();
    if (!async) {
      FeedSW_watchdog();
    }
    --nrScans;

    const bool passive             = Settings.PassiveWiFiScan();
    const uint32_t max_ms_per_chan = 120;
#  if CONFIG_SOC_WIFI_SUPPORT_5G

    // ESP32-C5 scans both 2.4 and 5 GHz band, which takes much longer
    // 14 channels on 2.4 GHz
    // 28 channels on 5 GHz: 36~177 (36, 40, 44 ... 177)
    // Typically a scan takes 40 ... 60 msec per channel longer.
    // To be safe, set the timeout to 2x max_ms_per_chan

    WiFi.setScanTimeout((14 + 28) * max_ms_per_chan * 2);
#  else // if CONFIG_SOC_WIFI_SUPPORT_5G
    WiFi.setScanTimeout(14 * max_ms_per_chan * 2);
#  endif // if CONFIG_SOC_WIFI_SUPPORT_5G
    WiFi.scanNetworks(async, show_hidden, passive, max_ms_per_chan /*, channel */);

    if (!async) {
      FeedSW_watchdog();
      WiFi_AP_Candidates.process_WiFiscan();
//      processScanDone();
    }
  }
#  if FEATURE_TIMING_STATS

  if (async) {
    STOP_TIMER(WIFI_SCAN_ASYNC);
  } else {
    STOP_TIMER(WIFI_SCAN_SYNC);
  }
#  endif // if FEATURE_TIMING_STATS

#  if ESP_IDF_VERSION_MAJOR < 5
  RTC.clearLastWiFi();

  if (WiFiConnected()) {
    #   ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_INFO, F("WiFi : Disconnect after scan"));
    #   endif

//    const bool needReconnect = WiFiEventData.wifiConnectAttemptNeeded;
    WifiDisconnect();
//    WiFiEventData.wifiConnectAttemptNeeded = needReconnect;
  }
#  endif // if ESP_IDF_VERSION_MAJOR < 5
#  if CONFIG_SOC_WIFI_SUPPORT_5G

  // Restore band mode
  WiFi.setBandMode(current_wifi_band_mode);
#  endif // if CONFIG_SOC_WIFI_SUPPORT_5G
}

float doGetRSSIthreshold(float& maxTXpwr) {
  maxTXpwr = Settings.getWiFi_TX_power();

  float threshold = WIFI_SENSITIVITY_n;

  switch (doGetConnectionProtocol())
  {
    case WiFiConnectionProtocol::WiFi_Protocol_11b:
      threshold = WIFI_SENSITIVITY_11b;

      if (maxTXpwr > MAX_TX_PWR_DBM_11b) { maxTXpwr = MAX_TX_PWR_DBM_11b; }
      break;
    case WiFiConnectionProtocol::WiFi_Protocol_11g:
      threshold = WIFI_SENSITIVITY_54g;

      if (maxTXpwr > MAX_TX_PWR_DBM_54g) { maxTXpwr = MAX_TX_PWR_DBM_54g; }
      break;
    case WiFiConnectionProtocol::WiFi_Protocol_HT20:
    case WiFiConnectionProtocol::WiFi_Protocol_HT40:
    case WiFiConnectionProtocol::WiFi_Protocol_HE20:

      threshold = WIFI_SENSITIVITY_n;

      if (maxTXpwr > MAX_TX_PWR_DBM_n) { maxTXpwr = MAX_TX_PWR_DBM_n; }
      break;
#  if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(5, 2, 0)
    case WiFiConnectionProtocol::WiFi_Protocol_11a:
    case WiFiConnectionProtocol::WiFi_Protocol_VHT20:
      // FIXME TD-er: Must determine max. TX power for these 5 GHz modi
#  endif // if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(5, 2, 0)
    case WiFiConnectionProtocol::WiFi_Protocol_LR:
    case WiFiConnectionProtocol::Unknown:
      break;
  }
  return threshold;
}

WiFiConnectionProtocol doGetConnectionProtocol()
{
  //  #ifndef ESP32P4
  if (WiFi.RSSI() < 0) {
    wifi_phy_mode_t phymode;
    esp_wifi_sta_get_negotiated_phymode(&phymode);

    switch (phymode)
    {
      case WIFI_PHY_MODE_11B:   return WiFiConnectionProtocol::WiFi_Protocol_11b;
      case WIFI_PHY_MODE_11G:   return WiFiConnectionProtocol::WiFi_Protocol_11g;
      case WIFI_PHY_MODE_HT20:  return WiFiConnectionProtocol::WiFi_Protocol_HT20;
      case WIFI_PHY_MODE_HT40:  return WiFiConnectionProtocol::WiFi_Protocol_HT40;
      case WIFI_PHY_MODE_HE20:  return WiFiConnectionProtocol::WiFi_Protocol_HE20;
      case WIFI_PHY_MODE_LR:    return WiFiConnectionProtocol::WiFi_Protocol_LR;
#  if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 3, 0)

      // 5 GHz
      case WIFI_PHY_MODE_11A:   return WiFiConnectionProtocol::WiFi_Protocol_11a;
      case WIFI_PHY_MODE_VHT20: return WiFiConnectionProtocol::WiFi_Protocol_VHT20;
#  endif // if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 3, 0)
    }
  }

  //  #endif
  return WiFiConnectionProtocol::Unknown;
}

#  if FEATURE_SET_WIFI_TX_PWR

void doSetWiFiTXpower(float& dBm)
{
  int8_t power = dBm * 4;

  esp_wifi_set_max_tx_power(power);

  if (esp_wifi_get_max_tx_power(&power) == ESP_OK)  {
    dBm = static_cast<float>(power) / 4.0f;
  }
}

float doGetWiFiTXpower()
{
  int8_t power{};

  if (esp_wifi_get_max_tx_power(&power) == ESP_OK) {
    float res = power;
    res /= 4.0f;
    return res;
  }

  return NAN;
}

#  endif // if FEATURE_SET_WIFI_TX_PWR

#  if CONFIG_SOC_WIFI_SUPPORT_5G

bool setProtocol(wifi_interface_t ifx, uint16_t protocol_2GHz, uint16_t protocol_5GHz)
{
  esp_err_t err;

  switch (WiFi.getBandMode())
  {
    case WIFI_BAND_MODE_2G_ONLY: err = esp_wifi_set_protocol(ifx, protocol_2GHz);
      break;
    case WIFI_BAND_MODE_5G_ONLY: err = esp_wifi_set_protocol(ifx, protocol_5GHz);
      break;
    default:
    {
      wifi_protocols_t protocols { .ghz_2g = protocol_2GHz, .ghz_5g = protocol_5GHz };
      err = esp_wifi_set_protocols(WIFI_IF_STA, &protocols);
      break;
    }
  }

  if (err != ESP_OK) {
    // TODO TD-er: Log
  }

  return err == ESP_OK;
}

#  else // if CONFIG_SOC_WIFI_SUPPORT_5G

bool setProtocol(wifi_interface_t ifx, uint8_t protocol_2GHz) {
  #   ifndef SOC_WIFI_SUPPORTED
  return false;
  #   else
  return esp_wifi_set_protocol(ifx, protocol_2GHz) == ESP_OK;
  #   endif // ifndef SOC_WIFI_SUPPORTED
}

#  endif // if CONFIG_SOC_WIFI_SUPPORT_5G


#  if CONFIG_SOC_WIFI_SUPPORT_5G

void doSetConnectionSpeed(bool ForceWiFi_bg_mode, wifi_band_mode_t WiFi_band_mode)
#  else // if CONFIG_SOC_WIFI_SUPPORT_5G

void doSetConnectionSpeed(bool ForceWiFi_bg_mode)
#  endif // if CONFIG_SOC_WIFI_SUPPORT_5G
{
  // Does not (yet) work, so commented out.

  // HT20 = 20 MHz channel width.
  // HT40 = 40 MHz channel width.
  // In theory, HT40 can offer upto 150 Mbps connection speed.
  // However since HT40 is using nearly all channels on 2.4 GHz WiFi,
  // Thus you are more likely to experience disturbances.
  // The response speed and stability is better at HT20 for ESP units.
  #  ifndef SOC_WIFI_SUPPORTED
  return;
  #  else
  esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW_HT20);


#   if CONFIG_SOC_WIFI_SUPPORT_5G
  WiFi.setBandMode(WiFi_band_mode);
#    if CONFIG_SOC_WIFI_HE_SUPPORT
  uint8_t protocol_5GHz = WIFI_PROTOCOL_11N | WIFI_PROTOCOL_11A | WIFI_PROTOCOL_11AC | WIFI_PROTOCOL_11AX;
#    else
  uint8_t protocol_5GHz = WIFI_PROTOCOL_11N | WIFI_PROTOCOL_11A | WIFI_PROTOCOL_11AC;
#    endif // if CONFIG_SOC_WIFI_HE_SUPPORT
#   endif // if CONFIG_SOC_WIFI_SUPPORT_5G

  uint8_t protocol = 0;

  if (ForceWiFi_bg_mode) {
    protocol = WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G; // Default to BG
  }

  const WiFi_AP_Candidate candidate = WiFi_AP_Candidates.getCurrent();

  if (candidate.phy_known()) {
    if (protocol == 0) {
      // No protocol preference set, so we try to match at least the modes supported by the AP
      if (candidate.bits.phy_11b) { protocol |= WIFI_PROTOCOL_11B; }

      if (candidate.bits.phy_11g) { protocol |= WIFI_PROTOCOL_11G; }

      if (candidate.bits.phy_11n) { protocol |= WIFI_PROTOCOL_11N; }
#   if CONFIG_SOC_WIFI_HE_SUPPORT

      if (candidate.bits.phy_11ax) { protocol |= WIFI_PROTOCOL_11AX; }
#   endif // if CONFIG_SOC_WIFI_HE_SUPPORT

      /*
       #  if CONFIG_SOC_WIFI_SUPPORT_5G

            if (candidate.bits.phy_11a) { protocol |= WIFI_PROTOCOL_11A; }

            if (candidate.bits.phy_11ac) { protocol |= WIFI_PROTOCOL_11AC; }
       #  endif // if CONFIG_SOC_WIFI_SUPPORT_5G
       */
    } else

    // Check to see if the access point is set to "N-only"
    if ((protocol & WIFI_PROTOCOL_11N) == 0) {
      if (!candidate.bits.phy_11b && !candidate.bits.phy_11g && candidate.bits.phy_11n) {
        if (candidate.bits.phy_11n) {
          // Set to use BGN
          protocol |= WIFI_PROTOCOL_11N;
          addLog(LOG_LEVEL_INFO, F("WIFI : AP is set to 802.11n only"));
        }
#   if CONFIG_SOC_WIFI_HE_SUPPORT

        if (candidate.bits.phy_11ax) {
          // Set to use WiFi6
          protocol |= WIFI_PROTOCOL_11AX;

          //          protocol_5GHz |= WIFI_PROTOCOL_11AX;
          addLog(LOG_LEVEL_INFO, F("WIFI : AP allows 802.11ax, Wi-Fi 6"));
        }
#    if CONFIG_SOC_WIFI_SUPPORT_5G

        if (candidate.bits.phy_11a) {
          // Set to use 5 GHz WiFi
          //          protocol_5GHz |= WIFI_PROTOCOL_11A;
          addLog(LOG_LEVEL_INFO, F("WIFI : AP allows 802.11a, 5 GHz"));
        }

        if (candidate.bits.phy_11ac) {
          // Set to use 5 GHz WiFi-5
          //          protocol_5GHz |= WIFI_PROTOCOL_11AC;
          addLog(LOG_LEVEL_INFO, F("WIFI : AP allows 802.11ac, 5 GHz Wi-Fi 5"));
        }
#    endif // if CONFIG_SOC_WIFI_SUPPORT_5G
#   endif // if CONFIG_SOC_WIFI_HE_SUPPORT
      }
    }
  }


  if (doWifiIsSTA(WiFi.getMode())) {
    // Set to use "Long GI" making it more resilliant to reflections
    // See: https://www.tp-link.com/us/configuration-guides/q_a_basic_wireless_concepts/?configurationId=2958#_idTextAnchor038
    esp_wifi_config_80211_tx_rate(WIFI_IF_STA, WIFI_PHY_RATE_MCS3_LGI);
    #   if CONFIG_SOC_WIFI_SUPPORT_5G
    setProtocol(WIFI_IF_STA, protocol, protocol_5GHz);
    #   else
    setProtocol(WIFI_IF_STA, protocol);
    #   endif // if CONFIG_SOC_WIFI_SUPPORT_5G
  }

  if (doWifiIsAP(WiFi.getMode())) {
    #   if CONFIG_SOC_WIFI_SUPPORT_5G
    setProtocol(WIFI_IF_AP,  protocol, protocol_5GHz);
    #   else
    setProtocol(WIFI_IF_STA, protocol);
    #   endif // if CONFIG_SOC_WIFI_SUPPORT_5G
  }
  #  endif // ifndef SOC_WIFI_SUPPORTED
}

void doSetWiFiNoneSleep() {
  #  ifndef ESP32P4
  WiFi.setSleep(WIFI_PS_NONE);
  #  endif
}

void doSetWiFiEcoPowerMode()
{
  // Maximum modem power saving.
  // In this mode, interval to receive beacons is determined by the listen_interval parameter in wifi_sta_config_t
  // FIXME TD-er: Must test if this is desired behavior in ESP32.
  #  ifndef ESP32P4
  WiFi.setSleep(WIFI_PS_MAX_MODEM);
  #  endif
}

void doSetWiFiDefaultPowerMode()
{
  // Minimum modem power saving.
  // In this mode, station wakes up to receive beacon every DTIM period
  #  ifndef ESP32P4
  WiFi.setSleep(WIFI_PS_MIN_MODEM);
  #  endif
}

void doSetWiFiCountryPolicyManual()
{
  /*   wifi_country_t config = {
      .cc     = "01",
      .schan  = 1,
      .nchan  = 14,
      .policy = WIFI_COUNTRY_POLICY_MANUAL,
     };

     esp_wifi_set_country(&config);
   */
}

/*
   class WiFi_Access_Static_IP : public WiFiSTAClass
   {
   public:

   void set_use_static_ip(bool enabled);
   };
 */
void doSetUseStaticIP(bool enabled) {}

} // namespace wifi
} // namespace net
} // namespace ESPEasy

# endif // ifdef ESP32
#endif // if FEATURE_WIFI
