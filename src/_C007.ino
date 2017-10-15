//#######################################################################################################
//########################### Controller Plugin 007: Emoncms ############################################
//#######################################################################################################

#define CPLUGIN_007
#define CPLUGIN_ID_007         7
#define CPLUGIN_NAME_007       "Emoncms"

boolean CPlugin_007(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_007;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesAccount = false;
        Protocol[protocolCount].usesPassword = true;
        Protocol[protocolCount].defaultPort = 80;
        Protocol[protocolCount].usesID = true;
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_007);
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

        String postDataStr = F("GET /emoncms/input/post.json?node=");

        postDataStr += Settings.Unit;
        postDataStr += F("&json=");

        switch (event->sensorType)
        {
          case SENSOR_TYPE_SINGLE:                      // single value sensor, used for Dallas, BH1750, etc
            postDataStr += F("{field");
            postDataStr += event->idx;
            postDataStr += ":";
            postDataStr += formatUserVar(event, 0);
            postDataStr += "}";
            break;
          case SENSOR_TYPE_TEMP_HUM:                      // dual value
          case SENSOR_TYPE_TEMP_BARO:
          case SENSOR_TYPE_DUAL:
            postDataStr += F("{field");
            postDataStr += event->idx;
            postDataStr += ":";
            postDataStr += formatUserVar(event, 0);
            postDataStr += F(",field");
            postDataStr += event->idx + 1;
            postDataStr += ":";
            postDataStr += formatUserVar(event, 1);
            postDataStr += "}";
            break;
          case SENSOR_TYPE_TEMP_HUM_BARO:
          case SENSOR_TYPE_TRIPLE:
            postDataStr += F("{field");
            postDataStr += event->idx;
            postDataStr += ":";
            postDataStr += formatUserVar(event, 0);
            postDataStr += F(",field");
            postDataStr += event->idx + 1;
            postDataStr += ":";
            postDataStr += formatUserVar(event, 1);
            postDataStr += F(",field");
            postDataStr += event->idx + 2;
            postDataStr += ":";
            postDataStr += formatUserVar(event, 2);
            postDataStr += "}";
            break;
          case SENSOR_TYPE_SWITCH:
            break;
        }
        postDataStr += F("&apikey=");
        postDataStr += SecuritySettings.ControllerPassword[event->ControllerIndex]; // "0UDNN17RW6XAS2E5" // api key

        String hostName = host;
        if (ControllerSettings.UseDNS)
          hostName = ControllerSettings.HostName;

        String postStr = F(" HTTP/1.1\r\n");
        postStr += F("Host: ");
        postStr += hostName;
        postStr += F("\r\n");
        postStr += F("Connection: close\r\n");
        postStr += F("\r\n");

        postDataStr += postStr;

        if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG_MORE)
          Serial.println(postDataStr);

        // This will send the request to the server
        client.print(postDataStr);

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
