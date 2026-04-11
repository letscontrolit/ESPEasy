#include "../WebServer/Chart_JS_title.h"


#if FEATURE_CHART_JS

# include "../Helpers/StringConverter.h"

ChartJS_title::ChartJS_title()
  : align(Align::Center) {}

ChartJS_title::ChartJS_title(const __FlashStringHelper *titleText, Align alignment)
  : text(titleText), align(alignment)
{}

ChartJS_title::ChartJS_title(const String& titleText, Align alignment)
  : text(titleText), align(alignment)
{}

String ChartJS_title::toString() const {
  if (text.isEmpty()) {
    return F("{\"display\": false}");
  }

  const String alignStr =
    align == Align::Start ? F("start") :
    align == Align::End ? F("end") : F("center");

  String colorStr;

  if (!color.isEmpty()) {
    colorStr = strformat(F(",\"color\":\"%s\""), color.c_str());
  }

  return strformat(
    F("{\"display\": true,\"align\":\"%s\",\"text\":\"%s\"%s}"),
    alignStr.c_str(),
    text.c_str(),
    colorStr.c_str());
}

#endif // if FEATURE_CHART_JS
