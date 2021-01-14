#include "_Plugin_Helper.h"
#ifdef USES_P033

// #######################################################################################################
// #################################### Plugin 033: Dummy ################################################
// #######################################################################################################

# define PLUGIN_033
# define PLUGIN_ID_033         33
# define PLUGIN_NAME_033       "Generic - Dummy Device"
# define PLUGIN_VALUENAME1_033 "Dummy"
boolean Plugin_033(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_033;
      Device[deviceCount].Type               = DEVICE_TYPE_DUMMY;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].DecimalsOnly       = true;
      Device[deviceCount].ValueCount         = 4;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].OutputDataType     = Output_Data_type_t::All;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_033);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      // FIXME TD-er: Copy names as done in P026_Sysinfo.ino.
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_033));
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(0)));
      success     = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = static_cast<Sensor_VType>(PCONFIG(0));
      event->idx        = 0;
      success           = true;
      break;
    }

    case PLUGIN_READ:
    {
      event->sensorType = static_cast<Sensor_VType>(PCONFIG(0));

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        for (byte x = 0; x < getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(0))); x++)
        {
          String log = F("Dummy: value ");
          log += x + 1;
          log += F(": ");
          log += formatUserVarNoCheck(event->TaskIndex, x);
          addLog(LOG_LEVEL_INFO, log);
        }
      }
      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      String command = parseString(string, 1);

      if (command == F("dummyvalueset"))
      {
        if (event->Par1 == event->TaskIndex + 1) // make sure that this instance is the target
        {
          float floatValue = 0;

          if (string2float(parseString(string, 4), floatValue))
          {
            if (loglevelActiveFor(LOG_LEVEL_INFO))
            {
              String log = F("Dummy: Index ");
              log += event->Par1;
              log += F(" value ");
              log += event->Par2;
              log += F(" set to ");
              log += floatValue;
              addLog(LOG_LEVEL_INFO, log);
            }
            UserVar[event->BaseVarIndex + event->Par2 - 1] = floatValue;
            success                                        = true;
          } else { // float conversion failed!
            if (loglevelActiveFor(LOG_LEVEL_ERROR))
            {
              String log = F("Dummy: Index ");
              log += event->Par1;
              log += F(" value ");
              log += event->Par2;
              log += F(" parameter3: ");
              log += parseStringKeepCase(string, 4);
              log += F(" not a float value!");
              addLog(LOG_LEVEL_ERROR, log);
            }
          }
        }
      }
      break;
    }
  }
  return success;
}

#endif // USES_P033
