#include "_Plugin_Helper.h"
#ifdef USES_P031

// #######################################################################################################
// #################### Plugin 031: SHT10/SHT11/SHT15 Temp/Humidity Sensor ###############################
// #######################################################################################################


# include "src/PluginStructs/P031_data_struct.h"

# define PLUGIN_031
# define PLUGIN_ID_031         31
# define PLUGIN_NAME_031       "Environment - SHT1X"
# define PLUGIN_VALUENAME1_031 "Temperature"
# define PLUGIN_VALUENAME2_031 "Humidity"

boolean Plugin_031(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_031;
      Device[deviceCount].Type               = DEVICE_TYPE_DUAL;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TEMP_HUM;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = true;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 2;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].PluginStats        = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_031);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_031));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_031));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_bidirectional(F("Data"));
      event->String2 = formatGpioName_output(F("SCK"));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("Clock Delay"), F("delay"), PCONFIG(0), 0, P031_MAX_CLOCK_DELAY);
      addUnit(F("usec"));
      addFormNote(F("Reduce clock/data frequency to allow for longer cables"));
      success = true;
      break;
    }
    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("delay"));
      success    = true;
      break;
    }


    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P031_data_struct());
      P031_data_struct *P031_data =
        static_cast<P031_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P031_data) {
        return success;
      }
        # ifndef BUILD_NO_DEBUG
      uint8_t status =
        # endif // ifndef BUILD_NO_DEBUG
      P031_data->init(
        CONFIG_PIN1, CONFIG_PIN2,
        Settings.TaskDevicePin1PullUp[event->TaskIndex],
        PCONFIG(0));
        # ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        String log = F("SHT1X : Status uint8_t: ");
        log += String(status, HEX);
        log += F(" - resolution: ");
        log += ((status & 1) ? F("low") : F("high"));
        log += F(" reload from OTP: ");
        log += (((status >> 1) & 1) ? F("yes") : F("no"));
        log += F(", heater: ");
        log += (((status >> 2) & 1) ? F("on") : F("off"));
        addLogMove(LOG_LEVEL_DEBUG, log);
      }
        # endif // ifndef BUILD_NO_DEBUG
      success = true;
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P031_data_struct *P031_data =
        static_cast<P031_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P031_data) {
        if (P031_data->process()) {
          // Measurement ready, schedule new read.
          Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 10);
        }
      }
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      P031_data_struct *P031_data =
        static_cast<P031_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P031_data) {
        if (P031_data->measurementReady()) {
          UserVar[event->BaseVarIndex]     = P031_data->tempC;
          UserVar[event->BaseVarIndex + 1] = P031_data->rhTrue;
          success                          = true;
          P031_data->state                 = P031_IDLE;
        } else if (P031_data->state == P031_IDLE) {
          P031_data->startMeasurement();
        } else if (P031_data->hasError()) {
          // Log error
          if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
            switch (P031_data->state) {
              case P031_COMMAND_NO_ACK:
                addLog(LOG_LEVEL_ERROR, F("SHT1X : Sensor did not ACK command"));
                break;
              case P031_NO_DATA:
                addLog(LOG_LEVEL_ERROR, F("SHT1X : Data not ready"));
                break;
              default:
                break;
            }
          }
          P031_data->state = P031_IDLE;
        }
      }
      break;
    }
  }
  return success;
}

#endif // USES_P031
