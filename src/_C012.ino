#ifdef USES_C012
//#######################################################################################################
//########################### Controller Plugin 012: Blynk  #############################################
//#######################################################################################################

// #ifdef PLUGIN_BUILD_TESTING

#define CPLUGIN_012
#define CPLUGIN_ID_012         12
#define CPLUGIN_NAME_012       "Blynk HTTP [TESTING]"

bool CPlugin_012(byte function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_012;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesAccount = false;
        Protocol[protocolCount].usesPassword = true;
        Protocol[protocolCount].defaultPort = 80;
        Protocol[protocolCount].usesID = true;
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_012);
        break;
      }

     case CPLUGIN_PROTOCOL_SEND:
      {
        // Collect the values at the same run, to make sure all are from the same sample
        byte valueCount = getValueCountFromSensorType(event->sensorType);
        C012_queue_element element(event, valueCount);
        if (ExtraTaskSettings.TaskIndex != event->TaskIndex)
          PluginCall(PLUGIN_GET_DEVICEVALUENAMES, event, dummyString);

        MakeControllerSettings(ControllerSettings);
        LoadControllerSettings(event->ControllerIndex, ControllerSettings);

        for (byte x = 0; x < valueCount; x++)
        {
          bool isvalid;
          String formattedValue = formatUserVar(event, x, isvalid);
          if (isvalid) {
            element.txt[x] = F("update/V");
            element.txt[x] += event->idx + x;
            element.txt[x] += F("?value=");
            element.txt[x] += formattedValue;
            addLog(LOG_LEVEL_DEBUG_MORE, element.txt[x]);
          }
        }
        success = C012_DelayHandler.addToQueue(element);
        scheduleNextDelayQueue(TIMER_C012_DELAY_QUEUE, C012_DelayHandler.getNextScheduleTime());
        break;
      }
  }
  return success;
}

//********************************************************************************
// Process Queued Blynk request, with data set to NULL
//********************************************************************************
bool do_process_c012_delay_queue(int controller_number, const C012_queue_element& element, ControllerSettingsStruct& ControllerSettings) {
  while (element.txt[element.valuesSent] == "") {
    // A non valid value, which we are not going to send.
    // Increase sent counter until a valid value is found.
    if (element.checkDone(true))
      return true;
  }
  if (wifiStatus != ESPEASY_WIFI_SERVICES_INITIALIZED) {
    return false;
  }
  return element.checkDone(Blynk_get(element.txt[element.valuesSent], element.controller_idx));
}

boolean Blynk_get(const String& command, byte controllerIndex, float *data )
{
  MakeControllerSettings(ControllerSettings);
  LoadControllerSettings(controllerIndex, ControllerSettings);

  if ((SecuritySettings.ControllerPassword[controllerIndex][0] == 0)) {
    addLog(LOG_LEVEL_ERROR, F("Blynk : No password set"));
    return false;
  }

  WiFiClient client;
  if (!try_connect_host(CPLUGIN_ID_012, client, ControllerSettings))
    return false;


  // We now create a URI for the request
  char request[300] = {0};
  sprintf_P(request,
            PSTR("GET /%s/%s HTTP/1.1\r\n Host: %s \r\n Connection: close\r\n\r\n"),
            SecuritySettings.ControllerPassword[controllerIndex],
            command.c_str(),
            ControllerSettings.getHost().c_str());
  addLog(LOG_LEVEL_DEBUG, request);
  client.print(request);
  bool success = !ControllerSettings.MustCheckReply;
  if (ControllerSettings.MustCheckReply || data) {
    unsigned long timer = millis() + 200;
    while (!client_available(client) && !timeOutReached(timer))
      delay(1);

    char log[80] = {0};
    timer = millis() + 1500;
    // Read all the lines of the reply from server and log them
    while (client_available(client) && !success && !timeOutReached(timer)) {
      String line;
      safeReadStringUntil(client, line, '\n');
      addLog(LOG_LEVEL_DEBUG_MORE, line);
      // success ?
      if (line.substring(0, 15) == F("HTTP/1.1 200 OK")) {
        strcpy_P(log, PSTR("HTTP : Success"));
        if (!data) success = true;
      }
      else if (line.substring(0, 24) == F("HTTP/1.1 400 Bad Request")) {
        strcpy_P(log, PSTR("HTTP : Unauthorized"));
      }
      else if (line.substring(0, 25) == F("HTTP/1.1 401 Unauthorized")) {
        strcpy_P(log, PSTR("HTTP : Unauthorized"));
      }
      addLog(LOG_LEVEL_DEBUG, log);

      // data only
      if (data && line.startsWith("["))
      {
        String strValue = line;
        byte pos = strValue.indexOf('"',2);
        strValue = strValue.substring(2, pos);
        strValue.trim();
        float value = strValue.toFloat();
        *data = value;
        success = true;

        char value_char[5] = {0};
        strValue.toCharArray(value_char, 5);
        sprintf_P(log, PSTR("Blynk get - %s => %s"),command.c_str(), value_char   );
        addLog(LOG_LEVEL_DEBUG, log);
      }
      delay(0);
    }
  }
  addLog(LOG_LEVEL_DEBUG, F("HTTP : closing connection (012)"));

  client.flush();
  client.stop();

  // important - backgroundtasks - free mem
  unsigned long timer = millis() + Settings.MessageDelay;
  while (!timeOutReached(timer))
              backgroundtasks();

  return success;
}
#endif
