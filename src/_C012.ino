//#######################################################################################################
//########################### Controller Plugin 012: Nettemp HTTP ######################################
//#######################################################################################################

#define CPLUGIN_012
#define CPLUGIN_ID_012         12
#define CPLUGIN_NAME_012       "Nettemp HTTP"

#define C012_KEY_MAX_LEN 16

struct C012_ConfigStruct
{
  char Key[C012_KEY_MAX_LEN];
};

C012_ConfigStruct customConfig;
boolean C012_config_loaded = false;

boolean CPlugin_012(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_012;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesAccount = true;
        Protocol[protocolCount].usesPassword = true;
        Protocol[protocolCount].defaultPort = 80;
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_012);
        break;
      }

    case CPLUGIN_WEBFORM_LOAD:
      {
        if (!C012_config_loaded) {
          LoadCustomControllerSettings((byte*)&customConfig, sizeof(customConfig));
          C012_config_loaded = true;
        }

        string += F("<TR><TD>Server key:<TD><input type='text' name='C012Key' size=80 maxlength='");
        string += C012_KEY_MAX_LEN - 1;
        string += F("' value='");
        string += customConfig.Key;
        string += F("'>");

        break;
      }

    case CPLUGIN_WEBFORM_SAVE:
      {
        String serverkey = WebServer.arg("C012Key");
        strncpy(customConfig.Key, serverkey.c_str(), sizeof(customConfig.Key));
        SaveCustomControllerSettings((byte*)&customConfig, sizeof(customConfig));
        break;
      }

    case CPLUGIN_PROTOCOL_SEND:
      {
        success = false;
        char log[80];

        if (!C012_config_loaded) {
          LoadCustomControllerSettings((byte*)&customConfig, sizeof(customConfig));
          C012_config_loaded = true;
        }

        if (!strlen(customConfig.Key))
        {
          connectionFailures++;
          strcpy_P(log, PSTR("Nettemp: no server key"));
          addLog(LOG_LEVEL_ERROR, log);
          return false;
        }

        String authHeader = "";
        if ((SecuritySettings.ControllerUser[0] != 0) && (SecuritySettings.ControllerPassword[0] != 0))
        {
          base64 encoder;
          String auth = SecuritySettings.ControllerUser;
          auth += ":";
          auth += SecuritySettings.ControllerPassword;
          authHeader = "Authorization: Basic " + encoder.encode(auth) + " \r\n";
        }

        char host[20];
        sprintf_P(host, PSTR("%u.%u.%u.%u"), Settings.Controller_IP[0], Settings.Controller_IP[1], Settings.Controller_IP[2], Settings.Controller_IP[3]);

        sprintf_P(log, PSTR("%s%s using port %u"), "HTTP : connecting to ", host, Settings.ControllerPort);
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

        IPAddress ip = WiFi.localIP();
        char strip[20];
        sprintf_P(strip, PSTR("%u.%u.%u.%u"), ip[0], ip[1], ip[2], ip[3]);

        // We now create a URI for the request
        String url = F("/receiver.php?");
        url += F("device=ip");
        url += F("&name=");
        url += ExtraTaskSettings.TaskDeviceName;

        url += F("&key=");
        url += customConfig.Key;
        url += F("&id=");
        url += event->idx;
        url += F("&type=");

        int nr = getValueCountFromSensorType(event->sensorType);
        if (nr >= 1) {
          url += ExtraTaskSettings.TaskDeviceValueNames[0];
        }
        if (nr >= 2) {
          url += ";";
          url += ExtraTaskSettings.TaskDeviceValueNames[1];
        }
        if (nr >= 3) {
          url += ";";
          url += ExtraTaskSettings.TaskDeviceValueNames[2];
        }
        if (nr >= 4) {
          url += ";";
          url += ExtraTaskSettings.TaskDeviceValueNames[3];
        }

        switch (event->sensorType)
        {
          case SENSOR_TYPE_SINGLE:                      // single value sensor, used for Dallas, BH1750, etc
            url += F("&value=");
            url += toString(UserVar[event->BaseVarIndex], ExtraTaskSettings.TaskDeviceValueDecimals[0]);
            break;
          case SENSOR_TYPE_LONG:                      // single LONG value, stored in two floats (rfid tags)
            url += F("&value=");
            url += (unsigned long)UserVar[event->BaseVarIndex] + ((unsigned long)UserVar[event->BaseVarIndex + 1] << 16);
            break;
          case SENSOR_TYPE_DUAL:                       // any sensor that uses two simple values
            url += F("&value=");
            url += toString(UserVar[event->BaseVarIndex], ExtraTaskSettings.TaskDeviceValueDecimals[0]);
            url += ";";
            url += toString(UserVar[event->BaseVarIndex + 1], ExtraTaskSettings.TaskDeviceValueDecimals[1]);
            break;
          case SENSOR_TYPE_TRIPLE:                       // any sensor that uses three simple values
            url += F("&value=");
            url += toString(UserVar[event->BaseVarIndex], ExtraTaskSettings.TaskDeviceValueDecimals[0]);
            url += ";";
            url += toString(UserVar[event->BaseVarIndex + 1], ExtraTaskSettings.TaskDeviceValueDecimals[1]);
            url += ";";
            url += toString(UserVar[event->BaseVarIndex + 2], ExtraTaskSettings.TaskDeviceValueDecimals[2]);
            break;
          case SENSOR_TYPE_QUAD:                       // any sensor that uses four simple values
            url += F("&value=");
            url += toString(UserVar[event->BaseVarIndex], ExtraTaskSettings.TaskDeviceValueDecimals[0]);
            url += ";";
            url += toString(UserVar[event->BaseVarIndex + 1], ExtraTaskSettings.TaskDeviceValueDecimals[1]);
            url += ";";
            url += toString(UserVar[event->BaseVarIndex + 2], ExtraTaskSettings.TaskDeviceValueDecimals[2]);
            url += ";";
            url += toString(UserVar[event->BaseVarIndex + 3], ExtraTaskSettings.TaskDeviceValueDecimals[3]);
            break;
          case SENSOR_TYPE_TEMP_HUM:                      // temp + hum + hum_stat, used for DHT11
            url += F("&value=");
            url += toString(UserVar[event->BaseVarIndex], ExtraTaskSettings.TaskDeviceValueDecimals[0]);
            url += ";";
            url += toString(UserVar[event->BaseVarIndex + 1], ExtraTaskSettings.TaskDeviceValueDecimals[1]);
            break;
          case SENSOR_TYPE_TEMP_BARO:                      // temp + hum + hum_stat + bar + bar_fore, used for BMP085
            url += F("&value=");
            url += toString(UserVar[event->BaseVarIndex], ExtraTaskSettings.TaskDeviceValueDecimals[0]);
            url += ";";
            url += toString(UserVar[event->BaseVarIndex + 1], ExtraTaskSettings.TaskDeviceValueDecimals[1]);
            break;
          case SENSOR_TYPE_TEMP_HUM_BARO:                      // temp + hum + hum_stat + bar + bar_fore, used for BME280
            url += F("&value=");
            url += toString(UserVar[event->BaseVarIndex], ExtraTaskSettings.TaskDeviceValueDecimals[0]);
            url += ";";
            url += toString(UserVar[event->BaseVarIndex + 1], ExtraTaskSettings.TaskDeviceValueDecimals[1]);
            url += ";";
            url += toString(UserVar[event->BaseVarIndex + 2], ExtraTaskSettings.TaskDeviceValueDecimals[2]);
            break;
          case SENSOR_TYPE_SWITCH:
            url = F("/receiver.php?device=ip");
            url += F("&name=");
            url += ExtraTaskSettings.TaskDeviceName;
            url += F("&key=");
            url += SecuritySettings.ControllerPassword;
            url += F("&id=");
            url += event->idx;
            url += F("&type=");
            url += ExtraTaskSettings.TaskDeviceValueNames[0];
            url += F("&value=");
            if (UserVar[event->BaseVarIndex] == 0)
              url += "0.0";
            else
              url += "1.0";
            url += F("&gpio=");
            url += Settings.TaskDevicePin1[event->TaskIndex];
            url += F("&ip=");
            url += strip;
            break;

        }

        url.toCharArray(log, sizeof(log));
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
          line.toCharArray(log, sizeof(log));
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

