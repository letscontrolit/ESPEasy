#include "_Plugin_Helper.h"

#ifdef USES_P038

// #######################################################################################################
// #################################### Plugin 038: NeoPixel Basic #######################################
// #######################################################################################################

// Changelog:
// 2022-12-26, tonhuisman:  Set initial brightness with default value 255, and allow 'only' values 1..255
// 2022-11-06, tonhuisman:  Add Initial and Max brightness settings, and NeoPixelBright[,0..255] command, 0 = initial
//                          Code optimizations
// 2022-01-29, tonhuisman:  Resolve FIXME for GPIO selection, update comments
// 2022-01-23, tonhuisman:  Some duplicate code unduplicated, some optimizations
// 2022-01-10, tonhuisman:  Make plugin multi-instance compatible, by moving variables and code to P038_data_struct

// 15-June-2017: Fixed broken plugin; tested with several neopixels (single LED, 8 LED bars and 300 leds strips.

// List of commands:
// (1) NeoPixel,<led nr>,<red 0-255>,<green 0-255>,<blue 0-255>
// (2) NeoPixelAll,<red 0-255>,<green 0-255>,<blue 0-255>
// (3) NeoPixelLine,<start led nr>,<stop led nr>,<red 0-255>,<green 0-255>,<blue 0-255>

// Usage:
// (1): Set RGB Color to specified LED number (eg. NeoPixel,5,255,255,255)
// (2): Set all LED to specified color (eg. NeoPixelAll,255,255,255)
//		If you use 'NeoPixelAll' this will off all LED (like NeoPixelAll,0,0,0)
// (3): Set color LED between <start led nr> and <stop led nr> to specified color (eg. NeoPixelLine,1,6,255,255,255)

// RGBW note:
// for RGBW strips append the additional <brightness> to the commands
// eg: NeoPixel,<led nr>,<red 0-255>,<green 0-255>,<blue 0-255>,<brightness 0-255>
// 2022-01-23: The NeoPixelLine command now also works for RGBW

// Added HSV compatibility for Homie convention and others
// Hue, Satuation and Value (Intensity/Brightness) is a more human readable and easier to adjust color space with only 3 values
// can therfor be used for RGBW LEDs too without limitations mentioned above.
// expects Hue from 0-360° and satuation and value form 0-100% so can be used with integers too.
// Used functions HUE2RGB & HUE2RGBW can handle float and are precice but not optimized for speed!

# include "./src/PluginStructs/P038_data_struct.h"

# define PLUGIN_038
# define PLUGIN_ID_038         38
# define PLUGIN_NAME_038       "Output - NeoPixel (Basic)"
# define PLUGIN_VALUENAME1_038 ""

boolean Plugin_038(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number    = PLUGIN_ID_038;
      Device[deviceCount].Type        = DEVICE_TYPE_SINGLE;
      Device[deviceCount].TimerOption = false;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_038);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_038));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_output(F("DIN"));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P038_CONFIG_BRIGHTNESS = 255;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("Led Count"), F("pleds"), P038_CONFIG_LEDCOUNT, 1, 999);

      {
        const __FlashStringHelper *options[] = { F("GRB"), F("GRBW") };
        int indices[]                        = { P038_STRIP_TYPE_RGB, P038_STRIP_TYPE_RGBW };
        addFormSelector(F("Strip Type"), F("pstrip"), 2, options, indices, P038_CONFIG_STRIPTYPE);
      }

      if (P038_CONFIG_BRIGHTNESS == 0) { P038_CONFIG_BRIGHTNESS = 255; }
      addFormNumericBox(F("Initial brightness"), F("ibright"), P038_CONFIG_BRIGHTNESS, 1, 255);
      addUnit(F("1..255"));

      if (P038_CONFIG_MAXBRIGHT == 0) { P038_CONFIG_MAXBRIGHT = 255; }
      addFormNumericBox(F("Maximum allowed brightness"), F("maxbright"), P038_CONFIG_MAXBRIGHT, 1, 255);
      addUnit(F("1..255"));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P038_CONFIG_LEDCOUNT   = getFormItemInt(F("pleds"));
      P038_CONFIG_STRIPTYPE  = getFormItemInt(F("pstrip"));
      P038_CONFIG_BRIGHTNESS = getFormItemInt(F("ibright"));
      P038_CONFIG_MAXBRIGHT  = getFormItemInt(F("maxbright"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      if (P038_CONFIG_BRIGHTNESS == 0) { P038_CONFIG_BRIGHTNESS = 255; }

      if (P038_CONFIG_MAXBRIGHT == 0) { P038_CONFIG_MAXBRIGHT = 255; }
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P038_data_struct(CONFIG_PIN1,
                                                                               P038_CONFIG_LEDCOUNT,
                                                                               P038_CONFIG_STRIPTYPE,
                                                                               P038_CONFIG_BRIGHTNESS,
                                                                               P038_CONFIG_MAXBRIGHT));
      P038_data_struct *P038_data = static_cast<P038_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P038_data) && P038_data->plugin_init(event);

      break;
    }

    case PLUGIN_EXIT:
    {
      P038_data_struct *P038_data = static_cast<P038_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P038_data) {
        success = P038_data->plugin_exit(event);
      }

      break;
    }

    case PLUGIN_WRITE:
    {
      P038_data_struct *P038_data = static_cast<P038_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P038_data) {
        success = P038_data->plugin_write(event, string);
      }

      break;
    }
  }
  return success;
}

#endif // USES_P038
