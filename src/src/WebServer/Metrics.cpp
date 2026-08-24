#include "../WebServer/Metrics.h"

#include "../Commands/Diagnostic.h"
#include "../../_Plugin_Helper.h"

#ifdef WEBSERVER_METRICS

#include "../../ESPEasy/net/ESPEasyNetwork.h"

# if FEATURE_NETWORK_STATS
#  ifdef ESP8266
#   define MAX_NR_NETWORKS_IN_TABLE  2
#  endif
#  ifdef ESP32
#   define MAX_NR_NETWORKS_IN_TABLE  NETWORK_MAX
#  endif

# endif // if FEATURE_NETWORK_STATS

# ifdef ESP32
#  include <esp_partition.h>
# endif // ifdef ESP32

void handle_metrics() {
  TXBuffer.startStream(F("text/plain"), F("*"));
  const __FlashStringHelper *prefixHELP = F("# HELP espeasy_");
  const __FlashStringHelper *prefixTYPE = F("# TYPE espeasy_");

  // uptime
  addHtml(prefixHELP);
  addHtml(F("uptime current device uptime in minutes\n"));
  addHtml(prefixTYPE);
  addHtml(F("uptime counter\n"));
  addHtml(F("espeasy_uptime "));
  addHtml(getValue(LabelType::UPTIME));
  addHtml('\n');

  // load
  addHtml(prefixHELP);
  addHtml(F("load device percentage load\n"));
  addHtml(prefixTYPE);
  addHtml(F("load gauge\n"));
  addHtml(F("espeasy_load "));
  addHtml(getValue(LabelType::LOAD_PCT));
  addHtml('\n');

  // Free RAM
  addHtml(prefixHELP);
  addHtml(F("free_ram device amount of RAM free in Bytes\n"));
  addHtml(prefixTYPE);
  addHtml(F("free_ram gauge\n"));
  addHtml(F("espeasy_free_ram "));
  addHtml(getValue(LabelType::FREE_MEM));
  addHtml('\n');

  // Free RAM
  addHtml(prefixHELP);
  addHtml(F("free_stack device amount of Stack free in Bytes\n"));
  addHtml(prefixTYPE);
  addHtml(F("free_stack gauge\n"));
  addHtml(F("espeasy_free_stack "));
  addHtml(getValue(LabelType::FREE_STACK));
  addHtml('\n');

  // Wifi strength
  addHtml(prefixHELP);
  addHtml(F("wifi_rssi Wifi connection Strength\n"));
  addHtml(prefixTYPE);
  addHtml(F("wifi_rssi gauge\n"));
  addHtml(F("espeasy_wifi_rssi "));
  addHtml(getValue(LabelType::WIFI_RSSI));
  addHtml('\n');

  // Wifi uptime
  addHtml(prefixHELP);
  addHtml(F("wifi_connected Time wifi has been connected in milliseconds\n"));
  addHtml(prefixTYPE);
  addHtml(F("wifi_connected counter\n"));
  addHtml(F("espeasy_wifi_connected "));
  addHtml(getValue(LabelType::CONNECTED_MSEC));
  addHtml('\n');

  // Wifi reconnects
  addHtml(prefixHELP);
  addHtml(F("wifi_reconnects Number of times Wifi has reconnected since boot\n"));
  addHtml(prefixTYPE);
  addHtml(F("wifi_reconnects counter\n"));
  addHtml(F("espeasy_wifi_reconnects "));
  addHtml(getValue(LabelType::NUMBER_RECONNECTS));
  addHtml('\n');

  # if FEATURE_NETWORK_STATS

  for (ESPEasy::net::networkIndex_t x = 0; x < MAX_NR_NETWORKS_IN_TABLE; x++)
  {
    const ESPEasy::net::nwpluginID_t nwpluginID = Settings.getNWPluginID_for_network(x);
    const bool nwplugin_set                     = nwpluginID.isValid();

    if (nwplugin_set)
    {
      struct EventStruct TempEvent;

      TempEvent.NetworkIndex = x;

      const bool res = ESPEasy::net::NWPluginCall(NWPlugin::Function::NWPLUGIN_GET_TRAFFIC_COUNT, &TempEvent);

      if (res) {
        const String networkName = ESPEasy::net::makeRFCCompliantName(ESPEasy::net::getNWPluginNameFromNWPluginID(nwpluginID), '_', '_', 0);

        // Network traffic
        addHtml(prefixHELP);
        addHtml(F("network_"));
        addHtml(networkName);
        addHtml(F(" Network Data TX and RX totals in bytes and frames\n"));
        addHtml(prefixTYPE);
        addHtml(F("network_"));
        addHtml(networkName);
        addHtml(F(" gauge\n"));

        const __FlashStringHelper*names[] = { F("TX_bytes"), F("RX_bytes"), F("TX_frames"), F("RX_frames") };
        const int64_t values[]            = { TempEvent.Par64_1, TempEvent.Par64_2, TempEvent.Par5, TempEvent.Par6 };
        constexpr uint8_t vcount          = NR_ELEMENTS(values);

        for (uint8_t n = 0; n < vcount; ++n) {
          addHtml(F("espeasy_network_"));
          addHtml(networkName);
          addHtml(F("{valueName=\""));
          addHtml(names[n]);
          addHtml(F("_total\"} "));
          addHtml(ll2String(values[n]));
          addHtml('\n');
        }
      }
    }
  }
  # endif // if FEATURE_NETWORK_STATS

  # if FEATURE_INTERNAL_TEMPERATURE

  // CPU Temperature
  addHtml(prefixHELP);
  addHtml(F("cpu_temperature Level of CPU temperature in Celcius\n"));
  addHtml(prefixTYPE);
  addHtml(F("cpu_temperature gauge\n"));
  addHtml(F("espeasy_cpu_temperature "));
  addHtml(getValue(LabelType::INTERNAL_TEMPERATURE));
  addHtml('\n');
  # endif // if FEATURE_INTERNAL_TEMPERATURE

  // devices
  handle_metrics_devices();

  TXBuffer.endStream();
}

