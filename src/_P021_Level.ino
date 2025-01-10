#include "_Plugin_Helper.h"
#ifdef USES_P021

// #######################################################################################################
// #################################### Plugin 021: Level Control ########################################
// #######################################################################################################
// Digital output controller based upon a analog input compared with a setpoint with hysteresis
// Original intention to control a level by opening a valve or switching a pump
// Extended by timer based state control to support pumps with additional requirements (floor heating ciculation pump)

// Changelog:
// 2024-12-14, tonhuisman: Move most defines to .h file to avoid compiler warnings, as Arduino doesn't support #ifdef in .ino files
//                         Format source using Uncrustify
//                         Remove unneeded includes
// 2024-07-07, flashmark:  Reworked to support floor heating pump (added state machine control)
// 2023-03-13, tonhuisman: Add setting to invert the Output state
// 2022-08-22, tonhuisman: Add setting to auto-save a changed setting after x minutes, size optimizations, add PCONFIG defines
// 2021-12-29, tonhuisman: Add setting to enable/disable saving the settings when the Set Level value is changed using the config
//                         command
// 2021-12-28, tonhuisman: Avoid saving settings if no change on config command https://github.com/letscontrolit/ESPEasy/issues/3477,
//                         cleanup source, prevent crashing when hysteresis is 0.0, run Uncrustify source formatter,
//                         apply float/double math compare functions instead of regular comparisons

// NOTE: Due to lack of flash memory for most ESP8266 builds the extensions are only available on a limited set of builds.
//       Code size and functionality is controlled by several build flags:
//       PLUGIN_021_DEBUG     When defined: Additional debugging available for logging and webform
//       P021_MIN_BUILD_SIZE  When defined: Leave out some less important functionality
//       FEATURE_P021_EXTRAS  0 : No additional functionality
//                            1 : New functionality per change 2024-07-07
//                            > : Reserved for future functionality improvements
//       Debugging is expected to be switched off when build size is limited by the other flags
//       Minimum build size is expected to be true when extras >=1
//////////////////////////////////////////////////////////////////////////////////////////////////////////

# include "src/Globals/RulesCalculate.h"

# include "src/PluginStructs/P021_data_struct.h"

# define PLUGIN_021
# define PLUGIN_ID_021          21
# define PLUGIN_NAME_021        "Regulator - Level Control"
# define PLUGIN_VALUENAME1_021  "Output"
# define PLUGIN_VALUENAME2_021  "State"

# if FEATURE_P021_EXTRAS >= 1
int millis2seconds(int32_t x) {
  return (uint)(x / 1000);
}

int32_t seconds2millis(int x) {
  return (uint32_t)(x * 1000);
}

# endif // if FEATURE_P021_EXTRAS >= 1

# ifndef P021_MIN_BUILD_SIZE
int minutes2seconds(int x) {
  return x * 60;
}

int hours2seconds(int x) {
  return x * 60 * 60;
}

int seconds2minutes(int x) {
  return x / 60;
}

int seconds2hours(int x) {
  return x / (60 * 60);
}

# endif // ifndef P021_MIN_BUILD_SIZE

// Operation modes for the control algorithm
enum P021_opmode
{
  P021_OPMODE_CLASSIC, // Original, stateless control
  P021_OPMODE_OFF,     // Output is fully shut down, no forced curculation
  P021_OPMODE_STANDBY, // Output is only switched on for forced maintenance runs
  P021_OPMODE_ON,      // Output is always switched on
  P021_OPMODE_TEMP,    // Control algorithm based on temperature only
  P021_OPMODE_REMOTE   // Both temperature and remote command can switch on Output
};

// Control state for the control algorithm
enum P021_control_state
{
  P021_STATE_IDLE,   // Output is inactive
  P021_STATE_ACTIVE, // Output is active due to level control (based on input value)
  P021_STATE_EXTEND, // Output is active due to minimum duration
  P021_STATE_FORCE   // Output is forced active due to maximum inactive duration is exceeded
};

// Static storage for global state info. Track per ESPeasy plugin instance
static bool P021_remote[TASKS_MAX]; // Static storage for remote control.
// Static storage for last change timestamp. Not required with extended features disabled
# if FEATURE_P021_EXTRAS >= 1
static uint32_t P021_timestamp[TASKS_MAX];
# endif // if FEATURE_P021_EXTRAS >= 1

