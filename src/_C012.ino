#ifdef USES_C012
//#######################################################################################################
//########################### Controller Plugin 012: Blynk  #############################################
//#######################################################################################################

// #ifdef PLUGIN_BUILD_TESTING

#define CPLUGIN_012
#define CPLUGIN_ID_012         12
#define CPLUGIN_NAME_012       "Blynk HTTP [TESTING]"

boolean CPlugin_012(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

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
        if (wifiStatus != ESPEASY_WIFI_SERVICES_INITIALIZED) {
          success = false;
          break;
        }

        String postDataStr = F("");
        const byte valueCount = getValueCountFromSensorType(event->sensorType);
        success = CPlugin_012_send(event, valueCount);
        break;
      }
  }
  return success;
}

boolean CPlugin_012_send(struct EventStruct *event, int nrValues) {
  String postDataStr = F("");
  boolean success = true;
  for (int i = 0; i < nrValues && success; ++i) {
    postDataStr = F("update/V") ;
    postDataStr += event->idx + i;
    postDataStr += F("?value=");
    postDataStr += formatUserVarNoCheck(event, i);
    success = Blynk_get(postDataStr, event->ControllerIndex);
  }
  return success;
}


boolean Blynk_get(const String& command, byte controllerIndex, float *data )
{
  if (wifiStatus != ESPEASY_WIFI_SERVICES_INITIALIZED) {
    return false;
  }

  ControllerSettingsStruct ControllerSettings;
  LoadControllerSettings(controllerIndex, (byte*)&ControllerSettings, sizeof(ControllerSettings));
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if ((SecuritySettings.ControllerPassword[controllerIndex][0] == 0) || !ControllerSettings.connectToHost(client))
  {
    connectionFailures++;
    addLog(LOG_LEVEL_ERROR, F("Blynk : connection failed"));
    return false;
  }
  if (connectionFailures)
    connectionFailures--;

  // We now create a URI for the request
  char request[300] = {0};
  sprintf_P(request,
            PSTR("GET /%s/%s HTTP/1.1\r\n Host: %s \r\n Connection: close\r\n\r\n"),
            SecuritySettings.ControllerPassword[controllerIndex],
            command.c_str(),
            ControllerSettings.getHost().c_str());
  addLog(LOG_LEVEL_DEBUG, request);
  client.print(request);

  unsigned long timer = millis() + 200;
  while (!client.available() && !timeOutReached(timer))
    yield();

  boolean success = false;
  char log[80] = {0};

  // Read all the lines of the reply from server and log them
  while (client.available()) {
    String line;
    safeReadStringUntil(client, line, '\n');
    addLog(LOG_LEVEL_DEBUG_MORE, line);
    // success ?
    if (line.substring(0, 15) == F("HTTP/1.1 200 OK")) {
      strcpy_P(log, PSTR("HTTP : Success"));
      success = true;
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
    yield();
  }
  strcpy_P(log, PSTR("HTTP : closing connection"));
  addLog(LOG_LEVEL_DEBUG, log);

  client.flush();
  client.stop();

  // important - backgroudtasks - free mem
  timer = millis() + Settings.MessageDelay;
  while (!timeOutReached(timer))
              backgroundtasks();

  return success;
}
#endif
