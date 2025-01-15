#ifdef USES_P099

// #######################################################################################################
// #################################### Plugin 099: XPT2046 TFT Touchscreen #################################
// #######################################################################################################

/**
 * Changelog:
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

    case PLUGIN_SET_DEFAULTS:
    {
      // if already configured take it from settings, else use default values
      if (P099_CONFIG_STATE != 1) {
        P099_CONFIG_CS_PIN      = P099_TS_CS;
        P099_CONFIG_TRESHOLD    = P099_TS_TRESHOLD;
        P099_CONFIG_ROTATION    = P099_TS_ROTATION;
        P099_CONFIG_X_RES       = P099_TS_X_RES;
        P099_CONFIG_Y_RES       = P099_TS_Y_RES;
        P099_CONFIG_OBJECTCOUNT = P099_INIT_OBJECTCOUNT;
        P099_CONFIG_DEBOUNCE_MS = P099_DEBOUNCE_MILLIS;

        constexpr uint32_t lSettings = 0
                                       + (P099_TS_SEND_XY          ? (1 << P099_FLAGS_SEND_XY) : 0)
                                       + (P099_TS_SEND_Z           ? (1 << P099_FLAGS_SEND_Z) : 0)
                                       + (P099_TS_SEND_OBJECTNAME  ? (1 << P099_FLAGS_SEND_OBJECTNAME) : 0)
                                       + (P099_TS_USE_CALIBRATION  ? (1 << P099_FLAGS_USE_CALIBRATION) : 0)
                                       + (P099_TS_LOG_CALIBRATION  ? (1 << P099_FLAGS_LOG_CALIBRATION) : 0)
                                       + (P099_TS_ROTATION_FLIPPED ? (1 << P099_FLAGS_ROTATION_FLIPPED) : 0);
        P099_CONFIG_FLAGS = lSettings;
      }
      success = true;
      break;
    }
    case PLUGIN_WEBFORM_LOAD:
    {
      addFormSubHeader(F("Screen"));

      uint16_t width_ = P099_CONFIG_X_RES;

      if (width_ == 0) {
        width_ = P099_TS_X_RES; // default value
      }
      addFormNumericBox(F("Screen Width (px) (x)"), F("pwidth"), width_, 1, 65535);

      uint16_t height_ = P099_CONFIG_Y_RES;

      if (height_ == 0) {
        height_ = P099_TS_Y_RES; // default value
      }
      addFormNumericBox(F("Screen Height (px) (y)"), F("pheight"), height_, 1, 65535);

      {
        const uint8_t choice2                 = P099_CONFIG_ROTATION;
        const __FlashStringHelper *options2[] = { F("Normal"), F("+90&deg;"), F("+180&deg;"), F("+270&deg;") }; // Avoid unicode
        const int optionValues2[]             = { 0, 1, 2, 3 };                                                 // Rotation similar to the
                                                                                                                // TFT ILI9341 rotation
        constexpr size_t optionCount = NR_ELEMENTS(optionValues2);
        addFormSelector(F("Rotation"), F("protate"), optionCount, options2, optionValues2, choice2);
      }

      const bool bRotationFlipped = bitRead(P099_CONFIG_FLAGS, P099_FLAGS_ROTATION_FLIPPED);
      addFormCheckBox(F("Flip rotation 180&deg;"), F("protation_flipped"), bRotationFlipped);
      addFormNote(F("Some touchscreens are mounted 180&deg; rotated on the display."));

      addFormSubHeader(F("Touch configuration"));

      addFormNumericBox(F("Touch minimum pressure"), F("ptreshold"), P099_CONFIG_TRESHOLD, 0, 255);

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
        addFormSelector(F("Events"), F("pevents"), optionCount, options3, optionValues3, choice3);
      }

      if (!Settings.UseRules) {
        addFormNote(F("Tools / Advanced / Rules must be enabled for events to be fired."));
      }

      {
        P099_data_struct *P099_data = new (std::nothrow) P099_data_struct();

        if (nullptr == P099_data) {
          return success;
        }
        P099_data->loadTouchObjects(event->TaskIndex);

        addFormSubHeader(F("Calibration"));

        const bool tbUseCalibration = bitRead(P099_CONFIG_FLAGS, P099_FLAGS_USE_CALIBRATION);
        addFormSelector_YesNo(F("Calibrate to screen resolution"), F("puse_calibration"), tbUseCalibration ? 1 : 0, true);

        if (tbUseCalibration) {
          addRowLabel(F("Calibration"));
          html_table(EMPTY_STRING, false); // Sub-table
          html_table_header(F(""),  100);
          html_table_header(F("x"), 70);
          html_table_header(F("y"), 70);
          html_table_header(F(""),  100);
          html_table_header(F("x"), 70);
          html_table_header(F("y"), 70);

          html_TR_TD();
          addHtml(F("Top-left"));
          html_TD();
          addNumericBox(F("pcal_tl_x"), P099_data->StoredSettings.Calibration.top_left.x, 0, 65535);
          html_TD();
          addNumericBox(F("pcal_tl_y"), P099_data->StoredSettings.Calibration.top_left.y, 0, 65535);
          html_TD();
          addHtml(F("Bottom-right"));
          html_TD();
          addNumericBox(F("pcal_br_x"), P099_data->StoredSettings.Calibration.bottom_right.x, 0, 65535);
          html_TD();
          addNumericBox(F("pcal_br_y"), P099_data->StoredSettings.Calibration.bottom_right.y, 0, 65535);

          html_end_table();
          addFormNote(F("All x/y values must be <> 0 to enable calibration."));
        }
        const bool bEnableCalibrationLog = bitRead(P099_CONFIG_FLAGS, P099_FLAGS_LOG_CALIBRATION);
        addFormCheckBox(F("Enable logging for calibration"), F("plog_calibration"), bEnableCalibrationLog);

        addFormSubHeader(F("Touch objects"));

        {
          if (P099_CONFIG_OBJECTCOUNT > P099_MaxObjectCount) { P099_CONFIG_OBJECTCOUNT = P099_MaxObjectCount; }
          uint8_t choice5 = P099_CONFIG_OBJECTCOUNT;

          if (choice5 == 0) { // Uninitialized, so use default
            choice5 = P099_CONFIG_OBJECTCOUNT = P099_INIT_OBJECTCOUNT;
          }
          {
            const __FlashStringHelper *options5[] = { F("None"), F("8"), F("16"), F("24"), F("32"), F("40") };
            const int optionValues5[]             = { -1, 8, 16, 24, 32, 40 };
            constexpr size_t optionCount          = NR_ELEMENTS(optionValues5);
            addFormSelector(F("# of objects"), F("pobjectcount"), optionCount, options5, optionValues5, choice5, true);
          }
        }

        if (P099_CONFIG_OBJECTCOUNT > -1) {
          addRowLabel(F("Object"));
          html_table(EMPTY_STRING, false); // Sub-table
          html_table_header(F("&nbsp;#&nbsp;"),  30);
          html_table_header(F("Objectname"),     200);
          html_table_header(F("Top-left x"),     120);
          html_table_header(F("Top-left y"),     120);
          html_table_header(F("Bottom-right x"), 150);
          html_table_header(F("Bottom-right y"), 150);
          html_table_header(F("On/Off button"),  150);
          html_table_header(F("Inverted"),       120);

          for (int objectNr = 0; objectNr < P099_CONFIG_OBJECTCOUNT; ++objectNr) {
            html_TR_TD();
            addHtml(F("&nbsp;"));
            addHtmlInt(objectNr + 1);
            html_TD();
            addTextBox(getPluginCustomArgName(objectNr),
                       String(P099_data->StoredSettings.TouchObjects[objectNr].objectname),
                       P099_MaxObjectNameLength - 1,
                       false, false, EMPTY_STRING, F(""));
            html_TD();
            addNumericBox(getPluginCustomArgName(objectNr + 100), P099_data->StoredSettings.TouchObjects[objectNr].top_left.x, 0, 65535);
            html_TD();
            addNumericBox(getPluginCustomArgName(objectNr + 200), P099_data->StoredSettings.TouchObjects[objectNr].top_left.y, 0, 65535);
            html_TD();
            addNumericBox(getPluginCustomArgName(objectNr + 300), P099_data->StoredSettings.TouchObjects[objectNr].bottom_right.x, 0, 65535);
            html_TD();
            addNumericBox(getPluginCustomArgName(objectNr + 400), P099_data->StoredSettings.TouchObjects[objectNr].bottom_right.y, 0, 65535);
            html_TD();
            addCheckBox(getPluginCustomArgName(objectNr + 500),
                        bitRead(P099_data->StoredSettings.TouchObjects[objectNr].flags, P099_FLAGS_ON_OFF_BUTTON), false);
            html_TD();
            addCheckBox(getPluginCustomArgName(objectNr + 600),
                        bitRead(P099_data->StoredSettings.TouchObjects[objectNr].flags, P099_FLAGS_INVERT_BUTTON), false);
          }
          html_end_table();
          addFormNote(F("Start objectname with '_' to ignore/disable the object (temporarily)."));

          const uint8_t debounce = P099_CONFIG_DEBOUNCE_MS;
          addFormNumericBox(F("Debounce delay for On/Off buttons"), F("pdebounce"), debounce, 0, 255);
          addUnit(F("0-255 msec."));
        }
        delete P099_data;
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P099_CONFIG_STATE       = 1; // mark config as already saved (next time, will not use default values)
      P099_CONFIG_TRESHOLD    = getFormItemInt(F("ptreshold"));
      P099_CONFIG_ROTATION    = getFormItemInt(F("protate"));
      P099_CONFIG_X_RES       = getFormItemInt(F("pwidth"));
      P099_CONFIG_Y_RES       = getFormItemInt(F("pheight"));
      P099_CONFIG_OBJECTCOUNT = getFormItemInt(F("pobjectcount"));

      if (P099_CONFIG_OBJECTCOUNT > P099_MaxObjectCount) { P099_CONFIG_OBJECTCOUNT = P099_MaxObjectCount; }

      uint32_t lSettings = 0;
      bitWrite(lSettings, P099_FLAGS_SEND_XY,          bitRead(getFormItemInt(F("pevents")), P099_FLAGS_SEND_XY));
      bitWrite(lSettings, P099_FLAGS_SEND_Z,           bitRead(getFormItemInt(F("pevents")), P099_FLAGS_SEND_Z));
      bitWrite(lSettings, P099_FLAGS_SEND_OBJECTNAME,  bitRead(getFormItemInt(F("pevents")), P099_FLAGS_SEND_OBJECTNAME));
      bitWrite(lSettings, P099_FLAGS_USE_CALIBRATION,  getFormItemInt(F("puse_calibration")) == 1);
      bitWrite(lSettings, P099_FLAGS_LOG_CALIBRATION,  isFormItemChecked(F("plog_calibration")));
      bitWrite(lSettings, P099_FLAGS_ROTATION_FLIPPED, isFormItemChecked(F("protation_flipped")));
      P099_CONFIG_FLAGS = lSettings;

      P099_data_struct *P099_data = new (std::nothrow) P099_data_struct();

      if (nullptr == P099_data) {
        return success; // Save other settings even though this didn't initialize properly
      }
      P099_data->StoredSettings.Calibration.top_left.x     = getFormItemInt(F("pcal_tl_x"));
      P099_data->StoredSettings.Calibration.top_left.y     = getFormItemInt(F("pcal_tl_y"));
      P099_data->StoredSettings.Calibration.bottom_right.x = getFormItemInt(F("pcal_br_x"));
      P099_data->StoredSettings.Calibration.bottom_right.y = getFormItemInt(F("pcal_br_y"));

      String error;

      for (int objectNr = 0; objectNr < P099_CONFIG_OBJECTCOUNT; objectNr++) {
        if (!safe_strncpy(P099_data->StoredSettings.TouchObjects[objectNr].objectname, webArg(getPluginCustomArgName(objectNr)),
                          P099_MaxObjectNameLength)) {
          error += getCustomTaskSettingsError(objectNr);
        }
        P099_data->StoredSettings.TouchObjects[objectNr]
        .objectname[P099_MaxObjectNameLength - 1] = 0;                            // Terminate in case of uninitalized data

        if (!ExtraTaskSettings.checkInvalidCharInNames(
              &P099_data->StoredSettings.TouchObjects[objectNr].objectname[0])) { // Check for invalid characters in objectname
          error += strformat(F("Invalid character in objectname #%d. "
                               "Do not use ',-+/*=^%!#[]{}()' or space.\n"),
                             objectNr + 1);
        }
        P099_data->StoredSettings.TouchObjects[objectNr].top_left.x     = getFormItemIntCustomArgName(objectNr + 100);
        P099_data->StoredSettings.TouchObjects[objectNr].top_left.y     = getFormItemIntCustomArgName(objectNr + 200);
        P099_data->StoredSettings.TouchObjects[objectNr].bottom_right.x = getFormItemIntCustomArgName(objectNr + 300);
        P099_data->StoredSettings.TouchObjects[objectNr].bottom_right.y = getFormItemIntCustomArgName(objectNr + 400);

        uint8_t flags = 0;
        bitWrite(flags, P099_FLAGS_ON_OFF_BUTTON, isFormItemChecked(getPluginCustomArgName(objectNr + 500)));
        bitWrite(flags, P099_FLAGS_INVERT_BUTTON, isFormItemChecked(getPluginCustomArgName(objectNr + 600)));
        P099_data->StoredSettings.TouchObjects[objectNr].flags =          flags;
      }

      if (P099_CONFIG_OBJECTCOUNT > 0) {
        P099_CONFIG_DEBOUNCE_MS = getFormItemInt(F("pdebounce"));
      }

      if (error.length() > 0) {
        addHtmlError(error);
      }
      #ifdef PLUGIN_099_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLogMove(LOG_LEVEL_INFO, concat(F("P099 data save size: "), sizeof(P099_data->StoredSettings)));
      }
      #endif // PLUGIN_099_DEBUG
      SaveCustomTaskSettings(event->TaskIndex, reinterpret_cast<const uint8_t *>(&(P099_data->StoredSettings)),
                             sizeof(P099_data->StoredSettings));
      delete P099_data;

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P099_data_struct());
      P099_data_struct *P099_data = static_cast<P099_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P099_data) && P099_data->init(event->TaskIndex,
                                                          P099_CONFIG_CS_PIN,
                                                          P099_CONFIG_ROTATION,
                                                          bitRead(P099_CONFIG_FLAGS, P099_FLAGS_ROTATION_FLIPPED),
                                                          P099_CONFIG_TRESHOLD,
                                                          bitRead(P099_CONFIG_FLAGS, P099_FLAGS_SEND_XY),
                                                          bitRead(P099_CONFIG_FLAGS, P099_FLAGS_SEND_Z),
                                                          bitRead(P099_CONFIG_FLAGS, P099_FLAGS_USE_CALIBRATION),
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

    case PLUGIN_TEN_PER_SECOND: // Should be often/fast enough, as this is user-interaction
    {
      P099_data_struct *P099_data = static_cast<P099_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P099_data) {
        return success;
      }

      if (P099_data->isInitialized()) {
        if (P099_data->touched()) {
          uint16_t x, y, rx, ry;
          uint8_t  z;
          P099_data->readData(&x, &y, &z);

          if (!((x >= P099_TOUCH_X_INVALID) || (y >= P099_TOUCH_Y_INVALID) || (z == P099_TOUCH_Z_INVALID) || (z <= P099_CONFIG_TRESHOLD))) {
            rx = x;
            ry = y;
            P099_data->scaleRawToCalibrated(x, y); // Map to screen coordinates if so configured

            P099_SET_VALUE_X(x);
            P099_SET_VALUE_Y(y);
            P099_SET_VALUE_Z(z);

            bool bEnableCalibrationLog = bitRead(P099_CONFIG_FLAGS, P099_FLAGS_LOG_CALIBRATION);

            if (bEnableCalibrationLog && loglevelActiveFor(LOG_LEVEL_INFO)) {
              // REQUIRED for calibration and setting up objects, so do not
              // make this optional!
              // Space before the logged values was added for readability
              addLogMove(LOG_LEVEL_INFO, strformat(
                           F("Touch calibration rx= %u, ry= %u; z= %u, x= %u, y= %u"),
                           rx,
                           ry,
                           z, // Always log the z value even if not used.
                           x,
                           y));
            }

            if (Settings.UseRules) {                                                                   // No events to handle if rules not
                                                                                                       // enabled
              if (bitRead(P099_CONFIG_FLAGS, P099_FLAGS_SEND_XY)) {                                    // Send events for each touch
                const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(event->TaskIndex);

                if (!bitRead(P099_CONFIG_FLAGS, P099_FLAGS_SEND_Z) && validDeviceIndex(DeviceIndex)) { // Do NOT send a Z event for each
                                                                                                       // touch?
                  // FIXME TD-er: Should not change anything in the Device vector.
                  #ifdef ESP8266
                  Device[DeviceIndex].VType      = Sensor_VType::SENSOR_TYPE_DUAL;
                  Device[DeviceIndex].ValueCount = 2;
                  #else // ifdef ESP8266
                  Device.getDeviceStructForEdit(DeviceIndex).VType      = Sensor_VType::SENSOR_TYPE_DUAL;
                  Device.getDeviceStructForEdit(DeviceIndex).ValueCount = 2;
                  #endif // ifdef ESP8266
                }
                sendData(event);                                                                       // Send X/Y(/Z) event

                if (!bitRead(P099_CONFIG_FLAGS, P099_FLAGS_SEND_Z) && validDeviceIndex(DeviceIndex)) { // Reset device configuration
                  // FIXME TD-er: Should not change anything in the Device vector.
                  #ifdef ESP8266
                  Device[DeviceIndex].VType      = Sensor_VType::SENSOR_TYPE_TRIPLE;
                  Device[DeviceIndex].ValueCount = 3;
                  #else // ifdef ESP8266
                  Device.getDeviceStructForEdit(DeviceIndex).VType      = Sensor_VType::SENSOR_TYPE_TRIPLE;
                  Device.getDeviceStructForEdit(DeviceIndex).ValueCount = 3;
                  #endif // ifdef ESP8266
                }
              }

              if (bitRead(P099_CONFIG_FLAGS, P099_FLAGS_SEND_OBJECTNAME)) { // Send events for objectname if within reach
                String selectedObjectName;
                int    selectedObjectIndex = -1;

                if (P099_data->isValidAndTouchedTouchObject(x, y, selectedObjectName, selectedObjectIndex, P099_CONFIG_OBJECTCOUNT)) {
                  if ((selectedObjectIndex > -1) &&
                      bitRead(P099_data->StoredSettings.TouchObjects[selectedObjectIndex].flags, P099_FLAGS_ON_OFF_BUTTON)) {
                    if ((P099_data->TouchTimers[selectedObjectIndex] == 0) ||
                        (P099_data->TouchTimers[selectedObjectIndex] < millis() - (2 * P099_CONFIG_DEBOUNCE_MS))) { // Not touched yet or
                                                                                                                    // too long ago
                      P099_data->TouchTimers[selectedObjectIndex] = millis() + P099_CONFIG_DEBOUNCE_MS;             // From now wait the
                                                                                                                    // debounce time
                    } else {
                      if (P099_data->TouchTimers[selectedObjectIndex] <= millis()) {                                // Debouncing time
                                                                                                                    // elapsed?
                        P099_data->TouchStates[selectedObjectIndex] = !P099_data->TouchStates[selectedObjectIndex];
                        P099_data->TouchTimers[selectedObjectIndex] = 0;

                        bool eventValue = P099_data->TouchStates[selectedObjectIndex];

                        if (bitRead(P099_data->StoredSettings.TouchObjects[selectedObjectIndex].flags, P099_FLAGS_INVERT_BUTTON)) {
                          eventValue = !eventValue; // Act like an inverted button, 0 = On, 1 = Off
                        }
                        eventQueue.add(event->TaskIndex, selectedObjectName, eventValue);
                      }
                    }
                  } else {
                    // Matching object is found, send <TaskDeviceName>#<ObjectName> event with x, y and z as %eventvalue1/2/3%
                    eventQueue.add(
                      event->TaskIndex,
                      selectedObjectName,
                      strformat(F("%u,%u,%u"), x, y, z));
                  }
                }
              }
            }
            success = true;
          }
        }
      }
      break;
    }
  } // switch(function)
  return success;
}   // Plugin_099

#endif // USES_P099
