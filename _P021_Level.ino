//#######################################################################################################
//#################################### Plugin 021: Level Control ########################################
//#######################################################################################################

#define PLUGIN_021
#define PLUGIN_ID_021        21
#define PLUGIN_NAME_021       "Level Control"
#define PLUGIN_VALUENAME1_021 "Output"

boolean Plugin_021(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static byte switchstate[TASKS_MAX];

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_021;
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
        string = F(PLUGIN_NAME_021);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_021));
        break;
      }
      
    case PLUGIN_WEBFORM_LOAD:
      {
        char tmpString[128];

        string += F("<TR><TD>Check Task:<TD>");
        addTaskSelect(string, "plugin_021_task", Settings.TaskDevicePluginConfig[event->TaskIndex][0]);

        LoadTaskSettings(Settings.TaskDevicePluginConfig[event->TaskIndex][0]);
        string += F("<TR><TD>Check Value:<TD>");
        addTaskValueSelect(string, "plugin_021_value", Settings.TaskDevicePluginConfig[event->TaskIndex][1], Settings.TaskDevicePluginConfig[event->TaskIndex][0]);

        // bug with %f in sprintf, this is a workaround:
        //sprintf(tmpString, "<TR><TD>Set Value:<TD><input type='text' name='plugin_021_setvalue' value='%f'>", Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0]);
        //string += tmpString;
        //sprintf(tmpString, "<TR><TD>Hysteresis:<TD><input type='text' name='plugin_021_hyst' value='%f'>", Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1]);
        //string += tmpString;
        string += F("<TR><TD>Set Value:<TD><input type='text' name='plugin_021_setvalue' value='");
        string += Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0];
        string += F("'>");
        string += F("<TR><TD>Hysteresis:<TD><input type='text' name='plugin_021_hyst' value='");
        string += Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1];
        string += F("'>");

        LoadTaskSettings(event->TaskIndex);
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin1 = WebServer.arg("plugin_021_task");
        String plugin2 = WebServer.arg("plugin_021_value");
        String plugin3 = WebServer.arg("plugin_021_setvalue");
        String plugin4 = WebServer.arg("plugin_021_hyst");
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin1.toInt();
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = plugin2.toInt();
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0] = plugin3.toFloat();
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] = plugin4.toFloat();
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        Serial.print(F("INIT : Output "));
        Serial.println(Settings.TaskDevicePin1[event->TaskIndex]);
        pinMode(Settings.TaskDevicePin1[event->TaskIndex], OUTPUT);
        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        // we're checking a var from another task, so calculate that basevar
        byte TaskIndex = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        byte BaseVarIndex = TaskIndex * VARS_PER_TASK + Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        float value = UserVar[BaseVarIndex];
        byte state = switchstate[event->TaskIndex];
        // compare with threshold value
        float valueLowThreshold = Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0] - (Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] / 2);
        float valueHighThreshold = Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0] + (Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] / 2);
        if (value <= valueLowThreshold)
          state = 1;
        if (value >= valueHighThreshold)
          state = 0;
        if (state != switchstate[event->TaskIndex])
        {
          Serial.print(F("Out : State "));
          Serial.println(state);
          switchstate[event->TaskIndex] = state;
          digitalWrite(Settings.TaskDevicePin1[event->TaskIndex],state);
          UserVar[event->BaseVarIndex] = state;
          sendData(event);
        }

        success = true;
        break;
      }

  }
  return success;
}

