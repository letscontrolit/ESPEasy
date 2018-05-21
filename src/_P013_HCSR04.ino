#ifdef USES_P013
//#######################################################################################################
//############################### Plugin 013: HC-SR04, RCW-0001, etc. ###################################
//#######################################################################################################

#define PLUGIN_013
#define PLUGIN_ID_013        13
#define PLUGIN_NAME_013       "Distance - HC-SR04, RCW-0001, etc."
#define PLUGIN_VALUENAME1_013 "Distance"

#include <NewPing.h>

boolean Plugin_013_init = false;
byte Plugin_013_TRIG_Pin = 0;
byte Plugin_013_IRQ_Pin = 0;
NewPing *sonar = NULL;

boolean Plugin_013(byte function, struct EventStruct *event, String& string)
{
  static byte switchstate[TASKS_MAX];
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_013;
        Device[deviceCount].Type = DEVICE_TYPE_DUAL;
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
        string = F(PLUGIN_NAME_013);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_013));
        break;
      }


    case PLUGIN_WEBFORM_LOAD:
      {
        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        String options[2];
        options[0] = F("Value");
        options[1] = F("State");
        int optionValues[2] = { 1, 2 };
        addFormSelector(F("Mode"), F("plugin_013_mode"), 2, options, optionValues, choice);

        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == 2)
        {
        	addFormNumericBox(F("Threshold"), F("plugin_013_threshold"), Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
        }
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("plugin_013_mode"));
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == 2)
        {
          Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("plugin_013_threshold"));
        }
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        Plugin_013_init = true;

        Plugin_013_TRIG_Pin = Settings.TaskDevicePin1[event->TaskIndex];
        Plugin_013_IRQ_Pin = Settings.TaskDevicePin2[event->TaskIndex];

        if (sonar)
        {
          delete sonar;
          sonar=NULL;
        }

        sonar = new NewPing(Plugin_013_TRIG_Pin, Plugin_013_IRQ_Pin);
        success = true;
        break;
      }

    case PLUGIN_EXIT:
      {
        if (sonar)
        {
          delete sonar;
          sonar=NULL;
        }
        break;
      }

    case PLUGIN_READ: // If we select value mode, read and send the value based on global timer
      {
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == 1)
        {
          Plugin_013_TRIG_Pin = Settings.TaskDevicePin1[event->TaskIndex];
          float value = Plugin_013_read();
          String log = F("ULTRASONIC : Distance: ");
          if (value > 0)
          {
            UserVar[event->BaseVarIndex] = value;
            log += UserVar[event->BaseVarIndex];
            success = true;
          }
          else
            log += F("No reading!");

        addLog(LOG_LEVEL_INFO,log);
        }
        break;
      }

    case PLUGIN_TEN_PER_SECOND: // If we select state mode, do more frequent checks and send only state changes
      {
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == 2)
        {
          Plugin_013_TRIG_Pin = Settings.TaskDevicePin1[event->TaskIndex];
          byte state = 0;
          float value = Plugin_013_read();
          if (value > 0)
          {
            if (value < Settings.TaskDevicePluginConfig[event->TaskIndex][1])
              state = 1;
            if (state != switchstate[event->TaskIndex])
            {
              String log = F("ULTRASONIC : State ");
              log += state;
              addLog(LOG_LEVEL_INFO,log);
              switchstate[event->TaskIndex] = state;
              UserVar[event->BaseVarIndex] = state;
              event->sensorType = SENSOR_TYPE_SWITCH;
              sendData(event);
            }
          }
        }
        success = true;
        break;
      }
  }
  return success;
}

/*********************************************************************/
float Plugin_013_read()
/*********************************************************************/
{
  if (!sonar)
  {
    return 0;
  }

  return sonar->ping_cm();
}
#endif // USES_P013
