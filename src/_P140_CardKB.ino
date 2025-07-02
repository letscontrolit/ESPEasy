#include "_Plugin_Helper.h"
#ifdef USES_P140

// #######################################################################################################
// ########################### Plugin 140: Keypad - M5Stack CardKB I2C Keyboard ##########################
// #######################################################################################################

/** Changelog:
 * 2025-02-08 tonhuisman: Change category to Keypad from Input
 *                        Add 'Execute input as command' setting (default enabled)
 *                        Add 'Accept input only' setting
 *                        Show length and buffer content only in Devices overview if Execute command or Accept input setting(s) are enabled
 * 2025-02-06 tonhuisman: Start plugin for M5Stack CardKB I2C mini keyboard
 **/

/** Supported commands:
 * cardkb,exec,<0|1>      : disable/enable Execute input as command
 * cardkb,input,<0|1>     : disable/enable Accept input only
 * cardkb,events,<0|1>    : disable/enable Send event on keypress
 * cardkb,clear           : clear the current buffer content
 **/

# define PLUGIN_140
# define PLUGIN_ID_140          140
# define PLUGIN_NAME_140        "Keypad - CardKB I2C Keyboard"
# define PLUGIN_VALUENAME1_140  "Key"
# define PLUGIN_VALUENAME2_140  "Length"
# define PLUGIN_VALUENAME3_140  "Buffer"

# include "./src/PluginStructs/P140_data_struct.h"

boolean Plugin_140(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number           = PLUGIN_ID_140;
      dev.Type             = DEVICE_TYPE_I2C;
      dev.VType            = Sensor_VType::SENSOR_TYPE_DUAL;
      dev.ValueCount       = 3;
      dev.SendDataOption   = true;
      dev.HasFormatUserVar = true;

      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_140);

      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_140));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_140));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_140));

      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      success = event->Par1 == P140_I2C_ADDR;
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P140_I2C_ADDR;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      P140_EXEC_COMMAND                            = 1;
      ExtraTaskSettings.TaskDeviceValueDecimals[0] = 0;
      ExtraTaskSettings.TaskDeviceValueDecimals[1] = 0;
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = (P140_EXEC_COMMAND || P140_GET_INPUT) ? 3 : 1; // Suppress Length & Buffer if no command is to be executed
      success     = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = (P140_EXEC_COMMAND || P140_GET_INPUT) ? Sensor_VType::SENSOR_TYPE_TRIPLE : Sensor_VType::SENSOR_TYPE_SINGLE;
      success           = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormCheckBox(F("Execute input as command"), F("exec"),   P140_EXEC_COMMAND == 1);

      addFormCheckBox(F("Accept input only"),        F("input"),  P140_GET_INPUT == 1);

      addFormCheckBox(F("Send event on keypress"),   F("events"), P140_SEND_EVENTS == 1);

      P140_data_struct *P140_data = static_cast<P140_data_struct *>(getPluginTaskData(event->TaskIndex));
      String buffer;

      if ((nullptr != P140_data) && P140_data->getBufferValue(buffer)) {
        addFormSubHeader(F("Buffer"));
        addRowLabel(F("Current buffer content"));
        addHtml(buffer);
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P140_SEND_EVENTS  = isFormItemChecked(F("events")) ? 1 : 0;
      P140_EXEC_COMMAND = isFormItemChecked(F("exec")) ? 1 : 0;
      P140_GET_INPUT    = isFormItemChecked(F("input")) ? 1 : 0;
      success           = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P140_data_struct(event));
      P140_data_struct *P140_data = static_cast<P140_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P140_data);

      break;
    }

    case PLUGIN_READ:
    {
      P140_data_struct *P140_data = static_cast<P140_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P140_data) && P140_data->plugin_read(event);

      break;
    }

    case PLUGIN_WRITE:
    {
      P140_data_struct *P140_data = static_cast<P140_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P140_data) && P140_data->plugin_write(event, string);

      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P140_data_struct *P140_data = static_cast<P140_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P140_data) && P140_data->plugin_ten_per_second(event);

      break;
    }

    case PLUGIN_FORMAT_USERVAR:
    {
      if ((P140_EXEC_COMMAND || P140_GET_INPUT) && (2 == event->idx)) { // Only Buffer is user-formatted
        P140_data_struct *P140_data = static_cast<P140_data_struct *>(getPluginTaskData(event->TaskIndex));
        String buffer;

        if ((nullptr != P140_data) && P140_data->getBufferValue(buffer)) {
          string = wrapWithQuotesIfContainsParameterSeparatorChar(buffer);
        } else {
          string = ' '; // Empty buffer indicator
        }
      }
      break;
    }
  }
  return success;
}

#endif // USES_P140
