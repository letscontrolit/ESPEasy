#ifndef PLUGINSTRUCTS_P095_DATA_STRUCT_H
#define PLUGINSTRUCTS_P095_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P095


# if !defined(LIMIT_BUILD_SIZE) && !defined(PLUGIN_095_FONT_INCLUDED)
  #  define PLUGIN_095_FONT_INCLUDED // enable to use fonts in this plugin
# endif // if !defined(LIMIT_BUILD_SIZE) && !defined(PLUGIN_095_FONT_INCLUDED)

# include <Adafruit_ILI9341.h>

# ifdef PLUGIN_095_FONT_INCLUDED
#  include "../Static/Fonts/Seven_Segment24pt7b.h"
#  include "../Static/Fonts/Seven_Segment18pt7b.h"
#  include <Fonts/FreeSans9pt7b.h>                      // included in Adafruit-GFX-Library
# endif // ifdef PLUGIN_095_FONT_INCLUDED

# define P095_CONFIG_FLAGS              PCONFIG_LONG(0) // All flags
# define P095_CONFIG_FLAG_TYPE          20              // Flag-offset to store 4 bits for Display type, uses bits 20..24

// // Getters
# define P095_CONFIG_FLAG_GET_TYPE          (get4BitFromUL(P095_CONFIG_FLAGS, P095_CONFIG_FLAG_TYPE))

enum class ILI9xxx_type_e : uint8_t {
  ILI9341_240x320        = 0u,
  ILI9342_240x320        = 1u,
  ILI9481_320x480        = 2u,
  ILI9481_CPT29_320x480  = 3u,
  ILI9481_PVI35_320x480  = 4u,
  ILI9481_AUO317_320x480 = 5u,
  ILI9481_CMO35_320x480  = 6u,
  ILI9481_RGB_320x480    = 7u,
  ILI9486_320x480        = 8u,
  ILI9488_320x480        = 9u,
  ILI9xxx_MAX            = 10u // last value = count
};

const __FlashStringHelper* ILI9xxx_type_toString(ILI9xxx_type_e device);

void                       ILI9xxx_type_toResolution(ILI9xxx_type_e device,
                                                     uint16_t     & x,
                                                     uint16_t     & y);

struct P095_data_struct : public PluginTaskData_base {
public:

  P095_data_struct(ILI9xxx_type_e displayType,
                   int8_t         _CS,
                   int8_t         _DC,
                   int8_t         _RST = -1);
  ~P095_data_struct();

  // Print some text
  // param [in] string : The text to display
  // param [in] X : The left position (X)
  // param [in] Y : The top position (Y)
  // param [in] textSize : The text size (default 1)
  // param [in] color : The fore color (default ILI9341_WHITE)
  // param [in] bkcolor : The background color (default ILI9341_BLACK)

  void printText(const String & string,
                 int            X,
                 int            Y,
                 unsigned int   textSize = 1,
                 unsigned short color    = ILI9341_WHITE,
                 unsigned short bkcolor  = ILI9341_BLACK);

  // Parse color string to ILI9341 color
  // param [in] s : The color string (white, red, ...)
  // return : color (default ILI9341_WHITE)
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

  Adafruit_ILI9341 *tft = nullptr;

private:

  ILI9xxx_type_e _displayType;
  uint16_t       _xpix;
  uint16_t       _ypix;
};
#endif // ifdef USES_P095
#endif // ifndef PLUGINSTRUCTS_P095_DATA_STRUCT_H
