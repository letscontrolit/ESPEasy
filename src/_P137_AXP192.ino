#include "_Plugin_Helper.h"
#ifdef USES_P137

# ifdef ESP32

// #######################################################################################################
// #################################### Plugin 137: AXP192 Powermanagement ###############################
// #######################################################################################################

/**
 * Changelog:
 * 2022-12-27 tonhuisman: Add predefined config settings for LilyGO T-Beam LoRa units
 * 2022-12-07 tonhuisman: Re-order device configuration to use PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR
 *                        Enable PluginStats feature
 * 2022-10-30 tonhuisman: Add support for 'missing' AXP192 pins, not used in M5Stack StickC but used in Core/Core2
 *                        Add support for Pre-intialize of plugins, so plugins can be initialized before SPI is enabled.
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
 * axp,ldoio,<voltage>        : Set voltage for AXP192 LDOIO (GPIO) pins
 * axp,gpio0,<state>          : Set state for AXP192 GPIO0 pin (0/1)
 * axp,gpio1,<state>          : Set state for AXP192 GPIO1 pin
 * axp,gpio2,<state>          : Set state for AXP192 GPIO2 pin
 * axp,gpio3,<state>          : Set state for AXP192 GPIO3 pin
 * axp,gpio4,<state>          : Set state for AXP192 GPIO4 pin
 * axp,dcdc2,<voltage>        : Set voltage for AXP192 DCDC2 pin
 * axp,dcdc3,<voltage>        : Set voltage for AXP192 DCDC3 pin
 * axp,ldo2map,<low>,<high>   : Set mapping range for percentage 0..100, <low> >= 0, <high> <= 3300
 * axp,ldo3map,<low>,<high>   : Set mapping range for percentage 0..100
 * axp,ldoiomap,<low>,<high>  : Set mapping range for percentage 1..100, when 0 the command will fail.
 * axp,dcdc2map,<low>,<high>  : Set mapping range for percentage 0..100
 * axp,dcdc3map,<low>,<high>  : Set mapping range for percentage 0..100
 * axp,ldo2perc,<percentage>  : Set voltage to percentage of mapped values, 0..100, 0 = off, 1..100 maps to map range
 * axp,ldo3perc,<percentage>  : Set voltage to percentage of mapped values
 * axp,ldoioperc,<percentage> : Set voltage to percentage of mapped values
 * axp,dcdc2perc,<percentage> : Set voltage to percentage of mapped values
 * axp,dcdc3perc,<percentage> : Set voltage to percentage of mapped values
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
 * [<taskname>#ldoiovolt]     : LDOIO voltage
 * [<taskname>#dcdc2volt]     : DCDC2 voltage
 * [<taskname>#dcdc3volt]     : DCDC3 voltage
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
      #  ifndef BUILD_NO_DEBUG
      addLogMove(LOG_LEVEL_DEBUG, F("P137: PLUGIN_PRIORITY_INIT"));
      #  endif // ifndef BUILD_NO_DEBUG
      success = Settings.isPowerManagerTask(event->TaskIndex); // Are we the PowerManager task?
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

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = I2C_AXP192_DEFAULT_ADDRESS;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      PCONFIG(0)                      = static_cast<int>(P137_valueOptions_e::VbusVoltage);
      PCONFIG(1)                      = static_cast<int>(P137_valueOptions_e::VbusCurrent);
      PCONFIG(2)                      = static_cast<int>(P137_valueOptions_e::BatteryVoltage);
      PCONFIG(3)                      = static_cast<int>(P137_valueOptions_e::InternalTemperature);
      PCONFIG(P137_SENSOR_TYPE_INDEX) = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_QUAD);
      P137_CONFIG_DECIMALS            = 2; // 2 decimals for all get config values
      P137_CONFIG_PREDEFINED          = 2; // M5Stack Core2
      P137_CheckPredefinedParameters(event);
      P137_CONFIG_PREDEFINED = 0;          // Settings applied

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("Decimals for config values"), F("decimals"), P137_CONFIG_DECIMALS, 0, 4);

      addFormSubHeader(F("Hardware outputs AXP192"));

      {
        P137_CheckPredefinedParameters(event);
        const __FlashStringHelper *predefinedNames[] = {
          toString(P137_PredefinedDevices_e::Unselected),
          toString(P137_PredefinedDevices_e::M5Stack_StickC),
          toString(P137_PredefinedDevices_e::M5Stack_Core2),
          toString(P137_PredefinedDevices_e::LilyGO_TBeam),
          toString(P137_PredefinedDevices_e::UserDefined) // keep last and at 99 !!
        };
        const int predefinedValues[] = {
          static_cast<int>(P137_PredefinedDevices_e::Unselected),
          static_cast<int>(P137_PredefinedDevices_e::M5Stack_StickC),
          static_cast<int>(P137_PredefinedDevices_e::M5Stack_Core2),
          static_cast<int>(P137_PredefinedDevices_e::LilyGO_TBeam),
          static_cast<int>(P137_PredefinedDevices_e::UserDefined) }; // keep last and at 99 !!
        addFormSelector(F("Predefined device configuration"), F("predef"),
                        sizeof(predefinedValues) / sizeof(int),
                        predefinedNames, predefinedValues, 0, !Settings.isPowerManagerTask(event->TaskIndex));

        if (!Settings.isPowerManagerTask(event->TaskIndex)) {
          addFormNote(F("Page will reload when selection is changed."));
        }

        if (static_cast<P137_PredefinedDevices_e>(P137_CURRENT_PREDEFINED) != P137_PredefinedDevices_e::Unselected) {
          String note;
          note.reserve(55);
          note += F("Last selected: ");
          note += toString(static_cast<P137_PredefinedDevices_e>(P137_CURRENT_PREDEFINED));
          addFormNote(note);
        }
      }
      const __FlashStringHelper *notConnected = F("N/C - Unused");
      {
        const __FlashStringHelper *ldoioRangeUnit = F("range 1800 - 3300mV");

        addFormNumericBox(F("LDO2"), F("pldo2"), P137_GET_CONFIG_LDO2, -1, P137_CONST_MAX_LDO,
                          #  if FEATURE_TOOLTIPS
                          EMPTY_STRING,
                          #  endif // if FEATURE_TOOLTIPS
                          bitRead(P137_CONFIG_DISABLEBITS, 0));
        addUnit(bitRead(P137_CONFIG_DISABLEBITS, 0) && (P137_GET_CONFIG_LDO2 == -1) ? notConnected : ldoioRangeUnit);
        addFormNumericBox(F("LDO3"), F("pldo3"), P137_GET_CONFIG_LDO3, -1, P137_CONST_MAX_LDO,
                          #  if FEATURE_TOOLTIPS
                          EMPTY_STRING,
                          #  endif // if FEATURE_TOOLTIPS
                          bitRead(P137_CONFIG_DISABLEBITS, 1));
        addUnit(bitRead(P137_CONFIG_DISABLEBITS, 1) && (P137_GET_CONFIG_LDO3 == -1) ? notConnected : ldoioRangeUnit);
        addFormNumericBox(F("GPIO LDO (LDOIO)"), F("ldoiovolt"), P137_GET_CONFIG_LDOIO, -1, P137_CONST_MAX_LDOIO,
                          #  if FEATURE_TOOLTIPS
                          EMPTY_STRING,
                          #  endif // if FEATURE_TOOLTIPS
                          bitRead(P137_CONFIG_DISABLEBITS, 2));
        addUnit(bitRead(P137_CONFIG_DISABLEBITS, 2) && (P137_GET_CONFIG_LDOIO == -1) ? notConnected : ldoioRangeUnit);

        addFormNumericBox(F("DCDC2"), F("pdcdc2"), P137_GET_CONFIG_DCDC2, -1, P137_CONST_MAX_DCDC2,
                          #  if FEATURE_TOOLTIPS
                          EMPTY_STRING,
                          #  endif // if FEATURE_TOOLTIPS
                          bitRead(P137_CONFIG_DISABLEBITS, 8));
        addUnit(bitRead(P137_CONFIG_DISABLEBITS, 8) && (P137_GET_CONFIG_DCDC2 == -1) ? notConnected : F("range 700 - 2750mV"));
        addFormNumericBox(F("DCDC3"), F("pdcdc3"), P137_GET_CONFIG_DCDC3, -1, P137_CONST_MAX_DCDC,
                          #  if FEATURE_TOOLTIPS
                          EMPTY_STRING,
                          #  endif // if FEATURE_TOOLTIPS
                          bitRead(P137_CONFIG_DISABLEBITS, 9));
        addUnit(bitRead(P137_CONFIG_DISABLEBITS, 9) && (P137_GET_CONFIG_DCDC3 == -1) ? notConnected : F("range 700 - 3500mV"));

        addFormNote(F("Values &lt; min. range will switch off the output. Set to -1 to not initialize/unused."));
        addFormNote(F("Check your device documentation for what is connected to each output."));
      }

      {
        const __FlashStringHelper *bootStates[] = {
          toString(P137_GPIOBootState_e::Default),
          toString(P137_GPIOBootState_e::Output_low),
          toString(P137_GPIOBootState_e::Output_high),
          toString(P137_GPIOBootState_e::Input),
          toString(P137_GPIOBootState_e::PWM),
        };
        const int bootStateValues[] = {
          static_cast<int>(P137_GPIOBootState_e::Default),
          static_cast<int>(P137_GPIOBootState_e::Output_low),
          static_cast<int>(P137_GPIOBootState_e::Output_high),
          static_cast<int>(P137_GPIOBootState_e::Input),
          static_cast<int>(P137_GPIOBootState_e::PWM),
        };
        const String bootStateAttributes[] = {
          F(""),
          F(""),
          F(""),
          F("disabled"),
          F("disabled"),
        };

        for (int i = 0; i < 5; i++) { // GPIO0..4
          const String id = concat(F("pgpio"), i);
          addRowLabel(concat(F("Initial state GPIO"), i));
          addSelector(id, sizeof(bootStateValues) / sizeof(int),
                      bootStates, bootStateValues, bootStateAttributes,
                      get3BitFromUL(P137_CONFIG_FLAGS, i * 3),
                      false, !bitRead(P137_CONFIG_DISABLEBITS, i + 3), F("")
                      #  if FEATURE_TOOLTIPS
                      , EMPTY_STRING
                      #  endif // if FEATURE_TOOLTIPS
                      );

          if (bitRead(P137_CONFIG_DISABLEBITS, i + 3)) {
            addUnit(notConnected);
          }
        }
        addFormNote(F("This refers to AXP192 GPIO pins, <B>not</B> ESP GPIO pins!"));
      }

      {
        // Keep this setting hidden from the UI
        addHtml(F("<div hidden>"));
        addNumericBox(F("pbits"), P137_CONFIG_DISABLEBITS, 0, 0xFFFF);
        addHtml(F("</div>"));
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR:
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
        toString(P137_valueOptions_e::LDOIO),
        toString(P137_valueOptions_e::DCDC2),
        toString(P137_valueOptions_e::DCDC3),
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
        static_cast<int>(P137_valueOptions_e::LDOIO),
        static_cast<int>(P137_valueOptions_e::DCDC2),
        static_cast<int>(P137_valueOptions_e::DCDC3),
      };

      for (uint8_t i = 0; i < P137_NR_OUTPUT_VALUES; i++) {
        sensorTypeHelper_loadOutputSelector(event,
                                            P137_CONFIG_BASE + i,
                                            i,
                                            sizeof(valValues) / sizeof(int),
                                            valOptions,
                                            valValues);
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      for (uint8_t i = 0; i < P137_NR_OUTPUT_VALUES; i++) {
        sensorTypeHelper_saveOutputSelector(event, P137_CONFIG_BASE + i, i,
                                            toString(static_cast<P137_valueOptions_e>(PCONFIG(P137_CONFIG_BASE + i)), false));
      }

      P137_REG_DCDC2_LDO2 = (P137_valueToSetting(getFormItemInt(F("pdcdc2")), P137_CONST_MAX_DCDC2) << 16) |
                            P137_valueToSetting(getFormItemInt(F("pldo2")), P137_CONST_MAX_LDO);
      P137_REG_DCDC3_LDO3 = (P137_valueToSetting(getFormItemInt(F("pdcdc3")), P137_CONST_MAX_DCDC) << 16) |
                            P137_valueToSetting(getFormItemInt(F("pldo3")), P137_CONST_MAX_LDO);
      P137_REG_LDOIO = P137_valueToSetting(getFormItemInt(F("ldoiovolt")), P137_CONST_MAX_LDOIO);

      for (int i = 0; i < 5; i++) { // GPIO0..4
        P137_SET_GPIO_FLAGS(i, getFormItemInt(concat(F("pgpio"), i)));
      }

      P137_CONFIG_DECIMALS    = getFormItemInt(F("decimals"));
      P137_CONFIG_PREDEFINED  = getFormItemInt(F("predef"));
      P137_CONFIG_DISABLEBITS = getFormItemInt(F("pbits"), static_cast<int>(P137_CONFIG_DISABLEBITS)); // Keep previous value if not found

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      P137_data_struct *P137_init = static_cast<P137_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P137_init) {
        #  ifndef BUILD_NO_DEBUG
        addLogMove(LOG_LEVEL_INFO, F("P137: Already initialized, skipped."));
        #  endif // ifndef BUILD_NO_DEBUG
        // has been initialized so nothing to do here
        success = true; // Still was successful (to keep plugin enabled!)
      } else {
        #  ifndef BUILD_NO_DEBUG
        addLogMove(LOG_LEVEL_DEBUG, F("P137: PLUGIN_INIT"));
        #  endif // ifndef BUILD_NO_DEBUG
        initPluginTaskData(event->TaskIndex, new (std::nothrow) P137_data_struct(event));
        P137_data_struct *P137_data = static_cast<P137_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != P137_data) {
          success = true;
        }
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
