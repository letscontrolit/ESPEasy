#include "_Plugin_Helper.h"

#ifdef USES_P128

// #######################################################################################################
// #################################### Plugin 128: NeoPixelBusFX ########################################
// #######################################################################################################

// Changelog:
// 2022-07-20, tonhuisman Make FakeTV compile-time optional, disabled by default on ESP8266, enabled by default on ESP32
//                        P128_ENABLE_FAKETV can be set to 0/1 in Custom.h
// 2022-07-02, tonhuisman Introduce Max Brightness setting for protecting the hardware and power supply (and the eyes :-))
// 2022-06-12, tonhuisman Optimizations, revert Makuna/NeopixelBus library to 2.6.9 for incompatibilties like [[maybe_unused]] arguments
// 2022-01-30, tonhuisman Fix JSON message to use proper JSON functions, some bugfixes and small source improvements
// 2022-01-09, tonhuisman Add conditional defines P128_USES_<colormode> (options: GRB/GRBW/RGB/RGBW/BRG/BRG) for selecting the
//                        desired pixel type, optionally, from Custom.h
// 2022-01-08, tonhuisman Small UI fixes while writing documentation.
// 2022-01-06, tonhuisman Move from Development to Testing
// 2022-01-05, tonhuisman Code optimizations and review feedback
// 2022-01-04, tonhuisman Code optimizations, reduce calls to parseString and rgbStr2Num, string handling
//                        Ensure no instance is still running when PLUGIN_INIT is called, as it will cause RMT issues
// 2022-01-03, tonhuisman Code optimizations, struct variable initialization
// 2022-01-02, tonhuisman Move all code to Plugin_data_struct, with minor modifications (initialization, *char[] size)
// 2022-01-02, tonhuisman Fixed ESP32 related issues (conditional compilation, wrong stripe type)
//                        Add configuration for ESP32 GPIO pin
// 2022-01-01, tonhuisman On request migrated from https://github.com/djcysmic/NeopixelBusFX to ESPEasy (that is an
//                        extension from plugin _P124_NeoPixelBusFX.ino on the ESPEasyPluginPlayground)
//                        - Adjusted to work combined with FastLED library (random8() issues)
//                        - Moved firetv.h data to P128_data_struct.h
//                        - Use PCONFIG(0) macro where applicable

/*
   List of commands:

   nfx off [fadetime] [delay]
   switches the stripe off

   nfx on [fadetime] [delay]
   restores last state of the stripe

   nfx dim [dimvalue]
   dimvalue 0-255

   nfx line startpixel endpixel color
   nfx hsvline startpixel endpixel hue saturation brightness

   nfx one pixel color
   nfx hsvone pixel hue saturation brightness

   nfx all color [fadetime] [delay]
   nfx rgb color [fadetime] [delay]
   nfx fade color [fadetime] [delay]

   nfx hsv hue saturation brightness [fadetime] [delay]

   nfx colorfade startcolor endcolor [startpixel] [endpixel]

   nfx rainbow [speed] [fadetime]

   nfx kitt color [speed]

   nfx comet color [speed]

   nfx theatre color [backgroundcolor] [count] [speed]

   nfx scan color [backgroundcolor] [speed]

   nfx dualscan color [backgroundcolor] [speed]

   nfx twinkle color [backgroundcolor] [speed]

   nfx twinklefade color [count] [speed]

   nfx sparkle color [backgroundcolor] [speed]

   nfx wipe color [dotcolor] [speed]

   nfx dualwipe [dotcolor] [speed]

   nfx fire [fps] [brightness] [cooling] [sparking]

   nfx fireflicker [intensity] [speed]

   nfx faketv [startpixel] [endpixel]

   nfx simpleclock [bigtickcolor] [smalltickcolor] [hourcolor] [minutecolor] [secondcolor, set "off" to disable] [backgroundcolor]


   nfx stop
   stops the effect

   nfx statusrequest
   sends status

   nfx fadetime
   nfx fadedelay
   nfx speed
   nfx count
   nfx bgcolor
   sets default parameter

   Use:

   needed:
   color,backgroundcolor -> targetcolor in hex format e.g. ff0000 for red

   [optional]:
   fadetime ->  fadetime per pixel in ms
   delay ->  delay time to next pixel in ms, if delay < 0 fades from other end of the stripe
   speed -> 0-50, speed < 0 for reverse

   Sourced from: https://github.com/djcysmic/NeopixelBusFX

   Based on Adafruit Fake TV Light for Engineers, WS2812FX, NeoPixelBus, Lights, NeoPixel - Basic and Candle modules

   https://learn.adafruit.com/fake-tv-light-for-engineers/overview
   https://github.com/letscontrolit/ESPEasy
   https://github.com/kitesurfer1404/WS2812FX
   https://github.com/Makuna/NeoPixelBus
   https://github.com/ddtlabs/ESPEasy-Plugin-Lights

   Thank you to all developers
 */

