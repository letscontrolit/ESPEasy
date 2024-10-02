#include "../WebServer/Chart_JS.h"

#if FEATURE_CHART_JS

# include "../Helpers/StringConverter.h"
# include "../WebServer/HTML_wrappers.h"


void add_ChartJS_array(int          valueCount,
                       const String array[])
{
  for (int i = 0; i < valueCount; ++i) {
    if (i != 0) {
      addHtml(',', '\n');
    }
    addHtml(to_json_value(array[i]));
  }
}

void add_ChartJS_array(int          valueCount,
                       const float  array[],
                       unsigned int nrDecimals)
{
  for (int i = 0; i < valueCount; ++i) {
    if (i != 0) {
      addHtml(',', '\n');
    }
    addHtmlFloat(array[i], nrDecimals);
  }
}

void add_ChartJS_array(int       valueCount,
                       const int array[])
{
  for (int i = 0; i < valueCount; ++i) {
    if (i != 0) {
      addHtml(',', '\n');
    }
    addHtmlInt(array[i]);
  }
}

void add_ChartJS_chart_header(
  const __FlashStringHelper *chartType,
  const __FlashStringHelper *id,
  const ChartJS_title      & chartTitle,
  int                        width,
  int                        height,
  const String             & options,
  bool                       enableZoom,
  size_t                     nrSamples,
  bool                       onlyJSON)
{
  add_ChartJS_chart_header(
    chartType,
    String(id),
    chartTitle,
    width,
    height,
    options,
    enableZoom,
    nrSamples,
    onlyJSON);
}

void add_ChartJS_chart_header(
  const __FlashStringHelper *chartType,
  const String             & id,
  const ChartJS_title      & chartTitle,
  int                        width,
  int                        height,
  const String             & options,
  bool                       enableZoom,
  size_t                     nrSamples,
  bool                       onlyJSON)
{
  if (!onlyJSON) {
    addHtml(F("<canvas"));
    addHtmlAttribute(F("id"),     id);
    addHtmlAttribute(F("width"),  width);
    addHtmlAttribute(F("height"), height);
    addHtml(F("></canvas>"));
    const char *id_c_str = id.c_str();
    addHtml(strformat(
              F("<script>"
                "const %sc=document.getElementById('%s');"
                "const my_%s_C=new Chart(%sc,"),
              id_c_str,
              id_c_str,
              id_c_str,
              id_c_str));
  }
  String plugins;

  if (enableZoom) {
    plugins = F(
      "\"zoom\":"
      "{"
      "\"limits\":{"
      "\"x\":{\"min\":\"original\",\"max\":\"original\",\"minRange\":1000}," // 1 sec min range
      "},"
      "\"pan\":{"
      "\"enabled\":true,"
      "\"mode\":\"x\","
      "\"modifierKey\":\"ctrl\","
      "},"
      "\"zoom\":{"
      "\"wheel\":{"
      "\"enabled\":true,"
      "},"
      "\"drag\":{"
      "\"enabled\":true,"
      "},"
      "\"pinch\":{"
      "\"enabled\":true"
      "},"
      "\"mode\":\"x\","
      "}"
      "}"
      );
  }
  add_ChartJS_chart_JSON_header(
    chartType,
    plugins,
    chartTitle,
    options,
    nrSamples);
}

void add_ChartJS_chart_JSON_header(
  const __FlashStringHelper *chartType,
  const String             & plugins,
  const ChartJS_title      & chartTitle,
  const String             & options,
  size_t                     nrSamples)
{
  addHtml(F("{\"type\":\""));
  addHtml(chartType);
  addHtml(F("\",\"options\":{"
            "\"responsive\":false,\"plugins\":{"));
  addHtml(F("\"legend\":{"
            "\"position\":\"top\""
            "},\"title\":"));
  addHtml(chartTitle.toString());

  if (plugins.length() > 0) {
    addHtml(',');
  }
  addHtml(plugins);
  addHtml('}'); // end plugins

  if (nrSamples >= 60) {
    // Default point radius = 3
    // Typically when having > 64 samples, these points become really cluttered
    // Thus it is best to reduce their radius.
    const float radius = (plugins.length() > 0) ? 2.5f : 2.0f;
    addHtml(strformat(
      F(",\"elements\":{\"point\":{\"radius\":%.1f}}"),
      radius));
  }

  if (!options.isEmpty()) {
    addHtml(',', '\n');
    addHtml(options);
  }

  addHtml(F("}," // end options
            "\n\"data\":{"));
}

void add_ChartJS_chart_labels(
  int       valueCount,
  const int labels[])
{
  addHtml(F("\n\"labels\":["));
  add_ChartJS_array(valueCount, labels);
  addHtml(F("],\n\"datasets\":["));
}

void add_ChartJS_chart_labels(
  int          valueCount,
  const String labels[])
{
  addHtml(F("\n\"labels\":["));
  add_ChartJS_array(valueCount, labels);
  addHtml(F("],\n\"datasets\":["));
}

void add_ChartJS_scatter_data_point(float x, float y, int nrDecimals)
{
  addHtml(strformat(
            F("{\"x\":%s,\"y\":%s},"),
            toString(x, nrDecimals).c_str(),
            toString(y, nrDecimals).c_str()));
}

void add_ChartJS_dataset(
  const ChartJS_dataset_config& config,
  const float                   values[],
  int                           valueCount,
  unsigned int                  nrDecimals,
  const String                & options)
{
  add_ChartJS_dataset_header(config);
  add_ChartJS_array(valueCount, values, nrDecimals);
  add_ChartJS_dataset_footer(options);
}

void add_ChartJS_dataset_header(const ChartJS_dataset_config& config)
{
  addHtml('{');

  if (!config.label.isEmpty()) {
    addHtml(strformat(F("\"label\":\"%s\","), config.label.c_str()));
  }

  if (!config.color.isEmpty()) {
    addHtml(strformat(F("\"backgroundColor\":\"%s\","), config.color.c_str()));
    addHtml(strformat(F("\"borderColor\":\"%s\","), config.color.c_str()));
  }

  if (!config.axisID.isEmpty()) {
    addHtml(strformat(F("\"yAxisID\":\"%s\","), config.axisID.c_str()));
  }

  if (config.hidden || config.displayConfig.showHidden()) {
    addHtml(F("\"hidden\":true,"));
  }

  addHtml(F("\n\"data\":[\n"));
}

void add_ChartJS_dataset_footer(const String& options) {
  addHtml(']');

  if (!options.isEmpty()) {
    addHtml(',', '\n');
    addHtml(options);
  }

  addHtml('}', '\n');
}

void add_ChartJS_chart_footer(bool onlyJSON) {
  addHtml(F("]}}"));

  if (!onlyJSON) {
    addHtml(F(");</script>"));
  }
}

#endif // if FEATURE_CHART_JS
