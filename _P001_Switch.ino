//#######################################################################################################
//#################################### Plugin 001: Input Switch #########################################
//#######################################################################################################

#define PLUGIN_001
#define PLUGIN_ID_001        1

boolean Plugin_001(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static byte switchstate[TASKS_MAX];

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_001;
        strcpy(Device[deviceCount].Name, "Switch input");
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = 10;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = true;
        Device[deviceCount].InverseLogicOption = true;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 1;
        strcpy(Device[deviceCount].ValueNames[0], "Switch");
        break;
      }

    case PLUGIN_INIT:
      {
        if (Settings.TaskDevicePin1PullUp[event->TaskIndex])
        {
          Serial.print(F("INIT : InputPullup "));
          pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT_PULLUP);
        }
        else
        {
          Serial.print(F("INIT : Input "));
          pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT);
        }
        switchstate[event->TaskIndex] = digitalRead(Settings.TaskDevicePin1[event->TaskIndex]);
        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        byte state = digitalRead(Settings.TaskDevicePin1[event->TaskIndex]);
        if (state != switchstate[event->TaskIndex])
        {
          Serial.print(F("SW   : State "));
          Serial.println(state);
          switchstate[event->TaskIndex] = state;
          if (Settings.TaskDevicePin1Inversed[event->TaskIndex])
            state = !state;
          UserVar[event->BaseVarIndex] = state;
          sendData(event->TaskIndex, 10, Settings.TaskDeviceID[event->TaskIndex], event->BaseVarIndex);
        }
        success = true;
        break;
      }

  }
  return success;
}