///////////////////////////////////////////////////////////////////////////////
// ESPeasy main entry point for a plugin
///////////////////////////////////////////////////////////////////////////////
boolean Plugin_021(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_021;
      dev.Type           = DEVICE_TYPE_SINGLE;
      dev.VType          = Sensor_VType::SENSOR_TYPE_SWITCH;
      dev.ValueCount     = 1;
      dev.SendDataOption = true;
      dev.setPin1Direction(gpio_direction::gpio_output);
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_021);
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = bitRead(P021_FLAGS, P021_STATE_OUTP) ? 2 : 1;
      success     = true;
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_021));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_021));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_output(F("Output"));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P021_DONT_ALWAYS_SAVE = 1;               // Do not save
      P021_GPIO_RELAY       = -1;              // GPIO for relay output not assigned
      P021_CHECK_TASK       = -1;              // No input source assigned
      P021_CHECK_VALUE      = -1;              // No input source assigned
      P021_SETPOINT         = 25.0f;           // Switch output active above 25 [deg C]
      P021_HYSTERESIS       = 5.0f;            // Switch off hysteresis 5 [deg C] below switch on value
      P021_MIN_TIME         = 30 * 60;         // Once switched on output should be active at least 30 [min]
      P021_INTERVAL_TIME    = 24 * 60 * 60;    // Output shall run after 24 [hour] stand still
      P021_FORCE_TIME       = 5 * 30;          // Forced circulation for 5 [min]
      P021_OPMODE           = P021_OPMODE_OFF; // Don't touch output unless selected by operator
      P021_FLAGS            = 0;               // Reset all flags
      // UserVar.setFloat(event->TaskIndex, P021_VALUE_STATE, (float)P021_STATE_IDLE);
      // P021_remote[event->TaskIndex] = 0;       // Remote control state is "off"
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      # if FEATURE_P021_EXTRAS >= 1
      const uint16_t flags   = P021_FLAGS;                     // shortcut to read existing configuration flags
      const bool     extFunc = bitRead(flags, P021_EXT_FUNCT); // Extended functionality selected
      # endif // if FEATURE_P021_EXTRAS >= 1

      // For debugging purposes the webform provides some internal data
      # ifdef PLUGIN_021_DEBUG
      {
        const P021_control_state control_state = (P021_control_state)UserVar.getFloat(event->TaskIndex, P021_VALUE_STATE);

        // Add some debug information
        const String outpstring = essentiallyZero(UserVar.getFloat(event->TaskIndex, P021_VALUE_OUTPUT)) ? F("off") : F("on");
        addFormNote(strformat(F("State= %s, Output= %s, Remote= %d, Timer= %d sec"),
                              P021_printControlState(control_state),
                              outpstring.c_str(),
                              P021_remote[event->TaskIndex],
                              millis2seconds(timePassedSince(P021_timestamp[event->TaskIndex]))));
      }
      # endif // ifdef PLUGIN_021_DEBUG

      const taskIndex_t check_task = P021_CHECK_TASK; // Optimze reference
      addRowLabel(F("Input Task"));
      addTaskSelect(F(P021_GUID_CHECK_TASK), check_task);

      if (validTaskIndex(check_task)) {
        addRowLabel(F("Input Value"));
        addTaskValueSelect(F(P021_GUID_CHECK_VALUE), P021_CHECK_VALUE, check_task);
      }

      addFormTextBox(F("Setpoint"),   F(P021_GUID_SETPOINT),   toString(P021_SETPOINT),   8);

      addFormTextBox(F("Hysteresis"), F(P021_GUID_HYSTERESIS), toString(P021_HYSTERESIS), 8);

      addFormCheckBox(F("Invert Output"), F(P021_GUID_INV_OUTPUT), bitRead(P021_FLAGS, P021_INV_OUTPUT));

      // inverted flag!
      addFormCheckBox(F("Save 'Setpoint'/'Hysteresis' after change via <pre>config</pre> command"),
                      F(P021_GUID_DONT_ALWAYS_SAVE),
                      P021_DONT_ALWAYS_SAVE == 0);

      // # ifndef BUILD_NO_DEBUG
      addFormNote(F("Saving settings too often can wear out the flash chip on your ESP!"));

      // # endif // ifndef BUILD_NO_DEBUG

      addFormNumericBox(F("Auto-save interval"), F(P021_GUID_AUTOSAVE_TIMER), P021_AUTOSAVE_TIMER / 60, 0, 1440); // Present in minutes
      # ifndef P021_MIN_BUILD_SIZE
      addUnit(F("minutes"));

      addFormNote(F("Interval to check if settings are changed via <pre>config</pre> command and saves that. Max. 24h, 0 = Off"));
      # endif // ifndef P021_MIN_BUILD_SIZE

      // Next are settings introduced with the extension for state dependend features
      # if FEATURE_P021_EXTRAS >= 1

      // Settings extension for new operation modes. Will reload the page.
      addFormSelector_YesNo(F("Extended functionality"), F(P021_GUID_EXT_FUNCT), extFunc, true);

      if (extFunc)
      {
        #  ifndef P021_MIN_BUILD_SIZE

        // Selection of timer units. Will reload the page.
        addFormSelector_YesNo(F("Long time span"), F(P021_GUID_LONG_TIMER_UNIT), bitRead(flags, P021_LONG_TIMER_UNIT), true);
        #  endif // ifndef P021_MIN_BUILD_SIZE

        // FormSelector with all operation mode options
        const __FlashStringHelper *options[] = { F("Classic"), F("Off"), F("Standby"), F("On"), F("Local"), F("Remote") };
        const int optionValues[]             =
        { P021_OPMODE_CLASSIC, P021_OPMODE_OFF, P021_OPMODE_STANDBY, P021_OPMODE_ON, P021_OPMODE_TEMP, P021_OPMODE_REMOTE };
        constexpr size_t optionCount = NR_ELEMENTS(optionValues);
        addFormSelector(F("Control mode"), F(P021_GUID_OPMODE), optionCount, options, optionValues, P021_OPMODE);

        // Add timer values depending on build size
        //  - minimum build size: units are always in seconds; drop the units on the form
        //  - standard build size: units are either seconds or minutes/hours
        #  ifdef P021_MIN_BUILD_SIZE

        // Minimum on time
        addFormNumericBox(F("Minimum running time"),    F(P021_GUID_MIN_TIME),      P021_MIN_TIME,      0);

        // Interval time (max idle time)
        addFormNumericBox(F("Maximum idle time"),       F(P021_GUID_INTERVAL_TIME), P021_INTERVAL_TIME, 0);

        // Interval circulation time (forced circulation)
        addFormNumericBox(F("Forced circulation time"), F(P021_GUID_FORCE_TIME),    P021_FORCE_TIME,    1);

        #  else // ifdef P021_MIN_BUILD_SIZE
        uint32_t min_time      = P021_MIN_TIME;          // Value to display for minimum timer
        uint32_t interval_time = P021_INTERVAL_TIME;     // Value to display for max idling time
        uint32_t force_time    = P021_FORCE_TIME;        // Value for forces run

        const __FlashStringHelper *unit1 = F("seconds"); // use minutes or seconds
        const __FlashStringHelper *unit2 = F("seconds"); // use hours or seconds

        if (bitRead(flags, P021_LONG_TIMER_UNIT))
        {
          min_time      = seconds2minutes(P021_MIN_TIME);
          interval_time = seconds2hours(P021_INTERVAL_TIME);
          force_time    = seconds2minutes(P021_FORCE_TIME);
          unit1         = F("minutes");
          unit2         = F("hours");
        }

        // Minimum on time
        addFormNumericBox(F("Minimum running time"), F(P021_GUID_MIN_TIME), min_time, 0);
        addUnit(unit1);

        // Interval time (max idle time)
        addFormNumericBox(F("Maximum idle time"), F(P021_GUID_INTERVAL_TIME), interval_time, 0);
        addUnit(unit2);

        // Interval circulation time (forced circulation)
        addFormNumericBox(F("Forced circulation time"), F(P021_GUID_FORCE_TIME), force_time, 1);
        addUnit(unit1);
        #  endif // ifdef P021_MIN_BUILD_SIZE - else


        // Symetrical/asymetrical hysteresis [checkbox]
        addFormCheckBox(F("Symetrical hysteresis"), F(P021_GUID_SYM_HYSTERESIS), bitRead(flags, P021_SYM_HYSTERESIS));

        // Extension period time calculated from start or stop time [checkbox]
        addFormCheckBox(F("Extend at end"),         F(P021_GUID_EXTEND_END),     bitRead(flags, P021_EXTEND_END));

        // Direction of input comparator ( normal = input>setpoint; invert = input<setpoint) [checkbox]
        addFormCheckBox(F("Invert Input"),          F(P021_GUID_INV_INPUT),      bitRead(flags, P021_INV_INPUT));

        // Control evaluation at low speed (1Hz) or high speed (10Hz) [checkbox]
        addFormCheckBox(F("Slow evaluation"),       F(P021_GUID_SLOW_EVAL),      bitRead(flags, P021_SLOW_EVAL));

        // Provide the controller state as second sensor output value [checkbox]
        addFormCheckBox(F("State as output value"), F(P021_GUID_STATE_OUTP),     bitRead(flags, P021_STATE_OUTP));
      }
      # endif // FEATURE_P021_EXTRAS >= 1

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      uint16_t flags = P021_FLAGS; // Reduce expensive access to P021 to write new flags

      P021_CHECK_TASK       = getFormItemInt(F(P021_GUID_CHECK_TASK));
      P021_CHECK_VALUE      = getFormItemInt(F(P021_GUID_CHECK_VALUE));
      P021_DONT_ALWAYS_SAVE = isFormItemChecked(F(P021_GUID_DONT_ALWAYS_SAVE)) ? 0 : 1; // inverted flag!
      P021_SETPOINT         = getFormItemFloat(F(P021_GUID_SETPOINT));
      P021_SETP_LAST_STORED = P021_SETPOINT;
      P021_HYSTERESIS       = getFormItemFloat(F(P021_GUID_HYSTERESIS));
      P021_AUTOSAVE_TIMER   = getFormItemInt(F(P021_GUID_AUTOSAVE_TIMER)) * 60; // Store in seconds
      bitWrite(flags, (P021_INV_OUTPUT), isFormItemChecked(F(P021_GUID_INV_OUTPUT)));

      # if FEATURE_P021_EXTRAS >= 1
      const bool newExtFunct =  (getFormItemInt(F(P021_GUID_EXT_FUNCT)) != 0);

      // Save extended parameters only when they are selected and are shown on the page
      if (newExtFunct && (bitRead(flags, P021_EXT_FUNCT)))
      {
        P021_OPMODE = getFormItemInt(F(P021_GUID_OPMODE));
        #  ifndef P021_MIN_BUILD_SIZE
        const bool new_units = getFormItemInt(F(P021_GUID_LONG_TIMER_UNIT)) != 0;
        const bool old_units = bitRead(flags, P021_LONG_TIMER_UNIT) != 0;

        // Check if timer unit flag is stable to prevent misalignment
        if (new_units == old_units)
        {
          if (new_units) // Longer time units
          {
            P021_MIN_TIME      = minutes2seconds(getFormItemInt(F(P021_GUID_MIN_TIME)));
            P021_INTERVAL_TIME = hours2seconds(getFormItemInt(F(P021_GUID_INTERVAL_TIME)));
            P021_FORCE_TIME    = minutes2seconds(getFormItemInt(F(P021_GUID_FORCE_TIME)));
          }
          else
          {
            P021_MIN_TIME      = getFormItemInt(F(P021_GUID_MIN_TIME));
            P021_INTERVAL_TIME = getFormItemInt(F(P021_GUID_INTERVAL_TIME));
            P021_FORCE_TIME    = getFormItemInt(F(P021_GUID_FORCE_TIME));
          }
        }
        bitWrite(flags, P021_LONG_TIMER_UNIT, new_units);
        #  else // ifndef P021_MIN_BUILD_SIZE
        P021_MIN_TIME      = getFormItemInt(F(P021_GUID_MIN_TIME));
        P021_INTERVAL_TIME = getFormItemInt(F(P021_GUID_INTERVAL_TIME));
        P021_FORCE_TIME    = getFormItemInt(F(P021_GUID_FORCE_TIME));
        #  endif // ifndef P021_MIN_BUILD_SIZE

        bitWrite(flags, P021_INV_INPUT,      isFormItemChecked(F(P021_GUID_INV_INPUT)));
        bitWrite(flags, P021_EXTEND_END,     isFormItemChecked(F(P021_GUID_EXTEND_END)));
        bitWrite(flags, P021_SYM_HYSTERESIS, isFormItemChecked(F(P021_GUID_SYM_HYSTERESIS)));
        bitWrite(flags, P021_SLOW_EVAL,      isFormItemChecked(F(P021_GUID_SLOW_EVAL)));
        bitWrite(flags, P021_STATE_OUTP,     isFormItemChecked(F(P021_GUID_STATE_OUTP)));
      }

      // Set extended parameters to backwards compatible values when extension is disabled
      if (!newExtFunct)
      {
        P021_OPMODE = P021_OPMODE_CLASSIC;           // Switch to classic control algorithm
        bitWrite(flags, P021_INV_INPUT,      false); // Standard input direction
        bitWrite(flags, P021_SYM_HYSTERESIS, true);  // Symetrical hysteresis
        bitWrite(flags, P021_SLOW_EVAL,      false); // 10Hz evaluation
        bitWrite(flags, P021_STATE_OUTP,     false); // Don't provide state as value
        // Keep al other extra settings, thay should not affect CLASSIC mode
      }

      bitWrite(flags, P021_EXT_FUNCT, newExtFunct);
      # endif // if FEATURE_P021_EXTRAS >= 1

      P021_FLAGS = flags; // Don't forget to write back the new flags
      success    = true;
      break;
    }

    case PLUGIN_SET_CONFIG:
    {
      const String command      = parseString(string, 1);
      const String value        = parseString(string, 2);
      const bool   isHysteresis = equals(command, F("sethysteresis"));
      const bool   isSetLevel   = equals(command, F("setlevel"));
      ESPEASY_RULES_FLOAT_TYPE result{};

      # ifndef P021_MIN_BUILD_SIZE
      const bool isRemote = equals(command, F("remote"));

      if (isRemote) {
        P021_remote[event->TaskIndex] = !essentiallyZero(result);
      }
      # endif // ifndef P021_MIN_BUILD_SIZE

      if (isSetLevel || isHysteresis) {
        if (!isError(Calculate(value, result))) {
          bool isChanged = false;

          if (isSetLevel &&
              !essentiallyEqual(static_cast<ESPEASY_RULES_FLOAT_TYPE>(P021_SETPOINT), result)) { // Save only if different
            P021_SETPOINT = result;
            isChanged     = true;
          }

          if (isHysteresis &&
              !essentiallyEqual(static_cast<ESPEASY_RULES_FLOAT_TYPE>(P021_HYSTERESIS), result)) { // Save only if different
            P021_HYSTERESIS = result;
            isChanged       = true;
          }

          if (isChanged) {
            if (P021_DONT_ALWAYS_SAVE == 0) { // save only if explicitly enabled
              P021_SETP_LAST_STORED = P021_SETPOINT;
              SaveSettings();
            } else {
              UserVar.setUint32(event->TaskIndex, P021_VALUE_AUTOSAVE_FLAG, 1);                     // Set flag for auto-save

              if ((P021_AUTOSAVE_TIMER > 0) &&                                                      // - Autosave is set
                  (P021_DONT_ALWAYS_SAVE != 0) &&                                                   // - Save is off
                  ((UserVar.getUint32(event->TaskIndex, P021_VALUE_AUTOSAVE_TIME) == 0u) ||         // - Timer not yet started or
                                                                                                    // uninitialized
                   (UserVar.getUint32(event->TaskIndex, P021_VALUE_AUTOSAVE_TIME) > P021_AUTOSAVE_TIMER))) {
                UserVar.setUint32(event->TaskIndex, P021_VALUE_AUTOSAVE_TIME, P021_AUTOSAVE_TIMER); // Start timer
                # ifndef LIMIT_BUILD_SIZE
                addLogMove(LOG_LEVEL_INFO, F("LEVEL: Auto-save timer started."));
                # endif // ifndef LIMIT_BUILD_SIZE
              }
            }
          }
          success = true;
        }
      }
      break;
    }

    case PLUGIN_GET_CONFIG_VALUE:
    {
      const String command = parseString(string, 1);

      if (equals(command, F("getlevel"))) {
        string  = toString(P021_SETPOINT);
        success = true;
      }
      else
      if (equals(command, F("gethysteresis"))) {
        string  = toString(P021_HYSTERESIS);
        success = true;
      }
      break;
    }

    case PLUGIN_INIT:
    {
      if (validGpio(P021_GPIO_RELAY)) {
        pinMode(P021_GPIO_RELAY, OUTPUT);
      }

      // I am not sure we want to reset the state at every init.
      // UserVar seems to be persistent
      UserVar.setFloat(event->TaskIndex, P021_VALUE_STATE, (float)P021_STATE_IDLE);
      P021_remote[event->TaskIndex] = false;       // Remote control state is "off"
      # if FEATURE_P021_EXTRAS >= 1
      P021_timestamp[event->TaskIndex] = millis(); // Initialize last switching time
      # endif // if FEATURE_P021_EXTRAS >= 1
      P021_evaluate(event);                        // Calculate the new control outputs
      sendData(event);                             // Force an update event for the plugin
      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      // this case defines code to be executed when the plugin executes an action (command).
      // Commands can be accessed via rules or via http.
      // As an example, http://192.168.1.12//control?cmd=dothis
      // implies that there exists the comamnd "dothis"
      // Expected commands:
      // * levelcontrol,remote, [on|off]

      # if FEATURE_P021_EXTRAS >= 1

      // parse string to extract the command
      const String command  = parseString(string, 1); // already converted to lowercase
      const String subcmd   = parseString(string, 2);
      const bool   hasValue = !parseString(string, 3).isEmpty();

      if (equals(command, F("levelcontrol")))
      {
        if (equals(subcmd, F("remote")) && hasValue && ((event->Par2 == 0) || (event->Par2 == 1)))
        {
          P021_remote[event->TaskIndex] = event->Par2 != 0;
          #  ifndef P021_MIN_BUILD_SIZE
          addLogMove(LOG_LEVEL_INFO, concat(F("P021 write: levelcontrol remote="), P021_remote[event->TaskIndex] ? F("on") : F("off")));
          #  endif // ifndef P021_MIN_BUILD_SIZE
          success = true;
        }

        // If not successful rely upon ESPeasy framework to report the issue
      }
      P021_evaluate(event);
      # endif // ifndef P021_MIN_BUILD_SIZE
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      # if FEATURE_P021_EXTRAS >= 1

      if (!bitRead(P021_FLAGS, P021_SLOW_EVAL))
      # endif // if FEATURE_P021_EXTRAS >= 1
      {
        P021_evaluate(event);
      }
      success = true;
      break;
    }
    case PLUGIN_ONCE_A_SECOND:
    {
      P021_check_autosave(event); // This function relies on being called exactly once a second

      # if FEATURE_P021_EXTRAS >= 1

      if (bitRead(P021_FLAGS, P021_SLOW_EVAL))
      {
        P021_evaluate(event);
      }
      # endif // if FEATURE_P021_EXTRAS >= 1
      success = true;
      break;
    }
  }
  return success;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Convert control state to string for debugging purposes
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
# ifdef PLUGIN_021_DEBUG
const __FlashStringHelper* P021_printControlState(int state)
{
  switch (state)
  {
    case P021_STATE_IDLE:   return F("Idle");
    case P021_STATE_ACTIVE: return F("Active");
    case P021_STATE_EXTEND: return F("Extend");
    case P021_STATE_FORCE:  return F("Force");
    default:                return F("***ERROR***");
  }
}

