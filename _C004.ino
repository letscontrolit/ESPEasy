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
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_004);
        break;
      }
      
    case CPLUGIN_PROTOCOL_SEND:
      {
        char log[80];
        boolean success = false;
        char host[20];
        sprintf_P(host, PSTR("%u.%u.%u.%u"), Settings.Controller_IP[0], Settings.Controller_IP[1], Settings.Controller_IP[2], Settings.Controller_IP[3]);

        sprintf_P(log, PSTR("%s%s using port %u"), "HTTP : connecting to ", host,Settings.ControllerPort);
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

        String postDataStr = SecuritySettings.ControllerPassword; // "0UDNN17RW6XAS2E5" // api key

        switch (event->sensorType)
        {
          case SENSOR_TYPE_SINGLE:                      // single value sensor, used for Dallas, BH1750, etc
          case SENSOR_TYPE_SWITCH:
            postDataStr += F("&field");
            postDataStr += event->idx;
            postDataStr += "=";
            postDataStr += toString(UserVar[event->BaseVarIndex],ExtraTaskSettings.TaskDeviceValueDecimals[0]);
            break;
          case SENSOR_TYPE_TEMP_HUM:                      // dual value
          case SENSOR_TYPE_TEMP_BARO:
            postDataStr += F("&field");
            postDataStr += event->idx;
            postDataStr += "=";
            postDataStr += toString(UserVar[event->BaseVarIndex],ExtraTaskSettings.TaskDeviceValueDecimals[0]);
            postDataStr += F("&field");
            postDataStr += event->idx + 1;
            postDataStr += "=";
            postDataStr += toString(UserVar[event->BaseVarIndex + 1],ExtraTaskSettings.TaskDeviceValueDecimals[1]);
            break;
          case SENSOR_TYPE_TEMP_HUM_BARO:
            postDataStr += F("&field");
            postDataStr += event->idx;
            postDataStr += "=";
            postDataStr += toString(UserVar[event->BaseVarIndex],ExtraTaskSettings.TaskDeviceValueDecimals[0]);
            postDataStr += F("&field");
            postDataStr += event->idx + 1;
            postDataStr += "=";
            postDataStr += toString(UserVar[event->BaseVarIndex + 1],ExtraTaskSettings.TaskDeviceValueDecimals[1]);
            postDataStr += F("&field");
            postDataStr += event->idx + 2;
            postDataStr += "=";
            postDataStr += toString(UserVar[event->BaseVarIndex + 2],ExtraTaskSettings.TaskDeviceValueDecimals[2]);
            break;
        }
        postDataStr += F("\r\n\r\n");

        String postStr = F("POST /update HTTP/1.1\n");
        postStr += F("Host: api.thingspeak.com\n");
        postStr += F("Connection: close\n");
        postStr += F("X-THINGSPEAKAPIKEY: ");
        postStr += SecuritySettings.ControllerPassword;
        postStr += "\n";
        postStr += F("Content-Type: application/x-www-form-urlencoded\n");
        postStr += F("Content-Length: ");
        postStr += postDataStr.length();
        postStr += F("\n\n");
        postStr += postDataStr;

        // This will send the request to the server
        client.print(postStr);

        unsigned long timer = millis() + 200;
        while (!client.available() && millis() < timer)
          delay(1);

        // Read all the lines of the reply from server and print them to Serial
        while (client.available()) {
          String line = client.readStringUntil('\n');
          line.toCharArray(log, 80);
          addLog(LOG_LEVEL_DEBUG_MORE, log);
          if (line.substring(0, 15) == "HTTP/1.1 200 OK")
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

