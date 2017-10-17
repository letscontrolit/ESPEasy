//#######################################################################################################
//########################### Controller Plugin 011: Generic HTTP #######################################
//#######################################################################################################

#ifdef PLUGIN_BUILD_TESTING

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

  IPAddress host(ControllerSettings.IP[0], ControllerSettings.IP[1], ControllerSettings.IP[2], ControllerSettings.IP[3]);

  addLog(LOG_LEVEL_DEBUG, String(F("HTTP : connecting to "))+
  		(ControllerSettings.UseDNS ? ControllerSettings.HostName : host.toString() ) +":"+ControllerSettings.Port);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
   if (ControllerSettings.UseDNS ? !client.connect(ControllerSettings.HostName, ControllerSettings.Port) : !client.connect(host, ControllerSettings.Port))
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

  String hostName = host.toString();
  if (ControllerSettings.UseDNS)
    hostName = ControllerSettings.HostName;

  String payload = String(customConfig.HttpMethod) + " /";
  payload += customConfig.HttpUri;
  payload += F(" HTTP/1.1\r\n");
  payload += F("Host: ");
  payload += hostName + ":" + ControllerSettings.Port;
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
  while (!client.available() && millis() < timer)
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

	switch (event->sensorType)
	{
		case SENSOR_TYPE_SINGLE:
		case SENSOR_TYPE_SWITCH:
		case SENSOR_TYPE_DIMMER:
		case SENSOR_TYPE_WIND:
		case SENSOR_TYPE_LONG:
		{
			DeleteNotNeededValues(s,1);
			break;
		}
		case SENSOR_TYPE_DUAL:
		case SENSOR_TYPE_TEMP_HUM:
		case SENSOR_TYPE_TEMP_BARO:
		{
			DeleteNotNeededValues(s,2);
			break;
		}
		case SENSOR_TYPE_TRIPLE:
		case SENSOR_TYPE_TEMP_HUM_BARO:
		{
			DeleteNotNeededValues(s,3);
			break;
		}
		case SENSOR_TYPE_QUAD:
		{
			DeleteNotNeededValues(s,4);
			break;
		}
	}

	addLog(LOG_LEVEL_DEBUG_MORE, F("HTTP after parsing: "));
	addLog(LOG_LEVEL_DEBUG_MORE, s);

  //NOTE: cant we just call parseTemplate() for all the standard stuff??

  s.replace(F("%systime%"), getTimeString(':'));

	#if FEATURE_ADC_VCC
		s.replace(F("%vcc%"), String(vcc));
	#endif

  // IPAddress ip = WiFi.localIP();
  // char strIP[20];
  // sprintf_P(strIP, PSTR("%u.%u.%u.%u"), ip[0], ip[1], ip[2], ip[3]);
  s.replace(F("%ip%"),WiFi.localIP().toString());

  s.replace(F("%sysload%"), String(100 - (100 * loopCounterLast / loopCounterMax)));
  s.replace(F("%uptime%"), String(wdcounter / 2));

  s.replace(F("%CR%"), F("\r"));
  s.replace(F("%LF%"), F("\n"));
  s.replace(F("%sysname%"), URLEncode(Settings.Name));
  s.replace(F("%tskname%"), URLEncode(ExtraTaskSettings.TaskDeviceName));
  s.replace(F("%id%"), String(event->idx));
  s.replace(F("%vname1%"), URLEncode(ExtraTaskSettings.TaskDeviceValueNames[0]));
  s.replace(F("%vname2%"), URLEncode(ExtraTaskSettings.TaskDeviceValueNames[1]));
  s.replace(F("%vname3%"), URLEncode(ExtraTaskSettings.TaskDeviceValueNames[2]));
  s.replace(F("%vname4%"), URLEncode(ExtraTaskSettings.TaskDeviceValueNames[3]));

  if (event->sensorType == SENSOR_TYPE_LONG)
    s.replace(F("%val1%"), String((unsigned long)UserVar[event->BaseVarIndex] + ((unsigned long)UserVar[event->BaseVarIndex + 1] << 16)));
  else {
    s.replace(F("%val1%"), formatUserVar(event, 0));
    s.replace(F("%val2%"), formatUserVar(event, 1));
    s.replace(F("%val3%"), formatUserVar(event, 2));
    s.replace(F("%val4%"), formatUserVar(event, 3));
  }
	addLog(LOG_LEVEL_DEBUG_MORE, F("HTTP after replacements: "));
	addLog(LOG_LEVEL_DEBUG_MORE, s);
}

#endif
