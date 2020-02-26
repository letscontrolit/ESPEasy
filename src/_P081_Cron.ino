// #######################################################################################################
// #################################### Plugin 081: CRON tasks Scheduler       ###########################
// #######################################################################################################

// Maxim Integrated
#ifdef USES_P081

#include <ctype.h>
#include <time.h>

extern "C"
{
#include "ccronexpr.h"
}


#define PLUGIN_081
#define PLUGIN_ID_081      81                        // plugin id
#define PLUGIN_NAME_081   "Generic - CRON [TESTING]" // "Plugin Name" is what will be displayed in the selection list
#define PLUGIN_VALUENAME1_081 "LastExecution"
#define PLUGIN_VALUENAME2_081 "NextExecution"
#ifndef PLUGIN_081_DEBUG
  # define PLUGIN_081_DEBUG  false // set to true for extra log info in the debug
#endif // ifndef PLUGIN_081_DEBUG
#define PLUGIN_081_EXPRESSION_SIZE 41
#define LASTEXECUTION UserVar[event->BaseVarIndex]
#define NEXTEXECUTION UserVar[event->BaseVarIndex + 1]

union timeToFloat
{
  time_t time;
  float  value;
};


String P081_getCronExpr(taskIndex_t taskIndex)
{
  char expression[PLUGIN_081_EXPRESSION_SIZE + 1];

  ZERO_FILL(expression);
  LoadCustomTaskSettings(taskIndex, (byte *)&expression, PLUGIN_081_EXPRESSION_SIZE);
  return String(expression);
}

time_t P081_computeNextCronTime(const String& expression, time_t last, const char *error)
{
  cron_expr expr;

  memset(&expr, 0, sizeof(expr));
  error = nullptr;
  cron_parse_expr(expression.c_str(), &expr, &error);

  if (error)
  {
    // TODO: Notify at ui de error
    addLog(LOG_LEVEL_ERROR, String(F("CRON Expression: ")) + String(error));
    return CRON_INVALID_INSTANT;
  }
  return cron_next((cron_expr *)&expr, last);
}

time_t P081_computeNextCronTime(taskIndex_t taskIndex, time_t last)
{
  const char *error = nullptr;
  String expression = P081_getCronExpr(taskIndex);

  return P081_computeNextCronTime(expression, last, error);
}

time_t P081_getCronExecTime(float execTime)
{
  timeToFloat converter;

  converter.value = execTime;
  return converter.time;
}

void P081_setCronExecTimes(struct EventStruct *event, time_t lastExecTime, time_t nextExecTime) {
  timeToFloat converter;

  converter.time = lastExecTime;
  LASTEXECUTION  = converter.value;

  converter.time = nextExecTime;
  NEXTEXECUTION  = converter.value;
}

time_t P081_getCurrentTime()
{
  now();

  // FIXME TD-er: Why work on a deepcopy of tm?
  struct tm current = tm;
  return mktime((struct tm *)&current);
}

