//#######################################################################################################
//########################### Controller Plugin 009: FHEM HTTP ##########################################
//#######################################################################################################

/*******************************************************************************
 * Copyright 2016-2017 dev0
 * Contact: https://forum.fhem.de/index.php?action=profile;u=7465
 *          https://github.com/ddtlabs/
 *
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
 - v1.03
 - changed http request from GET to POST (RFC conform)
 - removed obsolet http get url code
 - v1.04
 - added build options and node_type_id to JSON/device
 /******************************************************************************/

#define CPLUGIN_009
#define CPLUGIN_ID_009         9
#define CPLUGIN_NAME_009       "FHEM HTTP"
#include <ArduinoJson.h>

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
        Protocol[protocolCount].usesID = false;
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
        String url = F("/ESPEasy");

        // Create json root object
        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();
        root[F("module")] = "ESPEasy";
        root[F("version")] = "1.04";

        // Create nested objects
        JsonObject& data = root.createNestedObject("data");
        JsonObject& ESP = data.createNestedObject("ESP");
        ESP[F("name")] = Settings.Name;
        ESP[F("unit")] = Settings.Unit;
        ESP[F("version")] = Settings.Version;
        ESP[F("build")] = Settings.Build;
        ESP[F("build_notes")] = BUILD_NOTES;
        ESP[F("build_git")] = BUILD_GIT;
        ESP[F("node_type_id")] = NODE_TYPE_ID;
        ESP[F("sleep")] = Settings.deepSleep;

        // embed IP, important if there is NAT/PAT
        char ipStr[20];
        IPAddress ip = WiFi.localIP();
        sprintf_P(ipStr, PSTR("%u.%u.%u.%u"), ip[0], ip[1], ip[2], ip[3]);
        ESP[F("ip")] = ipStr;

        // Create nested SENSOR json object
        JsonObject& SENSOR = data.createNestedObject(PSTR("SENSOR"));
        byte valueCount = getValueCountFromSensorType(event->sensorType);
        char itemNames[valueCount][2];
        for (byte x = 0; x < valueCount; x++)
        {
          // Each sensor value get an own object (0..n)
          sprintf(itemNames[x],"%d",x);
          JsonObject& val = SENSOR.createNestedObject(itemNames[x]);
          val[F("deviceName")] = ExtraTaskSettings.TaskDeviceName;
          val[F("valueName")]  = ExtraTaskSettings.TaskDeviceValueNames[x];
          val[F("type")]       = event->sensorType;

          if (event->sensorType == SENSOR_TYPE_LONG) {
            val[F("value")] = (unsigned long)UserVar[event->BaseVarIndex] + ((unsigned long)UserVar[event->BaseVarIndex + 1] << 16);
          }
          else { // All other sensor types
            val[F("value")] = toString(UserVar[event->BaseVarIndex + x], ExtraTaskSettings.TaskDeviceValueDecimals[x]);
          }
        }

        // Create json buffer
        char buffer[root.measureLength() +1];
        root.printTo(buffer, sizeof(buffer));
        // Push data to server
        FHEMHTTPsend(url, buffer, event->ControllerIndex);
        break;
      }
  }
  return success;
}


//********************************************************************************
// FHEM HTTP request
//********************************************************************************
boolean FHEMHTTPsend(String url, char* buffer, byte index)
{
  ControllerSettingsStruct ControllerSettings;
  LoadControllerSettings(index, (byte*)&ControllerSettings, sizeof(ControllerSettings));

  boolean success = false;

  String authHeader = "";
  if ((SecuritySettings.ControllerUser[index][0] != 0) && (SecuritySettings.ControllerPassword[index][0] != 0)) {
    base64 encoder;
    String auth = SecuritySettings.ControllerUser[index];
    auth += ":";
    auth += SecuritySettings.ControllerPassword[index];
    authHeader = PSTR("Authorization: Basic ") + encoder.encode(auth) + " \r\n";
  }

  char log[80];
  url.toCharArray(log, 80);
  addLog(LOG_LEVEL_DEBUG_MORE, log);

  char host[20];
  sprintf_P(host, PSTR("%u.%u.%u.%u"), ControllerSettings.IP[0], ControllerSettings.IP[1], ControllerSettings.IP[2], ControllerSettings.IP[3]);

  sprintf_P(log, PSTR("%s%s using port %u"), "HTTP : connecting to ", host,ControllerSettings.Port);
  addLog(LOG_LEVEL_DEBUG, log);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, ControllerSettings.Port)) {
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
  client.print(String("POST ") + url + F(" HTTP/1.1\r\n") +
              F("Content-Length: ")+ len + F("\r\n") +
              F("Host: ") + host + F("\r\n") + authHeader +
              F("Connection: close\r\n\r\n")
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
    if (line.substring(0, 15) == F("HTTP/1.1 200 OK")) {
      strcpy_P(log, PSTR("HTTP : Success"));
      success = true;
    }
    else if (line.substring(0, 24) == F("HTTP/1.1 400 Bad Request")) {
      strcpy_P(log, PSTR("HTTP : Unauthorized"));
    }
    else if (line.substring(0, 25) == F("HTTP/1.1 401 Unauthorized")) {
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
