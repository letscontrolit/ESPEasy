#include "_Plugin_Helper.h"
#ifdef USES_P021

// #######################################################################################################
// #################################### Plugin 021: Level Control ########################################
// #######################################################################################################
// Digital output controller based upon a analog input compared with a setpoint with hysteresis
// Original intention to control a level by opening a valve or switching a pump
// Extended by timer based state control to support pumps with additional requirements (floor heating ciculation pump)

// Changelog:
// 2024-07-07, flashmark:  Reworked to support floor heating pump (added state machine control)
// 2023-03-13, tonhuisman: Add setting to invert the Output state
// 2022-08-22, tonhuisman: Add setting to auto-save a changed setting after x minutes, size optimizations, add PCONFIG defines
// 2021-12-29, tonhuisman: Add setting to enable/disable saving the settings when the Set Level value is changed using the config
//                         command
// 2021-12-28, tonhuisman: Avoid saving settings if no change on config command https://github.com/letscontrolit/ESPEasy/issues/3477,
//                         cleanup source, prevent crashing when hysteresis is 0.0, run Uncrustify source formatter,
//                         apply float/double math compare functions instead of regular comparisons

# include "src/Helpers/ESPEasy_math.h"
# include "src/Globals/RulesCalculate.h"
# include "src/WebServer/ESPEasy_WebServer.h"

# define PLUGIN_021_DEBUG

# define PLUGIN_021
# define PLUGIN_ID_021          21
# define PLUGIN_NAME_021        "Regulator - Level Control"
# define PLUGIN_VALUENAME1_021  "Output"
# define PLUGIN_VALUENAME2_021  "State"

# define P021_GPIO_RELAY            CONFIG_PIN1

// Define the configuration options using ESPeasy standard structures
// For identification on the HTML form use P021_GUID_<option>
// ------------------------------------------------------------------

// Input signal is read from another task (plugin) by accessing its TASK & VALUE
# define P021_CHECK_TASK            PCONFIG(0)
# define P021_GUID_CHECK_TASK       "f0"
# define P021_CHECK_VALUE           PCONFIG(1)
# define P021_GUID_CHECK_VALUE      "f1"

// Flag to prevent excessive writing. Backwards compatible with previous version
# define P021_DONT_ALWAYS_SAVE      PCONFIG(2)
# define P021_GUID_DONT_ALWAYS_SAVE "f2"

// Set of individual bits packed in a single PCONFIG(3)
// Use P021_getConfigBit() and P021_setConfigBit() to access the individual bits
// For backwards compatibility first bit shall be equal original P021_INVERT_OUTPUT
# define P021_FLAGS                 PCONFIG(3)
# define P021_INV_OUTPUT            0 // Invert output signal
# define P021_GUID_INV_OUTPUT       "b0"
# define P021_EXT_FUNCT             1 // Extended functionality on setup screen
# define P021_GUID_EXT_FUNCT        "b1"
# define P021_INV_INPUT             2 // Invert input comparator
# define P021_GUID_INV_INPUT        "b2"
# define P021_SYM_HYSTERESIS        3 // Use symetrical hysteresis
# define P021_GUID_SYM_HYSTERESIS   "b3"
# define P021_SLOW_EVAL             4 // 1Hz evaluation i.s.o. 10Hz
# define P021_GUID_SLOW_EVAL        "b4"
# define P021_STATE_OUTP            5 // Add state as additional output value
# define P021_GUID_STATE_OUTP       "b5"
# define P021_EXTEND_END            6 // Extend state timed from start
# define P021_GUID_EXTEND_END       "b6"

// Minimum time output shall be active [min]
# define P021_MIN_TIME              PCONFIG(4)
# define P021_GUID_MIN_TIME         "f4"

// Maximum time output may be idle [hour]
# define P021_INTERVAL_TIME         PCONFIG(5)
# define P021_GUID_INTERVAL_TIME    "f5"

// Duration for forced circulation [min]
# define P021_FORCE_TIME            PCONFIG(6)
# define P021_GUID_FORCE_TIME       "f6"

// Operation mode [enum]
# define P021_OPMODE                PCONFIG(7)
# define P021_GUID_OPMODE           "f7"

// Setpoint (set level)
# define P021_SETPOINT              PCONFIG_FLOAT(0)
# define P021_GUID_SETPOINT         "f8"

