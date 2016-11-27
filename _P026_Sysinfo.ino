//#######################################################################################################
//#################################### Plugin 026: Analog ###############################################
//#######################################################################################################

#define PLUGIN_026
#define PLUGIN_ID_026         26
#define PLUGIN_NAME_026       "System Info"
#define PLUGIN_VALUENAME1_026 ""

boolean Plugin_026(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_026;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].FormulaOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_026);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_026));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        String options[5];
        options[0] = F("Uptime");
        options[1] = F("Free RAM");
        options[2] = F("Wifi RSSI");
        options[3] = F("Input VCC");
        options[4] = F("System load");
        int optionValues[5];
        optionValues[0] = 0;
        optionValues[1] = 1;
        optionValues[2] = 2;
        optionValues[3] = 3;
        optionValues[4] = 4;
        string += F("<TR><TD>Indicator:<TD><select name='plugin_026'>");
        for (byte x = 0; x < 5; x++)
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
        String plugin1 = WebServer.arg("plugin_026");
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin1.toInt();
        success = true;
        break;
      }
      
    case PLUGIN_READ:
      {
        float value = 0;
        switch(Settings.TaskDevicePluginConfig[event->TaskIndex][0])
        {
          case 0:
          {
            value = (wdcounter /2);
            break;
          }
          case 1:
          {
            value = ESP.getFreeHeap();
            break;
          }
          case 2:
          {
            value = WiFi.RSSI();
            break;
          }
          case 3:
          {
#if FEATURE_ADC_VCC
            value = vcc;
#else
            value = -1.0;
#endif
            break;
          }
          case 4:
          {
            value = (100 - (100 * loopCounterLast / loopCounterMax));
            break;
          }
        }
        UserVar[event->BaseVarIndex] = value;
        String log = F("SYS  : ");
        log += value;
        addLog(LOG_LEVEL_INFO,log);
        success = true;
        break;
      }
  }
  return success;
}
