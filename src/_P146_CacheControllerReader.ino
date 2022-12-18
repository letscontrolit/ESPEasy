#include "_Plugin_Helper.h"
#ifdef USES_P146

# include "src/PluginStructs/P146_data_struct.h"

// #######################################################################################################
// #################### Plugin 146 Cache Controller Reader ###############################################
// #######################################################################################################
//
// Allow to read back recorded data from the Cache Controller
// Can be used to trigger to re-send data to controllers when those are again available to receive data
//
// Typical use cases:
// - Dumping data to a MQTT broker
// - Upload binary data to a web server (not yet implemented)
//
// Receive "instructions" from MQTT controller or web command to initiate a dump.
//
// Ideas: (thus not yet implemented)
// - Filter data based on taskindex/type
// - Filter data to only upload data related to sufficient change in value
// - Allow to upload the original timestamp along with the sample


# define PLUGIN_146
# define PLUGIN_ID_146          146
# define PLUGIN_NAME_146       "Generic - Cache Controller Reader"
# define PLUGIN_VALUENAME1_146 "FileNr"
# define PLUGIN_VALUENAME2_146 "FilePos"

# define P146_TASKVALUE_FILENR  UserVar[event->BaseVarIndex + 0]
# define P146_TASKVALUE_FILEPOS UserVar[event->BaseVarIndex + 1]

# define P146_GET_SEND_BINARY    bitRead(PCONFIG(0), 0)
# define P146_SET_SEND_BINARY(X) bitWrite(PCONFIG(0), 0, X)

# define P146_GET_APPEND_BINARY_TOPIC    bitRead(PCONFIG(0), 1)
# define P146_SET_APPEND_BINARY_TOPIC(X) bitWrite(PCONFIG(0), 1, X)

# define P146_GET_SEND_TIMESTAMP    bitRead(PCONFIG(0), 2)
# define P146_SET_SEND_TIMESTAMP(X) bitWrite(PCONFIG(0), 2, X)

# define P146_GET_SEND_READ_POS    bitRead(PCONFIG(0), 3)
# define P146_SET_SEND_READ_POS(X) bitWrite(PCONFIG(0), 3, X)


# define P146_MINIMAL_SEND_INTERVAL             PCONFIG_LONG(0)
# define P146_MQTT_MESSAGE_LENGTH               PCONFIG_LONG(1)
# define P146_MQTT_SEND_TASKVALUENAMES_INTERVAL PCONFIG_LONG(2)


# include "src/Globals/C016_ControllerCache.h"
# include "src/Globals/CPlugins.h"

boolean Plugin_146(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_146;
      Device[deviceCount].Type               = DEVICE_TYPE_DUMMY;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_DUAL;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].DecimalsOnly       = true;
      Device[deviceCount].ValueCount         = 2;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].TimerOptional      = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].OutputDataType     = Output_Data_type_t::Default;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_146);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_146));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_146));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P146_SET_SEND_BINARY(0);
      P146_SET_APPEND_BINARY_TOPIC(0);
      P146_SET_SEND_TIMESTAMP(1);
      P146_SET_SEND_READ_POS(1);

      P146_MINIMAL_SEND_INTERVAL = 100;
      P146_MQTT_MESSAGE_LENGTH   = 800;

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      // Restore the last position from RTC when rebooting.
      ControllerCache.setPeekFilePos(
        P146_TASKVALUE_FILENR,
        P146_TASKVALUE_FILEPOS);
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      if (P146_GET_SEND_BINARY) {
        // FIXME TD-er: Implement flushing binary format to MQTT controller
        // Find
      } else {
        // Do not set the "success" or else the task values of this Cache reader task will be sent to the same controller too.
        // FIXME TD-er: Maybe decimate this, so the broker does have an idea of where we are?

        /*success =*/ P146_data_struct::sendViaOriginalTask(event->TaskIndex, P146_GET_SEND_TIMESTAMP);
        Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + P146_MINIMAL_SEND_INTERVAL);
      }

      //      if (success) {
      int readFileNr    = 0;
      const int readPos = ControllerCache.getPeekFilePos(readFileNr);
      P146_TASKVALUE_FILENR  = readFileNr;
      P146_TASKVALUE_FILEPOS = readPos;

      //      }

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormSubHeader(F("MQTT Output Options"));
      addFormCheckBox(F("HEX encoded Binary"),    F("binary"),         P146_GET_SEND_BINARY);
      addFormCheckBox(F("Append 'bin' to topic"), F("appendbintopic"), P146_GET_APPEND_BINARY_TOPIC);
      addFormCheckBox(F("Send ReadPos"),          F("sendreadpos"),    P146_GET_SEND_READ_POS);
      addFormNumericBox(F("Minimal Send Interval"), F("minsendinterval"), P146_MINIMAL_SEND_INTERVAL, 0, 1000);
      addFormNumericBox(F("Max Message Size"),      F("maxmsgsize"),      P146_MQTT_MESSAGE_LENGTH,   0, 1000);

      addFormSubHeader(F("Non MQTT Output Options"));
      addFormCheckBox(F("Send Timestamp"), F("sendtimestamp"), P146_GET_SEND_TIMESTAMP);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P146_SET_SEND_BINARY(isFormItemChecked(F("binary")));
      P146_SET_APPEND_BINARY_TOPIC(isFormItemChecked(F("appendbintopic")));
      P146_SET_SEND_TIMESTAMP(isFormItemChecked(F("sendtimestamp")));
      P146_SET_SEND_READ_POS(isFormItemChecked(F("sendreadpos")));

      P146_MINIMAL_SEND_INTERVAL = getFormItemInt(F("minsendinterval"));
      P146_MQTT_MESSAGE_LENGTH   = getFormItemInt(F("maxmsgsize"));

      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      const String command    = parseString(string, 1);
      const String subcommand = parseString(string, 2);

      if (command.equals(F("cachereader"))) {
        if (subcommand.equals(F("setreadpos"))) {
          P146_data_struct::setPeekFilePos(event->Par2, event->Par3);
          success = true;
        }
      }
      break;
    }
  }
  return success;
}

#endif // ifdef USES_P146
