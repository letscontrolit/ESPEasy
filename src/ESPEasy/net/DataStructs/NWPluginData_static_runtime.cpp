#include "../DataStructs/NWPluginData_static_runtime.h"

#include "../../../src/Globals/EventQueue.h"
#include "../../../src/Globals/Settings.h"

#include "../../../src/Helpers/Networking.h"
#include "../../../src/Helpers/NetworkStatusLED.h"
#include "../../../src/Helpers/StringConverter.h"

#include "../wifi/ESPEasyWifi.h"

#ifdef ESP32
# include <esp_netif.h>
#endif


#if FEATURE_MDNS

// #include "../../../src/Helpers/MDNS_Helper.h"
#endif // if FEATURE_MDNS


namespace ESPEasy {
namespace net {

#define CONNECTION_CONSIDERED_STABLE_MSEC    60000
#define CONNECT_TIMEOUT_MAX                  10000 // in milliSeconds

#if FEATURE_NETWORK_TRAFFIC_COUNT

typedef std::map<int, TX_RX_traffic_count> InterfaceTrafficCount_t;

static InterfaceTrafficCount_t interfaceTrafficCount{};

static void tx_rx_event_handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data)
{
  if (event_data == nullptr) { return; }
  ip_event_tx_rx_t *event = (ip_event_tx_rx_t *)event_data;

  if (event->len > 0) {
    const int key = esp_netif_get_netif_impl_index(event->esp_netif);


    if (event->dir == ESP_NETIF_TX) {
      interfaceTrafficCount[key]._tx_count += event->len;
      interfaceTrafficCount[key]._tx_packets++;
    } else if (event->dir == ESP_NETIF_RX) {
      interfaceTrafficCount[key]._rx_count += event->len;
      interfaceTrafficCount[key]._rx_packets++;

      /*
          addLog(LOG_LEVEL_INFO, strformat(
                   F("RX: %s key: %d len: %d total: %d"),
                   esp_netif_get_desc(event->esp_netif),
                   key,
                   event->len,
                   interfaceTrafficCount[key]._rx_count
                   ));
       */
    }
  }
}

void NWPluginData_static_runtime::enable_txrx_events()
{
  if (_netif) {
    const int key = _netif->impl_index();
    interfaceTrafficCount[key].clear();

    if (_netif->netif()) {
      static bool registered_IP_EVENT_TX_RX = false;
      const int   key                       = esp_netif_get_netif_impl_index(_netif->netif());
      interfaceTrafficCount[key].clear();
      esp_netif_tx_rx_event_enable(_netif->netif());

      if (!registered_IP_EVENT_TX_RX) {
        esp_event_handler_instance_register(IP_EVENT, IP_EVENT_TX_RX, &tx_rx_event_handler, NULL, &_handler_inst);
        registered_IP_EVENT_TX_RX = true;
      }
      esp_netif_tx_rx_event_enable(_netif->netif());
    }

    //  _netif->netif()->tx_rx_events_enabled = true;
  }
}

bool NWPluginData_static_runtime::getTrafficCount(TX_RX_traffic_count& traffic) const
{
  if (_netif == nullptr) { return false; }
  const int key = _netif->impl_index();
  auto it       = interfaceTrafficCount.find(key);

  if (it == interfaceTrafficCount.end()) { return false; }
  traffic = it->second;
  return true;
}

#endif // if FEATURE_NETWORK_TRAFFIC_COUNT

void NWPluginData_static_runtime::clear(networkIndex_t networkIndex)
{
  _connectedStats.reset();
  _gotIPStats.reset();
#if FEATURE_USE_IPV6
  _gotIP6Stats.reset();

  // TODO TD-er: Must also clear _gotIP6Events
#endif // if FEATURE_USE_IPV6
  _operationalStats.reset();
#if FEATURE_NETWORK_TRAFFIC_COUNT

  if (_netif) {
    const int key = _netif->impl_index();
    interfaceTrafficCount[key].clear();
  }
#endif // if FEATURE_NETWORK_TRAFFIC_COUNT

  _networkIndex = networkIndex;
#ifdef ESP32

  if (_netif) {
    if ((_eventInterfaceName.length() == 0) && _netif) {
      _eventInterfaceName = _netif->desc();
      _eventInterfaceName.toUpperCase();
    }
    _enableIPv6 = Settings.EnableIPv6() && Settings.getNetworkEnabled_IPv6(_networkIndex);
  }
  _routePrio = Settings.getRoutePrio_for_network(_networkIndex);

  _dns_cache[0] = INADDR_NONE;
  _dns_cache[1] = INADDR_NONE;
#endif // ifdef ESP32

  _connectionFailures = 0;

  _ip      = INADDR_NONE;
  _gateway = INADDR_NONE;
  _sn      = INADDR_NONE;
  _dns     = INADDR_NONE;
}

void NWPluginData_static_runtime::processEvent_and_clear()
{
  processEvents();
  clear();
}

bool NWPluginData_static_runtime::operational() const
{
  if (!Settings.getNetworkEnabled(_networkIndex)) { return false; }

  if (_isAP) {
    return ESPEasy::net::wifi::wifiAPmodeActivelyUsed();
  }
  return connected() && hasIP();

  // FIXME TD-er: WiFi STA keeps reporting it is
  // connected and has IP even after call to networkdisable,1
}

void NWPluginData_static_runtime::processEvents()
{
#ifdef ESP32

  if (_netif == nullptr) { return; }
#endif // ifdef ESP32

  // TD-er: Just set these just to be sure we didn't miss any events.
  _connectedStats.set(connected());

  //  _gotIPStats.set(hasIP());
#if FEATURE_USE_IPV6

  //  _gotIP6Stats.set(hasIPv6());
#endif // if FEATURE_USE_IPV6
  const bool connected_changed        = _connectedStats.changedSinceLastCheck_and_clear();
  const bool establishConnect_changed = _establishConnectStats.changedSinceLastCheck_and_clear();

  if (_gotIPStats.changedSinceLastCheck_and_clear()) {
#ifdef ESP32

    if (loglevelActiveFor(LOG_LEVEL_INFO) && _netif) {
      auto ip = _netif->localIP();

      if (IPAddressSet(ip)) {
        addLog(LOG_LEVEL_INFO, strformat(
                 F("%s: Got IP: %s/%d GW: %s"),
                 _eventInterfaceName.c_str(),
                 ip.toString().c_str(),
                 _netif->subnetCIDR(),
                 _netif->gatewayIP().toString().c_str()
                 ));
      }
    }
#endif // ifdef ESP32
#ifdef ESP8266
  # ifndef BUILD_NO_DEBUG

    if (_isAP) {
      addLog(LOG_LEVEL_INFO, F("AP: Got IP"));
    }
    else {
      addLog(LOG_LEVEL_INFO, concat(
               F("STA: Got IP "),
               WiFi.localIP().toString()));
    }
  # endif // ifndef BUILD_NO_DEBUG
#endif // ifdef ESP8266
#if FEATURE_MDNS

    //    update_mDNS();
#endif // if FEATURE_MDNS
  }

#if FEATURE_USE_IPV6

  if (_gotIP6Stats.changedSinceLastCheck_and_clear()) {
    for (uint8_t i = 0; i < NR_ELEMENTS(_gotIP6Events); ++i) {
      ip_event_got_ip6_t ip6Event;
      memcpy(&ip6Event, &_gotIP6Events[i], sizeof(ip_event_got_ip6_t));
      memset(&_gotIP6Events[i], 0, sizeof(ip_event_got_ip6_t));

      if (loglevelActiveFor(LOG_LEVEL_INFO) && (ip6Event.esp_netif != nullptr)) {
        esp_ip6_addr_type_t addr_type    = esp_netif_ip6_get_addr_type(&ip6Event.ip6_info.ip);
        static const char  *addr_types[] = { "UNKNOWN", "GLOBAL", "LINK_LOCAL", "SITE_LOCAL", "UNIQUE_LOCAL", "IPV4_MAPPED_IPV6" };

        if (addr_type < NR_ELEMENTS(addr_types)) {
          addLog(LOG_LEVEL_INFO, strformat(
                   F("%s: Got IPv6: IP Index: %d, Type: %s, Zone: %d, Address: " IPV6STR),
                   _eventInterfaceName.c_str(),
                   ip6Event.ip_index,
                   addr_types[addr_type],
                   ip6Event.ip6_info.ip.zone,
                   IPV62STR(ip6Event.ip6_info.ip)
                   ));
        }
      }
    }
# if FEATURE_MDNS

    //    update_mDNS();
# endif // if FEATURE_MDNS
  }
#endif // if FEATURE_USE_IPV6

  if (connected_changed || establishConnect_changed)
  {
    if (_connectedStats.isOn()) {
      log_connected();

      //    _establishConnectStats.resetCount();
    } else if (_connectedStats.isOff() && !_establishConnectStats.isOn()) {
      log_disconnected();
    }
  }

  _operationalStats.set(operational());

  if (_operationalStats.changedSinceLastCheck_and_clear()) {
#if FEATURE_NETWORK_TRAFFIC_COUNT

    if (_operationalStats.isOn()) {
      enable_txrx_events();
    }
#endif // if FEATURE_NETWORK_TRAFFIC_COUNT

#ifdef ESP8266

    if (!_isAP) {
      // Check for priority changes on ESP8266 is essentially a check to see
      // if we were connected via WiFi or not.
      // So we can call priority route changed here
      String dummy;
      ESPEasy::net::NWPluginCall(NWPlugin::Function::NWPLUGIN_PRIORITY_ROUTE_CHANGED, 0, dummy);
    }
#endif // ifdef ESP8266

    // Send out event
    if (Settings.UseRules && _eventInterfaceName.length())
    {
      if (_operationalStats.isOn()) {
        if (_isAP) {
          eventQueue.addDeDup(F("WiFi#APmodeConnected"));
        }
        else {
          eventQueue.addDeDup(concat(_eventInterfaceName, F("#Connected")));
        }
      } else if (_operationalStats.isOff()) {
        if (_isAP) {
          eventQueue.addDeDup(F("WiFi#APmodeDisconnected"));
        }
        else {
          eventQueue.addDeDup(concat(_eventInterfaceName, F("#Disconnected")));
        }
      }
    }
    statusLED(true);
  }

  if (_startStopStats.changedSinceLastCheck_and_clear() && Settings.UseRules) {
    if (_startStopStats.isOn()) {
      if (_isAP) {
        eventQueue.addDeDup(F("WiFi#APmodeEnabled"));
      }
      else {
        eventQueue.addDeDup(concat(_eventInterfaceName, F("#Enabled")));
      }
    } else if (_startStopStats.isOff()) {
      if (_isAP) {
        eventQueue.addDeDup(F("WiFi#APmodeDisabled"));
      }
      else {
        eventQueue.addDeDup(concat(_eventInterfaceName, F("#Disabled")));
      }
    }
  }
}

void NWPluginData_static_runtime::setStaticIP(const IPAddress& ip, const IPAddress& gateway, const IPAddress& subnetmask,
                                              const IPAddress& dns)
{
  _useStaticIP = IPAddressSet(ip) && IPAddressSet(gateway) && IPAddressSet(subnetmask);
  _ip          = ip;
  _gateway     = gateway;
  _sn          = subnetmask;
  _dns         = dns;
#ifdef ESP32

  if (_useStaticIP) {
    _dns_cache[0] = dns;
    _dns_cache[1] = INADDR_NONE;
  }
#endif // ifdef ESP32
}

String NWPluginData_static_runtime::statusToString() const
{
  String log;

  if (connected()) {
    log += F("Conn. ");
  }

  if (hasIP()) {
    log += F("IP ");

    if (_useStaticIP) { log += F("(static) "); }
  }

  if (operational()) {
    log += F("Init");
  }

  if (log.isEmpty()) { log = F("DISCONNECTED"); }
  return log;
}

bool NWPluginData_static_runtime::stableConnection() const
{
  return _connectedStats.isOn() && _connectedStats.getLastOnDuration_ms() > CONNECTION_CONSIDERED_STABLE_MSEC;
}

uint32_t NWPluginData_static_runtime::getSuggestedTimeout(
  int      index,
  uint32_t minimum_timeout) const
{
  auto it = _connectDurations.find(index);

  if (it == _connectDurations.end()) {
    // Store the suggested timeout as negative value,
    // to make clear it was last suggested, but not yet confirmed.
    _connectDurations[index] = -3 * minimum_timeout;
    return 3 * minimum_timeout;
  }
  const uint32_t returnvalue = constrain(std::abs(3 * it->second), minimum_timeout, CONNECT_TIMEOUT_MAX);

  if (it->second < 0) {
    _connectDurations[index] = -1 * returnvalue;
  } else {
    _connectDurations[index] = returnvalue;
  }
  return returnvalue;
}

void NWPluginData_static_runtime::markConnectionSuccess(
  int      index,
  uint32_t duration_ms) const
{
  auto it = _connectDurations.find(index);

  if ((it == _connectDurations.end()) || (it->second < 0)) { _connectDurations[index] = duration_ms; }

  // Apply some slight filtering so we won't get into trouble when a connection suddenly was faster.
  _connectDurations[index] += 2 * duration_ms;
  _connectDurations[index] /= 3;

  if (_connectionFailures > 0) {
    --_connectionFailures;
  }
}

void NWPluginData_static_runtime::markPublishSuccess() const
{
  if (_connectionFailures > 0) {
    --_connectionFailures;
  }
}

void NWPluginData_static_runtime::markConnectionFailed(int index) const
{
  ++_connectionFailures;
}

} // namespace net
} // namespace ESPEasy
