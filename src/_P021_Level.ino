#include "_Plugin_Helper.h"
#ifdef USES_P021

// #######################################################################################################
// #################################### Plugin 021: Level Control ########################################
// #######################################################################################################

// Changelog:
// 2021-12-29, tonhuisman: Add setting to enable/disable saving the settings when the Set Level value is changed using the config
//                         command
// 2021-12-28, tonhuisman: Avoid saving settings if no change on config command https://github.com/letscontrolit/ESPEasy/issues/3477,
//                         cleanup source, prevent crashing when hysteresis is 0.0, run Uncrustify source formatter,
//                         apply float/double math compare functions instead of regular comparisons

# include "src/Helpers/ESPEasy_math.h"
# include "src/Helpers/Rules_calculate.h"
# include "src/WebServer/WebServer.h"

# define PLUGIN_021
# define PLUGIN_ID_021          21
# define PLUGIN_NAME_021        "Regulator - Level Control"
# define PLUGIN_VALUENAME1_021  "Output"

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
      PCONFIG(2) = 1; // Do not save
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addRowLabel(F("Check Task"));
      addTaskSelect(F("p021_task"), PCONFIG(0));

      LoadTaskSettings(PCONFIG(0)); // we need to load the values from another task for selection!
      addRowLabel(F("Check Value"));
      addTaskValueSelect(F("p021_value"), PCONFIG(1), PCONFIG(0));

      addFormTextBox(F("Set Level"),  F("p021_setvalue"), String(PCONFIG_FLOAT(0)), 8);

      addFormTextBox(F("Hysteresis"), F("p021_hyst"),     String(PCONFIG_FLOAT(1)), 8);

      addFormCheckBox(F("Save 'Set Level' after change via <tt>config</tt> command"), F("p021_save_always"), PCONFIG(2) == 0); // inverted
                                                                                                                               // flag!
      addFormNote(F("Saving settings too often can wear out the flash chip on your ESP!"));

      // we need to restore our original taskvalues!
      LoadTaskSettings(event->TaskIndex);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0)       = getFormItemInt(F("p021_task"));
      PCONFIG(1)       = getFormItemInt(F("p021_value"));
      PCONFIG(2)       = isFormItemChecked(F("p021_save_always")) ? 0 : 1; // inverted flag!
      PCONFIG_FLOAT(0) = getFormItemFloat(F("p021_setvalue"));
      PCONFIG_FLOAT(1) = getFormItemFloat(F("p021_hyst"));

      success = true;
      break;
    }

    case PLUGIN_SET_CONFIG:
    {
      String command = parseString(string, 1);

      if (command == F("setlevel")) {
        String value  = parseString(string, 2);
        double result = 0.0;

        if (!isError(Calculate(value, result))) {
          if (!essentiallyEqual(static_cast<double>(PCONFIG_FLOAT(0)), result)) { // Save only if different
            PCONFIG_FLOAT(0) = result;

            if (PCONFIG(2) == 0) {                                                // save only if explicitly enabled
              SaveSettings();
            }
          }
          success = true;
        }
      }
      break;
    }

    case PLUGIN_GET_CONFIG:
    {
      String command = parseString(string, 1);

      if (command == F("getlevel")) {
        string  = PCONFIG_FLOAT(0);
        success = true;
      }
      break;
    }

    case PLUGIN_INIT:
    {
      pinMode(CONFIG_PIN1, OUTPUT);
      success = true;
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      // we're checking a var from another task, so calculate that basevar
      taskIndex_t TaskIndex    = PCONFIG(0);
      uint8_t     BaseVarIndex = TaskIndex * VARS_PER_TASK + PCONFIG(1);
      float   value            = UserVar[BaseVarIndex];
      uint8_t state            = switchstate[event->TaskIndex];

      // compare with threshold value
      bool  isZero             = essentiallyEqual(PCONFIG_FLOAT(1), 0.0f);
      float valueLowThreshold  = PCONFIG_FLOAT(0) - (isZero ? 0.0f : (PCONFIG_FLOAT(1) / 2.0f));
      float valueHighThreshold = PCONFIG_FLOAT(0) + (isZero ? 1.0f : (PCONFIG_FLOAT(1) / 2.0f)); // Include setvalue on 0-hysteresis

      if (!definitelyGreaterThan(value, valueLowThreshold)) {
        state = 1;
      }

      if (!definitelyLessThan(value, valueHighThreshold)) {
        state = 0;
      }

      if (state != switchstate[event->TaskIndex])
      {
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("LEVEL: State ");
          log += state;
          addLog(LOG_LEVEL_INFO, log);
        }
        switchstate[event->TaskIndex] = state;
        digitalWrite(CONFIG_PIN1, state);
        UserVar[event->BaseVarIndex] = state;
        sendData(event);
      }

      success = true;
      break;
    }
  }
  return success;
}

#endif // USES_P021
