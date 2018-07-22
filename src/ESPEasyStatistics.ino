void logStatistics(byte loglevel, bool clearLog) {
  if (loglevelActiveFor(loglevel)) {
    String log;
    log.reserve(80);
    for (auto& x: pluginStats) {
        if (!x.second.isEmpty()) {
            const int pluginId = x.first/32;
            String P_name = "";
            Plugin_ptr[pluginId](PLUGIN_GET_DEVICENAME, NULL, P_name);
            log = F("PluginStats P_");
            log += pluginId + 1;
            log += '_';
            log += P_name;
            log += ' ';
            log += getPluginFunctionName(x.first%32);
            log += ' ';
            log += getLogLine(x.second);
            addLog(loglevel, log);
            if (clearLog) x.second.reset();
        }
    }
    for (auto& x: miscStats) {
        if (!x.second.isEmpty()) {
            log = getMiscStatsName(x.first);
            log += F(" stats: ");
            log += getLogLine(x.second);
            addLog(loglevel, log);
            if (clearLog) x.second.reset();
        }
    }
    log = getMiscStatsName(TIME_DIFF_COMPUTE);
    log += F(" stats: Count: ");
    log += timediff_calls;
    log += F(" - CPU cycles per call: ");
    log += static_cast<float>(timediff_cpu_cycles_total) / static_cast<float>(timediff_calls);
    addLog(loglevel, log);
    if (clearLog) {
      timediff_calls = 0;
      timediff_cpu_cycles_total = 0;
    }
  }
}
