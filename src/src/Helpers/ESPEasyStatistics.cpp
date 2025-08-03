#include "../Helpers/ESPEasyStatistics.h"


#if FEATURE_TIMING_STATS

#include "../DataStructs/TimingStats.h"
#include "../WebServer/ESPEasy_WebServer.h"
#include "../Helpers/Convert.h"
#include "../Helpers/_Plugin_init.h"


void stream_json_timing_stats(const TimingStats& stats, long timeSinceLastReset) {
  uint32_t minVal, maxVal;
  uint32_t  count = stats.getMinMax(minVal, maxVal);
  float call_per_sec = static_cast<float>(count) / static_cast<float>(timeSinceLastReset) * 1000.0f;

  json_number(F("count"), ull2String(count));
  json_number(F("call-per-sec"),   toString(call_per_sec, 2));
  json_number(F("min"),   ull2String(minVal));
  json_number(F("max"),   ull2String(maxVal));
  json_number(F("avg"),   toString(stats.getAvg(), 2));
  json_prop(F("unit"), F("usec"));
}

void jsonStatistics(bool clearStats) {
  bool firstPlugin     = true;
  deviceIndex_t  currentDeviceIndex = INVALID_DEVICE_INDEX;
  long timeSinceLastReset = timePassedSince(timingstats_last_reset);


  json_open(true, F("plugin"));

  for (auto& x: pluginStats) {
    if (!x.second.isEmpty()) {
      const deviceIndex_t deviceIndex = deviceIndex_t::toDeviceIndex(x.first >> 8);

      if (deviceIndex != currentDeviceIndex) {
        // new plugin
        currentDeviceIndex = deviceIndex;
        if (!firstPlugin) {
          json_close();
          json_close(true); // close previous function list
          json_close();     // close previous plugin
        }
        // Start new plugin stream
        json_open(); // open new plugin
        json_prop(F("name"), getPluginNameFromDeviceIndex(deviceIndex));
        json_prop(F("id"),   String(getPluginID_from_DeviceIndex(deviceIndex).value));
        json_open(true, F("function")); // open function
        json_open(); // open first function element
      }

      // Stream function timing stats
      json_open(false, getPluginFunctionName(x.first % 256));
      {
        stream_json_timing_stats(x.second, timeSinceLastReset);
      }
      json_close(false);
      firstPlugin = false;
    }
  }
  if (!firstPlugin) {
    // We added some, so we must make sure to close the last entry
    json_close();     // close first function element
    json_close(true); // close previous function
    json_close();     // close previous plugin
  }
  json_close(true);   // Close plugin list


  json_open(true, F("controller"));
  bool firstController = true;
  int  currentProtocolIndex = -1;
  for (auto& x: controllerStats) {
    if (!x.second.isEmpty()) {      
      const int ProtocolIndex = x.first / 256;
      if (currentProtocolIndex != ProtocolIndex) {
        // new protocol
        currentProtocolIndex = ProtocolIndex;
        if (!firstController) {
          json_close();
          json_close(true); // close previous function list
          json_close();     // close previous protocol
        }
        // Start new protocol stream
        json_open(); // open new plugin
        json_prop(F("name"), getCPluginNameFromProtocolIndex(ProtocolIndex));
        json_prop(F("id"),   String(getCPluginID_from_ProtocolIndex(ProtocolIndex)));
        json_open(true, F("function")); // open function
        json_open(); // open first function element

      }
      // Stream function timing stats
      json_open(false, getCPluginCFunctionName(static_cast<CPlugin::Function>(x.first % 256)));
      {
        stream_json_timing_stats(x.second, timeSinceLastReset);
      }
      json_close(false);
      firstController = false;
    }
  }
  if (!firstController) {
    // We added some, so we must make sure to close the last entry
    json_close();     // close first function element
    json_close(true); // close previous function
    json_close();     // close previous plugin
  }

  json_close(true);   // Close controller list


  json_open(true, F("misc"));
  for (auto& x: miscStats) {
    if (!x.second.isEmpty()) {
      json_open(); // open new misc item
      json_prop(F("name"), getMiscStatsName(x.first));
      json_prop(F("id"),   String(static_cast<int>(x.first)));
      json_open(true, F("function")); // open function
      json_open(); // open first function element
      // Stream function timing stats
      json_open(false, to_internal_string(getMiscStatsName(x.first), '-'));
      {
        stream_json_timing_stats(x.second, timeSinceLastReset);
      }
      json_close(false);
      json_close();     // close first function element
      json_close(true); // close function
      json_close();     // close misc item
    }
  }

  json_close(true);   // Close misc list

  if (clearStats) {
    pluginStats.clear();
    controllerStats.clear();
    miscStats.clear();
    timingstats_last_reset = millis();
  }
}


#endif // if FEATURE_TIMING_STATS