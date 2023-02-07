#include "_Plugin_Helper.h"
#ifdef USES_P138

// #######################################################################################################
// #################################### Plugin 138: IP5306 Powermanagement ###############################
// #######################################################################################################

/**
 * Changelog:
 * 2022-12-06 tonhuisman: Reorder Device configuration because of added PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR state
 *                        Enable PluginStats option
 * 2022-12-05 tonhuisman: Remove [Testing] tag
 * 2022-11-08 tonhuisman: Fix a few typos, cleanup some comments
 * 2022-08-26 tonhuisman: Initial plugin development, using codewitch-honey-crisis/htcw_ip5306 library
 **/

/**
 * Supported commands: (none yet)
 **/
/**
 * Get Config options:
 * [<taskname>#batcurrent]    : Battery current
 * [<taskname>#chundervolt]   : Charge Undervoltage
 * [<taskname>#stopvolt]      : Input voltage
 * [<taskname>#inpcurrent]    : Input current
 * [<taskname>#pwrsource]     : Powersource
 * // To re-use these lines:
 * [<taskname>#vbuscurr]      : VBus current
 * [<taskname>#inttemp]       : Internal temperature
 * [<taskname>#apsvolt]       : APS voltage
 * [<taskname>#ldo2volt]      : LDO2 voltage
 * [<taskname>#ldo3volt]      : LDO3 voltage
 * [<taskname>#gpio0volt]     : GPIO0 voltage
 **/


# define PLUGIN_138
# define PLUGIN_ID_138         138
# define PLUGIN_NAME_138       "Power mgt - IP5306 Power management"

# include "./src/PluginStructs/P138_data_struct.h"

boolean Plugin_138(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number       = PLUGIN_ID_138;
      Device[deviceCount].Type           = DEVICE_TYPE_I2C;
      Device[deviceCount].VType          = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].OutputDataType = Output_Data_type_t::Simple;
      Device[deviceCount].FormulaOption  = true;
      Device[deviceCount].ValueCount     = 4;
      Device[deviceCount].SendDataOption = true;
      Device[deviceCount].TimerOption    = true;
      Device[deviceCount].TimerOptional  = true;
      Device[deviceCount].PluginStats    = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_138);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        if (i < P138_NR_OUTPUT_VALUES) {
          safe_strncpy(
            ExtraTaskSettings.TaskDeviceValueNames[i],
            toString(static_cast<P138_valueOptions_e>(PCONFIG(P138_CONFIG_BASE + i)), false),
            sizeof(ExtraTaskSettings.TaskDeviceValueNames[i]));
        } else {
          ZERO_FILL(ExtraTaskSettings.TaskDeviceValueNames[i]);
        }
      }
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = P138_NR_OUTPUT_VALUES;
      success     = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = static_cast<Sensor_VType>(PCONFIG(P138_SENSOR_TYPE_INDEX));
      event->idx        = P138_SENSOR_TYPE_INDEX;
      success           = true;
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      success = event->Par1 == 0x75;
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = 0x75;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      PCONFIG(0)                      = static_cast<int>(P138_valueOptions_e::StopVoltage);
      PCONFIG(1)                      = static_cast<int>(P138_valueOptions_e::InCurrent);
      PCONFIG(2)                      = static_cast<int>(P138_valueOptions_e::BatteryCurrent);
      PCONFIG(3)                      = static_cast<int>(P138_valueOptions_e::ChargeLevel);
      PCONFIG(P138_SENSOR_TYPE_INDEX) = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_QUAD);
      P138_CONFIG_DECIMALS            = 2;              // 2 decimals for all get config values
      bitSet(P138_CONFIG_FLAGS, P138_FLAG_POWERCHANGE); // Enable event

      break;
    }

    case PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR:
    {
      {
        const __FlashStringHelper *valOptions[] = {
          toString(P138_valueOptions_e::None),
          toString(P138_valueOptions_e::BatteryCurrent),
          toString(P138_valueOptions_e::ChargeUnderVoltage),
          toString(P138_valueOptions_e::StopVoltage),
          toString(P138_valueOptions_e::InCurrent),
          toString(P138_valueOptions_e::ChargeLevel),
          toString(P138_valueOptions_e::PowerSource),
        };
        const int valValues[] = {
          static_cast<int>(P138_valueOptions_e::None),
          static_cast<int>(P138_valueOptions_e::BatteryCurrent),
          static_cast<int>(P138_valueOptions_e::ChargeUnderVoltage),
          static_cast<int>(P138_valueOptions_e::StopVoltage),
          static_cast<int>(P138_valueOptions_e::InCurrent),
          static_cast<int>(P138_valueOptions_e::ChargeLevel),
          static_cast<int>(P138_valueOptions_e::PowerSource),
        };

        for (uint8_t i = 0; i < P138_NR_OUTPUT_VALUES; i++) {
          sensorTypeHelper_loadOutputSelector(event,
                                              P138_CONFIG_BASE + i,
                                              i,
                                              sizeof(valValues) / sizeof(int),
                                              valOptions,
                                              valValues);
        }
      }
      success = true;

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("Decimals for config values"), F("decimals"), P138_CONFIG_DECIMALS, 0, 4);

      addFormCheckBox(F("Event on PowerSource change"), F("eventpwrchg"), bitRead(P138_CONFIG_FLAGS, P138_FLAG_POWERCHANGE));
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      for (uint8_t i = 0; i < P138_NR_OUTPUT_VALUES; i++) {
        sensorTypeHelper_saveOutputSelector(event, P138_CONFIG_BASE + i, i,
                                            toString(static_cast<P138_valueOptions_e>(PCONFIG(P138_CONFIG_BASE + i)), false));
      }

      P138_CONFIG_DECIMALS = getFormItemInt(F("decimals"));
      bitWrite(P138_CONFIG_FLAGS, P138_FLAG_POWERCHANGE, isFormItemChecked(F("eventpwrchg")));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P138_data_struct(event));
      P138_data_struct *P138_data = static_cast<P138_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P138_data) {
        success = true;
      }

      break;
    }

    case PLUGIN_READ:
    {
      P138_data_struct *P138_data = static_cast<P138_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P138_data) {
        success = P138_data->plugin_read(event);
      }
      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      P138_data_struct *P138_data = static_cast<P138_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P138_data) {
        success = P138_data->plugin_fifty_per_second(event);
      }
      break;
    }

    // case PLUGIN_WRITE:
    // {
    //   P138_data_struct *P138_data = static_cast<P138_data_struct *>(getPluginTaskData(event->TaskIndex));

    //   if (nullptr != P138_data) {
    //     success = P138_data->plugin_write(event, string);
    //   }
    //   break;
    // }

    case PLUGIN_GET_CONFIG_VALUE:
    {
      P138_data_struct *P138_data = static_cast<P138_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P138_data) {
        success = P138_data->plugin_get_config_value(event, string); // GetConfig operation, handle variables
      }
      break;
    }
  }

  return success;
}

#endif // USES_P138
