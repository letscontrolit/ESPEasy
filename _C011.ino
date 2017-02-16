#ifdef PLUGIN_BUILD_TESTING

//#######################################################################################################
//########################### Controller Plugin 011: Blynk  #############################################
//#######################################################################################################

#define CPLUGIN_011
#define CPLUGIN_ID_011         11
#define CPLUGIN_NAME_011       "Blynk HTTP [TESTING]"




boolean CPlugin_011(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;



  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_011;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesAccount = false;
        Protocol[protocolCount].usesPassword = true;
        Protocol[protocolCount].defaultPort = 8443;
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_011);
        break;
      }

     case CPLUGIN_PROTOCOL_SEND:
      {

        String postDataStr = "";

        switch (event->sensorType)
        {
          case SENSOR_TYPE_SINGLE:                      // single value sensor, used for Dallas, BH1750, etc
            postDataStr = F("update/V") ;
            postDataStr += event->idx;
            postDataStr += F("?value=");
            postDataStr += toString(UserVar[event->BaseVarIndex],ExtraTaskSettings.TaskDeviceValueDecimals[0]);
            Blynk_get(postDataStr );
            break;
          case SENSOR_TYPE_TEMP_HUM:                      // dual value
          case SENSOR_TYPE_TEMP_BARO:
          case SENSOR_TYPE_DUAL:
            postDataStr = F("update/V") ;
            postDataStr += event->idx;
            postDataStr += F("?value=");
            postDataStr += toString(UserVar[event->BaseVarIndex],ExtraTaskSettings.TaskDeviceValueDecimals[0]);
            Blynk_get(postDataStr );

            postDataStr = F("update/V") ;
            postDataStr += event->idx + 1;
            postDataStr += F("?value=");
            postDataStr += toString(UserVar[event->BaseVarIndex + 1],ExtraTaskSettings.TaskDeviceValueDecimals[1]);
            Blynk_get(postDataStr );
            break;
          case SENSOR_TYPE_TEMP_HUM_BARO:
          case SENSOR_TYPE_TRIPLE:
            postDataStr = F("update/V") ;
            postDataStr += event->idx;
            postDataStr += F("?value=");
            postDataStr += toString(UserVar[event->BaseVarIndex],ExtraTaskSettings.TaskDeviceValueDecimals[0]);
            Blynk_get(postDataStr );

            postDataStr = F("update/V") ;
            postDataStr += event->idx + 1;
            postDataStr += F("?value=");
            postDataStr += toString(UserVar[event->BaseVarIndex + 1],ExtraTaskSettings.TaskDeviceValueDecimals[1]);
            Blynk_get(postDataStr );

            postDataStr = F("update/V") ;
            postDataStr += event->idx + 2;
            postDataStr += F("?value=");
            postDataStr += toString(UserVar[event->BaseVarIndex + 2],ExtraTaskSettings.TaskDeviceValueDecimals[2]);
            Blynk_get(postDataStr );

            break;
          case SENSOR_TYPE_SWITCH:
            break;
        }



        break;
      }

  }
  return success;
}

boolean Blynk_get(String command,float *data )
{
  boolean success = false;
  char host[20];
  char log[80];
  char command_char[50];

  command.toCharArray(command_char, 50);

  sprintf_P(host, PSTR("%u.%u.%u.%u"), Settings.Controller_IP[0], Settings.Controller_IP[1], Settings.Controller_IP[2], Settings.Controller_IP[3]);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, Settings.ControllerPort))
  {
    connectionFailures++;
    return false;
  }
  if (connectionFailures)
    connectionFailures--;

  // We now create a URI for the request
  char requete[300];
  sprintf_P(requete, PSTR("GET /%s/%s HTTP/1.1\r\n Host: %s \r\n Connection: close\r\n\r\n"),SecuritySettings.ControllerPassword,command_char , host   );
  addLog(LOG_LEVEL_DEBUG, requete);
  client.print(requete);

  unsigned long timer = millis() + 200;
  while (!client.available() && millis() < timer)
    delay(1);

  // Read all the lines of the reply from server and print them to Serial

  while (client.available()) {

    String line = client.readStringUntil('\n');

    // success ?
    if (line.substring(0, 15) == "HTTP/1.1 200 OK") {
      strcpy_P(log, PSTR("HTTP : Success"));
      success = true;
    }
    else if (line.substring(0, 24) == "HTTP/1.1 400 Bad Request") {
      strcpy_P(log, PSTR("HTTP : Unauthorized"));
    }
    else if (line.substring(0, 25) == "HTTP/1.1 401 Unauthorized") {
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

      char value_char[5] ;
      strValue.toCharArray(value_char, 5);
      sprintf_P(log, PSTR("Blynk get - %s => %s"),command_char , value_char   );
      addLog(LOG_LEVEL_DEBUG, log);
    }


  }
  strcpy_P(log, PSTR("HTTP : closing connection"));
  addLog(LOG_LEVEL_DEBUG, log);

  client.flush();
  client.stop();

  // important - backgroudtasks - free mem
  timer = millis() + Settings.MessageDelay;
  while (millis() < timer)
              backgroundtasks();

  return success;
}


#endif
