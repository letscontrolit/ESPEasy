#ifdef USES_P043
//#######################################################################################################
//#################################### Plugin 043: Clock Output #########################################
//#######################################################################################################
#define PLUGIN_043
#define PLUGIN_ID_043         43
#define PLUGIN_NAME_043       "Output - Clock"
#define PLUGIN_VALUENAME1_043 "Output"
#define PLUGIN_043_MAX_SETTINGS 8

boolean Plugin_043(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_043;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_SWITCH;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_043);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_043));
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        event->String1 = formatGpioName_output(F("Clock Event"));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        String options[3];
        options[0] = "";
        options[1] = F("Off");
        options[2] = F("On");

        for (byte x = 0; x < PLUGIN_043_MAX_SETTINGS; x++)
        {
        	addFormTextBox(String(F("Day,Time ")) + (x + 1), String(F("p043_clock")) + (x), timeLong2String(ExtraTaskSettings.TaskDevicePluginConfigLong[x]), 32);
//          addHtml(F("<TR><TD>Day,Time "));
//          addHtml(x+1);
//          addHtml(F(":<TD><input type='text' name='plugin_043_clock"));
//          addHtml(x);
//          addHtml(F("' value='"));
//          addHtml(timeLong2String(ExtraTaskSettings.TaskDevicePluginConfigLong[x]));
//          addHtml("'>");

          addHtml(" ");
          byte choice = ExtraTaskSettings.TaskDevicePluginConfig[x];
          addSelector(String(F("p043_state")) + (x), 3, options, NULL, NULL, choice, false);
        }
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        for (byte x = 0; x < PLUGIN_043_MAX_SETTINGS; x++)
        {
          String argc = F("p043_clock");
          argc += x;
          String plugin1 = WebServer.arg(argc);
          ExtraTaskSettings.TaskDevicePluginConfigLong[x] = string2TimeLong(plugin1);

          argc = F("p043_state");
          argc += x;
          String plugin2 = WebServer.arg(argc);
          ExtraTaskSettings.TaskDevicePluginConfig[x] = plugin2.toInt();
        }
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        success = true;
        break;
      }

    case PLUGIN_CLOCK_IN:
      {
        LoadTaskSettings(event->TaskIndex);
        for (byte x = 0; x < PLUGIN_043_MAX_SETTINGS; x++)
        {
          unsigned long clockEvent = (unsigned long)minute() % 10 | (unsigned long)(minute() / 10) << 4 | (unsigned long)(hour() % 10) << 8 | (unsigned long)(hour() / 10) << 12 | (unsigned long)weekday() << 16;
          unsigned long clockSet = ExtraTaskSettings.TaskDevicePluginConfigLong[x];

          if (matchClockEvent(clockEvent,clockSet))
          {
            byte state = ExtraTaskSettings.TaskDevicePluginConfig[x];
            if (state != 0)
            {
              state--;
              pinMode(Settings.TaskDevicePin1[event->TaskIndex], OUTPUT);
              digitalWrite(Settings.TaskDevicePin1[event->TaskIndex], state);
              UserVar[event->BaseVarIndex] = state;
              String log = F("TCLK : State ");
              log += state;
              addLog(LOG_LEVEL_INFO, log);
              sendData(event);
            }
          }
        }
        break;
      }
  }
  return success;
}
#endif // USES_P043
