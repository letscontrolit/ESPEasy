#include "_Plugin_Helper.h"
#ifdef USES_P142

// #######################################################################################################
// ############################# Plugin 142: Position - AS5600 Megnetic angle ############################
// #######################################################################################################

/** Changelog:
 * 2024-06-18 tonhuisman: All settings implemented, values and command added.
 *                        Settings for output pin configuration (analog/pwm etc.) not implemented.
 * 2024-06-15 tonhuisman: Start plugin for AS5600 Magnetic angle sensor using RobTillaart/AS5600 library
 * (Newest changes on top)
 **/

/** Commands:
 * as5600,dir,<value>     : Set the direction 0 = Clockwise, 1 = Counterclockwise
 */

/** Get Config values:
 * [<taskname>#angle]     : The calculated angle (0..360)
 * [<taskname>#magnitude] : The detected magnet strength
 * [<taskname>#rpm]       : The measured rotational speed of the magnet
 * [<taskname>#readangle] : The read angle value with applied corrections and filters (0..4095)
 * [<taskname>#rawangle]  : The raw angle value (0..4095)
 * [<taskname>#agc]       : The current automatic gain control value value (0..128 for 3.3V, 0..255 for 5V powered devices)
 * [<taskname>#hasmagnet] : Is a magnet detected (1 or 0)
 */

# define PLUGIN_142
# define PLUGIN_ID_142          142
# define PLUGIN_NAME_142        "Position - AS5600(L) Magnetic angle"
# define PLUGIN_VALUENAME1_142  "Angle"
# define PLUGIN_VALUENAME2_142  "Direction"
# define PLUGIN_VALUENAME3_142  "Rpm"

# include "./src/PluginStructs/P142_data_struct.h"

