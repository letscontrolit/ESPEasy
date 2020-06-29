#ifdef USES_C011
//#######################################################################################################
//########################### Controller Plugin 011: Generic HTTP Advanced ##############################
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
  void zero_last() {
    HttpMethod[C011_HTTP_METHOD_MAX_LEN - 1] = 0;
    HttpUri[C011_HTTP_URI_MAX_LEN - 1] = 0;
    HttpHeader[C011_HTTP_HEADER_MAX_LEN - 1] = 0;
    HttpBody[C011_HTTP_BODY_MAX_LEN - 1] = 0;
  }

  char          HttpMethod[C011_HTTP_METHOD_MAX_LEN] = {0};
  char          HttpUri[C011_HTTP_URI_MAX_LEN] = {0};
  char          HttpHeader[C011_HTTP_HEADER_MAX_LEN] = {0};
  char          HttpBody[C011_HTTP_BODY_MAX_LEN] = {0};
};

bool CPlugin_011(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_011;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesAccount = true;
        Protocol[protocolCount].usesPassword = true;
        Protocol[protocolCount].usesExtCreds = true;
        Protocol[protocolCount].defaultPort = 80;
        Protocol[protocolCount].usesID = false;
        break;
      }

    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_011);
        break;
      }

    case CPlugin::Function::CPLUGIN_WEBFORM_LOAD:
      {
        String escapeBuffer;
        std::shared_ptr<C011_ConfigStruct> customConfig(new C011_ConfigStruct);
        if (customConfig) {
          LoadCustomControllerSettings(event->ControllerIndex,(byte*)customConfig.get(), sizeof(customConfig));
          customConfig->zero_last();
          {
            byte choice = 0;
            String methods[] = { F("GET"), F("POST"), F("PUT"), F("HEAD"), F("PATCH") };
            for (byte i = 0; i < 5; i++)
            {
              if (methods[i].equals(customConfig->HttpMethod)) {
                choice = i;
              }
            }
            addFormSelector(F("HTTP Method"), F("P011httpmethod"), 5, methods, NULL, choice);
          }

          addFormTextBox(F("HTTP URI"), F("P011httpuri"), customConfig->HttpUri, C011_HTTP_URI_MAX_LEN-1);
          {
            String escapeBuffer = customConfig->HttpHeader;
            htmlEscape(escapeBuffer);
            addFormTextArea(F("HTTP Header"), F("P011httpheader"), escapeBuffer, C011_HTTP_HEADER_MAX_LEN-1, 4, 50);
          }
          {
            String escapeBuffer = customConfig->HttpBody;
            htmlEscape(escapeBuffer);
            addFormTextArea(F("HTTP Body"), F("P011httpbody"), escapeBuffer, C011_HTTP_BODY_MAX_LEN-1, 8, 50);
          }
        }
        break;
      }

    case CPlugin::Function::CPLUGIN_WEBFORM_SAVE:
      {
        std::shared_ptr<C011_ConfigStruct> customConfig(new C011_ConfigStruct);
        if (customConfig) {
          byte choice = 0;
          String methods[] = { F("GET"), F("POST"), F("PUT"), F("HEAD"), F("PATCH") };
          for (byte i = 0; i < 5; i++)
          {
            if (methods[i].equals(customConfig->HttpMethod)) {
              choice = i;
            }
          }

          int httpmethod = getFormItemInt(F("P011httpmethod"), choice);
          String httpuri = web_server.arg(F("P011httpuri"));
          String httpheader = web_server.arg(F("P011httpheader"));
          String httpbody = web_server.arg(F("P011httpbody"));

          strlcpy(customConfig->HttpMethod, methods[httpmethod].c_str(), sizeof(customConfig->HttpMethod));
          strlcpy(customConfig->HttpUri, httpuri.c_str(), sizeof(customConfig->HttpUri));
          strlcpy(customConfig->HttpHeader, httpheader.c_str(), sizeof(customConfig->HttpHeader));
          strlcpy(customConfig->HttpBody, httpbody.c_str(), sizeof(customConfig->HttpBody));
          customConfig->zero_last();
          SaveCustomControllerSettings(event->ControllerIndex,(byte*)customConfig.get(), sizeof(customConfig));
        }
        break;
      }

    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
      {
      	success = Create_schedule_HTTP_C011(event);
        break;
      }

    case CPlugin::Function::CPLUGIN_FLUSH:
      {
        process_c011_delay_queue();
        delay(0);
        break;
      }

    default:
      break;

  }
  return success;
}

