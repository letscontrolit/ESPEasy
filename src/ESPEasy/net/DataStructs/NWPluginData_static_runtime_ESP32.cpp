#include "../DataStructs/NWPluginData_static_runtime.h"

#ifdef ESP32

# include "../ESPEasyNetwork.h"

# include "../../../src/Helpers/StringConverter.h"
# include "../../../src/Globals/Settings.h"

# include "../Globals/NetworkState.h"

# include <esp_netif.h>
# include <esp_netif_types.h>

# define NW_PLUGIN_LOG_EVENTS   false

namespace ESPEasy {
namespace net {

NWPluginData_static_runtime::NWPluginData_static_runtime(
  NetworkInterface *netif,
  const String    & eventInterfaceName)
  : _netif(netif),
  _isAP(false),
  _eventInterfaceName(eventInterfaceName)
{
  if (eventInterfaceName.isEmpty() && (_netif != nullptr)) {
    _eventInterfaceName = _netif->desc();
    _eventInterfaceName.toUpperCase();
  }
}

NWPluginData_static_runtime::NWPluginData_static_runtime(
  bool              isAP,
  NetworkInterface *netif,
  const String    & eventInterfaceName)
  : _netif(netif),
  _isAP(isAP),
  _eventInterfaceName(eventInterfaceName)
{
  if (eventInterfaceName.isEmpty() && (_netif != nullptr)) {
    _eventInterfaceName = _netif->desc();
    _eventInterfaceName.toUpperCase();
  }
}

bool NWPluginData_static_runtime::started() const
{
  if (!_netif) { return false; }
  return _netif->started();
}

bool NWPluginData_static_runtime::connected() const
{
  if (!_netif) { return false; }
  return _netif->connected();
}

# if FEATURE_ETHERNET

bool NWPluginData_static_runtime::linkUp() const
{
  if (!_netif) { return false; }
  return _netif->linkUp();
}

# endif // if FEATURE_ETHERNET

bool NWPluginData_static_runtime::isDefaultRoute() const
{
  if (!_netif) { return false; }
  return _netif->isDefault();
}

bool NWPluginData_static_runtime::hasIP() const
{
  if (!_netif) { return false; }
  return _netif->hasIP();
}

# if FEATURE_USE_IPV6

bool NWPluginData_static_runtime::hasIPv6() const
{
  if (!_netif) { return false; }
  return _netif->hasGlobalIPv6() || _netif->hasLinkLocalIPv6();
}

# endif // if FEATURE_USE_IPV6

void NWPluginData_static_runtime::mark_start()
{
  //  if (!_startStopStats.setOn()) { return; }
  _startStopStats.setOn();

  if (!_netif) { return; }

  _netif->setHostname(NetworkCreateRFCCompliantHostname().c_str());
# if FEATURE_USE_IPV6
  _netif->enableIPv6(_enableIPv6);
# endif

# if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)

  if (_routePrio > 0) {
    _netif->setRoutePrio(_routePrio);
  }
# endif // if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
# if NW_PLUGIN_LOG_EVENTS

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, strformat(
             F("%s: Started"),
             _eventInterfaceName.c_str()));
  }
# endif // if NW_PLUGIN_LOG_EVENTS
}

void NWPluginData_static_runtime::mark_stop()
{
  _startStopStats.setOff();
# if NW_PLUGIN_LOG_EVENTS

  if (_netif && loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, strformat(
             F("%s: Stopped"),
             _eventInterfaceName.c_str()));
  }
# endif // if NW_PLUGIN_LOG_EVENTS
}

void NWPluginData_static_runtime::mark_got_IP()
{
  // Set OnOffTimer to off so we can also count how often we get new IP
  _gotIPStats.forceSet(true);

  if (!_netif) { return; }

  if (!_netif->isDefault()) {
    nonDefaultNetworkInterface_gotIP = true;
  }

  for (size_t i = 0; i < NR_ELEMENTS(_dns_cache); ++i) {
    auto tmp = _netif->dnsIP(i);

    _dns_cache[i] = tmp; // Also set the 'empty' ones so we won't set left-over DNS server from when another interface was active.
# if NW_PLUGIN_LOG_EVENTS

    if ((tmp != INADDR_NONE) && loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO, strformat(
               F("%s: DNS Cache %d set to %s"),
               _eventInterfaceName.c_str(),
               i,
               tmp.toString(true).c_str()));
    }
# endif // if NW_PLUGIN_LOG_EVENTS
  }
