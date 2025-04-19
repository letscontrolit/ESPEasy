#include "_Plugin_Helper.h"
#ifdef USES_P139

# ifdef ESP32

// #######################################################################################################
// ################################### Plugin 139: AXP2101 Powermanagement ###############################
// #######################################################################################################

/**
 * Changelog:
 * 2025-03-15 tonhuisman: Removed unneeded I2C Enabled check.
 * 2025-01-18 tonhuisman: Enable writing values to chip.
 * 2024-02-25 tonhuisman: Add I2C-enabled check on plugin startup, implement FsP macro
 * 2024-02-21 tonhuisman: Add support for ChipID and ChargingDetail data supplied by AXP2101
 * 2024-02-18 tonhuisman: Add setting for Generate events, support for chargestate and isBatteryDetected, fix some issues
 * 2024-02-17 tonhuisman: Add setting for Charge led and battery charge level, fix saving adjusted port settings,
 *                        set to 0 decimals as we're using mV values
 * 2024-02-15 tonhuisman: First plugin version, in ReadOnly mode only, no data is written to the AXP2101, only the register to read
 * 2024-02-04 tonhuisman: Initial plugin development, only available for ESP32
 **/

/**
 * Supported commands: (using same command as P137 AXP192 as no hardware overlap is possible)
 * axp,readchip                       : Read current settings from AXP2101 chip and list values and state to the log at INFO level
 * axp,voltage,<port>,<voltage>       : Set port to given voltage and on, or turn off if below minimum value
 * axp,on,<port>                      : Turn On port
 * axp,off,<port>                     : Turn Off port
 * axp,percentage,<port>,<percentage> : Set port to percentage of Low to High range (min/max or set range per port)
 * axp,range,<port>,<low>,<high>      : Define low/high range for port. Low and High must be withing technical range of port
 * axp,range                          : List current range configuration (or when providing an out of range low/high argument)
 * axp,chargeled,<ledstate>           : Set charge-led state, 0 : off, 1 : flash 1Hz, 2 : flash 4Hz, 3 : on
 * TODO: Add more commands?
 **/
/**
 * Get Config options:
 * [<taskname>#dcdc1]       : Returns the voltage from the named port.
 * [<taskname>#dcdc2]       : Can also read the status by using [<taskname>#dcdc1.status] (text: On/Off/Default/Disabled/Protected)
 * [<taskname>#dcdc3]       : Can also read the numeric status by using [<taskname>#dcdc1.state] (0/1/2/3/7)
 * [<taskname>#dcdc4]       :
 * [<taskname>#dcdc5]       :
 * [<taskname>#aldo1]       :
 * [<taskname>#aldo2]       :
 * [<taskname>#aldo3]       :
 * [<taskname>#aldo4]       :
 * [<taskname>#bldo1]       :
 * [<taskname>#bldo2]       :
 * [<taskname>#dldo1]       :
 * [<taskname>#dldo2]       :
 * [<taskname>#cpuldos]     :
 * [<taskname>#chargeled]   :
 * [<taskname>#batcharge]   : (Doesn't support the .status and .state variants of the variable)
 * [<taskname>#chargingstate] : Charging state, -1 = discharging, 0 = standby, 1 = charging
 * [<taskname>#batpresent]  : (Doesn't support the .status and .state variants of the variable)
 * [<taskname>#chipid]      : (Doesn't support the .state variant of the variable)
 * [<taskname>#chargingdet] : (Doesn't support the .state variant of the variable)
 * TODO: Define additional values?
 **/
/**
 * Events:
 * <taskname>#ChargingState=<new>,<old> : On change of the charging-state, new/old values: -1 = discharging, 0 = standby, 1 = charging
 * TODO: Define events?
 */


#  define PLUGIN_139
#  define PLUGIN_ID_139         139
#  define PLUGIN_NAME_139       "Power mgt - AXP2101 Power management"

#  include "./src/PluginStructs/P139_data_struct.h"

