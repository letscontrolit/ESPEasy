#ifndef DATASTRUCTS_CHARTJS_DATASET_CONFIG_H
#define DATASTRUCTS_CHARTJS_DATASET_CONFIG_H

#include "../../ESPEasy_common.h"

#if FEATURE_CHART_JS
struct ChartJS_dataset_config {
  String label;
  String color;
  bool   hidden = false;
};
#endif // if FEATURE_CHART_JS

#endif // ifndef DATASTRUCTS_CHARTJS_DATASET_CONFIG_H
