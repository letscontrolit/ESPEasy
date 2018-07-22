#ifdef USES_C010
//#######################################################################################################
//########################### Controller Plugin 010: Generic UDP ########################################
//#######################################################################################################

#define CPLUGIN_010
#define CPLUGIN_ID_010         10
#define CPLUGIN_NAME_010       "Generic UDP"

boolean CPlugin_010(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_010;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesTemplate = true;
        Protocol[protocolCount].usesAccount = false;
        Protocol[protocolCount].usesPassword = false;
        Protocol[protocolCount].defaultPort = 514;
        Protocol[protocolCount].usesID = false;
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_010);
        break;
      }

    case CPLUGIN_PROTOCOL_TEMPLATE:
      {
        event->String1 = "";
        event->String2 = F("%sysname%_%tskname%_%valname%=%value%");
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
            C010_Send(event, x, formattedValue);
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
// Generic UDP message
//********************************************************************************
void C010_Send(struct EventStruct *event, byte varIndex, const String& formattedValue)
{
  ControllerSettingsStruct ControllerSettings;
  LoadControllerSettings(event->ControllerIndex, (byte*)&ControllerSettings, sizeof(ControllerSettings));

  // boolean success = false;
  addLog(LOG_LEVEL_DEBUG, String(F("UDP  : sending to ")) + ControllerSettings.getHostPortString());
  statusLED(true);

  if (ExtraTaskSettings.TaskDeviceValueNames[0][0] == 0)
    PluginCall(PLUGIN_GET_DEVICEVALUENAMES, event, dummyString);

  String msg = "";
  msg += ControllerSettings.Publish;
  parseControllerVariables(msg, event, false);
  msg.replace(F("%valname%"), ExtraTaskSettings.TaskDeviceValueNames[varIndex]);
  msg.replace(F("%value%"), formattedValue);

  if (wifiStatus == ESPEASY_WIFI_SERVICES_INITIALIZED) {
    ControllerSettings.beginPacket(portUDP);
    portUDP.write((uint8_t*)msg.c_str(),msg.length());
    portUDP.endPacket();
  }

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) {
    char log[80];
    msg.toCharArray(log, 80);
    addLog(LOG_LEVEL_DEBUG_MORE, log);
  }
}
#endif
