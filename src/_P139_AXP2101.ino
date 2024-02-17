#include "_Plugin_Helper.h"
#ifdef USES_P139

# ifdef ESP32

// #######################################################################################################
// ################################### Plugin 139: AXP2101 Powermanagement ###############################
// #######################################################################################################

/**
 * Changelog:
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
 * TODO: Define additional values?
 **/
/**
 * Events:
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
      Device[++deviceCount].Number       = PLUGIN_ID_139;
      Device[deviceCount].Type           = DEVICE_TYPE_I2C;
      Device[deviceCount].VType          = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].OutputDataType = Output_Data_type_t::Simple;
      Device[deviceCount].PowerManager   = true; // So it can be started before SPI is initialized
      Device[deviceCount].FormulaOption  = true;
      Device[deviceCount].ValueCount     = 4;
      Device[deviceCount].SendDataOption = true;
      Device[deviceCount].TimerOption    = true;
      Device[deviceCount].TimerOptional  = true;
      Device[deviceCount].PluginStats    = true;
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
          ExtraTaskSettings.setTaskDeviceValueName(i, toString(static_cast<AXP2101_registers_e>(PCONFIG(P139_CONFIG_BASE + i)), false));
        } else {
          ExtraTaskSettings.clearTaskDeviceValueName(i);
        }
        ExtraTaskSettings.TaskDeviceValueDecimals[i] = 0; // No values have decimals
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
      P139_CONFIG_DECIMALS            = 2;               // 2 decimals for all get config values
      AXP2101_device_model_e device = AXP2101_device_model_e::M5Stack_Core2_v1_1;
      P139_CONFIG_PREDEFINED = static_cast<int>(device); // M5Stack Core2 v1.1

      P139_data_struct *P139_data = new (std::nothrow) P139_data_struct(event);

      if (nullptr != P139_data) {
        P139_data->applySettings(device);
        P139_data->saveSettings(event);
        delete P139_data;
      }
      P139_CONFIG_PREDEFINED = 0; // Settings applied

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      const bool isPowerManagerTask = Settings.isPowerManagerTask(event->TaskIndex);
      bool created_new              = false;
      P139_data_struct *P139_data   = static_cast<P139_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P139_data) {
        P139_data   = new (std::nothrow) P139_data_struct(event);
        created_new = true;
      }

      if (nullptr == P139_data) {
        break;
      }

      {
        const __FlashStringHelper *chargeledNames[] = {
          toString(AXP2101_chargeled_d::Off),
          toString(AXP2101_chargeled_d::Flash_1Hz),
          toString(AXP2101_chargeled_d::Flash_4Hz),
          toString(AXP2101_chargeled_d::Steady_On),
        };
        const int chargeledValues[] = {
          static_cast<int>(AXP2101_chargeled_d::Off),
          static_cast<int>(AXP2101_chargeled_d::Flash_1Hz),
          static_cast<int>(AXP2101_chargeled_d::Flash_4Hz),
          static_cast<int>(AXP2101_chargeled_d::Steady_On),
        };
        addFormSelector(F("Charge LED"), F("led"),
                        NR_ELEMENTS(chargeledValues),
                        chargeledNames, chargeledValues,
                        static_cast<int>(P139_data->_settings.getChargeLed()));
      }

      addFormSubHeader(F("Hardware outputs AXP2101"));

      {
        if (P139_CONFIG_PREDEFINED > 0) {
          P139_CURRENT_PREDEFINED = P139_CONFIG_PREDEFINED;
          P139_data->applySettings(static_cast<AXP2101_device_model_e>(P139_CONFIG_PREDEFINED));
        }
        const __FlashStringHelper *predefinedNames[] = {
          toString(AXP2101_device_model_e::Unselected),
          toString(AXP2101_device_model_e::M5Stack_Core2_v1_1),
          toString(AXP2101_device_model_e::M5Stack_CoreS3),
          toString(AXP2101_device_model_e::LilyGO_TBeam_v1_2),
          toString(AXP2101_device_model_e::LilyGO_TBeamS3_v3),
          toString(AXP2101_device_model_e::LilyGO_TPCie_v1_2),
          toString(AXP2101_device_model_e::UserDefined) // keep last !!
        };
        const int predefinedValues[] = {
          static_cast<int>(AXP2101_device_model_e::Unselected),
          static_cast<int>(AXP2101_device_model_e::M5Stack_Core2_v1_1),
          static_cast<int>(AXP2101_device_model_e::M5Stack_CoreS3),
          static_cast<int>(AXP2101_device_model_e::LilyGO_TBeam_v1_2),
          static_cast<int>(AXP2101_device_model_e::LilyGO_TBeamS3_v3),
          static_cast<int>(AXP2101_device_model_e::LilyGO_TPCie_v1_2),
          static_cast<int>(AXP2101_device_model_e::UserDefined) }; // keep last !!
        addFormSelector(F("Predefined device configuration"), F("predef"),
                        NR_ELEMENTS(predefinedValues),
                        predefinedNames, predefinedValues, 0, !isPowerManagerTask);

        if (!isPowerManagerTask) {
          addFormNote(F("Page will reload when selection is changed."));
        }

        const AXP2101_device_model_e device = static_cast<AXP2101_device_model_e>(P139_CURRENT_PREDEFINED);

        if (AXP2101_device_model_e::Unselected != device) {
          addFormNote(concat(F("Last selected: "), toString(device)));

          if (AXP2101_device_model_e::UserDefined == device) {
            addHtml(F(
                      "<div class='note'><span style=\"color:red\">Warning: Configuring invalid values can damage your device or render it useless!</span></div>"));
          }
        }
      }

      {
        const __FlashStringHelper *bootStates[] = {
          toString(AXP_pin_s::Off),
          toString(AXP_pin_s::On),
          toString(AXP_pin_s::Default),
        };
        const int bootStateValues[] = {
          static_cast<int>(AXP_pin_s::Off),
          static_cast<int>(AXP_pin_s::On),
          static_cast<int>(AXP_pin_s::Default),
        };

        // Don't include Disabled or Protected here, not user-selectable
        constexpr int bootStatesCount = NR_ELEMENTS(bootStates);

        addRowLabel(F("Output ports"));

        html_table(EMPTY_STRING);
        html_table_header(F("Name"),         100);
        html_table_header(F("Voltage (mV)"), 270);
        html_table_header(F("Pin state"),    150);

        for (int s = 0; s < AXP2101_settings_count; ++s) {
          const AXP2101_registers_e reg = AXP2101_intToRegister(s);
          const AXP_pin_s pin           = P139_data->_settings.getState(reg);
          html_TR_TD();
          addHtml(toString(reg));
          html_TD();
          addNumericBox(toString(reg, false),
                        P139_data->_settings.getVoltage(reg, false),
                        -1,
                        AXP2101_maxVoltage(reg),
                        AXP2101_isPinDefault(pin));
          addUnit(strformat(F("range %d - %d"), AXP2101_minVoltage(reg), AXP2101_maxVoltage(reg)));
          html_TD();

          if (AXP2101_isPinProtected(pin)) {
            addUnit(toString(pin));
          } else {
            addSelector(concat(F("ps"), toString(reg, false)),
                        bootStatesCount,
                        bootStates,
                        bootStateValues,
                        nullptr,
                        static_cast<int>(pin));
          }
        }
        html_end_table();

        // addFormNote(F("Values &lt; min. range will switch off the output. Set to -1 to not initialize/unused."));
        addFormNote(F("Check your device documentation for what is connected to each output."));
      }

      {
        // Keep this setting hidden from the UI
        // addHtml(F("<div hidden>"));
        // addNumericBox(F("pbits"), P139_CONFIG_DISABLEBITS, 0, 0xFFFF);
        // addHtml(F("</div>"));
      }

      if (created_new) {
        delete P139_data;
      }

      success = true;
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

      for (uint8_t i = 0; i < P139_NR_OUTPUT_VALUES; ++i) {
        sensorTypeHelper_loadOutputSelector(event,
                                            P139_CONFIG_BASE + i,
                                            i,
                                            NR_ELEMENTS(valValues),
                                            valOptions,
                                            valValues);
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      for (uint8_t i = 0; i < P139_NR_OUTPUT_VALUES; ++i) {
        sensorTypeHelper_saveOutputSelector(event, P139_CONFIG_BASE + i, i,
                                            toString(static_cast<AXP2101_registers_e>(PCONFIG(P139_CONFIG_BASE + i)), false));
      }

      P139_CONFIG_PREDEFINED = getFormItemInt(F("predef"));

      bool created_new            = false;
      P139_data_struct *P139_data = static_cast<P139_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P139_data) {
        P139_data   = new (std::nothrow) P139_data_struct(event);
        created_new = true;
      }

      if (nullptr == P139_data) {
        break;
      }

      for (int s = 0; s < AXP2101_settings_count; ++s) {
        const AXP2101_registers_e reg = AXP2101_intToRegister(s);

        if (!AXP2101_isPinProtected(P139_data->_settings.getState(reg))) {
          P139_data->_settings.setVoltage(reg, getFormItemInt(toString(reg, false)));
          P139_data->_settings.setState(reg, static_cast<AXP_pin_s>(getFormItemInt(concat(F("ps"), toString(reg, false)))));
        }
      }

      P139_data->_settings.setChargeLed(static_cast<AXP2101_chargeled_d>(getFormItemInt(F("led"))));

      P139_data->saveSettings(event);

      if (created_new) {
        delete P139_data;
      }

      success = true;
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

    case PLUGIN_TEN_PER_SECOND:
    {
      P139_data_struct *P139_data = static_cast<P139_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P139_data) {
        success = P139_data->plugin_ten_per_second(event);
      }
      break;
    }

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
