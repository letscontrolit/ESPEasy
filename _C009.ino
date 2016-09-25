//#######################################################################################################
//########################### Controller Plugin 009: FHEM HTTP ##########################################
//#######################################################################################################

/*******************************************************************************
 * Modified version of "Domoticz HTTP CPLUGIN"
 * Copyright 2016 dev0 (https://forum.fhem.de/index.php?action=profile;u=7465)
 * Release notes:
 - v1.0
 - changed switch and dimmer setreading cmds
 - v1.01
 - added json content to http requests
 - v1.02
 - some optimizations as requested by mvdbro
 - fixed JSON TaskDeviceValueDecimals handling
 - ArduinoJson Library v5.6.4 required (as used by stable R120)
 - parse for HTTP errors 400, 401
 - moved on/off translation for SENSOR_TYPE_SWITCH/DIMMER to FHEM module
 /******************************************************************************/

#define CPLUGIN_009
#define CPLUGIN_ID_009         9
#define CPLUGIN_NAME_009       "FHEM HTTP"

boolean CPlugin_009(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_009;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesTemplate = false;
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

        if (ExtraTaskSettings.TaskDeviceValueNames[0][0] == 0)
          PluginCall(PLUGIN_GET_DEVICEVALUENAMES, event, dummyString);

        // We now create a URI for the request
        String url = F("/fhem?cmd=");

        // Create json root object
        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();
        root["module"] = "ESPEasy";
        root["version"] = "1.02";

        // Create nested objects
        JsonObject& data = root.createNestedObject("data");
        JsonObject& ESP = data.createNestedObject("ESP");
        ESP["name"] = Settings.Name;
        ESP["unit"] = Settings.Unit;
        ESP["version"] = Settings.Version;
        ESP["build"] = Settings.Build;
        ESP["sleep"] = Settings.deepSleep;

        // embed IP, important if there is NAT/PAT
        char ipStr[20];
        IPAddress ip = WiFi.localIP();
        sprintf(ipStr, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
        ESP["ip"] = ipStr;

        // Create nested SENSOR json object
        JsonObject& SENSOR = data.createNestedObject("SENSOR");
        byte valueCount = getValueCountFromSensorType(event->sensorType);
        char itemNames[valueCount][2];
        for (byte x = 0; x < valueCount; x++)
        {
          String s;
          url += F("setreading%20");
          url += Settings.Name;
          url += F("%20");
          url += ExtraTaskSettings.TaskDeviceValueNames[x];
          url += F("%20");

          // Each sensor value get an own object (0..n)
          sprintf(itemNames[x],"%d",x);
          JsonObject& val = SENSOR.createNestedObject(itemNames[x]);
          val["deviceName"] = ExtraTaskSettings.TaskDeviceName;
          val["valueName"]  = ExtraTaskSettings.TaskDeviceValueNames[x];
          val["type"]       = event->sensorType;

          if (event->sensorType == SENSOR_TYPE_LONG) {
            s = (unsigned long)UserVar[event->BaseVarIndex] + ((unsigned long)UserVar[event->BaseVarIndex + 1] << 16);
            url += s;
            val["value"] = s;

          } else { // All other sensor types
            s = toString(UserVar[event->BaseVarIndex + x], ExtraTaskSettings.TaskDeviceValueDecimals[x]);
            url += s;
            val["value"] = s;
          }

          // Split FHEM commands by ";"
          if (x < valueCount-1)
            url += F("%3B");
        }

        // Create json buffer
        char buffer[root.measureLength() +1];
        root.printTo(buffer, sizeof(buffer));
        // Push data to server
        FHEMHTTPsend(url, buffer);
        break;
      }
  }
  return success;
}


//********************************************************************************
// FHEM HTTP request
//********************************************************************************
boolean FHEMHTTPsend(String url, char* buffer)
{
  boolean success = false;

  String authHeader = "";
  if ((SecuritySettings.ControllerUser[0] != 0) && (SecuritySettings.ControllerPassword[0] != 0)) {
    base64 encoder;
    String auth = SecuritySettings.ControllerUser;
    auth += ":";
    auth += SecuritySettings.ControllerPassword;
    authHeader = "Authorization: Basic " + encoder.encode(auth) + " \r\n";
  }

  char log[80];
  url.toCharArray(log, 80);
  addLog(LOG_LEVEL_DEBUG_MORE, log);

  char host[20];
  sprintf_P(host, PSTR("%u.%u.%u.%u"), Settings.Controller_IP[0], Settings.Controller_IP[1], Settings.Controller_IP[2], Settings.Controller_IP[3]);

  sprintf_P(log, PSTR("%s%s using port %u"), "HTTP : connecting to ", host,Settings.ControllerPort);
  addLog(LOG_LEVEL_DEBUG, log);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, Settings.ControllerPort)) {
    connectionFailures++;
    strcpy_P(log, PSTR("HTTP : connection failed"));
    addLog(LOG_LEVEL_ERROR, log);
    return false;
  }

  statusLED(true);
  if (connectionFailures)
    connectionFailures--;

  // This will send the request to the server
  int len = strlen(buffer);
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
    String helper = line;
    line.toCharArray(log, 80);
    addLog(LOG_LEVEL_DEBUG_MORE, log);
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
    delay(1);
  }
  strcpy_P(log, PSTR("HTTP : closing connection"));
  addLog(LOG_LEVEL_DEBUG, log);
  client.flush();
  client.stop();
}
