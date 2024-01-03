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
// - add_ChartJS_chart_footer
//
// Split into several parts so a long array of
// values can also be served directly
// to reduce memory usage.
// *********************************************

#if FEATURE_CHART_JS

# include "../WebServer/Chart_JS_scale.h"
# include "../DataStructs/ChartJS_dataset_config.h"

void add_ChartJS_chart_header(
  const __FlashStringHelper *chartType,
  const __FlashStringHelper *id,
  const ChartJS_title      & chartTitle,
  int                        width,
  int                        height,
  const String             & options   = EMPTY_STRING,
  size_t                     nrSamples = 0,
  bool                       onlyJSON  = false);

void add_ChartJS_chart_header(
  const __FlashStringHelper *chartType,
  const String             & id,
  const ChartJS_title      & chartTitle,
  int                        width,
  int                        height,
  const String             & options   = EMPTY_STRING,
  size_t                     nrSamples = 0,
  bool                       onlyJSON  = false);

void add_ChartJS_chart_JSON_header(
  const __FlashStringHelper *chartType,
  const ChartJS_title      & chartTitle,
  const String             & options,
  size_t                     nrSamples);

void add_ChartJS_chart_labels(
  int       valueCount,
  const int labels[]);

void add_ChartJS_chart_labels(
  int          valueCount,
  const String labels[]);


void add_ChartJS_scatter_data_point(float x,
                                    float y,
                                    int   nrDecimals);

void add_ChartJS_dataset(
  const ChartJS_dataset_config& config,
  const float                   values[],
  int                           valueCount,
  unsigned int                  nrDecimals = 3,
  const String                & options    = EMPTY_STRING);

void add_ChartJS_dataset_header(const ChartJS_dataset_config& config);

void add_ChartJS_dataset_footer(const String& options = EMPTY_STRING);


void add_ChartJS_chart_footer(bool onlyJSON = false);
#endif // if FEATURE_CHART_JS

#endif // ifndef WEBSERVER_CHART_JS_H
