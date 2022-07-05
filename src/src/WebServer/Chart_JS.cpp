#include "../WebServer/Chart_JS.h"


#include "../Helpers/StringConverter.h"
#include "../WebServer/HTML_wrappers.h"


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

void add_ChartJS_array(int         valueCount,
                       const float array[])
{
  for (int i = 0; i < valueCount; ++i) {
    if (i != 0) {
      addHtml(',');
    }
    addHtmlFloat(array[i], 3);
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
  const __FlashStringHelper *chartTitle,
  int                        width,
  int                        height)
{
  add_ChartJS_chart_header(chartType, String(id), String(chartTitle), width, height);
}

void add_ChartJS_chart_header(
  const __FlashStringHelper *chartType,
  const String             & id,
  const String             & chartTitle,
  int                        width,
  int                        height)
{
  addHtml(F("<canvas"));
  addHtmlAttribute(F("id"),     id);
  addHtmlAttribute(F("width"),  width);
  addHtmlAttribute(F("height"), height);
  addHtml(F("></canvas>"));
  addHtml(F("<script>const "));
  addHtml(id);
  addHtml(F("_ctx = document.getElementById('"));
  addHtml(id);
  addHtml(F("');const my_"));
  addHtml(id);
  addHtml(F("_Chart = new Chart("));
  addHtml(id);
  addHtml(F("_ctx, {type: '"));
  addHtml(chartType);
  addHtml('\'', ',');
  addHtml(F("options: {responsive: false,plugins: {legend: {position: 'top',},title: {display: true,text: '"));
  addHtml(chartTitle);
  addHtml(F("'}}},"));
  addHtml(F("data: {labels: ["));
}

void add_ChartJS_chart_labels(
  int       valueCount,
  const int labels[]) {
  add_ChartJS_array(valueCount, labels);
  addHtml(F("],datasets: ["));
}

void add_ChartJS_chart_labels(
  int          valueCount,
  const String labels[])
{
  add_ChartJS_array(valueCount, labels);
  addHtml(F("],datasets: ["));
}

void add_ChartJS_dataset(
  const __FlashStringHelper *label,
  const __FlashStringHelper *color,
  const float                values[],
  int                        valueCount,
  bool                       hidden,
  const String& options)
{
  add_ChartJS_dataset_header(label, color);
  add_ChartJS_array(valueCount, values);
  add_ChartJS_dataset_footer(hidden, options);
}

void add_ChartJS_dataset_header(
  const __FlashStringHelper *label,
  const __FlashStringHelper *color) {
  addHtml('{');
  addHtml(F("label: '"));
  addHtml(label);
  addHtml('\'', ',');
  addHtml(F("backgroundColor: '"));
  addHtml(color);
  addHtml('\'', ',');
  addHtml(F("borderColor: '"));
  addHtml(color);
  addHtml('\'', ',');
  addHtml(F("data: ["));
}

void add_ChartJS_dataset_footer(bool hidden, const String& options) {
  addHtml(']', ',');

  if (hidden) {
    addHtml(F("hidden: true,"));
  }
  if (!options.isEmpty()) {
    addHtml(options);
//    if (!options.endsWith(F(","))) { addHtml(','); }
  }

  addHtml('}', ',');
}

void add_ChartJS_chart_footer() {
  addHtml(F("]}});</script>"));
}
