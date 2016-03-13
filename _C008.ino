//#######################################################################################################
//########################### Controller Plugin 008: Generic HTTP #######################################
//#######################################################################################################

#define CPLUGIN_008
#define CPLUGIN_ID_008         8
#define CPLUGIN_NAME_008       "Generic HTTP"

boolean CPlugin_008(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_008;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesAccount = false;
        Protocol[protocolCount].usesPassword = false;
        Protocol[protocolCount].defaultPort = 80;
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_008);
        break;
      }
      
    case CPLUGIN_PROTOCOL_SEND:
      {
        switch (event->sensorType)
        {
          case SENSOR_TYPE_SINGLE:                      // single value sensor, used for Dallas, BH1750, etc
          case SENSOR_TYPE_SWITCH:
          case SENSOR_TYPE_DIMMER:
            HTTPSend(event, 0, UserVar[event->BaseVarIndex], 0);
            break;
          case SENSOR_TYPE_LONG:                      // single LONG value, stored in two floats (rfid tags)
            HTTPSend(event, 0, 0, (unsigned long)UserVar[event->BaseVarIndex] + ((unsigned long)UserVar[event->BaseVarIndex + 1] << 16));
            break;
          case SENSOR_TYPE_TEMP_HUM:
          case SENSOR_TYPE_TEMP_BARO:
            {
              HTTPSend(event, 0, UserVar[event->BaseVarIndex], 0);
              unsigned long timer = millis() + Settings.MessageDelay;
              while (millis() < timer)
                backgroundtasks();
              HTTPSend(event, 1, UserVar[event->BaseVarIndex + 1], 0);
              break;
            }
          case SENSOR_TYPE_TEMP_HUM_BARO:
            {
              HTTPSend(event, 0, UserVar[event->BaseVarIndex], 0);
              unsigned long timer = millis() + Settings.MessageDelay;
              while (millis() < timer)
                backgroundtasks();
              HTTPSend(event, 1, UserVar[event->BaseVarIndex + 1], 0);
              timer = millis() + Settings.MessageDelay;
              while (millis() < timer)
                backgroundtasks();
              HTTPSend(event, 2, UserVar[event->BaseVarIndex + 2], 0);
              break;
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
  char log[80];
  boolean success = false;
  char host[20];
  sprintf_P(host, PSTR("%u.%u.%u.%u"), Settings.Controller_IP[0], Settings.Controller_IP[1], Settings.Controller_IP[2], Settings.Controller_IP[3]);

  sprintf_P(log, PSTR("%s%s using port %u"), "HTTP : connecting to ", host,Settings.ControllerPort);
  addLog(LOG_LEVEL_DEBUG, log);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, Settings.ControllerPort))
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
  url += Settings.MQTTpublish;
  url.replace("%sysname%", URLEncode(Settings.Name));
  url.replace("%tskname%", URLEncode(ExtraTaskSettings.TaskDeviceName));
  url.replace("%id%", String(event->idx));
  url.replace("%valname%", URLEncode(ExtraTaskSettings.TaskDeviceValueNames[varIndex]));
  if (longValue)
    url.replace("%value%", String(longValue));
  else
    url.replace("%value%", String(value));

  url.toCharArray(log, 80);
  addLog(LOG_LEVEL_DEBUG_MORE, log);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  unsigned long timer = millis() + 200;
  while (!client.available() && millis() < timer)
    delay(1);

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\n');
    line.toCharArray(log, 80);
    addLog(LOG_LEVEL_DEBUG_MORE, log);
    if (line.substring(0, 15) == "HTTP/1.1 200 OK")
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

