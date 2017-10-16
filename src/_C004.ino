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
        Protocol[protocolCount].usesAccount = false;
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

    case CPLUGIN_PROTOCOL_SEND:
      {
        ControllerSettingsStruct ControllerSettings;
        LoadControllerSettings(event->ControllerIndex, (byte*)&ControllerSettings, sizeof(ControllerSettings));

        char log[80];
        // boolean success = false;
        char host[20];
        sprintf_P(host, PSTR("%u.%u.%u.%u"), ControllerSettings.IP[0], ControllerSettings.IP[1], ControllerSettings.IP[2], ControllerSettings.IP[3]);

        sprintf_P(log, PSTR("%s%s using port %u"), "HTTP : connecting to ", host,ControllerSettings.Port);
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

        String postDataStr = F("api_key=");
        postDataStr += SecuritySettings.ControllerPassword[event->ControllerIndex]; // used for API key

        byte valueCount = getValueCountFromSensorType(event->sensorType);
        for (byte x = 0; x < valueCount; x++)
        {
          postDataStr += F("&field");
          postDataStr += event->idx + x;
          postDataStr += "=";
          postDataStr += formatUserVar(event, x);
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
        while (!client.available() && millis() < timer)
          delay(1);

        // Read all the lines of the reply from server and print them to Serial
        while (client.available()) {
          //   String line = client.readStringUntil('\n');
          String line;
          safeReadStringUntil(client, line, '\n');

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
        break;
      }

  }
  return success;
}
