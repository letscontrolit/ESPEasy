#include "../WebServer/Metrics.h"
#include "../WebServer/ESPEasy_WebServer.h"
#include "../../ESPEasy-Globals.h"
#include "../Commands/Diagnostic.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyWifi.h"
#include "../../_Plugin_Helper.h"
#include "../Helpers/ESPEasyStatistics.h"
#include "../Static/WebStaticData.h"

#ifdef WEBSERVER_METRICS

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

  // devices
  handle_metrics_devices();

  TXBuffer.endStream();
}

void handle_metrics_devices() {
  for (taskIndex_t x = 0; validTaskIndex(x); x++) {
    const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(x);
    const bool pluginID_set         = INVALID_PLUGIN_ID != Settings.TaskDeviceNumber[x];

    if (pluginID_set) {
      if (Settings.TaskDeviceEnabled[x]) {
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

            for (uint8_t varNr = 0; varNr < valueCount; varNr++) {
              if (validPluginID_fullcheck(Settings.TaskDeviceNumber[x])) {
                addHtml(F("espeasy_device_"));
                addHtml(deviceName);
                addHtml(F("{valueName=\""));
                addHtml(getTaskValueName(x, varNr));
                addHtml(F("\"} "));
                addHtml(formatUserVarNoCheck(x, varNr));
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
