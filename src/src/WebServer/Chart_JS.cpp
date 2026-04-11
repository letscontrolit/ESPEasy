#include "../WebServer/Chart_JS.h"

#if FEATURE_CHART_JS

# include "../Helpers/KeyValueWriter_JSON.h"
# include "../Helpers/StringConverter.h"
# include "../WebServer/HTML_wrappers.h"
# include "../WebServer/JSON.h"

void add_ChartJS_array(KeyValueWriter& parent,
                       int             valueCount,
                       const String    array[])
{
  for (int i = 0; i < valueCount; ++i) {
    parent.write({ EMPTY_STRING, array[i] });
  }
}

void add_ChartJS_array(KeyValueWriter& parent,
                       int             valueCount,
                       const float     array[],
                       uint8_t         nrDecimals)
{
  for (int i = 0; i < valueCount; ++i) {
    parent.write({ EMPTY_STRING, array[i], nrDecimals });
  }
}

void add_ChartJS_array(KeyValueWriter& parent,
                       int             valueCount,
                       const int       array[])
{
  for (int i = 0; i < valueCount; ++i) {
    parent.write({ EMPTY_STRING, array[i] });
  }
}

UP_KeyValueWriter add_ChartJS_chart_header(
  const __FlashStringHelper *chartType,
  const __FlashStringHelper *id,
  const ChartJS_title      & chartTitle,
  ChartJS_options_scales   & options,
  bool                       enableZoom,
  size_t                     nrSamples,
  bool                       onlyJSON)
{
  return add_ChartJS_chart_header(
    chartType,
    String(id),
    chartTitle,
    options,
    enableZoom,
    nrSamples,
    onlyJSON);
}

UP_KeyValueWriter add_ChartJS_chart_header(
  const __FlashStringHelper *chartType,
  const String             & id,
  const ChartJS_title      & chartTitle,
  ChartJS_options_scales   & options,
  bool                       enableZoom,
  size_t                     nrSamples,
  bool                       onlyJSON)
{
  if (!onlyJSON) {
    addHtml(F("<div class=\"chart-container\" style=\"position: relative; height:40vh; width:95vw\">"));
    addHtml(F("<canvas"));
    addHtmlAttribute(F("id"),     id);
    addHtml(F("></canvas>"));
    addHtml(F("</div>"));
    const char *id_c_str = id.c_str();
    addHtml(strformat(
              F("<script>"
                "const %sc=document.getElementById('%s');"
                "const my_%s_C=new Chart(%sc,\n"),
              id_c_str,
              id_c_str,
              id_c_str,
              id_c_str));
  }
  UP_KeyValueWriter_JSON chartJSON(new (std::nothrow) KeyValueWriter_JSON(true));

  if (chartJSON) {
    chartJSON->allowFormatOverrides(false);

    if (!onlyJSON) {
      chartJSON->setFooter(F("\n);</script>"));
    }

    add_ChartJS_chart_JSON_header(
      *chartJSON,
      chartType,
      chartTitle,
      options,
      nrSamples,
      enableZoom);
  }

  return std::move(chartJSON);
}

void add_ChartJS_chart_JSON_header(
  KeyValueWriter_JSON      & parent,
  const __FlashStringHelper *chartType,
  const ChartJS_title      & chartTitle,
  ChartJS_options_scales   & options,
  size_t                     nrSamples,
  bool                       enableZoom)
{
  parent.write({ F("type"), chartType });
  auto optionsArr = parent.createChild(F("options"));

  if (optionsArr) {
    const bool b_true(true);
    const bool b_false(false);
    optionsArr->write({ F("responsive"), b_true });
    optionsArr->write({ F("maintainAspectRatio"), b_false });
    {
      auto plugins = optionsArr->createChild(F("plugins"));

      if (plugins) {
        {
          auto legend = plugins->createChild(F("legend"));

          if (legend) {
            legend->write({ F("position"), F("top") });
          }
        }
        plugins->write({ F("title"), chartTitle.toString() });

        if (enableZoom) {
          plugins->write({
            F("zoom"),
            F("{\"limits\":{"
              "\"x\":{\"min\":\"original\",\"max\":\"original\",\"minRange\":1000}},"
              "\"pan\":{\"enabled\":true,\"mode\":\"x\",\"modifierKey\":\"ctrl\"},"
              "\"zoom\":{"
              "\"wheel\":{\"enabled\":true},"
              "\"drag\":{\"enabled\":true},"
              "\"pinch\":{\"enabled\":true},"
              "\"mode\":\"x\"}}"
              ) });
        }

      }
    }

    if (nrSamples >= 60) {
      // Default point radius = 3
      // Typically when having > 64 samples, these points become really cluttered
      // Thus it is best to reduce their radius.
      const float radius = (enableZoom) ? 2.5f : 2.0f;
      optionsArr->write({ F("elements"),
                          strformat(
                            F("{\"point\":{\"radius\":%.1f}}"),
                            radius)
                        });
    }
    options.toString(*optionsArr);
  }
}

void add_ChartJS_chart_labels(
  KeyValueWriter& parent,
  int             valueCount,
  const int       labels[])
{
  auto labelsWriter = parent.createChildArray(F("labels"));

  if (labelsWriter) {
    add_ChartJS_array(*labelsWriter, valueCount, labels);
  }
}

void add_ChartJS_chart_labels(
  KeyValueWriter& parent,
  int             valueCount,
  const String    labels[])
{
  auto labelsWriter = parent.createChildArray(F("labels"));

  if (labelsWriter) {
    add_ChartJS_array(*labelsWriter, valueCount, labels);
  }
}

void add_ChartJS_scatter_data_point(
  KeyValueWriter& parent,
  float x, float y, uint8_t nrDecimalsX, uint8_t nrDecimalsY)
{
  auto element = parent.createChild();

  if (element) {
    element->write({ F("x"), x, nrDecimalsX });
    element->write({ F("y"), y, nrDecimalsY });
  }
}

void add_ChartJS_dataset(
  KeyValueWriter              & datasets,
  const ChartJS_dataset_config& config,
  const float                   values[],
  int                           valueCount,
  uint8_t                       nrDecimals,
  const String                & options)
{
  auto dataset = datasets.createChild();

  if (dataset) {
    auto data = add_ChartJS_dataset_header(*dataset, config);

    if (data) {
      add_ChartJS_array(*data, valueCount, values, nrDecimals);
    }
  }
}

UP_KeyValueWriter add_ChartJS_dataset_header(KeyValueWriter& dataset, const ChartJS_dataset_config& config)
{
  if (!config.label.isEmpty()) {
    dataset.write({ F("label"), config.label });
  }

  if (!config.color.isEmpty()) {
    dataset.write({ F("backgroundColor"), config.color });
    dataset.write({ F("borderColor"), config.color });
  }

  if (!config.axisID.isEmpty()) {
    dataset.write({ F("yAxisID"), config.axisID });
  }

  if (config.hidden || config.displayConfig.showHidden()) {
    dataset.write({ F("hidden"), true });
  }

  return dataset.createChildArray(F("data"));
}

#endif // if FEATURE_CHART_JS
