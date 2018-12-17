#ifdef USES_P018
//#######################################################################################################
//#################################### Plugin 018: GP2Y10 ###############################################
//#######################################################################################################

#define PLUGIN_018
#define PLUGIN_ID_018 18
#define PLUGIN_NAME_018 "Dust - Sharp GP2Y10"
#define PLUGIN_VALUENAME1_018 "Dust"

boolean Plugin_018_init = false;
byte Plugin_GP2Y10_LED_Pin = 0;

boolean Plugin_018(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_018;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_018);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_018));
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        event->String1 = formatGpioName_output(F("LED"));
        break;
      }

    case PLUGIN_INIT:
      {
        Plugin_018_init = true;
        pinMode(Settings.TaskDevicePin1[event->TaskIndex], OUTPUT);
        Plugin_GP2Y10_LED_Pin = Settings.TaskDevicePin1[event->TaskIndex];
        digitalWrite(Plugin_GP2Y10_LED_Pin, HIGH);
        success = true;
        break;
      }


    case PLUGIN_READ:
      {
        Plugin_GP2Y10_LED_Pin = Settings.TaskDevicePin1[event->TaskIndex];
        noInterrupts();
        byte x;
        int value;
        value = 0;
        for (x = 0; x < 25; x++)
        {
          digitalWrite(Plugin_GP2Y10_LED_Pin, LOW);
          delayMicroseconds(280);
          value = value + analogRead(A0);
          delayMicroseconds(40);
          digitalWrite(Plugin_GP2Y10_LED_Pin, HIGH);
          delayMicroseconds(9680);
        }
        interrupts();
        UserVar[event->BaseVarIndex] = (float)value;
        String log = F("GPY  : Dust value: ");
        log += value;
        addLog(LOG_LEVEL_INFO, log);
        success = true;
        break;
      }
  }
  return success;
}
#endif // USES_P018
