#ifndef DATASTRUCTS_CHARTJS_DATASET_CONFIG_H
#define DATASTRUCTS_CHARTJS_DATASET_CONFIG_H

#include "../../ESPEasy_common.h"

#if FEATURE_CHART_JS

# include "../DataStructs/PluginStats_Config.h"

struct ChartJS_dataset_config {
  ChartJS_dataset_config() = default;

  ChartJS_dataset_config(
    const __FlashStringHelper *set_label,
    const __FlashStringHelper *set_color);

  ChartJS_dataset_config(
    const String& set_label,
    const String& set_color);

  String               axisID;
  String               label;
  String               color;
  PluginStats_Config_t displayConfig;
  bool                 hidden = false;
};
#endif // if FEATURE_CHART_JS

#endif // ifndef DATASTRUCTS_CHARTJS_DATASET_CONFIG_H
