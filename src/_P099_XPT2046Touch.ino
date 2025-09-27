#ifdef USES_P099

// #######################################################################################################
// #################################### Plugin 099: XPT2046 TFT Touchscreen #################################
// #######################################################################################################

/**
 * Changelog:
 * 2025-09-27 tonhuisman: Implement ESPEasy_TouchHandler
 * 2025-01-12 tonhuisman: Add support for MQTT AutoDiscovery (not supported for Touch)
 * 2020-11-01 tonhuisman: Solved previous strange rotation settings to be compatible with TFT ILI9341
 * 2020-11-01 tonhuisman: Add option to flip rotation by 180 deg, and command touch,flip,<0|1>
 * 2020-11-01 tonhuisman: Add option for the debounce timeout for On/Off buttons
 * 2020-11-01 tonhuisman: Some code cleanup and string optimizations
 * 2020-09-23/24/25 tonhuisman: Add object disable/enable commands, add On/Off button mode, and inverted, for touchobjects
 * 2020-09-10 tonhuisman: Clean up code, testing
 * 2020-09-07/08/09 tonhuisman: Fix code issues
 * 2020-09-06 tonhuisman: Add object 'layering' so the 'top-most' object only sends an event
 * 2020-09-05 tonhuisman: Add touch to touchobject mapping, generate events
 * 2020-09-03 tonhuisman: Add touchobject settings
 * 2020-08-31 tonhuisman: Add Calibration settings
 * 2020-08-30 tonhuisman: Add settings and 2/3 event support
 * 2020-08-29 tonhuisman: Initial plugin, based on XPT2046_Touchscreen by Paul Stoffregen from
 * https://github.com/PaulStoffregen/XPT2046_Touchscreen
 */

/**
 * Commands supported:
 * -------------------
 * touch,rot,<0..3>             : Set rotation to 0(0), 90(1), 180(2), 270(3) degrees
 * touch,flip,<0|1>             : Set rotation normal(0) or flipped by 180 degrees(1)
 * touch,enable,<objectName>    : Enables a disabled objectname (removes a leading underscore)
 * touch,disable,<objectName>   : Disables an enabled objectname (adds a leading underscore)
 */

#define PLUGIN_099
#define PLUGIN_ID_099         99
#define PLUGIN_NAME_099       "Touch - XPT2046 on a TFT display"
#define PLUGIN_VALUENAME1_099 "X"
#define PLUGIN_VALUENAME2_099 "Y"
#define PLUGIN_VALUENAME3_099 "Z"

#include "_Plugin_Helper.h"
#include "src/PluginStructs/P099_data_struct.h"


