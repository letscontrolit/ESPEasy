#include "../wifi/ESPEasyWifi_abstracted.h"

#if FEATURE_WIFI

# include "../../net/ESPEasyNetwork.h"
# include "../../../src/Globals/EventQueue.h"
# include "../../../src/Globals/SecuritySettings.h"
# include "../../../src/Globals/Services.h"
# include "../../../src/Globals/Settings.h"
# include "../Globals/WiFi_AP_Candidates.h"
# include "../../../src/Helpers/StringConverter.h"
# include "../Globals/ESPEasyWiFiEvent.h"
# include "../Globals/ESPEasyWiFi.h"
# include "../Globals/NetworkState.h"

namespace ESPEasy {
namespace net {
namespace wifi {

const __FlashStringHelper* doGetWifiModeString(WiFiMode_t wifimode)
{
  switch (wifimode)
  {
    case WIFI_OFF:    return F("OFF");
    case WIFI_STA:    return F("STA");
    case WIFI_AP:     return F("AP");
    case WIFI_AP_STA: return F("AP+STA");
    default:
      break;
  }
  return F("Unknown");
}

# if CONFIG_SOC_WIFI_SUPPORT_5G

const __FlashStringHelper* doGetWifiBandModeString(wifi_band_mode_t wifiBandMode)
{
  switch (wifiBandMode)
  {
    case WIFI_BAND_MODE_2G_ONLY: return F("2.4 GHz only");
    case WIFI_BAND_MODE_5G_ONLY: return F("5 GHz only");
    default:  break;
  }
  return F("2.4 GHz + 5 GHz");
}

# endif // if CONFIG_SOC_WIFI_SUPPORT_5G

bool doSetSTA(bool enable) { return doSetSTA_AP(enable,  doWifiIsAP(WiFi.getMode())); }

bool doSetAP(bool enable)  { return doSetSTA_AP(doWifiIsSTA(WiFi.getMode()), enable); }

bool doSetSTA_AP(bool sta_enable, bool ap_enable)
{
# ifndef SOC_WIFI_SUPPORTED
  return true;
# else

  if (ap_enable) {
    return doSetWifiMode(sta_enable ? WIFI_AP_STA : WIFI_AP);
  }
  return doSetWifiMode(sta_enable ? WIFI_STA : WIFI_OFF);
# endif // ifndef SOC_WIFI_SUPPORTED
}

// ********************************************************************************
// Scan WiFi network
// ********************************************************************************
bool doWiFiScanAllowed() {
  if (WiFi_AP_Candidates.scanComplete() == WIFI_SCAN_RUNNING) {
    return false;
  }
  return true; // WiFiEventData.processedConnect;
}

// Only internal scope
void doSetAPinternal(bool enable)
{
  if (enable) {
    if (!Settings.getNetworkEnabled(NETWORK_INDEX_WIFI_AP)) { return; }

    // create and store unique AP SSID/PW to prevent ESP from starting AP mode with default SSID and No password!
    // setup ssid for AP Mode when needed
    String softAPSSID = NetworkCreateRFCCompliantHostname();
    String pwd        = SecuritySettings.WifiAPKey;
    IPAddress subnet(DEFAULT_AP_SUBNET);
    # ifdef ESP32
    IPAddress dhcp_lease_start = (uint32_t)0;

    IPAddress dns(DEFAULT_AP_DNS);

    if (Settings.ApCaptivePortal()) {
      dns = apIP;
    } else {
      if (WiFi.STA.dnsIP()) {
        dns = WiFi.STA.dnsIP();
      }
    }

    if (!WiFi.softAPConfig(apIP, apIP, subnet, dhcp_lease_start, dns)) {
      addLog(LOG_LEVEL_ERROR, strformat(
               ("WIFI : [AP] softAPConfig failed! IP: %s, GW: %s, SN: %s DNS: %s"),
               apIP.toString().c_str(),
               apIP.toString().c_str(),
               subnet.toString().c_str(),
               dns.toString().c_str())
             );
    }
    WiFi.AP.bandwidth(WIFI_BW_HT20);
    # endif // ifdef ESP32
    # ifdef ESP8266

    if (!WiFi.softAPConfig(apIP, apIP, subnet)) {
#  ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_ERROR, strformat(
               ("WIFI : [AP] softAPConfig failed! IP: %s, GW: %s, SN: %s"),
               apIP.toString().c_str(),
               apIP.toString().c_str(),
               subnet.toString().c_str())
             );
#endif
    }
    # endif // ifdef ESP8266

    int channel = 1;

    if (Settings.WiFiAP_channel) {
      channel = Settings.WiFiAP_channel;
    }

    if (WifiIsSTA(WiFi.getMode()) && WiFiConnected()) {
      channel = WiFi.channel();
    }

    doSetAP(true);

