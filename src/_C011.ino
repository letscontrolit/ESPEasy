#ifdef USES_C011
//#######################################################################################################
//########################### Controller Plugin 011: Generic HTTP #######################################
//#######################################################################################################

// #ifdef PLUGIN_BUILD_TESTING

#define CPLUGIN_011
#define CPLUGIN_ID_011         11
#define CPLUGIN_NAME_011       "Generic HTTP Advanced [TESTING]"

#define C011_HTTP_METHOD_MAX_LEN          16
#define C011_HTTP_URI_MAX_LEN             240
#define C011_HTTP_HEADER_MAX_LEN          256
#define C011_HTTP_BODY_MAX_LEN            512

struct C011_ConfigStruct
{
  char          HttpMethod[C011_HTTP_METHOD_MAX_LEN];
  char          HttpUri[C011_HTTP_URI_MAX_LEN];
  char          HttpHeader[C011_HTTP_HEADER_MAX_LEN];
  char          HttpBody[C011_HTTP_BODY_MAX_LEN];
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
        Protocol[protocolCount].usesAccount = true;
        Protocol[protocolCount].usesPassword = true;
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
        String escapeBuffer;

        C011_ConfigStruct customConfig;

        LoadCustomControllerSettings(event->ControllerIndex,(byte*)&customConfig, sizeof(customConfig));
        String methods[] = { F("GET"), F("POST"), F("PUT"), F("HEAD"), F("PATCH") };
        string += F("<TR><TD>HTTP Method :<TD><select name='P011httpmethod'>");
        for (byte i = 0; i < 5; i++)
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
        string += C011_HTTP_URI_MAX_LEN-1;
        string += F("' value='");
        string += customConfig.HttpUri;

        string += F("'>");

        string += F("<TR><TD>HTTP Header:<TD><textarea name='P011httpheader' rows='4' cols='50' maxlength='");
        string += C011_HTTP_HEADER_MAX_LEN-1;
        string += F("'>");
        escapeBuffer=customConfig.HttpHeader;
        htmlEscape(escapeBuffer);
        string += escapeBuffer;
        string += F("</textarea>");

        string += F("<TR><TD>HTTP Body:<TD><textarea name='P011httpbody' rows='8' cols='50' maxlength='");
        string += C011_HTTP_BODY_MAX_LEN-1;
        string += F("'>");
        escapeBuffer=customConfig.HttpBody;
        htmlEscape(escapeBuffer);
        string += escapeBuffer;
        string += F("</textarea>");
        break;
      }

    case CPLUGIN_WEBFORM_SAVE:
      {
        C011_ConfigStruct customConfig;
        String httpmethod = WebServer.arg(F("P011httpmethod"));
        String httpuri = WebServer.arg(F("P011httpuri"));
        String httpheader = WebServer.arg(F("P011httpheader"));
        String httpbody = WebServer.arg(F("P011httpbody"));

        strlcpy(customConfig.HttpMethod, httpmethod.c_str(), sizeof(customConfig.HttpMethod));
        strlcpy(customConfig.HttpUri, httpuri.c_str(), sizeof(customConfig.HttpUri));
        strlcpy(customConfig.HttpHeader, httpheader.c_str(), sizeof(customConfig.HttpHeader));
        strlcpy(customConfig.HttpBody, httpbody.c_str(), sizeof(customConfig.HttpBody));
        SaveCustomControllerSettings(event->ControllerIndex,(byte*)&customConfig, sizeof(customConfig));
        break;
      }

    case CPLUGIN_PROTOCOL_SEND:
      {
      	HTTPSend011(event);
      }

  }
  return success;
}


