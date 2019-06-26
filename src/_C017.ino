#ifdef USES_C017
//#######################################################################################################
//###########################   Controller Plugin 017: ZABBIX  ##########################################
//#######################################################################################################
// Based on https://www.zabbix.com/documentation/current/manual/appendix/items/trapper
// and https://www.zabbix.com/documentation/4.2/manual/appendix/protocols/header_datalen

// USAGE: at Zabbix server you go at Configuration -> Hosts -> Create host
// The "Host name" should match exactly the EspEasy name (Config -> Unit Name)
// Add a group (mandatory) and hit add. No need to set up IP address or agent.
// Go to the newly created host ->Items ->Create Item
// Nane the item something descriptive
// For Key add the EspEasy task Value name (case sensitive)
// Type of information select "Numeric (float)" and press add. Thats it.

#define CPLUGIN_017
#define CPLUGIN_ID_017 17
#define CPLUGIN_NAME_017 "Zabbix"
#include <ArduinoJson.h>

bool CPlugin_017(byte function, struct EventStruct *event, String &string)
{
  bool success = false;

  switch (function)
  {
  case CPLUGIN_PROTOCOL_ADD:
  {
    Protocol[++protocolCount].Number = CPLUGIN_ID_017;
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
    string = F(CPLUGIN_NAME_017);
    break;
  }

  case CPLUGIN_PROTOCOL_SEND:
  {
    byte valueCount = getValueCountFromSensorType(event->sensorType);
    C017_queue_element element(event);

    MakeControllerSettings(ControllerSettings);
    LoadControllerSettings(event->ControllerIndex, ControllerSettings);

    for (byte x = 0; x < valueCount; x++)
    {
      element.txt[x] = formatUserVarNoCheck(event, x);
    }
    success = C017_DelayHandler.addToQueue(element);
    scheduleNextDelayQueue(TIMER_C017_DELAY_QUEUE, C017_DelayHandler.getNextScheduleTime());
    break;
  }

  case CPLUGIN_FLUSH:
  {
    process_c017_delay_queue();
    delay(0);
    break;
  }
  }
  return success;
}

bool do_process_c017_delay_queue(int controller_number, const C017_queue_element &element, ControllerSettingsStruct &ControllerSettings)
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

  byte valueCount = getValueCountFromSensorType(element.sensorType);
  // char itemNames[valueCount][2];

  const size_t capacity = JSON_ARRAY_SIZE(4) + JSON_OBJECT_SIZE(2) + 4 * JSON_OBJECT_SIZE(3); //Size for esp8266: 513
  DynamicJsonBuffer jsonBuffer(capacity);

  JsonObject &root = jsonBuffer.createObject();
  root["request"] = "sender data";

  JsonArray &data = root.createNestedArray("data");

  //In my understanding ESPEasy wont return more than 4 Device Values per task
  JsonObject &data_0 = data.createNestedObject();
  data_0["host"] = Settings.Name;                            // zabbix hostname, Unit Name for the ESP easy
  data_0["key"] = ExtraTaskSettings.TaskDeviceValueNames[0]; //valueName // zabbix item
  data_0["value"] = atof(element.txt[0].c_str());            // ESPeasy supports only floats

  if (valueCount >= 1)
  {
    JsonObject &data_1 = data.createNestedObject();
    data_1["host"] = Settings.Name;                            // zabbix hostname, Unit Name for the ESP easy
    data_1["key"] = ExtraTaskSettings.TaskDeviceValueNames[1]; //valueName // zabbix item key
    data_1["value"] = atof(element.txt[1].c_str());            // ESPeasy supports only floats
  }
  if (valueCount >= 2)
  {
    JsonObject &data_2 = data.createNestedObject();
    data_2["host"] = Settings.Name;                            // zabbix hostname, Unit Name for the ESP easy
    data_2["key"] = ExtraTaskSettings.TaskDeviceValueNames[2]; //valueName // zabbix item key
    data_2["value"] = atof(element.txt[2].c_str());            // ESPeasy supports only floats
  }
  if (valueCount >= 3)
  {
    JsonObject &data_3 = data.createNestedObject();
    data_3["host"] = Settings.Name;                            // zabbix hostname, Unit Name for the ESP easy
    data_3["key"] = ExtraTaskSettings.TaskDeviceValueNames[3]; //valueName // zabbix item key
    data_3["value"] = atof(element.txt[3].c_str());            // ESPeasy supports only floats
  }

  char packet_header[] = "ZBXD\1";
  char packet_content[capacity];

  root.printTo((char *)packet_content, root.measureLength() + 1);
  unsigned long long content_len = sizeof(packet_content);

  //addLog(LOG_LEVEL_INFO, String(F("ZBX: ")) + packet_content);
  ControllerSettings.connectToHost(client);
  client.write(packet_header, sizeof(packet_header) - 1);
  client.write((char *)&content_len, sizeof(content_len));
  client.write(packet_content, content_len);
  
  client.stop();
  return true;
}
#endif
