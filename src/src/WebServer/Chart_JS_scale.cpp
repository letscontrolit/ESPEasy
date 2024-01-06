#include "../WebServer/Chart_JS_scale.h"

#if FEATURE_CHART_JS

# include "../Helpers/StringConverter.h"

ChartJS_options_scale::ChartJS_options_scale(const String& id, const String& title)
  : axisID(id), position(Position::Left)
{
  // Set some proper defaults, based on the id string.
  if (axisID.startsWith(F("x"))) {
    position = Position::Bottom;

    // Make sure the X-axis is shown even when no dataset is selected.
    display = Display::True;
  }
  else if (axisID.indexOf(F("right")) != -1) {
    position = Position::Right;
  }
  axisTitle.text = title;
}

ChartJS_options_scale::ChartJS_options_scale(const PluginStats_Config_t& config, const String& title)
{
  const bool isLeft = config.isLeft();

  position = isLeft ? Position::Left : Position::Right;
  weight   = config.getAxisIndex();
  axisID   = strformat((isLeft ? F("y-left-%d") : F("y-right-%d")),
                       weight);
  axisTitle.text = title;
  display        = Display::Auto;
}

String ChartJS_options_scale::toString() const
{
  if (!axisID.isEmpty()) {
    // In JSON, boolean values do not need quotes
    const String displayStr =
      display == Display::Auto ? F("\"auto\"") :
      display == Display::True ? F("true") : F("false");

    String typeStr = scaleType;

    if (typeStr.isEmpty()) { typeStr = F("linear"); }

    String positionStr;

    switch (position) {
      case Position::Top:    positionStr = F("top"); break;
      case Position::Bottom: positionStr = F("bottom"); break;
      case Position::Right:  positionStr = F("right"); break;
      case Position::Center: positionStr = F("center"); break;
      case Position::Left:   positionStr = F("left"); break;
    }

    String ticksStr;

    if (tickCount > 0) {
      ticksStr = strformat(F(",\"ticks\":{\"count\":%d}"), tickCount);
    }
    return strformat(
      F("\"%s\":{\"display\":%s,\"type\":\"%s\",\"position\":\"%s\",\"title\":%s,\"weight\":%d%s}"),
      axisID.c_str(),
      displayStr.c_str(),
      typeStr.c_str(),
      positionStr.c_str(),
      axisTitle.toString().c_str(),
      weight,
      ticksStr.c_str());
  }
  return EMPTY_STRING;
}

bool ChartJS_options_scale::is_Y_axis() const
{
  return position == Position::Left ||
         position == Position::Right;
}

void ChartJS_options_scales::add(const ChartJS_options_scale& scale)
{
  for (auto it = _scales.begin(); it != _scales.end(); ++it) {
    if (it->axisID.equals(scale.axisID)) {
      // Found an axis with same ID.
      // Combine labels and don't create a new one.
      if (!scale.axisTitle.text.isEmpty()) {
        if (!it->axisTitle.text.isEmpty()) {
          it->axisTitle.color.clear();
          it->axisTitle.text += F(" / ");
        }
        it->axisTitle.text += scale.axisTitle.text;
      }
      return;
    }
  }
  _scales.push_back(scale);
}

void ChartJS_options_scales::update_Yaxis_TickCount()
{
  // For single Y-axis, use a dynamic tick count based on the data.
  // For multiple Y-axis, we want 10 intervals, thus 11 ticks.
  const int newTickCount = (nr_Y_scales() <= 1) ? 0 : 11;

  for (auto it = _scales.begin(); it != _scales.end(); ++it) {
    if (it->is_Y_axis()) {
      it->tickCount = newTickCount;
    }
  }
}

String ChartJS_options_scales::toString() const
{
  if (_scales.empty()) { return EMPTY_STRING; }

  String res   = F("\"scales\":{");
  bool   first = true;

  for (auto it = _scales.begin(); it != _scales.end(); ++it) {
    const String scale_str = it->toString();

    if (!scale_str.isEmpty()) {
      if (!first) {
        res += ',';
      }
      first = false;
      res  += '\n';
      res  += scale_str;
    }
  }
  res += '}';
  return res;
}

size_t ChartJS_options_scales::nr_Y_scales() const
{
  size_t count{};

  for (auto it = _scales.begin(); it != _scales.end(); ++it) {
    if (it->is_Y_axis()) { ++count; }
  }

  return count;
}

#endif // if FEATURE_CHART_JS
