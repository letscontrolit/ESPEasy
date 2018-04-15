#ifdef USES_P021
//#######################################################################################################
//#################################### Plugin 021: Level Control ########################################
//#######################################################################################################

#define PLUGIN_021
#define PLUGIN_ID_021        21
#define PLUGIN_NAME_021       "Regulator - Level Control"
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
        Device[deviceCount].TimerOption = false;
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
        // char tmpString[128];

        addHtml(F("<TR><TD>Check Task:<TD>"));
        addTaskSelect("plugin_021_task", Settings.TaskDevicePluginConfig[event->TaskIndex][0]);

        LoadTaskSettings(Settings.TaskDevicePluginConfig[event->TaskIndex][0]); // we need to load the values from another task for selection!
        addHtml(F("<TR><TD>Check Value:<TD>"));
        addTaskValueSelect("plugin_021_value", Settings.TaskDevicePluginConfig[event->TaskIndex][1], Settings.TaskDevicePluginConfig[event->TaskIndex][0]);

      	addFormTextBox(F("Set Level"), F("plugin_021_setvalue"), String(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0]), 8);

      	addFormTextBox(F("Hysteresis"), F("plugin_021_hyst"), String(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1]), 8);

        LoadTaskSettings(event->TaskIndex); // we need to restore our original taskvalues!
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("plugin_021_task"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("plugin_021_value"));
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0] = getFormItemFloat(F("plugin_021_setvalue"));
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] = getFormItemFloat(F("plugin_021_hyst"));
        success = true;
        break;
      }

    case PLUGIN_SET_CONFIG:
      {
        String command = parseString(string, 1);
        if (command == F("setlevel"))
        {
          String value = parseString(string, 2);
          float result=0;
          Calculate(value.c_str(), &result);
          Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0] = result;
          SaveSettings();
          success = true;
        }
        break;
      }

    case PLUGIN_GET_CONFIG:
      {
        String command = parseString(string, 1);
        if (command == F("getlevel"))
        {
          string = Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0];
          success = true;
        }
        break;
      }

    case PLUGIN_INIT:
      {
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
          String log = F("LEVEL: State ");
          log += state;
          addLog(LOG_LEVEL_INFO, log);
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
#endif // USES_P021
