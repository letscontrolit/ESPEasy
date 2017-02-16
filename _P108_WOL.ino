#ifdef PLUGIN_BUILD_DEV

//#######################################################################################################
//#################################### Plugin 108: WOL receiver #########################################
//#######################################################################################################

// This plugin receives Wake On Lan (WOL) messages that can be used to power on a connected device.
// Note that the ESP itself has no WOL feature. It must be active to receive this message.
// So you can't wake an ESP from deepsleep using WOL because Wifi is down during deepsleep.

#define PLUGIN_108
#define PLUGIN_ID_108         108
#define PLUGIN_NAME_108       "WOL Receiver [DEVELOPMENT]"
#define PLUGIN_VALUENAME1_108 "WOL"

WiFiUDP *WOL;

boolean Plugin_108(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_108;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].Custom = true;
        Device[deviceCount].TimerOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_108);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_108));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        string += F("<TR><TD>Power control pin:<TD>");
        addPinSelect(false, string, "taskdevicepin1", Settings.TaskDevicePin1[event->TaskIndex]);

        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        String options[2];
        options[0] = F("Active Low");
        options[1] = F("Active High");
        int optionValues[2];
        optionValues[0] = 0;
        optionValues[1] = 1;
        string += F("<TR><TD>Output state:<TD><select name='plugin_108_state'>");
        for (byte x = 0; x < 2; x++)
        {
          string += F("<option value='");
          string += optionValues[x];
          string += "'";
          if (choice == optionValues[x])
            string += F(" selected");
          string += ">";
          string += options[x];
          string += F("</option>");
        }
        string += F("</select>");

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin1 = WebServer.arg("plugin_108_state");
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin1.toInt();
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        if (!WOL)
        {
          WOL = new WiFiUDP;
          WOL->begin(7);
        }
        if (Settings.TaskDevicePin1[event->TaskIndex] != -1)
          {
            pinMode(Settings.TaskDevicePin1[event->TaskIndex], OUTPUT);
            digitalWrite(Settings.TaskDevicePin1[event->TaskIndex], !Settings.TaskDevicePluginConfig[event->TaskIndex][0]);
          }
      }
      success = true;
      break;

    case PLUGIN_ONCE_A_SECOND:
      {
        int packetSize = WOL->parsePacket();
        if (packetSize)
        {
          statusLED(true);
          char packetBuffer[128];
          int len = WOL->read(packetBuffer, 128);
          byte match = 0;
          uint8_t mac[] = {0, 0, 0, 0, 0, 0};
          uint8_t* macread = WiFi.macAddress(mac);
          for (byte x = 0; x < 10; x++)
            if (packetBuffer[x + 6] == macread[x])
              match++;
          if (match == 6)
          {
            String log = F("WOL  : Magic packet received!");
            addLog(LOG_LEVEL_INFO, log);
            if (Settings.TaskDevicePin1[event->TaskIndex] != -1)
            {
              String log = F("WOL  : Power cycle using pin: ");
              log += Settings.TaskDevicePin1[event->TaskIndex];
              addLog(LOG_LEVEL_INFO, log);
              digitalWrite(Settings.TaskDevicePin1[event->TaskIndex], Settings.TaskDevicePluginConfig[event->TaskIndex][0]);
              delay(500);
              digitalWrite(Settings.TaskDevicePin1[event->TaskIndex], !Settings.TaskDevicePluginConfig[event->TaskIndex][0]);
            }
          }
        }
        success = true;
        break;
      }
  }
  return success;
}

#endif
