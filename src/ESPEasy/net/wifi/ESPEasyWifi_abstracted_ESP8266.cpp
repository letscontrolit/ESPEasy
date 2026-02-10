#include "../wifi/ESPEasyWifi_abstracted.h"

#if FEATURE_WIFI
# ifdef ESP8266

#  include "../../../src/DataStructs/TimingStats.h"
#  include "../../../src/Helpers/StringConverter.h"
#  include "../../../src/Globals/Services.h"
#  include "../../../src/Globals/Settings.h"
#  include "../Globals/WiFi_AP_Candidates.h"

#  include "../Globals/ESPEasyWiFiEvent.h"
#  include "../wifi/ESPEasyWiFi_STA_Event_ESP8266.h"
#  include "../../net/ESPEasyNetwork.h"

// #  include "../wifi/ESPEasyWifi.h"

namespace ESPEasy {
namespace net {
namespace wifi {

bool WiFi_pre_setup() {
  if (!ESPEasyWiFi_STA_EventHandler::initialized()) { return false; }

  doSetSTA_AP(false, false);
  delay(100);
  return true;
}

bool WiFi_pre_STA_setup() {
  if (!doSetSTA(true)) { return false; }

  // FIXME TD-er: Should ESP8266 first disable autoconnect/autoreconnect?
  // On ESP32 this does clear the last used credentials, so it will be able to accept different credentials to connect to.

  // Assign to 2 separate bools to make sure both are executed.
  const bool autoConnect   = WiFi.setAutoConnect(true);
  const bool autoReconnect = WiFi.setAutoReconnect(true);
#ifndef LIMIT_BUILD_SIZE
  if (!autoConnect || !autoReconnect) {
    addLog(LOG_LEVEL_ERROR, F("WiFi  : Disabling auto (re)connect failed"));
  }
#endif
  delay(100);
  return true;
}

void doWiFiDisconnect() {
  // Only call disconnect when STA is active
  if (doWifiIsSTA(WiFi.getMode())) {
    wifi_station_disconnect();
  }
  station_config conf{};
  memset(&conf, 0, sizeof(conf));
  ETS_UART_INTR_DISABLE();
  wifi_station_set_config_current(&conf);
  ETS_UART_INTR_ENABLE();
}

bool doWifiIsAP(WiFiMode_t wifimode)  { return (wifimode == WIFI_AP) || (wifimode == WIFI_AP_STA); }

bool doWifiIsSTA(WiFiMode_t wifimode) { return (wifimode == WIFI_STA) || (wifimode == WIFI_AP_STA); }

bool doSetWifiMode(WiFiMode_t new_mode)
{
  if (!Settings.getNetworkEnabled(NETWORK_INDEX_WIFI_AP)) {
    if (new_mode == WIFI_AP) { new_mode = WIFI_OFF; }

    if (new_mode == WIFI_AP_STA) { new_mode = WIFI_STA; }
  }

  if (!Settings.getNetworkEnabled(NETWORK_INDEX_WIFI_STA)) {
    if (new_mode == WIFI_STA) { new_mode = WIFI_OFF; }

    if (new_mode == WIFI_AP_STA) { new_mode = WIFI_AP; }
  }

  const WiFiMode_t cur_mode = WiFi.getMode();

  // Made this static flag an int as ESP8266 and ESP32 differ in the "not set" values
  static int8_t processing_wifi_mode = -1;

  if (cur_mode == new_mode) {
    return true;
  }

  if (processing_wifi_mode == static_cast<int8_t>(new_mode)) {
    // Prevent loops
    return true;
  }
  processing_wifi_mode = static_cast<int8_t>(new_mode);


  if (!doWifiIsSTA(new_mode)) {
    // calls lwIP's dhcp_stop(),
    // safe to call even if not started
    // See: https://github.com/esp8266/Arduino/pull/5703/files
    wifi_station_dhcpc_stop();
  }

  if (new_mode != WIFI_OFF) {
    // See: https://github.com/esp8266/Arduino/issues/6172#issuecomment-500457407
    WiFi.forceSleepWake(); // Make sure WiFi is really active.
    delay(100);
  }
#  ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_INFO, concat(F("WIFI : Set WiFi to "), doGetWifiModeString(new_mode)));
#  endif

# if FEATURE_DNS_SERVER
   if (!doWifiIsAP(new_mode) && dnsServerActive) {
     dnsServerActive = false;
     dnsServer.stop();
   }
# endif // if FEATURE_DNS_SERVER

  int retry = 2;

  while (!WiFi.mode(new_mode) && retry > 0) {
    delay(100);
    --retry;
  }
  retry = 2;

  while (WiFi.getMode() != new_mode && retry > 0) {
#  ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_INFO, F("WIFI : mode not yet set"));
#  endif
    delay(100);
    --retry;
  }

  if (WiFi.getMode() != new_mode) {
#  ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_ERROR, F("WIFI : Cannot set mode!!!!!"));
#  endif
    return false;
  }


  if (new_mode == WIFI_OFF) {
    WiFi.forceSleepBegin();
    delay(1);
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
    WiFi.scanNetworks(async, show_hidden, channel);

    if (!async) {
      FeedSW_watchdog();
      WiFi_AP_Candidates.process_WiFiscan();

      // FIXME TD-er: This should call WiFi_AP_Candidates.process_WiFiscan();
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
    case WiFiConnectionProtocol::WiFi_Protocol_11n:
      threshold = WIFI_SENSITIVITY_n;

      if (maxTXpwr > MAX_TX_PWR_DBM_n) { maxTXpwr = MAX_TX_PWR_DBM_n; }
      break;
    case WiFiConnectionProtocol::Unknown:
      break;
  }
  return threshold;
}

WiFiConnectionProtocol doGetConnectionProtocol()
{
  if (WiFi.RSSI() < 0) {
    switch (wifi_get_phy_mode())
    {
      case PHY_MODE_11B:
        return WiFiConnectionProtocol::WiFi_Protocol_11b;
      case PHY_MODE_11G:
        return WiFiConnectionProtocol::WiFi_Protocol_11g;
      case PHY_MODE_11N:
        return WiFiConnectionProtocol::WiFi_Protocol_11n;
    }
  }
  return WiFiConnectionProtocol::Unknown;
}

#  if FEATURE_SET_WIFI_TX_PWR

void  doSetWiFiTXpower(float& dBm) { WiFi.setOutputPower(dBm); }

float doGetWiFiTXpower()           { return WiFi.getOutputPower(); }

#  endif // if FEATURE_SET_WIFI_TX_PWR

void doSetConnectionSpeed(bool ForceWiFi_bg_mode) {
  // ESP8266 only supports 802.11g mode when running in STA+AP
  const bool forcedByAPmode =  doWifiIsAP(WiFi.getMode());
  WiFiPhyMode_t phyMode     = (ForceWiFi_bg_mode || forcedByAPmode) ? WIFI_PHY_MODE_11G : WIFI_PHY_MODE_11N;

  if (!forcedByAPmode) {
    const WiFi_AP_Candidate candidate = WiFi_AP_Candidates.getCurrent();

    if (candidate.phy_known() && (candidate.bits.phy_11g != candidate.bits.phy_11n)) {
      if ((WIFI_PHY_MODE_11G == phyMode) && !candidate.bits.phy_11g) {
        phyMode = WIFI_PHY_MODE_11N;
#  ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_INFO, F("WIFI : AP is set to 802.11n only"));
#  endif
      } else if ((WIFI_PHY_MODE_11N == phyMode) && !candidate.bits.phy_11n) {
        phyMode = WIFI_PHY_MODE_11G;
#  ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_INFO, F("WIFI : AP is set to 802.11g only"));
#  endif
      }

      /*
          } else {
            bool useAlternate = WiFiEventData.connectionFailures > 10;

            if (useAlternate) {
              phyMode = (WIFI_PHY_MODE_11G == phyMode) ? WIFI_PHY_MODE_11N : WIFI_PHY_MODE_11G;
            }
       */
    }
  } else {
    // No need to perform a next attempt.
    WiFi_AP_Candidates.markAttempt();
  }

  if (WiFi.getPhyMode() == phyMode) {
    return;
  }
  #  ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = concat(F("WIFI : Set to 802.11"), (WIFI_PHY_MODE_11G == phyMode) ? 'g' : 'n');

    if (forcedByAPmode) {
      log += (F(" (AP+STA mode)"));
    }

    if (Settings.ForceWiFi_bg_mode()) {
      log += F(" Force B/G mode");
    }
    addLogMove(LOG_LEVEL_INFO, log);
  }
  #  endif // ifndef BUILD_NO_DEBUG
  WiFi.setPhyMode(phyMode);
}

void doSetWiFiNoneSleep() { WiFi.setSleepMode(WIFI_NONE_SLEEP); }

void doSetWiFiEcoPowerMode()
{
  // Allow light sleep during idle times
  WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
}

void doSetWiFiDefaultPowerMode() { WiFi.setSleepMode(WIFI_MODEM_SLEEP); }

void doSetWiFiCountryPolicyManual() {
  // Not yet implemented/required for ESP8266
}

class WiFi_Access_Static_IP : public ESP8266WiFiSTAClass
{
public:

  void set_use_static_ip(bool enabled);
};

void WiFi_Access_Static_IP::set_use_static_ip(bool enabled) { _useStaticIp = enabled; }

void doSetUseStaticIP(bool enabled) {
  WiFi_Access_Static_IP tmp;

  tmp.set_use_static_ip(enabled);
}

} // namespace wifi
} // namespace net
} // namespace ESPEasy

# endif // ifdef ESP8266
#endif // if FEATURE_WIFI
