#ifdef USES_P013
//#######################################################################################################
//############################### Plugin 013: HC-SR04, RCW-0001, etc. ###################################
//#######################################################################################################

#define PLUGIN_013
#define PLUGIN_ID_013        13
#define PLUGIN_NAME_013       "Distance - HC-SR04, RCW-0001, etc."
#define PLUGIN_VALUENAME1_013 "Distance"

#include <Arduino.h>
#include <map>
#include <NewPingESP8266.h>

struct P_013_sensordef {
  P_013_sensordef() : sonar(NULL) {}

  P_013_sensordef(byte TRIG_Pin, byte IRQ_Pin, int16_t max_cm_distance) : sonar(NULL) {
    sonar = new NewPingESP8266(TRIG_Pin, IRQ_Pin, max_cm_distance);
  }

  ~P_013_sensordef() {
    if (sonar != NULL) {
      delete sonar;
      sonar = NULL;
    }
  }

  NewPingESP8266 *sonar;
};

std::map<unsigned int, P_013_sensordef> P_013_sensordefs;




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
        addFormNumericBox(F("Max Distance"), F("plugin_013_max_distance"), Settings.TaskDevicePluginConfig[event->TaskIndex][2], 0, 500);
        addUnit(F("cm"));

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
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = getFormItemInt(F("plugin_013_max_distance"));
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        byte Plugin_013_TRIG_Pin = Settings.TaskDevicePin1[event->TaskIndex];
        byte Plugin_013_IRQ_Pin = Settings.TaskDevicePin2[event->TaskIndex];
        int16_t max_cm_distance = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        P_013_sensordefs[event->TaskIndex] =
          P_013_sensordef(Plugin_013_TRIG_Pin, Plugin_013_IRQ_Pin, max_cm_distance);
        String log = F("ULTRASONIC : TaskNr: ");
        log += event->TaskIndex +1;
        log += F(" TrigPin: ");
        log += Plugin_013_TRIG_Pin;
        log += F(" IRQ_Pin: ");
        log += Plugin_013_IRQ_Pin;
        log += F(" max dist cm: ");
        log += max_cm_distance;
        log += F(" max echo: ");
        log += P_013_sensordefs[event->TaskIndex].sonar->getMaxEchoTime();
        log += F(" nr_tasks: ");
        log += P_013_sensordefs.size();
        addLog(LOG_LEVEL_INFO, log);

        unsigned long tmpmillis = millis();
        unsigned long tmpmicros = micros();
        delay(100);
        long millispassed = timePassedSince(tmpmillis);
        long microspassed = usecPassedSince(tmpmicros);

        log = F("ULTRASONIC : micros() test: ");
        log += millispassed;
        log += F(" msec, ");
        log += microspassed;
        log += F(" usec, ");
        addLog(LOG_LEVEL_INFO, log);


        success = true;
        break;
      }

    case PLUGIN_EXIT:
      {
        P_013_sensordefs.erase(event->TaskIndex);
        break;
      }

    case PLUGIN_READ: // If we select value mode, read and send the value based on global timer
      {
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == 1)
        {
          float value = Plugin_013_read(event->TaskIndex);
          String log = F("ULTRASONIC : TaskNr: ");
          log += event->TaskIndex +1;
          log += F(" Distance: ");
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
          byte state = 0;
          float value = Plugin_013_read(event->TaskIndex);
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
float Plugin_013_read(unsigned int taskIndex)
/*********************************************************************/
{
  if (P_013_sensordefs.count(taskIndex) == 0) return 0;
  if (P_013_sensordefs[taskIndex].sonar == NULL) return 0;
  delay(1);
  float distance = P_013_sensordefs[taskIndex].sonar->ping_cm();
  delay(1);
  return distance;
}
#endif // USES_P013
