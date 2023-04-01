#include "_Plugin_Helper.h"
#ifdef USES_P021

// #######################################################################################################
// #################################### Plugin 021: Level Control ########################################
// #######################################################################################################

// Changelog:
// 2023-03-13, tonhuisman: Add setting to invert the Output state
// 2022-08-22, tonhuisman: Add setting to auto-save a changed setting after x minutes, size optimizations, add PCONFIG defines
// 2021-12-29, tonhuisman: Add setting to enable/disable saving the settings when the Set Level value is changed using the config
//                         command
// 2021-12-28, tonhuisman: Avoid saving settings if no change on config command https://github.com/letscontrolit/ESPEasy/issues/3477,
//                         cleanup source, prevent crashing when hysteresis is 0.0, run Uncrustify source formatter,
//                         apply float/double math compare functions instead of regular comparisons

# include "src/Helpers/ESPEasy_math.h"
# include "src/Helpers/Rules_calculate.h"
# include "src/WebServer/ESPEasy_WebServer.h"

# define PLUGIN_021
# define PLUGIN_ID_021          21
# define PLUGIN_NAME_021        "Regulator - Level Control"
# define PLUGIN_VALUENAME1_021  "Output"

# define P021_CHECK_TASK          PCONFIG(0)
# define P021_CHECK_VALUE         PCONFIG(1)
# define P021_DONT_ALWAYS_SAVE    PCONFIG(2)
# define P021_INVERT_OUTPUT       PCONFIG(3)
# define P021_TRIGGER_LEVEL       PCONFIG_FLOAT(0)
# define P021_TRIGGER_HYSTERESIS  PCONFIG_FLOAT(1)
# define P021_TRIGGER_LAST_STORED PCONFIG_FLOAT(2)
# define P021_AUTOSAVE_TIMER      PCONFIG_ULONG(0)

