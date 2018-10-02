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
//#include <shared_ptr>
#include <NewPingESP8266.h>

// map of sensors
std::map<unsigned int, std::shared_ptr<NewPingESP8266> > P_013_sensordefs;

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

        // set default values
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = 500;   // max. distance
        Settings.TaskDevicePluginConfig[event->TaskIndex][5] = 5;     // Filtersize

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
        byte choiceMode = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        byte choiceUnit = Settings.TaskDevicePluginConfig[event->TaskIndex][3];
        byte choiceFilter = Settings.TaskDevicePluginConfig[event->TaskIndex][4];

        String strUnit = (choiceUnit == 1) ? F("cm") : F("inch");
        String options[2];
        options[0] = F("Value");
        options[1] = F("State");
        int optionValues[2] = { 1, 2 };
        addFormSelector(F("Mode"), F("plugin_013_mode"), 2, options, optionValues, choiceMode);

        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == 2)
        {
        	addFormNumericBox(F("Threshold"), F("plugin_013_threshold"), Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
          addUnit(strUnit);
        }
        addFormNumericBox(F("Max Distance"), F("plugin_013_max_distance"), Settings.TaskDevicePluginConfig[event->TaskIndex][2], 0, 500);
        addUnit(strUnit);

        String options1[2];
        options1[0] = F("Metric");
        options1[1] = F("Imperial");
        int optionValues1[2] = { 1, 2 };
        addFormSelector(F("Unit"), F("plugin_013_Unit"), 2, options1, optionValues1, choiceUnit);

        String options2[2];
        options2[0] = F("None");
        options2[1] = F("Median");
        int optionValues2[2] = { 1, 2 };
        addFormSelector(F("Filter"), F("plugin_013_FilterType"), 2, options2, optionValues2, choiceFilter);

        if (Settings.TaskDevicePluginConfig[event->TaskIndex][4] == 2)
        {
        	addFormNumericBox(F("Filter size"), F("Plugin_013_FilterSize"), Settings.TaskDevicePluginConfig[event->TaskIndex][5], 2, 20);
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
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = getFormItemInt(F("plugin_013_max_distance"));

        Settings.TaskDevicePluginConfig[event->TaskIndex][3] = getFormItemInt(F("plugin_013_Unit"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][4] = getFormItemInt(F("plugin_013_FilterType"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][5] = getFormItemInt(F("Plugin_013_FilterSize"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        int16_t Plugin_013_TRIG_Pin = Settings.TaskDevicePin1[event->TaskIndex];
        int16_t Plugin_013_IRQ_Pin = Settings.TaskDevicePin2[event->TaskIndex];
        int16_t max_distance = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        int16_t choiceUnit = Settings.TaskDevicePluginConfig[event->TaskIndex][3];
        int16_t choiceFilter = Settings.TaskDevicePluginConfig[event->TaskIndex][4];
        int16_t filterSize = Settings.TaskDevicePluginConfig[event->TaskIndex][5];

        P_013_sensordefs.erase(event->TaskIndex);
        P_013_sensordefs[event->TaskIndex] =
          std::shared_ptr<NewPingESP8266> (new NewPingESP8266(Plugin_013_TRIG_Pin, Plugin_013_IRQ_Pin, max_distance));

        String log = F("ULTRASONIC : TaskNr: ");
        log += event->TaskIndex +1;
        log += F(" TrigPin: ");
        log += Plugin_013_TRIG_Pin;
        log += F(" IRQ_Pin: ");
        log += Plugin_013_IRQ_Pin;
        log += F(" max dist ");
        log += (choiceUnit == 1) ? F("[cm]: ") : F("[inch]: ");
        log += max_distance;
        log += F(" max echo: ");
        log += P_013_sensordefs[event->TaskIndex]->getMaxEchoTime();
        log += F(" Filter: ");
        if (choiceFilter == 1)
          log += F("none");
        else
          if (choiceFilter == 2) {
            log += F("Median size: ");
            log += filterSize;
          }
          else
            log += F("invalid!");
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
          UserVar[event->BaseVarIndex] = value;
          log += UserVar[event->BaseVarIndex];
          if (value == 0)
          {
             log += F(" Error: ");
             log += Plugin_013_getErrorStatusString(event->TaskIndex);
          }

          addLog(LOG_LEVEL_INFO,log);
        }
        success = true;
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
              String log = F("ULTRASONIC : TaskNr: ");
              log += event->TaskIndex +1;
              log += F(" state: ");
              log += state;
              addLog(LOG_LEVEL_INFO,log);
              switchstate[event->TaskIndex] = state;
              UserVar[event->BaseVarIndex] = state;
              event->sensorType = SENSOR_TYPE_SWITCH;
              sendData(event);
            }
          }
          else {
            String log = F("ULTRASONIC : TaskNr: ");
            log += event->TaskIndex +1;
            log += F(" Error: ");
            log += Plugin_013_getErrorStatusString(event->TaskIndex);
            addLog(LOG_LEVEL_INFO,log);
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

  int16_t max_cm_distance = Settings.TaskDevicePluginConfig[taskIndex][2];
  int16_t choiceUnit = Settings.TaskDevicePluginConfig[taskIndex][3];
  int16_t choiceFilter = Settings.TaskDevicePluginConfig[taskIndex][4];
  int16_t filterSize = Settings.TaskDevicePluginConfig[taskIndex][5];

  unsigned int distance = 0;

  switch  (choiceFilter) {
    case 1:
      distance = (P_013_sensordefs[taskIndex])->ping();
      break;
    case 2:
      distance = (P_013_sensordefs[taskIndex])->ping_median(filterSize, max_cm_distance);
      break;
    default:
      addLog(LOG_LEVEL_INFO, F("invalid Filter Type setting!"));
  }

  if (choiceUnit == 1)
    return NewPingESP8266::convert_cm_F(distance);
  else
    return NewPingESP8266::convert_in_F(distance);
}

/*********************************************************************/
String Plugin_013_getErrorStatusString(unsigned int taskIndex)
/*********************************************************************/
{
  if (P_013_sensordefs.count(taskIndex) == 0)
    return String(F("invalid taskindex"));

  switch ((P_013_sensordefs[taskIndex])->getErrorState()) {
    case NewPingESP8266::STATUS_SENSOR_READY: {
      return String(F("Sensor ready"));
      break;
    }

    case NewPingESP8266::STATUS_MEASUREMENT_VALID: {
      return String(F("no error, measurement valid"));
      break;
    }

    case NewPingESP8266::STATUS_ECHO_TRIGGERED: {
      return String(F("Echo triggered, waiting for Echo end"));
      break;
    }

    case NewPingESP8266::STATUS_ECHO_STATE_ERROR: {
      return String(F("Echo pulse error, Echopin not low on trigger"));
      break;
    }

    case NewPingESP8266::STATUS_ECHO_START_TIMEOUT_50ms: {
      return String(F("Echo timeout error, no echo start whithin 50 ms"));
      break;
    }

    case NewPingESP8266::STATUS_ECHO_START_TIMEOUT_DISTANCE: {
      return String(F("Echo timeout error, no echo start whithin time for max. distance"));
      break;
    }

    case NewPingESP8266::STATUS_MAX_DISTANCE_EXCEEDED: {
      return String(F("Echo too late, maximum distance exceeded"));
      break;
    }

    default: {
      return String(F("unknown error"));
      break;
    }

  }
}
#endif // USES_P013
