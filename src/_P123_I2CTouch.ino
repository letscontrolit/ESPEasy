#include "_Plugin_Helper.h"
#ifdef USES_P123

// #######################################################################################################
// ###################################### Plugin 123: I2C Touchscreens ###################################
// #######################################################################################################

/**
 * Changelog:
 * 2024-06-10 tonhuisman: Add support for CHSC5816, as found in https://github.com/lewisxhe/SensorLib by Lewis He (added to bb_captouch)
 * 2024-06-02 tonhuisman: Renamed to I2C Touchscreens
 *                        Refactor to use modified bb_captouch library https://github.com/bitbank2/bb_captouch to add support for
 *                        FT62x6 and GT911 (tested), CST820, CST226 and AXS15231 (untested)
 *                        The library supports auto-configure, but that only checks if an I2C device is available at certain addresses
 *                        so that's not really reliable. Added methods to override the used touchscreen with the known device
 *                        Only single-point support for now.
 *                        For the auto-configure option the I2C device check is disabled for this plugin.
 * 2024-03-21 tonhuisman: Refactor increment/decrement to next/prevButtonGroup/Page functions, to align with ESPEasy_TouchHandler
 * 2023-12-31 tonhuisman: Code optimizations
 * 2023-10-01 tonhuisman: Re-implement (fix) switching of X/Y/Z vs X/Y output values using PLUGIN_GET_DEVICEVALUECOUNT, store (also) in task
 *                        settings for speed
 *                        Implement PLUGIN_I2C_GET_ADDRESS function
 * 2023-08-15 tonhuisman: Implement Extended CustomTaskSettings
 * 2022-12-04 tonhuisman: Remove [Testing] tag from plugin name
 * 2022-09-26 tonhuisman: Add nullptr checks, improved log/string handling
 * 2022-08-15 tonhuisman: Add Swipe and Slider support (to TouchHandler)
 * 2022-08-15 tonhuisman: UI improvement, settings table uses alternate color per 2 rows, code improvements
 * 2022-06-10 tonhuisman: Remove p123_ prefixes on Settings variables
 * 2022-06-06 tonhuisman: Move PLUGIN_WRITE handling mostly to ESPEasy_TouchHandler (only rot and flip subcommands remain)
 *                        Move PLUGIN_GET_CONFIG_VALUE handling to ESPEasy_TouchHandler
 * 2022-05-29 tonhuisman: Extend enable,disable subcommands to support a list of objects
 * 2022-05-28 tonhuisman: Add incpage and decpage subcommands that + and - 10 to the current buttongroup
 * 2022-05-26 tonhuisman: Add touch,updatebutton command
 * 2022-05-23 tonhuisman: Refactor touch settings and button emulation into ESPEasy_TouchHandler class for reuse in P099
 * 2022-05-02 tonhuisman: Small updates and improvements
 * 2022-04-30 tonhuisman: Add support for AdaGFX btn subcommand use and (local) button groups
 *                        Start preparations for refactoring touch objects into separate helper class
 * 2022-04-25 tonhuisman: Code cleanup, initialize object event -2 for disabled objects, -1 for enabled objects
 *                        Add on and off subcommands to switch a touchbutton object. Generate init-event for enable
 *                        disable, on and off states when init events option enabled
 * 2022-04-24 tonhuisman: Code improvements, increased button response speed
 * 2022-04-24 tonhuisman: Add event arguments for OnOff button objects, fix addLog statements, minor improvements
 * 2022-04-23 tonhuisman: Rename struct TS_Point in FT6206 library to FT_Point to avoid conflict with XPT2048 library (P099)
 * 2021-11-07 tonhuisman: Initial plugin, based on _P099_XPT2046_Touchscreen.ino plugin and Adafruit FT6206 Library
 */

/**
 * Commands supported:
 * -------------------
 * touch,rot,<0..3>       : Set rotation to 0(0), 90(1), 180(2), 270(3) degrees
 * touch,flip,<0|1>       : Set rotation normal(0) or flipped by 180 degrees(1)
 *
 * Other commands: see ESPEasy_TouchHandler.h
 */

# define PLUGIN_123
# define PLUGIN_ID_123         123
# define PLUGIN_NAME_123       "Touch - I2C Touchscreens"
# define PLUGIN_VALUENAME1_123 "X"
# define PLUGIN_VALUENAME2_123 "Y"
# define PLUGIN_VALUENAME3_123 "Z"

# include "src/PluginStructs/P123_data_struct.h"


