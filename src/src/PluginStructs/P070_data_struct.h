#ifndef PLUGINSTRUCTS_P070_DATA_STRUCT_H
#define PLUGINSTRUCTS_P070_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P070


#include <NeoPixelBus_wrapper.h>


# define NUMBER_LEDS_60   60  // number of LEDs for 60-LED ring
# define NUMBER_LEDS_24   24  // number of LEDs for 24-LED ring
# define NUMBER_LEDS_16   16  // number of LEDs for 16-LED ring
# define NUMBER_LEDS_12   12  // number of LEDs for 12-LED ring

# define P070_LED_MODE_60  0  // Selector value for 60-LED mode
# define P070_LED_MODE_24  1  // Selector value for 24-LED mode
# define P070_LED_MODE_16  2  // Selector value for 16-LED mode
# define P070_LED_MODE_12  3  // Selector value for 12-LED mode

// Task name of the BH1750 lux sensor (P010), used for auto brightness
# define P070_LUX_TASK_NAME  "BH1750"

// LED strip type
# define P070_LED_TYPE_RGB   0  // RGB  LEDs (3 channel)
# define P070_LED_TYPE_RGBW  1  // RGBW LEDs (4 channel, dedicated white)

// Lightshow modes
# define P070_LIGHTSHOW_OFF    0  // no lightshow
# define P070_LIGHTSHOW_CHASE  1  // colour-chase / Lauflicht
# define P070_LIGHTSHOW_FLASH  2  // all-LED flash + hand blink

// Predefined color indices
# define P070_COLOR_RED      0
# define P070_COLOR_GREEN    1
# define P070_COLOR_BLUE     2
# define P070_COLOR_YELLOW   3
# define P070_COLOR_CYAN     4
# define P070_COLOR_MAGENTA  5
# define P070_COLOR_WHITE    6
# define P070_COLOR_COUNT    7

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

  // Returns a brightness value (32..255) based on lux level
  uint8_t luxToBrightness(float lux);

  // Reads lux from BH1750 task and updates brightness when auto_brightness is active
  void applyAutoBrightness();

  // Fills r,g,b with the color for colorIndex scaled by bright
  void colorFromIndex(uint8_t colorIndex, uint8_t bright, uint8_t &r, uint8_t &g, uint8_t &b);

  // Sets a pixel color, using W channel for white if RGBW mode
  void setPixel(uint16_t pos, uint8_t r, uint8_t g, uint8_t b);

  // Runs one step of the hourly lightshow; called once per second
  void runLightshow();

  bool    display_enabled       = false;            // used to enable/disable the display
  uint8_t brightness            = 0;                // brightness of the clock "hands"
  uint8_t brightness_hour_marks = 0;                // brightness of the hour marks
  uint8_t offset_12h_mark       = 0;                // position of the 12 o'clock LED on the strip
  bool    thick_12_mark         = false;            // thicker marking of the 12h position
  uint8_t led_mode              = P070_LED_MODE_60; // 0 = 60 LEDs, 1 = 24 LEDs
  uint8_t led_type              = P070_LED_TYPE_RGB; // 0 = RGB, 1 = RGBW
  uint8_t number_leds           = NUMBER_LEDS_60;   // actual LED count based on mode
  bool    auto_brightness       = false;            // automatic brightness control via BH1750

  // Hand / mark colors (use P070_COLOR_* indices)
  uint8_t color_hours           = P070_COLOR_RED;
  uint8_t color_minutes         = P070_COLOR_GREEN;
  uint8_t color_seconds         = P070_COLOR_BLUE;
  uint8_t color_marks           = P070_COLOR_WHITE;

  // Hourly lightshow
  uint8_t lightshow_mode        = P070_LIGHTSHOW_OFF; // 0=off, 1=chase, 2=flash
  uint8_t lightshow_duration    = 5;                  // duration in seconds (1-30)
  uint8_t lightshow_color       = P070_COLOR_RED;     // random color chosen at start of show
  bool    lightshow_running     = false;              // true while show is active
  uint8_t lightshow_step        = 0;                  // current LED position in chase
  uint8_t lightshow_flash_phase = 0;                  // sub-phase counter for flash show

  uint8_t marks[14]             = { 0 };          // positions of the hour marks

  NeoPixelBus_wrapper *Plugin_070_pixels = nullptr;
};


#endif // ifdef USES_P070
#endif // ifndef PLUGINSTRUCTS_P070_DATA_STRUCT_H