void handle_metrics_devices() {
  for (taskIndex_t x = 0; validTaskIndex(x); x++) {
    const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(x);
    const bool pluginID_set         = INVALID_PLUGIN_ID != Settings.getPluginID_for_task(x);

    if (pluginID_set) {
      if (Settings.TaskDeviceEnabled(x)) {
        String deviceName = getTaskDeviceName(x);

        if (deviceName.isEmpty()) { // Empty name, then use taskN
          deviceName  = F("task");
          deviceName += x + 1;
        }
        addHtml(F("# HELP espeasy_device_"));
        addHtml(deviceName);
        addHtml(F(" Values from connected device\n"));
        addHtml(F("# TYPE espeasy_device_"));
        addHtml(deviceName);
        addHtml(F(" gauge\n"));

        if (validDeviceIndex(DeviceIndex)) {
          String customValuesString;

          // const bool customValues = PluginCall(PLUGIN_WEBFORM_SHOW_VALUES, &TempEvent, customValuesString);
          const bool customValues = 0; // TODO: handle custom values

          if (!customValues) {
            const uint8_t valueCount = getValueCountForTask(x);
            struct EventStruct TempEvent(x);

            for (uint8_t varNr = 0; varNr < valueCount; varNr++) {
              if (validPluginID_fullcheck(Settings.getPluginID_for_task(x))) {
                addHtml(F("espeasy_device_"));
                addHtml(deviceName);
                addHtml(F("{valueName=\""));
                addHtml(Cache.getTaskDeviceValueName(x, varNr));
                addHtml(F("\"} "));
                const String value(formatUserVarNoCheck(&TempEvent, varNr));

                if (value.isEmpty()) {
                  addHtml('0'); // Return 0 for not-set values
                } else {
                  addHtml(value);
                }
                addHtml('\n');
              }
            }
          }
        }
      }
    }
  }
}

#endif // WEBSERVER_METRICS
