#include "src/Helpers/_CPlugin_Helper.h"
#ifdef USES_C004

// #######################################################################################################
// ########################### Controller Plugin 004: ThingSpeak #########################################
// #######################################################################################################

# define CPLUGIN_004
# define CPLUGIN_ID_004         4
# define CPLUGIN_NAME_004       "ThingSpeak"

bool CPlugin_004(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
    {
      Protocol[++protocolCount].Number     = CPLUGIN_ID_004;
      Protocol[protocolCount].usesMQTT     = false;
      Protocol[protocolCount].usesAccount  = true;
      Protocol[protocolCount].usesPassword = true;
      Protocol[protocolCount].defaultPort  = 80;
      Protocol[protocolCount].usesID       = true;
      break;
    }

    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
    {
      string = F(CPLUGIN_NAME_004);
      break;
    }

    case CPlugin::Function::CPLUGIN_INIT:
    {
      success = init_c004_delay_queue(event->ControllerIndex);
      break;
    }

    case CPlugin::Function::CPLUGIN_EXIT:
    {
      exit_c004_delay_queue();
      break;
    }

    case CPlugin::Function::CPLUGIN_GET_PROTOCOL_DISPLAY_NAME:
    {
      success = true;

      switch (event->idx) {
        case ControllerSettingsStruct::CONTROLLER_USER:
          string = F("ThingHTTP Name");
          break;
        case ControllerSettingsStruct::CONTROLLER_PASS:
          string = F("API Key");
          break;
        default:
          success = false;
          break;
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
    {
      if (C004_DelayHandler == nullptr) {
        break;
      }
      success = C004_DelayHandler->addToQueue(C004_queue_element(event));
      Scheduler.scheduleNextDelayQueue(ESPEasy_Scheduler::IntervalTimer_e::TIMER_C004_DELAY_QUEUE, C004_DelayHandler->getNextScheduleTime());

      break;
    }

    case CPlugin::Function::CPLUGIN_FLUSH:
    {
      process_c004_delay_queue();
      delay(0);
      break;
    }

    default:
      break;
  }
  return success;
}

// Uncrustify may change this into multi line, which will result in failed builds
// *INDENT-OFF*
bool do_process_c004_delay_queue(int controller_number, const C004_queue_element& element, ControllerSettingsStruct& ControllerSettings);

bool do_process_c004_delay_queue(int controller_number, const C004_queue_element& element, ControllerSettingsStruct& ControllerSettings) {
// *INDENT-ON*
  WiFiClient client;

  if (!try_connect_host(controller_number, client, ControllerSettings)) {
    return false;
  }

  String postDataStr = F("api_key=");

  postDataStr += getControllerPass(element.controller_idx, ControllerSettings); // used for API key

  if (element.sensorType == Sensor_VType::SENSOR_TYPE_STRING) {
    postDataStr += F("&status=");
    postDataStr += element.txt[0]; // FIXME TD-er: Is this correct?
    // See: https://nl.mathworks.com/help/thingspeak/writedata.html
  } else {
    for (byte x = 0; x < element.valueCount; x++)
    {
      postDataStr += F("&field");
      postDataStr += element.idx + x;
      postDataStr += '=';
      postDataStr += element.txt[x];
    }
  }
  String hostName = F("api.thingspeak.com"); // PM_CZ: HTTP requests must contain host headers.

  if (ControllerSettings.UseDNS) {
    hostName = ControllerSettings.HostName;
  }

  String postStr = do_create_http_request(
    hostName, F("POST"),
    F("/update"), // uri
    EMPTY_STRING,           // auth_header
    F("Content-Type: application/x-www-form-urlencoded\r\n"),
    postDataStr.length());

  postStr += postDataStr;

  return send_via_http(controller_number, client, postStr, ControllerSettings.MustCheckReply);
}

#endif // ifdef USES_C004
