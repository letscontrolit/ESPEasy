#include "_Plugin_Helper.h"

#ifdef USES_P187

// #################################################################################################
// ################################ Plugin 187  Dummy Data Injector ################################
// #################################################################################################

/** Changelog:
 * 2026-06-18 SuksAe: Initial plugin development
 * 2026-06-19 SuksAe: implemented pull request review feedback
 */

# include "src/PluginStructs/P187_data_struct.h"

# define PLUGIN_187
# define PLUGIN_ID_187             187                             // plugin id
# define PLUGIN_NAME_187           "Generic - Dummy Data Injector" // "Plugin Name" is what will be dislpayed in the selection list
# define PLUGIN_187_DEBUG          false                           // set to true for extra log info in the debug

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
      dev.OutputDataType = Output_Data_type_t::All;        // Subset of selectable output data types  (Default = no selection)
      dev.FormulaOption  = true;                           // Allow to enter a formula to convert values during read. (not possible with
                                                           // Custom enabled)
      dev.SendDataOption = true;                           // Allow to send data to a controller.
      dev.TimerOption    = true;                           // Allow to set the "Interval" timer for the plugin.
      dev.TimerOptional  = true;                           // When taskdevice timer is not set and not optional, use default "Interval"
                                                           // delay (Settings.Delay)
      dev.PluginStats    = true;                           // Support for PluginStats to record last N task values, show charts etc.
      dev.CustomVTypeVar = true;                           // Enable to allow the user to configure the Sensor_VType per Value that's available for the plugin
      dev.MqttStateClass = true;

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
      /* 
      Configuration and working variables are implemented as is because:

      -- excerpt from https://github.com/letscontrolit/ESPEasy/pull/5566#issuecomment-4739927575:

      PLUGIN_SET_DEFAULTS is called only once, right after the new plugin instance is added to the Devices list. 
      And never again after that. This is the place to set initial defaults for your plugin.

      PLUGIN_INIT is called every time the plugin is enabled, and should return true if all is OK, 
      or false if there is an error causing the plugin to stay disabled.

      -- excerpt end

      So in PLUGIN_SET_DEFAULTS, the persistent configuration values in flash are initialized. 
      This flash data is used in PLUGIN_WEBFORM_LOAD to provide data for the UI generation. The 
      flash data is updated in PLUGIN_WEBFORM_SAVE when new configuration values are submitted by 
      the user.

      Afterwards, when the plugin task instance is enabled via the UI (and output data starts
      to be generated), a working copy of the configuration values is created by loading them 
      from flash to RAM. 
      Stored together with the volatile variables, this data is used for fast processing of 
      the signal generation code.
      */
    
      P187_config_struct tmp_config[VARS_PER_TASK];

      P187_OUTPUT_TYPE = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_QUAD);

      memset(tmp_config, 0, sizeof(tmp_config));

      for (int i = 0; i < VARS_PER_TASK; i++)
      {
        P187_OUTPUT_OPTIONx_CONFIG(i) = P187_OUTPUT_SINUS;
        tmp_config[i].param0 = 100;
        tmp_config[i].param2 = 60;
      }

      SaveCustomTaskSettings(event->TaskIndex, (uint8_t *)&(tmp_config), sizeof(tmp_config), 0);  // save configuration to flash

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
      P187_config_struct tmp_config[VARS_PER_TASK];

      LoadCustomTaskSettings(event->TaskIndex, (uint8_t *)&(tmp_config), sizeof(tmp_config));  // load configuration from flash

      const int valueCount = getValueCountFromSensorType(static_cast<Sensor_VType>(P187_OUTPUT_TYPE));

      for (int i = 0; i < valueCount; i++)
      {
        addTableSeparator(strformat(F("Output %d - %s"), i + 1, FsP(Plugin_187_optionname(PCONFIG(i + P187_OUTPUT_OPTION_CONFIG_POS), false))), 2, 4);

        switch (P187_OUTPUT_OPTIONx_CONFIG(i))
        {
          case P187_OUTPUT_SINUS:
            addFormNumericBox(F("Amplitude"), getPluginCustomArgName(i * 8 + 0), tmp_config[i].param0, 0,      65537);
            addFormNumericBox(F("Offset"),    getPluginCustomArgName(i * 8 + 1), tmp_config[i].param1, -65537, 65537);
            addFormNumericBox(F("Period"),    getPluginCustomArgName(i * 8 + 2), tmp_config[i].param2, 30,     3600);
            addUnit('s');
            addFormNumericBox(F("Phase"),     getPluginCustomArgName(i * 8 + 3), tmp_config[i].param3, 0, 359);
            addUnit("°");
            break;
          case P187_OUTPUT_TRAPEZ:
            addFormNumericBox(F("On-Level"),  getPluginCustomArgName(i * 8 + 0), tmp_config[i].param0, -65537, 65537);
            addFormNumericBox(F("Off-Level"), getPluginCustomArgName(i * 8 + 1), tmp_config[i].param1, -65537, 65537);
            addFormNumericBox(F("Period"),    getPluginCustomArgName(i * 8 + 2), tmp_config[i].param2, 30,     3600);
            addUnit('s');
            addFormNumericBox(F("On-Time"),   getPluginCustomArgName(i * 8 + 3), tmp_config[i].param3, 0, 3600);
            addUnit('s');
            addFormNumericBox(F("Rise-Time"), getPluginCustomArgName(i * 8 + 4), tmp_config[i].param4, 0, 3600);
            addUnit('s');
            addFormNumericBox(F("Fall-Time"), getPluginCustomArgName(i * 8 + 5), tmp_config[i].param5, 0, 3600);
            addUnit('s');
            break;
          case P187_OUTPUT_RANDOM:
            addFormNumericBox(F("Max-Level"), getPluginCustomArgName(i * 8 + 0), tmp_config[i].param0, -65537, 65537);
            addFormNumericBox(F("Min-Level"), getPluginCustomArgName(i * 8 + 1), tmp_config[i].param1, -65537, 65537);
            break;
          default:
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
      P187_config_struct tmp_config[VARS_PER_TASK];
      
      const int valueCount = getValueCountFromSensorType(static_cast<Sensor_VType>(P187_OUTPUT_TYPE));
      memset(&(tmp_config), 0, sizeof(tmp_config));

      for (int i = 0; i < valueCount; i++)
      {
        tmp_config[i].param0 = getFormItemInt(getPluginCustomArgName(i * 8 + 0));
        tmp_config[i].param1 = getFormItemInt(getPluginCustomArgName(i * 8 + 1));

        switch (P187_OUTPUT_OPTIONx_CONFIG(i))
        {
          case P187_OUTPUT_SINUS:
            tmp_config[i].param2 = getFormItemInt(getPluginCustomArgName(i * 8 + 2));
            tmp_config[i].param3 = getFormItemInt(getPluginCustomArgName(i * 8 + 3));
            break;
          case P187_OUTPUT_TRAPEZ:
            tmp_config[i].param2 = getFormItemInt(getPluginCustomArgName(i * 8 + 2));
            tmp_config[i].param3 = getFormItemInt(getPluginCustomArgName(i * 8 + 3));
            tmp_config[i].param4 = getFormItemInt(getPluginCustomArgName(i * 8 + 4));
            tmp_config[i].param5 = getFormItemInt(getPluginCustomArgName(i * 8 + 5));

            if (tmp_config[i].param3 >= tmp_config[i].param2)
            {
              tmp_config[i].param3 = tmp_config[i].param2;
              tmp_config[i].param4 = 0;
              tmp_config[i].param5 = 0;
            }
            else
            {
              if ((tmp_config[i].param3 + tmp_config[i].param4 > tmp_config[i].param2) || (tmp_config[i].param3 + tmp_config[i].param4 + tmp_config[i].param5 > tmp_config[i].param2))
              {
                tmp_config[i].param4 = (tmp_config[i].param2 - tmp_config[i].param3) * tmp_config[i].param4 / (tmp_config[i].param4 + tmp_config[i].param5);
                tmp_config[i].param5 =  tmp_config[i].param2 - tmp_config[i].param3 - tmp_config[i].param4;
              }
            }
            break;
          case P187_OUTPUT_RANDOM:
            break;
          default:
            P187_OUTPUT_OPTIONx_CONFIG(i) = P187_OUTPUT_SINUS;
            tmp_config[i].param0 = 100;
            tmp_config[i].param2 = 60;
            addLog(LOG_LEVEL_ERROR, F("P187: Output type unknown -> reset to default Sinus output"));
            break;
        }

      }
      success = SaveCustomTaskSettings(event->TaskIndex, (uint8_t *)&(tmp_config), sizeof(tmp_config), 0);  // save configuration to flash
      break;
    }

    case PLUGIN_READ:
    {
      P187_data_struct *P187_data = static_cast<P187_data_struct *>(getPluginTaskData(event->TaskIndex));
      if (P187_data == nullptr) {
        break;
      }

      const int valueCount = getValueCountFromSensorType(static_cast<Sensor_VType>(P187_OUTPUT_TYPE));

      for (int i = 0; i < valueCount; i++)
      {
        switch (P187_OUTPUT_OPTIONx_CONFIG(i))
        {
          case P187_OUTPUT_SINUS:
            UserVar.setFloat(event->TaskIndex, i, P187_data->P187_param[i].param1 + (P187_data->P187_param[i].param0 * sinf((P187_data->P187_time[i] + P187_data->P187_param[i].param3) * PI / 180.0f)));
            break;
          case P187_OUTPUT_TRAPEZ:

            if ((P187_data->P187_time[i] * P187_data->P187_param[i].param2 / 360.0f)  < P187_data->P187_param[i].param4) // rising edge
            {
              UserVar.setFloat(event->TaskIndex, i,
                               P187_data->P187_param[i].param1 + ((P187_data->P187_param[i].param0- P187_data->P187_param[i].param1) * (P187_data->P187_time[i] * P187_data->P187_param[i].param2 / 360.0f) / P187_data->P187_param[i].param4));
            }
            else if ((P187_data->P187_time[i] * P187_data->P187_param[i].param2 / 360.0f)  < (P187_data->P187_param[i].param4 + P187_data->P187_param[i].param3)) // on level
            {
              UserVar.setFloat(event->TaskIndex, i, P187_data->P187_param[i].param0);
            }
            else if ((P187_data->P187_time[i] * P187_data->P187_param[i].param2 / 360.0f)  < (P187_data->P187_param[i].param4 + P187_data->P187_param[i].param3 + P187_data->P187_param[i].param5)) // falling edge
            {
              UserVar.setFloat(event->TaskIndex, i,
                               P187_data->P187_param[i].param1 +
                               ((P187_data->P187_param[i].param0 - P187_data->P187_param[i].param1) *
                                (1.0f - (((P187_data->P187_time[i] * P187_data->P187_param[i].param2 / 360.0f) - P187_data->P187_param[i].param4 - P187_data->P187_param[i].param3) / P187_data->P187_param[i].param5))));
            }
            else // off level
            {
              UserVar.setFloat(event->TaskIndex, i, P187_data->P187_param[i].param1);
            }
            break;
          case P187_OUTPUT_RANDOM:
            UserVar.setFloat(event->TaskIndex, i, P187_data->P187_param[i].param1 + (random(0, 10000) / 10000.0f) * (P187_data->P187_param[i].param0 - P187_data->P187_param[i].param1));
            break;
        }
      }

      success = true;
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P187_data_struct *P187_data = static_cast<P187_data_struct *>(getPluginTaskData(event->TaskIndex));
      if (P187_data == nullptr) {
        break;
      }

      const int valueCount = getValueCountFromSensorType(static_cast<Sensor_VType>(P187_OUTPUT_TYPE));

      for (int i = 0; i < valueCount; i++)
      {
        if (P187_data->P187_param[i].param2) // check for period value != 0
          P187_data->P187_time[i] += 36.0f / P187_data->P187_param[i].param2; // advance time for each output channel

        if (P187_data->P187_time[i] > 360.0f) // timer range is 0 - 360 (convenient for sinus output)
          P187_data->P187_time[i] -= 360.0f;  // so wrap around after one period
      }
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P187_data_struct());
      P187_data_struct *P187_data = static_cast<P187_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P187_data);
      if (success)
      {
        LoadCustomTaskSettings(event->TaskIndex, (uint8_t *)&(P187_data->P187_param), sizeof(P187_data->P187_param));  // load configuration from flash

        for (int i = 0; i < VARS_PER_TASK; i++)
          P187_data->P187_time[i] = 0.0f;   // initialize time
      }
      break;
    }

  } // switch

  return success;
}   // function

#endif // USES_P187
