#include "_Plugin_Helper.h"
#ifdef USES_P028

// #######################################################################################################
// #################### Plugin 028 BME280 I2C Temp/Hum/Barometric Pressure Sensor  #######################
// #######################################################################################################

/** Changelog:
 * 2023-07-27 tonhuisman: Revert most below changes and implement PLUGIN_GET_DEVICEVTYPE so the P2P controller validates against the correct
 *                        setting. Setting is only available if a remote data-feed is active, and offers BME280 and BMP280 options only.
 * 2023-07-26 tonhuisman: Ignore all humidity data (and log messages) if BMP280 Sensor model is selected
 * 2023-07-25 tonhuisman: Add setting to enable forcing the plugin into either BME280 or BMP280 mode, default is Auto-detect
 *                        Add changelog
 */

# include "src/PluginStructs/P028_data_struct.h"

// #include <math.h>

# define PLUGIN_028
# define PLUGIN_ID_028         28
# define PLUGIN_NAME_028       "Environment - BMx280"
# define PLUGIN_VALUENAME1_028 "Temperature"
# define PLUGIN_VALUENAME2_028 "Humidity"
# define PLUGIN_VALUENAME3_028 "Pressure"


boolean Plugin_028(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number           = PLUGIN_ID_028;
      dev.Type             = DEVICE_TYPE_I2C;
      dev.VType            = Sensor_VType::SENSOR_TYPE_TEMP_HUM_BARO;
      dev.FormulaOption    = true;
      dev.ValueCount       = 3;
      dev.SendDataOption   = true;
      dev.TimerOption      = true;
      dev.ErrorStateValues = true;
      dev.PluginStats      = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_028);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_028));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_028));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_028));
      break;
    }

    case PLUGIN_INIT_VALUE_RANGES:
    {
      // Min/Max values obtained from the BMP280/BME280 datasheets (both have equal ranges)
      ExtraTaskSettings.setAllowedRange(0, -40.0f, 85.0f);   // Temperature min/max
      ExtraTaskSettings.setAllowedRange(1, 0.0f,   100.0f);  // Humidity min/max
      ExtraTaskSettings.setAllowedRange(2, 300.0f, 1100.0f); // Barometric Pressure min/max

      switch (P028_ERROR_STATE_OUTPUT) {                     // Only temperature error is configurable
        case P028_ERROR_IGNORE:
          ExtraTaskSettings.setIgnoreRangeCheck(0);
          break;
        case P028_ERROR_MIN_RANGE:
          ExtraTaskSettings.TaskDeviceErrorValue[0] = ExtraTaskSettings.TaskDeviceMinValue[0] - 1.0f;
          break;
        case P028_ERROR_ZERO:
          ExtraTaskSettings.TaskDeviceErrorValue[0] = 0.0f;
          break;
        case P028_ERROR_MAX_RANGE:
          ExtraTaskSettings.TaskDeviceErrorValue[0] = ExtraTaskSettings.TaskDeviceMaxValue[0] + 1.0f;
          break;
        case P028_ERROR_NAN:
          ExtraTaskSettings.TaskDeviceErrorValue[0] = NAN;
          break;
        # ifndef LIMIT_BUILD_SIZE
        case P028_ERROR_MIN_K:
          ExtraTaskSettings.TaskDeviceErrorValue[0] = -274.0f;
          break;
        # endif // ifndef LIMIT_BUILD_SIZE
        default:
          break;
      }

      ExtraTaskSettings.TaskDeviceErrorValue[1] = -1.0f; // Humidity error
      ExtraTaskSettings.TaskDeviceErrorValue[2] = -1.0f; // Pressure error

      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      const P028_data_struct::BMx_DetectMode detectMode = static_cast<P028_data_struct::BMx_DetectMode>(P028_DETECTION_MODE);

      // We want to configure this only when a remote data-feed is used
      if ((Settings.TaskDeviceDataFeed[event->TaskIndex] != 0) && (P028_data_struct::BMx_DetectMode::BMP280 == detectMode)) {
        // Patch the sensor type to output only the measured values, and/or match with a P2P remote sensor
        event->sensorType = Sensor_VType::SENSOR_TYPE_TEMP_EMPTY_BARO;
        event->idx        = getValueCountFromSensorType(Sensor_VType::SENSOR_TYPE_TEMP_EMPTY_BARO);
      }

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      const float tempOffset = P028_TEMPERATURE_OFFSET / 10.0f;
      success = initPluginTaskData(
        event->TaskIndex,
        new (std::nothrow) P028_data_struct(P028_I2C_ADDRESS, tempOffset));

      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x76, 0x77 };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), 2, i2cAddressValues, P028_I2C_ADDRESS);
        addFormNote(F("SDO Low=0x76, High=0x77"));
      } else {
        success = intArrayContains(2, i2cAddressValues, event->Par1);
      }
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P028_I2C_ADDRESS;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_WEBFORM_LOAD:
    {
      bool wire_status      = false;
      const uint8_t chip_id = I2C_read8_reg(P028_I2C_ADDRESS, BMx280_REGISTER_CHIPID, &wire_status);

      if (wire_status) {
        addRowLabel(F("Detected Sensor Type"));
        addHtml(P028_data_struct::getDeviceName(static_cast<P028_data_struct::BMx_ChipId>(chip_id)));
      }

      addFormNumericBox(F("Altitude"), F("elev"), P028_ALTITUDE);
      addUnit('m');

      addFormNumericBox(F("Temperature offset"), F("tempoffset"), P028_TEMPERATURE_OFFSET);
      addUnit(F("x 0.1C"));
      String offsetNote = F("Offset in units of 0.1 degree Celsius");

      P028_data_struct *P028_data =
        static_cast<P028_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P028_data) {
        if ((P028_data_struct::BMx_DetectMode::BMP280 != static_cast<P028_data_struct::BMx_DetectMode>(P028_DETECTION_MODE)) &&
            P028_data->hasHumidity()) {
          offsetNote += F(" (also correct humidity)");
        }
      }
      addFormNote(offsetNote);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD_ALWAYS:
    {
      if (Settings.TaskDeviceDataFeed[event->TaskIndex] != 0) { // We want to configure this *only* when a remote data-feed is used
        const __FlashStringHelper *detectOptionList[] = {
          P028_data_struct::getDeviceName(P028_data_struct::BMx_ChipId::BME280_DEVICE),
          P028_data_struct::getDeviceName(P028_data_struct::BMx_ChipId::BMP280_DEVICE),
        };
        const int detectOptions[] = {
          static_cast<int>(P028_data_struct::BMx_DetectMode::BME280),
          static_cast<int>(P028_data_struct::BMx_DetectMode::BMP280),
        };
        addFormSelector(F("Output values mode"), F("det"), 2, detectOptionList, detectOptions, P028_DETECTION_MODE);

        success = true;
      }
      break;
    }


