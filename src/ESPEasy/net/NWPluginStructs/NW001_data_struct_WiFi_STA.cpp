#include "../NWPluginStructs/NW001_data_struct_WiFi_STA.h"

#ifdef USES_NW001

# include "../../../src/Globals/Settings.h"

# include "../../../src/Helpers/ESPEasy_time_calc.h"
# include "../../../src/Helpers/LongTermOnOffTimer.h"
# include "../../../src/Helpers/StringConverter.h"
# include "../../../src/WebServer/HTML_wrappers.h"
# include "../../../src/WebServer/Markup.h"

# include "../wifi/ESPEasyWifi.h"

# define NW_PLUGIN_ID  1
# ifdef ESP32
#  define NW_PLUGIN_INTERFACE   WiFi.STA
# endif

# define NW001_RSSI_STATS_INDEX       0
# define NW001_TX_PWR_STATS_INDEX     1


namespace ESPEasy {
namespace net {
namespace wifi {


NW001_data_struct_WiFi_STA::NW001_data_struct_WiFi_STA(networkIndex_t networkIndex)
  : NWPluginData_base(
    nwpluginID_t(NW_PLUGIN_ID)
    , networkIndex
# ifdef ESP32
    , &NW_PLUGIN_INTERFACE
# endif
    ),
  _WiFiEventHandler(networkIndex)
{}

NW001_data_struct_WiFi_STA::~NW001_data_struct_WiFi_STA()
{}

void NW001_data_struct_WiFi_STA::webform_load(EventStruct *event)
{


}

void NW001_data_struct_WiFi_STA::webform_save(EventStruct *event) {}

bool NW001_data_struct_WiFi_STA::webform_getPort(KeyValueWriter *writer)     { return true; }

bool NW001_data_struct_WiFi_STA::init(EventStruct *event)
{
  /*
     // Taken from ESPEasy_setup()
     // Might no longer be needed as we now have a proper WiFi state machine.

   # ifdef ESP32

      // FIXME TD-er: Disabled for now, as this may not return and thus block the ESP forever.
      // See: https://github.com/espressif/esp-idf/issues/15862
      // check_and_update_WiFi_Calibration();
   # endif // ifdef ESP32

      ESPEasy::net::wifi::WiFi_AP_Candidates.clearCache();
      ESPEasy::net::wifi::WiFi_AP_Candidates.load_knownCredentials();
      ESPEasy::net::wifi::setSTA(true);

      if (!ESPEasy::net::wifi::WiFi_AP_Candidates.hasCandidateCredentials()) {
        WiFiEventData.wifiSetup = true;
        RTC.clearLastWiFi(); // Must scan all channels
        // Wait until scan has finished to make sure as many as possible are found
        // We're still in the setup phase, so nothing else is taking resources of the ESP.
        ESPEasy::net::wifi::WifiScan(false);
        WiFiEventData.lastScanMoment.clear();
      }



      // Always perform WiFi scan
      // It appears reconnecting from RTC may take just as long to be able to send first packet as performing a scan first and then
   #connect.
      // Perhaps the WiFi radio needs some time to stabilize first?
      if (!ESPEasy::net::wifi::WiFi_AP_Candidates.hasCandidates()) {
        ESPEasy::net::wifi::WifiScan(false, RTC.lastWiFiChannel);
      }
      ESPEasy::net::wifi::WiFi_AP_Candidates.clearCache();
      ESPEasy::net::wifi::processScanDone();
      ESPEasy::net::wifi::WiFi_AP_Candidates.load_knownCredentials();

      if (!ESPEasy::net::wifi::WiFi_AP_Candidates.hasCandidates()) {
   # ifndef BUILD_MINIMAL_OTA
        addLog(LOG_LEVEL_INFO, F("Setup: Scan all channels"));
   # endif
        ESPEasy::net::wifi::WifiScan(false);
      }

      //    ESPEasy::net::wifi::setWifiMode(WIFI_OFF);
   */

  ESPEasy::net::wifi::initWiFi();
  return true;
}

bool NW001_data_struct_WiFi_STA::exit(EventStruct *event) {
  ESPEasy::net::wifi::exitWiFi();
  return true;
}

NWPluginData_static_runtime * NW001_data_struct_WiFi_STA::getNWPluginData_static_runtime() {
  return _WiFiEventHandler.getNWPluginData_static_runtime();
}

const __FlashStringHelper * NW001_data_struct_WiFi_STA::getWiFi_encryptionType() const
{
  return _WiFiEventHandler.getWiFi_encryptionType();
}

WiFiDisconnectReason NW001_data_struct_WiFi_STA::getWiFi_disconnectReason() const
{
  return _WiFiEventHandler.getLastDisconnectReason();
}

# ifdef ESP32

bool NW001_data_struct_WiFi_STA::handle_priority_route_changed()
{
  if (NW_PLUGIN_INTERFACE.isDefault()) {
    if (NWPlugin::forceDHCP_request(&NW_PLUGIN_INTERFACE)) { return true; }
    return _WiFiEventHandler.restore_dns_from_cache();
  }
  return false;
}

# endif // ifdef ESP32

# if FEATURE_NETWORK_STATS

bool NW001_data_struct_WiFi_STA::initPluginStats()
{
  networkStatsVarIndex_t networkStatsVarIndex{};
  PluginStats_Config_t   displayConfig;

  displayConfig.setAxisPosition(PluginStats_Config_t::AxisPosition::Left);
  displayConfig.setEnabled(true);

  displayConfig.setAxisIndex(networkStatsVarIndex);
  NWPluginData_base::initPluginStats(
    networkStatsVarIndex,
    F("RSSI"),
    1,
    NAN,
    displayConfig);
#  if FEATURE_SET_WIFI_TX_PWR
  ++networkStatsVarIndex;
  displayConfig.setAxisIndex(networkStatsVarIndex);
  displayConfig.setHidden(true);
  NWPluginData_base::initPluginStats(
    networkStatsVarIndex,
    F("TX Power"),
    1,
    NAN,
    displayConfig);
#  endif // if FEATURE_SET_WIFI_TX_PWR
#  if FEATURE_NETWORK_TRAFFIC_COUNT
  initPluginStats_trafficCount(++networkStatsVarIndex, true);  // TX
  initPluginStats_trafficCount(++networkStatsVarIndex, false); // RX
#  endif // if FEATURE_NETWORK_TRAFFIC_COUNT
  return true;
}

bool NW001_data_struct_WiFi_STA::record_stats()
{
  if (_plugin_stats_array != nullptr) {
    EventStruct tmpEvent;
    size_t valueCount{};
    const int rssi =
#  ifdef ESP32
      WiFi.STA.RSSI();
#  else
      WiFi.RSSI();
#  endif // ifdef ESP32

    tmpEvent.ParfN[valueCount++] = rssi < 0 ? rssi : NAN;
#  if FEATURE_SET_WIFI_TX_PWR
    tmpEvent.ParfN[valueCount++] = ESPEasy::net::wifi::GetWiFiTXpower();
#  endif

    bool trackPeaks                  = true;
    bool onlyUpdateTimestampWhenSame = true;
    return pushStatsValues(&tmpEvent, valueCount, trackPeaks, onlyUpdateTimestampWhenSame);
  }
  return false;
}

bool NW001_data_struct_WiFi_STA::webformLoad_show_stats(struct EventStruct *event) const
{
  if (_plugin_stats_array != nullptr) {
#  if FEATURE_SET_WIFI_TX_PWR
#   if FEATURE_CHART_JS
    addRowColspan(2);

    plot_ChartJS_scatter(
      NW001_RSSI_STATS_INDEX,
      NW001_TX_PWR_STATS_INDEX,
      F("rssitxpwrscatter"),
      { F("RSSI/TX pwr Scatter Plot") },
      { F("rssi/tx_pwr"), F("rgb(255, 99, 132)") });
    addHtml(F("</td></tr>"));
#   endif // if FEATURE_CHART_JS
#  endif // if FEATURE_SET_WIFI_TX_PWR
    return _plugin_stats_array->webformLoad_show_stats(event);
  }
  return false;
}

# endif // if FEATURE_NETWORK_STATS


} // namespace wifi
} // namespace net
} // namespace ESPEasy

#endif // ifdef USES_NW001
