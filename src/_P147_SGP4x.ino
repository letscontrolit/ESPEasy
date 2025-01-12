#include "_Plugin_Helper.h"
#ifdef USES_P147

// #######################################################################################################
// ############################ Plugin 147: Gases - SGP4x CO2 (VOC), NOx (SGP41) #########################
// #######################################################################################################

/** Changelog:
 * 2023-05-07 tonhuisman: Make Temperature and Humidity compensation selection independent, so if either setting is configured
 *                        it will still be applied, with the other value using the default. Minor UI improvement.
 * 2023-05-02 tonhuisman: Fix Low-power measurement, introducing a new State for reading the second measurement only
 *                        Remove unsupported sampling interval from MOxGasIndexAlgorithm
 *                        Fix settings-save issue
 * 2023-05-01 tonhuisman: Adjust timing for reading SGP41 (needs a little more time than SGP40, according to the specs)
 * 2023-05-01 tonhuisman: Change Calibration to Compensation
 * 2023-04-30 tonhuisman: Ignore first read data (invalid) in low-power mode
 *                        Implement Sensirion GasIndexAlgorithm library for VOC and NOx index. (compile-time option)
 *                        When disabled, only raw values are available, so no need to have a Raw setting
 *                        Finish implementation (different commands) for SGP41
 * 2023-04-30 tonhuisman: Rename plugin to Gases - SGP4x VOC(/NOx)
 * 2023-04-29 tonhuisman: Implement sensor raw reading
 * 2023-04-27 tonhuisman: Start of plugin
 *
 * Using direct I2C functions and Sensirion VOC/NOx index calculation library
 **/

/** Commands supported:
 * (None yet)
 */

/** Get Config values:
 * [<taskname>#Serialnumber]    : Returns the serial number of the connected sensor.
 * [<taskname>#RawVOC]          : Returns the last measured raw VOC value
 * [<taskname>#RawNOx]          : Returns the last measured raw NOx value
 */

# define PLUGIN_147
# define PLUGIN_ID_147          147
# define PLUGIN_NAME_147        "Gases - SGP4x VOC(/NOx)"
# define PLUGIN_VALUENAME1_147  "VOC"
# define PLUGIN_VALUENAME2_147  "NOx"

# include "./src/PluginStructs/P147_data_struct.h"

boolean Plugin_147(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_147;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_SINGLE;
      dev.FormulaOption  = true;
      dev.ValueCount     = 2;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.PluginStats    = true;

      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_147);

      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_147));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_147));

      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = P147_SENSOR_TYPE == static_cast<int>(P147_sensor_e::SGP41) ? 2 : 1; // SGP41 also has NOx value
      success     = true;
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      success = event->Par1 == P147_I2C_ADDRESS;
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P147_I2C_ADDRESS;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      ExtraTaskSettings.TaskDeviceValueDecimals[0] = 0; // VOC index value is an integer
      ExtraTaskSettings.TaskDeviceValueDecimals[1] = 0; // NOx index value is an integer
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        const __FlashStringHelper *sensorTypes[] = {
          F("SGP40"),
          F("SGP41"),
        };
        const int sensorTypeOptions[] = {
          static_cast<int>(P147_sensor_e::SGP40),
          static_cast<int>(P147_sensor_e::SGP41),
        };
        constexpr size_t optionCount = NR_ELEMENTS(sensorTypeOptions);
        addFormSelector(F("Sensor model"), F("ptype"), optionCount, sensorTypes, sensorTypeOptions, P147_SENSOR_TYPE, true);
        # ifndef BUILD_NO_DEBUG
        addFormNote(F("Page will reload on change."));
        # endif // ifndef BUILD_NO_DEBUG
      }

      addFormSelector_YesNo(F("Use Compensation"), F("comp"), P147_GET_USE_COMPENSATION, true);
      # ifndef BUILD_NO_DEBUG
      addFormNote(F("Page will reload on change."));
      # endif // ifndef BUILD_NO_DEBUG

      if (P147_GET_USE_COMPENSATION) {
        addRowLabel(F("Temperature Task"));
        addTaskSelect(F("ttask"), P147_TEMPERATURE_TASK);

        if (validTaskIndex(P147_TEMPERATURE_TASK)) {
          addRowLabel(F("Temperature Value"));
          addTaskValueSelect(F("tvalue"), P147_TEMPERATURE_VALUE, P147_TEMPERATURE_TASK);
        }

        addRowLabel(F("Humidity Task"));
        addTaskSelect(F("htask"), P147_HUMIDITY_TASK);

        if (validTaskIndex(P147_HUMIDITY_TASK)) {
          addRowLabel(F("Humidity Value"));
          addTaskValueSelect(F("hvalue"), P147_HUMIDITY_VALUE, P147_HUMIDITY_TASK);
        }
      }

      addFormSeparator(2);

      addFormCheckBox(F("Low-power measurement"), F("plow"), P147_LOW_POWER_MEASURE == 1);
      addFormNote(F("Unchecked= 1 sec., continuous heating, Checked= 10 sec. measurement interval."));

      # if P147_FEATURE_GASINDEXALGORITHM
      addFormCheckBox(F("Show raw data only"), F("raw"), P147_GET_RAW_DATA_ONLY);
      # endif // if P147_FEATURE_GASINDEXALGORITHM

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      const int prevSensor = P147_SENSOR_TYPE;
      P147_SENSOR_TYPE       = getFormItemInt(F("ptype"));
      P147_LOW_POWER_MEASURE = isFormItemChecked(F("plow")) ? 1 : 0;
      P147_SET_USE_COMPENSATION(getFormItemInt(F("comp")));
      # if P147_FEATURE_GASINDEXALGORITHM
      P147_SET_RAW_DATA_ONLY(isFormItemChecked(F("raw")));
      # endif // if P147_FEATURE_GASINDEXALGORITHM

      if (P147_GET_USE_COMPENSATION) {
        P147_TEMPERATURE_TASK  = getFormItemInt(F("ttask"));
        P147_TEMPERATURE_VALUE = getFormItemInt(F("tvalue"));
        P147_HUMIDITY_TASK     = getFormItemInt(F("htask"));
        P147_HUMIDITY_VALUE    = getFormItemInt(F("hvalue"));
      }

      if ((prevSensor != P147_SENSOR_TYPE) && (P147_SENSOR_TYPE == static_cast<int>(P147_sensor_e::SGP41))) {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_147));
        ExtraTaskSettings.TaskDeviceValueDecimals[1] = 0; // NOx index value is an integer
      }
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P147_data_struct(event));
      P147_data_struct *P147_data = static_cast<P147_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P147_data) && P147_data->init(event);

      break;
    }

    case PLUGIN_TASKTIMER_IN:
    {
      P147_data_struct *P147_data = static_cast<P147_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P147_data) {
        success = P147_data->plugin_tasktimer_in(event);
      }

      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      P147_data_struct *P147_data = static_cast<P147_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P147_data) {
        success = P147_data->plugin_once_a_second(event);
      }

      break;
    }

    case PLUGIN_READ:
    {
      P147_data_struct *P147_data = static_cast<P147_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P147_data) {
        success = P147_data->plugin_read(event);
      }

      break;
    }

    case PLUGIN_WRITE:
    {
      P147_data_struct *P147_data = static_cast<P147_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P147_data) {
        success = P147_data->plugin_write(event, string);
      }

      break;
    }

    case PLUGIN_GET_CONFIG_VALUE:
    {
      P147_data_struct *P147_data = static_cast<P147_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P147_data) {
        success = P147_data->plugin_get_config_value(event, string);
      }

      break;
    }
  }
  return success;
}

#endif // USES_P147
