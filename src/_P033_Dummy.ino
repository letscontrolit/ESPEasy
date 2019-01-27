#ifdef USES_P033
//#######################################################################################################
//#################################### Plugin 033: Dummy ################################################
//#######################################################################################################

#define PLUGIN_033
#define PLUGIN_ID_033         33
#define PLUGIN_NAME_033       "Generic - Dummy Device"
#define PLUGIN_VALUENAME1_033 "Dummy"
#define P33_Nlines 4
#define P33_Nchars 8

boolean Plugin_033(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_033;
        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
        Device[deviceCount].VType = SENSOR_TYPE_QUAD;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].DecimalsOnly = true;
        Device[deviceCount].ValueCount = P33_Nlines;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_033);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_033));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        String options[11];
        options[0] = F("SENSOR_TYPE_SINGLE");
        options[1] = F("SENSOR_TYPE_TEMP_HUM");
        options[2] = F("SENSOR_TYPE_TEMP_BARO");
        options[3] = F("SENSOR_TYPE_TEMP_HUM_BARO");
        options[4] = F("SENSOR_TYPE_DUAL");
        options[5] = F("SENSOR_TYPE_TRIPLE");
        options[6] = F("SENSOR_TYPE_QUAD");
        options[7] = F("SENSOR_TYPE_SWITCH");
        options[8] = F("SENSOR_TYPE_DIMMER");
        options[9] = F("SENSOR_TYPE_LONG");
        options[10] = F("SENSOR_TYPE_WIND");
        int optionValues[11];
        optionValues[0] = SENSOR_TYPE_SINGLE;
        optionValues[1] = SENSOR_TYPE_TEMP_HUM;
        optionValues[2] = SENSOR_TYPE_TEMP_BARO;
        optionValues[3] = SENSOR_TYPE_TEMP_HUM_BARO;
        optionValues[4] = SENSOR_TYPE_DUAL;
        optionValues[5] = SENSOR_TYPE_TRIPLE;
        optionValues[6] = SENSOR_TYPE_QUAD;
        optionValues[7] = SENSOR_TYPE_SWITCH;
        optionValues[8] = SENSOR_TYPE_DIMMER;
        optionValues[9] = SENSOR_TYPE_LONG;
        optionValues[10] = SENSOR_TYPE_WIND;

        addFormSelector(F("Simulate Data Type"), F("p033_sensortype"), 11, options, optionValues, choice );
        boolean keepValue = Settings.TaskDevicePluginConfig[event->TaskIndex][1];

        addRowLabel(F("Save on read or publish")); // Keep TaskValueSet on reboot
        addCheckBox(F("p033_keepvalue"), keepValue);

        char deviceTemplate[P33_Nlines][P33_Nchars];
        LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));
        for (byte varNr = 0; varNr < P33_Nlines; varNr++)
        {
          addFormTextBox(String(F("Persistent value ")) + (varNr + 1), String(F("p033_template")) + (varNr + 1), deviceTemplate[varNr], P33_Nchars);
        }
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("p033_sensortype"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = isFormItemChecked(F("p033_keepvalue"));

        char deviceTemplate[P33_Nlines][P33_Nchars];
        String error;
        for (byte varNr = 0; varNr < P33_Nlines; varNr++)
        {
          String argName = F("p033_template");
          argName += varNr + 1;
          if (!safe_strncpy(deviceTemplate[varNr], WebServer.arg(argName), P33_Nchars)) {
            error += getCustomTaskSettingsError(varNr);
          }
        }
        if (error.length() > 0) {
          addHtmlError(error);
        }
        SaveCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        char deviceTemplate[P33_Nlines][P33_Nchars];
        LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));
        for (byte varNr = 0; varNr < P33_Nlines; varNr++)
        {
          float temp;
          temp=atof(deviceTemplate[varNr]);
          UserVar[event->BaseVarIndex + varNr] = temp;
        }

        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        event->sensorType = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        char deviceTemplate[P33_Nlines][P33_Nchars];
        LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));
        boolean valueChanged = false;

        for (byte varNr = 0; varNr < P33_Nlines; varNr++)
        {
          float ramValue = UserVar[event->BaseVarIndex + varNr];
          String log = F("Dummy: value ");
          log += varNr + 1;
          log += F(": ");
          log += ramValue;

          float persistentValue;
          persistentValue = atof(deviceTemplate[varNr]);

          if (ramValue != persistentValue){
            log += F(" (changed)");
            valueChanged = true;
          }
          addLog(LOG_LEVEL_INFO,log);
        }

        byte keepValue = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        if (valueChanged && keepValue){
          for (byte varNr = 0; varNr < P33_Nlines; varNr++)
          {
            float value=UserVar[event->BaseVarIndex + varNr];
              String tmp = F("");
            tmp += value;
            safe_strncpy(deviceTemplate[varNr], tmp, P33_Nchars);
          }
          SaveCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));

          String log = F("persistent values saved for task ");
          log += (event->TaskIndex + 1);

        }

        success = true;
        break;
      }

  }
  return success;
}
#endif // USES_P033
