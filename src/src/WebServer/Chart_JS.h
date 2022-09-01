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

struct ChartJS_title {
  ChartJS_title();
  ChartJS_title(const String& titleText);

  String align;
  String text;

  String toString() const;
};

String   make_ChartJS_scale_options(
  const ChartJS_title& xAxisTitle,
  const ChartJS_title& yAxisTitle,
  const String       & xAxisType = EMPTY_STRING,
  const String       & yAxisType = EMPTY_STRING);


void add_ChartJS_chart_header(
  const __FlashStringHelper *chartType,
  const __FlashStringHelper *id,
  const __FlashStringHelper *chartTitle,
  int                        width,
  int                        height,
  const String             & options = EMPTY_STRING);

void add_ChartJS_chart_header(
  const __FlashStringHelper *chartType,
  const String             & id,
  const String             & chartTitle,
  int                        width,
  int                        height,
  const String             & options = EMPTY_STRING);


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
  bool                       hidden  = false,
  const String             & options = EMPTY_STRING);

void add_ChartJS_dataset(
  const String&              label,
  const String&              color,
  const float                values[],
  int                        valueCount,
  bool                       hidden  = false,
  const String             & options = EMPTY_STRING);

void add_ChartJS_dataset_header(
  const __FlashStringHelper *label,
  const __FlashStringHelper *color);

void add_ChartJS_dataset_header(
  const String& label,
  const String& color);

void add_ChartJS_dataset_footer(bool          hidden  = false,
                                const String& options = EMPTY_STRING);


void add_ChartJS_chart_footer();
#endif // if FEATURE_CHART_JS

#endif // ifndef WEBSERVER_CHART_JS_H
