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
# define PLUGIN_NAME_146       "Generic - Cache Reader"
# define PLUGIN_VALUENAME1_146 "FileNr"
# define PLUGIN_VALUENAME2_146 "FilePos"


# include "src/ControllerQueue/C016_queue_element.h"
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
      Device[deviceCount].DecimalsOnly       = false;
      Device[deviceCount].ValueCount         = 2;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = false;
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
      P146_SET_ERASE_BINFILES(0);
      P146_SET_SEND_BINARY(0);
      P146_SET_SEND_BULK(1);
      P146_SET_SEND_TIMESTAMP(1);
      P146_SET_SEND_READ_POS(1);
      P146_SET_JOIN_TIMESTAMP(1);
      P146_SET_ONLY_SET_TASKS(1);
      P146_SEPARATOR_CHARACTER = ',';

      P146_MINIMAL_SEND_INTERVAL = 100;
      P146_MQTT_MESSAGE_LENGTH   = 800;

      String strings[P146_Nlines];
      strings[P146_TaskInfoTopicIndex] = F("%sysname%_%unit%/%tskname%/upload_meta");
      strings[P146_PublishTopicIndex]  = F("%sysname%_%unit%/%tskname%/upload");

      SaveCustomTaskSettings(event->TaskIndex, strings, P146_Nlines, 0);


      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      // Init the controller cache handler, just in case the cache controller may not be enabled
      ControllerCache.init();

      // Restore the last position from RTC when rebooting.
      ControllerCache.setPeekFilePos(
        P146_TASKVALUE_FILENR,
        P146_TASKVALUE_FILEPOS);
      initPluginTaskData(event->TaskIndex,
                         new (std::nothrow) P146_data_struct(event));
      P146_data_struct *P146_data = static_cast<P146_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P146_data) {
        success = true;
      }
      break;
    }

    case PLUGIN_READ:
    {
      if (ControllerCache.peekDataAvailable()) {
        Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + P146_MINIMAL_SEND_INTERVAL);

        if (P146_GET_SEND_BULK) {
          if (P146_GET_SEND_BINARY) {
            P146_data_struct::prepare_BulkMQTT_message(event->TaskIndex);
          } else {
            P146_data_struct *P146_data = static_cast<P146_data_struct *>(getPluginTaskData(event->TaskIndex));

            if (nullptr != P146_data) {
              const char separator = static_cast<char>(P146_SEPARATOR_CHARACTER);
              P146_data->prepareCSVInBulk(event->TaskIndex, P146_GET_JOIN_TIMESTAMP, P146_GET_ONLY_SET_TASKS, separator);
            }
          }
        } else {
          // Do not set the "success" or else the task values of this Cache reader task will be sent to the same controller too.

          if (P146_data_struct::sendViaOriginalTask(event->TaskIndex, P146_GET_SEND_TIMESTAMP)) {
            int readFileNr    = 0;
            const int readPos = ControllerCache.getPeekFilePos(readFileNr);

            if (P146_GET_ERASE_BINFILES) {
              // Check whether we must delete the oldest file
              if (P146_TASKVALUE_FILENR != 0 && P146_TASKVALUE_FILENR  < readFileNr) {
                ControllerCache.deleteCacheBlock(P146_TASKVALUE_FILENR);
              }
            }

            P146_TASKVALUE_FILENR  = readFileNr;
            P146_TASKVALUE_FILEPOS = readPos;
          }
        }
      } else {
        // Default to 1 sec
        Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 1000);
      }

      break;
    }

    case PLUGIN_PROCESS_CONTROLLER_DATA:
    {
      if (P146_GET_SEND_BULK) {
        P146_data_struct *P146_data = static_cast<P146_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != P146_data) {
          bool data_sent = false;

          if (P146_GET_SEND_BINARY) {
            data_sent = (0 != P146_data->sendBinaryInBulk(event->TaskIndex, P146_MQTT_MESSAGE_LENGTH));
          } else {
            data_sent = (0 != P146_data->sendCSVInBulk(event->TaskIndex, P146_MQTT_MESSAGE_LENGTH));
          }

          if (data_sent) {
            int readFileNr    = 0;
            const int readPos = ControllerCache.getPeekFilePos(readFileNr);

            if (P146_GET_ERASE_BINFILES) {
              // Check whether we must delete the oldest file
              if (P146_TASKVALUE_FILENR != 0 && P146_TASKVALUE_FILENR  < readFileNr) {
                ControllerCache.deleteCacheBlock(P146_TASKVALUE_FILENR);
              }
            }

            P146_TASKVALUE_FILENR  = readFileNr;
            P146_TASKVALUE_FILEPOS = readPos;
          }
          success = true;
        }
      }

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormCheckBox(F("Delete Cache Files After Send"), F("deletebin"), P146_GET_ERASE_BINFILES);
      addFormSubHeader(F("MQTT Output Options"));
      addFormCheckBox(F("Send Bulk"),          F("sendbulk"), P146_GET_SEND_BULK);
      addFormCheckBox(F("HEX encoded Binary"), F("binary"),   P146_GET_SEND_BINARY);

      //      addFormCheckBox(F("Send ReadPos"),          F("sendreadpos"),    P146_GET_SEND_READ_POS);
      addFormNumericBox(F("Minimal Send Interval"), F("minsendinterval"), P146_MINIMAL_SEND_INTERVAL, 0, 1000);
      addFormNumericBox(F("Max Message Size"),
                        F("maxmsgsize"),
                        P146_MQTT_MESSAGE_LENGTH,
                        sizeof(C016_binary_element) + 16,
                        32768);

      String strings[P146_Nlines];
      LoadCustomTaskSettings(event->TaskIndex, strings, P146_Nlines, 0);
      addFormTextBox(F("TaskInfo Topic"), getPluginCustomArgName(P146_TaskInfoTopicIndex), strings[P146_TaskInfoTopicIndex], P146_Nchars);
      addFormTextBox(F("Publish Topic"),  getPluginCustomArgName(P146_PublishTopicIndex),  strings[P146_PublishTopicIndex],  P146_Nchars);


      //      addFormSubHeader(F("Non MQTT Output Options"));
      //      addFormCheckBox(F("Send Timestamp"), F("sendtimestamp"), P146_GET_SEND_TIMESTAMP);

      addTableSeparator(F("Export to CSV"), 2, 3);

      {
        const __FlashStringHelper *separatorLabels[] = {
          F("Tab"),
          F("Comma"),
          F("Semicolon")
        };
        const int separatorOptions[] = {
          '\t',
          ',',
          ';'
        };
        addFormSelector(F("Separator"), F("separator"), 3, separatorLabels, separatorOptions, P146_SEPARATOR_CHARACTER);
      }
      addFormCheckBox(F("Join Samples with same Timestamp"), F("jointimestamp"), P146_GET_JOIN_TIMESTAMP);
      addFormCheckBox(F("Export only set tasks"),            F("onlysettasks"),  P146_GET_ONLY_SET_TASKS);

      addFormNote(F("Download button link only updated after saving"));

      addRowLabel(EMPTY_STRING);
      html_add_button_prefix();
      addHtml(F("dumpcache?separator="));

      switch (static_cast<char>(P146_SEPARATOR_CHARACTER)) {
        case '\t': addHtml(F("Tab")); break;
        case ',':  addHtml(F("Comma")); break;
        case ';':
        default:   addHtml(F("Semicolon")); break;
      }

      if (P146_GET_JOIN_TIMESTAMP) {
        addHtml(F("&jointimestamp=1"));
      }

      if (P146_GET_ONLY_SET_TASKS) {
        addHtml(F("&onlysettasks=1"));
      }
      addHtml(F("'>Download as CSV</a>"));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P146_SET_ERASE_BINFILES(isFormItemChecked(F("deletebin")));
      P146_SET_SEND_BULK(isFormItemChecked(F("sendbulk")));
      P146_SET_SEND_BINARY(isFormItemChecked(F("binary")));

      //      P146_SET_SEND_READ_POS(isFormItemChecked(F("sendreadpos")));
      //      P146_SET_SEND_TIMESTAMP(isFormItemChecked(F("sendtimestamp")));

      P146_MINIMAL_SEND_INTERVAL = getFormItemInt(F("minsendinterval"));
      P146_MQTT_MESSAGE_LENGTH   = getFormItemInt(F("maxmsgsize"));

      P146_SET_JOIN_TIMESTAMP(isFormItemChecked(F("jointimestamp")));
      P146_SET_ONLY_SET_TASKS(isFormItemChecked(F("onlysettasks")));
      P146_SEPARATOR_CHARACTER = getFormItemInt(F("separator"));

      String strings[P146_Nlines];
      String error;

      for (uint8_t varNr = 0; varNr < P146_Nlines; varNr++) {
        strings[varNr] = webArg(getPluginCustomArgName(varNr));
      }

      error = SaveCustomTaskSettings(event->TaskIndex, strings, P146_Nlines, 0);

      if (!error.isEmpty()) {
        addHtmlError(error);
      }


      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      const String command    = parseString(string, 1);
      const String subcommand = parseString(string, 2);

      if (equals(command, F("cachereader"))) {
        if (equals(subcommand, F("setreadpos"))) {
          P146_data_struct::setPeekFilePos(event->Par2, event->Par3);
          success = true;
        } else if (equals(subcommand, F("sendtaskinfo"))) {
          P146_data_struct *P146_data = static_cast<P146_data_struct *>(getPluginTaskData(event->TaskIndex));

          if (nullptr != P146_data) {
            P146_data->sendTaskInfoInBulk(event);
            success = true;
          }
        } else if (equals(subcommand, F("flush"))) {
          P146_data_struct::flush();
          success = true;
        }
      }
      break;
    }
  }
  return success;
}

#endif // ifdef USES_P146
