#pragma once

#include "../DataTypes/NetworkIndex.h"

#include "../../../ESPEasy_common.h"

#include "../../../src/Helpers/LongTermOnOffTimer.h"

#include <IPAddress.h>
#ifdef ESP32
# include <NetworkInterface.h>
#endif

#include <map>
#include <list>

namespace ESPEasy {
namespace net {

#if FEATURE_NETWORK_TRAFFIC_COUNT
struct TX_RX_traffic_count {

  void clear() { _tx_count = 0; _rx_count = 0; _tx_packets = 0; _rx_packets = 0; }

  uint64_t _tx_count{};
  uint64_t _rx_count{};
  uint32_t _tx_packets{};
  uint32_t _rx_packets{};

};

#endif // if FEATURE_NETWORK_TRAFFIC_COUNT

struct NWPluginData_static_runtime {
#ifdef ESP32
  NWPluginData_static_runtime(
    NetworkInterface *netif,
    const String    & eventInterfaceName = EMPTY_STRING);

  NWPluginData_static_runtime(
    bool              isAP,
    NetworkInterface *netif,
    const String    & eventInterfaceName = EMPTY_STRING);
#else // ifdef ESP32
  NWPluginData_static_runtime(bool        isAP,
                              const char *eventInterfaceName);

#endif // ifdef ESP32

#if FEATURE_NETWORK_TRAFFIC_COUNT
  void   enable_txrx_events();

  bool   getTrafficCount(TX_RX_traffic_count& traffic) const;
#endif // if FEATURE_NETWORK_TRAFFIC_COUNT

  void   clear(networkIndex_t networkIndex = INVALID_NETWORK_INDEX);

  void   processEvent_and_clear();

  bool   started() const;

  bool   connected() const;

#if FEATURE_ETHERNET
  bool   linkUp() const;
#endif

  bool   isDefaultRoute() const;

  bool   hasIP() const;

#if FEATURE_USE_IPV6
  bool   hasIPv6() const;
#endif

  bool   operational() const;

  String statusToString() const;

  // Return true when connected over 1 minute.
  bool   stableConnection() const;


  // =============================================
  // mark_xxx() to act on typical events
  // =============================================
  void mark_start();

  void mark_stop();

  void mark_got_IP();
#if FEATURE_USE_IPV6
  void mark_got_IPv6(ip_event_got_ip6_t *event);
#endif

  void mark_lost_IP();

  void mark_begin_establish_connection();

  void mark_connected();
  void log_connected();

  void mark_disconnected();
  void log_disconnected();

  // =============================================
  // Keep track of connection durations
  // per host/interface
  //
  // When a suggested timeout is not 'acknowledged'
  // a next suggested timeout will be 3x longer.
  // =============================================

  // Return a suggested timeout
  uint32_t getSuggestedTimeout(int      index,
                               uint32_t minimum_timeout) const;

  void     markConnectionSuccess(int      index,
                                 uint32_t duration_ms) const;

  void     markPublishSuccess() const;

  void     markConnectionFailed(int index) const;

  uint32_t getConnectionFailures() const { return _connectionFailures; }

  void     processEvents();

  // =============================================
  // OnOffTimers for keeping track of:
  // - start/stop of interface
  // - Connected/Disconnected state
  // - GotIP/LostIP
  // - GotIPv6/LostIP
  // =============================================

  LongTermOnOffTimer _startStopStats{};
  LongTermOnOffTimer _establishConnectStats{};
  LongTermOnOffTimer _connectedStats{};
  LongTermOnOffTimer _gotIPStats{};
#if FEATURE_USE_IPV6
  LongTermOnOffTimer _gotIP6Stats{};
  ip_event_got_ip6_t _gotIP6Events[6]{};
#endif // if FEATURE_USE_IPV6
  LongTermOnOffTimer _operationalStats{}; // is started, connected and had IP

  // =============================================
  // Cached DNS servers & Route Prio
  // =============================================
#ifdef ESP32
  IPAddress _dns_cache[2]{};
  int       _routePrio = -1; // Cached route prio as it is being used from callbacks
#endif // ifdef ESP32

#if FEATURE_USE_IPV6
  bool _enableIPv6{}; // Cached enableIPv6 flag as it is being used from callbacks
#endif

private:

#ifdef ESP32
  NetworkInterface *_netif{};
#endif // ifdef ESP32

  networkIndex_t _networkIndex = INVALID_NETWORK_INDEX;
  const bool _isAP;

  String _eventInterfaceName;

  // Store durations it took to connect to some host.
  // This depends on host and interface.
  // Duration is negative when it was suggested but not actually set.
  // Duration is positive when actually being set
  mutable std::map<int, int32_t>_connectDurations;

  mutable uint32_t _connectionFailures{};

#if FEATURE_NETWORK_TRAFFIC_COUNT
  esp_event_handler_instance_t _handler_inst{};
#endif

};

DEF_UP(NWPluginData_static_runtime);

} // namespace net
} // namespace ESPEasy
