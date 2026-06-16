#include "_Plugin_Helper.h"

#ifdef USES_P187

# define PLUGIN_187
# define PLUGIN_ID_187             187                             // plugin id
# define PLUGIN_NAME_187           "Generic - Dummy Data Injector" // "Plugin Name" is what will be dislpayed in the selection list
# define PLUGIN_187_DEBUG          true                            // set to true for extra log info in the debug

enum P187_output_options {
  // do not modify order of these, as they are used in the code to determine which output is selected
  P187_OUTPUT_SINUS = 0,
  P187_OUTPUT_TRAPEZ,
  P187_OUTPUT_RANDOM,

  // keep as last:
  P187_NR_OUTPUT_OPTIONS

};

const __FlashStringHelper* Plugin_187_optionname(uint8_t value_nr,
                                                 bool    displayString) {
  const __FlashStringHelper *strings[] {
    F("Output Sinus"), F("SINUS"),
    F("Output Trapezoid"), F("TRAPEZOID"),
    F("Output Random"), F("RANDOM"),
  };

  const size_t index         = (2 * value_nr) + (displayString ? 0 : 1);
  constexpr size_t nrStrings = NR_ELEMENTS(strings);

  if (index < nrStrings) {
    return strings[index];
  }
  return F("");
}

float my_time[VARS_PER_TASK];


// storage for type of output to generate, e.g. sinus or trapezoid
# define P187_OUTPUT_OPTION_CONFIG_POS    0
# define P187_OUTPUT_OPTION0_CONFIG       PCONFIG(P187_OUTPUT_OPTION_CONFIG_POS + 0)
# define P187_OUTPUT_OPTION1_CONFIG       PCONFIG(P187_OUTPUT_OPTION_CONFIG_POS + 1)
# define P187_OUTPUT_OPTION2_CONFIG       PCONFIG(P187_OUTPUT_OPTION_CONFIG_POS + 2)
# define P187_OUTPUT_OPTION3_CONFIG       PCONFIG(P187_OUTPUT_OPTION_CONFIG_POS + 3)
# define P187_OUTPUT_OPTIONx_CONFIG(x)    PCONFIG(P187_OUTPUT_OPTION_CONFIG_POS + x)

// storage for the type of output to generate, e.g. SENSOR_TYPE_SINGLE, ...
# define P187_OUTPUT_TYPE_INDEX           4
# define P187_OUTPUT_TYPE                 PCONFIG(P187_OUTPUT_TYPE_INDEX)

// storage for output type configuration values, e.g. amplitude, offset, period, phase, ...
int param[VARS_PER_TASK][6]; // VARS_PER_TASK outputs, 6 parameters each


# define PLUGIN_187_EXPRESSION_SIZE       38

