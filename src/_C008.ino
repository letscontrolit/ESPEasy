//#######################################################################################################
//########################### Controller Plugin 008: Generic HTTP #######################################
//#######################################################################################################

#define CPLUGIN_008
#define CPLUGIN_ID_008         8
#define CPLUGIN_NAME_008       "Generic HTTP"
#include <ArduinoJson.h>

boolean CPlugin_008(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_008;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesTemplate = true;
        Protocol[protocolCount].usesAccount = true;
        Protocol[protocolCount].usesPassword = true;
        Protocol[protocolCount].defaultPort = 80;
        Protocol[protocolCount].usesID = false;
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_008);
        break;
      }

    case CPLUGIN_PROTOCOL_TEMPLATE:
      {
        event->String1 = "";
        event->String2 = F("demo.php?name=%sysname%&task=%tskname%&valuename=%valname%&value=%value%");
        break;
      }

    case CPLUGIN_PROTOCOL_SEND:
      {
        byte valueCount = getValueCountFromSensorType(event->sensorType);
        for (byte x = 0; x < valueCount; x++)
        {
          if (event->sensorType == SENSOR_TYPE_LONG)
            HTTPSend(event, 0, 0, (unsigned long)UserVar[event->BaseVarIndex] + ((unsigned long)UserVar[event->BaseVarIndex + 1] << 16));
          else
            HTTPSend(event, x, UserVar[event->BaseVarIndex + x], 0);
          if (valueCount > 1)
          {
            unsigned long timer = millis() + Settings.MessageDelay;
            while (millis() < timer)
              backgroundtasks();
          }
        }
        break;
      }

  }
  return success;
}


//********************************************************************************
// Generic HTTP get request
//********************************************************************************
boolean HTTPSend(struct EventStruct *event, byte varIndex, float value, unsigned long longValue)
{
  ControllerSettingsStruct ControllerSettings;
  LoadControllerSettings(event->ControllerIndex, (byte*)&ControllerSettings, sizeof(ControllerSettings));

  String authHeader = "";
  if ((SecuritySettings.ControllerUser[event->ControllerIndex][0] != 0) && (SecuritySettings.ControllerPassword[event->ControllerIndex][0] != 0))
  {
    base64 encoder;
    String auth = SecuritySettings.ControllerUser[event->ControllerIndex];
    auth += ":";
    auth += SecuritySettings.ControllerPassword[event->ControllerIndex];
    authHeader = PSTR("Authorization: Basic ") + encoder.encode(auth) + " \r\n";
  }

  char log[80];
  boolean success = false;
  char host[20];
  sprintf_P(host, PSTR("%u.%u.%u.%u"), ControllerSettings.IP[0], ControllerSettings.IP[1], ControllerSettings.IP[2], ControllerSettings.IP[3]);

  sprintf_P(log, PSTR("%s%s using port %u"), PSTR("HTTP : connecting to "), host, ControllerSettings.Port);
  addLog(LOG_LEVEL_DEBUG, log);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, ControllerSettings.Port))
  {
    connectionFailures++;
    strcpy_P(log, PSTR("HTTP : connection failed"));
    addLog(LOG_LEVEL_ERROR, log);
    return false;
  }
  statusLED(true);
  if (connectionFailures)
    connectionFailures--;

  if (ExtraTaskSettings.TaskDeviceValueNames[0][0] == 0)
    PluginCall(PLUGIN_GET_DEVICEVALUENAMES, event, dummyString);

  String url = "/";
  url += ControllerSettings.Publish;
  url.replace(F("%sysname%"), URLEncode(Settings.Name));
  url.replace(F("%tskname%"), URLEncode(ExtraTaskSettings.TaskDeviceName));
  url.replace(F("%id%"), String(event->idx));
  url.replace(F("%valname%"), URLEncode(ExtraTaskSettings.TaskDeviceValueNames[varIndex]));
  if (longValue)
    url.replace(F("%value%"), String(longValue));
  else
    url.replace(F("%value%"), toString(value, ExtraTaskSettings.TaskDeviceValueDecimals[varIndex]));

  url.toCharArray(log, 80);
  addLog(LOG_LEVEL_DEBUG_MORE, log);

  String hostName = host;
  if (ControllerSettings.UseDNS)
    hostName = ControllerSettings.HostName;

  // This will send the request to the server
  client.print(String(PSTR("GET ")) + url + PSTR(" HTTP/1.1\r\n") +
               PSTR("Host: ") + hostName + "\r\n" + authHeader +
               PSTR("Connection: close\r\n\r\n"));

  unsigned long timer = millis() + 200;
  while (!client.available() && millis() < timer)
    delay(1);

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\n');
    line.toCharArray(log, 80);
    addLog(LOG_LEVEL_DEBUG_MORE, log);
    if (line.substring(0, 15) == F("HTTP/1.1 200 OK"))
    {
      strcpy_P(log, PSTR("HTTP : Succes!"));
      addLog(LOG_LEVEL_DEBUG, log);
      success = true;
    }
    delay(1);
  }
  strcpy_P(log, PSTR("HTTP : closing connection"));
  addLog(LOG_LEVEL_DEBUG, log);

  client.flush();
  client.stop();
}