boolean Plugin_139(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_139;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_QUAD;
      dev.OutputDataType = Output_Data_type_t::Simple;
      dev.PowerManager   = true; // So it can be started before SPI is initialized
      dev.FormulaOption  = true;
      dev.ValueCount     = 4;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.TimerOptional  = true;
      dev.PluginStats    = true;
      break;
    }

    case PLUGIN_PRIORITY_INIT:
    {
      const bool isPowerManagerTask = Settings.isPowerManagerTask(event->TaskIndex);
      #  ifndef BUILD_NO_DEBUG
      addLogMove(LOG_LEVEL_DEBUG, F("P139: PLUGIN_PRIORITY_INIT"));
      #  endif // ifndef BUILD_NO_DEBUG
      success = isPowerManagerTask; // Are we the PowerManager task?
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_139);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        if (i < P139_NR_OUTPUT_VALUES) {
          const uint8_t choice = PCONFIG(P139_CONFIG_BASE + i);
          ExtraTaskSettings.setTaskDeviceValueName(i, toString(static_cast<AXP2101_registers_e>(choice), false));

          if ((choice != (static_cast<uint8_t>(AXP2101_registers_e::battemp))) &&
              (choice != (static_cast<uint8_t>(AXP2101_registers_e::chiptemp)))) {
            ExtraTaskSettings.TaskDeviceValueDecimals[i] = 0;
          }
        } else {
          ExtraTaskSettings.clearTaskDeviceValueName(i);
          ExtraTaskSettings.TaskDeviceValueDecimals[i] = 0;
        }
      }
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = P139_NR_OUTPUT_VALUES;
      success     = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = static_cast<Sensor_VType>(PCONFIG(P139_SENSOR_TYPE_INDEX));
      event->idx        = P139_SENSOR_TYPE_INDEX;
      success           = true;
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      success =  event->Par1 == AXP2101_ADDR;
      break;
    }

    #  if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = AXP2101_ADDR;
      success     = true;
      break;
    }
    #  endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      PCONFIG(0)                      = static_cast<int>(AXP2101_registers_e::dcdc1);
      PCONFIG(1)                      = static_cast<int>(AXP2101_registers_e::dcdc3);
      PCONFIG(2)                      = static_cast<int>(AXP2101_registers_e::aldo1);
      PCONFIG(3)                      = static_cast<int>(AXP2101_registers_e::dldo1);
      PCONFIG(P139_SENSOR_TYPE_INDEX) = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_QUAD);
      P139_CONFIG_DECIMALS            = 2;                // 2 decimals for all get config values
      AXP2101_device_model_e device = AXP2101_device_model_e::M5Stack_Core2_v1_1;
      P139_CURRENT_PREDEFINED = static_cast<int>(device); // M5Stack Core2 v1.1
      P139_SET_GENERATE_EVENTS(true);

      P139_data_struct *P139_data = new (std::nothrow) P139_data_struct();

      if (nullptr != P139_data) {
        P139_data->applyDeviceModelTemplate(device);
        P139_data->saveSettings(event);
        delete P139_data;
      }

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      bool created_new            = false;
      P139_data_struct *P139_data = static_cast<P139_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P139_data) {
        P139_data = new (std::nothrow) P139_data_struct();
        P139_data->loadSettings(event);
        created_new = true;
      }

      if (nullptr != P139_data) {
        P139_data->webform_load(event);

        if (created_new) {
          delete P139_data;
        }

        success = true;
      }

      break;
    }

    case PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR:
    {
      const __FlashStringHelper *valOptions[AXP2101_register_count + 1];
      int valValues[AXP2101_register_count + 1];

      valOptions[0] = F("None");
      valValues[0]  = 0;

      for (int r = 0; r < AXP2101_register_count; ++r) {
        AXP2101_registers_e reg = AXP2101_intToRegister(r);
        valOptions[r + 1] = toString(reg);
        valValues[r + 1]  = static_cast<int>(reg);
      }
      constexpr uint8_t valueCount = NR_ELEMENTS(valValues);

      for (uint8_t i = 0; i < P139_NR_OUTPUT_VALUES; ++i) {
        sensorTypeHelper_loadOutputSelector(event,
                                            P139_CONFIG_BASE + i,
                                            i,
                                            valueCount,
                                            valOptions,
                                            valValues);
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      bool created_new            = false;
      P139_data_struct *P139_data = static_cast<P139_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P139_data) {
        P139_data = new (std::nothrow) P139_data_struct();
        P139_data->loadSettings(event);
        created_new = true;
      }

      if (nullptr != P139_data) {
        P139_data->webform_save(event);

        if (created_new) {
          delete P139_data;
        }

        success = true;
      }

      break;
    }

    case PLUGIN_INIT:
    {
      P139_data_struct *P139_init = static_cast<P139_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P139_init) {
        #  ifndef BUILD_NO_DEBUG
        addLogMove(LOG_LEVEL_INFO, F("P139: Already initialized, skipped."));
        #  endif // ifndef BUILD_NO_DEBUG
        // has been initialized so nothing to do here
        success = true; // Still was successful (to keep plugin enabled!)
      } else {
        #  ifndef BUILD_NO_DEBUG
        addLogMove(LOG_LEVEL_DEBUG, F("P139: PLUGIN_INIT"));
        #  endif // ifndef BUILD_NO_DEBUG
        success = initPluginTaskData(event->TaskIndex, new (std::nothrow) P139_data_struct(event));
      }

      break;
    }

    case PLUGIN_READ:
    {
      P139_data_struct *P139_data = static_cast<P139_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P139_data) {
        success = P139_data->plugin_read(event);
      }
      break;
    }

    // case PLUGIN_TEN_PER_SECOND:
    // {
    //   P139_data_struct *P139_data = static_cast<P139_data_struct *>(getPluginTaskData(event->TaskIndex));

    //   if (nullptr != P139_data) {
    //     success = P139_data->plugin_ten_per_second(event);
    //   }
    //   break;
    // }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      P139_data_struct *P139_data = static_cast<P139_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P139_data) {
        success = P139_data->plugin_fifty_per_second(event);
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      P139_data_struct *P139_data = static_cast<P139_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P139_data) {
        success = P139_data->plugin_write(event, string);
      }
      break;
    }

    case PLUGIN_GET_CONFIG_VALUE:
    {
      P139_data_struct *P139_data = static_cast<P139_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P139_data) {
        success = P139_data->plugin_get_config_value(event, string); // GetConfig operation, handle variables
      }
      break;
    }
  }

  return success;
}

# endif // ifdef ESP32
#endif // USES_P139
