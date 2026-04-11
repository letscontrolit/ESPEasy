#include "_Plugin_Helper.h"
#ifdef USES_P031

// #######################################################################################################
// #################### Plugin 031: SHT10/SHT11/SHT15 Temp/Humidity Sensor ###############################
// #######################################################################################################


# include "src/PluginStructs/P031_data_struct.h"

# define PLUGIN_031
# define PLUGIN_ID_031         31
# define PLUGIN_NAME_031       "Environment - SHT1x"
# define PLUGIN_VALUENAME1_031 "Temperature"
# define PLUGIN_VALUENAME2_031 "Humidity"

boolean Plugin_031(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number        = PLUGIN_ID_031;
      dev.Type          = DEVICE_TYPE_DUAL;
      dev.VType         = Sensor_VType::SENSOR_TYPE_TEMP_HUM;
      dev.PullUpOption  = true;
      dev.FormulaOption = true;
      dev.ValueCount    = 2;
      dev.TimerOption   = true;
      dev.PluginStats   = true;
      dev.setPin2Direction(gpio_direction::gpio_output);
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
        addLog(LOG_LEVEL_DEBUG, strformat(F("SHT1x : Status uint8_t: %x - resolution: %s reload from OTP: %s, heater: %s"),
                                          status,
                                          FsP((status & 1) ? F("low") : F("high")),
                                          FsP(((status >> 1) & 1) ? F("yes") : F("no")),
                                          FsP(((status >> 2) & 1) ? F("on") : F("off"))));
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
          UserVar.setFloat(event->TaskIndex, 0, P031_data->tempC);
          UserVar.setFloat(event->TaskIndex, 1, P031_data->rhTrue);
          success          = true;
          P031_data->state = P031_IDLE;
        } else if (P031_data->state == P031_IDLE) {
          P031_data->startMeasurement();
        } else if (P031_data->hasError()) {
          // Log error
          if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
            switch (P031_data->state) {
              case P031_COMMAND_NO_ACK:
                addLog(LOG_LEVEL_ERROR, F("SHT1x : Sensor did not ACK command"));
                break;
              case P031_NO_DATA:
                addLog(LOG_LEVEL_ERROR, F("SHT1x : Data not ready"));
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
