#ifndef PLUGINSTRUCTS_P096_DATA_STRUCT_H
#define PLUGINSTRUCTS_P096_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P096

# include <LOLIN_EPD.h>
# include <Adafruit_GFX.h>


struct P096_data_struct : public PluginTaskData_base {
public:

  P096_data_struct(int    width,
                   int    height,
                   int8_t DC,
                   int8_t RST,
                   int8_t CS,
                   int8_t BUSY = -1);


  // Print some text
  // param [in] string : The text to display
  // param [in] X : The left position (X)
  // param [in] Y : The top position (Y)
  // param [in] textSize : The text size (default 1)
  // param [in] color : The fore color (default ILI9341_WHITE)
  // param [in] bkcolor : The background color (default ILI9341_BLACK)
  void printText(const char    *string,
                 int            X,
                 int            Y,
                 unsigned int   textSize = 1,
                 unsigned short color    = EPD_WHITE,
                 unsigned short bkcolor  = EPD_BLACK);

  // Parse color string to color
  // param [in] colorString : The color string (white, red, ...)
  // return : color (default EPD_WHITE)
  unsigned short ParseColor(const String& colorString);


  // Fix text with handling special characters (degrees and main monetary symbols)
  // This is specific case for current AdafruitGfx standard fontused for eink screen
  // param [in/out] s : The string to fix
  void FixText(String& s);

  // Split a string by delimiter
  // param [in] s : The input string
  // param [in] c : The delimiter
  // param [out] op : The resulting string array
  // param [in] limit : The maximum strings to find
  // return : The string count
  int StringSplit(const String& s,
                  char          c,
                  String        op[],
                  int           limit);

  LOLIN_IL3897 eInkScreen;
  uint8_t      plugin_096_sequence_in_progress = false;
};
#endif // ifdef USES_P096
#endif // ifndef PLUGINSTRUCTS_P096_DATA_STRUCT_H
