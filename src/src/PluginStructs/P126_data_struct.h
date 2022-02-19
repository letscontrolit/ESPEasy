#ifndef PLUGINSTRUCTS_P126_DATA_STRUCT_H
#define PLUGINSTRUCTS_P126_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P126


# if !defined(LIMIT_BUILD_SIZE) && !defined(PLUGIN_126_FONT_INCLUDED)
  #  define PLUGIN_126_FONT_INCLUDED // enable to use fonts in this plugin
# endif // if !defined(LIMIT_BUILD_SIZE) && !defined(PLUGIN_126_FONT_INCLUDED)


# include <TFT_eSPI.h>

// TODOPETER
# ifdef PLUGIN_126_FONT_INCLUDED
//#  include "../Static/Fonts/Seven_Segment24pt7b.h"
//#  include "../Static/Fonts/Seven_Segment18pt7b.h"
//#  include <Fonts/FreeSans9pt7b.h> // included in Adafruit-GFX-Library
# endif // ifdef PLUGIN_126_FONT_INCLUDED

struct P126_data_struct : public PluginTaskData_base {
public:

  P126_data_struct(int8_t _CS,
                   int8_t _DC,
                   int8_t _RST = -1,
                   int16_t _W = TFT_WIDTH,
                   int16_t _H = TFT_HEIGHT);


  // Print some text
  // param [in] string : The text to display
  // param [in] X : The left position (X)
  // param [in] Y : The top position (Y)
  // param [in] textSize : The text size (default 1)
  // param [in] color : The fore color (default TFT_WHITE)
  // param [in] bkcolor : The background color (default TFT_BLACK)

  void printText(const String & string,
                 int            X,
                 int            Y,
                 unsigned int   textSize = 1,
                 unsigned short color    = TFT_WHITE,
                 unsigned short bkcolor  = TFT_BLACK);

  // Parse color string to TFT_eSPI color
  // param [in] s : The color string (white, red, ...)
  // return : color (default TFT_WHITE)
  unsigned short ParseColor(String& s);

  // Split a string by delimiter
  // param [in] s : The input string
  // param [in] c : The delimiter
  // param [out] op : The resulting string array
  // param [in] limit : The maximum strings to find
  // return : The string count

  int StringSplit(String& s,
                  char    c,
                  String  op[],
                  int     limit);

  TFT_eSPI tft;
};
#endif // ifdef USES_P126
#endif // ifndef PLUGINSTRUCTS_P126_DATA_STRUCT_H
