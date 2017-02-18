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
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_001);
        break;
      }

    case CPLUGIN_PROTOCOL_SEND:
      {
        String authHeader = "";
        if ((SecuritySettings.ControllerUser[0] != 0) && (SecuritySettings.ControllerPassword[0] != 0))
        {
          base64 encoder;
          String auth = SecuritySettings.ControllerUser;
          auth += ":";
          auth += SecuritySettings.ControllerPassword;
          authHeader = "Authorization: Basic " + encoder.encode(auth) + " \r\n";
        }

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

        // We now create a URI for the request
        String url = F("/json.htm?type=command&param=udevice&idx=");
        url += event->idx;

        switch (event->sensorType)
        {
          case SENSOR_TYPE_SINGLE:                      // single value sensor, used for Dallas, BH1750, etc
            url += F("&svalue=");
            url += toString(UserVar[event->BaseVarIndex],ExtraTaskSettings.TaskDeviceValueDecimals[0]);
            break;
          case SENSOR_TYPE_LONG:                      // single LONG value, stored in two floats (rfid tags)
            url += F("&svalue=");
            url += (unsigned long)UserVar[event->BaseVarIndex] + ((unsigned long)UserVar[event->BaseVarIndex + 1] << 16);
            break;
          case SENSOR_TYPE_DUAL:                       // any sensor that uses two simple values
            url += F("&svalue=");
            url += toString(UserVar[event->BaseVarIndex],ExtraTaskSettings.TaskDeviceValueDecimals[0]);
            url += ";";
            url += toString(UserVar[event->BaseVarIndex + 1],ExtraTaskSettings.TaskDeviceValueDecimals[1]);
            break;
          case SENSOR_TYPE_TEMP_HUM:                      // temp + hum + hum_stat, used for DHT11
            url += F("&svalue=");
            url += toString(UserVar[event->BaseVarIndex],ExtraTaskSettings.TaskDeviceValueDecimals[0]);
            url += ";";
            url += toString(UserVar[event->BaseVarIndex + 1],ExtraTaskSettings.TaskDeviceValueDecimals[1]);
            url += ";";
            url += humStat(UserVar[event->BaseVarIndex + 1]);
            break;
          case SENSOR_TYPE_TEMP_BARO:                      // temp + hum + hum_stat + bar + bar_fore, used for BMP085
            url += F("&svalue=");
            url += toString(UserVar[event->BaseVarIndex],ExtraTaskSettings.TaskDeviceValueDecimals[0]);
            url += ";0;0;";
            url += toString(UserVar[event->BaseVarIndex + 1],ExtraTaskSettings.TaskDeviceValueDecimals[1]);
            url += ";0";
            break;
          case SENSOR_TYPE_TEMP_HUM_BARO:                      // temp + hum + hum_stat + bar + bar_fore, used for BME280
            url += F("&svalue=");
            url += toString(UserVar[event->BaseVarIndex],ExtraTaskSettings.TaskDeviceValueDecimals[0]);
            url += ";";
            url += toString(UserVar[event->BaseVarIndex + 1],ExtraTaskSettings.TaskDeviceValueDecimals[1]);
            url += ";";
            url += humStat(UserVar[event->BaseVarIndex + 1]);
            url += ";";
            url += toString(UserVar[event->BaseVarIndex + 2],ExtraTaskSettings.TaskDeviceValueDecimals[2]);
            url += ";0";
            break;
          case SENSOR_TYPE_SWITCH:
            url = F("/json.htm?type=command&param=switchlight&idx=");
            url += event->idx;
            url += F("&switchcmd=");
            if (UserVar[event->BaseVarIndex] == 0)
              url += "Off";
            else
              url += "On";
            break;
          case SENSOR_TYPE_DIMMER:
            url = F("/json.htm?type=command&param=switchlight&idx=");
            url += event->idx;
            url += F("&switchcmd=");
            if (UserVar[event->BaseVarIndex] == 0)
              url += "Off";
            else
            {
              url += F("Set%20Level&level=");
              url += UserVar[event->BaseVarIndex];
            }
            break;

#ifdef PLUGIN_186
            case (SENSOR_TYPE_WIND):
              url += F("&svalue=");
              url += toString(UserVar[event->BaseVarIndex],ExtraTaskSettings.TaskDeviceValueDecimals[0]);
              char* bearing[] = {";N;",";NNE;",";NE;",";ENE;",";E;",";ESE;",";SE;",";SSE;",";S;",";SSW;",";SW;",";WSW;",";W;",";WNW;",";NW;",";NNW;" };
              url += bearing[int(UserVar[event->BaseVarIndex] / 22.5)];
              url += toString(UserVar[event->BaseVarIndex + 1],ExtraTaskSettings.TaskDeviceValueDecimals[1]);
              url += ";";
              url += toString(UserVar[event->BaseVarIndex + 2],ExtraTaskSettings.TaskDeviceValueDecimals[2]);
              url += ";0";
              break;
#endif
        }

        url.toCharArray(log, 80);
        addLog(LOG_LEVEL_DEBUG_MORE, log);

        // This will send the request to the server
        client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                     "Host: " + host + "\r\n" + authHeader +
                     "Connection: close\r\n\r\n");

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
            strcpy_P(log, PSTR("HTTP : Success"));
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

int humStat(int hum){
  int lHumStat;
  if(hum<30){
     lHumStat = 2;
  }else if(hum<40){
    lHumStat = 0;
  }else if(hum<59){
    lHumStat = 1;
  }else{
    lHumStat = 3;

  }
  return lHumStat;
}
