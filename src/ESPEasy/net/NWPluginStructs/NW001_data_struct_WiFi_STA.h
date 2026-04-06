#pragma once

#include "../_NWPlugin_Helper.h"
#ifdef USES_NW001

# ifdef ESP8266
#  include "../wifi/ESPEasyWiFi_STA_Event_ESP8266.h"
# endif
# ifdef ESP32
#  include "../wifi/ESPEasyWiFi_STA_Event_ESP32.h"
# endif


namespace ESPEasy {
namespace net {
namespace wifi {

struct NW001_data_struct_WiFi_STA : public NWPluginData_base {

  NW001_data_struct_WiFi_STA(networkIndex_t networkIndex);
  ~NW001_data_struct_WiFi_STA();


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
  const __FlashStringHelper*   getWiFi_encryptionType() const;

  WiFiDisconnectReason getWiFi_disconnectReason() const;

private:

  ESPEasyWiFi_STA_EventHandler _WiFiEventHandler;

};


} // namespace wifi
} // namespace net
} // namespace ESPEasy


#endif // ifdef USES_NW001
