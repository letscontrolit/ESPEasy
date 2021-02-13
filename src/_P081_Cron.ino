#include "_Plugin_Helper.h"
// #######################################################################################################
// #################################### Plugin 081: CRON tasks Scheduler       ###########################
// #######################################################################################################

//-V::795

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
#endif  // ifndef PLUGIN_081_DEBUG
#define PLUGIN_081_EXPRESSION_SIZE 41
#define LASTEXECUTION         0
#define NEXTEXECUTION         1


struct P081_data_struct : public PluginTaskData_base {
  explicit P081_data_struct(const String& expression)
  {
    const char *error;

    memset(&_expr, 0, sizeof(_expr));
    cron_parse_expr(expression.c_str(), &_expr, &error);

    if (!error) {
      _initialized = true;
    } else {
      _error = String(error);
    }
  }

  ~P081_data_struct() {}

  bool isInitialized() {
    return _initialized;
  }

  bool hasError(String& error) {
    if (_initialized) { return false; }
    error = _error;
    return true;
  }

  time_t get_cron_next(time_t date) {
    if (!_initialized) { return CRON_INVALID_INSTANT; }
    return cron_next((cron_expr *)&_expr, date);
  }

  time_t get_cron_prev(time_t date) {
    if (!_initialized) { return CRON_INVALID_INSTANT; }
    return cron_prev((cron_expr *)&_expr, date);
  }

private:

  String    _error;
  cron_expr _expr;
  bool      _initialized = false;
};


String P081_getCronExpr(taskIndex_t taskIndex)
{
  char expression[PLUGIN_081_EXPRESSION_SIZE + 1];

  ZERO_FILL(expression);
  LoadCustomTaskSettings(taskIndex, (byte *)&expression, PLUGIN_081_EXPRESSION_SIZE);
  String res(expression);
  res.trim();
  return res;
}

time_t P081_computeNextCronTime(taskIndex_t taskIndex, time_t last)
{
  P081_data_struct *P081_data =
    static_cast<P081_data_struct *>(getPluginTaskData(taskIndex));

  if ((nullptr != P081_data) && P081_data->isInitialized()) {
    //    int32_t freeHeapStart = ESP.getFreeHeap();

    time_t res = P081_data->get_cron_next(last);

    /*
        int32_t freeHeapEnd = ESP.getFreeHeap();

        if (freeHeapEnd < freeHeapStart) {
          String log = F("Cron: Free Heap Decreased: ");
          log += String(freeHeapStart - freeHeapEnd);
          log += F(" (");
          log += freeHeapStart;
          log += F(" -> ");
          log += freeHeapEnd;
          addLog(LOG_LEVEL_INFO, log);
        }
     */
    return res;
  }
  return CRON_INVALID_INSTANT;
}

time_t P081_getCronExecTime(taskIndex_t taskIndex, byte varNr)
{
  return static_cast<time_t>(UserVar.getUint32(taskIndex, varNr));
}

void P081_setCronExecTimes(struct EventStruct *event, time_t lastExecTime, time_t nextExecTime) {
  UserVar.setUint32(event->TaskIndex, LASTEXECUTION, static_cast<uint32_t>(lastExecTime));
  UserVar.setUint32(event->TaskIndex, NEXTEXECUTION, static_cast<uint32_t>(nextExecTime));
}

time_t P081_getCurrentTime()
{
  node_time.now();

  // FIXME TD-er: Why work on a deepcopy of tm?
  struct tm current = node_time.tm;
  return mktime((struct tm *)&current);
}

void P081_check_or_init(struct EventStruct *event)
{
  if (node_time.systemTimePresent()) {
    const time_t current_time = P081_getCurrentTime();
    time_t last_exec_time     = P081_getCronExecTime(event->TaskIndex, LASTEXECUTION);
    time_t next_exec_time     = P081_getCronExecTime(event->TaskIndex, NEXTEXECUTION);

    // Must check if the values of LASTEXECUTION and NEXTEXECUTION make sense.
    // These can be invalid values from a reboot, or simply contain uninitialized values.
    if ((last_exec_time > current_time) || (last_exec_time == CRON_INVALID_INSTANT) || (next_exec_time == CRON_INVALID_INSTANT)) {
      // Last execution time cannot be correct.
      last_exec_time = CRON_INVALID_INSTANT;
      const time_t tmp_next = P081_computeNextCronTime(event->TaskIndex, current_time);

      if ((tmp_next < next_exec_time) || (next_exec_time == CRON_INVALID_INSTANT)) {
        next_exec_time = tmp_next;
      }
      P081_setCronExecTimes(event, CRON_INVALID_INSTANT, next_exec_time);
    }
  }
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
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_NONE;  // type of value the plugin will return, used only for Domoticz
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
      String expression = web_server.arg(F("p081_cron_exp"));
      String log;
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
            addLog(LOG_LEVEL_DEBUG, F("Cron Elapsed"));

            time_t last_exec_time = next_exec_time;
            next_exec_time = P081_computeNextCronTime(event->TaskIndex, current_time);
            P081_setCronExecTimes(event, last_exec_time, next_exec_time);

            addLog(LOG_LEVEL_DEBUG, String(F("Next execution:")) + ESPEasy_time::getDateTimeString(*gmtime(&next_exec_time)));

            if (function != PLUGIN_TIME_CHANGE) {
              if (Settings.UseRules) {
                LoadTaskSettings(event->TaskIndex);
                eventQueue.add(String(F("Cron#")) + String(ExtraTaskSettings.TaskDeviceName));
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


String P081_formatExecTime(taskIndex_t taskIndex, byte varNr) {
  time_t exec_time = P081_getCronExecTime(taskIndex, varNr);

  if (exec_time != CRON_INVALID_INSTANT) {
    return ESPEasy_time::getDateTimeString(*gmtime(&exec_time));
  }
  return F("-");
}

void P081_html_show_cron_expr(struct EventStruct *event) {
  P081_data_struct *P081_data =
    static_cast<P081_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr != P081_data) && P081_data->isInitialized()) {
    String error;

    if (P081_data->hasError(error)) {
      addRowLabel(F("Error"));
      addHtml(error);
    } else {
      addRowLabel(F("Last Exec Time"));
      addHtml(P081_formatExecTime(event->TaskIndex, LASTEXECUTION));
      addRowLabel(F("Next Exec Time"));
      addHtml(P081_formatExecTime(event->TaskIndex, NEXTEXECUTION));
    }
  }
}

#endif // USES_P081