# endif // ifdef PLUGIN_021_DEBUG

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Convert control state to string for debugging purposes
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
# ifdef PLUGIN_021_DEBUG
const __FlashStringHelper* P021_printControlMode(int mode)
{
  switch (mode)
  {
    case P021_OPMODE_CLASSIC: return F("Classic");
    case P021_OPMODE_OFF:     return F("Off");
    case P021_OPMODE_STANDBY: return F("Standby");
    case P021_OPMODE_ON:      return F("On");
    case P021_OPMODE_TEMP:    return F("Local");
    case P021_OPMODE_REMOTE:  return F("Remote");
    default:                  return F("***ERROR***");
  }
}

# endif // ifdef PLUGIN_021_DEBUG

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Check if updated control parameters should be saved to disk
// This function relies on being called once a second for time keeping
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void P021_check_autosave(struct EventStruct *event)
{
  uint32_t autosaveTime = UserVar.getUint32(event->TaskIndex, P021_VALUE_AUTOSAVE_TIME);

  // When set but not Save after change is set, don't want to save twice
  if ((P021_AUTOSAVE_TIMER > 0) && (P021_DONT_ALWAYS_SAVE != 0) &&
      (autosaveTime > 0) && // Only check if timer is running
      (autosaveTime <= P021_AUTOSAVE_TIMER))
  {
    autosaveTime--;         // Count down one unit [second]
    UserVar.setUint32(event->TaskIndex, 3, autosaveTime);

    if (autosaveTime == 0) {
      if ((UserVar.getUint32(event->TaskIndex, P021_VALUE_AUTOSAVE_FLAG) != 0) &&
          !essentiallyEqual(P021_SETPOINT, P021_SETP_LAST_STORED)) {
        # ifndef P021_MIN_BUILD_SIZE
        addLogMove(LOG_LEVEL_INFO, F("LEVEL: Auto-saving changed 'Set Level'."));
        # endif // ifndef P021_MIN_BUILD_SIZE
        P021_SETP_LAST_STORED = P021_SETPOINT;
        SaveSettings();
        UserVar.setUint32(event->TaskIndex, P021_VALUE_AUTOSAVE_FLAG, 0);
      }
    }
  }
}

