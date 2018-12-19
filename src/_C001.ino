#ifdef USES_C001
//#######################################################################################################
//########################### Controller Plugin 001: Domoticz HTTP ######################################
//#######################################################################################################

#define CPLUGIN_001
#define CPLUGIN_ID_001         1
#define CPLUGIN_NAME_001       "Domoticz HTTP"


bool CPlugin_001(byte function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_001;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesAccount = true;
        Protocol[protocolCount].usesPassword = true;
        Protocol[protocolCount].defaultPort = 8080;
        Protocol[protocolCount].usesID = true;
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_001);
        break;
      }

    case CPLUGIN_INIT:
      {
        MakeControllerSettings(ControllerSettings);
        LoadControllerSettings(event->ControllerIndex, ControllerSettings);
        C001_DelayHandler.configureControllerSettings(ControllerSettings);
        break;
      }

    case CPLUGIN_PROTOCOL_SEND:
      {
        if (event->idx != 0)
        {
          // We now create a URI for the request
          String url;

          switch (event->sensorType)
          {
            case SENSOR_TYPE_SWITCH:
              url = F("/json.htm?type=command&param=switchlight&idx=");
              url += event->idx;
              url += F("&switchcmd=");
              if (UserVar[event->BaseVarIndex] == 0)
                url += F("Off");
              else
                url += F("On");
              break;
            case SENSOR_TYPE_DIMMER:
              url = F("/json.htm?type=command&param=switchlight&idx=");
              url += event->idx;
              url += F("&switchcmd=");
              if (UserVar[event->BaseVarIndex] == 0) {
                url += ("Off");
              } else {
                url += F("Set%20Level&level=");
                url += UserVar[event->BaseVarIndex];
              }
              break;

            case SENSOR_TYPE_SINGLE:
            case SENSOR_TYPE_LONG:
            case SENSOR_TYPE_DUAL:
            case SENSOR_TYPE_TRIPLE:
            case SENSOR_TYPE_QUAD:
            case SENSOR_TYPE_TEMP_HUM:
            case SENSOR_TYPE_TEMP_BARO:
            case SENSOR_TYPE_TEMP_EMPTY_BARO:
            case SENSOR_TYPE_TEMP_HUM_BARO:
            case SENSOR_TYPE_WIND:
            default:
              url = F("/json.htm?type=command&param=udevice&idx=");
              url += event->idx;
              url += F("&nvalue=0");
              url += F("&svalue=");
              url += formatDomoticzSensorType(event);
              break;
          }

          // Add WiFi reception quality
          url += F("&rssi=");
          url += mapRSSItoDomoticz();
          #if FEATURE_ADC_VCC
            url += F("&battery=");
            url += mapVccToDomoticz();
          #endif

          success = C001_DelayHandler.addToQueue(C001_queue_element(event->ControllerIndex, url));
          scheduleNextDelayQueue(TIMER_C001_DELAY_QUEUE, C001_DelayHandler.getNextScheduleTime());
        } // if ixd !=0
        else
        {
          addLog(LOG_LEVEL_ERROR, F("HTTP : IDX cannot be zero!"));
        }
        break;
      }
  }
  return success;
}

bool do_process_c001_delay_queue(int controller_number, const C001_queue_element& element, ControllerSettingsStruct& ControllerSettings) {
  WiFiClient client;
  if (!try_connect_host(controller_number, client, ControllerSettings))
    return false;

  // This will send the request to the server
  String request = create_http_request_auth(controller_number, element.controller_idx, ControllerSettings, F("GET"), element.txt);

  addLog(LOG_LEVEL_DEBUG, element.txt);
  return send_via_http(controller_number, client, request, ControllerSettings.MustCheckReply);
}

#endif