# if FEATURE_PLUGIN_STATS && FEATURE_CHART_JS
    case PLUGIN_WEBFORM_LOAD_SHOW_STATS:
    {
      P028_data_struct *P028_data =
        static_cast<P028_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P028_data) {
        if ((P028_data_struct::BMx_DetectMode::BMP280 != static_cast<P028_data_struct::BMx_DetectMode>(P028_DETECTION_MODE)) &&
            P028_data->hasHumidity())
        {
          P028_data->plot_ChartJS_scatter(
            0,
            1,
            F("temphumscatter"),
            { F("Temp/Humidity Scatter Plot") },
            { F("temp/hum"), F("rgb(255, 99, 132)") },
            500,
            500);
        }
      }

      // Do not set success = true, since we're not actually adding stats, but just plotting a scatter plot
      break;
    }
# endif // if FEATURE_PLUGIN_STATS && FEATURE_CHART_JS


    case PLUGIN_WEBFORM_SHOW_ERRORSTATE_OPT:
    {
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_INFO, F("BMx280: SHOW_ERRORSTATE_OPT"));
      # endif // ifndef BUILD_NO_DEBUG

      // Value in case of Error
      const __FlashStringHelper *resultsOptions[] = {
        F("Ignore"),
        F("Min -1 (-41&deg;C)"),
        F("0"),
        F("Max +1 (+86&deg;C)"),
        F("NaN"),
        # ifndef LIMIT_BUILD_SIZE
        F("-1&deg;K (-274&deg;C)")
        # endif // ifndef LIMIT_BUILD_SIZE
      };
      const int resultsOptionValues[] = {
        P028_ERROR_IGNORE,
        P028_ERROR_MIN_RANGE,
        P028_ERROR_ZERO,
        P028_ERROR_MAX_RANGE,
        P028_ERROR_NAN,
        # ifndef LIMIT_BUILD_SIZE
        P028_ERROR_MIN_K
        # endif // ifndef LIMIT_BUILD_SIZE
      };
      constexpr int P028_ERROR_STATE_COUNT = NR_ELEMENTS(resultsOptions);
      addFormSelector(F("Temperature Error Value"),
                      F("err"),
                      P028_ERROR_STATE_COUNT,
                      resultsOptions,
                      resultsOptionValues,
                      P028_ERROR_STATE_OUTPUT);

      break;
    }

    case PLUGIN_READ_ERROR_OCCURED:
    {
      // Called if PLUGIN_READ returns false
      // Function returns "true" when last measurement was an error.
      P028_data_struct *P028_data =
        static_cast<P028_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P028_data) {
        if (P028_data->lastMeasurementError) {
          success = true; // "success" may be a confusing name here
          string  = F("Sensor Not Found");
        }
      }
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P028_I2C_ADDRESS        = getFormItemInt(F("i2c_addr"));
      P028_ALTITUDE           = getFormItemInt(F("elev"));
      P028_TEMPERATURE_OFFSET = getFormItemInt(F("tempoffset"));
      P028_ERROR_STATE_OUTPUT = getFormItemInt(F("err"));

      if (Settings.TaskDeviceDataFeed[event->TaskIndex] != 0) { // We want to configure this only when a remote data-feed is used
        P028_DETECTION_MODE = getFormItemInt(F("det"));
      }
      success = true;
      break;
    }
    case PLUGIN_ONCE_A_SECOND:
    {
      P028_data_struct *P028_data =
        static_cast<P028_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P028_data) {
        if (P028_data->updateMeasurements(event->TaskIndex)) {
          // Update was succesfull, schedule a read.
          Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 10);
        }
      }
      break;
    }

    case PLUGIN_READ:
    {
      P028_data_struct *P028_data =
        static_cast<P028_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P028_data) {
        // PLUGIN_READ is called from `TaskRun` or on the set interval or it has re-scheduled itself to output read samples.
        // So if there aren't any new values, it must have been called to get a new sample.
        if (P028_data->state != P028_data_struct::BMx_New_values) {
          P028_data->startMeasurement();

          if (P028_ERROR_STATE_OUTPUT != P028_ERROR_IGNORE) {
            if (P028_data->lastMeasurementError) {
              success = true; // "success" may be a confusing name here

              for (uint8_t i = 0; i < 3; i++) {
                UserVar.setFloat(event->TaskIndex, i, ExtraTaskSettings.TaskDeviceErrorValue[i]);
              }
            }
          }
        } else {
          P028_data->state = P028_data_struct::BMx_Values_read;

          if (!P028_data->hasHumidity()) {
            // Patch the sensor type to output only the measured values.
            event->sensorType = Sensor_VType::SENSOR_TYPE_TEMP_EMPTY_BARO;
            event->idx        = getValueCountFromSensorType(Sensor_VType::SENSOR_TYPE_TEMP_EMPTY_BARO);
          }
          UserVar.setFloat(event->TaskIndex, 0, ExtraTaskSettings.checkAllowedRange(0, P028_data->last_temp_val));
          UserVar.setFloat(event->TaskIndex, 1, P028_data->last_hum_val);
          const int elev = P028_ALTITUDE;

          if (elev != 0) {
            UserVar.setFloat(event->TaskIndex, 2, pressureElevation(P028_data->last_press_val, elev));
          } else {
            UserVar.setFloat(event->TaskIndex, 2, P028_data->last_press_val);
          }

          # ifndef LIMIT_BUILD_SIZE

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String hum;

            if (P028_data->hasHumidity()) {
              hum = formatUserVarNoCheck(event, 1);
            }
            addLogMove(LOG_LEVEL_INFO,
                       strformat(F("%s: Addr: %s T: %s H: %s P: %s"),
                                 FsP(P028_data_struct::getDeviceName(P028_data->sensorID)),
                                 formatToHex(P028_I2C_ADDRESS, 2).c_str(),
                                 formatUserVarNoCheck(event, 0).c_str(),
                                 hum.c_str(),
                                 formatUserVarNoCheck(event, 2).c_str()));
          }
          # endif // ifndef LIMIT_BUILD_SIZE
          success = true;
        }
      }
      break;
    }
  }
  return success;
}

#endif // USES_P028
