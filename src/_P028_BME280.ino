#include "_Plugin_Helper.h"
#ifdef USES_P028

// #######################################################################################################
// #################### Plugin 028 BME280 I2C Temp/Hum/Barometric Pressure Sensor  #######################
// #######################################################################################################

#include "src/PluginStructs/P028_data_struct.h"

// #include <math.h>

#define PLUGIN_028
#define PLUGIN_ID_028        28
#define PLUGIN_NAME_028       "Environment - BMx280"
#define PLUGIN_VALUENAME1_028 "Temperature"
#define PLUGIN_VALUENAME2_028 "Humidity"
#define PLUGIN_VALUENAME3_028 "Pressure"


boolean Plugin_028(byte function, struct EventStruct *event, String& string)
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
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P028_data_struct(PCONFIG(0)));
      P028_data_struct *P028_data =
        static_cast<P028_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P028_data) {
        return success;
      }
      success = true;

      break;
    }

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      int Plugin_28_i2c_addresses[2] = { 0x76, 0x77 };
      addFormSelectorI2C(F("p028_bme280_i2c"), 2, Plugin_28_i2c_addresses, PCONFIG(0));
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

      addFormNumericBox(F("Altitude"), F("p028_bme280_elev"), PCONFIG(1));
      addUnit(F("m"));

      addFormNumericBox(F("Temperature offset"), F("p028_bme280_tempoffset"), PCONFIG(2));
      addUnit(F("x 0.1C"));
      String offsetNote = F("Offset in units of 0.1 degree Celsius");

      if (nullptr != P028_data) {
        if (P028_data->hasHumidity()) {
          offsetNote += F(" (also correct humidity)");
        }
      }
      addFormNote(offsetNote);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("p028_bme280_i2c"));
      PCONFIG(1) = getFormItemInt(F("p028_bme280_elev"));
      PCONFIG(2) = getFormItemInt(F("p028_bme280_tempoffset"));
      success    = true;
      break;
    }
    case PLUGIN_ONCE_A_SECOND:
    {
      P028_data_struct *P028_data =
        static_cast<P028_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P028_data) {
        const float tempOffset = PCONFIG(2) / 10.0f;

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
        if (P028_data->state != BMx_New_values) {
          success = false;
          break;
        }
        P028_data->state = BMx_Values_read;

        if (!P028_data->hasHumidity()) {
          // Patch the sensor type to output only the measured values.
          event->sensorType = Sensor_VType::SENSOR_TYPE_TEMP_EMPTY_BARO;
        }
        UserVar[event->BaseVarIndex]     = P028_data->last_temp_val;
        UserVar[event->BaseVarIndex + 1] = P028_data->last_hum_val;
        const int elev = PCONFIG(1);

        if (elev != 0) {
          UserVar[event->BaseVarIndex + 2] = P028_data->pressureElevation(elev);
        } else {
          UserVar[event->BaseVarIndex + 2] = P028_data->last_press_val;
        }

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log;
          log.reserve(40); // Prevent re-allocation
          log  = P028_data->getDeviceName();
          log += F(" : Address: 0x");
          log += String(PCONFIG(0), HEX);
          addLog(LOG_LEVEL_INFO, log);
          log  = P028_data->getDeviceName();
          log += F(" : Temperature: ");
          log += UserVar[event->BaseVarIndex];
          addLog(LOG_LEVEL_INFO, log);

          if (P028_data->hasHumidity()) {
            log  = P028_data->getDeviceName();
            log += F(" : Humidity: ");
            log += UserVar[event->BaseVarIndex + 1];
            addLog(LOG_LEVEL_INFO, log);
          }
          log  = P028_data->getDeviceName();
          log += F(" : Barometric Pressure: ");
          log += UserVar[event->BaseVarIndex + 2];
          addLog(LOG_LEVEL_INFO, log);
        }
        success = true;
      }
      break;
    }
  }
  return success;
}

#endif // USES_P028