    if (WiFi.softAP(softAPSSID.c_str(), pwd.c_str(), channel)) {
      auto data = getWiFi_AP_NWPluginData_static_runtime();

      if (data) { data->mark_start(); }
#ifndef BUILD_NO_DEBUG
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLogMove(LOG_LEVEL_INFO, strformat(
                     F("WIFI : AP Mode enabled. SSID: %s IP: %s ch: %d"),
                     softAPSSID.c_str(),
                     formatIP(WiFi.softAPIP()).c_str(),
                     channel));
      }
    } else {
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        addLogMove(LOG_LEVEL_ERROR, strformat(
                     F("WIFI : Error while starting AP Mode with SSID: %s IP: %s"),
                     softAPSSID.c_str(),
                     formatIP(apIP).c_str()));
      }
#endif
    }

    if (Settings.ApCaptivePortal()) {
# ifdef ESP32
#  if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 4, 2)

      if (WiFi.AP.enableDhcpCaptivePortal()) {
        addLog(LOG_LEVEL_INFO, F("WIFI : AP Captive Portal enabled"));
      }
      else {
        addLog(LOG_LEVEL_ERROR, F("WIFI : Failed to enable AP Captive Portal"));
      }
#  endif // if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 4, 2)

#  if FEATURE_DNS_SERVER

      if (!dnsServerActive) {
        dnsServerActive = true;
        dnsServer.start();
      }
#  endif // if FEATURE_DNS_SERVER
# else // ifdef ESP32
#  if FEATURE_DNS_SERVER

      if (!dnsServerActive) {
        dnsServerActive = true;
        dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
      }
#  endif // if FEATURE_DNS_SERVER
# endif // ifdef ESP32
    }
  } else {
    # if FEATURE_DNS_SERVER

    if (dnsServerActive) {
      dnsServerActive = false;
      dnsServer.stop();
    }
    # endif // if FEATURE_DNS_SERVER
    auto data = getWiFi_AP_NWPluginData_static_runtime();

    if (data) { data->mark_stop(); }

    doSetAP(false);
  }
}

void doSetConnectionSpeed() {
  doSetConnectionSpeed(
    Settings.ForceWiFi_bg_mode()
# if CONFIG_SOC_WIFI_SUPPORT_5G
    , Settings.WiFi_band_mode()
# endif
    );
}

// ********************************************************************************
// Configure WiFi TX power
// ********************************************************************************
# if FEATURE_SET_WIFI_TX_PWR

void doSetWiFiTXpower() {
  doSetWiFiTXpower(0, WiFi.RSSI());

  // Just some minimal value, will be adjusted in doSetWiFiTXpower
}

void doSetWiFiTXpower(float dBm, float rssi) {
  const WiFiMode_t cur_mode = WiFi.getMode();

  if (cur_mode == WIFI_OFF) {
    return;
  }

  if (Settings.UseMaxTXpowerForSending()) {
    dBm = 30; // Just some max, will be limited later
  }

  // Range ESP32  : -1dBm - 20dBm
  // Range ESP8266: 0dBm - 20.5dBm
  float maxTXpwr;
  float threshold = GetRSSIthreshold(maxTXpwr);
  #  ifdef ESP8266
  float minTXpwr{};
  #  endif
  #  ifdef ESP32
  float minTXpwr = -1.0f;
  #  endif

  threshold += Settings.WiFi_sensitivity_margin; // Margin in dBm on top of threshold

  // Assume AP sends with max set by ETSI standard.
  // 2.4 GHz: 100 mWatt (20 dBm)
  // US and some other countries allow 1000 mW (30 dBm)
  // We cannot send with over 20 dBm, thus it makes no sense to force higher TX power all the time.
  const float newrssi = rssi - 20;

  if (newrssi < threshold) {
    minTXpwr = threshold - newrssi;
  }

  if (minTXpwr > maxTXpwr) {
    minTXpwr = maxTXpwr;
  }

  if (dBm > maxTXpwr) {
    dBm = maxTXpwr;
  } else if (dBm < minTXpwr) {
    dBm = minTXpwr;
  }

  doSetWiFiTXpower(dBm);

  //  if (WiFiEventData.wifi_TX_pwr < dBm) {
  // Will increase the TX power, give power supply of the unit some rest
  //    delay(1);
  // }

  //  WiFiEventData.wifi_TX_pwr = dBm;

  delay(0);

  #  ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    const int TX_pwr_int   = dBm * 4;
    const int maxTXpwr_int = maxTXpwr * 4;

    if (TX_pwr_int != maxTXpwr_int) {
      static int last_log = -1;

      if (TX_pwr_int != last_log) {
        last_log = TX_pwr_int;
        String log = strformat(
          F("WiFi : Set TX power to %ddBm sensitivity: %ddBm"),
          static_cast<int>(dBm),
          static_cast<int>(threshold));

        if (rssi < 0) {
          log += strformat(F(" RSSI: %ddBm"), static_cast<int>(rssi));
        }
        addLogMove(LOG_LEVEL_DEBUG, log);
      }
    }
  }
  #  endif // ifndef BUILD_NO_DEBUG
}

# endif // if FEATURE_SET_WIFI_TX_PWR

} // namespace wifi
} // namespace net
} // namespace ESPEasy

#endif // if FEATURE_WIFI
