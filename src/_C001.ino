#ifdef USES_C001
//#######################################################################################################
//########################### Controller Plugin 001: Domoticz HTTP ######################################
//#######################################################################################################

#define CPLUGIN_001
#define CPLUGIN_ID_001         1
#define CPLUGIN_NAME_001       "Domoticz HTTP"


boolean CPlugin_001(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_001;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesAccount = true;
        Protocol[protocolCount].usesPassword = true;
        Protocol[protocolCount].defaultPort = 8080;
        Protocol[protocolCount].usesID = true;
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_001);
        break;
      }

    case CPLUGIN_PROTOCOL_SEND:
      {
        if (event->idx != 0)
        {
          if (!WiFiConnected(100)) {
            success = false;
            break;
          }
          ControllerSettingsStruct ControllerSettings;
          LoadControllerSettings(event->ControllerIndex, (byte*)&ControllerSettings, sizeof (ControllerSettings));

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

          // boolean success = false;
          addLog(LOG_LEVEL_DEBUG, String(F("HTTP : connecting to "))+ControllerSettings.getHostPortString());


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

          // We now create a URI for the request
          String url = F("/json.htm?type=command&param=udevice&idx=");
          url += event->idx;

          switch (event->sensorType)
          {
            case SENSOR_TYPE_SWITCH:
              url = F("/json.htm?type=command&param=switchlight&idx=");
              url += event->idx;
              url += F("&switchcmd=");
              if (UserVar[event->BaseVarIndex] == 0)
                url += F("Off");
              else
                url += F("On");
              break;
            case SENSOR_TYPE_DIMMER:
              url = F("/json.htm?type=command&param=switchlight&idx=");
              url += event->idx;
              url += F("&switchcmd=");
              if (UserVar[event->BaseVarIndex] == 0) {
                url += ("Off");
              } else {
                url += F("Set%20Level&level=");
                url += UserVar[event->BaseVarIndex];
              }
              break;

            case SENSOR_TYPE_SINGLE:
            case SENSOR_TYPE_LONG:
            case SENSOR_TYPE_DUAL:
            case SENSOR_TYPE_TRIPLE:
            case SENSOR_TYPE_QUAD:
            case SENSOR_TYPE_TEMP_HUM:
            case SENSOR_TYPE_TEMP_BARO:
            case SENSOR_TYPE_TEMP_EMPTY_BARO:
            case SENSOR_TYPE_TEMP_HUM_BARO:
            case SENSOR_TYPE_WIND:
            default:
              url = F("/json.htm?type=command&param=udevice&idx=");
              url += event->idx;
              url += F("&nvalue=0");
              url += F("&svalue=");
              url += formatDomoticzSensorType(event);
              break;
          }

          // Add WiFi reception quality
          url += F("&rssi=");
          url += mapRSSItoDomoticz();
          #if FEATURE_ADC_VCC
            url += F("&battery=");
            url += mapVccToDomoticz();
          #endif

          // This will send the request to the server
          String request = F("GET ");
          request += url;
          request += F(" HTTP/1.1\r\n");
          request += F("Host: ");
          request += ControllerSettings.getHost();
          request += F("\r\n");
          request += authHeader;
          request += F("Connection: close\r\n\r\n");
          client.print(request);

          unsigned long timer = millis() + 200;
          while (!client.available() && !timeOutReached(timer))
            yield();

          // Read all the lines of the reply from server and log them
          while (client.available()) {
            // String line = client.readStringUntil('\n');
            String line;
            safeReadStringUntil(client, line, '\n');
            addLog(LOG_LEVEL_DEBUG_MORE, line);
            if (line.startsWith(F("HTTP/1.1 200 OK")) )
            {
              addLog(LOG_LEVEL_DEBUG, F("HTTP : Success"));
              success = true;
            }
            yield();
          }
          addLog(LOG_LEVEL_DEBUG, F("HTTP : closing connection"));

          client.flush();
          client.stop();
        } // if ixd !=0
        else
        {
          addLog(LOG_LEVEL_ERROR, F("HTTP : IDX cannot be zero!"));
        }
        break;
      }
  }
  return success;
}
#endif
