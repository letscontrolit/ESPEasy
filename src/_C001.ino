#include "src/Helpers/_CPlugin_Helper.h"
#ifdef USES_C001

# include "src/Helpers/_CPlugin_DomoticzHelper.h"

// #######################################################################################################
// ########################### Controller Plugin 001: Domoticz HTTP ######################################
// #######################################################################################################

# define CPLUGIN_001
# define CPLUGIN_ID_001         1
# define CPLUGIN_NAME_001       "Domoticz HTTP"


bool CPlugin_001(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
    {
      Protocol[++protocolCount].Number     = CPLUGIN_ID_001;
      Protocol[protocolCount].usesMQTT     = false;
      Protocol[protocolCount].usesAccount  = true;
      Protocol[protocolCount].usesPassword = true;
      Protocol[protocolCount].usesExtCreds = true;
      Protocol[protocolCount].defaultPort  = 8080;
      Protocol[protocolCount].usesID       = true;
      break;
    }

    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
    {
      string = F(CPLUGIN_NAME_001);
      break;
    }

    case CPlugin::Function::CPLUGIN_INIT:
    {
      success = init_c001_delay_queue(event->ControllerIndex);
      break;
    }

    case CPlugin::Function::CPLUGIN_EXIT:
    {
      exit_c001_delay_queue();
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
    {
      if (C001_DelayHandler == nullptr) {
        break;
      }

      if (event->idx != 0)
      {
        // We now create a URI for the request
        const Sensor_VType sensorType = event->getSensorType();
        String url;
        const size_t expectedSize = sensorType == Sensor_VType::SENSOR_TYPE_STRING ? 64 + event->String2.length() : 128;
        if (url.reserve(expectedSize)) {
          url = F("/json.htm?type=command&param=");

          switch (sensorType)
          {
            case Sensor_VType::SENSOR_TYPE_SWITCH:
            case Sensor_VType::SENSOR_TYPE_DIMMER:
              url += F("switchlight&idx=");
              url += event->idx;
              url += F("&switchcmd=");

              if (UserVar[event->BaseVarIndex] == 0) {
                url += F("Off");
              } else {
                if (sensorType == Sensor_VType::SENSOR_TYPE_SWITCH) {
                  url += F("On");
                } else {
                  url += F("Set%20Level&level=");
                  url += UserVar[event->BaseVarIndex];
                }
              }
              break;

            case Sensor_VType::SENSOR_TYPE_SINGLE:
            case Sensor_VType::SENSOR_TYPE_LONG:
            case Sensor_VType::SENSOR_TYPE_DUAL:
            case Sensor_VType::SENSOR_TYPE_TRIPLE:
            case Sensor_VType::SENSOR_TYPE_QUAD:
            case Sensor_VType::SENSOR_TYPE_TEMP_HUM:
            case Sensor_VType::SENSOR_TYPE_TEMP_BARO:
            case Sensor_VType::SENSOR_TYPE_TEMP_EMPTY_BARO:
            case Sensor_VType::SENSOR_TYPE_TEMP_HUM_BARO:
            case Sensor_VType::SENSOR_TYPE_WIND:
            case Sensor_VType::SENSOR_TYPE_STRING:
            default:
              url += F("udevice&idx=");
              url += event->idx;
              url += F("&nvalue=0");
              url += F("&svalue=");
              url += formatDomoticzSensorType(event);
              break;
          }

          // Add WiFi reception quality
          url += F("&rssi=");
          url += mapRSSItoDomoticz();
            # if FEATURE_ADC_VCC
          url += F("&battery=");
          url += mapVccToDomoticz();
            # endif // if FEATURE_ADC_VCC

          success = C001_DelayHandler->addToQueue(C001_queue_element(event->ControllerIndex, event->TaskIndex, url));
          Scheduler.scheduleNextDelayQueue(ESPEasy_Scheduler::IntervalTimer_e::TIMER_C001_DELAY_QUEUE,
                                          C001_DelayHandler->getNextScheduleTime());
        }
      } // if ixd !=0
      else
      {
        addLog(LOG_LEVEL_ERROR, F("HTTP : IDX cannot be zero!"));
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_FLUSH:
    {
      process_c001_delay_queue();
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
bool do_process_c001_delay_queue(int controller_number, const C001_queue_element& element, ControllerSettingsStruct& ControllerSettings);

bool do_process_c001_delay_queue(int controller_number, const C001_queue_element& element, ControllerSettingsStruct& ControllerSettings) {
// *INDENT-ON*
  WiFiClient client;

  if (!try_connect_host(controller_number, client, ControllerSettings)) {
    return false;
  }

  // This will send the request to the server
  String request = create_http_request_auth(controller_number, element.controller_idx, ControllerSettings, F("GET"), element.txt);

# ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_DEBUG, element.txt);
# endif // ifndef BUILD_NO_DEBUG
  return send_via_http(controller_number, client, request, ControllerSettings.MustCheckReply);
}

#endif // ifdef USES_C001
