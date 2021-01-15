#ifdef USES_P099
//#######################################################################################################
//#################################### Plugin 099: XPT2046 TFT Touchscreen #################################
//#######################################################################################################

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
 * 2020-08-29 tonhuisman: Initial plugin, based on XPT2046_Touchscreen by Paul Stoffregen from https://github.com/PaulStoffregen/XPT2046_Touchscreen
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
#define PLUGIN_NAME_099       "Touch - XPT2046 on a TFT display [TESTING]"
#define PLUGIN_VALUENAME1_099 "X"
#define PLUGIN_VALUENAME2_099 "Y"
#define PLUGIN_VALUENAME3_099 "Z"

#include "_Plugin_Helper.h"
#include "src/PluginStructs/P099_data_struct.h"

#define P099_FLAGS_SEND_XY          0   // Set in P099_CONFIG_FLAGS
#define P099_FLAGS_SEND_Z           1   // Set in P099_CONFIG_FLAGS
#define P099_FLAGS_SEND_OBJECTNAME  2   // Set in P099_CONFIG_FLAGS
#define P099_FLAGS_USE_CALIBRATION  3   // Set in P099_CONFIG_FLAGS
#define P099_FLAGS_LOG_CALIBRATION  4   // Set in P099_CONFIG_FLAGS
#define P099_FLAGS_ROTATION_FLIPPED 5   // Set in P099_CONFIG_FLAGS

#define P099_CONFIG_STATE       PCONFIG(0)
#define P099_CONFIG_CS_PIN      PIN(0)
#define P099_CONFIG_TRESHOLD    PCONFIG(1)
#define P099_CONFIG_ROTATION    PCONFIG(2)
#define P099_CONFIG_X_RES       PCONFIG(3)
#define P099_CONFIG_Y_RES       PCONFIG(4)
#define P099_CONFIG_OBJECTCOUNT PCONFIG(5)
#define P099_CONFIG_DEBOUNCE_MS PCONFIG(6)
#define P099_CONFIG_FLAGS       PCONFIG_LONG(0)    // 0-31 flags

#define P099_VALUE_X UserVar[event->BaseVarIndex + 0]
#define P099_VALUE_Y UserVar[event->BaseVarIndex + 1]
#define P099_VALUE_Z UserVar[event->BaseVarIndex + 2]

#define P099_TS_TRESHOLD         15    // Treshold before the value is registered as a proper touch
#define P099_TS_ROTATION         2     // Rotation 0-3 = 0/90/180/270 degrees, compatible with TFT ILI9341
#define P099_TS_SEND_XY          true  // Enable X/Y events
#define P099_TS_SEND_Z           false // Disable Z events
#define P099_TS_SEND_OBJECTNAME  true  // Enable objectname events
#define P099_TS_USE_CALIBRATION  false // Disable calibration
#define P099_TS_LOG_CALIBRATION  true  // Enable calibration logging
#define P099_TS_ROTATION_FLIPPED false // Enable rotation flipped 180 deg.
#define P099_TS_X_RES            240   // Pixels, should match with the screen it is mounted on
#define P099_TS_Y_RES            320
#define P099_INIT_OBJECTCOUNT    8     // Initial setting
#define P099_DEBOUNCE_MILLIS     150   // Debounce delay for On/Off button function

#define P099_TOUCH_X_INVALID  4095 // When picking up spurious noise (or an open/not connected TS-CS pin), these are the values that turn up
#define P099_TOUCH_Y_INVALID  4095
#define P099_TOUCH_Z_INVALID  255


