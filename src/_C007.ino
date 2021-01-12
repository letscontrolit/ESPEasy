#include "src/Helpers/_CPlugin_Helper.h"
#ifdef USES_C007

# include "src/ESPEasyCore/Serial.h"

// #######################################################################################################
// ########################### Controller Plugin 007: Emoncms ############################################
// #######################################################################################################

# define CPLUGIN_007
# define CPLUGIN_ID_007         7
# define CPLUGIN_NAME_007       "Emoncms"


bool CPlugin_007(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
    {
      Protocol[++protocolCount].Number     = CPLUGIN_ID_007;
      Protocol[protocolCount].usesMQTT     = false;
      Protocol[protocolCount].usesAccount  = false;
      Protocol[protocolCount].usesPassword = true;
      Protocol[protocolCount].defaultPort  = 80;
      Protocol[protocolCount].usesID       = true;
      break;
    }

    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
    {
      string = F(CPLUGIN_NAME_007);
      break;
    }

    case CPlugin::Function::CPLUGIN_INIT:
    {
      success = init_c007_delay_queue(event->ControllerIndex);
      break;
    }

    case CPlugin::Function::CPLUGIN_EXIT:
    {
      exit_c007_delay_queue();
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
    {
      if (C007_DelayHandler == nullptr) {
        break;
      }

      if (event->sensorType == Sensor_VType::SENSOR_TYPE_STRING) {
        addLog(LOG_LEVEL_ERROR, F("emoncms : No support for Sensor_VType::SENSOR_TYPE_STRING"));
        break;
      }
      const byte valueCount = getValueCountForTask(event->TaskIndex);

      if ((valueCount == 0) || (valueCount > VARS_PER_TASK)) {
        addLog(LOG_LEVEL_ERROR, F("emoncms : Unknown sensortype or too many sensor values"));
        break;
      }
      success = C007_DelayHandler->addToQueue(C007_queue_element(event));
      Scheduler.scheduleNextDelayQueue(ESPEasy_Scheduler::IntervalTimer_e::TIMER_C007_DELAY_QUEUE, C007_DelayHandler->getNextScheduleTime());
      break;
    }

    case CPlugin::Function::CPLUGIN_FLUSH:
    {
      process_c007_delay_queue();
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
bool do_process_c007_delay_queue(int controller_number, const C007_queue_element& element, ControllerSettingsStruct& ControllerSettings);

bool do_process_c007_delay_queue(int controller_number, const C007_queue_element& element, ControllerSettingsStruct& ControllerSettings) {
// *INDENT-ON*
  WiFiClient client;

  if (!try_connect_host(controller_number, client, ControllerSettings)) {
    return false;
  }

  String url = F("/emoncms/input/post.json?node=");

  url += Settings.Unit;
  url += F("&json=");

  for (byte i = 0; i < element.valueCount; ++i) {
    url += (i == 0) ? F("{") : F(",");
    url += F("field");
    url += element.idx + i;
    url += ":";
    url += element.txt[i];
  }
  url += "}";
  url += F("&apikey=");
  url += getControllerPass(element.controller_idx, ControllerSettings); // "0UDNN17RW6XAS2E5" // api key

  if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG_MORE) {
    serialPrintln(url);
  }

  return send_via_http(controller_number, client,
                       create_http_get_request(controller_number, ControllerSettings, url),
                       ControllerSettings.MustCheckReply);
}

#endif // ifdef USES_C007
