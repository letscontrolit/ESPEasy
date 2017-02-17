#ifdef PLUGIN_BUILD_DEV

//#######################################################################################################
//########################### Controller Plugin 025: Generic HTTP #######################################
//#######################################################################################################

#define CPLUGIN_025
#define CPLUGIN_ID_025         25
#define CPLUGIN_NAME_025       "Generic HTTP TEST [DEVELOPMENT]"

#define P025_HTTP_METHOD_MAX_LEN          16
#define P025_HTTP_URI_MAX_LEN             240
#define P025_HTTP_HEADER_MAX_LEN          256
#define P025_HTTP_BODY_MAX_LEN            512

struct P025_ConfigStruct
{
  char          HttpMethod[P025_HTTP_METHOD_MAX_LEN];
  char          HttpUri[P025_HTTP_URI_MAX_LEN];
  char          HttpHeader[P025_HTTP_HEADER_MAX_LEN];
  char          HttpBody[P025_HTTP_BODY_MAX_LEN];
};

boolean CPlugin_025(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_025;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesAccount = false;
        Protocol[protocolCount].usesPassword = false;
        Protocol[protocolCount].defaultPort = 80;
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_025);
        break;
      }

    case CPLUGIN_WEBFORM_LOAD:
      {
        P025_ConfigStruct customConfig;
        LoadCustomControllerSettings((byte*)&customConfig, sizeof(customConfig));
        String methods[] = { F("GET"), F("POST"), F("PUT") };
        string += F("<TR><TD>HTTP Method :<TD><select name='P025httpmethod'>");
        for (int i = 0; i < 3; i++)
        {
          string += F("<option value='");
          string += methods[i] + "'";
          string += methods[i].equals(customConfig.HttpMethod) ? F(" selected='selected'") : F("");
          string += F(">");
          string += methods[i];
          string += F("</option>");
        }
        string += F("</select>");

        string += F("<TR><TD>HTTP URI:<TD><input type='text' name='P025httpuri' size=80 maxlength='");
        string += P025_HTTP_URI_MAX_LEN-1;
        string += F("' value='");
        string += customConfig.HttpUri;
        string += F("'>");

        string += F("<TR><TD>HTTP Header:<TD><textarea name='P025httpheader' rows='4' cols='50' maxlength='");
        string += P025_HTTP_HEADER_MAX_LEN-1;
        string += F("'>");
        string += customConfig.HttpHeader;
        string += F("</textarea>");

        string += F("<TR><TD>HTTP Body:<TD><textarea name='P025httpbody' rows='8' cols='50' maxlength='");
        string += P025_HTTP_BODY_MAX_LEN-1;
        string += F("'>");
        string += customConfig.HttpBody;
        string += F("</textarea>");
        break;
      }

    case CPLUGIN_WEBFORM_SAVE:
      {
        P025_ConfigStruct customConfig;
        String httpmethod = WebServer.arg("P025httpmethod");
        String httpuri = WebServer.arg("P025httpuri");
        String httpheader = WebServer.arg("P025httpheader");
        String httpbody = WebServer.arg("P025httpbody");
        strncpy(customConfig.HttpMethod, httpmethod.c_str(), sizeof(customConfig.HttpMethod));
        strncpy(customConfig.HttpUri, httpuri.c_str(), sizeof(customConfig.HttpUri));
        strncpy(customConfig.HttpHeader, httpheader.c_str(), sizeof(customConfig.HttpHeader));
        strncpy(customConfig.HttpBody, httpbody.c_str(), sizeof(customConfig.HttpBody));
        SaveCustomControllerSettings((byte*)&customConfig, sizeof(customConfig));
        break;
      }

    case CPLUGIN_PROTOCOL_SEND:
      {
        switch (event->sensorType)
        {
          case SENSOR_TYPE_SINGLE:                      // single value sensor, used for Dallas, BH1750, etc
          case SENSOR_TYPE_SWITCH:
          case SENSOR_TYPE_DIMMER:
            HTTPSend025(event, 0, UserVar[event->BaseVarIndex], 0);
            break;
          case SENSOR_TYPE_LONG:                      // single LONG value, stored in two floats (rfid tags)
            HTTPSend025(event, 0, 0, (unsigned long)UserVar[event->BaseVarIndex] + ((unsigned long)UserVar[event->BaseVarIndex + 1] << 16));
            break;
          case SENSOR_TYPE_TEMP_HUM:
          case SENSOR_TYPE_TEMP_BARO:
            {
              HTTPSend025(event, 0, UserVar[event->BaseVarIndex], 0);
              unsigned long timer = millis() + Settings.MessageDelay;
              while (millis() < timer)
                backgroundtasks();
              HTTPSend025(event, 1, UserVar[event->BaseVarIndex + 1], 0);
              break;
            }
          case SENSOR_TYPE_TEMP_HUM_BARO:
            {
              HTTPSend025(event, 0, UserVar[event->BaseVarIndex], 0);
              unsigned long timer = millis() + Settings.MessageDelay;
              while (millis() < timer)
                backgroundtasks();
              HTTPSend025(event, 1, UserVar[event->BaseVarIndex + 1], 0);
              timer = millis() + Settings.MessageDelay;
              while (millis() < timer)
                backgroundtasks();
              HTTPSend025(event, 2, UserVar[event->BaseVarIndex + 2], 0);
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
boolean HTTPSend025(struct EventStruct *event, byte varIndex, float value, unsigned long longValue)
{
  P025_ConfigStruct customConfig;
  LoadCustomControllerSettings((byte*)&customConfig, sizeof(customConfig));

  char log[80];
  boolean success = false;
  char host[20];
  sprintf_P(host, PSTR("%u.%u.%u.%u"), Settings.Controller_IP[0], Settings.Controller_IP[1], Settings.Controller_IP[2], Settings.Controller_IP[3]);

  sprintf_P(log, PSTR("%s%s using port %u"), "HTTP : connecting to ", host, Settings.ControllerPort);
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

  String hostName = host;
  if (Settings.UseDNS)
    hostName = Settings.ControllerHostName;

  String payload = String(customConfig.HttpMethod) + " /";
  payload += customConfig.HttpUri;
  payload += String(" HTTP/1.1\r\n") +
             "Host: " + hostName + ":" +  Settings.ControllerPort + "\r\n" +
             "Connection: close\r\n";

  if (strlen(customConfig.HttpHeader) > 0)
    payload += customConfig.HttpHeader;
  ReplaceTokenByValue(payload, event, varIndex, value, longValue);

  if (strlen(customConfig.HttpBody) > 0)
  {
    String body = String(customConfig.HttpBody);
    ReplaceTokenByValue(body, event, varIndex, value, longValue);
    payload += "\r\nContent-Length: " + String(body.length());
    payload += "\r\n\r\n" + body;
  }

  // This will send the request to the server
  client.print(payload);

  unsigned long timer = millis() + 200;
  while (!client.available() && millis() < timer)
    delay(1);

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\n');
    line.toCharArray(log, 80);
    addLog(LOG_LEVEL_DEBUG_MORE, log);
    if (line.substring(0, 10) == "HTTP/1.1 2")
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


//********************************************************************************
// Replace the token in a string by real value.
//********************************************************************************
void ReplaceTokenByValue(String& s, struct EventStruct *event, byte varIndex, float value, unsigned long longValue)
{
  s.replace("%sysname%", URLEncode(Settings.Name));
  s.replace("%tskname%", URLEncode(ExtraTaskSettings.TaskDeviceName));
  s.replace("%id%", String(event->idx));
  s.replace("%valname%", URLEncode(ExtraTaskSettings.TaskDeviceValueNames[varIndex]));
  if (longValue)
    s.replace("%value%", String(longValue));
  else
    s.replace("%value%", toString(value, ExtraTaskSettings.TaskDeviceValueDecimals[varIndex]));
}

#endif
