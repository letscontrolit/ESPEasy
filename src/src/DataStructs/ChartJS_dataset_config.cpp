#include "../DataStructs/ChartJS_dataset_config.h"

#if FEATURE_CHART_JS

ChartJS_dataset_config::ChartJS_dataset_config(
  const __FlashStringHelper * set_label,
  const __FlashStringHelper * set_color)
  : label(set_label), color(set_color) {}

ChartJS_dataset_config::ChartJS_dataset_config(
  const String& set_label,
  const String& set_color)
  : label(set_label), color(set_color) {}

#endif