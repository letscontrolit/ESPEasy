#include "../WebServer/Chart_JS.h"

#if FEATURE_CHART_JS

# include "../Helpers/StringConverter.h"
# include "../WebServer/HTML_wrappers.h"


void add_ChartJS_array(int          valueCount,
                       const String array[])
{
  for (int i = 0; i < valueCount; ++i) {
    if (i != 0) {
      addHtml(',');
    }
    addHtml(wrapIfContains(array[i], ' ', '"'));
  }
}

void add_ChartJS_array(int          valueCount,
                       const float  array[],
                       unsigned int nrDecimals)
{
  for (int i = 0; i < valueCount; ++i) {
    if (i != 0) {
      addHtml(',');
    }
    addHtmlFloat(array[i], nrDecimals);
  }
}

void add_ChartJS_array(int       valueCount,
                       const int array[])
{
  for (int i = 0; i < valueCount; ++i) {
    if (i != 0) {
      addHtml(',');
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
  size_t                     nrSamples)
{
  add_ChartJS_chart_header(chartType, String(id), chartTitle, width, height, options, nrSamples);
}

void add_ChartJS_chart_header(
  const __FlashStringHelper *chartType,
  const String             & id,
  const ChartJS_title      & chartTitle,
  int                        width,
  int                        height,
  const String             & options,
  size_t                     nrSamples)
{
  addHtml(F("<canvas"));
  addHtmlAttribute(F("id"),     id);
  addHtmlAttribute(F("width"),  width);
  addHtmlAttribute(F("height"), height);
  addHtml(F("></canvas>"));
  addHtml(F("<script>const "));
  addHtml(id);
  addHtml(F("c=document.getElementById('"));
  addHtml(id);
  addHtml(F("');const my_"));
  addHtml(id);
  addHtml(F("_C=new Chart("));
  addHtml(id);
  addHtml(F("c,{type:'"));
  addHtml(chartType);
  addHtml('\'', ',');
  addHtml(F("options:{responsive:false,plugins:{legend:{position:'top',},title:"));
  addHtml(chartTitle.toString());
  addHtml('}',  ','); // end plugins

  if (nrSamples > 64) {
    // Default point radius = 3
    // Typically when having > 64 samples, these points become really cluttered
    // Thus it is best to remove them by setting the radius to 0.
    addHtml(F("elements:{point:{radius:0}},"));
  }

  if (!options.isEmpty()) {
    addHtml(options);
  }

  addHtml(F("},")); // end options
  addHtml(F("data:{labels:["));
}

void add_ChartJS_chart_labels(
  int       valueCount,
  const int labels[]) {
  add_ChartJS_array(valueCount, labels);
  addHtml(F("],datasets:["));
}

void add_ChartJS_chart_labels(
  int          valueCount,
  const String labels[])
{
  add_ChartJS_array(valueCount, labels);
  addHtml(F("],datasets:["));
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
    addHtml(strformat(F("label:'%s',"), config.label.c_str()));
  }

  if (!config.color.isEmpty()) {
    addHtml(strformat(F("backgroundColor:'%s',"), config.color.c_str()));
    addHtml(strformat(F("borderColor:'%s',"), config.color.c_str()));
  }

  if (!config.axisID.isEmpty()) {
    addHtml(strformat(F("yAxisID:'%s',"), config.axisID.c_str()));
  }

  if (config.hidden || config.displayConfig.showHidden()) {
    addHtml(F("hidden:true,"));
  }

  addHtml(F("data:["));
}

void add_ChartJS_dataset_footer(const String& options) {
  addHtml(']', ',');

  if (!options.isEmpty()) {
    addHtml(options);

    //    if (!options.endsWith(F(","))) { addHtml(','); }
  }

  addHtml('}', ',');
}

void add_ChartJS_chart_footer() {
  addHtml(F("]}});</script>"));
}

#endif // if FEATURE_CHART_JS
