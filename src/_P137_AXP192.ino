#include "_Plugin_Helper.h"
#ifdef USES_P137

# ifdef ESP32

// #######################################################################################################
// #################################### Plugin 137: AXP192 Powermanagement ###############################
// #######################################################################################################

/**
 * Changelog:
 * 2022-08-24 tonhuisman: Remove [TESTING] tag, move include for _Plugin_helper.h to correct line
 * 2022-07-18 tonhuisman: Add missing Get Config values
 * 2022-07-05 tonhuisman: Add commands for setting LDO2, LDO3 and GPIO0 pins to voltage, or percentage, mapped to range
 *                        Supported commands see list below.
 *                        Include LDO2, LDO3 and GPIO0 in available values (not available from library)
 * 2022-07-04 tonhuisman: Add configurable values for LDO2, LDO3, GPIO0 (of AXP192)
 *                        Get values via Get Config option, using [<taskname>#<configname>] (see below for list)
 * 2022-07-03 tonhuisman: Initial plugin development, only available for ESP32, as the library uses ESP_LOGD, not available for ESP8266
 **/

/**
 * Supported commands:
 * axp,ldo2,<voltage>         : Set voltage for AXP192 LDO2 pin, below 1800 mV or above 3300 mV will turn pin off
 * axp,ldo3,<voltage>         : Set voltage for AXP192 LDO3 pin, below 1800 mV or above 3300 mV will turn pin off
 * axp,gpio0,<voltage>        : Set voltage for AXP192 GPIO0 pin
 * axp,ldo2map,<low>,<high>   : Set mapping range for percentage 0..100, <low> >= 0, <high> <= 3300
 * axp,ldo3map,<low>,<high>   : Set mapping range for percentage 0..100
 * axp,gpio0map,<low>,<high>  : Set mapping range for percentage 0..100
 * axp,ldo2perc,<percentage>  : Set voltage to percentage of mapped values, 0..100, 0 = off, 1..100 maps to map range
 * axp,ldo3perc,<percentage>  : Set voltage to percentage of mapped values
 * axp,gpio0perc,<percentage> : Set voltage to percentage of mapped values
 **/
/**
 * Get Config options:
 * [<taskname>#batvoltage]    : Battery voltage
 * [<taskname>#batdischarge]  : Battery discharge current
 * [<taskname>#batcharge]     : Battery charge current
 * [<taskname>#batpower]      : Battery power
 * [<taskname>#inpvoltage]    : Input voltage
 * [<taskname>#inpcurrent]    : Input current
 * [<taskname>#vbusvolt]      : VBus voltage
 * [<taskname>#vbuscurr]      : VBus current
 * [<taskname>#inttemp]       : Internal temperature
 * [<taskname>#apsvolt]       : APS voltage
 * [<taskname>#ldo2volt]      : LDO2 voltage
 * [<taskname>#ldo3volt]      : LDO3 voltage
 * [<taskname>#gpio0volt]     : GPIO0 voltage
 **/


#  define PLUGIN_137
#  define PLUGIN_ID_137         137
#  define PLUGIN_NAME_137       "Power mgt - AXP192 Power management"

#  include "./src/PluginStructs/P137_data_struct.h"

