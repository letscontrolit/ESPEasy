#ifdef USES_C004
//#######################################################################################################
//########################### Controller Plugin 004: ThingSpeak #########################################
//#######################################################################################################

#define CPLUGIN_004
#define CPLUGIN_ID_004         4
#define CPLUGIN_NAME_004       "ThingSpeak"

boolean CPlugin_004(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_004;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesAccount = true;
        Protocol[protocolCount].usesPassword = true;
        Protocol[protocolCount].defaultPort = 80;
        Protocol[protocolCount].usesID = true;
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_004);
        break;
      }

    case CPLUGIN_INIT:
      {
        ControllerSettingsStruct ControllerSettings;
        LoadControllerSettings(event->ControllerIndex, ControllerSettings);
        C004_DelayHandler.configureControllerSettings(ControllerSettings);
        break;
      }

    case CPLUGIN_GET_PROTOCOL_DISPLAY_NAME:
      {
        success = true;
        switch (event->idx) {
          case CONTROLLER_USER:
            string = F("ThingHTTP Name");
            break;
          case CONTROLLER_PASS:
            string = F("API Key");
            break;
          default:
            success = false;
            break;
        }
      }

    case CPLUGIN_PROTOCOL_SEND:
      {
        success = C004_DelayHandler.addToQueue(C004_queue_element(event));
        scheduleNextDelayQueue(TIMER_C004_DELAY_QUEUE, C004_DelayHandler.getNextScheduleTime());

        break;
      }

  }
  return success;
}

bool do_process_c004_delay_queue(int controller_number, const C004_queue_element& element, ControllerSettingsStruct& ControllerSettings) {
  WiFiClient client;
  if (!try_connect_host(controller_number, client, ControllerSettings))
    return false;

  String postDataStr = F("api_key=");
  postDataStr += SecuritySettings.ControllerPassword[element.controller_idx]; // used for API key

  byte valueCount = getValueCountFromSensorType(element.sensorType);
  for (byte x = 0; x < valueCount; x++)
  {
    postDataStr += F("&field");
    postDataStr += element.idx + x;
    postDataStr += "=";
    postDataStr += formatUserVarNoCheck(element.TaskIndex, x);
  }
  String hostName = F("api.thingspeak.com"); // PM_CZ: HTTP requests must contain host headers.
  if (ControllerSettings.UseDNS)
    hostName = ControllerSettings.HostName;

  String postStr = do_create_http_request(
    hostName, F("POST"),
    F("/update"), // uri
    "",           // auth_header
    F("Content-Type: application/x-www-form-urlencoded\r\n"),
    postDataStr.length());
  postStr += postDataStr;

  return send_via_http(controller_number, client, postStr);
}
#endif
