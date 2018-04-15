#ifdef USES_P033
//#######################################################################################################
//#################################### Plugin 033: Dummy ################################################
//#######################################################################################################

#define PLUGIN_033
#define PLUGIN_ID_033         33
#define PLUGIN_NAME_033       "Generic - Dummy Device"
#define PLUGIN_VALUENAME1_033 "Dummy"
boolean Plugin_033(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_033;
        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].DecimalsOnly = true;
        Device[deviceCount].ValueCount = 4;
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

        addFormSelector(F("Simulate Data Type"), F("plugin_033_sensortype"), 11, options, optionValues, choice );

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("plugin_033_sensortype"));
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        event->sensorType = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        for (byte x=0; x<4;x++)
        {
          String log = F("Dummy: value ");
          log += x+1;
          log += F(": ");
          log += UserVar[event->BaseVarIndex+x];
          addLog(LOG_LEVEL_INFO,log);
        }
        success = true;
        break;
      }
  }
  return success;
}
#endif // USES_P033
