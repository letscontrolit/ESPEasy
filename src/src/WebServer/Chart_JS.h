#ifndef WEBSERVER_CHART_JS_H
#define WEBSERVER_CHART_JS_H

#include "../WebServer/common.h"

// *********************************************
// Support for ChartJS charts
//
// Typical way of adding a chart:
// - add_ChartJS_chart_header
// - add_ChartJS_chart_labels
// - add_ChartJS_dataset (1x or more)
//
// Typical JSON layout:
// {
//   "type": "line",
//   "options": {
//       ...
//   },
//   "data": {
//     "labels": [
//       ...
//     ],
//     "datasets": [
//       {
//         "label": "Error %",
//         "backgroundColor": "rgb(255, 99, 132)",
//         "borderColor": "rgb(255, 99, 132)",
//         "data": [
//           ...
//         ]
//       }
//     ]
//   }
// }
//
// Make sure the KeyValueWriter objects are in their own scope,
// since the closing braces are written from their destructor.
//
// Split into several parts so a long array of
// values can also be served directly
// to reduce memory usage.
// *********************************************

#if FEATURE_CHART_JS

# include "../Helpers/KeyValueWriter.h"
# include "../Helpers/KeyValueWriter_JSON.h"
# include "../WebServer/Chart_JS_scale.h"
# include "../DataStructs/ChartJS_dataset_config.h"

UP_KeyValueWriter add_ChartJS_chart_header(
  const __FlashStringHelper *chartType,
  const __FlashStringHelper *id,
  const ChartJS_title      & chartTitle,
  ChartJS_options_scales   & options,
  bool                       enableZoom = false,
  size_t                     nrSamples  = 0,
  bool                       onlyJSON   = false);

UP_KeyValueWriter add_ChartJS_chart_header(
  const __FlashStringHelper *chartType,
  const String             & id,
  const ChartJS_title      & chartTitle,
  ChartJS_options_scales   & options,
  bool                       enableZoom = false,
  size_t                     nrSamples  = 0,
  bool                       onlyJSON   = false);

void add_ChartJS_chart_JSON_header(
  KeyValueWriter_JSON      & parent,
  const __FlashStringHelper *chartType,
  const ChartJS_title      & chartTitle,
  ChartJS_options_scales   & options,
  size_t                     nrSamples,
  bool                       enableZoom
  );

void add_ChartJS_chart_labels(
  KeyValueWriter& parent,
  int             valueCount,
  const int       labels[]);

void add_ChartJS_chart_labels(
  KeyValueWriter& parent,
  int             valueCount,
  const String    labels[]);


void add_ChartJS_scatter_data_point(
  KeyValueWriter& parent,
  float           x,
  float           y,
  uint8_t         nrDecimalsX,
  uint8_t         nrDecimalsY);

void add_ChartJS_dataset(
  KeyValueWriter              & datasets,
  const ChartJS_dataset_config& config,
  const float                   values[],
  int                           valueCount,
  uint8_t                       nrDecimals = 3,
  const String                & options    = EMPTY_STRING);


UP_KeyValueWriter add_ChartJS_dataset_header(
  KeyValueWriter              & dataset,
  const ChartJS_dataset_config& config);


#endif // if FEATURE_CHART_JS

#endif // ifndef WEBSERVER_CHART_JS_H