# if FEATURE_P021_EXTRAS >= 1

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main control algorithm  -- Full feature set enabled
// Evaluation shall not be dependent on when or how often this function is called
// All stateful information is stored in the plugin context environment accessible through parameter event or
// stored in a static variable indexed by the TaskIndex
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void P021_evaluate(struct EventStruct *event)
{
  // Note: several const values are defined here to reduce the code size when accessing the value multiple times
  const taskIndex_t TaskIndex                = P021_CHECK_TASK; // TaskIndex for supplying sensor plugin
  float value                                = 0.0f;            // Input value for control algorithm
  const P021_control_state old_control_state = (P021_control_state)UserVar.getFloat(event->TaskIndex, P021_VALUE_STATE);
  P021_control_state new_control_state       = old_control_state;
  const bool invert_input                    = bitRead(P021_FLAGS, P021_INV_INPUT);
  const bool symetric_hyst                   = bitRead(P021_FLAGS, P021_SYM_HYSTERESIS);
  const bool extend_at_end                   = bitRead(P021_FLAGS, P021_EXTEND_END);
  bool relay_output                          = UserVar.getFloat(event->TaskIndex, P021_VALUE_OUTPUT);
  bool remote_state                          = P021_remote[event->TaskIndex];
  uint32_t    timestamp                      = P021_timestamp[event->TaskIndex];
  const bool  beyond_interval                = timePassedSince(timestamp) >= seconds2millis(P021_INTERVAL_TIME);
  const bool  beyond_force                   = timePassedSince(timestamp) >= seconds2millis(P021_FORCE_TIME);
  const bool  beyond_min_time                = timePassedSince(timestamp) >= seconds2millis(P021_MIN_TIME);
  const float hysteresis                     = P021_HYSTERESIS;
  const float setpoint                       = P021_SETPOINT;

  // Get the control value from the external task. If task does not exist use the default
  if (validTaskIndex(TaskIndex))
  {
    value = UserVar.getFloat(TaskIndex, P021_CHECK_VALUE);
  }

  switch ((P021_opmode)(P021_OPMODE))
  {
    // Classic (original P021) stateless control using hysteresis only
    case P021_OPMODE_CLASSIC:

      if (P021_check_on(value, setpoint, hysteresis, invert_input, symetric_hyst, remote_state))
      {
        new_control_state = P021_STATE_ACTIVE;
      }
      else if (P021_check_off(value, setpoint, hysteresis, invert_input, symetric_hyst, remote_state))
      {
        new_control_state = P021_STATE_IDLE;
      }
      break;

    // Control is switched off completely
    case P021_OPMODE_OFF:

      if (old_control_state != P021_STATE_IDLE)
      {
        timestamp         = millis();
        new_control_state = P021_STATE_IDLE;
      }
      break;

    // Control is always on
    case P021_OPMODE_ON:

      if (old_control_state == P021_STATE_IDLE)
      {
        timestamp = millis();
      }
      new_control_state = P021_STATE_ACTIVE;
      break;

    // Control is switched off with maintnenance interval enabled
    case P021_OPMODE_STANDBY:

      if (old_control_state == P021_STATE_IDLE) // Output was idling
      {
        if ((beyond_interval) && (P021_FORCE_TIME > 1))
        {
          timestamp         = millis();
          new_control_state = P021_STATE_FORCE;
        }
      }

      // Note any other state implies the output is active
      else if (!beyond_force)
      {
        // Output was active shorter than the forced on time
        new_control_state = P021_STATE_FORCE; // Keep running in state FORCE
      }
      else
      {
        timestamp         = millis();
        new_control_state = P021_STATE_IDLE; // Switch off
      }
      break;

    // Control based on temperature and optional remote control
    case P021_OPMODE_TEMP:
      remote_state = false; // Don't look at requests from remote systems
    // Continue with shared handling, break deliberately not used
    // ----------------------------------------------------------
    case P021_OPMODE_REMOTE:

      switch (old_control_state)
      {
        // Output is inactive
        case P021_STATE_IDLE:

          if (P021_check_on(value, setpoint, hysteresis, invert_input, symetric_hyst, remote_state))
          {
            // Setpoint comparator or remote request to activate the output
            timestamp         = millis();
            new_control_state = P021_STATE_ACTIVE;
          }
          else if (beyond_interval)
          {
            // Inactive for a long period, forced maintenance run
            timestamp         = millis();
            new_control_state = P021_STATE_FORCE;
          }
          break;

        // Output is active due to setpoint comparator request
        case P021_STATE_ACTIVE:

          if (P021_check_off(value, setpoint, hysteresis, invert_input, symetric_hyst, remote_state))
          {
            if ((beyond_min_time) && (!extend_at_end))
            {
              timestamp         = millis();
              new_control_state = P021_STATE_IDLE;
            }
            else
            {
              // Take timestamp if extending from stop condition otherwise keep timestamp from moment output was swiched
              if (extend_at_end)
              {
                timestamp = millis();
              }
              new_control_state = P021_STATE_EXTEND;
            }
          }
          break;

        // Output is active to extend a started cycle untill minimum time exceeded
        case P021_STATE_EXTEND:

          if (P021_check_on(value, setpoint, hysteresis, invert_input, symetric_hyst, remote_state))
          {
            new_control_state = P021_STATE_ACTIVE;
          }
          else if (beyond_min_time)
          {
            timestamp         = millis();
            new_control_state = P021_STATE_IDLE;
          }
          break;

        // Output was forced active when operation mode switched to temperature control
        case P021_STATE_FORCE:

          if (P021_check_on(value, setpoint, hysteresis, invert_input, symetric_hyst, remote_state))
          {
            // Keep timestamp from moment pump was swiched on
            new_control_state = P021_STATE_ACTIVE;
          }
          else if (beyond_force)
          {
            timestamp         = millis();
            new_control_state = P021_STATE_IDLE;
          }
          break;
      }
      break; // switch(opmode) case P021_OPMODE_TEMP, P021_OPMODE_REMOTE

    // Unexpected opmode, force to OPMODE_OFF
    default:
      P021_OPMODE = P021_OPMODE_OFF;
      break;
  } // switch P021_OPMODE

  // Calculate output state from the newly calculated control state
  switch (new_control_state)
  {
    case P021_STATE_IDLE:
      relay_output = false; // Relay output state
      break;
    case P021_STATE_ACTIVE:
      relay_output = true;  // Relay output state
      break;
    case P021_STATE_EXTEND:
      relay_output = true;  // Relay output state
      break;
    case P021_STATE_FORCE:
      relay_output = true;  // Relay output state
      break;
    default:                // unexpected state, switch pump off
      relay_output = false; // Relay output state
      break;
  }

  // Actuate the output pin taking output invert flag into account
  if (validGpio(P021_GPIO_RELAY))
  {
    relay_output ^= bitRead(P021_FLAGS, P021_INV_OUTPUT); // Invert when selected
    digitalWrite(P021_GPIO_RELAY, relay_output ? HIGH : LOW);
  }

  #  ifdef PLUGIN_021_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG))
  {
    addLogMove(LOG_LEVEL_DEBUG,
               strformat(F("P021: Calculated State= %s; GPIO= %d; timer= %d sec; mode= %s; value= %f; remote= %d"),
                         P021_printControlState(new_control_state),
                         relay_output,
                         millis2seconds(timePassedSince(timestamp)),
                         P021_printControlMode((P021_opmode)P021_OPMODE),
                         value,
                         remote_state));
  }
  #  endif // ifdef PLUGIN_021_DEBUG

  // Write back updated persistant control data:
  // - The logical state of the output signal [on,OFF]
  // - The state of the internal state machine
  // - Timestamp for last change of the output signal
  // Note: the actual state of the output is not stored, it can be calculated from the control_state
  // UserVar.setFloat(event->TaskIndex, P021_VALUE_OUTPUT, (float)(((P021_control_state)new_control_state == P021_STATE_IDLE) ? 0 : 1));
  UserVar.setFloat(event->TaskIndex, P021_VALUE_OUTPUT, relay_output ? 1.0f : 0.0f);
  UserVar.setFloat(event->TaskIndex, P021_VALUE_STATE,  (float)new_control_state);
  P021_timestamp[event->TaskIndex] = timestamp;

  if (new_control_state != old_control_state)
  {
    sendData(event);
  }
}

