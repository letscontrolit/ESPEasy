#ifdef USES_C008
//#######################################################################################################
//########################### Controller Plugin 008: Generic HTTP #######################################
//#######################################################################################################

#define CPLUGIN_008
#define CPLUGIN_ID_008         8
#define CPLUGIN_NAME_008       "Generic HTTP"
#include <ArduinoJson.h>

boolean CPlugin_008(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_008;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesTemplate = true;
        Protocol[protocolCount].usesAccount = true;
        Protocol[protocolCount].usesPassword = true;
        Protocol[protocolCount].defaultPort = 80;
        Protocol[protocolCount].usesID = true;
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_008);
        break;
      }

    case CPLUGIN_PROTOCOL_TEMPLATE:
      {
        event->String1 = "";
        event->String2 = F("demo.php?name=%sysname%&task=%tskname%&valuename=%valname%&value=%value%");
        break;
      }

    case CPLUGIN_PROTOCOL_SEND:
      {
        byte valueCount = getValueCountFromSensorType(event->sensorType);
        for (byte x = 0; x < valueCount; x++)
        {
          bool isvalid;
          String formattedValue = formatUserVar(event, x, isvalid);
          if (isvalid)
            HTTPSend(event, x, formattedValue);
          if (valueCount > 1)
          {
            delayBackground(Settings.MessageDelay);
            // unsigned long timer = millis() + Settings.MessageDelay;
            // while (!timeOutReached(timer))
            //   backgroundtasks();
          }
        }
        break;
      }

  }
  return success;
}


//********************************************************************************
// Generic HTTP get request
//********************************************************************************
boolean HTTPSend(struct EventStruct *event, byte varIndex, const String& formattedValue)
{
  if (!WiFiConnected(100)) {
    return false;
  }
  ControllerSettingsStruct ControllerSettings;
  LoadControllerSettings(event->ControllerIndex, (byte*)&ControllerSettings, sizeof(ControllerSettings));

  String authHeader = "";
  if ((SecuritySettings.ControllerUser[event->ControllerIndex][0] != 0) && (SecuritySettings.ControllerPassword[event->ControllerIndex][0] != 0))
  {
    base64 encoder;
    String auth = SecuritySettings.ControllerUser[event->ControllerIndex];
    auth += ":";
    auth += SecuritySettings.ControllerPassword[event->ControllerIndex];
    authHeader = F("Authorization: Basic ");
    authHeader += encoder.encode(auth) + " \r\n";
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

  if (ExtraTaskSettings.TaskDeviceValueNames[0][0] == 0)
    PluginCall(PLUGIN_GET_DEVICEVALUENAMES, event, dummyString);

  String url = "/";
  url += ControllerSettings.Publish;
  parseControllerVariables(url, event, true);

  url.replace(F("%valname%"), URLEncode(ExtraTaskSettings.TaskDeviceValueNames[varIndex]));
  url.replace(F("%value%"), formattedValue);

  // url.toCharArray(log, 80);
  addLog(LOG_LEVEL_DEBUG_MORE, url);

  // This will send the request to the server
  client.print(String(F("GET ")) + url + F(" HTTP/1.1\r\n") +
               F("Host: ") + ControllerSettings.getHost() + F("\r\n") + authHeader +
               F("Connection: close\r\n\r\n"));

  unsigned long timer = millis() + 200;
  while (!client.available() && !timeOutReached(timer))
    yield();

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    // String line = client.readStringUntil('\n');
    String line;
    safeReadStringUntil(client, line, '\n');

    // line.toCharArray(log, 80);
    addLog(LOG_LEVEL_DEBUG_MORE, line);
    if (line.startsWith(F("HTTP/1.1 200 OK")))
    {
      // strcpy_P(log, PSTR("HTTP : Success!"));
      // addLog(LOG_LEVEL_DEBUG, log);
      addLog(LOG_LEVEL_DEBUG, F("HTTP : Success!"));
      // success = true;
    }
    delay(1);
  }
  // strcpy_P(log, PSTR("HTTP : closing connection"));
  // addLog(LOG_LEVEL_DEBUG, log);
  addLog(LOG_LEVEL_DEBUG, F("HTTP : closing connection"));

  client.flush();
  client.stop();

  return(true);
}
#endif
