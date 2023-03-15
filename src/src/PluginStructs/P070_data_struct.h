#ifndef PLUGINSTRUCTS_P070_DATA_STRUCT_H
#define PLUGINSTRUCTS_P070_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P070


# include <Adafruit_NeoPixel.h>


# define NUMBER_LEDS      60 // number of LED in the strip

struct P070_data_struct : public PluginTaskData_base {
  P070_data_struct() = default;

  virtual ~P070_data_struct();

  void reset();

  void init(struct EventStruct *event);

  void set(struct EventStruct *event);


  void Clock_update();

  void calculateMarks();

  void clearClock();

  void timeToStrip(int hours,
                   int minutes,
                   int seconds);

  bool    display_enabled       = false; // used to enable/disable the display.
  uint8_t brightness            = 0;     // brightness of the clock "hands"
  uint8_t brightness_hour_marks = 0;     // brightness of the hour marks
  uint8_t offset_12h_mark       = 0;     // position of the 12 o'clock LED on the strip
  bool    thick_12_mark         = false; // thicker marking of the 12h position
  uint8_t marks[14]             = { 0 }; // Positions of the hour marks and dials

  Adafruit_NeoPixel *Plugin_070_pixels = nullptr;
};


#endif // ifdef USES_P070
#endif // ifndef PLUGINSTRUCTS_P070_DATA_STRUCT_H