boolean Plugin_137(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number       = PLUGIN_ID_137;
      Device[deviceCount].Type           = DEVICE_TYPE_I2C;
      Device[deviceCount].VType          = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].OutputDataType = Output_Data_type_t::Simple;
      Device[deviceCount].FormulaOption  = true;
      Device[deviceCount].ValueCount     = 4;
      Device[deviceCount].SendDataOption = true;
      Device[deviceCount].TimerOption    = true;
      Device[deviceCount].TimerOptional  = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_137);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        if (i < P137_NR_OUTPUT_VALUES) {
          safe_strncpy(
            ExtraTaskSettings.TaskDeviceValueNames[i],
            toString(static_cast<P137_valueOptions_e>(PCONFIG(P137_CONFIG_BASE + i)), false),
            sizeof(ExtraTaskSettings.TaskDeviceValueNames[i]));
        } else {
          ZERO_FILL(ExtraTaskSettings.TaskDeviceValueNames[i]);
        }
      }
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = P137_NR_OUTPUT_VALUES;
      success     = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = static_cast<Sensor_VType>(PCONFIG(P137_SENSOR_TYPE_INDEX));
      event->idx        = P137_SENSOR_TYPE_INDEX;
      success           = true;
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      success =  event->Par1 == I2C_AXP192_DEFAULT_ADDRESS;
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P137_CONFIG_LDO2                = 3000; // LDO2 = 3000 mV
      P137_CONFIG_LDO3                = 3000; // LDO3 = 3000 mV
      P137_CONFIG_GPIO0               = 2800; // (AXP192)GPIO0 = 2800 mV
      PCONFIG(0)                      = static_cast<int>(P137_valueOptions_e::VbusVoltage);
      PCONFIG(1)                      = static_cast<int>(P137_valueOptions_e::VbusCurrent);
      PCONFIG(2)                      = static_cast<int>(P137_valueOptions_e::BatteryVoltage);
      PCONFIG(3)                      = static_cast<int>(P137_valueOptions_e::InternalTemperature);
      PCONFIG(P137_SENSOR_TYPE_INDEX) = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_QUAD);
      P137_CONFIG_DECIMALS            = 2; // 2 decimals for all get config values

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        const __FlashStringHelper *valOptions[] = {
          toString(P137_valueOptions_e::None),
          toString(P137_valueOptions_e::BatteryVoltage),
          toString(P137_valueOptions_e::BatteryDischargeCurrent),
          toString(P137_valueOptions_e::BatteryChargeCurrent),
          toString(P137_valueOptions_e::BatteryPower),
          toString(P137_valueOptions_e::AcinVoltage),
          toString(P137_valueOptions_e::AcinCurrent),
          toString(P137_valueOptions_e::VbusVoltage),
          toString(P137_valueOptions_e::VbusCurrent),
          toString(P137_valueOptions_e::InternalTemperature),
          toString(P137_valueOptions_e::ApsVoltage),
          toString(P137_valueOptions_e::LDO2),
          toString(P137_valueOptions_e::LDO3),
          toString(P137_valueOptions_e::GPIO0)
        };
        const int valValues[] = {
          static_cast<int>(P137_valueOptions_e::None),
          static_cast<int>(P137_valueOptions_e::BatteryVoltage),
          static_cast<int>(P137_valueOptions_e::BatteryDischargeCurrent),
          static_cast<int>(P137_valueOptions_e::BatteryChargeCurrent),
          static_cast<int>(P137_valueOptions_e::BatteryPower),
          static_cast<int>(P137_valueOptions_e::AcinVoltage),
          static_cast<int>(P137_valueOptions_e::AcinCurrent),
          static_cast<int>(P137_valueOptions_e::VbusVoltage),
          static_cast<int>(P137_valueOptions_e::VbusCurrent),
          static_cast<int>(P137_valueOptions_e::InternalTemperature),
          static_cast<int>(P137_valueOptions_e::ApsVoltage),
          static_cast<int>(P137_valueOptions_e::LDO2),
          static_cast<int>(P137_valueOptions_e::LDO3),
          static_cast<int>(P137_valueOptions_e::GPIO0)
        };

        for (uint8_t i = 0; i < P137_NR_OUTPUT_VALUES; i++) {
          sensorTypeHelper_loadOutputSelector(event,
                                              P137_CONFIG_BASE + i,
                                              i,
                                              static_cast<int>(P137_valueOptions_e::OptionCount),
                                              valOptions,
                                              valValues);
        }
      }

      addFormNumericBox(F("Decimals for config values"), F("decimals"), P137_CONFIG_DECIMALS, 0, 4);

      addFormSubHeader(F("Hardware outputs AXP192"));

      addFormNumericBox(F("LDO2"), F("pldo2"), P137_CONFIG_LDO2, 0, 3300);
      addUnit(F("mV"));
      addFormNumericBox(F("LDO3"), F("pldo3"), P137_CONFIG_LDO3, 0, 3300);
      addUnit(F("mV"));
      addFormNumericBox(F("GPIO0"), F("pgpio0"), P137_CONFIG_GPIO0, 0, 3300);
      addUnit(F("mV"));

      addFormNote(F("Value range: 1800..3300mV in 100 mV steps. Values &lt; 1800mV will switch off the output."));
      addFormNote(F("Check your board documentation for what is connected to each output."));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      for (uint8_t i = 0; i < P137_NR_OUTPUT_VALUES; i++) {
        sensorTypeHelper_saveOutputSelector(event, P137_CONFIG_BASE + i, i,
                                            toString(static_cast<P137_valueOptions_e>(PCONFIG(P137_CONFIG_BASE + i)), false));
      }

      P137_CONFIG_LDO2     = getFormItemInt(F("pldo2"));
      P137_CONFIG_LDO3     = getFormItemInt(F("pldo3"));
      P137_CONFIG_GPIO0    = getFormItemInt(F("pgpio0"));
      P137_CONFIG_DECIMALS = getFormItemInt(F("decimals"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P137_data_struct(event));
      P137_data_struct *P137_data = static_cast<P137_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P137_data) {
        success = true;
      }

      break;
    }

    case PLUGIN_READ:
    {
      P137_data_struct *P137_data = static_cast<P137_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P137_data) {
        success = P137_data->plugin_read(event);
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      P137_data_struct *P137_data = static_cast<P137_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P137_data) {
        success = P137_data->plugin_write(event, string);
      }
      break;
    }

    case PLUGIN_GET_CONFIG_VALUE:
    {
      P137_data_struct *P137_data = static_cast<P137_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P137_data) {
        success = P137_data->plugin_get_config_value(event, string); // GetConfig operation, handle variables
      }
      break;
    }
  }

  return success;
}

# endif // ifdef ESP32
#endif // USES_P137
