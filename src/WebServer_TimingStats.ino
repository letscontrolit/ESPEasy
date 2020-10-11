#include "ESPEasy_common.h"


#if defined(WEBSERVER_TIMINGSTATS) && defined(USES_TIMING_STATS)
#include "src/Globals/Device.h"


#define TIMING_STATS_THRESHOLD 100000

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
  const float timespan = timeSinceLastReset / 1000.0f;
  addFormHeader(F("Statistics"));
  addRowLabel(F("Start Period"));
  struct tm startPeriod = node_time.addSeconds(node_time.tm, -1.0 * timespan, false);
  addHtml(ESPEasy_time::getDateTimeString(startPeriod, '-', ':', ' ', false));
  addRowLabelValue(LabelType::LOCAL_TIME);
  addRowLabel(F("Time span"));
  addHtml(String(timespan));
  addHtml(F(" sec"));
  html_end_table();

  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

// ********************************************************************************
// HTML table formatted timing statistics
// ********************************************************************************
void format_using_threshhold(unsigned long value) {
  float value_msec = value / 1000.0f;

  if (value > TIMING_STATS_THRESHOLD) {
    html_B(String(value_msec, 3));
  } else {
    addHtml(String(value_msec, 3));
  }
}

void stream_html_timing_stats(const TimingStats& stats, long timeSinceLastReset) {
  unsigned long minVal, maxVal;
  unsigned int  c = stats.getMinMax(minVal, maxVal);

  html_TD();
  addHtml(String(c));
  html_TD();
  float call_per_sec = static_cast<float>(c) / static_cast<float>(timeSinceLastReset) * 1000.0f;
  addHtml(String(call_per_sec, 2));
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
      const deviceIndex_t deviceIndex = static_cast<deviceIndex_t>(x.first / 256);

      if (validDeviceIndex(deviceIndex)) {
        if (x.second.thresholdExceeded(TIMING_STATS_THRESHOLD)) {
          html_TR_TD_highlight();
        } else {
          html_TR_TD();
        }
        {
          String html;
          html.reserve(64);
          html += F("P_");
          html += Device[deviceIndex].Number;
          html += '_';
          html += getPluginNameFromDeviceIndex(deviceIndex);
          addHtml(html);
        }
        html_TD();
        addHtml(getPluginFunctionName(x.first % 256));
        stream_html_timing_stats(x.second, timeSinceLastReset);
      }

      if (clearStats) { x.second.reset(); }
    }
  }

  for (auto& x: controllerStats) {
    if (!x.second.isEmpty()) {
      const int ProtocolIndex = x.first / 256;

      if (x.second.thresholdExceeded(TIMING_STATS_THRESHOLD)) {
        html_TR_TD_highlight();
      } else {
        html_TR_TD();
      }
      {
        String html;
        html.reserve(64);

        html += F("C_");
        html += Protocol[ProtocolIndex].Number;
        html += '_';
        html += getCPluginNameFromProtocolIndex(ProtocolIndex);
        addHtml(html);
      }
      html_TD();
      addHtml(getCPluginCFunctionName(static_cast<CPlugin::Function>(x.first % 256)));
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
      addHtml(getMiscStatsName(x.first));
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
