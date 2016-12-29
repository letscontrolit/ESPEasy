//#######################################################################################################
//#################################### Plugin 002: Analog ###############################################
//#######################################################################################################

#define PLUGIN_002
#define PLUGIN_ID_002         2
#define PLUGIN_NAME_002       "Analog input"
#define PLUGIN_VALUENAME1_002 "Analog1"
#define PLUGIN_VALUENAME2_002 "Analog2"
boolean Plugin_002(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_002;
        Device[deviceCount].Type = DEVICE_TYPE_DUAL;
        Device[deviceCount].VType = SENSOR_TYPE_DUAL;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 2;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_002);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_002));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_002));
        break;
      }
      
    case PLUGIN_READ:
      {
        int value;
        if (Settings.TaskDevicePin1[event->TaskIndex] == -1 && Settings.TaskDevicePin2[event->TaskIndex] == -1)
        {
          value = analogRead(A0);
          UserVar[event->BaseVarIndex] = (float)value;
          String log = F("ADC  : Analog value: ");
          log += value;
          addLog(LOG_LEVEL_INFO,log);
          UserVar[event->BaseVarIndex+1] = 0;
          success = true;
          break;
        }
        else
        {
          if(Settings.TaskDevicePin1[event->TaskIndex] > -1)
          {
            pinMode(Settings.TaskDevicePin1[event->TaskIndex], OUTPUT);
            digitalWrite(Settings.TaskDevicePin1[event->TaskIndex], 1);
            value = analogRead(A0);
            digitalWrite(Settings.TaskDevicePin1[event->TaskIndex], 0);
            UserVar[event->BaseVarIndex] = (float)value;
            String log = F("ADC  : Analog value1: ");
            log += value;
            addLog(LOG_LEVEL_INFO,log);
          }
          else
          {
            UserVar[event->BaseVarIndex] = 0;
          }
          
          if(Settings.TaskDevicePin2[event->TaskIndex] > -1)
          {
            pinMode(Settings.TaskDevicePin2[event->TaskIndex], OUTPUT);
            digitalWrite(Settings.TaskDevicePin2[event->TaskIndex], 1);
            value = analogRead(A0);
            digitalWrite(Settings.TaskDevicePin2[event->TaskIndex], 0);
            UserVar[event->BaseVarIndex+1] = (float)value;
            String log = F("ADC  : Analog value2: ");
            log += value;
            addLog(LOG_LEVEL_INFO,log);
          }
          else
          {
            UserVar[event->BaseVarIndex+1] = 0;
          }
          success = true;
          break;
        }
      }
  }
  return success;
}