boolean Plugin_081(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      // This case defines the device characteristics, edit appropriately

      Device[++deviceCount].Number           = PLUGIN_ID_081;
      Device[deviceCount].Type               = DEVICE_TYPE_DUMMY; // how the device is connected
      Device[deviceCount].VType              = SENSOR_TYPE_NONE;  // type of value the plugin will return, used only for Domoticz
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
      String expression = P081_getCronExpr(event->TaskIndex);
      addFormTextBox(F("CRON Expression")
                     , F("p081_cron_exp")
                     , expression
                     , 39);

      addFormNote(F("S  M  H  DoM  Month  DoW"));

      P081_html_show_cron_expr(event, expression);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      String expression = WebServer.arg(F("p081_cron_exp"));
      {
        String log;
        const char *err = NULL;
        time_t last     = P081_getCurrentTime();
        time_t next     = P081_computeNextCronTime(expression, last, err);

        if (!err)
        {
          P081_setCronExecTimes(event, last, next);
        }
        else
        {
          log = String(F("CRON Expression: Error ")) + String(err);
          addHtmlError(log);
          addLog(LOG_LEVEL_ERROR, log);
        }
        {
          char expression_c[PLUGIN_081_EXPRESSION_SIZE];
          ZERO_FILL(expression_c);
          safe_strncpy(expression_c, expression, PLUGIN_081_EXPRESSION_SIZE);
          log = SaveCustomTaskSettings(event->TaskIndex, (byte *)&expression_c, PLUGIN_081_EXPRESSION_SIZE);
        }

        if (log != "")
        {
          log = String(PSTR(PLUGIN_NAME_081)) + String(F(": Saving ")) + log;
          addLog(LOG_LEVEL_ERROR, log);
        }
      }
      success = true;
      break;
    }
    case PLUGIN_WEBFORM_SHOW_VALUES:
    {
      addHtml(F("<div class=\"div_l\">"));
      addHtml(ExtraTaskSettings.TaskDeviceValueNames[0]);
      addHtml(F(":</div><div class=\"div_r\">"));
      addHtml(P081_formatExecTime(LASTEXECUTION));
      addHtml(F("</div><div class=\"div_br\"></div><div class=\"div_l\">"));
      addHtml(ExtraTaskSettings.TaskDeviceValueNames[1]);
      addHtml(F(":</div><div class=\"div_r\">"));
      addHtml(P081_formatExecTime(NEXTEXECUTION));
      addHtml(F("</div>"));
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      // this case defines code to be executed when the plugin is initialised
      // after the plugin has been initialised successfuly, set success and break
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      break;
    }

    case PLUGIN_EXIT:
    {
      // perform cleanup tasks here. For example, free memory

      break;
    }

    case PLUGIN_TIME_CHANGE:
    case PLUGIN_ONCE_A_SECOND:
    {
      // code to be executed once a second. Tasks which do not require fast response can be added here
      if (statusNTPInitialized) {
        const time_t current_time = P081_getCurrentTime();
        time_t last_exec_time     = P081_getCronExecTime(LASTEXECUTION);
        time_t next_exec_time     = P081_getCronExecTime(NEXTEXECUTION);

        // Must check if the values of LASTEXECUTION and NEXTEXECUTION make sense.
        // These can be invalid values from a reboot, or simply contain uninitialized values.
        if ((last_exec_time > current_time) || (last_exec_time == CRON_INVALID_INSTANT)) {
          // Last execution time cannot be correct.
          last_exec_time = CRON_INVALID_INSTANT;
          const time_t tmp_next = P081_computeNextCronTime(event->TaskIndex, current_time);

          if ((tmp_next < next_exec_time) || (next_exec_time == CRON_INVALID_INSTANT)) {
            next_exec_time = tmp_next;
          }
          P081_setCronExecTimes(event, CRON_INVALID_INSTANT, next_exec_time);
        }

        if (next_exec_time != CRON_INVALID_INSTANT) {
          if ((function == PLUGIN_TIME_CHANGE) || (next_exec_time <= current_time)) {
            addLog(LOG_LEVEL_DEBUG, F("Cron Elapsed"));
            last_exec_time = next_exec_time;
            next_exec_time = P081_computeNextCronTime(event->TaskIndex, last_exec_time);
            P081_setCronExecTimes(event, last_exec_time, next_exec_time);

            addLog(LOG_LEVEL_DEBUG, String(F("Next execution:")) + getDateTimeString(*gmtime(&next_exec_time)));

            if (function != PLUGIN_TIME_CHANGE) {
              LoadTaskSettings(event->TaskIndex);
              eventQueue.add(String(F("Cron#")) + String(ExtraTaskSettings.TaskDeviceName));
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

    case PLUGIN_TEN_PER_SECOND:
    {
      // code to be executed 10 times per second. Tasks which require fast response can be added here
      // be careful on what is added here. Heavy processing will result in slowing the module down!

      success = true;
      break;
    }
  } // switch
  return success;
}   // function

#if PLUGIN_081_DEBUG
void PrintCronExp(struct cron_expr_t e) {
  serialPrintln(F("===DUMP Cron Expression==="));
  serialPrint(F("Seconds:"));

  for (int i = 0; i < 8; i++)
  {
    serialPrint(e.seconds[i]);
    serialPrint(",");
  }
  serialPrintln();
  serialPrint(F("Minutes:"));

  for (int i = 0; i < 8; i++)
  {
    serialPrint(e.minutes[i]);
    serialPrint(",");
  }
  serialPrintln();
  serialPrint(F("hours:"));

  for (int i = 0; i < 3; i++)
  {
    serialPrint(e.hours[i]);
    serialPrint(",");
  }
  serialPrintln();
  serialPrint(F("months:"));

  for (int i = 0; i < 2; i++)
  {
    serialPrint(e.months[i]);
    serialPrint(",");
  }
  serialPrintln();
  serialPrint(F("days_of_week:"));

  for (int i = 0; i < 1; i++)
  {
    serialPrint(e.days_of_week[i]);
    serialPrint(",");
  }
  serialPrintln();
  serialPrint(F("days_of_month:"));

  for (int i = 0; i < 4; i++)
  {
    serialPrint(e.days_of_month[i]);
    serialPrint(",");
  }
  serialPrintln();
  serialPrintln(F("END=DUMP Cron Expression==="));
}

#endif // if PLUGIN_081_DEBUG


String P081_formatExecTime(float execTime_f) {
  time_t exec_time = P081_getCronExecTime(execTime_f);

  if (exec_time != CRON_INVALID_INSTANT) {
    return getDateTimeString(*gmtime(&exec_time));
  }
  return F("-");
}

void P081_html_show_cron_expr(struct EventStruct *event, const String& expression) {
  cron_expr e;

  memset(&e, 0, sizeof(e));
  const char *error = nullptr;
  cron_parse_expr(expression.c_str(), &e, &error);

  if (error) {
    addRowLabel(F("Error"));
    addHtml(String(error));
  } else {
    addRowLabel(F("Last Exec Time"));
    addHtml(P081_formatExecTime(LASTEXECUTION));
    addRowLabel(F("Next Exec Time"));
    addHtml(P081_formatExecTime(NEXTEXECUTION));
  }
}

#endif // USES_P081