boolean Plugin_187(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {

      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_187;                  // Plugin ID number.   (PLUGIN_ID_187)
      dev.Type           = DEVICE_TYPE_DUMMY;              // How the device is connected. e.g. DEVICE_TYPE_SINGLE => connected through 1
                                                           // datapin
      dev.VType          = Sensor_VType::SENSOR_TYPE_QUAD; // Type of value the plugin will return. e.g. SENSOR_TYPE_STRING
      dev.ValueCount     = 4;                              // The number of output values of a plugin. The value should match the number of
                                                           // keys PLUGIN_VALUENAME1_187
      dev.OutputDataType = Output_Data_type_t::Simple;     // Subset of selectable output data types  (Default = no selection)
      dev.FormulaOption  = true;                           // Allow to enter a formula to convert values during read. (not possible with
                                                           // Custom enabled)
      dev.SendDataOption = true;                           // Allow to send data to a controller.
      dev.TimerOption    = true;                           // Allow to set the "Interval" timer for the plugin.
      dev.TimerOptional  = true;                           // When taskdevice timer is not set and not optional, use default "Interval"
                                                           // delay (Settings.Delay)
      dev.PluginStats    = true;                           // Support for PluginStats to record last N task values, show charts etc.
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_187);

      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      const int valueCount = getValueCountFromSensorType(static_cast<Sensor_VType>(P187_OUTPUT_TYPE));

      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        if (i < valueCount) {
          ExtraTaskSettings.setTaskDeviceValueName(i, Plugin_187_optionname(PCONFIG(i + P187_OUTPUT_OPTION_CONFIG_POS), false));
        } else {
          ExtraTaskSettings.clearTaskDeviceValueName(i);
        }
      }

      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = getValueCountFromSensorType(static_cast<Sensor_VType>(P187_OUTPUT_TYPE));
      success     = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->idx        = P187_OUTPUT_TYPE_INDEX;
      event->sensorType = static_cast<Sensor_VType>(P187_OUTPUT_TYPE);
      success           = true;
      break;
    }


    case PLUGIN_SET_DEFAULTS:
    {
      P187_OUTPUT_TYPE = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_QUAD);

      memset(&param, 0, sizeof(param));

      for (int i = 0; i < VARS_PER_TASK; i++)
      {
        P187_OUTPUT_OPTIONx_CONFIG(i) = P187_OUTPUT_SINUS;
        param[i][0]                   = 100;
        param[i][2]                   = 60;
        my_time[i]                    = 0.0f; // reset time
      }
      SaveCustomTaskSettings(event->TaskIndex, (uint8_t *)&param, sizeof(param), 0);

      success = true;
      break;
    }

    # if FEATURE_MQTT_DISCOVER || FEATURE_CUSTOM_TASKVAR_VTYPE
    case PLUGIN_GET_DISCOVERY_VTYPES:
    {
      #  if FEATURE_CUSTOM_TASKVAR_VTYPE

      for (uint8_t i = 0; i < event->Par5; ++i) {
        event->ParN[i] = ExtraTaskSettings.getTaskVarCustomVType(i);  // Custom/User selection
      }
      #  else // if FEATURE_CUSTOM_TASKVAR_VTYPE
      event->Par1 = static_cast<int>(Sensor_VType::SENSOR_TYPE_NONE); // Not yet supported
      #  endif // if FEATURE_CUSTOM_TASKVAR_VTYPE

      success = true;
      break;
    }
    # endif // if FEATURE_MQTT_DISCOVER || FEATURE_CUSTOM_TASKVAR_VTYPE

    case PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR:
    {
      const uint8_t optionCount = P187_NR_OUTPUT_OPTIONS;
      String options[optionCount];

      for (uint8_t option = 0; option < optionCount; ++option) {
        options[option] = Plugin_187_optionname(option, true);
      }

      const int valueCount = getValueCountFromSensorType(static_cast<Sensor_VType>(P187_OUTPUT_TYPE));

      for (uint8_t i = 0; i < valueCount; ++i) {
        const uint8_t pconfigIndex = i + P187_OUTPUT_OPTION_CONFIG_POS;
        sensorTypeHelper_loadOutputSelector(event, pconfigIndex, i, optionCount, options);
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      LoadCustomTaskSettings(event->TaskIndex, (uint8_t *)&param, sizeof(param));

      const int valueCount = getValueCountFromSensorType(static_cast<Sensor_VType>(P187_OUTPUT_TYPE));

      for (int i = 0; i < valueCount; i++)
      {
        String temptxt;
        temptxt = strformat(F("Output %d - %s"), i, Plugin_187_optionname(PCONFIG(i + P187_OUTPUT_OPTION_CONFIG_POS), false));
        addTableSeparator(temptxt, 2, 4);

        switch (P187_OUTPUT_OPTIONx_CONFIG(i))
        {
          case P187_OUTPUT_SINUS:
            addFormNumericBox(F("Sinus: Amplitude"), getPluginCustomArgName(i * 8 + 0), param[i][0], 0,      65537);
            addFormNumericBox(F("Sinus: Offset"),    getPluginCustomArgName(i * 8 + 1), param[i][1], -65537, 65537);
            addFormNumericBox(F("Sinus: Period"),    getPluginCustomArgName(i * 8 + 2), param[i][2], 60,     3600);
            addUnit('s');
            addFormNumericBox(F("Sinus: Phase"),     getPluginCustomArgName(i * 8 + 3), param[i][3], 0, 359);
            addUnit("°");
            break;
          case P187_OUTPUT_TRAPEZ:
            addFormNumericBox(F("Trapezoid: On-Level"),  getPluginCustomArgName(i * 8 + 0), param[i][0], -65537, 65537);
            addFormNumericBox(F("Trapezoid: Off-Level"), getPluginCustomArgName(i * 8 + 1), param[i][1], -65537, 65537);
            addFormNumericBox(F("Trapezoid: Period"),    getPluginCustomArgName(i * 8 + 2), param[i][2], 60,     3600);
            addUnit('s');
            addFormNumericBox(F("Trapezoid: On-Time"),   getPluginCustomArgName(i * 8 + 3), param[i][3], 0, 3600);
            addUnit('s');
            addFormNumericBox(F("Trapezoid: Rise-Time"), getPluginCustomArgName(i * 8 + 4), param[i][4], 0, 3600);
            addUnit('s');
            addFormNumericBox(F("Trapezoid: Fall-Time"), getPluginCustomArgName(i * 8 + 5), param[i][5], 0, 3600);
            addUnit('s');
            break;
          case P187_OUTPUT_RANDOM:
            addFormNumericBox(F("Random: Min"), getPluginCustomArgName(i * 8 + 0), param[i][0], -65537, 65537);
            addFormNumericBox(F("Random: Max"), getPluginCustomArgName(i * 8 + 1), param[i][1], -65537, 65537);
            break;
          default:
            addRowLabel(F("Output type Unknown"));
            addUnit(F("Press Submit to reset to default Sinus output"));
            break;
        }

        if (i < (valueCount - 1)) {
          addFormSeparator(2);
        }
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      const int valueCount = getValueCountFromSensorType(static_cast<Sensor_VType>(P187_OUTPUT_TYPE));

      for (int i = 0; i < valueCount; i++)
      {
        param[i][0] = getFormItemInt(getPluginCustomArgName(i * 8 + 0));
        param[i][1] = getFormItemInt(getPluginCustomArgName(i * 8 + 1));

        switch (P187_OUTPUT_OPTIONx_CONFIG(i))
        {
          case P187_OUTPUT_SINUS:
            param[i][2] = getFormItemInt(getPluginCustomArgName(i * 8 + 2));
            param[i][3] = getFormItemInt(getPluginCustomArgName(i * 8 + 3));
            param[i][4] = 0;
            param[i][5] = 0;
            break;
          case P187_OUTPUT_TRAPEZ:
            param[i][2] = getFormItemInt(getPluginCustomArgName(i * 8 + 2));
            param[i][3] = getFormItemInt(getPluginCustomArgName(i * 8 + 3));
            param[i][4] = getFormItemInt(getPluginCustomArgName(i * 8 + 4));
            param[i][5] = getFormItemInt(getPluginCustomArgName(i * 8 + 5));

            if (param[i][3] >= param[i][2])
            {
              param[i][3] = param[i][2];
              param[i][4] = 0;
              param[i][5] = 0;
            }
            else
            {
              if ((param[i][3] + param[i][4] > param[i][2]) || (param[i][3] + param[i][4] + param[i][5] > param[i][2]))
              {
                param[i][4] = (param[i][2] - param[i][3]) * param[i][4] / (param[i][4] + param[i][5]);
                param[i][5] = param[i][2] - param[i][3] - param[i][4];
              }
            }
            break;
          case P187_OUTPUT_RANDOM:
            param[i][2] = 0;
            param[i][3] = 0;
            param[i][4] = 0;
            param[i][5] = 0;
            break;
          default:
            P187_OUTPUT_OPTIONx_CONFIG(i) = P187_OUTPUT_SINUS;
            param[i][0]                   = 100;
            param[i][1]                   = 0;
            param[i][2]                   = 60;
            param[i][3]                   = 0;
            param[i][4]                   = 0;
            param[i][5]                   = 0;
            addLog(LOG_LEVEL_ERROR, F("P187: Ouput type unknown, reset to default Sinus output"));
            break;
        }

        my_time[i] = 0.0f; // reset time to start of cycle when saving new settings
      }
      success = SaveCustomTaskSettings(event->TaskIndex, (uint8_t *)&param, sizeof(param), 0);
      break;
    }

    case PLUGIN_READ:
    {
      const int valueCount = getValueCountFromSensorType(static_cast<Sensor_VType>(P187_OUTPUT_TYPE));

      for (int i = 0; i < valueCount; i++)
      {
        switch (P187_OUTPUT_OPTIONx_CONFIG(i))
        {
          case P187_OUTPUT_SINUS:
            UserVar.setFloat(event->TaskIndex, i, param[i][1] + (param[i][0] * sin((my_time[i] + param[i][3]) * PI / 180)));
            break;
          case P187_OUTPUT_TRAPEZ:

            if ((my_time[i] * param[i][2] / 360.0f)  < param[i][4]) // rising edge
            {
              UserVar.setFloat(event->TaskIndex, i,
                               param[i][1] + ((param[i][0] - param[i][1]) * (my_time[i] * param[i][2] / 360.0f) / param[i][4]));
            }
            else if ((my_time[i] * param[i][2] / 360.0f)  < (param[i][4] + param[i][3])) // on level
            {
              UserVar.setFloat(event->TaskIndex, i, param[i][0]);
            }
            else if ((my_time[i] * param[i][2] / 360.0f)  < (param[i][4] + param[i][3] + param[i][5])) // falling edge
            {
              UserVar.setFloat(event->TaskIndex, i,
                               param[i][1] +
                               ((param[i][0] - param[i][1]) *
                                (1.0f - (((my_time[i] * param[i][2] / 360.0f) - param[i][4] - param[i][3]) / param[i][5]))));
            }
            else // off level
            {
              UserVar.setFloat(event->TaskIndex, i, param[i][1]);
            }
            break;
          case P187_OUTPUT_RANDOM:
            UserVar.setFloat(event->TaskIndex, i, param[i][0] + (random(0, 10000) / 10000.0f) * (param[i][1] - param[i][0]));
            break;
        }
      }
      success = true;
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      const int valueCount = getValueCountFromSensorType(static_cast<Sensor_VType>(P187_OUTPUT_TYPE));

      for (int i = 0; i < valueCount; i++)
      {
        my_time[i] += 36.0f / param[i][2];

        if (my_time[i] > 360.0f) {
          my_time[i] -= 360.0f;
        }
      }
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      success = true;
      break;
    }

  } // switch

  return success;
}   // function

#endif // USES_P187
