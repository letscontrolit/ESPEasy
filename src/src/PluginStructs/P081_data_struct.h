#ifndef PLUGINSTRUCTS_P081_DATA_STRUCT_H
#define PLUGINSTRUCTS_P081_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P081


# include <ctype.h>
# include <time.h>


extern "C"
{
  # include "ccronexpr.h"
}

# ifndef PLUGIN_081_DEBUG
  #  define PLUGIN_081_DEBUG  false // set to true for extra log info in the debug
# endif // ifndef PLUGIN_081_DEBUG
# define PLUGIN_081_EXPRESSION_SIZE 41
# define LASTEXECUTION         0
# define NEXTEXECUTION         1


struct P081_data_struct : public PluginTaskData_base {
  P081_data_struct() = delete;
  explicit P081_data_struct(const String& expression);
  virtual ~P081_data_struct() = default;

  bool isInitialized() const {
    return _initialized;
  }

  bool   hasError(String& error) const;

  time_t get_cron_next(time_t date) const;

  time_t get_cron_prev(time_t date) const;

private:

  String    _error;
  cron_expr _expr;
  bool      _initialized = false;
};


String P081_getCronExpr(taskIndex_t taskIndex);

time_t P081_computeNextCronTime(taskIndex_t taskIndex,
                                time_t      last);

time_t P081_getCronExecTime(taskIndex_t taskIndex,
                            uint8_t     varNr);

void   P081_setCronExecTimes(struct EventStruct *event,
                             time_t              lastExecTime,
                             time_t              nextExecTime);

time_t P081_getCurrentTime();

void   P081_check_or_init(struct EventStruct *event);

# if PLUGIN_081_DEBUG
void   PrintCronExp(struct cron_expr_t e);
# endif // if PLUGIN_081_DEBUG

String P081_formatExecTime(taskIndex_t taskIndex,
                           uint8_t     varNr);

void   P081_html_show_cron_expr(struct EventStruct *event);

#endif // ifdef USES_P081
#endif // ifndef PLUGINSTRUCTS_P081_DATA_STRUCT_H