//********************************************************************************
// Generic HTTP get request
//********************************************************************************
boolean HTTPSend011(struct EventStruct *event)
{
  if (!WiFiConnected(100)) {
    return false;
  }
  ControllerSettingsStruct ControllerSettings;
  LoadControllerSettings(event->ControllerIndex, (byte*)&ControllerSettings, sizeof(ControllerSettings));

  String authHeader = "";
  if ((SecuritySettings.ControllerUser[event->ControllerIndex][0] != 0) && (SecuritySettings.ControllerPassword[event->ControllerIndex][0] != 0))
  {
    base64 encoder;
    String auth = SecuritySettings.ControllerUser[event->ControllerIndex];
    auth += ":";
    auth += SecuritySettings.ControllerPassword[event->ControllerIndex];
    authHeader = F("Authorization: Basic ");
    authHeader += encoder.encode(auth);
    authHeader += F(" \r\n");
  }

  C011_ConfigStruct customConfig;
  LoadCustomControllerSettings(event->ControllerIndex,(byte*)&customConfig, sizeof(customConfig));

  boolean success = false;
  addLog(LOG_LEVEL_DEBUG, String(F("HTTP : connecting to "))+
      ControllerSettings.getHostPortString());

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!ControllerSettings.connectToHost(client))
  {
    connectionFailures++;
    addLog(LOG_LEVEL_ERROR, F("HTTP : connection failed"));
    return false;
  }
  statusLED(true);
  if (connectionFailures)
    connectionFailures--;

  if (ExtraTaskSettings.TaskDeviceValueNames[0][0] == 0)
    PluginCall(PLUGIN_GET_DEVICEVALUENAMES, event, dummyString);

  String payload = String(customConfig.HttpMethod) + " /";
  payload += customConfig.HttpUri;
  payload += F(" HTTP/1.1\r\n");
  payload += F("Host: ");
  payload += ControllerSettings.getHostPortString();
  payload += F("\r\n");
  payload += authHeader;
  payload += F("Connection: close\r\n");

  if (strlen(customConfig.HttpHeader) > 0)
    payload += customConfig.HttpHeader;
  ReplaceTokenByValue(payload, event);

  if (strlen(customConfig.HttpBody) > 0)
  {
    String body = String(customConfig.HttpBody);
    ReplaceTokenByValue(body, event);
    payload += F("\r\nContent-Length: ");
    payload += String(body.length());
    payload += F("\r\n\r\n");
    payload += body;
  }
  payload += F("\r\n");

  // This will send the request to the server
  client.print(payload);
  addLog(LOG_LEVEL_DEBUG_MORE, payload);

  unsigned long timer = millis() + 200;
  while (!client.available() && !timeOutReached(timer))
    yield();

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    // String line = client.readStringUntil('\n');
    String line;
    safeReadStringUntil(client, line, '\n');


    // line.toCharArray(log, 80);
    addLog(LOG_LEVEL_DEBUG_MORE, line);
    if (line.startsWith(F("HTTP/1.1 2")))
    {
      addLog(LOG_LEVEL_DEBUG, F("HTTP : Success!"));
      success = true;
    }
    yield();
  }
  addLog(LOG_LEVEL_DEBUG, F("HTTP : closing connection"));

  client.flush();
  client.stop();

  return(success);
}

// parses the string and returns only the the number of name/values we want
// according to the parameter numberOfValuesWanted
void DeleteNotNeededValues(String &s, byte numberOfValuesWanted)
{
	numberOfValuesWanted++;
	for (byte i=1; i < 5; i++)
	{
    String startToken=String(F("%")) + i + F("%");
    String endToken=String(F("%/")) + i + F("%");

    //do we want to keep this one?
    if (i<numberOfValuesWanted)
    {
      //yes, so just remove the tokens
      s.replace(startToken, "");
      s.replace(endToken, "");
    }
    else
    {
      //remove all the whole strings including tokes
      int startIndex=s.indexOf(startToken);
      int endIndex=s.indexOf(endToken);
      while(startIndex != -1 && endIndex != -1  && endIndex>startIndex)
  		{
        String p = s.substring(startIndex,endIndex+4);
        //remove the whole string including tokens
				s.replace(p, F(""));

        //find next ones
        startIndex=s.indexOf(startToken);
        endIndex=s.indexOf(endToken);
  		}
    }
	}
}


//********************************************************************************
// Replace the token in a string by real value.
//
// Example:
// %1%%vname1%____%tskname%____%val1%%/1%%2%%__%%vname2%____%tskname%____%val2%%/2%
// will become in case of a sensor with 1 value:
// SENSORVALUENAME1____TASKNAME1____VALUE1  <- everything not between %1% and %/1% will be discarded
// in case of a sensor with 2 values:
// SENSORVALUENAME1____TASKNAME1____VALUE1__SENSORVALUENAME2____TASKNAME2____VALUE2
//********************************************************************************
void ReplaceTokenByValue(String& s, struct EventStruct *event)
{
// example string:
// write?db=testdb&type=%1%%vname1%%/1%%2%;%vname2%%/2%%3%;%vname3%%/3%%4%;%vname4%%/4%&value=%1%%val1%%/1%%2%;%val2%%/2%%3%;%val3%%/3%%4%;%val4%%/4%
//	%1%%vname1%,Standort=%tskname% Wert=%val1%%/1%%2%%LF%%vname2%,Standort=%tskname% Wert=%val2%%/2%%3%%LF%%vname3%,Standort=%tskname% Wert=%val3%%/3%%4%%LF%%vname4%,Standort=%tskname% Wert=%val4%%/4%
	addLog(LOG_LEVEL_DEBUG_MORE, F("HTTP before parsing: "));
	addLog(LOG_LEVEL_DEBUG_MORE, s);
  const byte valueCount = getValueCountFromSensorType(event->sensorType);
  DeleteNotNeededValues(s,valueCount);

	addLog(LOG_LEVEL_DEBUG_MORE, F("HTTP after parsing: "));
	addLog(LOG_LEVEL_DEBUG_MORE, s);

  parseControllerVariables(s, event, true);

	addLog(LOG_LEVEL_DEBUG_MORE, F("HTTP after replacements: "));
	addLog(LOG_LEVEL_DEBUG_MORE, s);
}

#endif

