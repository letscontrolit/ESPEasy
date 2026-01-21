#pragma once

#include "../../../ESPEasy_common.h"

#include "../DataTypes/NWPluginID.h"
#include "../DataTypes/NetworkIndex.h"
#include "../DataStructs/NWPluginData_static_runtime.h"
#include "../../../src/DataStructs/ESPEasy_EventStruct.h"
#include "../../../src/DataStructs/PluginStats_array.h"

#if FEATURE_STORE_NETWORK_INTERFACE_SETTINGS
# include "../../../src/Helpers/_ESPEasy_key_value_store.h"
# include "../../../src/Helpers/ESPEasy_key_value_store_import_export.h"
#endif

namespace ESPEasy {
namespace net {


// =============================================
// Data used by instances of NW-plugins.
// =============================================

// base class to be able to delete a data object from the array.
// N.B. in order to use this, a data object must inherit from this base class.
//      This is a compile time check.
struct NWPluginData_base {
  NWPluginData_base(nwpluginID_t      nwpluginID,
                    networkIndex_t networkIndex
#ifdef                                ESP32
                    ,
                    NetworkInterface *netif
#endif // ifdef ESP32
                    );

  virtual ~NWPluginData_base();

  virtual bool init(EventStruct *event) = 0;

  virtual bool exit(EventStruct *event) = 0;

  bool         hasPluginStats() const;

  bool         hasPeaks() const;

  size_t       nrSamplesPresent() const;

  #if FEATURE_NETWORK_STATS
  virtual bool initPluginStats();

  void         clearPluginStats(networkStatsVarIndex_t networkStatsVarIndex);

  // Update any logged timestamp with this newly set system time.
  void         processTimeSet(const double& time_offset);
  #endif // if FEATURE_NETWORK_STATS


  bool pushStatsValues(EventStruct *event,
                       size_t       valueCount,
                       bool         trackPeaks,
                       bool         onlyUpdateTimestampWhenSame);


  bool plugin_write_base(EventStruct  *event,
                         const String& string);

#if FEATURE_NETWORK_STATS
  virtual bool record_stats();
  virtual bool webformLoad_show_stats(EventStruct *event) const;

# if FEATURE_CHART_JS
  void         plot_ChartJS(bool onlyJSON = false) const;

  void         plot_ChartJS_scatter(
    networkStatsVarIndex_t        values_X_axis_index,
    networkStatsVarIndex_t        values_Y_axis_index,
    const __FlashStringHelper    *id,
    const ChartJS_title         & chartTitle,
    const ChartJS_dataset_config& datasetConfig,
    bool                          showAverage = true,
    const String                & options     = EMPTY_STRING,
    bool                          onlyJSON    = false) const;

# endif // if FEATURE_CHART_JS
#endif  // if FEATURE_NETWORK_STATS


  // Should only be called from initNWPluginData
#if FEATURE_STORE_NETWORK_INTERFACE_SETTINGS
  bool                            init_KVS();
#endif

  nwpluginID_t                    getNWPluginID() const { return _nw_data_pluginID; }

  virtual LongTermTimer::Duration getConnectedDuration_ms();
  virtual bool                    handle_nwplugin_write(EventStruct *event,
                                                        String     & str);

#ifdef ESP32
  virtual bool                         handle_priority_route_changed();
#endif
#if FEATURE_NETWORK_TRAFFIC_COUNT
  void                                 enable_txrx_events();
  bool                                 getTrafficCount(TX_RX_traffic_count& traffic);
#endif // if FEATURE_NETWORK_TRAFFIC_COUNT

  virtual NWPluginData_static_runtime* getNWPluginData_static_runtime() = 0;


#if FEATURE_NETWORK_STATS

  PluginStats* getPluginStats(networkStatsVarIndex_t networkStatsVarIndex) const;

  PluginStats* getPluginStats(networkStatsVarIndex_t networkStatsVarIndex);

protected:

  void initPluginStats(
    networkStatsVarIndex_t      networkStatsVarIndex,
    const String              & label,
    uint8_t                     nrDecimals,
    float                       errorValue,
    const PluginStats_Config_t& displayConfig);

# if FEATURE_NETWORK_TRAFFIC_COUNT
  void initPluginStats_trafficCount(networkStatsVarIndex_t networkStatsVarIndex,
                                    bool                   isTX);
# endif // if FEATURE_NETWORK_TRAFFIC_COUNT


  // Array of pointers to PluginStats. One per task value.
  PluginStats_array *_plugin_stats_array = nullptr;
#endif // if FEATURE_NETWORK_STATS

protected:

#ifdef ESP32
//  bool _restore_DNS_cache();
#endif

#if FEATURE_STORE_NETWORK_INTERFACE_SETTINGS

  bool _KVS_initialized() const { return _kvs != nullptr; }

  // Load settings in the _kvs
  bool _load();

  // Save settings from the _kvs to the settings
  bool _store();

  ESPEasy_key_value_store *_kvs = nullptr;
#endif // if FEATURE_STORE_NETWORK_INTERFACE_SETTINGS

  // We cannot use dynamic_cast, so we must keep track of the plugin ID to
  // perform checks on the casting.
  // This is also a check to only use these functions and not to insert pointers
  // at random in the Plugin_task_data array.
  nwpluginID_t   _nw_data_pluginID = INVALID_NW_PLUGIN_ID;
  networkIndex_t _networkIndex = INVALID_NETWORK_INDEX;

#ifdef ESP32
  NetworkInterface *_netif{};
# if FEATURE_NETWORK_STATS && FEATURE_NETWORK_TRAFFIC_COUNT
  uint64_t _prevTX{};
  uint64_t _prevRX{};
# endif // if FEATURE_NETWORK_STATS && FEATURE_NETWORK_TRAFFIC_COUNT
#endif // ifdef ESP32

};

} // namespace net
} // namespace ESPEasy
