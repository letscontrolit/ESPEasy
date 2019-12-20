
#ifdef WEBSERVER_TIMINGSTATS
#include "src/Globals/Device.h"


void handle_timingstats(void) {
  checkRAM(F("handle_timingstats"));
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream(void);
  sendHeadandTail_stdtemplate(_HEAD);
  html_table_class_multirow(void);
  html_TR(void);
  html_table_header(F("Description"));
  html_table_header(F("Function"));
  html_table_header(F("#calls"));
  html_table_header(F("call/sec"));
  html_table_header(F("min (ms)"));
  html_table_header(F("Avg (ms)"));
  html_table_header(F("max (ms)"));

  long timeSinceLastReset = stream_timing_statistics(true);
  html_end_table(void);

  html_table_class_normal(void);
  const float timespan = timeSinceLastReset / 1000.0;
  addFormHeader(F("Statistics"));
  addRowLabel(F("Start Period"));
  struct tm startPeriod = addSeconds(tm, -1.0 * timespan, false);
  TXBuffer += getDateTimeString(startPeriod, '-', ':', ' ', false);
  addRowLabelValue(LabelType::LOCAL_TIME);
  addRowLabel(F("Time span"));
  TXBuffer += String(timespan);
  TXBuffer += " sec";
  html_end_table(void);

  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream(void);
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

  html_TD(void);
  TXBuffer += c;
  html_TD(void);
  float call_per_sec = static_cast<float>(c) / static_cast<float>(timeSinceLastReset) * 1000.0;
  TXBuffer += call_per_sec;
  html_TD(void);
  format_using_threshhold(minVal);
  html_TD(void);
  format_using_threshhold(stats.getAvg(void));
  html_TD(void);
  format_using_threshhold(maxVal);
}

long stream_timing_statistics(bool clearStats) {
  long timeSinceLastReset = timePassedSince(timingstats_last_reset);

  for (auto& x: pluginStats) {
    if (!x.second.isEmpty(void)) {
      const deviceIndex_t deviceIndex = static_cast<deviceIndex_t>(x.first / 256);
      if (validDeviceIndex(deviceIndex)) {
        if (x.second.thresholdExceeded(TIMING_STATS_THRESHOLD)) {
          html_TR_TD_highlight(void);
        } else {
          html_TR_TD(void);
        }
        TXBuffer += F("P_");
        TXBuffer += Device[deviceIndex].Number;
        TXBuffer += '_';
        TXBuffer += getPluginNameFromDeviceIndex(deviceIndex);
        html_TD(void);
        TXBuffer += getPluginFunctionName(x.first % 256);
        stream_html_timing_stats(x.second, timeSinceLastReset);
      }

      if (clearStats) { x.second.reset(void); }
    }
  }

  for (auto& x: controllerStats) {
    if (!x.second.isEmpty(void)) {
      const int ProtocolIndex = x.first / 256;
      if (x.second.thresholdExceeded(TIMING_STATS_THRESHOLD)) {
        html_TR_TD_highlight(void);
      } else {
        html_TR_TD(void);
      }
      TXBuffer += F("C_");
      TXBuffer += Protocol[ProtocolIndex].Number;
      TXBuffer += '_';
      TXBuffer += getCPluginNameFromProtocolIndex(ProtocolIndex);
      html_TD(void);
      TXBuffer += getCPluginCFunctionName(x.first % 256);
      stream_html_timing_stats(x.second, timeSinceLastReset);

      if (clearStats) { x.second.reset(void); }
    }
  }

  for (auto& x: miscStats) {
    if (!x.second.isEmpty(void)) {
      if (x.second.thresholdExceeded(TIMING_STATS_THRESHOLD)) {
        html_TR_TD_highlight(void);
      } else {
        html_TR_TD(void);
      }
      TXBuffer += getMiscStatsName(x.first);
      html_TD(void);
      stream_html_timing_stats(x.second, timeSinceLastReset);

      if (clearStats) { x.second.reset(void); }
    }
  }

  if (clearStats) {
    timingstats_last_reset = millis(void);
  }
  return timeSinceLastReset;
}

#endif // WEBSERVER_TIMINGSTATS