# include "src/PluginStructs/P128_data_struct.h" // includes faketv.h, the color pattern for FakeTV

# define PLUGIN_128
# define PLUGIN_ID_128         128
# define PLUGIN_NAME_128       "Output - NeoPixel (BusFX)"
# define PLUGIN_VALUENAME1_128 "Mode"
# define PLUGIN_VALUENAME2_128 "Lastmode"
# define PLUGIN_VALUENAME3_128 "Fadetime"
# define PLUGIN_VALUENAME4_128 "Fadedelay"

boolean Plugin_128(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number = PLUGIN_ID_128;
      # if defined(ESP32)
      Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
      # endif // if defined(ESP32)
      # if defined(ESP8266)
      Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
      # endif // if defined(ESP8266)
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].Custom             = true;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 4;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].TimerOptional      = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].DecimalsOnly       = false;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_128);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_128));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_128));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_128));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_128));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      # ifdef ESP32
      PIN(0) = -1;                  // None
      # endif // ifdef ESP32
      P128_CONFIG_MAX_BRIGHT = 255; // Allow full brightness by default, range 1..255
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormSubHeader(F("Actuator"));

      addRowLabel(formatGpioName_output(F("Stripe data")));
      # ifdef ESP8266
      addHtml(F("<span style=\"color:red\">Please connect stripe to GPIO2!</span>"));
      # endif // ifdef ESP8266
      # ifdef ESP32
      addPinSelect(PinSelectPurpose::Generic_output, F("taskdevicepin1"), PIN(0));
      # endif // ifdef ESP32

      addFormNumericBox(F("Led Count"),      F("ledcnt"),    P128_CONFIG_LED_COUNT,  1, 999);

      if (P128_CONFIG_MAX_BRIGHT == 0) { P128_CONFIG_MAX_BRIGHT = 255; } // Set to default for existing installations
      addFormNumericBox(F("Max brightness"), F("maxbright"), P128_CONFIG_MAX_BRIGHT, 1, 255);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P128_CONFIG_LED_COUNT  = getFormItemInt(F("ledcnt"));
      P128_CONFIG_MAX_BRIGHT = getFormItemInt(F("maxbright"));

      # ifdef ESP32
      PIN(0) = getFormItemInt(F("taskdevicepin1"));
      # endif // ifdef ESP32

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      // *Ensure* there is no currently running process, if so, just clear it out neatly
      P128_data_struct *P128_clear = static_cast<P128_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P128_clear) {
        clearPluginTaskData(event->TaskIndex);
      }

      if (P128_CONFIG_MAX_BRIGHT == 0) { P128_CONFIG_MAX_BRIGHT = 255; } // Set to default for existing installations
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P128_data_struct(PIN(0), P128_CONFIG_LED_COUNT, P128_CONFIG_MAX_BRIGHT));
      P128_data_struct *P128_data = static_cast<P128_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = nullptr != P128_data;
      break;
    }

    case PLUGIN_READ: // ------------------------------------------->
    {
      P128_data_struct *P128_data = static_cast<P128_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P128_data) {
        success = P128_data->plugin_read(event);
      }

      break;
    }

    case PLUGIN_WRITE:
    {
      P128_data_struct *P128_data = static_cast<P128_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P128_data) {
        success = P128_data->plugin_write(event, string);
      }

      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      P128_data_struct *P128_data = static_cast<P128_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P128_data) {
        success = P128_data->plugin_fifty_per_second(event);
      }

      break;
    }
  }
  return success;
}

#endif // USES_P128
