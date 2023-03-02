#include "src/Helpers/_CPlugin_Helper.h"
#ifdef USES_C009

#include "src/Helpers/StringProvider.h"
#include "src/CustomBuild/ESPEasy_buildinfo.h"

// #######################################################################################################
// ########################### Controller Plugin 009: FHEM HTTP ##########################################
// #######################################################################################################

/*******************************************************************************
 * Copyright 2016-2017 dev0
 * Contact: https://forum.fhem.de/index.php?action=profile;u=7465
 *          https://github.com/ddtlabs/
 *
 * Release notes:
   - v1.0
   - changed switch and dimmer setreading cmds
   - v1.01
   - added json content to http requests
   - v1.02
   - some optimizations as requested by mvdbro
   - fixed JSON TaskDeviceValueDecimals handling
   - ArduinoJson Library v5.6.4 required (as used by stable R120)
   - parse for HTTP errors 400, 401
   - moved on/off translation for Sensor_VType::SENSOR_TYPE_SWITCH/DIMMER to FHEM module
   - v1.03
   - changed http request from GET to POST (RFC conform)
   - removed obsolete http get url code
   - v1.04
   - added build options and node_type_id to JSON/device
 ******************************************************************************/

# define CPLUGIN_009
# define CPLUGIN_ID_009         9
# define CPLUGIN_NAME_009       "FHEM HTTP"

bool CPlugin_009(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
    {
      Protocol[++protocolCount].Number     = CPLUGIN_ID_009;
      Protocol[protocolCount].usesMQTT     = false;
      Protocol[protocolCount].usesTemplate = false;
      Protocol[protocolCount].usesAccount  = true;
      Protocol[protocolCount].usesPassword = true;
      Protocol[protocolCount].usesExtCreds = true;
      Protocol[protocolCount].usesID       = false;
      Protocol[protocolCount].defaultPort  = 8383;
      break;
    }

    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
    {
      string = F(CPLUGIN_NAME_009);
      break;
    }

    case CPlugin::Function::CPLUGIN_INIT:
    {
      success = init_c009_delay_queue(event->ControllerIndex);
      break;
    }

    case CPlugin::Function::CPLUGIN_EXIT:
    {
      exit_c009_delay_queue();
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
    {
      if (C009_DelayHandler != nullptr) {
        if (C009_DelayHandler->queueFull(event->ControllerIndex)) {
          break;
        }

        std::unique_ptr<C009_queue_element> element(new C009_queue_element(event));
        success = C009_DelayHandler->addToQueue(std::move(element));
        Scheduler.scheduleNextDelayQueue(ESPEasy_Scheduler::IntervalTimer_e::TIMER_C009_DELAY_QUEUE, C009_DelayHandler->getNextScheduleTime());
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_FLUSH:
    {
      process_c009_delay_queue();
      delay(0);
      break;
    }

    default:
      break;
  }
  return success;
}

/*********************************************************************************************\
* FHEM HTTP request
\*********************************************************************************************/

// Uncrustify may change this into multi line, which will result in failed builds
// *INDENT-OFF*
bool do_process_c009_delay_queue(int controller_number, const Queue_element_base& element_base, ControllerSettingsStruct& ControllerSettings) {
  const C009_queue_element& element = static_cast<const C009_queue_element&>(element_base);
// *INDENT-ON*
  String jsonString;
  // Make an educated guess on the actual length, based on earlier requests.
  static size_t expectedJsonLength = 100;
  {
    #ifdef USE_SECOND_HEAP
    HeapSelectIram ephemeral;
    #endif
    // Reserve on the 2nd heap
    if (!jsonString.reserve(expectedJsonLength)) {
      // Not enough free memory
      return false;
    }
  }
  {
    jsonString += '{';
    {
      jsonString += to_json_object_value(F("module"),  F("ESPEasy"));
      jsonString += ',';
      jsonString += to_json_object_value(F("version"), F("1.04"));

      // Create nested object "ESP" inside "data"
      jsonString += ',';
      jsonString += F("\"data\":{");
      {
        jsonString += F("\"ESP\":{");
        {
          // Create nested objects in "ESP":
          jsonString += to_json_object_value(F("name"), Settings.getName());
          jsonString += ',';
          jsonString += to_json_object_value(F("unit"), String(Settings.Unit));
          jsonString += ',';
          jsonString += to_json_object_value(F("version"), String(Settings.Version));
          jsonString += ',';
          jsonString += to_json_object_value(F("build"), String(Settings.Build));
          jsonString += ',';
          jsonString += to_json_object_value(F("build_notes"), F(BUILD_NOTES));
          jsonString += ',';
          jsonString += to_json_object_value(F("build_git"), getValue(LabelType::GIT_BUILD));
          jsonString += ',';
          jsonString += to_json_object_value(F("node_type_id"), String(NODE_TYPE_ID));
          jsonString += ',';
          jsonString += to_json_object_value(F("sleep"), String(Settings.deepSleep_wakeTime));

          // embed IP, important if there is NAT/PAT
          // char ipStr[20];
          // IPAddress ip = NetworkLocalIP();
          // sprintf_P(ipStr, PSTR("%u.%u.%u.%u"), ip[0], ip[1], ip[2], ip[3]);
          jsonString += ',';
          jsonString += to_json_object_value(F("ip"), NetworkLocalIP().toString());
        }
        jsonString += '}'; // End "ESP"

        jsonString += ',';

        // Create nested object "SENSOR" json object inside "data"
        jsonString += F("\"SENSOR\":{");
        {
          // char itemNames[valueCount][2];
          for (uint8_t x = 0; x < element.valueCount; x++)
          {
            // Each sensor value get an own object (0..n)
            // sprintf(itemNames[x],"%d",x);
            if (x != 0) {
              jsonString += ',';
            }

            jsonString += '"';
            jsonString += x;
            jsonString += F("\":{");
            {
              jsonString += to_json_object_value(F("deviceName"), getTaskDeviceName(element._taskIndex));
              jsonString += ',';
              jsonString += to_json_object_value(F("valueName"), getTaskValueName(element._taskIndex, x));
              jsonString += ',';
              jsonString += to_json_object_value(F("type"), String(static_cast<int>(element.sensorType)));
              jsonString += ',';
              jsonString += to_json_object_value(F("value"), element.txt[x]);
            }
            jsonString += '}'; // End "sensor value N"
          }
        }
        jsonString += '}';     // End "SENSOR"
      }
      jsonString += '}';       // End "data"
    }
    jsonString += '}';         // End JSON structure
  }

  if (expectedJsonLength < jsonString.length()) {
    expectedJsonLength = jsonString.length();
  }

  // addLog(LOG_LEVEL_INFO, F("C009 Test JSON:"));
  // addLog(LOG_LEVEL_INFO, jsonString);

  int httpCode = -1;
  send_via_http(
    controller_number,
    ControllerSettings,
    element._controller_idx,
    F("/ESPEasy"),
    F("POST"),
    EMPTY_STRING,
    jsonString,
    httpCode);
  return (httpCode >= 100) && (httpCode < 300);
}

#endif // ifdef USES_C009