boolean Plugin_123(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number             = PLUGIN_ID_123;
      dev.Type               = DEVICE_TYPE_I2C;
      dev.VType              = Sensor_VType::SENSOR_TYPE_TRIPLE;
      dev.ValueCount         = 3;
      dev.ExitTaskBeforeSave = false;
      dev.I2CNoDeviceCheck   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string  = F(PLUGIN_NAME_123);
      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_123));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_123));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_123));
      success = true;
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      success = P123_data_struct::plugin_i2c_has_address(event->Par1);
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P123_CONFIG_DISPLAY_TASK = event->TaskIndex; // Preselect current task to avoid pointing to Task 1 by default
      P123_CONFIG_THRESHOLD    = P123_TS_THRESHOLD;
      P123_CONFIG_ROTATION     = P123_TS_ROTATION;
      P123_CONFIG_X_RES        = P123_TS_X_RES;
      P123_CONFIG_Y_RES        = P123_TS_Y_RES;
      P123_I2C_ADDRESS         = 0x38; // Former default was FT62x6
      P123_SET_TOUCH_TYPE(static_cast<int>(P123_TouchType_e::FT62x6));
      P123_INTERRUPTPIN = -1;          // No interrupt
      P123_RESETPIN     = -1;          // No reset

      success = true;
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P123_I2C_ADDRESS;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = P123_CONFIG_VTYPE;
      success     = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      # ifdef PLUGIN_123_DEBUG
      addLogMove(LOG_LEVEL_INFO, F("P123 PLUGIN_WEBFORM_LOAD"));
      # endif // ifdef PLUGIN_123_DEBUG
      {
        addRowLabel(F("Display task"));
        addTaskSelect(F("dsptask"), P123_CONFIG_DISPLAY_TASK);
        # ifndef P123_LIMIT_BUILD_SIZE
        addFormNote(F("Screen Width, Heigth, Rotation &amp; Color-depth will be fetched from the Display task if possible."));
        # endif // ifndef P123_LIMIT_BUILD_SIZE
      }

      uint16_t width_      = P123_CONFIG_X_RES;
      uint16_t height_     = P123_CONFIG_Y_RES;
      uint16_t rotation_   = P123_CONFIG_ROTATION;
      uint16_t colorDepth_ = P123_COLOR_DEPTH;

      if (P123_CONFIG_DISPLAY_TASK != P123_CONFIG_DISPLAY_PREV) { // Changed since last saved?
        getPluginDisplayParametersFromTaskIndex(P123_CONFIG_DISPLAY_TASK, width_, height_, rotation_, colorDepth_);
      }
      P123_COLOR_DEPTH = colorDepth_;

      if (width_ == 0) {
        width_ = P123_TS_X_RES; // default value
      }
      addFormNumericBox(F("Screen Width (px) (x)"), F("xres"), width_, 1, 65535);


      if (height_ == 0) {
        height_ = P123_TS_Y_RES; // default value
      }
      addFormNumericBox(F("Screen Height (px) (y)"), F("yres"), height_, 1, 65535);

      AdaGFXFormRotation(F("rotate"), rotation_);

      AdaGFXFormColorDepth(F("colordepth"), P123_COLOR_DEPTH, (colorDepth_ == 0));

      addFormNumericBox(F("Touch minimum pressure"), F("threshold"), P123_CONFIG_THRESHOLD, 0, 255);
      addUnit(F("Only used for FT62x6"));

      {
        P123_data_struct *P123_data = static_cast<P123_data_struct *>(getPluginTaskData(event->TaskIndex));
        bool deleteP123_data        = false;
        {
          const __FlashStringHelper *touchTypes[] = {
            toString(P123_TouchType_e::FT62x6),
            toString(P123_TouchType_e::GT911_1),
            toString(P123_TouchType_e::GT911_2),
            toString(P123_TouchType_e::CST820),
            toString(P123_TouchType_e::CST226),
            toString(P123_TouchType_e::AXS15231),
            toString(P123_TouchType_e::CHSC5816),
            toString(P123_TouchType_e::Automatic),
          };
          const int touchTypeOptions[] = {
            static_cast<int>(P123_TouchType_e::FT62x6),
            static_cast<int>(P123_TouchType_e::GT911_1),
            static_cast<int>(P123_TouchType_e::GT911_2),
            static_cast<int>(P123_TouchType_e::CST820),
            static_cast<int>(P123_TouchType_e::CST226),
            static_cast<int>(P123_TouchType_e::AXS15231),
            static_cast<int>(P123_TouchType_e::CHSC5816),
            static_cast<int>(P123_TouchType_e::Automatic),
          };
          constexpr size_t optionCount = NR_ELEMENTS(touchTypeOptions);
          addFormSelector(F("Touchscreen type (address)"),
                          F("ttype"),
                          optionCount,
                          touchTypes,
                          touchTypeOptions,
                          P123_GET_TOUCH_TYPE);

          if (nullptr != P123_data) {
            addUnit(concat(F("Detected: "), toString(P123_data->getTouchType())));
          }
        }
        addFormPinSelect(PinSelectPurpose::Generic_bidir, F("Interrupt pin"), F("taskdevicepin1"), P123_INTERRUPTPIN);
        addFormPinSelect(PinSelectPurpose::Generic_bidir, F("Reset pin"),     F("taskdevicepin2"), P123_RESETPIN);
        addFormNote(F("Interrupt and Reset pins are optional. Interrupt is only used by GT911."));

        if (nullptr == P123_data) {
          P123_data       = new (std::nothrow) P123_data_struct(static_cast<P123_TouchType_e>(P123_GET_TOUCH_TYPE));
          deleteP123_data = true;
        }

        if (nullptr != P123_data) {
          P123_data->plugin_webform_load(event);

          if (deleteP123_data) {
            delete P123_data;
          }
        }
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      # ifdef PLUGIN_123_DEBUG
      addLogMove(LOG_LEVEL_INFO, F("P123 PLUGIN_WEBFORM_SAVE"));
      # endif // ifdef PLUGIN_123_DEBUG
      P123_CONFIG_DISPLAY_PREV = P123_CONFIG_DISPLAY_TASK;
      P123_CONFIG_THRESHOLD    = getFormItemInt(F("threshold"));
      P123_CONFIG_DISPLAY_TASK = getFormItemInt(F("dsptask"));
      P123_CONFIG_ROTATION     = getFormItemInt(F("rotate"));
      P123_CONFIG_X_RES        = getFormItemInt(F("xres"));
      P123_CONFIG_Y_RES        = getFormItemInt(F("yres"));
      P123_SET_TOUCH_TYPE(getFormItemInt(F("ttype")));
      P123_I2C_ADDRESS = P123_data_struct::plugin_i2c_address(static_cast<P123_TouchType_e>(P123_GET_TOUCH_TYPE));

      // taskdevicepin1/taskdevicepin2 are saved automatically

      const int colorDepth = getFormItemInt(F("colordepth"), -1);

      if (colorDepth != -1) {
        P123_COLOR_DEPTH = colorDepth;
      }

      {
        P123_data_struct *P123_data = nullptr; // static_cast<P123_data_struct *>(getPluginTaskData(event->TaskIndex));
        bool deleteP123_data        = false;

        if (nullptr == P123_data) {
          P123_data       = new (std::nothrow) P123_data_struct(static_cast<P123_TouchType_e>(P123_GET_TOUCH_TYPE));
          deleteP123_data = true;
        }

        if (nullptr != P123_data) {
          success = P123_data->plugin_webform_save(event);

          if (deleteP123_data) {
            delete P123_data;
          }
        }
      }

      break;
    }

    case PLUGIN_INIT:
    {
      # ifdef PLUGIN_123_DEBUG
      addLogMove(LOG_LEVEL_INFO, F("P123 PLUGIN_INIT"));
      # endif // ifdef PLUGIN_123_DEBUG

      if (0 == P123_I2C_ADDRESS) {
        P123_I2C_ADDRESS = 0x38; // Former default was FT62x6
        P123_SET_TOUCH_TYPE(static_cast<int>(P123_TouchType_e::FT62x6));
      }
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P123_data_struct(static_cast<P123_TouchType_e>(P123_GET_TOUCH_TYPE)));
      P123_data_struct *P123_data = static_cast<P123_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P123_data) {
        return success;
      }

      success = true;

      if (!P123_data->init(event)) {
        clearPluginTaskData(event->TaskIndex);
        success = false;
      }
      break;
    }

    // case PLUGIN_READ: // Not implemented on purpose, *only* send out events/values when device is touched, and configured to send events

    case PLUGIN_WRITE:
    {
      P123_data_struct *P123_data = static_cast<P123_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P123_data) {
        success = P123_data->plugin_write(event, string);
      }

      break;
    }

    case PLUGIN_FIFTY_PER_SECOND: // Increased response
    {
      P123_data_struct *P123_data = static_cast<P123_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P123_data) {
        success = P123_data->plugin_fifty_per_second(event);
      }

      break;
    }

    case PLUGIN_GET_CONFIG_VALUE:
    {
      P123_data_struct *P123_data = static_cast<P123_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P123_data) {
        success = P123_data->plugin_get_config_value(event, string);
      }
      break;
    }
  } // switch(function)
  return success;
}   // Plugin_123

#endif // USES_P123
