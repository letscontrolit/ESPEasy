#pragma once

#include "../_NWPlugin_Helper.h"
#ifdef USES_NW001


namespace ESPEasy {
namespace net {
namespace wifi {

struct NW002_data_struct_WiFi_AP : public NWPluginData_base {

  NW002_data_struct_WiFi_AP(networkIndex_t networkIndex);
  ~NW002_data_struct_WiFi_AP();


  void webform_load(EventStruct *event);
  void webform_save(EventStruct *event);

  bool webform_getPort(KeyValueWriter *writer);

  bool init(EventStruct *event);

  bool exit(EventStruct *event);

# ifdef ESP32
  bool handle_priority_route_changed() override;
# endif

# if FEATURE_NETWORK_STATS
  bool                         initPluginStats() override;
  bool                         record_stats() override;
  bool                         webformLoad_show_stats(struct EventStruct *event) const override;
# endif // if FEATURE_NETWORK_STATS

  NWPluginData_static_runtime* getNWPluginData_static_runtime();

private:

# ifdef ESP32
  static void onEvent(arduino_event_id_t   event,
                      arduino_event_info_t info);

  network_event_handle_t nw_event_id = 0;
# endif // ifdef ESP32

};


} // namespace wifi
} // namespace net
} // namespace ESPEasy


#endif // ifdef USES_NW001
