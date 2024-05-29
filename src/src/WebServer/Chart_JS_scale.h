#ifndef WEBSERVER_CHART_JS_SCALE_H
#define WEBSERVER_CHART_JS_SCALE_H


#include "../WebServer/common.h"

#if FEATURE_CHART_JS

# include "../DataStructs/PluginStats_Config.h"
# include "../WebServer/Chart_JS_title.h"

# include <vector>

// Options for defining chart axis
struct ChartJS_options_scale {
  ChartJS_options_scale(const String& id,
                        const String& title = EMPTY_STRING);

  // Configure Y-axis scale based on PluginStats_Config_t
  ChartJS_options_scale(const PluginStats_Config_t& config,
                        const String              & title = EMPTY_STRING);

  // ID used along with the data set to indicate which axis should be used
  String axisID;

  // Type of scale, like 'linear', 'logarithmic', 'timeseries', 'category'  (etc.)
  String        scaleType;
  ChartJS_title axisTitle;

  enum class Position {
    Top,
    Bottom,
    Left,
    Right,
    Center
  };
  enum class Display {
    True,
    False,
    Auto // the axis is visible only if at least one associated dataset is visible.
  };
  Position position = Position::Left;
  Display  display  = Display::Auto;

  int tickCount{};
  int weight{};

  String toString() const;

  bool   is_Y_axis() const;
};

struct ChartJS_options_scales {
  ChartJS_options_scales();

  void   add(const ChartJS_options_scale& scale);

  void   update_Yaxis_TickCount();

  String toString() const;

  size_t nr_Y_scales() const;

private:

  std::vector<ChartJS_options_scale>_scales;
};


#endif // if FEATURE_CHART_JS


#endif // ifndef WEBSERVER_CHART_JS_SCALE_H
