#include "src/Helpers/_CPlugin_Helper.h"
#ifdef USES_C007

# include "src/ESPEasyCore/Serial.h"

// #######################################################################################################
// ########################### Controller Plugin 007: Emoncms ############################################
// #######################################################################################################

# define CPLUGIN_007
# define CPLUGIN_ID_007         7
# define CPLUGIN_NAME_007       "Emoncms"

# define C007_DEFAULT_URL "/emoncms/input/post.json"

bool CPlugin_007(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
    {
      ProtocolStruct& proto = getProtocolStruct(event->idx); //      = CPLUGIN_ID_007;
      proto.usesMQTT     = false;
      proto.usesAccount  = false;
      proto.usesPassword = true;
      proto.defaultPort  = 80;
      proto.usesID       = true;
      proto.usesTemplate = true;
      break;
    }

    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
    {
      string = F(CPLUGIN_NAME_007);
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_TEMPLATE:
    {
      event->String2 = F(C007_DEFAULT_URL);
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

      if (C007_DelayHandler->queueFull(event->ControllerIndex)) {
        break;
      }


      if (event->getSensorType() == Sensor_VType::SENSOR_TYPE_STRING) {
        addLog(LOG_LEVEL_ERROR, F("emoncms : No support for Sensor_VType::SENSOR_TYPE_STRING"));
        break;
      }
      const uint8_t valueCount = getValueCountForTask(event->TaskIndex);

      if ((valueCount == 0) || (valueCount > VARS_PER_TASK)) {
        addLog(LOG_LEVEL_ERROR, F("emoncms : Unknown sensortype or too many sensor values"));
        break;
      }

      std::unique_ptr<C007_queue_element> element(new (std::nothrow) C007_queue_element(event));
      success = C007_DelayHandler->addToQueue(std::move(element));

      Scheduler.scheduleNextDelayQueue(SchedulerIntervalTimer_e::TIMER_C007_DELAY_QUEUE, C007_DelayHandler->getNextScheduleTime());
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
bool do_process_c007_delay_queue(cpluginID_t cpluginID, const Queue_element_base& element_base, ControllerSettingsStruct& ControllerSettings) {
  const C007_queue_element& element = static_cast<const C007_queue_element&>(element_base);
// *INDENT-ON*
  if (ControllerSettings.Publish[0] == '\0') {
    strcpy_P(ControllerSettings.Publish, PSTR(C007_DEFAULT_URL));
  }
  String url = strformat(F("%s?node=%d&json="), ControllerSettings.Publish, Settings.Unit);

  for (uint8_t i = 0; i < element.valueCount; ++i) {
    url += strformat(F("%cfield%d:%s"), (i == 0) ? '{' : ',', element.idx + i, element.txt[i].c_str());
  }
  url += strformat(F("}&apikey=%s"), getControllerPass(element._controller_idx, ControllerSettings).c_str()); // "0UDNN17RW6XAS2E5" // api key

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) {
    addLog(LOG_LEVEL_DEBUG_MORE, url);
  }
  # endif // ifndef BUILD_NO_DEBUG

  int httpCode = -1;
  send_via_http(
    cpluginID,
    ControllerSettings,
    element._controller_idx,
    url,
    F("GET"),
    EMPTY_STRING,
    EMPTY_STRING,
    httpCode);
  return (httpCode >= 100) && (httpCode < 300);
}

#endif // ifdef USES_C007