// Hysteresis
# define P021_HYSTERESIS            PCONFIG_FLOAT(1)
# define P021_GUID_HYSTERESIS       "f9"

// SETP_LAST_STORED is not available on form. Used as storage for autosave function
# define P021_SETP_LAST_STORED      PCONFIG_FLOAT(2)
# define P021_GUID_SETP_LAST_STORED "f10"
# define P021_AUTOSAVE_TIMER        PCONFIG_ULONG(0)
# define P021_GUID_AUTOSAVE_TIMER   "f11"

// Positions in userVar array for the output values of this plugin
// For now we only advertise output & state. Autosave bookkeeping is hidden and used as static storage
#define P021_VALUE_OUTPUT           0 // Switch output, logical state [inactive/active]
#define P021_VALUE_STATE            1 // Control state, see P021_control_state
#define P021_VALUE_AUTOSAVE_FLAG    2 // Autosave bookkeeping
#define P021_VALUE_AUTOSAVE_TIME    3 // Autosave bookkeeping

// Make time interpretation easier by converting millis to the longer periods we want to check
#define millis2seconds(x) ((x) / ((ulong)1000))
#define millis2minutes(x) ((x) / ((ulong)1000 * 60))
#define millis2hours(x) ((x) / ((ulong)1000 * 60 * 24))

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
#define P021_OPMODE_SIZE  6

// Control state for the control algorithm
enum P021_control_state
{
  P021_STATE_IDLE,    // Output is inactive
  P021_STATE_HEATING, // Output is active due to level control (based on input value)
  P021_STATE_EXTEND,  // Output is active due to minimum duration
  P021_STATE_FORCE    // Output is forced active due to maximum inactive duration is exceeded
};

// Static storage for global state info. Track per ESPeasy plugin instance
static uint8_t  P021_remote[TASKS_MAX];    // Static storage for remote control.
static uint32_t P021_timestamp[TASKS_MAX]; // Static storage for timestamp last change

