#include "_Plugin_Helper.h"
#ifdef USES_P093

// #######################################################################################################
// ################################ Plugin 090: Mitsubishi Heat Pump #####################################
// #######################################################################################################

# include "src/PluginStructs/P093_data_struct.h"

# define PLUGIN_093
# define PLUGIN_ID_093         93
# define PLUGIN_NAME_093       "Energy (Heat) - Mitsubishi Heat Pump"
# define PLUGIN_VALUENAME1_093 "settings"

# define P093_REQUEST_STATUS          PCONFIG(0)
# define P093_REQUEST_STATUS_LABEL    PCONFIG_LABEL(0)


boolean Plugin_093(uint8_t function, struct EventStruct *event, String& string) {
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number       = PLUGIN_ID_093;
      Device[deviceCount].Type           = DEVICE_TYPE_SERIAL;
      Device[deviceCount].VType          = Sensor_VType::SENSOR_TYPE_STRING;
      Device[deviceCount].ValueCount     = 1;
      Device[deviceCount].SendDataOption = true;
      Device[deviceCount].TimerOption    = true;
      Device[deviceCount].TimerOptional  = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_093);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES: {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_093));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES: {
      serialHelper_getGpioNames(event);
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG: {
      string += serialHelper_getSerialTypeLabel(event);
      success = true;
      break;
    }

    case PLUGIN_SET_DEFAULTS: {
      P093_REQUEST_STATUS = 0;
      success             = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      addFormCheckBox(F("Include AC status"), P093_REQUEST_STATUS_LABEL, P093_REQUEST_STATUS);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      P093_REQUEST_STATUS = isFormItemChecked(P093_REQUEST_STATUS_LABEL);
      success             = true;
      break;
    }

    case PLUGIN_INIT: {
      const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P093_data_struct(port, CONFIG_PIN1, CONFIG_PIN2, P093_REQUEST_STATUS));
      P093_data_struct *heatPump = static_cast<P093_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (heatPump != nullptr) {
        heatPump->init();
        success = true;
      }
      break;
    }

    case PLUGIN_READ: {
      P093_data_struct *heatPump = static_cast<P093_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (heatPump != nullptr) {
        success = heatPump->read(event->String2);
      }
      break;
    }

    case PLUGIN_WRITE: {
      if (parseString(string, 1).equalsIgnoreCase(F("MitsubishiHP"))) {
        P093_data_struct *heatPump = static_cast<P093_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (heatPump != nullptr) {
          heatPump->write(parseString(string, 2), parseStringKeepCase(string, 3));
          success = true;
        }
      }
      break;
    }

    case PLUGIN_TEN_PER_SECOND: {
      P093_data_struct *heatPump = static_cast<P093_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((heatPump != nullptr) && heatPump->sync()) {
        Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 10);
      }
      break;
    }
  }

  return success;
}

#endif // USES_P092
