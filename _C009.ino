//#######################################################################################################
//########################### Controller Plugin 009: FHEM HTTP ##########################################
//#######################################################################################################

/*******************************************************************************
 * Modified version of "Domoticz HTTP CPLUGIN"
 * Copyright 2016 dev0 (https://forum.fhem.de/index.php?action=profile;u=7465)
 * Release notes:
 - changed switch and dimmer setreading cmds
 - added json content to http requests
 /******************************************************************************/

#define CPLUGIN_009
#define CPLUGIN_ID_009         9
#define CPLUGIN_NAME_009       "FHEM"

boolean CPlugin_009(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_009;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesAccount = true;
        Protocol[protocolCount].usesPassword = true;
        Protocol[protocolCount].defaultPort = 8383;
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_009);
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
        String url = F("/fhem?cmd=");

        // Create json root object
        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();
        root["module"] = "ESPEasy";
        root["version"] = "1.0";

        // Create json objects
        JsonObject& data = root.createNestedObject("data");
        JsonObject& ESP = data.createNestedObject("ESP");

        ESP["name"] = Settings.Name;
        ESP["unit"] = Settings.Unit;
        ESP["version"] = Settings.Version;
        ESP["build"] = Settings.Build;
        ESP["sleep"] = Settings.deepSleep;

        char ipStr[20];
        IPAddress ip = WiFi.localIP();
        sprintf(ipStr, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
        ESP["ip"] = ipStr;

        // Create other json objects
        JsonObject& SENSOR = data.createNestedObject("SENSOR");
        JsonObject& v0 = SENSOR.createNestedObject("v0");
        JsonObject& v1 = SENSOR.createNestedObject("v1");
        JsonObject& v2 = SENSOR.createNestedObject("v2");

        switch (event->sensorType)
        {
          case SENSOR_TYPE_SINGLE:                      // single value sensor, used for Dallas, BH1750, etc
            url += F("setreading%20");
            url += Settings.Name;
            url += F("%20");
            url += ExtraTaskSettings.TaskDeviceValueNames[0];
            url += F("%20");
            url += toString(UserVar[event->BaseVarIndex],ExtraTaskSettings.TaskDeviceValueDecimals[0]);
            v0["deviceName"] = ExtraTaskSettings.TaskDeviceName;
            v0["valueName"]  = ExtraTaskSettings.TaskDeviceValueNames[0];
            v0["value"]      = UserVar[event->BaseVarIndex],ExtraTaskSettings.TaskDeviceValueDecimals[0];
            v0["type"]       = event->sensorType;
            break;
          
          case SENSOR_TYPE_LONG:                      // single LONG value, stored in two floats (rfid tags)
            url += F("setreading%20");
            url += Settings.Name;
            url += F("%20");
            url += ExtraTaskSettings.TaskDeviceValueNames[0];
            url += F("%20");
            url += (unsigned long)UserVar[event->BaseVarIndex] + ((unsigned long)UserVar[event->BaseVarIndex + 1] << 16);
            v0["deviceName"] = ExtraTaskSettings.TaskDeviceName;
            v0["valueName"]  = ExtraTaskSettings.TaskDeviceValueNames[0];
            v0["value"]      = (unsigned long)UserVar[event->BaseVarIndex] + ((unsigned long)UserVar[event->BaseVarIndex + 1] << 16);
            v0["type"]       = event->sensorType;
            break;

          case SENSOR_TYPE_TEMP_HUM:                      // temp + hum + hum_stat, used for DHT11
            // setreading #1
            url += F("setreading%20");
            url += Settings.Name;
            url += F("%20");
            url += ExtraTaskSettings.TaskDeviceValueNames[0];
            url += F("%20");
            url += toString(UserVar[event->BaseVarIndex],ExtraTaskSettings.TaskDeviceValueDecimals[0]);
            v0["deviceName"] = ExtraTaskSettings.TaskDeviceName;
            v0["valueName"]  = ExtraTaskSettings.TaskDeviceValueNames[0];
            v0["value"]      = UserVar[event->BaseVarIndex],ExtraTaskSettings.TaskDeviceValueDecimals[0];
            v0["type"]       = event->sensorType;

            // setreading #2
            url += F("%3B");
            url += F("setreading%20");
            url += Settings.Name;
            url += F("%20");
            url += ExtraTaskSettings.TaskDeviceValueNames[1];
            url += F("%20");
            url += toString(UserVar[event->BaseVarIndex + 1],ExtraTaskSettings.TaskDeviceValueDecimals[1]);
            v1["deviceName"] = ExtraTaskSettings.TaskDeviceName;
            v1["valueName"]  = ExtraTaskSettings.TaskDeviceValueNames[1];
            v1["value"]      = UserVar[event->BaseVarIndex +1],ExtraTaskSettings.TaskDeviceValueDecimals[1];
            v1["type"]       = event->sensorType;
            break;
            
          case SENSOR_TYPE_TEMP_BARO:                      // temp + hum + hum_stat + bar + bar_fore, used for BMP085
            // setreading #1
            url += F("setreading%20");
            url += Settings.Name;
            url += F("%20");
            url += ExtraTaskSettings.TaskDeviceValueNames[0];
            url += F("%20");
            url += toString(UserVar[event->BaseVarIndex],ExtraTaskSettings.TaskDeviceValueDecimals[0]);
            v0["deviceName"] = ExtraTaskSettings.TaskDeviceName;
            v0["valueName"]  = ExtraTaskSettings.TaskDeviceValueNames[0];
            v0["value"]      = UserVar[event->BaseVarIndex],ExtraTaskSettings.TaskDeviceValueDecimals[0];
            v0["type"]       = event->sensorType;

            // setreading #2
            url += F("%3B");
            url += F("setreading%20");
            url += Settings.Name;
            url += F("%20");
            url += ExtraTaskSettings.TaskDeviceValueNames[1];
            url += F("%20");
            url += toString(UserVar[event->BaseVarIndex + 1],ExtraTaskSettings.TaskDeviceValueDecimals[1]);
            v1["deviceName"] = ExtraTaskSettings.TaskDeviceName;
            v1["valueName"]  = ExtraTaskSettings.TaskDeviceValueNames[1];
            v1["value"]      = UserVar[event->BaseVarIndex +1],ExtraTaskSettings.TaskDeviceValueDecimals[1];
            v1["type"]       = event->sensorType;
            break;

          case SENSOR_TYPE_DUAL:
            // setreading #1
            url += F("setreading%20");
            url += Settings.Name;
            url += F("%20");
            url += ExtraTaskSettings.TaskDeviceValueNames[0];
            url += F("%20");
            url += toString(UserVar[event->BaseVarIndex],ExtraTaskSettings.TaskDeviceValueDecimals[0]);
            v0["deviceName"] = ExtraTaskSettings.TaskDeviceName;
            v0["valueName"]  = ExtraTaskSettings.TaskDeviceValueNames[0];
            v0["value"]      = UserVar[event->BaseVarIndex],ExtraTaskSettings.TaskDeviceValueDecimals[0];
            v0["type"]       = event->sensorType;

            // setreading #2
            url += F("%3B");
            url += F("setreading%20");
            url += Settings.Name;
            url += F("%20");
            url += ExtraTaskSettings.TaskDeviceValueNames[1];
            url += F("%20");
            url += toString(UserVar[event->BaseVarIndex + 1],ExtraTaskSettings.TaskDeviceValueDecimals[1]);
            v1["deviceName"] = ExtraTaskSettings.TaskDeviceName;
            v1["valueName"]  = ExtraTaskSettings.TaskDeviceValueNames[1];
            v1["value"]      = UserVar[event->BaseVarIndex +1],ExtraTaskSettings.TaskDeviceValueDecimals[1];
            v1["type"]       = event->sensorType;
            break;

          case SENSOR_TYPE_TEMP_HUM_BARO:                      // temp + hum + hum_stat + bar + bar_fore, used for BME280
            // setreading #1
            url += F("setreading%20");
            url += Settings.Name;
            url += F("%20");
            url += ExtraTaskSettings.TaskDeviceValueNames[0];
            url += F("%20");
            url += toString(UserVar[event->BaseVarIndex],ExtraTaskSettings.TaskDeviceValueDecimals[0]);
            v0["deviceName"] = ExtraTaskSettings.TaskDeviceName;
            v0["valueName"]  = ExtraTaskSettings.TaskDeviceValueNames[0];
            v0["value"]      = UserVar[event->BaseVarIndex],ExtraTaskSettings.TaskDeviceValueDecimals[0];
            v0["type"]       = event->sensorType;

            // setreading #2
            url += F("%3B");
            url += F("setreading%20");
            url += Settings.Name;
            url += F("%20");
            url += ExtraTaskSettings.TaskDeviceValueNames[1];
            url += F("%20");
            url += toString(UserVar[event->BaseVarIndex + 1],ExtraTaskSettings.TaskDeviceValueDecimals[1]);
            v1["deviceName"] = ExtraTaskSettings.TaskDeviceName;
            v1["valueName"]  = ExtraTaskSettings.TaskDeviceValueNames[1];
            v1["value"]      = UserVar[event->BaseVarIndex +1],ExtraTaskSettings.TaskDeviceValueDecimals[1];
            v1["type"]       = event->sensorType;

            // setreading #3
            url += F("%3B");
            url += "setreading%20";
            url += Settings.Name;
            url += F("%20");
            url += ExtraTaskSettings.TaskDeviceValueNames[2];
            url += F("%20");
            url += UserVar[event->BaseVarIndex + 2];
            url += toString(UserVar[event->BaseVarIndex + 2],ExtraTaskSettings.TaskDeviceValueDecimals[2]);
            v2["deviceName"] = ExtraTaskSettings.TaskDeviceName;
            v2["valueName"]  = ExtraTaskSettings.TaskDeviceValueNames[2];
            v2["value"]      = UserVar[event->BaseVarIndex +2],ExtraTaskSettings.TaskDeviceValueDecimals[2];
            v2["type"]       = event->sensorType;
            break;

          case SENSOR_TYPE_TRIPLE:
            // setreading #1
            url += F("setreading%20");
            url += Settings.Name;
            url += F("%20");
            url += ExtraTaskSettings.TaskDeviceValueNames[0];
            url += F("%20");
            url += toString(UserVar[event->BaseVarIndex],ExtraTaskSettings.TaskDeviceValueDecimals[0]);
            v0["deviceName"] = ExtraTaskSettings.TaskDeviceName;
            v0["valueName"]  = ExtraTaskSettings.TaskDeviceValueNames[0];
            v0["value"]      = UserVar[event->BaseVarIndex],ExtraTaskSettings.TaskDeviceValueDecimals[0];
            v0["type"]       = event->sensorType;

            // setreading #2
            url += F("%3B");
            url += F("setreading%20");
            url += Settings.Name;
            url += F("%20");
            url += ExtraTaskSettings.TaskDeviceValueNames[1];
            url += F("%20");
            url += toString(UserVar[event->BaseVarIndex + 1],ExtraTaskSettings.TaskDeviceValueDecimals[1]);
            v1["deviceName"] = ExtraTaskSettings.TaskDeviceName;
            v1["valueName"]  = ExtraTaskSettings.TaskDeviceValueNames[1];
            v1["value"]      = UserVar[event->BaseVarIndex +1],ExtraTaskSettings.TaskDeviceValueDecimals[1];
            v1["type"]       = event->sensorType;

            // setreading #3
            url += F("%3B");
            url += "setreading%20";
            url += Settings.Name;
            url += F("%20");
            url += ExtraTaskSettings.TaskDeviceValueNames[2];
            url += F("%20");
            url += UserVar[event->BaseVarIndex + 2];
            url += toString(UserVar[event->BaseVarIndex + 2],ExtraTaskSettings.TaskDeviceValueDecimals[2]);
            v2["deviceName"] = ExtraTaskSettings.TaskDeviceName;
            v2["valueName"]  = ExtraTaskSettings.TaskDeviceValueNames[2];
            v2["value"]      = UserVar[event->BaseVarIndex +2],ExtraTaskSettings.TaskDeviceValueDecimals[2];
            v2["type"]       = event->sensorType;
            break;

          case SENSOR_TYPE_SWITCH:
            url += F("setreading%20");
            url += Settings.Name;
            url += F("%20");
            url += ExtraTaskSettings.TaskDeviceValueNames[0];
            url += F("%20");
            v0["deviceName"] = ExtraTaskSettings.TaskDeviceName;
            v0["valueName"]  = ExtraTaskSettings.TaskDeviceValueNames[0];
            v0["type"]       = event->sensorType;
            if (UserVar[event->BaseVarIndex] == 0) {
              url += "off";
              v0["value"] = "off";
            } else {
              url += "on";
              v0["value"] = "on";
            }
            break;

          case SENSOR_TYPE_DIMMER:
            url += F("setreading%20");
            url += Settings.Name;
            url += F("%20");
            url += ExtraTaskSettings.TaskDeviceValueNames[0];
            url += F("%20");
            v0["deviceName"] = ExtraTaskSettings.TaskDeviceName;
            v0["valueName"]  = ExtraTaskSettings.TaskDeviceValueNames[0];
            v0["type"]       = event->sensorType;
            if (UserVar[event->BaseVarIndex] == 0) {
              url += "off";
              v0["value"] = "off";
            } else {
              url += UserVar[event->BaseVarIndex];
              v0["value"] = UserVar[event->BaseVarIndex];
            }
            break;

        }

        url.toCharArray(log, 80);
        addLog(LOG_LEVEL_DEBUG_MORE, log);

        char buffer[2048];
        root.printTo(buffer, sizeof(buffer));
        int len = root.measureLength();

        // This will send the request to the server
        client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                     "Content-Length: "+ len + "\r\n" +
                     "Host: " + host + "\r\n" + authHeader +
                     "Connection: close\r\n\r\n"
                     + buffer);

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
