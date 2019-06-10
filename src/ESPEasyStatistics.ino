/*
void logStatistics(byte loglevel, bool clearStats) {
  if (loglevelActiveFor(loglevel)) {
    String log;
    log.reserve(80);
    for (auto& x: pluginStats) {
        if (!x.second.isEmpty()) {
            const int pluginId = x.first/256;
            String P_name = "";
            Plugin_ptr[pluginId](PLUGIN_GET_DEVICENAME, NULL, P_name);
            log = F("PluginStats P_");
            log += pluginId + 1;
            log += '_';
            log += P_name;
            log += ' ';
            log += getPluginFunctionName(x.first%256);
            log += ' ';
            log += getLogLine(x.second);
            addLog(loglevel, log);
            if (clearStats) x.second.reset();
        }
    }
    for (auto& x: miscStats) {
        if (!x.second.isEmpty()) {
            log = getMiscStatsName(x.first);
            log += F(" stats: ");
            log += getLogLine(x.second);
            addLog(loglevel, log);
            if (clearStats) x.second.reset();
        }
    }
    log = getMiscStatsName(TIME_DIFF_COMPUTE);
    log += F(" stats: Count: ");
    log += timediff_calls;
    log += F(" - CPU cycles per call: ");
    log += static_cast<float>(timediff_cpu_cycles_total) / static_cast<float>(timediff_calls);
    addLog(loglevel, log);
    if (clearStats) {
      timediff_calls = 0;
      timediff_cpu_cycles_total = 0;
    }
  }
}
*/
void jsonStatistics(bool clearStats) {
  bool firstPlugin = true;
  bool firstFunction = true;
  int currentPluginId = -1;
  stream_json_start_array(F("plugin"));
  for (auto& x: pluginStats) {
    if (!x.second.isEmpty()) {
      const int pluginId = x.first/256;
      if (currentPluginId != pluginId) {
        // new plugin
        currentPluginId = pluginId;
        if (!firstFunction) {
          // close previous function
          stream_json_end_object_element(true); // close open function object
        }
        if (!firstPlugin) {
          // close previous plugin
          stream_json_end_array_element(true); // Close open function array
          stream_json_end_object_element(false);
        }
        // Start new plugin stream
        stream_plugin_timing_stats_json(pluginId);
        firstFunction = true;
      } else {
        if (!firstFunction) {
          // add comma to start new function
          stream_json_end_object_element(false);
        }
      }

      unsigned long minVal, maxVal;
      unsigned int c = x.second.getMinMax(minVal, maxVal);
      stream_plugin_function_timing_stats_json(getPluginFunctionName(x.first%256),
                                               c, minVal, maxVal, x.second.getAvg());
      if (clearStats) x.second.reset();
      firstFunction = false;
      firstPlugin = false;
    }
  }
  stream_json_end_object_element(true); // end "function" object
  stream_json_end_array_element(true);  // end "function" array
  stream_json_end_object_element(true); // end "plugin" object
  stream_json_end_array_element(true);  // end "plugin" array
  if (clearStats) {
    timediff_calls = 0;
    timediff_cpu_cycles_total = 0;
  }
}