# else // if FEATURE_P021_EXTRAS >= 1

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main control algorithm -- no state dependend control (old behavior ony to prevent out of flash memory)
// Evaluation shall not be dependent on when or how often this function is called
// All stateful information is stored in the plugin context environment accessible through parameter event or
// stored in a static variable indexed by the TaskIndex
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void P021_evaluate(struct EventStruct *event)
{
  // Note: several const values are defined here to reduce the code size when accessing the value multiple times
  const taskIndex_t TaskIndex                = P021_CHECK_TASK; // TaskIndex for supplying sensor plugin
  float value                                = 0.0f;            // Input value for control algorithm
  const P021_control_state old_control_state = (P021_control_state)UserVar.getFloat(event->TaskIndex, P021_VALUE_STATE);
  P021_control_state new_control_state       = old_control_state;
  bool relay_output                          = UserVar.getFloat(event->TaskIndex, P021_VALUE_OUTPUT);

  const float hysteresis = P021_HYSTERESIS;
  const float setpoint   = P021_SETPOINT;

  // Get the control value from the external task. If task does not exist use the default
  if (validTaskIndex(TaskIndex))
  {
    value = UserVar.getFloat(TaskIndex, P021_CHECK_VALUE);
  }

  // The real calculation based upon current measurement, setpoint and hysteresis
  // All new goodies to default (backwards compatible with previous versions)
  if (P021_check_on(value, setpoint, hysteresis, false, true, false))
  {
    new_control_state = P021_STATE_ACTIVE;
  }
  else if (P021_check_off(value, setpoint, hysteresis, false, true, false))
  {
    new_control_state = P021_STATE_IDLE;
  }

  // Calculate output state from the newly calculated control state
  switch (new_control_state)
  {
    case P021_STATE_IDLE:
      relay_output = false; // Relay output state
      break;
    case P021_STATE_ACTIVE:
      relay_output = true;  // Relay output state
      break;
    default:                // unexpected state, switch pump off
      relay_output = false; // Relay output state
      break;
  }

  // Actuate the output pin taking output invert flag into account
  relay_output ^= bitRead(P021_FLAGS, P021_INV_OUTPUT); // Invert when selected

  if (validGpio(P021_GPIO_RELAY))
  {
    digitalWrite(P021_GPIO_RELAY, relay_output ? HIGH : LOW);
  }

  #  ifdef PLUGIN_021_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG))
  {
    addLogMove(LOG_LEVEL_DEBUG,
               strformat(F("P021: Calculated State= %s; GPIO= %d; value= %f"),
                         P021_printControlState(new_control_state),
                         relay_output,
                         value));
  }
  #  endif // ifdef PLUGIN_021_DEBUG

  // Write back updated persistant control data:
  // - The physical state of the output signal [on,OFF]
  // - The state of the internal state machine
  UserVar.setFloat(event->TaskIndex, P021_VALUE_OUTPUT, relay_output ? 1.0f : 0.0f);
  UserVar.setFloat(event->TaskIndex, P021_VALUE_STATE,  (float)new_control_state);

  if (new_control_state != old_control_state)
  {
    sendData(event);
  }
}

