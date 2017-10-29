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
          IPAddress host(ControllerSettings.IP[0], ControllerSettings.IP[1], ControllerSettings.IP[2], ControllerSettings.IP[3]);
          addLog(LOG_LEVEL_DEBUG, String(F("HTTP : connecting to "))+host.toString()+":"+ControllerSettings.Port);


          // Use WiFiClient class to create TCP connections
          WiFiClient client;
          if (!client.connect(host, ControllerSettings.Port))
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
            case SENSOR_TYPE_SINGLE:                      // single value sensor, used for Dallas, BH1750, etc
              url += F("&svalue=");
              url += formatUserVar(event, 0);
              break;
            case SENSOR_TYPE_LONG:                      // single LONG value, stored in two floats (rfid tags)
              url += F("&svalue=");
              url += (unsigned long)UserVar[event->BaseVarIndex] + ((unsigned long)UserVar[event->BaseVarIndex + 1] << 16);
              break;
            case SENSOR_TYPE_DUAL:                       // any sensor that uses two simple values
              url += F("&svalue=");
              url += formatUserVar(event, 0);
              url += (";");
              url += formatUserVar(event, 1);
              break;
            case SENSOR_TYPE_TEMP_HUM:                      // temp + hum + hum_stat, used for DHT11
              url += F("&svalue=");
              url += formatUserVar(event, 0);
              url += F(";");
              url += formatUserVar(event, 1);
              url += F(";");
              url += humStat(UserVar[event->BaseVarIndex + 1]);
              break;
            case SENSOR_TYPE_TEMP_BARO:                      // temp + hum + hum_stat + bar + bar_fore, used for BMP085
              url += F("&svalue=");
              url += formatUserVar(event, 0);
              url += F(";0;0;");
              url += formatUserVar(event, 1);
              url += F(";0");
              break;
            case SENSOR_TYPE_TRIPLE:
              url += F("&svalue=");
              url += formatUserVar(event, 0);
              url += F(";");
              url += formatUserVar(event, 1);
              url += F(";");
              url += formatUserVar(event, 2);
              break;
            case SENSOR_TYPE_TEMP_HUM_BARO:                      // temp + hum + hum_stat + bar + bar_fore, used for BME280
              url += F("&svalue=");
              url += formatUserVar(event, 0);
              url += F(";");
              url += formatUserVar(event, 1);
              url += F(";");
              url += humStat(UserVar[event->BaseVarIndex + 1]);
              url += F(";");
              url += formatUserVar(event, 2);
              url += F(";0");
              break;
            case SENSOR_TYPE_QUAD:
              url += F("&svalue=");
              url += formatUserVar(event, 0);
              url += F(";");
              url += formatUserVar(event, 1);
              url += F(";");
              url += formatUserVar(event, 2);
              url += F(";");
              url += formatUserVar(event, 3);
              break;
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
              if (UserVar[event->BaseVarIndex] == 0)
                url += ("Off");
              else
              {
                url += F("Set%20Level&level=");
                url += UserVar[event->BaseVarIndex];
              }
              break;
            case (SENSOR_TYPE_WIND):
              url += F("&svalue=");                   // WindDir in degrees; WindDir as text; Wind speed average ; Wind speed gust; 0
              url += formatUserVar(event, 0);
              url += ";";
              url += getBearing(UserVar[event->BaseVarIndex]);
              url += ";";
              // Domoticz expects the wind speed in (m/s * 10)
              url += toString((UserVar[event->BaseVarIndex + 1] * 10),ExtraTaskSettings.TaskDeviceValueDecimals[1]);
              url += ";";
              url += toString((UserVar[event->BaseVarIndex + 2] * 10),ExtraTaskSettings.TaskDeviceValueDecimals[2]);
              url += ";0";
              break;
          }


          // This will send the request to the server
          String request = F("GET ");
          request += url;
          request += F(" HTTP/1.1\r\n");
          request += F("Host: ");
          request += host.toString();
          request += F("\r\n");
          request += authHeader;
          request += F("Connection: close\r\n\r\n");
          client.print(request);

          unsigned long timer = millis() + 200;
          while (!client.available() && millis() < timer)
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
