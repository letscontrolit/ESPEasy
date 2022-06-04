#ifdef USES_P123

// #######################################################################################################
// #################################### Plugin 123: FT6206 Touchscreen ###################################
// #######################################################################################################

/**
 * Changelog:
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
 * touch,rot,<0..3>                     : Set rotation to 0(0), 90(1), 180(2), 270(3) degrees
 * touch,flip,<0|1>                     : Set rotation normal(0) or flipped by 180 degrees(1)
 * touch,enable,<objectName|Nr>[,...]   : Enable disabled objectname(s)
 * touch,disable,<objectName|Nr>[,...]  : Disable enabled objectname(s)
 * touch,on,<buttonObjectName|Nr>       : Switch a TouchButton on (must be enabled)
 * touch,off,<buttonObjectName|Nr>      : Switch a TouchButton off (must be enabled)
 * touch,setgrp,<group>                 : Switch to button group
 * touch,incgrp                         : Switch to next button group
 * touch,decgrp                         : Switch to previous button group
 * touch,incpage                        : Switch to next button group page (+10)
 * touch,decpage                        : Switch to previous button group page (-10)
 * touch,updatebutton,<buttonName|Nr>[,<group>[,<mode>]] : Update a button by name or number
 */

#define PLUGIN_123
#define PLUGIN_ID_123         123
#define PLUGIN_NAME_123       "Touch - FT62x6 touchscreen [TESTING]"
#define PLUGIN_VALUENAME1_123 "X"
#define PLUGIN_VALUENAME2_123 "Y"
#define PLUGIN_VALUENAME3_123 "Z"

#include "_Plugin_Helper.h"
#include "src/PluginStructs/P123_data_struct.h"


boolean Plugin_123(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_123;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TRIPLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 3;
      Device[deviceCount].SendDataOption     = false;
      Device[deviceCount].TimerOption        = false;
      Device[deviceCount].ExitTaskBeforeSave = false;
      success                                = true;
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
      success = (event->Par1 == 0x38); // Fixed I2C address
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P123_CONFIG_DISPLAY_TASK = event->TaskIndex; // Preselect current task to avoid pointing to Task 1 by default
      P123_CONFIG_ROTATION     = P123_TS_ROTATION;
      P123_CONFIG_X_RES        = P123_TS_X_RES;
      P123_CONFIG_Y_RES        = P123_TS_Y_RES;

      success = true;
      break;
    }
    case PLUGIN_WEBFORM_LOAD:
    {
      addFormSubHeader(F("Screen"));

      {
        addRowLabel(F("Display task"));
        addTaskSelect(F("p123_task"), P123_CONFIG_DISPLAY_TASK);
        addFormNote(F("Screen Width, Heigth, Rotation &amp; Color-depth will be fetched from the Display task if possible."));
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
      addFormNumericBox(F("Screen Width (px) (x)"), F("p123_width"), width_, 1, 65535);


      if (height_ == 0) {
        height_ = P123_TS_Y_RES; // default value
      }
      addFormNumericBox(F("Screen Height (px) (y)"), F("p123_height"), height_, 1, 65535);

      AdaGFXFormRotation(F("p123_rotate"), rotation_);

      AdaGFXFormColorDepth(F("p123_colordepth"), P123_COLOR_DEPTH, (colorDepth_ == 0));

      addFormNumericBox(F("Touch minimum pressure"), F("p123_treshold"), P123_CONFIG_TRESHOLD, 0, 255);

      {
        P123_data_struct *P123_data = new (std::nothrow) P123_data_struct();

        if (nullptr == P123_data) {
          return success;
        }
        P123_data->loadTouchObjects(event);

        P123_data->plugin_webform_load(event);

        delete P123_data;
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P123_CONFIG_DISPLAY_PREV = P123_CONFIG_DISPLAY_TASK;
      P123_CONFIG_TRESHOLD     = getFormItemInt(F("p123_treshold"));
      P123_CONFIG_DISPLAY_TASK = getFormItemInt(F("p123_task"));
      P123_CONFIG_ROTATION     = getFormItemInt(F("p123_rotate"));
      P123_CONFIG_X_RES        = getFormItemInt(F("p123_width"));
      P123_CONFIG_Y_RES        = getFormItemInt(F("p123_height"));

      int colorDepth = getFormItemInt(F("p123_colordepth"), -1);

      if (colorDepth != -1) {
        P123_COLOR_DEPTH = colorDepth;
      }

      P123_data_struct *P123_data = new (std::nothrow) P123_data_struct();

      if (nullptr == P123_data) {
        return success; // Save other settings even though this didn't initialize properly
      }

      success = P123_data->plugin_webform_save(event);

      delete P123_data;

      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P123_data_struct());
      P123_data_struct *P123_data = static_cast<P123_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P123_data) {
        return success;
      }

      success = true;

      if (!(P123_data->init(event))) {
        delete P123_data;
        success = false;
      }
      break;
    }

    // case PLUGIN_READ: // Not implemented on purpose, *only* send out events/values when device is touched, and configured to send events

    case PLUGIN_WRITE:
    {
      {
        P123_data_struct *P123_data = static_cast<P123_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr == P123_data) {
          return success;
        }
        success = P123_data->plugin_write(event, string);
      }
      break;
    }

    case PLUGIN_FIFTY_PER_SECOND: // Increased response
    {
      P123_data_struct *P123_data = static_cast<P123_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P123_data) {
        return success;
      }

      success = P123_data->plugin_fifty_per_second(event);

      break;
    }
    case PLUGIN_GET_CONFIG_VALUE:
    {
      P123_data_struct *P123_data = static_cast<P123_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P123_data) {
        return success;
      }
      String command = parseString(string, 1);

      if (command == F("buttongroup")) {
        string  = P123_data->getButtonGroup();
        success = true;
      } else if (command == F("hasgroup")) {
        int group; // We'll be ignoring group 0 if there are multiple button groups

        if (validIntFromString(parseString(string, 2), group)) {
          string  = P123_data->validButtonGroup(group, true) ? 1 : 0;
          success = true;
        } else {
          string = '0'; // invalid number
        }
      }
      break;
    }
  } // switch(function)
  return success;
}   // Plugin_123

#endif // USES_P123
