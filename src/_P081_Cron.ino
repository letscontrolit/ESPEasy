#include "_Plugin_Helper.h"

// #######################################################################################################
// #################################### Plugin 081: CRON tasks Scheduler       ###########################
// #######################################################################################################

// -V::795

#ifdef USES_P081


# include "src/PluginStructs/P081_data_struct.h"

# define PLUGIN_081
# define PLUGIN_ID_081      81                        // plugin id
# define PLUGIN_NAME_081   "Generic - CRON" // "Plugin Name" is what will be displayed in the selection list
# define PLUGIN_VALUENAME1_081 "LastExecution"
# define PLUGIN_VALUENAME2_081 "NextExecution"


boolean Plugin_081(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      // This case defines the device characteristics, edit appropriately

      Device[++deviceCount].Number           = PLUGIN_ID_081;
      Device[deviceCount].Type               = DEVICE_TYPE_DUMMY;              // how the device is connected
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_NONE; // type of value the plugin will return, used only for
                                                                               // Domoticz
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 2; // number of output variables. The value should match the number of keys
                                                  // PLUGIN_VALUENAME1_xxx
      Device[deviceCount].SendDataOption   = false;
      Device[deviceCount].TimerOption      = false;
      Device[deviceCount].TimerOptional    = false;
      Device[deviceCount].GlobalSyncOption = true;
      Device[deviceCount].DecimalsOnly     = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      // return the device name
      string = F(PLUGIN_NAME_081);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      // called when the user opens the module configuration page
      // it allows to add a new row for each output variable of the plugin
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_081));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_081));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormSubHeader(F("Schedule"));
      addFormTextBox(F("CRON Expression")
                     , F("p081_cron_exp")
                     , P081_getCronExpr(event->TaskIndex)
                     , 39);

      addFormNote(F("S  M  H  DoM  Month  DoW"));

      P081_html_show_cron_expr(event);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      String expression = webArg(F("p081_cron_exp"));
      String log;
      {
        char expression_c[PLUGIN_081_EXPRESSION_SIZE] = {};
        safe_strncpy(expression_c, expression, PLUGIN_081_EXPRESSION_SIZE);
        log = SaveCustomTaskSettings(event->TaskIndex, reinterpret_cast<const uint8_t *>(&expression_c), PLUGIN_081_EXPRESSION_SIZE);
      }

      if (log.length() > 0)
      {
        addLog(LOG_LEVEL_ERROR, String(PSTR(PLUGIN_NAME_081)) + F(": Saving ") + log);
      }

      clearPluginTaskData(event->TaskIndex);
      P081_setCronExecTimes(event, CRON_INVALID_INSTANT, CRON_INVALID_INSTANT);
      success = true;
      break;
    }

    case PLUGIN_FORMAT_USERVAR:
    {
      switch (event->idx) {
        case 0:
          string = P081_formatExecTime(event->TaskIndex, LASTEXECUTION);
          break;
        case 1:
          string = P081_formatExecTime(event->TaskIndex, NEXTEXECUTION);
          break;
      }
      success = string.length() > 0;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P081_data_struct(P081_getCronExpr(event->TaskIndex)));
      P081_data_struct *P081_data =
        static_cast<P081_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P081_data) {
        return success;
      }

      if (P081_data->isInitialized()) {
        P081_check_or_init(event);
        success = true;
      } else {
        clearPluginTaskData(event->TaskIndex);
      }
      break;
    }


    case PLUGIN_READ:
    {
      // Need to return true here, so the last and next exec times are stored in RTC.
      success = true;
      break;
    }

    case PLUGIN_TIME_CHANGE:
    case PLUGIN_ONCE_A_SECOND:
    {
      // code to be executed once a second. Tasks which do not require fast response can be added here
      if (node_time.systemTimePresent()) {
        P081_check_or_init(event);
        time_t next_exec_time = P081_getCronExecTime(event->TaskIndex, NEXTEXECUTION);

        if (next_exec_time != CRON_INVALID_INSTANT) {
          const time_t current_time = P081_getCurrentTime();
          const bool   cron_elapsed = (next_exec_time <= current_time);

          if (cron_elapsed) {
            # ifndef BUILD_NO_DEBUG
            addLog(LOG_LEVEL_DEBUG, F("Cron Elapsed"));
            # endif // ifndef BUILD_NO_DEBUG

            time_t last_exec_time = next_exec_time;
            next_exec_time = P081_computeNextCronTime(event->TaskIndex, current_time);
            P081_setCronExecTimes(event, last_exec_time, next_exec_time);

            # ifndef BUILD_NO_DEBUG
            addLog(LOG_LEVEL_DEBUG, String(F("Next execution:")) + formatDateTimeString(*gmtime(&next_exec_time)));
            # endif // ifndef BUILD_NO_DEBUG

            if (function != PLUGIN_TIME_CHANGE) {
              if (Settings.UseRules) {
                eventQueue.add(String(F("Cron#")) + getTaskDeviceName(event->TaskIndex));
              }
              success = true;
            }
          }
        } else {
          addLog(LOG_LEVEL_ERROR, F("CRON: INVALID INSTANT"));
        }
      } else {
        addLog(LOG_LEVEL_ERROR, F("CRON: Time not synced"));
      }


      break;
    }
  } // switch

  return success;
}   // function


#endif // USES_P081