# endif // if FEATURE_P021_EXTRAS >= 1

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper function for P021_check_on() and P021_check_off() to reduce code size
// Return the threshold level for the symetrical hysteresis handling
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float P021_symetric_threshold(float setpoint, float hysteresis, bool invert) {
  const bool  isZero = essentiallyZero(hysteresis);
  const float delta  = (isZero ? 1.0f : (hysteresis / 2.0f));

  return setpoint + ((invert ? -1.0f : 1.0f) * delta);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Input evaluation to determine switching the control signal on
// Return true when controller output indicates activation (switching on). See documentation
//  Value       : measured value to be compared
//  setpoint    : Setpoint value
//  hysteresis  : Hysteresis value
//  invert      : Invert compare direction w.r.t. measured value
//  symetric    : Use symetric/asymetric hysteresis around the setpoint
//  force       : Flag to overrule the analog compare with hysteresis and activate the output
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool P021_check_on(float value, float setpoint, float hysteresis, bool invert, bool symetric, bool force)
{
  if (force) { return true; }
  const float Threshold = symetric
    ? P021_symetric_threshold(setpoint, hysteresis, invert)
    : setpoint;
  return invert ? definitelyLessThan(value, Threshold) : definitelyGreaterThan(value, Threshold);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Input evaluation to determine switching the control signal off
// Return true when controller output indicates deactivation (switching off). See documentation
//  Value       : measured value to be compared
//  setpoint    : Setpoint value
//  hysteresis  : Hysteresis value
//  invert      : Invert compare direction w.r.t. measured value
//  symetric    : Use symetric/asymetric hysteresis around the setpoint
//  force       : Flag to overrule the analog compare with hysteresis and activate the output
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool P021_check_off(float value, float setpoint, float hysteresis, bool invert, bool symetric, bool force)
{
  if (force) { return false; }
  const float Threshold = symetric
    ? P021_symetric_threshold(setpoint, hysteresis, !invert)
    : setpoint + ((invert ? 1.0f : -1.0f) * hysteresis);
  return invert ? definitelyGreaterThan(value, Threshold) : definitelyLessThan(value, Threshold);
}

#endif // USES_P021
