// #######################################################################################################
// ################ Plugin 105: AHT10            Temperature and Humidity Sensor (I2C) ###################
// #######################################################################################################

/* AHT10/15/20 - Temperature and Humidity
 *
 * AHT1x I2C Address: 0x38, 0x39
 * the driver supports two I2c adresses but only one Sensor allowed.
 *
 * ATTENTION: The AHT10/15 Sensor is incompatible with other I2C devices on I2C bus.
 *
 * The Datasheet write:
 * "Only a single AHT10 can be connected to the I2C bus and no other I2C
 *  devices can be connected".
 *
 * after lot of search and tests, now is confirmed that works only reliable with one sensor
 * on I2C Bus
 */

// #######################################################################################################

#ifdef USES_P105

#include "_Plugin_Helper.h"

#include "src/PluginStructs/P105_data_struct.h"

#define PLUGIN_105
#define PLUGIN_ID_105         105
#define PLUGIN_NAME_105       "Environment - AHT10 [TESTING]"
#define PLUGIN_VALUENAME1_105 "Temperature"
#define PLUGIN_VALUENAME2_105 "Humidity"


// ==============================================
// PLUGIN
// =============================================

boolean Plugin_105(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_105;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TEMP_HUM;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 2;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_105);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_105));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_105));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      int optionValues[2] = { 0x38, 0x39 };
      addFormSelectorI2C(F("i2c_addr"), 2, optionValues, PCONFIG(0));
      addFormNote(F("AO Low=0x38, High=0x39"));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("i2c_addr"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P105_data_struct(PCONFIG(0)));
      P105_data_struct *P105_data = static_cast<P105_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P105_data) {
        return success;
      }

      success = true;
      break;
    }
    case PLUGIN_READ:
    {
      P105_data_struct *P105_data = static_cast<P105_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P105_data) {
        if (P105_data->state != New_values) {
          P105_data->update(event->TaskIndex); // run the state machine
          Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + AHT10_MEASURMENT_DELAY);
          success = false;
          break;
        }
        P105_data->state = Values_read;


        UserVar[event->BaseVarIndex]     = P105_data->last_temp_val;
        UserVar[event->BaseVarIndex + 1] = P105_data->last_hum_val;

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("AHT1x: Temperature: ");
          log += UserVar[event->BaseVarIndex + 0];
          addLog(LOG_LEVEL_INFO, log);
          log  = F("AHT1x: Humidity: ");
          log += UserVar[event->BaseVarIndex + 1];
          addLog(LOG_LEVEL_INFO, log);
        }

        success = true;
        break;
      }
    }
  }
  return success;
}

#endif // USES_P105
