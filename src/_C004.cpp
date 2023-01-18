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
      if (C004_DelayHandler->queueFull(event->ControllerIndex)) {
        break;
      }

      std::unique_ptr<C004_queue_element> element(new C004_queue_element(event));

      success = C004_DelayHandler->addToQueue(std::move(element));
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
bool do_process_c004_delay_queue(int controller_number, const Queue_element_base& element_base, ControllerSettingsStruct& ControllerSettings) {
  const C004_queue_element& element = static_cast<const C004_queue_element&>(element_base);
// *INDENT-ON*
  String postDataStr = F("api_key=");

  postDataStr += getControllerPass(element._controller_idx, ControllerSettings); // used for API key

  if (element.sensorType == Sensor_VType::SENSOR_TYPE_STRING) {
    postDataStr += F("&status=");
    postDataStr += element.txt[0]; // FIXME TD-er: Is this correct?
    // See: https://nl.mathworks.com/help/thingspeak/writedata.html
  } else {
    for (uint8_t x = 0; x < element.valueCount; x++)
    {
      postDataStr += F("&field");
      postDataStr += element.idx + x;
      postDataStr += '=';
      postDataStr += element.txt[x];
    }
  }
  if (!ControllerSettings.UseDNS) {
    // Patch the ControllerSettings to make sure we're using a hostname instead of an IP address
    ControllerSettings.setHostname(F("api.thingspeak.com")); // PM_CZ: HTTP requests must contain host headers.
    ControllerSettings.UseDNS = true;
  }

  int httpCode = -1;
  send_via_http(
    controller_number,
    ControllerSettings,
    element._controller_idx,
    F("/update"), // uri
    F("POST"),
    F("Content-Type: application/x-www-form-urlencoded\r\n"),
    postDataStr,
    httpCode);
  return (httpCode >= 100) && (httpCode < 300);
}

#endif // ifdef USES_C004
