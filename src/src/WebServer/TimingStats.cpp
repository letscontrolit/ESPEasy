#include "../WebServer/TimingStats.h"

#if defined(WEBSERVER_TIMINGSTATS) && FEATURE_TIMING_STATS

#include "../WebServer/ESPEasy_WebServer.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Forms.h"

#include "../DataTypes/ESPEasy_plugin_functions.h"

#include "../Globals/ESPEasy_time.h"
#include "../Globals/RamTracker.h"

#include "../Globals/Device.h"

#include "../Helpers/_Plugin_init.h"


#define TIMING_STATS_THRESHOLD 100000

void handle_timingstats() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_timingstats"));
  #endif
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);
  html_table_class_multirow();
  html_TR();
  {
    const __FlashStringHelper * headers[] = {
      F("Description"),
      F("Function"),
      F("#calls"),
      F("call/sec"),
      F("duty (%)"),
      F("min (ms)"),
      F("Avg (ms)"),
      F("max (ms)")};
    for (unsigned int i = 0; i < NR_ELEMENTS(headers); ++i) {
      html_table_header(headers[i]);
    }
  }


  const long timeSinceLastReset = stream_timing_statistics(true);
  html_end_table();

  html_table_class_normal();
  const float timespan = timeSinceLastReset / 1000.0f;
  addFormHeader(F("Statistics"));
  addRowLabel(F("Start Period"));
  struct tm startPeriod = node_time.addSeconds(node_time.local_tm, -1.0f * timespan, true, true);
  addHtml(formatDateTimeString(startPeriod, '-', ':', ' ', false));
  addRowLabelValue(LabelType::LOCAL_TIME);
  addRowLabel(F("Time span"));
  addHtmlFloat(timespan);
  addHtml(F(" sec"));
  addRowLabel(F("*"));
  addHtml(F("Duty cycle based on average < 1 msec is highly unreliable"));
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
    html_B(toString(value_msec, 3));
  } else {
    addHtmlFloat(value_msec, 3);
  }
}

void stream_html_timing_stats(const TimingStats& stats, long timeSinceLastReset) {
  uint64_t minVal, maxVal;
  const uint64_t c = stats.getMinMax(minVal, maxVal);

  html_TD();
  addHtmlInt(c);
  html_TD();
  const float call_per_sec = static_cast<float>(c) / static_cast<float>(timeSinceLastReset) * 1000.0f;
  const float avg = stats.getAvg();
  addHtmlFloat(call_per_sec, 2);
  html_TD();
  {
    const float duty = (call_per_sec * avg / 10000.0f);
    String duty_str = toString(duty, 2);
    if (avg < 1000) {
      // Unreliable as average is below 1 msec
      duty_str += '*';
      html_I(duty_str);
    } else if (duty > 10.0f) {
      // Over 10% of the time
      html_B(duty_str);
    } else {
      addHtml(duty_str);
    }
  }

  html_TD();
  format_using_threshhold(minVal);
  html_TD();
  format_using_threshhold(avg);
  html_TD();
  format_using_threshhold(maxVal);
}

long stream_timing_statistics(bool clearStats) {
  const long timeSinceLastReset = timePassedSince(timingstats_last_reset);

  for (auto& x: pluginStats) {
    if (!x.second.isEmpty()) {
      const deviceIndex_t deviceIndex = deviceIndex_t::toDeviceIndex(x.first >> 8);

      if (validDeviceIndex(deviceIndex)) {
        if (x.second.thresholdExceeded(TIMING_STATS_THRESHOLD)) {
          html_TR_TD_highlight();
        } else {
          html_TR_TD();
        }
        {
          const pluginID_t pluginID = getPluginID_from_DeviceIndex(deviceIndex);
          addHtml(get_formatted_Plugin_number(pluginID));
          addHtml(' ');
          addHtml(getPluginNameFromDeviceIndex(deviceIndex));
        }
        html_TD();
        addHtml(getPluginFunctionName(x.first % 256));
        stream_html_timing_stats(x.second, timeSinceLastReset);
      }
    }
  }

  for (auto& x: controllerStats) {
    if (!x.second.isEmpty()) {
      const int ProtocolIndex = x.first >> 8;

      if (x.second.thresholdExceeded(TIMING_STATS_THRESHOLD)) {
        html_TR_TD_highlight();
      } else {
        html_TR_TD();
      }
      {
        addHtml(get_formatted_Controller_number(getCPluginID_from_ProtocolIndex(ProtocolIndex)));
        addHtml(' ');
        addHtml(getCPluginNameFromProtocolIndex(ProtocolIndex));
      }
      html_TD();
      addHtml(getCPluginCFunctionName(static_cast<CPlugin::Function>(x.first % 256)));
      stream_html_timing_stats(x.second, timeSinceLastReset);
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
    }
  }

  if (clearStats) {
    pluginStats.clear();
    controllerStats.clear();
    miscStats.clear();
    timingstats_last_reset = millis();
  }
  return timeSinceLastReset;
}

#endif // WEBSERVER_TIMINGSTATS
