#include "_Plugin_Helper.h"
#ifdef USES_P033

// #######################################################################################################
// #################################### Plugin 033: Dummy ################################################
// #######################################################################################################

# define PLUGIN_033
# define PLUGIN_ID_033         33
# define PLUGIN_NAME_033       "Generic - Dummy Device"
# define PLUGIN_VALUENAME1_033 "Dummy"
boolean Plugin_033(uint8_t function, struct EventStruct *event, String& string)
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
      Device[deviceCount].TimerOptional      = true;
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

#if PLUGIN_CONFIGFLOATVAR_MAX >= VARS_PER_TASK
    case PLUGIN_WEBFORM_LOAD:
    {
      addFormCheckBox(F("Restore state after power lost"), F("p033_restore"), static_cast<bool>(PCONFIG(1)));
#ifdef SIZE_1M
      addFormNote(F("May afect on flash health!"));
#else
      addFormNote(F("When enabled, the task will automatically monitor changes of variables every second and write changes to the filesystem.<br/>"
        "If your rules will too hard use this task, it may be dangerous for your storage.<br/>"
        "For this reason, the writings to the flash storage has daily limit, that also affect on this feature.<br/>"
        "So, you should use such task very carefully. For example for sloughly process, like heating boiler at winter-summer season."
        ));
#endif /*SIZE_1M*/
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(1) = isFormItemChecked(F("p033_restore"));
      success    = true;
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      if (PCONFIG(1))
      {
        unsigned count = getValueCountFromSensorType(static_cast < Sensor_VType > (PCONFIG(0)));
        if (memcmp(&PCONFIG_FLOAT(0), &UserVar[event->BaseVarIndex], count * sizeof(PCONFIG_FLOAT(0))))
        {
          memcpy(&PCONFIG_FLOAT(0), &UserVar[event->BaseVarIndex], count * sizeof(PCONFIG_FLOAT(0)));

          //SaveTaskSettings(event->TaskIndex);
          String res = SaveSettings();
          if (loglevelActiveFor(LOG_LEVEL_INFO))
          {
            String log = F("Dummy: flush changes: ");
            log += res;
            addLogMove(LOG_LEVEL_INFO, log);
          }
        }
      }
      break;
    }
#endif /* PLUGIN_CONFIGFLOATVAR_MAX >= VARS_PER_TASK */

    case PLUGIN_INIT:
    {
#if PLUGIN_CONFIGFLOATVAR_MAX >= VARS_PER_TASK
      if (PCONFIG(1))
      {
        unsigned count = getValueCountFromSensorType(static_cast < Sensor_VType > (PCONFIG(0)));
        memcpy(&UserVar[event->BaseVarIndex], &PCONFIG_FLOAT(0), count * sizeof(PCONFIG_FLOAT(0)));

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("Dummy: values restored");
          addLogMove(LOG_LEVEL_INFO, log);
        }
      }
#endif /* PLUGIN_CONFIGFLOATVAR_MAX >= VARS_PER_TASK */
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      event->sensorType = static_cast<Sensor_VType>(PCONFIG(0));

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        for (uint8_t x = 0; x < getValueCountFromSensorType(static_cast < Sensor_VType > (PCONFIG(0))); x++)
        {
          String log = F("Dummy: value ");
          log += x + 1;
          log += F(": ");
          log += formatUserVarNoCheck(event->TaskIndex, x);
          addLogMove(LOG_LEVEL_INFO, log);
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
              addLogMove(LOG_LEVEL_INFO, log);
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
              addLogMove(LOG_LEVEL_ERROR, log);
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