boolean Plugin_099(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number     = PLUGIN_ID_099;
      dev.Type       = DEVICE_TYPE_SPI;
      dev.VType      = Sensor_VType::SENSOR_TYPE_TRIPLE;
      dev.ValueCount = 3;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string  = F(PLUGIN_NAME_099);
      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_099));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_099));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_099));
      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_output(F("TS CS"));
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = P099_GET_CONFIG_VTYPE;
      success     = true;
      break;
    }

    #if FEATURE_MQTT_DISCOVER
    case PLUGIN_GET_DISCOVERY_VTYPES:
    {
      event->Par1 = static_cast<int>(Sensor_VType::SENSOR_TYPE_NONE); // Not yet supported
      success     = true;
      break;
    }
    #endif // if FEATURE_MQTT_DISCOVER

    case PLUGIN_SET_DEFAULTS:
    {
      P099_SET_CONFIG_DISPLAY(event->TaskIndex); // Preselect current task to avoid pointing to Task 1 by default
      P099_CONFIG_CS_PIN    = P099_TS_CS;
      P099_CONFIG_THRESHOLD = P099_TS_TRESHOLD;
      P099_CONFIG_ROTATION  = P099_TS_ROTATION;
      P099_CONFIG_X_RES     = P099_TS_X_RES;
      P099_CONFIG_Y_RES     = P099_TS_Y_RES;
      #if P099_ENABLE_OLD_CONFIG
      P099_CONFIG_OBJECTCOUNT = P099_INIT_OBJECTCOUNT;
      P099_CONFIG_DEBOUNCE_MS = P099_DEBOUNCE_MILLIS;
      #endif // if P099_ENABLE_OLD_CONFIG

      constexpr uint32_t lSettings = 0
                                     + (P099_TS_SEND_XY          ? (1 << P099_FLAGS_SEND_XY) : 0)
                                     + (P099_TS_SEND_Z           ? (1 << P099_FLAGS_SEND_Z) : 0)
                                     + (P099_TS_SEND_OBJECTNAME  ? (1 << P099_FLAGS_SEND_OBJECTNAME) : 0)
                                     + (P099_TS_USE_CALIBRATION  ? (1 << P099_FLAGS_USE_CALIBRATION) : 0)
                                     + (P099_TS_LOG_CALIBRATION  ? (1 << P099_FLAGS_LOG_CALIBRATION) : 0)
                                     + (P099_TS_ROTATION_FLIPPED ? (1 << P099_FLAGS_ROTATION_FLIPPED) : 0);
      P099_CONFIG_FLAGS = lSettings;

      success = true;
      break;
    }
    case PLUGIN_WEBFORM_LOAD:
    {
      addFormSubHeader(F("Screen"));

      {
        addRowLabel(F("Display task"));
        addTaskSelect(F("dsptask"), P099_GET_CONFIG_DISPLAY);
        #ifndef P099_LIMIT_BUILD_SIZE
        addFormNote(F("Screen Width, Heigth, Rotation &amp; Color-depth will be fetched from the Display task if possible."));
        #endif // ifndef P099_LIMIT_BUILD_SIZE
      }

      uint16_t width_      = P099_CONFIG_X_RES;
      uint16_t height_     = P099_CONFIG_Y_RES;
      uint16_t rotation_   = P099_CONFIG_ROTATION;
      uint16_t colorDepth_ = P099_COLOR_DEPTH;

      if (P099_GET_CONFIG_DISPLAY != static_cast<uint8_t>(P099_CONFIG_DISPLAY_PREV)) { // Changed since last saved?
        getPluginDisplayParametersFromTaskIndex(P099_GET_CONFIG_DISPLAY, width_, height_, rotation_, colorDepth_);
      }
      P099_COLOR_DEPTH = colorDepth_;

      if (width_ == 0) {
        width_ = P099_TS_X_RES; // default value
      }
      addFormNumericBox(F("Screen Width (px) (x)"), F("pwidth"), width_, 1, 65535);

      if (height_ == 0) {
        height_ = P099_TS_Y_RES; // default value
      }
      addFormNumericBox(F("Screen Height (px) (y)"), F("pheight"), height_, 1, 65535);

      AdaGFXFormRotation(F("protate"), rotation_);

      AdaGFXFormColorDepth(F("colordepth"), P099_COLOR_DEPTH, (colorDepth_ == 0));

      const bool bRotationFlipped = bitRead(P099_CONFIG_FLAGS, P099_FLAGS_ROTATION_FLIPPED);
      addFormCheckBox(F("Flip rotation 180&deg;"), F("protation_flipped"), bRotationFlipped);
      addFormNote(F("Some touchscreens are mounted 180&deg; rotated on the display."));

      addFormSubHeader(F("Touch configuration"));

      addFormNumericBox(F("Touch minimum pressure"), F("ptreshold"), P099_CONFIG_THRESHOLD, 0, 255);

      uint8_t choice3 = 0;
      bitWrite(choice3, P099_FLAGS_SEND_XY,         bitRead(P099_CONFIG_FLAGS, P099_FLAGS_SEND_XY));
      bitWrite(choice3, P099_FLAGS_SEND_Z,          bitRead(P099_CONFIG_FLAGS, P099_FLAGS_SEND_Z));
      bitWrite(choice3, P099_FLAGS_SEND_OBJECTNAME, bitRead(P099_CONFIG_FLAGS, P099_FLAGS_SEND_OBJECTNAME));
      {
        const __FlashStringHelper *options3[] =
        { F("None"),
          F("X and Y"),
          F("X, Y and Z"),
          F("Objectnames only"),
          F("Objectnames, X and Y"),
          F("Objectnames, X, Y and Z") };
        const int optionValues3[]    = { 0, 1, 3, 4, 5, 7 }; // Already used as a bitmap!
        constexpr size_t optionCount = NR_ELEMENTS(optionValues3);
        const FormSelectorOptions selector(optionCount, options3, optionValues3);
        selector.addFormSelector(F("Events"), F("pevents"), choice3);
      }

      if (!Settings.UseRules) {
        addFormNote(F("Tools / Advanced / Rules must be enabled for events to be fired."));
      }

      {
        P099_data_struct *P099_data = static_cast<P099_data_struct *>(getPluginTaskData(event->TaskIndex));
        bool deleteP099_data        = false;

        if (nullptr == P099_data) {
          P099_data       = new (std::nothrow) P099_data_struct();
          deleteP099_data = true;
        }

        if (nullptr == P099_data) {
          return success;
        }

        // P099_data->loadTouchObjects(event);

        success = P099_data->plugin_webform_load(event);

        if (deleteP099_data) {
          delete P099_data;
        }
      }

      // success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P099_CONFIG_VERSION = 2; // Storage layout changed to use ESPEasy_TouchHandler
      P099_SET_CONFIG_DISPLAY(P099_CONFIG_DISPLAY_PREV);
      P099_CONFIG_THRESHOLD = getFormItemInt(F("ptreshold"));
      P099_CONFIG_ROTATION  = getFormItemInt(F("protate"));
      P099_CONFIG_X_RES     = getFormItemInt(F("pwidth"));
      P099_CONFIG_Y_RES     = getFormItemInt(F("pheight"));
      #if P099_ENABLE_OLD_CONFIG
      P099_CONFIG_OBJECTCOUNT = getFormItemInt(F("pobjectcount"));

      if (P099_CONFIG_OBJECTCOUNT > P099_MaxObjectCount) { P099_CONFIG_OBJECTCOUNT = P099_MaxObjectCount; }
      #endif // if P099_ENABLE_OLD_CONFIG

      uint32_t lSettings = 0;
      bitWrite(lSettings, P099_FLAGS_SEND_XY,          bitRead(getFormItemInt(F("pevents")), P099_FLAGS_SEND_XY));
      bitWrite(lSettings, P099_FLAGS_SEND_Z,           bitRead(getFormItemInt(F("pevents")), P099_FLAGS_SEND_Z));
      bitWrite(lSettings, P099_FLAGS_SEND_OBJECTNAME,  bitRead(getFormItemInt(F("pevents")), P099_FLAGS_SEND_OBJECTNAME));
      bitWrite(lSettings, P099_FLAGS_USE_CALIBRATION,  getFormItemInt(F("puse_calibration")) == 1);
      bitWrite(lSettings, P099_FLAGS_LOG_CALIBRATION,  isFormItemChecked(F("plog_calibration")));
      bitWrite(lSettings, P099_FLAGS_ROTATION_FLIPPED, isFormItemChecked(F("protation_flipped")));
      P099_CONFIG_FLAGS = lSettings;

      {
        P099_data_struct *P099_data = nullptr; // static_cast<P099_data_struct *>(getPluginTaskData(event->TaskIndex));
        bool deleteP099_data        = false;

        if (nullptr == P099_data) {
          P099_data       = new (std::nothrow) P099_data_struct();
          deleteP099_data = true;
        }

        if (nullptr != P099_data) {
          success = P099_data->plugin_webform_save(event);

          if (deleteP099_data) {
            delete P099_data;
          }
        }
      }

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P099_data_struct());
      P099_data_struct *P099_data = static_cast<P099_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P099_data) && P099_data->init(event,
                                                          P099_CONFIG_CS_PIN,
                                                          P099_CONFIG_ROTATION,
                                                          bitRead(P099_CONFIG_FLAGS, P099_FLAGS_ROTATION_FLIPPED),
                                                          P099_CONFIG_THRESHOLD,
                                                          P099_CONFIG_X_RES,
                                                          P099_CONFIG_Y_RES);

      break;
    }

    case PLUGIN_EXIT:
    {
      success = true;
      break;
    }

    // case PLUGIN_READ: // Not implemented on purpose, *only* send out events/values when device is touched, and configured to send events

    case PLUGIN_WRITE:
    {
      P099_data_struct *P099_data = static_cast<P099_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P099_data) {
        success = P099_data->plugin_write(event, string);
      }

      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      P099_data_struct *P099_data = static_cast<P099_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P099_data) {
        success = P099_data->plugin_fifty_per_second(event);
      }

      break;
    }

    case PLUGIN_GET_CONFIG_VALUE:
    {
      P099_data_struct *P099_data = static_cast<P099_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P099_data) {
        success = P099_data->plugin_get_config_value(event, string);
      }
      break;
    }
  } // switch(function)
  return success;
}   // Plugin_099

#endif // USES_P099