boolean Plugin_021(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_021;
      Device[deviceCount].Type               = DEVICE_TYPE_SINGLE;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SWITCH;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = false;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_021);
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = P021_getConfigBit(P021_FLAGS, P021_STATE_OUTP) ? 2 : 1;
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
      P021_DONT_ALWAYS_SAVE = 1;                   // Do not save
      P021_GPIO_RELAY       = -1;                  // GPIO for relay output not assigned
      P021_CHECK_TASK       = -1;                  // No input source assigned
      P021_CHECK_VALUE      = -1;                  // No input source assigned
      P021_SETPOINT         = 25.0f;               // Switch output active above 25 [deg C]
      P021_HYSTERESIS       = 5.0f;                // Switch off hysteresis 5 [deg C] below switch on value
      P021_MIN_TIME         = 30;                  // Once switched on output should be active at least 30 [min]
      P021_INTERVAL_TIME    = 24;                  // Output shall run after 24 [hour] stand still
      P021_FORCE_TIME       = 5;                   // Forced circulation for 5 [min]
      P021_OPMODE           = P021_OPMODE_OFF;     // Don't touch output unless selected by operator
      P021_FLAGS            = 0;                   // Reset all flags
      UserVar.setFloat(event->TaskIndex, P021_VALUE_STATE, (float)P021_STATE_IDLE);
      P021_remote[event->TaskIndex]    = 0;        // Remote control state is "off"
      P021_timestamp[event->TaskIndex] = millis(); // Start counting time from now on
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
       #ifdef PLUGIN_021_DEBUG
      {
        P021_control_state control_state = (P021_control_state)UserVar.getFloat(event->TaskIndex, P021_VALUE_STATE);

        // Add some debug information
        String msg = F("State= ");
        msg += P021_printControlState(control_state);
        msg += (F(", Pump= "));
        msg += (int)UserVar.getFloat(event->TaskIndex, P021_VALUE_OUTPUT) == 1 ? F("on") : F("off");
        msg += F(", Remote= ");
        msg += P021_remote[event->TaskIndex];
        msg += F(", Timer= ");
        msg += (int)(millis2seconds(timePassedSince(P021_timestamp[event->TaskIndex])));
        addFormNote(msg);
      }
      #endif // ifdef PLUGIN_021_DEBUG

      addRowLabel(F("Input Task"));
      addTaskSelect(F(P021_GUID_CHECK_TASK), P021_CHECK_TASK);

      if (validTaskIndex(P021_CHECK_TASK)) {
        addRowLabel(F("Input Value"));
        addTaskValueSelect(F(P021_GUID_CHECK_VALUE), P021_CHECK_VALUE, P021_CHECK_TASK);
      }

      addFormTextBox(F("Setpoint"),   F(P021_GUID_SETPOINT),   toString(P021_SETPOINT),   8);

      addFormTextBox(F("Hysteresis"), F(P021_GUID_HYSTERESIS), toString(P021_HYSTERESIS), 8);

      addFormCheckBox(F("Invert Output"), F(P021_GUID_INV_OUTPUT), P021_getConfigBit(P021_FLAGS, P021_INV_OUTPUT));

      // inverted flag!
      addFormCheckBox(F("Save 'Setpoint'/'Hysteresis' after change via <pre>config</pre> command"),
                      F(P021_GUID_DONT_ALWAYS_SAVE),
                      P021_DONT_ALWAYS_SAVE == 0);

      addFormNote(F("Saving settings too often can wear out the flash chip on your ESP!"));

      addFormNumericBox(F("Auto-save interval"), F(P021_GUID_AUTOSAVE_TIMER), P021_AUTOSAVE_TIMER / 60, 0, 1440); // Present in minutes
      addUnit(F("minutes"));

      addFormNote(F("Interval to check if settings are changed via <pre>config</pre> command and saves that. Max. 24h, 0 = Off"));

      // Settings extension for new operation modes. Will reload the page.
      addFormSelector_YesNo(F("Extended functionality"), F(P021_GUID_EXT_FUNCT), P021_getConfigBit(P021_FLAGS, P021_EXT_FUNCT), true);

      if (P021_getConfigBit(P021_FLAGS, P021_EXT_FUNCT))
      {
        // FormSelector with all operation mode options
        String options[P021_OPMODE_SIZE]      = { F("Classic"), F("Off"), F("Standby"), F("On"), F("Local"), F("Remote") };
        int    optionValues[P021_OPMODE_SIZE] =
        { P021_OPMODE_CLASSIC, P021_OPMODE_OFF, P021_OPMODE_STANDBY, P021_OPMODE_ON, P021_OPMODE_TEMP, P021_OPMODE_REMOTE };
        addFormSelector(F("Control mode"), F(P021_GUID_OPMODE), P021_OPMODE_SIZE, options, optionValues, P021_OPMODE);

        // Minimum on time [min]
        addFormNumericBox(F("Minimum running time"), F(P021_GUID_MIN_TIME), P021_MIN_TIME, 0);
        addUnit(F("minute"));

        // Interval time (max idle time) [hour]
        addFormNumericBox(F("Maximum idle time"), F(P021_GUID_INTERVAL_TIME), P021_INTERVAL_TIME, 0);
        addUnit(F("hour"));

        // Interval circulation time (forced circulation) [min]
        addFormNumericBox(F("Forced circulation time"), F(P021_GUID_FORCE_TIME), P021_FORCE_TIME, 1);
        addUnit(F("minute"));

        // Symetrical/asymetrical hysteresis [checkbox]
        addFormCheckBox(F("Symetrical hysteresis"), F(P021_GUID_SYM_HYSTERESIS), P021_getConfigBit(P021_FLAGS, P021_SYM_HYSTERESIS));

        // Extension period time calculated from start or stop time [checkbox]
        addFormCheckBox(F("Extend at end"),         F(P021_GUID_EXTEND_END),     P021_getConfigBit(P021_FLAGS, P021_EXTEND_END));

        // Direction of input comparator ( normal = input>setpoint; invert = input<setpoint) [checkbox]
        addFormCheckBox(F("Invert Input"),          F(P021_GUID_INV_INPUT),      P021_getConfigBit(P021_FLAGS, P021_INV_INPUT));

        // Control evaluation at low speed (1Hz) or high speed (10Hz) [checkbox]
        addFormCheckBox(F("Slow evaluation"),       F(P021_GUID_SLOW_EVAL),      P021_getConfigBit(P021_FLAGS, P021_SLOW_EVAL));

        // Provide the controller state as second sensor output value [checkbox]
        addFormCheckBox(F("State as output value"), F(P021_GUID_STATE_OUTP),     P021_getConfigBit(P021_FLAGS, P021_STATE_OUTP));
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P021_CHECK_TASK       = getFormItemInt(F(P021_GUID_CHECK_TASK));
      P021_CHECK_VALUE      = getFormItemInt(F(P021_GUID_CHECK_VALUE));
      P021_DONT_ALWAYS_SAVE = isFormItemChecked(F(P021_GUID_DONT_ALWAYS_SAVE)) ? 0 : 1; // inverted flag!
      P021_SETPOINT         = getFormItemFloat(F(P021_GUID_SETPOINT));
      P021_SETP_LAST_STORED = P021_SETPOINT;
      P021_HYSTERESIS       = getFormItemFloat(F(P021_GUID_HYSTERESIS));
      P021_AUTOSAVE_TIMER   = getFormItemInt(F(P021_GUID_AUTOSAVE_TIMER)) * 60; // Store in seconds
      P021_FLAGS            = P021_setConfigBit(P021_FLAGS, P021_INV_OUTPUT, isFormItemChecked(F(P021_GUID_INV_OUTPUT)));

      // Save extended parameters only when they are selected and are shown on the page
      if ((getFormItemInt(F(P021_GUID_EXT_FUNCT)) != 0) && (P021_getConfigBit(P021_FLAGS, P021_EXT_FUNCT)))
      {
        P021_OPMODE        = getFormItemInt(F(P021_GUID_OPMODE));
        P021_MIN_TIME      = getFormItemInt(F(P021_GUID_MIN_TIME));
        P021_INTERVAL_TIME = getFormItemInt(F(P021_GUID_INTERVAL_TIME));
        P021_FORCE_TIME    = getFormItemInt(F(P021_GUID_FORCE_TIME));
        P021_FLAGS         = P021_setConfigBit(P021_FLAGS, P021_INV_INPUT, isFormItemChecked(F(P021_GUID_INV_INPUT)));
        P021_FLAGS         = P021_setConfigBit(P021_FLAGS, P021_EXTEND_END, isFormItemChecked(F(P021_GUID_EXTEND_END)));
        P021_FLAGS         = P021_setConfigBit(P021_FLAGS, P021_SYM_HYSTERESIS, isFormItemChecked(F(P021_GUID_SYM_HYSTERESIS)));
        P021_FLAGS         = P021_setConfigBit(P021_FLAGS, P021_SLOW_EVAL, isFormItemChecked(F(P021_GUID_SLOW_EVAL)));
        P021_FLAGS         = P021_setConfigBit(P021_FLAGS, P021_STATE_OUTP, isFormItemChecked(F(P021_GUID_STATE_OUTP)));
      }

      // Set extended parameters to backwards compatible values when extension is disabled
      if (getFormItemInt(F(P021_GUID_EXT_FUNCT)) == 0)
      {
        P021_OPMODE = P021_OPMODE_CLASSIC;                                      // Switch to classic control algorithm
        P021_FLAGS  = P021_setConfigBit(P021_FLAGS, P021_INV_INPUT, false);     // Standard input direction
        P021_FLAGS  = P021_setConfigBit(P021_FLAGS, P021_SYM_HYSTERESIS, true); // Symetrical hysteresis
        P021_FLAGS  = P021_setConfigBit(P021_FLAGS, P021_SLOW_EVAL, false);     // 10Hz evaluation
        P021_FLAGS  = P021_setConfigBit(P021_FLAGS, P021_STATE_OUTP, false);    // Don't provide state as value
        // Keep al other extra settings, thay should not affect CLASSIC mode
      }

      // Update the extension flag last.
      P021_FLAGS = P021_setConfigBit(P021_FLAGS, P021_EXT_FUNCT, getFormItemInt(F(P021_GUID_EXT_FUNCT)) != 0);

      success = true;
      break;
    }

    case PLUGIN_SET_CONFIG:
    {
      const String command      = parseString(string, 1);
      const String value        = parseString(string, 2);
      const bool   isHysteresis = equals(command, F("sethysteresis"));
      const bool   isSetLevel   = equals(command, F("setlevel"));
      const bool   isRemote     = equals(command, F("remote"));

      if (isSetLevel || isHysteresis || isRemote) {
        ESPEASY_RULES_FLOAT_TYPE result{};

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

          if (isRemote) {
            P021_remote[event->TaskIndex] = (uint8_t)result;
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
      String command = parseString(string, 1);

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

      // parse string to extract the command
      String command = parseString(string, 1); // already converted to lowercase
      String subcmd  = parseString(string, 2);
      String value   = parseString(string, 3);
      String log     = F("P021 write: ");

      if (equals(command, F("levelcontrol"))) {
        if (equals(subcmd, F("remote"))) {
          int remote_value = P021_remote[event->TaskIndex];
          log += F(" levelcontrol");

          if (equals(value, F("on")))
          {
            log         += F(" on");
            remote_value = 1;
            success      = true;
          }
          else if (equals(value, F("off")))
          {
            log         += F(" off");
            remote_value = 0;
            success      = true;
          }
          else
          {
            log += F(" **no valid value**");
          }
          P021_remote[event->TaskIndex] = remote_value;
        }
        else
        {
          log += F(" **no valid command**");
        }
      }
      P021_evaluate(event);
      addLogMove(LOG_LEVEL_INFO, log);
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      if (!P021_getConfigBit(P021_FLAGS, P021_SLOW_EVAL))
      {
        P021_evaluate(event);
      }
      success = true;
      break;
    }
    case PLUGIN_ONCE_A_SECOND:
    {
      P021_check_autosafe(event); // This function relies on being called exactly once a second

      if (P021_getConfigBit(P021_FLAGS, P021_SLOW_EVAL))
      {
        P021_evaluate(event);
      }
      success = true;
      break;
    }
  }

  return success;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Convert control state to string for debugging purposes
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef PLUGIN_021_DEBUG
String P021_printControlState(int state)
{
  switch (state)
  {
    case P021_STATE_IDLE:     return F("Idle"); break;
    case P021_STATE_HEATING:  return F("Heating"); break;
    case P021_STATE_EXTEND:   return F("Extend"); break;
    case P021_STATE_FORCE:    return F("Force"); break;
    default:                  return F("***ERROR***"); break;
  }
}

#endif // ifdef PLUGIN_021_DEBUG

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Convert control state to string for debugging purposes
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef PLUGIN_021_DEBUG
String P021_printControlMode(int mode)
{
  switch (mode)
  {
    case P021_OPMODE_CLASSIC: return F("Classic"); break;
    case P021_OPMODE_OFF:     return F("Off"); break;
    case P021_OPMODE_STANDBY: return F("Standby"); break;
    case P021_OPMODE_ON:      return F("On"); break;
    case P021_OPMODE_TEMP:    return F("Local"); break;
    case P021_OPMODE_REMOTE:  return F("Remote"); break;
    default:                  return F("***ERROR***"); break;
  }
}

#endif // ifdef PLUGIN_021_DEBUG

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Check if updated control parameters should be saved to disk
// This function relies on being called once a second for time keeping
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void P021_check_autosafe(struct EventStruct *event)
{
  // When set but not Save after change is set, don't want to save twice
  if ((P021_AUTOSAVE_TIMER > 0) && (P021_DONT_ALWAYS_SAVE != 0) &&
      (UserVar.getUint32(event->TaskIndex, P021_VALUE_AUTOSAVE_TIME) > 0) &&                                    // Only check if timer is
                                                                                                                // running
      (UserVar.getUint32(event->TaskIndex, P021_VALUE_AUTOSAVE_TIME) <= P021_AUTOSAVE_TIMER))
  {
    UserVar.setUint32(event->TaskIndex, 3, UserVar.getUint32(event->TaskIndex, P021_VALUE_AUTOSAVE_TIME) - 1u); // Count down per second

    if (UserVar.getUint32(event->TaskIndex, P021_VALUE_AUTOSAVE_TIME) == 0) {
      if ((UserVar.getUint32(event->TaskIndex, P021_VALUE_AUTOSAVE_FLAG) != 0) &&
          !essentiallyEqual(P021_SETPOINT, P021_SETP_LAST_STORED)) {
        addLogMove(LOG_LEVEL_INFO, F("LEVEL: Auto-saving changed 'Set Level'."));
        P021_SETP_LAST_STORED = P021_SETPOINT;
        SaveSettings();
        UserVar.setUint32(event->TaskIndex, P021_VALUE_AUTOSAVE_FLAG, 0);
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main control algorithm
// Evaluation shall not be dependent on when or how often this function is called
// All stateful information is stored in the plugin context environment accessible through parameter event or
// stored in a static variable indexed by the TaskIndex
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void P021_evaluate(struct EventStruct *event)
{
  taskIndex_t TaskIndex                = P021_CHECK_TASK; // TaskIndex for supplying sensor plugin
  float value                          = 0.0f;            // Input value for control algorithm
  P021_control_state old_control_state = (P021_control_state)UserVar.getFloat(event->TaskIndex, P021_VALUE_STATE);
  P021_control_state new_control_state = old_control_state;
  bool  invert_input                   = P021_getConfigBit(P021_FLAGS, P021_INV_INPUT);
  bool  symetric_hyst                  = P021_getConfigBit(P021_FLAGS, P021_SYM_HYSTERESIS);
  bool  relay_output                   = UserVar.getFloat(event->TaskIndex, P021_VALUE_OUTPUT);
  bool  remote_state                   = P021_remote[event->TaskIndex] != 0;
  ulong timestamp                      = P021_timestamp[event->TaskIndex];

  // Get the control value from the external task. If task does not exist use the default
  if (validTaskIndex(TaskIndex))
  {
    value = UserVar.getFloat(TaskIndex, P021_CHECK_VALUE);
  }

  switch ((P021_opmode)(P021_OPMODE))
  {
    // Classic (original P021) stateless control using hysteresis only
    case P021_OPMODE_CLASSIC:

      if (P021_check_on(value, P021_SETPOINT, P021_HYSTERESIS, invert_input, symetric_hyst, remote_state))
      {
        new_control_state = P021_STATE_HEATING;
      }
      else if (P021_check_off(value, P021_SETPOINT, P021_HYSTERESIS, invert_input, symetric_hyst, remote_state))
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
      new_control_state = P021_STATE_HEATING;
      break;

    // Control is switched off with maintnenance interval enabled
    case P021_OPMODE_STANDBY:

      if (old_control_state == P021_STATE_IDLE) // Output was idling
      {
        if ((millis2hours(timePassedSince(timestamp)) >= (ulong)P021_INTERVAL_TIME) && ((ulong)P021_FORCE_TIME >= 1))
        {
          timestamp         = millis();
          new_control_state = P021_STATE_FORCE;
        }
      }
      else if (millis2minutes(timePassedSince(timestamp)) < (ulong)P021_FORCE_TIME)
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

          if (P021_check_on(value, P021_SETPOINT, P021_HYSTERESIS, invert_input, symetric_hyst, remote_state))
          {
            // Setpoint comparator or remote request to activate the output
            timestamp         = millis();
            new_control_state = P021_STATE_HEATING;
          }
          else if (millis2hours(timePassedSince(timestamp)) >= (ulong)P021_INTERVAL_TIME)
          {
            // Inactive for a long period, forced maintenance run
            timestamp         = millis();
            new_control_state = P021_STATE_FORCE;
          }
          break;

        // Output is active due to setpoint comparator request
        case P021_STATE_HEATING:

          if (P021_check_off(value, P021_SETPOINT, P021_HYSTERESIS, invert_input, symetric_hyst, remote_state))
          {
            if (millis2minutes(timePassedSince(timestamp)) >= (ulong)P021_MIN_TIME)
            {
              timestamp         = millis();
              new_control_state = P021_STATE_IDLE;
            }
            else
            {
              // Take timestamp if extending from stop condition otherwise keep timestamp from moment output was swiched
              if (P021_getConfigBit(P021_FLAGS, P021_EXTEND_END))
              {
                timestamp = millis();
              }
              new_control_state = P021_STATE_EXTEND;
            }
          }
          break;

        // Output is active to extend a started cycle untill minimum time exceeded
        case P021_STATE_EXTEND:

          if (P021_check_on(value, P021_SETPOINT, P021_HYSTERESIS, invert_input, symetric_hyst, remote_state))
          {
            new_control_state = P021_STATE_HEATING;
          }
          else if (millis2minutes(timePassedSince(timestamp)) >= (ulong)P021_MIN_TIME)
          {
            timestamp         = millis();
            new_control_state = P021_STATE_IDLE;
          }
          break;

        // Output was forced active when operation mode switched to temperature control
        case P021_STATE_FORCE:

          if (P021_check_on(value, P021_SETPOINT, P021_HYSTERESIS, invert_input, symetric_hyst, remote_state))
          {
            // Keep timestamp from moment pump was swiched on
            new_control_state = P021_STATE_HEATING;
          }
          else if (millis2minutes(timePassedSince(timestamp)) >= (ulong)P021_FORCE_TIME)
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
    case P021_STATE_HEATING:
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
    relay_output ^= P021_getConfigBit(P021_FLAGS, P021_INV_OUTPUT); // Invert when selected
    digitalWrite(P021_GPIO_RELAY, relay_output ? HIGH : LOW);
  }

  #ifdef PLUGIN_021_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("P021: Calculated State= ");
    log += P021_printControlState(new_control_state);
    log += F(" -> ");
    log += F("; relay= ");
    log += relay_output;
    log += F("; timer= ");
    log += timePassedSince(timestamp) / 1000;
    log += F("; mode= ");
    log += P021_printControlMode(P021_OPMODE);
    log += F("; value= ");
    log += value;
    log += F("; remote= ");
    log += remote_state;
    addLogMove(LOG_LEVEL_DEBUG, log);
  }
  #endif // ifdef PLUGIN_021_DEBUG

  // Write back updated persistant control data:
  // - The logical state of the output signal [on,OFF]
  // - The state of the internal state machine
  // - Timestamp for last change of the output signal
  // Note: the actual state of the output is not stored, it can be calculated from the control_state
  UserVar.setFloat(event->TaskIndex, P021_VALUE_OUTPUT, (float)(((P021_control_state)new_control_state == P021_STATE_IDLE) ? 0 : 1));
  UserVar.setFloat(event->TaskIndex, P021_VALUE_STATE,  (float)new_control_state);
  P021_timestamp[event->TaskIndex] = timestamp;

  if (new_control_state != old_control_state)
  {
    sendData(event);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Input evaluation to determine switching the control signal on
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool P021_check_on(float value, float setpoint, float hysteresis, bool invert, bool symetric, bool force)
{
  if (force)
  {
    return true;
  }

  if (symetric)
  {
    bool  isZero             = essentiallyZero(hysteresis);
    float valueLowThreshold  = setpoint - (isZero ? 0.0f : (hysteresis / 2.0f));
    float valueHighThreshold = setpoint + (isZero ? 1.0f : (hysteresis / 2.0f)); // Include setvalue on 0-hysteresis
    return invert ? !definitelyGreaterThan(value, valueLowThreshold) : !definitelyLessThan(value, valueHighThreshold);
  }
  else
  {
    return invert ? (value < setpoint) : (value > setpoint);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Input evaluation to determine switching the control signal off
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool P021_check_off(float value, float setpoint, float hysteresis, bool invert, bool symetric, bool force)
{
  if (force)
  {
    return false;
  }

  if (symetric)
  {
    bool  isZero             = essentiallyZero(hysteresis);
    float valueLowThreshold  = setpoint - (isZero ? 0.0f : (hysteresis / 2.0f));
    float valueHighThreshold = setpoint + (isZero ? 1.0f : (hysteresis / 2.0f)); // Include setvalue on 0-hysteresis
    return invert ? !definitelyLessThan(value, valueHighThreshold) : !definitelyGreaterThan(value, valueLowThreshold);
  }
  else
  {
    return invert ? (value > (setpoint + hysteresis)) : (value < (setpoint - hysteresis));
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Retrieve one bit flag from a set of flags stored in one UINT16
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
boolean P021_getConfigBit(uint16_t config, int pos)
{
  boolean res = ((config >> pos) & 0x0001) == 1;

  return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Set one bit flag in a set of flags sored in one UINT16
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t P021_setConfigBit(uint16_t config, int pos, boolean val)
{
  uint16_t mask = (0x0001 << pos);

  if (val)
  {
    config |= mask;
  }
  else
  {
    mask   ^= 0xFFFF;
    config &= mask;
  }
  return config;
}

#endif // USES_P021
