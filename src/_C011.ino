//#######################################################################################################
//########################### Controller Plugin 011: Generic HTTP #######################################
//#######################################################################################################

#define CPLUGIN_011
#define CPLUGIN_ID_011         11
#define CPLUGIN_NAME_011       "Generic HTTP Advanced"

#define P011_HTTP_METHOD_MAX_LEN          16
#define P011_HTTP_URI_MAX_LEN             240
#define P011_HTTP_HEADER_MAX_LEN          256
#define P011_HTTP_BODY_MAX_LEN            512
 
struct P011_ConfigStruct
{
  char          HttpMethod[P011_HTTP_METHOD_MAX_LEN];
  char          HttpUri[P011_HTTP_URI_MAX_LEN];
  char          HttpHeader[P011_HTTP_HEADER_MAX_LEN];
  char          HttpBody[P011_HTTP_BODY_MAX_LEN];
};

boolean CPlugin_011(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_011;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesAccount = false;
        Protocol[protocolCount].usesPassword = false;
        Protocol[protocolCount].defaultPort = 80;
        Protocol[protocolCount].usesID = false;
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_011);
        break;
      }

    case CPLUGIN_WEBFORM_LOAD:
      {
        P011_ConfigStruct customConfig;
        LoadCustomControllerSettings((byte*)&customConfig, sizeof(customConfig));
        String methods[] = { F("GET"), F("POST"), F("PUT") };
        string += F("<TR><TD>HTTP Method :<TD><select name='P011httpmethod'>");
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

        string += F("<TR><TD>HTTP URI:<TD><input type='text' name='P011httpuri' size=80 maxlength='");
        string += P011_HTTP_URI_MAX_LEN-1;
        string += F("' value='");
        string += customConfig.HttpUri;
        string += F("'>");

        string += F("<TR><TD>HTTP Header:<TD><textarea name='P011httpheader' rows='4' cols='50' maxlength='");
        string += P011_HTTP_HEADER_MAX_LEN-1;
        string += F("'>");
        string += customConfig.HttpHeader;
        string += F("</textarea>");

        string += F("<TR><TD>HTTP Body:<TD><textarea name='P011httpbody' rows='8' cols='50' maxlength='");
        string += P011_HTTP_BODY_MAX_LEN-1;
        string += F("'>");
        string += customConfig.HttpBody;
        string += F("</textarea>");
        break;
      }

    case CPLUGIN_WEBFORM_SAVE:
      {
        P011_ConfigStruct customConfig;
        String httpmethod = WebServer.arg(F("P011httpmethod"));
        String httpuri = WebServer.arg(F("P011httpuri"));
        String httpheader = WebServer.arg(F("P011httpheader"));
        String httpbody = WebServer.arg(F("P011httpbody"));
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
            HTTPSend011(event, 0, UserVar[event->BaseVarIndex], 0);
            break;
          case SENSOR_TYPE_LONG:                      // single LONG value, stored in two floats (rfid tags)
            HTTPSend011(event, 0, 0, (unsigned long)UserVar[event->BaseVarIndex] + ((unsigned long)UserVar[event->BaseVarIndex + 1] << 16));
            break;
          case SENSOR_TYPE_TEMP_HUM:
          case SENSOR_TYPE_TEMP_BARO:
            {
              HTTPSend011(event, 0, UserVar[event->BaseVarIndex], 0);
              unsigned long timer = millis() + Settings.MessageDelay;
              while (millis() < timer)
                backgroundtasks();
              HTTPSend011(event, 1, UserVar[event->BaseVarIndex + 1], 0);
              break;
            }
          case SENSOR_TYPE_TEMP_HUM_BARO:
            {
              HTTPSend011(event, 0, UserVar[event->BaseVarIndex], 0);
              unsigned long timer = millis() + Settings.MessageDelay;
              while (millis() < timer)
                backgroundtasks();
              HTTPSend011(event, 1, UserVar[event->BaseVarIndex + 1], 0);
              timer = millis() + Settings.MessageDelay;
              while (millis() < timer)
                backgroundtasks();
              HTTPSend011(event, 2, UserVar[event->BaseVarIndex + 2], 0);
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
boolean HTTPSend011(struct EventStruct *event, byte varIndex, float value, unsigned long longValue)
{
  ControllerSettingsStruct ControllerSettings;
  LoadControllerSettings(event->ControllerIndex, (byte*)&ControllerSettings, sizeof(ControllerSettings));

  P011_ConfigStruct customConfig;
  LoadCustomControllerSettings((byte*)&customConfig, sizeof(customConfig));

  char log[80];
  boolean success = false;
  char host[20];
  sprintf_P(host, PSTR("%u.%u.%u.%u"), ControllerSettings.IP[0], ControllerSettings.IP[1], ControllerSettings.IP[2], ControllerSettings.IP[3]);

  sprintf_P(log, PSTR("%s%s using port %u"), "HTTP : connecting to ", host, ControllerSettings.Port);
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

  String hostName = host;
  if (ControllerSettings.UseDNS)
    hostName = ControllerSettings.HostName[event->ProtocolIndex];
    
  String payload = String(customConfig.HttpMethod) + " /";
  payload += customConfig.HttpUri;
  payload += F(" HTTP/1.1\r\n");
  payload += F("Host: ");
  payload += hostName;
  payload += F("\r\nConnection: close\r\n\r\n");

  if (strlen(customConfig.HttpHeader) > 0)
    payload += customConfig.HttpHeader;
  ReplaceTokenByValue(payload, event, varIndex, value, longValue);

  if (strlen(customConfig.HttpBody) > 0)
  {
    String body = String(customConfig.HttpBody);
    ReplaceTokenByValue(body, event, varIndex, value, longValue);
    payload += F("\r\nContent-Length: ");
    payload += String(body.length());
    payload += F("\r\n\r\n");
    payload += body;
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


//********************************************************************************
// Replace the token in a string by real value.
//********************************************************************************
void ReplaceTokenByValue(String& s, struct EventStruct *event, byte varIndex, float value, unsigned long longValue)
{
  s.replace(F("%sysname%"), URLEncode(Settings.Name));
  s.replace(F("%tskname%"), URLEncode(ExtraTaskSettings.TaskDeviceName));
  s.replace(F("%id%"), String(event->idx));
  s.replace(F("%valname%"), URLEncode(ExtraTaskSettings.TaskDeviceValueNames[varIndex]));
  if (longValue)
    s.replace(F("%value%"), String(longValue));
  else
    s.replace(F("%value%"), toString(value, ExtraTaskSettings.TaskDeviceValueDecimals[varIndex]));
}

