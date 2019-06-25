#ifdef USES_C015
//#######################################################################################################
//###########################   Controller Plugin 015: ZABBIX  ##########################################
//#######################################################################################################
// Based on https://www.zabbix.com/documentation/current/manual/appendix/items/trapper
// and https://www.zabbix.com/documentation/4.2/manual/appendix/protocols/header_datalen



#define CPLUGIN_015
#define CPLUGIN_ID_015 15
#define CPLUGIN_NAME_015 "Zabbix"
#include <ArduinoJson.h>

bool CPlugin_015(byte function, struct EventStruct *event, String &string)
{
  bool success = false;

  switch (function)
  {
  case CPLUGIN_PROTOCOL_ADD:
  {
    Protocol[++protocolCount].Number = CPLUGIN_ID_015;
    Protocol[protocolCount].usesMQTT = false;
    Protocol[protocolCount].usesTemplate = false;
    Protocol[protocolCount].usesAccount = false;
    Protocol[protocolCount].usesPassword = false;
    Protocol[protocolCount].usesID = false;
    Protocol[protocolCount].defaultPort = 10051;
    break;
  }

  case CPLUGIN_GET_DEVICENAME:
  {
    string = F(CPLUGIN_NAME_015);
    break;
  }

  case CPLUGIN_PROTOCOL_SEND:
  {
    byte valueCount = getValueCountFromSensorType(event->sensorType);
    C015_queue_element element(event);

    MakeControllerSettings(ControllerSettings);
    LoadControllerSettings(event->ControllerIndex, ControllerSettings);

    for (byte x = 0; x < valueCount; x++)
    {
      element.txt[x] = formatUserVarNoCheck(event, x);
    }
    success = C015_DelayHandler.addToQueue(element);
    scheduleNextDelayQueue(TIMER_C015_DELAY_QUEUE, C015_DelayHandler.getNextScheduleTime());
    break;
  }

  case CPLUGIN_FLUSH:
  {
    process_c015_delay_queue();
    delay(0);
    break;
  }
  }
  return success;
}

bool do_process_c015_delay_queue(int controller_number, const C015_queue_element &element, ControllerSettingsStruct &ControllerSettings)
{

  if (!WiFiConnected(10))
  {
    return false;
  }

  WiFiClient client;
  if (!ControllerSettings.connectToHost(client))
  {
    connectionFailures++;
    addLog(LOG_LEVEL_ERROR, String(F("ZBX: Cannot connect")));
    return false;
  }
  statusLED(true);
  if (connectionFailures)
    connectionFailures--;

  LoadTaskSettings(element.TaskIndex);

  {

    byte valueCount = getValueCountFromSensorType(element.sensorType);
    // char itemNames[valueCount][2];
    for (byte x = 0; x < valueCount; x++)
    {


const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3);
DynamicJsonBuffer jsonBuffer(capacity);

JsonObject& root = jsonBuffer.createObject();
root["request"] = "sender data";

JsonArray& data = root.createNestedArray("data");

JsonObject& data_0 = data.createNestedObject();
data_0["host"] = Settings.Name;                            // zabbix hostname
data_0["key"] = ExtraTaskSettings.TaskDeviceValueNames[x]; //valueName // zabbix item
data_0["value"] = atof(element.txt[x].c_str());            
    
  char packet_header[] = "ZBXD\1";
  char packet_content[capacity];
  

  root.printTo((char*)packet_content, root.measureLength()+1);
  unsigned long long content_len = sizeof(packet_content);

  //addLog(LOG_LEVEL_INFO, String(F("ZBX: ")) + packet_content);
  ControllerSettings.connectToHost(client);
  client.write(packet_header, sizeof(packet_header) - 1);
  client.write((char *)&content_len, sizeof(content_len));
  client.write(packet_content, content_len);
  client.stop(); 
    }
  }

  client.stop();  
  return true;
}
#endif
