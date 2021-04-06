#include "../Commands/Tasks.h"


#include "../../ESPEasy_common.h"


#include "../Commands/Common.h"

#include "../ESPEasyCore/Controller.h"
#include "../ESPEasyCore/Serial.h"

#include "../Globals/RuntimeData.h"

#include "../Helpers/Misc.h"
#include "../Helpers/Rules_calculate.h"
#include "../Helpers/StringConverter.h"

//      taskIndex = (event->Par1 - 1);   Par1 is here for 1 ... TASKS_MAX
//	varNr = event->Par2 - 1;
bool validTaskVars(struct EventStruct *event, taskIndex_t& taskIndex, unsigned int& varNr)
{
  if (event == nullptr) { return false; }

  if (event->Par1 <= 0) { return false; }
  taskIndex_t tmp_taskIndex = static_cast<taskIndex_t>(event->Par1 - 1);

  varNr = 0;

  if (event->Par2 > 0) {
    varNr = event->Par2 - 1;
  }

  if (!validTaskIndex(tmp_taskIndex)) { return false; }

  if (varNr >= VARS_PER_TASK) { return false; }

  taskIndex = tmp_taskIndex;

  return true;
}

bool taskValueSet(struct EventStruct *event, const char *Line, taskIndex_t& taskIndex)
{
  String TmpStr1;
  unsigned int varNr;

  if (!validTaskVars(event, taskIndex, varNr)) { return false; }
  unsigned int uservarIndex = (VARS_PER_TASK * taskIndex) + varNr;

  if (GetArgv(Line, TmpStr1, 4)) {
    // Perform calculation with float result.
    double result = 0;

    if (isError(Calculate(TmpStr1, result))) {
      return false;
    }
    UserVar[uservarIndex] = result;
  } else  {
    // TODO: Get Task description and var name
    serialPrintln(String(UserVar[uservarIndex]));
  }
  return true;
}

String Command_Task_Clear(struct EventStruct *event, const char *Line)
{
  taskIndex_t  taskIndex;
  unsigned int varNr;

  if (!validTaskVars(event, taskIndex, varNr)) { return return_command_failed(); }

  taskClear(taskIndex, true);
  return return_command_success();
}

String Command_Task_ClearAll(struct EventStruct *event, const char *Line)
{
  for (taskIndex_t t = 0; t < TASKS_MAX; t++) {
    taskClear(t, false);
  }
  return return_command_success();
}

String Command_Task_EnableDisable(struct EventStruct *event, bool enable)
{
  taskIndex_t  taskIndex;
  unsigned int varNr;
  String dummy;

  if (validTaskVars(event, taskIndex, varNr)) {
    // This is a command so no guarantee the taskIndex is correct in the event
    event->setTaskIndex(taskIndex);

    if (setTaskEnableStatus(event, enable)) {
      return return_command_success();
    }
  }
  return return_command_failed();
}

String Command_Task_Disable(struct EventStruct *event, const char *Line)
{
  return Command_Task_EnableDisable(event, false);
}

String Command_Task_Enable(struct EventStruct *event, const char *Line)
{
  return Command_Task_EnableDisable(event, true);
}

String Command_Task_ValueSet(struct EventStruct *event, const char *Line)
{
  taskIndex_t taskIndex;

  if (taskValueSet(event, Line, taskIndex)) { return return_command_success(); }
  return return_command_failed();
}

String Command_Task_ValueToggle(struct EventStruct *event, const char *Line)
{
  taskIndex_t  taskIndex;
  unsigned int varNr;

  if (!validTaskVars(event, taskIndex, varNr)) { return return_command_failed(); }
  unsigned int uservarIndex = (VARS_PER_TASK * taskIndex) + varNr;
  const int    result       = round(UserVar[uservarIndex]);

  if ((result == 0) || (result == 1)) {
    UserVar[uservarIndex] = (result == 0) ? 1.0f : 0.0f;
  }
  return return_command_success();
}

String Command_Task_ValueSetAndRun(struct EventStruct *event, const char *Line)
{
  taskIndex_t taskIndex;

  if (taskValueSet(event, Line, taskIndex))
  {
    SensorSendTask(taskIndex);
    return return_command_success();
  }
  return return_command_failed();
}

String Command_Task_Run(struct EventStruct *event, const char *Line)
{
  taskIndex_t  taskIndex;
  unsigned int varNr;

  if (!validTaskVars(event, taskIndex, varNr)) { return return_command_failed(); }

  SensorSendTask(taskIndex);
  return return_command_success();
}

String Command_Task_RemoteConfig(struct EventStruct *event, const char *Line)
{
  struct EventStruct TempEvent(event->TaskIndex);
  String request = Line;

  // FIXME TD-er: Should we call ExecuteCommand here? The command is not parsed like any other call.
  remoteConfig(&TempEvent, request);
  return return_command_success();
}
