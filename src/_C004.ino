#ifdef USES_C004
//#######################################################################################################
//########################### Controller Plugin 004: ThingSpeak #########################################
//#######################################################################################################

#define CPLUGIN_004
#define CPLUGIN_ID_004         4
#define CPLUGIN_NAME_004       "ThingSpeak"

boolean CPlugin_004(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_004;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesAccount = true;
        Protocol[protocolCount].usesPassword = true;
        Protocol[protocolCount].defaultPort = 80;
        Protocol[protocolCount].usesID = true;
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_004);
        break;
      }

    case CPLUGIN_GET_PROTOCOL_DISPLAY_NAME:
      {
        success = true;
        switch (event->idx) {
          case CONTROLLER_USER:
            string = F("ThingHTTP Name");
            break;
          case CONTROLLER_PASS:
            string = F("API Key");
            break;
          default:
            success = false;
            break;
        }
      }

    case CPLUGIN_PROTOCOL_SEND:
      {
        ControllerSettingsStruct ControllerSettings;
        LoadControllerSettings(event->ControllerIndex, (byte*)&ControllerSettings, sizeof(ControllerSettings));

        // boolean success = false;
        addLog(LOG_LEVEL_DEBUG, String(F("HTTP : connecting to "))+ControllerSettings.getHostPortString());
        char log[80];
        // Use WiFiClient class to create TCP connections
        WiFiClient client;
        if (!ControllerSettings.connectToHost(client))
        {
          connectionFailures++;
          if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
            strcpy_P(log, PSTR("HTTP : connection failed"));
            addLog(LOG_LEVEL_ERROR, log);
          }
          return false;
        }
        statusLED(true);
        if (connectionFailures)
          connectionFailures--;

        String postDataStr = F("api_key=");
        postDataStr += SecuritySettings.ControllerPassword[event->ControllerIndex]; // used for API key

        byte valueCount = getValueCountFromSensorType(event->sensorType);
        for (byte x = 0; x < valueCount; x++)
        {
          postDataStr += F("&field");
          postDataStr += event->idx + x;
          postDataStr += "=";
          postDataStr += formatUserVarNoCheck(event, x);
        }
        String hostName = F("api.thingspeak.com"); // PM_CZ: HTTP requests must contain host headers.
        if (ControllerSettings.UseDNS)
          hostName = ControllerSettings.HostName;

        String postStr = F("POST /update HTTP/1.1\r\n");
        postStr += F("Host: ");
        postStr += hostName;
        postStr += F("\r\n");
        postStr += F("Connection: close\r\n");

        postStr += F("Content-Type: application/x-www-form-urlencoded\r\n");
        postStr += F("Content-Length: ");
        postStr += postDataStr.length();
        postStr += F("\r\n\r\n");
        postStr += postDataStr;

        // This will send the request to the server
        client.print(postStr);

        unsigned long timer = millis() + 200;
        while (!client.available() && !timeOutReached(timer))
          delay(1);

        // Read all the lines of the reply from server and print them to Serial
        while (client.available()) {
          //   String line = client.readStringUntil('\n');
          String line;
          safeReadStringUntil(client, line, '\n');

          if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) {
            line.toCharArray(log, 80);
            addLog(LOG_LEVEL_DEBUG_MORE, log);
          }
          if (line.substring(0, 15) == F("HTTP/1.1 200 OK"))
          {
            if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
              strcpy_P(log, PSTR("HTTP : Success!"));
              addLog(LOG_LEVEL_DEBUG, log);
            }
            success = true;
          }
          delay(1);
        }
        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          strcpy_P(log, PSTR("HTTP : closing connection"));
          addLog(LOG_LEVEL_DEBUG, log);
        }

        client.flush();
        client.stop();
        break;
      }

  }
  return success;
}
#endif
