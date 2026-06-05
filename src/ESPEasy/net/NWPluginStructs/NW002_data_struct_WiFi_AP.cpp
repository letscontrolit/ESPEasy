#include "../NWPluginStructs/NW002_data_struct_WiFi_AP.h"

#ifdef USES_NW002

# include "../wifi/ESPEasyWifi.h"
# include "../Globals/NetworkState.h"

# include "../../../src/Globals/Settings.h"
# include "../../../src/Helpers/Networking.h"

# ifdef ESP32
#  include <esp_wifi.h>
#  include <esp_wifi_ap_get_sta_list.h>
#  include <NetworkEvents.h>
#  include <NetworkManager.h>
# endif // ifdef ESP32

# define NW_PLUGIN_ID  2
# ifdef ESP32
#  define NW_PLUGIN_INTERFACE   WiFi.AP
# endif

# define NW002_STATION_COUNT_STATS_INDEX       0

namespace ESPEasy {
namespace net {
namespace wifi {

# ifdef ESP32
static NWPluginData_static_runtime stats_and_cache(true, &NW_PLUGIN_INTERFACE, "AP");
# else
static NWPluginData_static_runtime stats_and_cache(true, "AP"); // Cannot use flash strings during init of static objects
# endif // ifdef ESP32
static bool nw002_initialized{};
# ifdef ESP32
static bool nw002_enable_NAPT{};

static bool NW002_update_NAPT() {
  if (nw002_initialized) {
    NW_PLUGIN_INTERFACE.enableNAPT(false);

    if (nw002_enable_NAPT && NW_PLUGIN_INTERFACE.stationCount()) {
      NW_PLUGIN_INTERFACE.enableNAPT(true);
      return true;
    }
  }

  return false;
}

# endif // ifdef ESP32

NW002_data_struct_WiFi_AP::NW002_data_struct_WiFi_AP(networkIndex_t networkIndex)
  : NWPluginData_base(
    nwpluginID_t(NW_PLUGIN_ID)
    , networkIndex
# ifdef ESP32
    , &NW_PLUGIN_INTERFACE
# endif
    )
{
  stats_and_cache.clear(networkIndex);
# ifdef ESP32
  nw002_enable_NAPT = Settings.WiFi_AP_enable_NAPT();
  nw_event_id       = Network.onEvent(NW002_data_struct_WiFi_AP::onEvent);
# endif // ifdef ESP32
  nw002_initialized = true;
}

NW002_data_struct_WiFi_AP::~NW002_data_struct_WiFi_AP()
{
  nw002_initialized = false;
# ifdef ESP32
  nw002_enable_NAPT = false;

  if (nw_event_id != 0) {
    Network.removeEvent(nw_event_id);
  }
  nw_event_id = 0;
# endif // ifdef ESP32
  stats_and_cache.processEvent_and_clear();
}

void NW002_data_struct_WiFi_AP::webform_load(EventStruct *event)        {}

void NW002_data_struct_WiFi_AP::webform_save(EventStruct *event)        {}

bool NW002_data_struct_WiFi_AP::webform_getPort(KeyValueWriter *writer) { return true; }

bool NW002_data_struct_WiFi_AP::init(EventStruct *event)
{
  {
    auto runtime_data = getNWPluginData_static_runtime();

    if (runtime_data) {
      runtime_data->clear(event->NetworkIndex);
      IPAddress ip, gateway, sn, dns;
      getStaticIPAddresses(ip, gateway, sn, dns);
      runtime_data->setStaticIP(ip, gateway, sn, dns);
    }
  }

# ifdef ESP32
  nw002_enable_NAPT = Settings.WiFi_AP_enable_NAPT();
# endif

  if (!ESPEasy::net::wifi::setAPinternal(true)) {
    return false;
  }
# ifdef ESP32
  if (NW_PLUGIN_INTERFACE.hasIP()) {
    stats_and_cache.mark_got_IP();
  }
  NW002_update_NAPT();
# endif
  # if FEATURE_MDNS
  #  ifdef ESP8266

  // notifyAPChange() is not present in the ESP32 MDNSResponder
  MDNS.notifyAPChange();
  #  endif // ifdef ESP8266
  # endif // if FEATURE_MDNS

  return true;
}

bool NW002_data_struct_WiFi_AP::exit(EventStruct *event)
{
  ESPEasy::net::wifi::setAPinternal(false);
# ifdef ESP32
  NW_PLUGIN_INTERFACE.enableNAPT(false);
  NW_PLUGIN_INTERFACE.end();
# endif // ifdef ESP32
# ifdef ESP8266
  WiFi.softAPdisconnect(true);
# endif // ifdef ESP8266

  stats_and_cache.processEvents();

  return true;
}

NWPluginData_static_runtime * NW002_data_struct_WiFi_AP::getNWPluginData_static_runtime()
{
  if (nw002_initialized) {
    return &stats_and_cache;
  }
  return nullptr;
}

bool NW002_data_struct_WiFi_AP::getStaticIPAddress(IPAddressType addressType, IPAddress& ip) const
{
  switch (addressType)
  {
    case IPAddressType::IP:
      ip = apIP;
      return true;
    case IPAddressType::Gateway:
      ip = apIP;
      return true;
    case IPAddressType::Subnetmask:
      ip = IPAddress(DEFAULT_AP_SUBNET);
      return true;
    case IPAddressType::DNS:
      if (Settings.ApCaptivePortal()) {
        ip = apIP;
      } else {
        ip = IPAddress(DEFAULT_AP_DNS);
      }
      return true;
  }
  return false;
}

# ifdef ESP32

bool NW002_data_struct_WiFi_AP::handle_priority_route_changed() { return NW002_update_NAPT(); }

# endif // ifdef ESP32


# if FEATURE_NETWORK_STATS

bool NW002_data_struct_WiFi_AP::initPluginStats()
{
  if (!Settings.getNetworkCollectStats(_networkIndex)) { return false; }
  networkStatsVarIndex_t networkStatsVarIndex{};
  PluginStats_Config_t   displayConfig;

  displayConfig.setAxisPosition(PluginStats_Config_t::AxisPosition::Left);
  displayConfig.setEnabled(true);

  displayConfig.setAxisIndex(networkStatsVarIndex);
  displayConfig.setHidden(true);
  NWPluginData_base::initPluginStats(
    networkStatsVarIndex,
    F("Station Count"),
    1,
    NAN,
    displayConfig);
#  ifdef ESP32
  ++networkStatsVarIndex;
  displayConfig.setHidden(false);
  displayConfig.setAxisIndex(networkStatsVarIndex);
  NWPluginData_base::initPluginStats(
    networkStatsVarIndex,
    F("RSSI"),
    1,
    NAN,
    displayConfig);

#   if FEATURE_NETWORK_TRAFFIC_COUNT
  initPluginStats_trafficCount(++networkStatsVarIndex, true);  // TX
  initPluginStats_trafficCount(++networkStatsVarIndex, false); // RX
#   endif // if FEATURE_NETWORK_TRAFFIC_COUNT
#  endif // ifdef ESP32
  return true;
}

bool NW002_data_struct_WiFi_AP::record_stats()
{
  if (_plugin_stats_array != nullptr) {
    EventStruct tmpEvent;
    size_t valueCount{};
    tmpEvent.ParfN[valueCount++] = SOFTAP_STATION_COUNT;

#  ifdef ESP32
    {
      wifi_sta_list_t wifi_sta_list = { 0 };
      esp_wifi_ap_get_sta_list(&wifi_sta_list);

      if (wifi_sta_list.num == 0) {
        tmpEvent.ParfN[valueCount++] = NAN;
      } else {
        // FIXME TD-er: Should we list the 1st one, average or best/worst value?
        // For now, just use the first one, which is likely the only user scenario actually being used
        tmpEvent.ParfN[valueCount++] = wifi_sta_list.sta[0].rssi;
      }
    }
#  endif // ifdef ESP32

    bool trackPeaks                  = true;
    bool onlyUpdateTimestampWhenSame = true;
    return pushStatsValues(&tmpEvent, valueCount, trackPeaks, onlyUpdateTimestampWhenSame);
  }
  return false;
}

bool NW002_data_struct_WiFi_AP::webformLoad_show_stats(struct EventStruct *event) const
{
  if (_plugin_stats_array != nullptr) {
    return _plugin_stats_array->webformLoad_show_stats(event);
  }
  return false;
}

# endif // if FEATURE_NETWORK_STATS

# ifdef ESP32

void NW002_data_struct_WiFi_AP::onEvent(arduino_event_id_t   event,
                                        arduino_event_info_t info)
{
  switch (event)
  {
    case ARDUINO_EVENT_WIFI_AP_START:
      stats_and_cache.mark_start();

      if (NW_PLUGIN_INTERFACE.hasIP()) {
        stats_and_cache.mark_got_IP();
      }
      break;
    case ARDUINO_EVENT_WIFI_AP_STOP:
      stats_and_cache.mark_stop();
      break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
      stats_and_cache.mark_connected();
      NW002_update_NAPT();
      addLog(LOG_LEVEL_INFO, F("AP_STACONNECTED"));
      break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:

      if (!ESPEasy::net::wifi::wifiAPmodeActivelyUsed()) {
        stats_and_cache.mark_disconnected();
      }
      NW002_update_NAPT();
      addLog(LOG_LEVEL_INFO, F("AP_STADISCONNECTED"));
      break;
    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
      addLog(LOG_LEVEL_INFO, F("AP_STAIPASSIGNED"));

      if (!stats_and_cache.hasIP() && NW_PLUGIN_INTERFACE.hasIP()) {
        stats_and_cache.mark_got_IP();
      }
      break;
    case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
      addLog(LOG_LEVEL_INFO, F("AP_PROBEREQRECVED"));
      break;
    case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
      addLog(LOG_LEVEL_INFO, F("AP_GOT_IP6"));
      break;

    default: break;
  }
}

# endif // ifdef ESP32

} // namespace wifi
} // namespace net
} // namespace ESPEasy

#endif // ifdef USES_NW002
