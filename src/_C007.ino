#ifdef USES_C007
//#######################################################################################################
//########################### Controller Plugin 007: Emoncms ############################################
//#######################################################################################################

#define CPLUGIN_007
#define CPLUGIN_ID_007         7
#define CPLUGIN_NAME_007       "Emoncms"

boolean CPlugin_007(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_007;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesAccount = false;
        Protocol[protocolCount].usesPassword = true;
        Protocol[protocolCount].defaultPort = 80;
        Protocol[protocolCount].usesID = true;
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_007);
        break;
      }

    case CPLUGIN_INIT:
      {
        ControllerSettingsStruct ControllerSettings;
        LoadControllerSettings(event->ControllerIndex, ControllerSettings);
        C007_DelayHandler.configureControllerSettings(ControllerSettings);
        break;
      }

    case CPLUGIN_PROTOCOL_SEND:
      {
        const byte valueCount = getValueCountFromSensorType(event->sensorType);
        if (valueCount == 0 || valueCount > 3) {
          addLog(LOG_LEVEL_ERROR, F("emoncms : Unknown sensortype or too many sensor values"));
          break;
        }
        success = C007_DelayHandler.addToQueue(C007_queue_element(event));
        scheduleNextDelayQueue(TIMER_C007_DELAY_QUEUE, C007_DelayHandler.getNextScheduleTime());
        break;
      }

  }
  return success;
}

bool do_process_c007_delay_queue(int controller_number, const C007_queue_element& element, ControllerSettingsStruct& ControllerSettings) {
  WiFiClient client;
  if (!try_connect_host(controller_number, client, ControllerSettings))
    return false;

  String url = F("/emoncms/input/post.json?node=");
  url += Settings.Unit;
  url += F("&json=");
  const byte valueCount = getValueCountFromSensorType(element.sensorType);
  for (byte i = 0; i < valueCount; ++i) {
    url += (i == 0) ? F("{") : F(",");
    url += F("field");
    url += element.idx + i;
    url += ":";
    url += formatUserVarNoCheck(element.TaskIndex, i);
  }
  url += "}";
  url += F("&apikey=");
  url += SecuritySettings.ControllerPassword[element.controller_idx]; // "0UDNN17RW6XAS2E5" // api key

  if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG_MORE)
    Serial.println(url);

  return send_via_http(controller_number, client,
    create_http_get_request(controller_number, ControllerSettings, url));
}
#endif
