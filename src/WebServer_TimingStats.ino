
#ifdef WEBSERVER_TIMINGSTATS
#include "src/Globals/Device.h"


void handle_timingstats() {
  checkRAM(F("handle_timingstats"));
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);
  html_table_class_multirow();
  html_TR();
  html_table_header(F("Description"));
  html_table_header(F("Function"));
  html_table_header(F("#calls"));
  html_table_header(F("call/sec"));
  html_table_header(F("min (ms)"));
  html_table_header(F("Avg (ms)"));
  html_table_header(F("max (ms)"));

  long timeSinceLastReset = stream_timing_statistics(true);
  html_end_table();

  html_table_class_normal();
  const float timespan = timeSinceLastReset / 1000.0;
  addFormHeader(F("Statistics"));
  addRowLabel(F("Start Period"));
  struct tm startPeriod = addSeconds(tm, -1.0 * timespan, false);
  TXBuffer += getDateTimeString(startPeriod, '-', ':', ' ', false);
  addRowLabelValue(LabelType::LOCAL_TIME);
  addRowLabel(F("Time span"));
  TXBuffer += String(timespan);
  TXBuffer += " sec";
  html_end_table();

  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

// ********************************************************************************
// HTML table formatted timing statistics
// ********************************************************************************
void format_using_threshhold(unsigned long value) {
  float value_msec = value / 1000.0;

  if (value > TIMING_STATS_THRESHOLD) {
    html_B(String(value_msec, 3));
  } else {
    TXBuffer += String(value_msec, 3);
  }
}

void stream_html_timing_stats(const TimingStats& stats, long timeSinceLastReset) {
  unsigned long minVal, maxVal;
  unsigned int  c = stats.getMinMax(minVal, maxVal);

  html_TD();
  TXBuffer += c;
  html_TD();
  float call_per_sec = static_cast<float>(c) / static_cast<float>(timeSinceLastReset) * 1000.0;
  TXBuffer += call_per_sec;
  html_TD();
  format_using_threshhold(minVal);
  html_TD();
  format_using_threshhold(stats.getAvg());
  html_TD();
  format_using_threshhold(maxVal);
}

long stream_timing_statistics(bool clearStats) {
  long timeSinceLastReset = timePassedSince(timingstats_last_reset);

  for (auto& x: pluginStats) {
    if (!x.second.isEmpty()) {
      const int pluginId = x.first / 256;
      if (x.second.thresholdExceeded(TIMING_STATS_THRESHOLD)) {
        html_TR_TD_highlight();
      } else {
        html_TR_TD();
      }
      TXBuffer += F("P_");
      TXBuffer += Device[pluginId].Number;
      TXBuffer += '_';
      TXBuffer += getPluginNameFromDeviceIndex(pluginId);
      html_TD();
      TXBuffer += getPluginFunctionName(x.first % 256);
      stream_html_timing_stats(x.second, timeSinceLastReset);

      if (clearStats) { x.second.reset(); }
    }
  }

  for (auto& x: controllerStats) {
    if (!x.second.isEmpty()) {
      const int pluginId = x.first / 256;
      String    C_name   = "";
      CPluginCall(pluginId, CPLUGIN_GET_DEVICENAME, NULL, C_name);

      if (x.second.thresholdExceeded(TIMING_STATS_THRESHOLD)) {
        html_TR_TD_highlight();
      } else {
        html_TR_TD();
      }
      TXBuffer += F("C_");
      TXBuffer += Protocol[pluginId].Number;
      TXBuffer += '_';
      TXBuffer += C_name;
      html_TD();
      TXBuffer += getCPluginCFunctionName(x.first % 256);
      stream_html_timing_stats(x.second, timeSinceLastReset);

      if (clearStats) { x.second.reset(); }
    }
  }

  for (auto& x: miscStats) {
    if (!x.second.isEmpty()) {
      if (x.second.thresholdExceeded(TIMING_STATS_THRESHOLD)) {
        html_TR_TD_highlight();
      } else {
        html_TR_TD();
      }
      TXBuffer += getMiscStatsName(x.first);
      html_TD();
      stream_html_timing_stats(x.second, timeSinceLastReset);

      if (clearStats) { x.second.reset(); }
    }
  }

  if (clearStats) {
    timingstats_last_reset = millis();
  }
  return timeSinceLastReset;
}

#endif // WEBSERVER_TIMINGSTATS
