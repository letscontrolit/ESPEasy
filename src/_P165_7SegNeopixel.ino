#include "_Plugin_Helper.h"
#ifdef USES_P165

// #######################################################################################################
// ################################## Plugin-165: Display - 7-Segment NeoPixel ###########################
// #######################################################################################################

/** Changelog:
 * 2024-09-19 tonhuisman: Add option to use decimal dot of second digit for blinking time indicator (with applied offset).
 * 2024-09-17 tonhuisman: Add numbering start a g-segment. That disables the g-split and dot-at-end options, as those are then not useful.
 *                        Also supported for counter-clockwise numbering, but then the right-to-left option should best also be enabled.
 * 2024-09-05 tonhuisman: Disable automatic period handling if no decimal dots are defined.
 * 2024-09-01 tonhuisman: Fixing some bugs/typos: g-split was enabled for width-pixels > 1, P073 font characters must have 7 bits reverted.
 *                        7groupcolor wasn't handling arguments correctly. Restoring groupcolor after digitcolor fixed.
 * 2024-08-31 tonhuisman: Apply Device[].PinXDirection feature so only an output-capable GPIO can be selected for the Strip
 * 2024-08-29 tonhuisman: Add Counter-clockwise numbering option pe group. Enable GroupColor and DigitColor features also fog
 *                        LIMIT_BUILD_SIZE builds, as the ESP8266 NeoPixel builds still have enough space available.
 * 2024-08-28 tonhuisman: Fix digit mapping when using Right to Left digit ordering
 *                        Moved around a few UI options, and changed some captions, fix editing digit options by keyboard.
 * 2024-08-27 tonhuisman: Changed max. number of pixels per segment to 7 (from 5), changed settings storage from 3 to 4 bits for
 *                        height, width and dot pixels per segment (not fully used yet, just preparing for future expansion).
 *                        Add checks for not having > 64 bits per digit, as that's technically not supported.
 *                        Reduce Digit display to ca. 80% size to save some screen real-estate.
 *                        Add command 7dbefore, like 7dextra, to set the Pixels-offset before pixels on/off, left half, right half.
 * 2024-08-26 tonhuisman: Rework the UI somewhat to improve the display of the Extra pixels per group
 *                        Add option for Clear on exit
 *                        Add option to split the g-segment in 2 halves when assigning the number plan (should best be set from 3 pixels)
 *                        this option was enabled by default from 3 horizontal pixels, can now be set manually.
 * 2024-08-22 tonhuisman: Increased segment pixels for height to 5, equal to width
 *                        Increased decimal point digits to 7 (max) and extra pixels to 12
 *                        When font color is either white or black, uses default color to stay visible in both light and dark Web-UI mode.
 * 2024-08-20 tonhuisman: Draws either pixeled digits (edit mode) or the number plan (non-edit mode)
 *                        Commands mostly similar to P073, including standard date/time content, text scrolling and binary content
 *                        Additional RGB(W) colors can be set globally, per group, and per digit, both fontcolor and backcolor
 * 2024-08-03 tonhuisman: Allow 1 to 4 groups of 1 to 4 digits, with decimal point (0..4 px) and extra pixels (0..10) per group
 *                        Max pixels: horizontal segments 5/7, vertical segments 4/6. (7/6 with corner-overlap) Optional start-offset.
 *                        Default content: Manual, Date/Time (12/24h), similar to P073
 *                        Has javascript code to show selected configuration-example
 * 2024-05-12 tonhuisman: Initial start of plugin,
 *                        based on Noiasca library from https://werner.rothschopf.net/202005_arduino_neopixel_display_en.htm
 */

# include "src/PluginStructs/P165_data_struct.h"

# define PLUGIN_165
# define PLUGIN_ID_165         165
# define PLUGIN_NAME_165       "Display - NeoPixel (7-Segment)"

boolean Plugin_165(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number = PLUGIN_ID_165;
      dev.Type   = DEVICE_TYPE_SINGLE;
      dev.VType  = Sensor_VType::SENSOR_TYPE_NONE;
      dev.setPin1Direction(gpio_direction::gpio_output);
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_165);
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P165_CONFIG_GROUPCOUNT = 1;
      P165_CONFIG_DEF_BRIGHT = 40;
      P165_CONFIG_MAX_BRIGHT = 255;
      P165_CONFIG_FG_COLOR   = ADAGFX_RED;

      for (uint8_t grp = 0; grp < PLUGIN_CONFIGLONGVAR_MAX; ++grp) {
        P165_data_struct::initDigitGroup(event, grp);
      }

      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_output(F("Strip"));
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P165_data_struct(event));

      P165_data_struct *P165_data = static_cast<P165_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P165_data && P165_data->isInitialized());
      break;
    }

    case PLUGIN_READ:
    {
      // Nothing to see here
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      success = P165_data_struct::plugin_webform_load(event);
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      success = P165_data_struct::plugin_webform_save(event);
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      P165_data_struct *P165_data = static_cast<P165_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P165_data && P165_data->plugin_once_a_second(event));
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P165_data_struct *P165_data = static_cast<P165_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P165_data && P165_data->plugin_ten_per_second(event));
      break;
    }
    case PLUGIN_WRITE:
    {
      P165_data_struct *P165_data = static_cast<P165_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P165_data && P165_data->plugin_write(event, string));
      break;
    }
  }
  return success;
}

#endif // ifdef USES_P165
