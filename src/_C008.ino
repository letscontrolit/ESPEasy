#ifdef USES_C008
//#######################################################################################################
//########################### Controller Plugin 008: Generic HTTP #######################################
//#######################################################################################################

#define CPLUGIN_008
#define CPLUGIN_ID_008         8
#define CPLUGIN_NAME_008       "Generic HTTP"
#include <ArduinoJson.h>

bool CPlugin_008(byte function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_008;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesTemplate = true;
        Protocol[protocolCount].usesAccount = true;
        Protocol[protocolCount].usesPassword = true;
        Protocol[protocolCount].defaultPort = 80;
        Protocol[protocolCount].usesID = true;
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_008);
        break;
      }

    case CPLUGIN_INIT:
      {
        MakeControllerSettings(ControllerSettings);
        LoadControllerSettings(event->ControllerIndex, ControllerSettings);
        C008_DelayHandler.configureControllerSettings(ControllerSettings);
        break;
      }

    case CPLUGIN_PROTOCOL_TEMPLATE:
      {
        event->String1 = "";
        event->String2 = F("demo.php?name=%sysname%&task=%tskname%&valuename=%valname%&value=%value%");
        break;
      }

    case CPLUGIN_PROTOCOL_SEND:
      {
        // Collect the values at the same run, to make sure all are from the same sample
        byte valueCount = getValueCountFromSensorType(event->sensorType);
        C008_queue_element element(event, valueCount);
        if (ExtraTaskSettings.TaskIndex != event->TaskIndex)
          PluginCall(PLUGIN_GET_DEVICEVALUENAMES, event, dummyString);

        MakeControllerSettings(ControllerSettings);
        LoadControllerSettings(event->ControllerIndex, ControllerSettings);

        for (byte x = 0; x < valueCount; x++)
        {
          bool isvalid;
          String formattedValue = formatUserVar(event, x, isvalid);
          if (isvalid) {
            element.txt[x] = "/";
            element.txt[x] += ControllerSettings.Publish;
            parseControllerVariables(element.txt[x], event, true);

            element.txt[x].replace(F("%valname%"), URLEncode(ExtraTaskSettings.TaskDeviceValueNames[x]));
            element.txt[x].replace(F("%value%"), formattedValue);
            addLog(LOG_LEVEL_DEBUG_MORE, element.txt[x]);
          }
        }
        success = C008_DelayHandler.addToQueue(element);
        scheduleNextDelayQueue(TIMER_C008_DELAY_QUEUE, C008_DelayHandler.getNextScheduleTime());
        break;
      }

  }
  return success;
}

//********************************************************************************
// Generic HTTP get request
//********************************************************************************
bool do_process_c008_delay_queue(int controller_number, const C008_queue_element& element, ControllerSettingsStruct& ControllerSettings) {
  while (element.txt[element.valuesSent] == "") {
    // A non valid value, which we are not going to send.
    // Increase sent counter until a valid value is found.
    if (element.checkDone(true))
      return true;
  }

  WiFiClient client;
  if (!try_connect_host(controller_number, client, ControllerSettings))
    return false;

  String request = create_http_request_auth(controller_number, element.controller_idx, ControllerSettings, F("GET"), element.txt[element.valuesSent]);
  return element.checkDone(send_via_http(controller_number, client, request, ControllerSettings.MustCheckReply));
}

#endif