# if NW_PLUGIN_LOG_EVENTS

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, strformat(
             F("%s: Got IP: %s"),
             _eventInterfaceName.c_str(),
             _netif->localIP().toString().c_str()
             ));
  }
# endif // if NW_PLUGIN_LOG_EVENTS

}

# if FEATURE_USE_IPV6

void NWPluginData_static_runtime::mark_got_IPv6(ip_event_got_ip6_t *event)
{
  if (!_netif || !event) { return; }

  if (_netif->netif() != event->esp_netif) { return; }
  _gotIP6Stats.setOn();


  if (!_netif->isDefault()) {
    nonDefaultNetworkInterface_gotIP = true;
  }

  if (event) {
    if (event->ip_index < NR_ELEMENTS(_gotIP6Events)) {
      memcpy(&_gotIP6Events[event->ip_index], event, sizeof(ip_event_got_ip6_t));
      _gotIP6Stats.forceSet(true);
    }
  }
}

# endif // if FEATURE_USE_IPV6

void NWPluginData_static_runtime::mark_lost_IP()
{
  _gotIPStats.setOff();
# if FEATURE_USE_IPV6
  _gotIP6Stats.setOff();
# endif
# if NW_PLUGIN_LOG_EVENTS

  if (_netif && loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, strformat(
             F("%s: Lost IP"),
             _eventInterfaceName.c_str()));
  }
# endif // if NW_PLUGIN_LOG_EVENTS
}

void NWPluginData_static_runtime::mark_begin_establish_connection()
{
  _establishConnectStats.forceSet(true);
  _connectedStats.setOff();
  _operationalStats.setOff();
# if FEATURE_USE_IPV6

  if (_netif) {
    _enableIPv6 = Settings.EnableIPv6() && Settings.getNetworkEnabled_IPv6(_networkIndex);
    _netif->enableIPv6(_enableIPv6);
  } else {
    _enableIPv6 = false;
  }
# endif // if FEATURE_USE_IPV6
# ifdef ESP32
  _routePrio = Settings.getRoutePrio_for_network(_networkIndex);
# endif
}

void NWPluginData_static_runtime::mark_connected()
{
  _establishConnectStats.setOff();
  _connectedStats.forceSet(true);
}

void NWPluginData_static_runtime::log_connected()
{
  if (_netif && loglevelActiveFor(LOG_LEVEL_INFO)) {
    if (_establishConnectStats.getCycleCount()) {
      // Log duration
      addLog(LOG_LEVEL_INFO, strformat(
               F("%s: Connected, took: %s in %d attempts"),
               _eventInterfaceName.c_str(),
               format_msec_duration_HMS(
                 _establishConnectStats.getLastOnDuration_ms()).c_str(),
               _establishConnectStats.getCycleCount()));
    } else if (_establishConnectStats.isOff()) {
      addLog(LOG_LEVEL_INFO, strformat(
               F("%s: Connected, took: %s"),
               _eventInterfaceName.c_str(),
               format_msec_duration_HMS(
                 _establishConnectStats.getLastOnDuration_ms()).c_str()));
    } else {
      addLog(LOG_LEVEL_INFO, strformat(
               F("%s: Connected"),
               _eventInterfaceName.c_str()));
    }
  }
}

void NWPluginData_static_runtime::mark_disconnected()
{
  _establishConnectStats.setOff();
  _connectedStats.setOff();
  _operationalStats.setOff();

  // TODO TD-er: Also clear _gotIPStats and _gotIP6Stats ?
}

void NWPluginData_static_runtime::log_disconnected()
{
  if (_netif && loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, strformat(
             F("%s: Disconnected. Connected for: %s"),
             _eventInterfaceName.c_str(),
             format_msec_duration_HMS(
               _connectedStats.getLastOnDuration_ms()).c_str()));
  }
}

} // namespace net
} // namespace ESPEasy

#endif // ifdef ESP32
