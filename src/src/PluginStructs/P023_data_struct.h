#ifndef PLUGINSTRUCTS_P023_DATA_STRUCT_H
#define PLUGINSTRUCTS_P023_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#include "../../ESPEasy_common.h"

#ifdef USES_P023

# define P23_Nlines 8 // The number of different lines which can be displayed
# define P23_Nchars 64


struct P023_data_struct : public PluginTaskData_base {
  enum
  {
    OLED_64x48   = 0x01,
    OLED_rotated = 0x02,
    OLED_128x32  = 0x04
  };

  enum class Spacing
  {
    normal    = 0x01,
    optimized = 0x02
  };

  P023_data_struct(byte    _address,
                   byte    _type,
                   Spacing _font_spacing,
                   byte    _displayTimer);

  void   setDisplayTimer(byte _displayTimer);
  void   checkDisplayTimer();

  String parseTemplate(String& tmpString,
                       byte    lineSize);

  void   resetDisplay();

  void   StartUp_OLED();

  void   displayOn();

  void   displayOff();

  void   clearDisplay();

  // Actually this sends a byte, not a char to draw in the display.
  void   sendChar(unsigned char data);

  // Prints a display char (not just a byte) in coordinates X Y,
  // currently unused:
  // void Plugin_023_sendCharXY(unsigned char data, int X, int Y);

  void sendCommand(unsigned char com);

  // Set the cursor position in a 16 COL * 8 ROW map (128x64 pixels)
  // or 8 COL * 5 ROW map (64x48 pixels)
  void setXY(unsigned char row,
             unsigned char col);

  // Prints a string regardless the cursor position.
  // unused:
  // void Plugin_023_sendStr(unsigned char *string);


  // Prints a string in coordinates X Y, being multiples of 8.
  // This means we have 16 COLS (0-15) and 8 ROWS (0-7).
  void sendStrXY(const char *string,
                 int         X,
                 int         Y);

  void init_OLED();

  byte    address      = 0;
  byte    type         = 0;
  Spacing font_spacing = Spacing::normal;
  byte    displayTimer = 0;
};

#endif // ifdef USES_P023
#endif // ifndef PLUGINSTRUCTS_P023_DATA_STRUCT_H