boolean Plugin_021(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static uint8_t switchstate[TASKS_MAX];

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

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_021));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_output(F("Level low"));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P021_DONT_ALWAYS_SAVE = 1; // Do not save
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addRowLabel(F("Check Task"));
      addTaskSelect(F("ptask"), P021_CHECK_TASK);

      if (validTaskIndex(P021_CHECK_TASK)) {
        addRowLabel(F("Check Value"));
        addTaskValueSelect(F("pvalue"), P021_CHECK_VALUE, P021_CHECK_TASK);
      }

      addFormTextBox(F("Set Level"),  F("psetvalue"), toString(P021_TRIGGER_LEVEL),      8);

      addFormTextBox(F("Hysteresis"), F("physt"),     toString(P021_TRIGGER_HYSTERESIS), 8);

      addFormCheckBox(F("Invert Output"),                                               F("inv"),          P021_INVERT_OUTPUT == 1);

      // inverted flag!
      addFormCheckBox(F("Save 'Set Level' after change via <pre>config</pre> command"), F("psave_always"), P021_DONT_ALWAYS_SAVE == 0);
      # ifndef BUILD_NO_DEBUG
      addFormNote(F("Saving settings too often can wear out the flash chip on your ESP!"));
      # endif // ifndef BUILD_NO_DEBUG

      addFormNumericBox(F("Auto-save interval"), F("pautosave"), P021_AUTOSAVE_TIMER / 60, 0, 1440); // Present in minutes
      addUnit(F("minutes"));
      # ifndef BUILD_NO_DEBUG
      addFormNote(F("Interval to check if 'Set Level' is changed via <pre>config</pre> command and saves it. Max. 24h, 0 = Off"));
      # endif // ifndef BUILD_NO_DEBUG

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P021_CHECK_TASK          = getFormItemInt(F("ptask"));
      P021_CHECK_VALUE         = getFormItemInt(F("pvalue"));
      P021_DONT_ALWAYS_SAVE    = isFormItemChecked(F("psave_always")) ? 0 : 1; // inverted flag!
      P021_TRIGGER_LEVEL       = getFormItemFloat(F("psetvalue"));
      P021_TRIGGER_LAST_STORED = P021_TRIGGER_LEVEL;
      P021_TRIGGER_HYSTERESIS  = getFormItemFloat(F("physt"));
      P021_AUTOSAVE_TIMER      = getFormItemInt(F("pautosave")) * 60; // Store in seconds
      P021_INVERT_OUTPUT       = isFormItemChecked(F("inv")) ? 1 : 0;

      success = true;
      break;
    }

    case PLUGIN_SET_CONFIG:
    {
      String command = parseString(string, 1);

      if (equals(command, F("setlevel"))) {
        String value  = parseString(string, 2);
        double result = 0.0;

        if (!isError(Calculate(value, result))) {
          if (!essentiallyEqual(static_cast<double>(P021_TRIGGER_LEVEL), result)) { // Save only if different
            P021_TRIGGER_LEVEL = result;

            if (P021_DONT_ALWAYS_SAVE == 0) {                                       // save only if explicitly enabled
              P021_TRIGGER_LAST_STORED = P021_TRIGGER_LEVEL;
              SaveSettings();
            } else {
              UserVar.setUint32(event->TaskIndex, 2, 1);                     // Set flag for auto-save

              if ((P021_AUTOSAVE_TIMER > 0) &&                               // - Autosave is set
                  (P021_DONT_ALWAYS_SAVE != 0) &&                            // - Save is off
                  ((UserVar.getUint32(event->TaskIndex, 3) == 0u) ||         // - Timer not yet started or uninitialized
                   (UserVar.getUint32(event->TaskIndex, 3) > P021_AUTOSAVE_TIMER))) {
                UserVar.setUint32(event->TaskIndex, 3, P021_AUTOSAVE_TIMER); // Start timer
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
        string  = toString(P021_TRIGGER_LEVEL);
        success = true;
      }
      break;
    }

    case PLUGIN_INIT:
    {
      if (validGpio(CONFIG_PIN1)) {
        pinMode(CONFIG_PIN1, OUTPUT);
      }
      success = true;
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      // we're checking a var from another task, so calculate that basevar
      taskIndex_t TaskIndex = P021_CHECK_TASK;

      if (!validTaskIndex(TaskIndex)) {
        break;
      }
      uint8_t BaseVarIndex = TaskIndex * VARS_PER_TASK + P021_CHECK_VALUE;
      float   value        = UserVar[BaseVarIndex];
      uint8_t state        = switchstate[event->TaskIndex];

      // compare with threshold value
      bool  isZero             = essentiallyZero(P021_TRIGGER_HYSTERESIS);
      float valueLowThreshold  = P021_TRIGGER_LEVEL - (isZero ? 0.0f : (P021_TRIGGER_HYSTERESIS / 2.0f));
      float valueHighThreshold = P021_TRIGGER_LEVEL + (isZero ? 1.0f : (P021_TRIGGER_HYSTERESIS / 2.0f)); // Include setvalue on
                                                                                                          // 0-hysteresis

      if (!definitelyGreaterThan(value, valueLowThreshold)) {
        state = P021_INVERT_OUTPUT == 0 ? 1 : 0;
      }

      if (!definitelyLessThan(value, valueHighThreshold)) {
        state = P021_INVERT_OUTPUT == 0 ? 0 : 1;
      }

      if (state != switchstate[event->TaskIndex])
      {
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("LEVEL: State ");
          log += state;
          addLogMove(LOG_LEVEL_INFO, log);
        }
        switchstate[event->TaskIndex] = state;

        if (validGpio(CONFIG_PIN1)) {
          digitalWrite(CONFIG_PIN1, state);
        }
        UserVar[event->BaseVarIndex] = state;
        sendData(event);
      }

      success = true;
      break;
    }
    case PLUGIN_ONCE_A_SECOND:
    {
      // When set but not Save after change is set, don't want to save twice
      if ((P021_AUTOSAVE_TIMER > 0) && (P021_DONT_ALWAYS_SAVE != 0) &&
          (0 != UserVar.getUint32(event->TaskIndex, 3)) &&                                   // Only check if timer is running
          (UserVar.getUint32(event->TaskIndex, 3) <= P021_AUTOSAVE_TIMER)) {
        UserVar.setUint32(event->TaskIndex, 3, UserVar.getUint32(event->TaskIndex, 3) - 1u); // Count down per second

        if (UserVar.getUint32(event->TaskIndex, 3) == 0) {
          if ((UserVar.getUint32(event->TaskIndex, 2) != 0) &&
              !essentiallyEqual(P021_TRIGGER_LEVEL, P021_TRIGGER_LAST_STORED)) {
            addLogMove(LOG_LEVEL_INFO, F("LEVEL: Auto-saving changed 'Set Level'."));
            P021_TRIGGER_LAST_STORED = P021_TRIGGER_LEVEL;
            SaveSettings();
            UserVar.setUint32(event->TaskIndex, 2, 0);
          }
        }
      }
      success = true;
      break;
    }
  }
  return success;
}

#endif // USES_P021