boolean Plugin_142(uint8_t function, struct EventStruct *event, String& string)
{
  const __FlashStringHelper *angleWarning = F("Angle between Start position and Max position must be &gt; than 18 &deg; (205 steps)");
  boolean success                         = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_142;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_QUAD;
      dev.OutputDataType = Output_Data_type_t::Simple;
      dev.FormulaOption  = true;
      dev.ValueCount     = 4;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.TimerOptional  = true;
      dev.PluginStats    = true;

      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_142);

      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      const int valueCount = P142_NR_OUTPUT_VALUES;

      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        if (i < valueCount) {
          const uint8_t pconfigIndex = i + P142_QUERY1_CONFIG_POS;
          const uint8_t option       = PCONFIG(pconfigIndex);
          ExtraTaskSettings.setTaskDeviceValueName(i, Plugin_142_valuename(option, false));
        } else {
          ExtraTaskSettings.clearTaskDeviceValueName(i);
        }
      }
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x36, 0x40 };

      if (PLUGIN_WEBFORM_SHOW_I2C_PARAMS == function) {
        addFormSelectorI2C(F("i2c_addr"), NR_ELEMENTS(i2cAddressValues), i2cAddressValues, P142_I2C_ADDRESS);
        addFormNote(F("AS5600 = 0x36, AS5600L = 0x40"));
      } else {
        success = intArrayContains(NR_ELEMENTS(i2cAddressValues), i2cAddressValues, event->Par1);
      }

      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P142_I2C_ADDRESS;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      PCONFIG(0) = P142_OUTPUT_ANGLE;
      PCONFIG(1) = P142_OUTPUT_MAGNET;
      PCONFIG(2) = P142_OUTPUT_RPM;
      PCONFIG(3) = P142_OUTPUT_RAW_ANGLE;

      PCONFIG(P142_SENSOR_TYPE_INDEX) = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_QUAD);
      P142_I2C_ADDRESS                = 0x36;
      P142_SET_OUTPUT_MODE(AS5600_MODE_DEGREES);
      P142_SET_COUNTER_CLOCKWISE(AS5600_CLOCK_WISE);
      P142_START_POSITION = 0;
      P142_MAX_POSITION   = 0;
      P142_PRESET_ANGLE   = 0.0f;

      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = P142_NR_OUTPUT_VALUES;
      success     = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = static_cast<Sensor_VType>(PCONFIG(P142_SENSOR_TYPE_INDEX));
      event->idx        = P142_SENSOR_TYPE_INDEX;
      success           = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR:
    {
      const uint8_t optionCount = P142_NR_OUTPUT_OPTIONS;
      String options[optionCount];

      for (uint8_t option = 0; option < optionCount; ++option) {
        options[option] = Plugin_142_valuename(option, true);
      }

      const int valueCount = P142_NR_OUTPUT_VALUES;

      for (uint8_t i = 0; i < valueCount; ++i) {
        const uint8_t pconfigIndex = i + P142_QUERY1_CONFIG_POS;
        sensorTypeHelper_loadOutputSelector(event, pconfigIndex, i, optionCount, options);
      }

      success = true;

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        const __FlashStringHelper *configurations[] = {
          F("Degrees"),
          F("Radians"),
        };
        const int configurationOptions[] = {
          AS5600_MODE_DEGREES,
          AS5600_MODE_RADIANS,
        };
        constexpr size_t optionCount = NR_ELEMENTS(configurationOptions);
        addFormSelector(F("Output range"),
                        F("range"),
                        optionCount,
                        configurations,
                        configurationOptions,
                        P142_GET_OUTPUT_MODE);
      }
      addFormCheckBox(F("Generate Events only when changed"),         F("diff"), P142_GET_UPDATE_DIFF_ONLY);
      addFormCheckBox(F("Generate Events only when magnet detected"), F("cmag"), P142_GET_CHECK_MAGNET);
      addFormCheckBox(F("Log basic data (Info)"),                     F("plog"), P142_GET_ENABLE_LOG);

      addFormSubHeader(F("Sensor setup"));

      addFormCheckBox(F("Direction Counter-clockwise"), F("pdir"), P142_GET_COUNTER_CLOCKWISE == AS5600_COUNTERCLOCK_WISE);

      addFormNumericBox(F("Start position"), F("pstart"), P142_START_POSITION, 0, 4095);
      addUnit(F("0..4095"));
      addFormNumericBox(F("Max position"), F("pmax"), P142_MAX_POSITION, 0, 4095);
      addUnit(F("0..4095"));
      addFormNote(angleWarning);

      addFormFloatNumberBox(F("Angle offset"), F("pangle"), P142_PRESET_ANGLE, -359.99f, 359.99f, 2);
      addUnit(F("-359.99..359.99 &deg;"));

      addFormSubHeader(F("Power management"));
      {
        const __FlashStringHelper *configurations[] = {
          F("Normal"),
          F("Low power mode 1"),
          F("Low power mode 2"),
          F("Low power mode 3"),
        };
        const int configurationOptions[] = {
          AS5600_POWERMODE_NOMINAL,
          AS5600_POWERMODE_LOW1,
          AS5600_POWERMODE_LOW2,
          AS5600_POWERMODE_LOW3,
        };
        constexpr size_t optionCount = NR_ELEMENTS(configurationOptions);
        addFormSelector(F("Power mode"),
                        F("pow"),
                        optionCount,
                        configurations,
                        configurationOptions,
                        P142_GET_POWER_MODE);
      }
      addFormCheckBox(F("Power watchdog"), F("wdog"), P142_GET_WATCHDOG);
      addFormNote(F("Switches to 'Low power mode 3' after 1 minute of less than 4 LSBs change"));

      addFormSubHeader(F("Sensitivity & filtering"));
      {
        const __FlashStringHelper *configurations[] = {
          F("Off"),
          F("1 LSB"),
          F("2 LSBs"),
          F("3 LSBs"),
        };
        const int configurationOptions[] = {
          AS5600_HYST_OFF,
          AS5600_HYST_LSB1,
          AS5600_HYST_LSB2,
          AS5600_HYST_LSB3,
        };
        constexpr size_t optionCount = NR_ELEMENTS(configurationOptions);
        addFormSelector(F("Hysteresis"),
                        F("hyst"),
                        optionCount,
                        configurations,
                        configurationOptions,
                        P142_GET_HYSTERESIS);
      }
      {
        const __FlashStringHelper *configurations[] = {
          F("16x"),
          F("8x"),
          F("4x"),
          F("2x"),
        };
        const int configurationOptions[] = {
          AS5600_SLOW_FILT_16X,
          AS5600_SLOW_FILT_8X,
          AS5600_SLOW_FILT_4X,
          AS5600_SLOW_FILT_2X,
        };
        constexpr size_t optionCount = NR_ELEMENTS(configurationOptions);
        addFormSelector(F("Slow filter"),
                        F("sflt"),
                        optionCount,
                        configurations,
                        configurationOptions,
                        P142_GET_SLOW_FILTER);
      }
      {
        const __FlashStringHelper *configurations[] = {
          F("Slow filter only"),
          F("6 LSBs"),
          F("7 LSBs"),
          F("9 LSBs"),
          F("18 LSBs"),
          F("21 LSBs"),
          F("24 LSBs"),
          F("10 LSBs"),
        };
        const int configurationOptions[] = {
          AS5600_FAST_FILT_NONE,
          AS5600_FAST_FILT_LSB6,
          AS5600_FAST_FILT_LSB7,
          AS5600_FAST_FILT_LSB9,
          AS5600_FAST_FILT_LSB18,
          AS5600_FAST_FILT_LSB21,
          AS5600_FAST_FILT_LSB24,
          AS5600_FAST_FILT_LSB10,
        };
        constexpr size_t optionCount = NR_ELEMENTS(configurationOptions);
        addFormSelector(F("Fast filter"),
                        F("fflt"),
                        optionCount,
                        configurations,
                        configurationOptions,
                        P142_GET_FAST_FILTER);
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P142_I2C_ADDRESS = getFormItemInt(F("i2c_addr"));
      P142_SET_OUTPUT_MODE(getFormItemInt(F("range")));
      P142_SET_UPDATE_DIFF_ONLY(isFormItemChecked(F("diff")));
      P142_SET_COUNTER_CLOCKWISE(isFormItemChecked(F("pdir")) ? AS5600_COUNTERCLOCK_WISE : AS5600_CLOCK_WISE);
      P142_START_POSITION = getFormItemInt(F("pstart"));
      P142_MAX_POSITION   = getFormItemInt(F("pmax"));
      P142_PRESET_ANGLE   = getFormItemFloat(F("pangle"));
      P142_SET_POWER_MODE(getFormItemInt(F("pow")));
      P142_SET_HYSTERESIS(getFormItemInt(F("hyst")));
      P142_SET_SLOW_FILTER(getFormItemInt(F("sflt")));
      P142_SET_FAST_FILTER(getFormItemInt(F("fflt")));
      P142_SET_WATCHDOG(isFormItemChecked(F("wdog")));
      P142_SET_ENABLE_LOG(isFormItemChecked(F("plog")));
      P142_SET_CHECK_MAGNET(isFormItemChecked(F("cmag")));

      success = true;

      // Validate angle between start position and max position
      if ((0 != P142_START_POSITION) && (0 != P142_MAX_POSITION) &&
          definitelyLessThan(
            (P142_MAX_POSITION * AS5600_RAW_TO_DEGREES) - (P142_MAX_POSITION * AS5600_RAW_TO_DEGREES),
            18.0f
            )) {
        P142_START_POSITION = 0;
        P142_MAX_POSITION   = 0;
        addHtmlError(angleWarning);
        success = false;
      }

      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P142_data_struct(event));
      P142_data_struct *P142_data = static_cast<P142_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P142_data) && P142_data->init(event);

      break;
    }

    case PLUGIN_READ:
    {
      P142_data_struct *P142_data = static_cast<P142_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P142_data) {
        success = P142_data->plugin_read(event);
      }

      break;
    }

    // case PLUGIN_TEN_PER_SECOND:
    case PLUGIN_ONCE_A_SECOND:
    {
      P142_data_struct *P142_data = static_cast<P142_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P142_data) {
        success = P142_data->plugin_ten_per_second(event);
      }

      break;
    }

    case PLUGIN_WRITE:
    {
      P142_data_struct *P142_data = static_cast<P142_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P142_data) {
        success = P142_data->plugin_write(event, string);
      }

      break;
    }

    case PLUGIN_GET_CONFIG_VALUE:
    {
      P142_data_struct *P142_data = static_cast<P142_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P142_data) {
        success = P142_data->plugin_get_config_value(event, string);
      }

      break;
    }
  }
  return success;
}

#endif // USES_P142
