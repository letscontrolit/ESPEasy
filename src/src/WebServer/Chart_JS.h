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


void add_ChartJS_chart_header(
  const __FlashStringHelper *chartType,
  const __FlashStringHelper *id,
  const __FlashStringHelper *chartTitle,
  int                        width,
  int                        height);

void add_ChartJS_chart_header(
  const __FlashStringHelper *chartType,
  const String             & id,
  const String             & chartTitle,
  int                        width,
  int                        height);

void add_ChartJS_chart_labels(
  int       valueCount,
  const int labels[]);

void add_ChartJS_chart_labels(
  int          valueCount,
  const String labels[]);


void add_ChartJS_dataset(
  const __FlashStringHelper *label,
  const __FlashStringHelper *color,
  const float                values[],
  int                        valueCount,
  bool                       hidden = false,
  const String& options = EMPTY_STRING);

void add_ChartJS_dataset_header(
  const __FlashStringHelper *label,
  const __FlashStringHelper *color);

void add_ChartJS_dataset_footer(bool hidden = false, const String& options = EMPTY_STRING);


void add_ChartJS_chart_footer();

#endif // ifndef WEBSERVER_CHART_JS_H
