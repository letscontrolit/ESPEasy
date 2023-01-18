#include "../PluginStructs/P081_data_struct.h"

#ifdef USES_P081

P081_data_struct::P081_data_struct(const String& expression)
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

bool P081_data_struct::hasError(String& error) const {
  if (_initialized) { return false; }
  error = _error;
  return true;
}

time_t P081_data_struct::get_cron_next(time_t date) const {
  if (!_initialized) { return CRON_INVALID_INSTANT; }
  return cron_next((cron_expr *)&_expr, date);
}

time_t P081_data_struct::get_cron_prev(time_t date) const {
  if (!_initialized) { return CRON_INVALID_INSTANT; }
  return cron_prev((cron_expr *)&_expr, date);
}

String P081_getCronExpr(taskIndex_t taskIndex)
{
  char expression[PLUGIN_081_EXPRESSION_SIZE + 1];

  ZERO_FILL(expression);
  LoadCustomTaskSettings(taskIndex, reinterpret_cast<uint8_t *>(&expression), PLUGIN_081_EXPRESSION_SIZE);
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

time_t P081_getCronExecTime(taskIndex_t taskIndex, uint8_t varNr)
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
  struct tm current = node_time.local_tm;

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

# if PLUGIN_081_DEBUG
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

# endif // if PLUGIN_081_DEBUG


String P081_formatExecTime(taskIndex_t taskIndex, uint8_t varNr) {
  time_t exec_time = P081_getCronExecTime(taskIndex, varNr);

  if (exec_time != CRON_INVALID_INSTANT) {
    return formatDateTimeString(*gmtime(&exec_time));
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

#endif // ifdef USES_P081
