#include "src/Helpers/_CPlugin_Helper.h"
#ifdef USES_C009
//#######################################################################################################
//########################### Controller Plugin 009: FHEM HTTP ##########################################
//#######################################################################################################

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

#define CPLUGIN_009
#define CPLUGIN_ID_009         9
#define CPLUGIN_NAME_009       "FHEM HTTP"
#include <ArduinoJson.h>

bool CPlugin_009(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_009;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesTemplate = false;
        Protocol[protocolCount].usesAccount = true;
        Protocol[protocolCount].usesPassword = true;
        Protocol[protocolCount].usesExtCreds = true;
        Protocol[protocolCount].usesID = false;
        Protocol[protocolCount].defaultPort = 8383;
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
        if (C009_DelayHandler == nullptr) {
          break;
        }

        byte valueCount = getValueCountForTask(event->TaskIndex);
        C009_queue_element element(event);

        for (byte x = 0; x < valueCount; x++)
        {
          element.txt[x] = formatUserVarNoCheck(event, x);
        }
        // FIXME TD-er must define a proper move operator
        success = C009_DelayHandler->addToQueue(C009_queue_element(element));
        Scheduler.scheduleNextDelayQueue(ESPEasy_Scheduler::IntervalTimer_e::TIMER_C009_DELAY_QUEUE, C009_DelayHandler->getNextScheduleTime());
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
bool do_process_c009_delay_queue(int controller_number, const C009_queue_element& element, ControllerSettingsStruct& ControllerSettings);
// *INDENT-ON*

bool do_process_c009_delay_queue(int controller_number, const C009_queue_element& element, ControllerSettingsStruct& ControllerSettings) {
  WiFiClient client;
  if (!try_connect_host(controller_number, client, ControllerSettings))
    return false;

  LoadTaskSettings(element.TaskIndex);
  String jsonString;
  {
    // Create json root object
    DynamicJsonDocument root(1024);
    root[F("module")] = String(F("ESPEasy"));
    root[F("version")] = String(F("1.04"));

    // Create nested objects
    JsonObject data = root.createNestedObject(String(F("data")));
    JsonObject ESP = data.createNestedObject(String(F("ESP")));
    ESP[F("name")] = Settings.Name;
    ESP[F("unit")] = Settings.Unit;
    ESP[F("version")] = Settings.Version;
    ESP[F("build")] = Settings.Build;
    ESP[F("build_notes")] = String(F(BUILD_NOTES));
    ESP[F("build_git")] = String(F(BUILD_GIT));
    ESP[F("node_type_id")] = NODE_TYPE_ID;
    ESP[F("sleep")] = Settings.deepSleep_wakeTime;

    // embed IP, important if there is NAT/PAT
    // char ipStr[20];
    // IPAddress ip = NetworkLocalIP();
    // sprintf_P(ipStr, PSTR("%u.%u.%u.%u"), ip[0], ip[1], ip[2], ip[3]);
    ESP[F("ip")] = NetworkLocalIP().toString();

    // Create nested SENSOR json object
    JsonObject SENSOR = data.createNestedObject(String(F("SENSOR")));
    byte valueCount = getValueCountForTask(element.TaskIndex);
    // char itemNames[valueCount][2];
    for (byte x = 0; x < valueCount; x++)
    {
      // Each sensor value get an own object (0..n)
      // sprintf(itemNames[x],"%d",x);
      JsonObject val = SENSOR.createNestedObject(String(x));
      val[F("deviceName")] = getTaskDeviceName(element.TaskIndex);
      val[F("valueName")]  = ExtraTaskSettings.TaskDeviceValueNames[x];
      val[F("type")]       = static_cast<int>(element.sensorType);
      val[F("value")]      = element.txt[x];
    }

    // Create json buffer
    serializeJson(root, jsonString);
  }

  // We now create a URI for the request
  String request = create_http_request_auth(
      controller_number, element.controller_idx, ControllerSettings,
      F("POST"), F("/ESPEasy"), jsonString.length());
  request += jsonString;

  return send_via_http(controller_number, client, request, ControllerSettings.MustCheckReply);
}
#endif