//********************************************************************************
// Generic HTTP request
//********************************************************************************

// Uncrustify may change this into multi line, which will result in failed builds
// *INDENT-OFF*
bool do_process_c011_delay_queue(int controller_number, const C011_queue_element& element, ControllerSettingsStruct& ControllerSettings);
// *INDENT-ON*

bool do_process_c011_delay_queue(int controller_number, const C011_queue_element& element, ControllerSettingsStruct& ControllerSettings) {
  WiFiClient client;
  if (!try_connect_host(controller_number, client, ControllerSettings))
    return false;

  return send_via_http(controller_number, client, element.txt, ControllerSettings.MustCheckReply);
}

//********************************************************************************
// Create request
//********************************************************************************
boolean Create_schedule_HTTP_C011(struct EventStruct *event)
{
  String authHeader;
  String hostportString;
  if (ExtraTaskSettings.TaskIndex != event->TaskIndex) {
    String dummy;
    PluginCall(PLUGIN_GET_DEVICEVALUENAMES, event, dummy);
  }

  {
    // Have the ControllerSettings in a separate scope so we can free it as soon as possible
    MakeControllerSettings(ControllerSettings);
    if (!AllocatedControllerSettings()) {
      return false;
    }
    LoadControllerSettings(event->ControllerIndex, ControllerSettings);

    authHeader = get_auth_header(event->ControllerIndex, ControllerSettings);
    const bool defaultport = ControllerSettings.Port == 0 || ControllerSettings.Port == 80;
    hostportString = defaultport ? ControllerSettings.getHost() : ControllerSettings.getHostPortString();
  }


  std::shared_ptr<C011_ConfigStruct> customConfig(new C011_ConfigStruct);
  if (!customConfig) {
    return false;
  }
  LoadCustomControllerSettings(event->ControllerIndex,(byte*)customConfig.get(), sizeof(customConfig));
  customConfig->zero_last();

  bool success = false;
  {
    String payload = do_create_http_request(
      hostportString,
      customConfig->HttpMethod,
      customConfig->HttpUri,
      authHeader,
      "",
      -1);

    // Remove extra newline, see https://github.com/letscontrolit/ESPEasy/issues/1970
    removeExtraNewLine(payload);

    // Add a new element to the queue with the minimal payload
    success = C011_DelayHandler.addToQueue(C011_queue_element(event->ControllerIndex, payload));
  }
  if (success) {
    // Element was added.
    // Now we try to append to the existing element 
    // and thus preventing the need to create a long string only to copy it to a queue element.
    C011_queue_element &element = C011_DelayHandler.sendQueue.back();

    if (strlen(customConfig->HttpHeader) > 0) {
      element.txt += customConfig->HttpHeader;
      removeExtraNewLine(element.txt);
    }
    ReplaceTokenByValue(element.txt, event);

    if (strlen(customConfig->HttpBody) > 0)
    {
      String body = String(customConfig->HttpBody);
      ReplaceTokenByValue(body, event);
      element.txt += F("Content-Length: ");
      element.txt += String(body.length());
      addNewLine(element.txt);
      addNewLine(element.txt); // Need 2 CRLF between header and body.
      element.txt += body;
    }
    addNewLine(element.txt);
  }

  scheduleNextDelayQueue(TIMER_C011_DELAY_QUEUE, C011_DelayHandler.getNextScheduleTime());
  return success;
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
				s.replace(p, "");

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
