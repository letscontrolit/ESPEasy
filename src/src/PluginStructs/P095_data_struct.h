#ifndef PLUGINSTRUCTS_P095_DATA_STRUCT_H
#define PLUGINSTRUCTS_P095_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P095


#if !defined(LIMIT_BUILD_SIZE) && !defined(PLUGIN_095_FONT_INCLUDED)
  #define PLUGIN_095_FONT_INCLUDED   // enable to use fonts in this plugin
#endif


#include <Adafruit_ILI9341.h>

#ifdef PLUGIN_095_FONT_INCLUDED
      #include "../../static/Fonts/Seven_Segment24pt7b.h"
      #include "../../static/Fonts/Seven_Segment18pt7b.h"
      #include <Fonts/FreeSans9pt7b.h> // included in Adafruit-GFX-Library
#endif   

struct P095_data_struct : public PluginTaskData_base {
public:

  P095_data_struct(int8_t _CS, int8_t _DC, int8_t _RST = -1);


//Print some text
//param [in] string : The text to display
//param [in] X : The left position (X)
//param [in] Y : The top position (Y)
//param [in] textSize : The text size (default 1)
//param [in] color : The fore color (default ILI9341_WHITE)
//param [in] bkcolor : The background color (default ILI9341_BLACK)

  void Plugin_095_printText(const String& string, int X, int Y, unsigned int textSize = 1, unsigned short color = ILI9341_WHITE, unsigned short bkcolor = ILI9341_BLACK);

unsigned short Plugin_095_ParseColor(String & s);
  int Plugin_095_StringSplit(String &s, char c, String op[], int limit);

  Adafruit_ILI9341 tft;

};
#endif // ifdef USES_P095
#endif // ifndef PLUGINSTRUCTS_P095_DATA_STRUCT_H