boolean Plugin_099(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number = PLUGIN_ID_099;
      Device[deviceCount].Type = DEVICE_TYPE_SPI;
      Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_TRIPLE;
      Device[deviceCount].Ports = 0;
      Device[deviceCount].PullUpOption = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption = false;
      Device[deviceCount].ValueCount = 3;
      Device[deviceCount].SendDataOption = false;
      Device[deviceCount].TimerOption = false;
      success = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_099);
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
      if(P099_CONFIG_STATE != 1) {
        P099_CONFIG_CS_PIN      = P099_TS_CS;
        P099_CONFIG_TRESHOLD    = P099_TS_TRESHOLD;
        P099_CONFIG_ROTATION    = P099_TS_ROTATION;
        P099_CONFIG_X_RES       = P099_TS_X_RES;
        P099_CONFIG_Y_RES       = P099_TS_Y_RES;
        P099_CONFIG_OBJECTCOUNT = P099_INIT_OBJECTCOUNT;
        P099_CONFIG_DEBOUNCE_MS = P099_DEBOUNCE_MILLIS;

        uint32_t lSettings = 0;
        bitWrite(lSettings, P099_FLAGS_SEND_XY,          P099_TS_SEND_XY);
        bitWrite(lSettings, P099_FLAGS_SEND_Z,           P099_TS_SEND_Z);
        bitWrite(lSettings, P099_FLAGS_SEND_OBJECTNAME,  P099_TS_SEND_OBJECTNAME);
        bitWrite(lSettings, P099_FLAGS_USE_CALIBRATION,  P099_TS_USE_CALIBRATION);
        bitWrite(lSettings, P099_FLAGS_LOG_CALIBRATION,  P099_TS_LOG_CALIBRATION);
        bitWrite(lSettings, P099_FLAGS_ROTATION_FLIPPED, P099_TS_ROTATION_FLIPPED);
        P099_CONFIG_FLAGS  = lSettings;
      }
      success = true;
      break;
    }
    case PLUGIN_WEBFORM_LOAD:
    {
      addFormSubHeader(F("Screen"));

      uint16_t width_ = P099_CONFIG_X_RES;
      if(width_ == 0) {
        width_ = P099_TS_X_RES; // default value
      }
      addFormNumericBox(F("Screen Width (px) (x)"), F("p099_width"), width_, 1, 65535);

      uint16_t height_ = P099_CONFIG_Y_RES;
      if(height_ == 0) {
        height_ = P099_TS_Y_RES; // default value
      }
      addFormNumericBox(F("Screen Height (px) (y)"), F("p099_height"), height_, 1, 65535);

      byte choice2 = P099_CONFIG_ROTATION;
      String options2[4] = { F("Normal"), F("+90&deg;"), F("+180&deg;"), F("+270&deg;") }; // Avoid unicode
      int optionValues2[4] = { 0, 1, 2, 3 }; // Rotation similar to the TFT ILI9341 rotation
      addFormSelector(F("Rotation"), F("p099_rotate"), 4, options2, optionValues2, choice2);

      bool bRotationFlipped = bitRead(P099_CONFIG_FLAGS, P099_FLAGS_ROTATION_FLIPPED);
      addFormCheckBox(F("Flip rotation 180&deg;"), F("p099_rotation_flipped"),  bRotationFlipped);
      addFormNote(F("Some touchscreens are mounted 180&deg; rotated on the display."));
      
      addFormSubHeader(F("Touch configuration"));

      byte treshold = P099_CONFIG_TRESHOLD;
      addFormNumericBox(F("Touch minimum pressure"), F("p099_treshold"), treshold, 0, 255);

      # define P099_EVENTS_OPTIONS 6
      byte choice3 = 0;
      bitWrite(choice3, P099_FLAGS_SEND_XY,         bitRead(P099_CONFIG_FLAGS, P099_FLAGS_SEND_XY));
      bitWrite(choice3, P099_FLAGS_SEND_Z,          bitRead(P099_CONFIG_FLAGS, P099_FLAGS_SEND_Z));
      bitWrite(choice3, P099_FLAGS_SEND_OBJECTNAME, bitRead(P099_CONFIG_FLAGS, P099_FLAGS_SEND_OBJECTNAME));
      String options3[P099_EVENTS_OPTIONS] = { F("None"), F("X and Y"), F("X, Y and Z"), F("Objectnames only"), F("Objectnames, X and Y"), F("Objectnames, X, Y and Z")};
      int optionValues3[P099_EVENTS_OPTIONS] = { 0, 1, 3, 4, 5, 7 }; // Already used as a bitmap!
      addFormSelector(F("Events"), F("p099_events"), P099_EVENTS_OPTIONS, options3, optionValues3, choice3);
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

        bool tbUseCalibration = bitRead(P099_CONFIG_FLAGS, P099_FLAGS_USE_CALIBRATION);
        String options4[2] = { F("No"), F("Yes") };
        int optionValues4[2] = { 0, 1 };
        int choice4 = tbUseCalibration ? 1 : 0;
        addFormSelector(F("Calibrate to screen resolution"), F("p099_use_calibration"), 2, options4, optionValues4, choice4, true);
        if (tbUseCalibration) {

          addRowLabel(F("Calibration"), F(""));
          html_table(F(""), false);  // Sub-table
          html_table_header(F(""));
          html_table_header(F("x"));
          html_table_header(F("y"));
          html_table_header(F(""));
          html_table_header(F("x"));
          html_table_header(F("y"));

          html_TR_TD();
          addHtml(F("Top-left"));
          html_TD();
          addNumericBox(F("p099_cal_tl_x"), P099_data->StoredSettings.Calibration.top_left.x, 0, 65535);
          html_TD();
          addNumericBox(F("p099_cal_tl_y"), P099_data->StoredSettings.Calibration.top_left.y, 0, 65535);
          html_TD();
          addHtml(F("Bottom-right"));
          html_TD();
          addNumericBox(F("p099_cal_br_x"), P099_data->StoredSettings.Calibration.bottom_right.x, 0, 65535);
          html_TD();
          addNumericBox(F("p099_cal_br_y"), P099_data->StoredSettings.Calibration.bottom_right.y, 0, 65535);

          html_end_table();
          addFormNote(F("All x/y values must be <> 0 to enable calibration."));
        }
        bool bEnableCalibrationLog = bitRead(P099_CONFIG_FLAGS, P099_FLAGS_LOG_CALIBRATION);
        addFormCheckBox(F("Enable logging for calibration"), F("p099_log_calibration"),  bEnableCalibrationLog);

        addFormSubHeader(F("Touch objects"));

        {
          if (P099_CONFIG_OBJECTCOUNT > P099_MaxObjectCount) P099_CONFIG_OBJECTCOUNT = P099_MaxObjectCount;
          byte choice5 = P099_CONFIG_OBJECTCOUNT;
          if (choice5 == 0) { // Uninitialized, so use default
            choice5 = P099_CONFIG_OBJECTCOUNT = P099_INIT_OBJECTCOUNT;
          }
          # define P099_OBJECTCOUNT_OPTIONS 6
          String options5[P099_OBJECTCOUNT_OPTIONS] = { F("None"), F("8"), F("16"), F("24"), F("32"), F("40") };
          int optionValues5[P099_OBJECTCOUNT_OPTIONS] = { -1, 8, 16, 24, 32, 40 };
          addFormSelector(F("# of objects"), F("p099_objectcount"), P099_OBJECTCOUNT_OPTIONS, options5, optionValues5, choice5, true);
        }
        if (P099_CONFIG_OBJECTCOUNT > -1) {

          addRowLabel(F("Object"), F(""));
          html_table(F(""), false);  // Sub-table
          html_table_header(F("&nbsp;#&nbsp;"));
          html_table_header(F("Objectname"));
          html_table_header(F("Top-left x"));
          html_table_header(F("Top-left y"));
          html_table_header(F("Bottom-right x"));
          html_table_header(F("Bottom-right y"));
          html_table_header(F("On/Off button"));
          html_table_header(F("Inverted"));

          for (int objectNr = 0; objectNr < P099_CONFIG_OBJECTCOUNT; objectNr++) {
            html_TR_TD();
            addHtml(F("&nbsp;"));
            addHtmlInt(objectNr + 1);
            html_TD();
            addTextBox(getPluginCustomArgName(objectNr),
                      String(P099_data->StoredSettings.TouchObjects[objectNr].objectname),
                      P099_MaxObjectNameLength - 1,
                      false, false, F(""), F(""));
            html_TD();
            addNumericBox(getPluginCustomArgName(objectNr + 100), P099_data->StoredSettings.TouchObjects[objectNr].top_left.x,     0, 65535);
            html_TD();
            addNumericBox(getPluginCustomArgName(objectNr + 200), P099_data->StoredSettings.TouchObjects[objectNr].top_left.y,     0, 65535);
            html_TD();
            addNumericBox(getPluginCustomArgName(objectNr + 300), P099_data->StoredSettings.TouchObjects[objectNr].bottom_right.x, 0, 65535);
            html_TD();
            addNumericBox(getPluginCustomArgName(objectNr + 400), P099_data->StoredSettings.TouchObjects[objectNr].bottom_right.y, 0, 65535);
            html_TD();
            addCheckBox(  getPluginCustomArgName(objectNr + 500), bitRead(P099_data->StoredSettings.TouchObjects[objectNr].flags, P099_FLAGS_ON_OFF_BUTTON), false);
            html_TD();
            addCheckBox(  getPluginCustomArgName(objectNr + 600), bitRead(P099_data->StoredSettings.TouchObjects[objectNr].flags, P099_FLAGS_INVERT_BUTTON), false);
          }
          html_end_table();
          addFormNote(F("Start objectname with '_' to ignore/disable the object (temporarily)."));

          byte debounce = P099_CONFIG_DEBOUNCE_MS;
          addFormNumericBox(F("Debounce delay for On/Off buttons"), F("p099_debounce"), debounce, 0, 255);
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
      P099_CONFIG_TRESHOLD    = getFormItemInt(F("p099_treshold"));
      P099_CONFIG_ROTATION    = getFormItemInt(F("p099_rotate"));
      P099_CONFIG_X_RES       = getFormItemInt(F("p099_width"));
      P099_CONFIG_Y_RES       = getFormItemInt(F("p099_height"));
      P099_CONFIG_OBJECTCOUNT = getFormItemInt(F("p099_objectcount"));
      if (P099_CONFIG_OBJECTCOUNT > P099_MaxObjectCount) P099_CONFIG_OBJECTCOUNT = P099_MaxObjectCount;

      uint32_t lSettings = 0;
      bitWrite(lSettings, P099_FLAGS_SEND_XY,          bitRead(getFormItemInt(F("p099_events")), P099_FLAGS_SEND_XY));
      bitWrite(lSettings, P099_FLAGS_SEND_Z,           bitRead(getFormItemInt(F("p099_events")), P099_FLAGS_SEND_Z));
      bitWrite(lSettings, P099_FLAGS_SEND_OBJECTNAME,  bitRead(getFormItemInt(F("p099_events")), P099_FLAGS_SEND_OBJECTNAME));
      bitWrite(lSettings, P099_FLAGS_USE_CALIBRATION,  getFormItemInt(F("p099_use_calibration")) == 1);
      bitWrite(lSettings, P099_FLAGS_LOG_CALIBRATION,  isFormItemChecked(F("p099_log_calibration")));
      bitWrite(lSettings, P099_FLAGS_ROTATION_FLIPPED, isFormItemChecked(F("p099_rotation_flipped")));
      P099_CONFIG_FLAGS  = lSettings;

      P099_data_struct *P099_data = new (std::nothrow) P099_data_struct();

      if (nullptr == P099_data) {
        return success; // Save other settings even though this didn't initialize properly
      }
      P099_data->StoredSettings.Calibration.top_left.x     = getFormItemInt(F("p099_cal_tl_x"));
      P099_data->StoredSettings.Calibration.top_left.y     = getFormItemInt(F("p099_cal_tl_y"));
      P099_data->StoredSettings.Calibration.bottom_right.x = getFormItemInt(F("p099_cal_br_x"));
      P099_data->StoredSettings.Calibration.bottom_right.y = getFormItemInt(F("p099_cal_br_y"));

      String error;

      for (int objectNr = 0; objectNr < P099_CONFIG_OBJECTCOUNT; objectNr++) {
        if (!safe_strncpy(P099_data->StoredSettings.TouchObjects[objectNr].objectname, web_server.arg(getPluginCustomArgName(objectNr)), P099_MaxObjectNameLength)) {
          error += getCustomTaskSettingsError(objectNr);
        }
        P099_data->StoredSettings.TouchObjects[objectNr].objectname[P099_MaxObjectNameLength - 1] = 0; // Terminate in case of uninitalized data
        if (!ExtraTaskSettings.checkInvalidCharInNames(&P099_data->StoredSettings.TouchObjects[objectNr].objectname[0])) { // Check for invalid characters in objectname
          error += F("Invalid character in objectname #");
          error += (objectNr + 1);
          error += F(". Do not use ',-+/*=^%!#[]{}()' or space.\n");
        }
        P099_data->StoredSettings.TouchObjects[objectNr].top_left.x =     getFormItemInt(getPluginCustomArgName(objectNr + 100));
        P099_data->StoredSettings.TouchObjects[objectNr].top_left.y =     getFormItemInt(getPluginCustomArgName(objectNr + 200));
        P099_data->StoredSettings.TouchObjects[objectNr].bottom_right.x = getFormItemInt(getPluginCustomArgName(objectNr + 300));
        P099_data->StoredSettings.TouchObjects[objectNr].bottom_right.y = getFormItemInt(getPluginCustomArgName(objectNr + 400));

        uint8_t flags = 0;
        bitWrite(flags, P099_FLAGS_ON_OFF_BUTTON,                         isFormItemChecked(getPluginCustomArgName(objectNr + 500)));
        bitWrite(flags, P099_FLAGS_INVERT_BUTTON,                         isFormItemChecked(getPluginCustomArgName(objectNr + 600)));
        P099_data->StoredSettings.TouchObjects[objectNr].flags =          flags;
      }
      if (P099_CONFIG_OBJECTCOUNT > 0) {
        P099_CONFIG_DEBOUNCE_MS = getFormItemInt(F("p099_debounce"));
      }
      if (error.length() > 0) {
        addHtmlError(error);
      }
#ifdef PLUGIN_099_DEBUG
      String log = F("p099_data save size: ");
      log += sizeof(P099_data->StoredSettings);
      addLog(LOG_LEVEL_INFO, log);
#endif // PLUGIN_099_DEBUG
      SaveCustomTaskSettings(event->TaskIndex, (uint8_t *)&(P099_data->StoredSettings), sizeof(P099_data->StoredSettings) /*+ sizeof(P099_data->TouchObjects)*/);
      delete P099_data;

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P099_data_struct());
      P099_data_struct *P099_data = static_cast<P099_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P099_data) {
        return success;
      }

      bool send_xy          = bitRead(P099_CONFIG_FLAGS, P099_FLAGS_SEND_XY);
      bool send_z           = bitRead(P099_CONFIG_FLAGS, P099_FLAGS_SEND_Z);
      bool useCalibration   = bitRead(P099_CONFIG_FLAGS, P099_FLAGS_USE_CALIBRATION);
      bool bRotationFlipped = bitRead(P099_CONFIG_FLAGS, P099_FLAGS_ROTATION_FLIPPED);

      if (!(P099_data->init(event->TaskIndex,
                            P099_CONFIG_CS_PIN,
                            P099_CONFIG_ROTATION,
                            bRotationFlipped,
                            P099_CONFIG_TRESHOLD,
                            send_xy,
                            send_z,
                            useCalibration,
                            P099_CONFIG_X_RES,
                            P099_CONFIG_Y_RES))) {
        clearPluginTaskData(event->TaskIndex);
        P099_data = nullptr;
        success = true;
      }
      break;
    }

    case PLUGIN_EXIT:
    {
      P099_data_struct *P099_data = static_cast<P099_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P099_data) {
        return success;
      }
      clearPluginTaskData(event->TaskIndex);
      P099_data = nullptr;
      success = true;

      break;
    }

    // case PLUGIN_READ: // Not implemented on purpose, *only* send out events/values when device is touched, and configured to send events

    case PLUGIN_WRITE:
    {
      String command = F("");
      String subcommand = F("");
      String arguments = F("");
      arguments.reserve(24);

      int argIndex = string.indexOf(',');
      if (argIndex) {
        command = parseString(string, 1);
        subcommand = parseString(string, 2);

        P099_data_struct *P099_data = static_cast<P099_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr == P099_data) {
          return success;
        }
        if (command.equals(F("touch"))) {
          if(subcommand.equals(F("rot"))) { // touch,rot,<0..3> : Set rotation to 0, 90, 180, 270 degrees
            arguments = parseString(string, 3);
            uint8_t rot_ = static_cast<uint8_t>(arguments.toInt() % 4);

            P099_data->setRotation(rot_);
            success = true;
          } else if (subcommand.equals(F("flip"))) { // touch,flip,<0|1> : Flip rotation by 0 or 180 degrees
            arguments = parseString(string, 3);
            bool flip_ = (arguments.toInt() > 0);

            P099_data->setRotationFlipped(flip_);
            success = true;
          } else if (subcommand.equals(F("enable"))) { // touch,enable,<objectName> : Enables a disabled objectname (with a leading underscore)
            arguments = parseString(string, 3);
            success = P099_data->setTouchObjectState(arguments, true, P099_CONFIG_OBJECTCOUNT);
          } else if (subcommand.equals(F("disable"))) { // touch,disable,<objectName> : Disables an enabled objectname (without a leading underscore)
            arguments = parseString(string, 3);
            success = P099_data->setTouchObjectState(arguments, false, P099_CONFIG_OBJECTCOUNT);
          }
        }
      }
      break;
    }

    case PLUGIN_TEN_PER_SECOND:   // Should be often/fast enough, as this is user-interaction
    {
      P099_data_struct *P099_data = static_cast<P099_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P099_data) {
        return success;
      }
      if (P099_data->isInitialized()) {
        if (P099_data->touched()) {
          uint16_t x, y, rx, ry;
          uint8_t z;
          P099_data->readData(&x, &y, &z);
          if (!(x >= P099_TOUCH_X_INVALID || y >= P099_TOUCH_Y_INVALID || z == P099_TOUCH_Z_INVALID || z <= P099_CONFIG_TRESHOLD)) {

            rx = x;
            ry = y;
            P099_data->scaleRawToCalibrated(x, y);  // Map to screen coordinates if so configured

            P099_VALUE_X = x;
            P099_VALUE_Y = y;
            P099_VALUE_Z = z;

            bool bEnableCalibrationLog = bitRead(P099_CONFIG_FLAGS, P099_FLAGS_LOG_CALIBRATION);
            if (bEnableCalibrationLog && loglevelActiveFor(LOG_LEVEL_INFO)) { // REQUIRED for calibration and setting up objects, so do not make this optional!
              String log;
              log.reserve(72);
              log = F("Touch calibration rx= "); // Space before the logged values was added for readability
              log += rx;
              log += F(", ry= ");
              log += ry;
              log += F("; z= "); // Always log the z value even if not used.
              log += z;
              log += F(", x= ");
              log += x;
              log += F(", y= ");
              log += y;
              addLog(LOG_LEVEL_INFO, log);
            }

            if (Settings.UseRules) { // No events to handle if rules not enabled
              if (bitRead(P099_CONFIG_FLAGS, P099_FLAGS_SEND_XY)) {   // Send events for each touch
                const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(event->TaskIndex);
                if (!bitRead(P099_CONFIG_FLAGS, P099_FLAGS_SEND_Z) && validDeviceIndex(DeviceIndex)) {   // Do NOT send a Z event for each touch?
                  Device[DeviceIndex].VType = Sensor_VType::SENSOR_TYPE_DUAL;
                  Device[DeviceIndex].ValueCount = 2;
                }
                sendData(event); // Send X/Y(/Z) event
                if (!bitRead(P099_CONFIG_FLAGS, P099_FLAGS_SEND_Z) && validDeviceIndex(DeviceIndex)) {   // Reset device configuration
                  Device[DeviceIndex].VType = Sensor_VType::SENSOR_TYPE_TRIPLE;
                  Device[DeviceIndex].ValueCount = 3;
                }
              }
              if (bitRead(P099_CONFIG_FLAGS, P099_FLAGS_SEND_OBJECTNAME)) {   // Send events for objectname if within reach
                String selectedObjectName;
                int    selectedObjectIndex = -1;
                if (P099_data->isValidAndTouchedTouchObject(x, y, selectedObjectName, selectedObjectIndex, P099_CONFIG_OBJECTCOUNT)) {
                  if (selectedObjectIndex > -1 && bitRead(P099_data->StoredSettings.TouchObjects[selectedObjectIndex].flags, P099_FLAGS_ON_OFF_BUTTON)) {
                    if (P099_data->TouchTimers[selectedObjectIndex] == 0 || P099_data->TouchTimers[selectedObjectIndex] < millis() - (2 * P099_CONFIG_DEBOUNCE_MS)) { // Not touched yet or too long ago
                      P099_data->TouchTimers[selectedObjectIndex] = millis() + P099_CONFIG_DEBOUNCE_MS; // From now wait the debounce time
                    } else {
                      if (P099_data->TouchTimers[selectedObjectIndex] <= millis()) { // Debouncing time elapsed?
                        P099_data->TouchStates[selectedObjectIndex] = !P099_data->TouchStates[selectedObjectIndex];
                        P099_data->TouchTimers[selectedObjectIndex] = 0;
                        String eventCommand;
                        eventCommand.reserve(48);
                        eventCommand = getTaskDeviceName(event->TaskIndex);
                        eventCommand += '#';
                        eventCommand += selectedObjectName;
                        eventCommand += '='; // Add arguments
                        if (bitRead(P099_data->StoredSettings.TouchObjects[selectedObjectIndex].flags, P099_FLAGS_INVERT_BUTTON)) {
                          eventCommand += (P099_data->TouchStates[selectedObjectIndex] ? '0' : '1'); // Act like an inverted button, 0 = On, 1 = Off
                        } else {
                          eventCommand += (P099_data->TouchStates[selectedObjectIndex] ? '1' : '0'); // Act like a button, 1 = On, 0 = Off
                        }
                        eventQueue.add(eventCommand);
                      }
                    }
                  } else {
                    // Matching object is found, send <TaskDeviceName>#<ObjectName> event with x, y and z as %eventvalue1/2/3%
                    String eventCommand;
                    eventCommand.reserve(48);
                    eventCommand = getTaskDeviceName(event->TaskIndex);
                    eventCommand += '#';
                    eventCommand += selectedObjectName;
                    eventCommand += '='; // Add arguments
                    eventCommand += x;
                    eventCommand += ',';
                    eventCommand += y;
                    eventCommand += ',';
                    eventCommand += z;
                    eventQueue.add(eventCommand);
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
} // Plugin_099

#endif // USES_P099