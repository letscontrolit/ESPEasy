#include "_Plugin_Helper.h"
#ifdef USES_P028

// #######################################################################################################
// #################### Plugin 028 BME280 I2C Temp/Hum/Barometric Pressure Sensor  #######################
// #######################################################################################################

# include "src/PluginStructs/P028_data_struct.h"

// #include <math.h>

# define PLUGIN_028
# define PLUGIN_ID_028        28
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
      Device[++deviceCount].Number           = PLUGIN_ID_028;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TEMP_HUM_BARO;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 3;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
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

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P028_data_struct(P028_I2C_ADDRESS));
      P028_data_struct *P028_data =
        static_cast<P028_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P028_data) {
        return success;
      }
      success = true;

      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x76, 0x77 };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), 2, i2cAddressValues, P028_I2C_ADDRESS);
      } else {
        success = intArrayContains(2, i2cAddressValues, event->Par1);
      }
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      P028_data_struct *P028_data =
        static_cast<P028_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P028_data) {
        if (P028_data->sensorID != Unknown_DEVICE) {
          String detectedString = F("Detected: ");
          detectedString += P028_data->getFullDeviceName();
          addUnit(detectedString);
        }
      }
      addFormNote(F("SDO Low=0x76, High=0x77"));

      addFormNumericBox(F("Altitude"), F("p028_bme280_elev"), P028_ALTITUDE);
      addUnit('m');

      addFormNumericBox(F("Temperature offset"), F("p028_bme280_tempoffset"), P028_TEMPERATURE_OFFSET);
      addUnit(F("x 0.1C"));
      String offsetNote = F("Offset in units of 0.1 degree Celsius");

      if (nullptr != P028_data) {
        if (P028_data->hasHumidity()) {
          offsetNote += F(" (also correct humidity)");
        }
      }
      addFormNote(offsetNote);

      {
        // Value in case of Error
        const __FlashStringHelper *resultsOptions[5] = { F("Ignore"), F("-127"), F("0"), F("125"), F("NaN") };
        int resultsOptionValues[5]                   =
        { P028_ERROR_IGNORE, P028_ERROR_MIN_RANGE, P028_ERROR_ZERO, P028_ERROR_MAX_RANGE, P028_ERROR_NAN };
        addFormSelector(F("Error State Value"), F("p028_err"), 5, resultsOptions, resultsOptionValues, P028_ERROR_STATE_OUTPUT);
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P028_I2C_ADDRESS        = getFormItemInt(F("i2c_addr"));
      P028_ALTITUDE           = getFormItemInt(F("p028_bme280_elev"));
      P028_TEMPERATURE_OFFSET = getFormItemInt(F("p028_bme280_tempoffset"));
      P028_ERROR_STATE_OUTPUT = getFormItemInt(F("p028_err"));
      success                 = true;
      break;
    }
    case PLUGIN_ONCE_A_SECOND:
    {
      P028_data_struct *P028_data =
        static_cast<P028_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P028_data) {
        const float tempOffset = P028_TEMPERATURE_OFFSET / 10.0f;

        if (P028_data->updateMeasurements(tempOffset, event->TaskIndex)) {
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
        if ((P028_data->state != BMx_New_values) &&
            (P028_data->state != BMx_Error)) {
          success = false;
          break;
        }

        if (P028_data->state == BMx_Error) {
          if (P028_ERROR_STATE_OUTPUT != P028_ERROR_IGNORE) {
            float errorValue = NAN;

            success = true; // Act if like we measured something

            switch (P028_ERROR_STATE_OUTPUT) {
              case P028_ERROR_MIN_RANGE: errorValue = -127; break;
              case P028_ERROR_ZERO:      errorValue = 0; break;
              case P028_ERROR_MAX_RANGE: errorValue = 125; break;
              default:
                break;
            }
            UserVar[event->BaseVarIndex]     = errorValue;
            UserVar[event->BaseVarIndex + 1] = errorValue;
            UserVar[event->BaseVarIndex + 2] = errorValue;
          }
        } else {
          P028_data->state = BMx_Values_read;

          if (!P028_data->hasHumidity()) {
            // Patch the sensor type to output only the measured values.
            event->sensorType = Sensor_VType::SENSOR_TYPE_TEMP_EMPTY_BARO;
          }
          UserVar[event->BaseVarIndex]     = P028_data->last_temp_val;
          UserVar[event->BaseVarIndex + 1] = P028_data->last_hum_val;
          const int elev = P028_ALTITUDE;

          if (elev != 0) {
            UserVar[event->BaseVarIndex + 2] = pressureElevation(P028_data->last_press_val, elev);
          } else {
            UserVar[event->BaseVarIndex + 2] = P028_data->last_press_val;
          }
          success = true;
        }

        if (success &&
            loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log;

          if (log.reserve(40)) { // Prevent re-allocation
            log  = P028_data->getDeviceName();
            log += F(" : Address: 0x");
            log += String(P028_I2C_ADDRESS, HEX);
            addLogMove(LOG_LEVEL_INFO, log);

            // addLogMove does also clear the string.
            log  = P028_data->getDeviceName();
            log += F(" : Temperature: ");
            log += formatUserVarNoCheck(event->TaskIndex, 0);
            addLogMove(LOG_LEVEL_INFO, log);

            if (P028_data->hasHumidity()) {
              log  = P028_data->getDeviceName();
              log += F(" : Humidity: ");
              log += formatUserVarNoCheck(event->TaskIndex, 1);
              addLogMove(LOG_LEVEL_INFO, log);
            }
            log  = P028_data->getDeviceName();
            log += F(" : Barometric Pressure: ");
            log += formatUserVarNoCheck(event->TaskIndex, 2);
            addLogMove(LOG_LEVEL_INFO, log);
          }
        }
      }
      break;
    }
  }
  return success;
}

#endif // USES_P028
