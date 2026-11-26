#include "../eth/ETH_NWPluginData_static_runtime.h"

#if FEATURE_ETHERNET


# include "../../../src/Helpers/StringConverter.h"


namespace ESPEasy {
namespace net {
namespace eth {

struct ETH_stats_and_cache_t {
  ETH_stats_and_cache_t(uint8_t eth_index, networkIndex_t networkIndex) :
    _networkIndex(networkIndex),
    _eth_index(eth_index),
    _eth(eth_index),
    _stats_and_cache(&_eth, concat(F("eth"), eth_index)) {
    _stats_and_cache.clear(networkIndex);
# if FEATURE_USE_IPV6
    _eth.enableIPv6(_stats_and_cache._enableIPv6);
# endif

    if (_eth.getStatusBits() & ESP_NETIF_STARTED_BIT) {
      // Bit has been set, so it might be for this interface
      _stats_and_cache.mark_start();
    }
  }

  ~ETH_stats_and_cache_t() {
    _eth.end();
    _stats_and_cache.processEvent_and_clear();
  }

  const networkIndex_t _networkIndex;

  const uint8_t               _eth_index;
  ETHClass                    _eth;
  NWPluginData_static_runtime _stats_and_cache;

};

DEF_UP(ETH_stats_and_cache_t);

network_event_handle_t   nw_event_id = 0;
UP_ETH_stats_and_cache_t ETH_stats_and_cache[3]{};

ETH_stats_and_cache_t* getStatsAndCache(networkIndex_t networkIndex)
{
  for (size_t i = 0; i < NR_ELEMENTS(ETH_stats_and_cache); ++i) {
    if (ETH_stats_and_cache[i] && (ETH_stats_and_cache[i]->_networkIndex == networkIndex)) {
      return ETH_stats_and_cache[i].get();
    }
  }
  return nullptr;
}

ETHClass * ETH_NWPluginData_static_runtime::getInterface(networkIndex_t networkIndex)
{
  auto data = getStatsAndCache(networkIndex);

  if (data) { return &data->_eth; }

  return nullptr;
}

NWPluginData_static_runtime * ETH_NWPluginData_static_runtime::getNWPluginData_static_runtime(networkIndex_t networkIndex)
{
  auto data = getStatsAndCache(networkIndex);

  if (data) { return &data->_stats_and_cache; }

  return nullptr;
}

ETHClass * ETH_NWPluginData_static_runtime::init(networkIndex_t networkIndex)
{
  exit(networkIndex);

  for (size_t i = 0; i < NR_ELEMENTS(ETH_stats_and_cache); ++i) {
    if (!ETH_stats_and_cache[i]) {
      // Found an empty slot
      if (nw_event_id != 0) {
        Network.removeEvent(nw_event_id);
        nw_event_id = 0;
      }

      ETH_stats_and_cache[i].reset(new (std::nothrow) ETH_stats_and_cache_t(i, networkIndex));

      if (!ETH_stats_and_cache[i]) { return nullptr; }
      nw_event_id = Network.onEvent(ETH_NWPluginData_static_runtime::onEvent);

      return &ETH_stats_and_cache[i]->_eth;
    }
  }
  return nullptr;
}

void ETH_NWPluginData_static_runtime::exit(networkIndex_t networkIndex)
{
  for (size_t i = 0; i < NR_ELEMENTS(ETH_stats_and_cache); ++i) {
    if (ETH_stats_and_cache[i]) {
      if (ETH_stats_and_cache[i]->_networkIndex == networkIndex) {
        ETH_stats_and_cache[i].reset(nullptr);

        if (nw_event_id != 0) {
          Network.removeEvent(nw_event_id);
          nw_event_id = 0;
        }
      }
    }
  }

  if (nw_event_id == 0) {
    nw_event_id = Network.onEvent(ETH_NWPluginData_static_runtime::onEvent);
  }
}

void ETH_NWPluginData_static_runtime::onEvent(
  arduino_event_id_t   event,
  arduino_event_info_t info)
{
  for (size_t i = 0; i < NR_ELEMENTS(ETH_stats_and_cache); ++i) {
    if (ETH_stats_and_cache[i]) {
      switch (event)
      {
        case ARDUINO_EVENT_ETH_START:
          if (ETH_stats_and_cache[i]->_eth.handle() == info.eth_started) {
            ETH_stats_and_cache[i]->_stats_and_cache.queueEvent(
              NWPluginData_static_runtime::DeferredEventType::Started);
          }
          break;
        case ARDUINO_EVENT_ETH_STOP:
          if (ETH_stats_and_cache[i]->_eth.handle() == info.eth_stopped) {
            ETH_stats_and_cache[i]->_stats_and_cache.queueEvent(
              NWPluginData_static_runtime::DeferredEventType::Stopped);
          }
          break;
        case ARDUINO_EVENT_ETH_CONNECTED:
          if (ETH_stats_and_cache[i]->_eth.handle() == info.eth_connected) {
            ETH_stats_and_cache[i]->_stats_and_cache.queueEvent(
              NWPluginData_static_runtime::DeferredEventType::Connected);
          }
          break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
          if (ETH_stats_and_cache[i]->_eth.handle() == info.eth_disconnected) {
            ETH_stats_and_cache[i]->_stats_and_cache.queueEvent(
              NWPluginData_static_runtime::DeferredEventType::Disconnected);
          }
          break;
        case ARDUINO_EVENT_ETH_GOT_IP:
          if (info.got_ip.esp_netif == ETH_stats_and_cache[i]->_eth.netif()) {
            ETH_stats_and_cache[i]->_stats_and_cache.queueEvent(
              NWPluginData_static_runtime::DeferredEventType::GotIP);
          }
          break;
# if FEATURE_USE_IPV6
        case ARDUINO_EVENT_ETH_GOT_IP6:
          if (info.got_ip6.esp_netif == ETH_stats_and_cache[i]->_eth.netif()) {
            ETH_stats_and_cache[i]->_stats_and_cache.queueEvent(
              NWPluginData_static_runtime::DeferredEventType::GotIPv6,
              &info.got_ip6);
          }
          break;
# endif // if FEATURE_USE_IPV6
        case ARDUINO_EVENT_ETH_LOST_IP:
          if (info.lost_ip.esp_netif == ETH_stats_and_cache[i]->_eth.netif()) {
            ETH_stats_and_cache[i]->_stats_and_cache.queueEvent(
              NWPluginData_static_runtime::DeferredEventType::LostIP);
          }
          break;

        default:
          // addLog(LOG_LEVEL_INFO, concat(F("ETH Event: "), event));
          break;
      }
    }
  }
}

} // namespace eth
} // namespace net
} // namespace ESPEasy


#endif // if FEATURE_ETHERNET
