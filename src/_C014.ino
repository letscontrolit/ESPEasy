#ifdef USES_C014
//#######################################################################################################
//########################### Controller Plugin 014: Cached HTTP ########################################
//#######################################################################################################

#define CPLUGIN_014
#define CPLUGIN_ID_014         14
#define CPLUGIN_NAME_014       "Cached HTTP"
#include <ArduinoJson.h>

ControllerCache_struct ControllerCache;


bool CPlugin_014(byte function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_014;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesTemplate = true;
        Protocol[protocolCount].usesAccount = true;
        Protocol[protocolCount].usesPassword = true;
        Protocol[protocolCount].defaultPort = 80;
        Protocol[protocolCount].usesID = true;
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_014);
        break;
      }

    case CPLUGIN_INIT:
      {
        MakeControllerSettings(ControllerSettings);
        LoadControllerSettings(event->ControllerIndex, ControllerSettings);
        C014_DelayHandler.configureControllerSettings(ControllerSettings);
        ControllerCache.init();
        break;
      }

    case CPLUGIN_PROTOCOL_TEMPLATE:
      {
        event->String1 = "";
        event->String2 = "";
        break;
      }

    case CPLUGIN_PROTOCOL_SEND:
      {
        // Collect the values at the same run, to make sure all are from the same sample
        byte valueCount = getValueCountFromSensorType(event->sensorType);
        C014_queue_element element(event, valueCount, getUnixTime());

        MakeControllerSettings(ControllerSettings);
        LoadControllerSettings(event->ControllerIndex, ControllerSettings);
        success = C014_DelayHandler.addToQueue(element);
        scheduleNextDelayQueue(TIMER_C014_DELAY_QUEUE, C014_DelayHandler.getNextScheduleTime());
        break;
      }

    case CPLUGIN_FLUSH:
      {
        process_c014_delay_queue();
        delay(0);
        break;
      }

  }
  return success;
}



//********************************************************************************
// Generic HTTP get request
//********************************************************************************
bool do_process_c014_delay_queue(int controller_number, const C014_queue_element& element, ControllerSettingsStruct& ControllerSettings) {
  ControllerCache.write((uint8_t*)&element, sizeof(element));
  return true;
/*
  WiFiClient client;
  if (!try_connect_host(controller_number, client, ControllerSettings)) {
    // Try to dump to file.
    // Return true if element was successful sent to file cache.
  } else {
    // 1) Process element
    // 2) Read next cached element from file and place it in the queue.

  }

//  String request = create_http_request_auth(controller_number, element.controller_idx, ControllerSettings, F("GET"), element.txt[element.valuesSent]);
//  return element.checkDone(send_via_http(controller_number, client, request, ControllerSettings.MustCheckReply));
  return true;
*/
}

bool c014_startCSVdump() {
  ControllerCache.resetpeek();
  return ControllerCache.isInitialized();
}

String c014_getCacheFileName(bool& islast) {
  return ControllerCache.getPeakCacheFileName(islast);
}


bool c014_getCSVline(
  unsigned long& timestamp,
  byte& controller_idx,
  byte& TaskIndex,
  byte& sensorType,
  byte& valueCount,
  float& val1,
  float& val2,
  float& val3,
  float& val4)
{
  C014_queue_element element;
  bool result = ControllerCache.peek((uint8_t*)&element, sizeof(element));
  timestamp = element.timestamp;
  controller_idx = element.controller_idx;
  TaskIndex = element.TaskIndex;
  sensorType = element.sensorType;
  valueCount = element.valueCount;
  val1 = element.values[0];
  val2 = element.values[1];
  val3 = element.values[2];
  val4 = element.values[3];
  return result;
}

#endif
