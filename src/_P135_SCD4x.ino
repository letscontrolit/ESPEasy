#include "_Plugin_Helper.h"
#ifdef USES_P135

// #######################################################################################################
// ########################## Plugin 135: Gases - SCD4x CO2, Humidity, Temperature #######################
// #######################################################################################################

/**
 * 2022-08-28 tonhuisman: Include 'CO2' in plugin name, to be in line with other CO2 plugins
 * 2022-08-24 tonhuisman: Removed [TESTING] tag
 * 2022-08-04 tonhuisman: Add forced recalibration subcommand scd4x,setfrc,<frcvalue>
 *                        Add [<taskname>#SerialNumber] get config value
 * 2022-08-04 tonhuisman: Stop plugin after 100 consecutive mis-readings have happened, as then most likely
 *                        something is wrong with the sensor. The CO2 value will be set to 0 in that case.
 *                        Describe Stats option in the documentation.
 * 2022-08-03 tonhuisman: Add single-shot measurement mode for SCD41, commands and get config values
 *                        Changed category from Environment to Gases, to match with other CO2 sensors
 * 2022-08-02 tonhuisman: Start plugin for SCD4x (SCD40/SDC41) CO2, Humidity and Temperature
 *                        From a forum request: https://www.letscontrolit.com/forum/viewtopic.php?t=9166
 *
 * Using https://github.com/sparkfun/SparkFun_SCD4x_Arduino_Library (SparkFun SCD4x Arduino Library)
 **/
# define PLUGIN_135
# define PLUGIN_ID_135          135
# define PLUGIN_NAME_135        "Gases - CO2 SCD4x"
# define PLUGIN_VALUENAME1_135  "CO2"
# define PLUGIN_VALUENAME2_135  "Humidity"
# define PLUGIN_VALUENAME3_135  "Temperature"

# include "./src/PluginStructs/P135_data_struct.h"

boolean Plugin_135(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_135;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TRIPLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 3;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].PluginStats        = true;

      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_135);

      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_135));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_135));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_135));

      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      success = event->Par1 == 0x62;
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = 0x62;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      ExtraTaskSettings.TaskDeviceValueDecimals[0] = 0; // CO2 value is an integer
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        const __FlashStringHelper *sensorTypes[] = {
          F("SCD40"),
          F("SCD41"),
        };
        const int sensorTypeOptions[] = {
          static_cast<int>(scd4x_sensor_type_e::SCD4x_SENSOR_SCD40),
          static_cast<int>(scd4x_sensor_type_e::SCD4x_SENSOR_SCD41),
        };
        addFormSelector(F("Sensor model"), F("ptype"), 2, sensorTypes, sensorTypeOptions, P135_SENSOR_TYPE, true);
        # ifndef LIMIT_BUILD_SIZE
        addFormNote(F("Page will reload on change."));
        # endif // ifndef LIMIT_BUILD_SIZE
      }

      addFormNumericBox(F("Altitude"), F("altitude"), P135_SENSOR_ALTITUDE, 0, 2000);
      addUnit(F("0..2000 m"));

      addFormTextBox(F("Temp offset"), F("tempoffset"), toString(P135_TEMPERATURE_OFFSET, 2), 5);
      addUnit(F("&deg;C"));

      addFormCheckBox(F("Low-power measurement"), F("pinterval"), P135_MEASURE_INTERVAL == 1);
      addFormNote(F("Unchecked= 5 sec. Checked= 30 sec. measuring duration."));

      if (P135_SENSOR_TYPE == static_cast<int>(scd4x_sensor_type_e::SCD4x_SENSOR_SCD41)) {
        addFormCheckBox(F("Single-shot measurements (SCD41 only)"), F("singleshot"), P135_MEASURE_SINGLE_SHOT == 1);
        # ifndef LIMIT_BUILD_SIZE
        addFormNote(F("When enabled will start a single measurement every Interval, duration 5 sec."));
        # endif // ifndef LIMIT_BUILD_SIZE
      }

      addFormCheckBox(F("Automatic Self Calibration"), F("autocal"), P135_AUTO_CALIBRATION == 1);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P135_SENSOR_TYPE = getFormItemInt(F("ptype"));
      uint16_t alt = getFormItemInt(F("altitude"));

      if (alt > 2000) { alt = 2000; }
      P135_SENSOR_ALTITUDE    = alt;
      P135_TEMPERATURE_OFFSET = getFormItemFloat(F("tempoffset"));
      P135_AUTO_CALIBRATION   = isFormItemChecked(F("autocal")) ? 1 : 0;
      P135_MEASURE_INTERVAL   = isFormItemChecked(F("pinterval")) ? 1 : 0;

      if (P135_SENSOR_TYPE == static_cast<int>(scd4x_sensor_type_e::SCD4x_SENSOR_SCD41)) {
        P135_MEASURE_SINGLE_SHOT = isFormItemChecked(F("singleshot")) ? 1 : 0;
      }
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P135_data_struct(event->TaskIndex,
                                                                               P135_SENSOR_TYPE,
                                                                               P135_SENSOR_ALTITUDE,
                                                                               P135_TEMPERATURE_OFFSET,
                                                                               P135_AUTO_CALIBRATION == 1,
                                                                               P135_MEASURE_INTERVAL == 1,
                                                                               (P135_MEASURE_SINGLE_SHOT == 1) &&
                                                                               (P135_SENSOR_TYPE ==
                                                                                static_cast<int>(scd4x_sensor_type_e::SCD4x_SENSOR_SCD41))));
      P135_data_struct *P135_data = static_cast<P135_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P135_data) && P135_data->init();

      break;
    }

    case PLUGIN_READ:
    {
      P135_data_struct *P135_data = static_cast<P135_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P135_data) {
        success = P135_data->plugin_read(event);
      }

      break;
    }

    case PLUGIN_WRITE:
    {
      P135_data_struct *P135_data = static_cast<P135_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P135_data) {
        success = P135_data->plugin_write(event, string);
      }

      break;
    }

    case PLUGIN_GET_CONFIG_VALUE:
    {
      P135_data_struct *P135_data = static_cast<P135_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P135_data) {
        success = P135_data->plugin_get_config_value(event, string);
      }

      break;
    }
  }
  return success;
}

#endif // USES_P135
