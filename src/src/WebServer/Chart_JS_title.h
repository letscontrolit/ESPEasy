#ifndef WEBSERVER_CHART_JS_TITLE_H
#define WEBSERVER_CHART_JS_TITLE_H

#include "../WebServer/common.h"

#if FEATURE_CHART_JS

struct ChartJS_title {
  enum class Align {
    Start,
    Center,
    End
  };

  ChartJS_title();
  ChartJS_title(const __FlashStringHelper *titleText,
                Align                      alignment = Align::Center);
  ChartJS_title(const String& titleText,
                Align         alignment = Align::Center);

  String text;
  String color;
  Align  align;

  String toString() const;
};


#endif // if FEATURE_CHART_JS

#endif // ifndef WEBSERVER_CHART_JS_TITLE_H
