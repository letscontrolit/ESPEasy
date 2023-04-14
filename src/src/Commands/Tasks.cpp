#include "../Commands/Tasks.h"


#include "../../ESPEasy_common.h"
#include "../../_Plugin_Helper.h"

#include "../Commands/Common.h"

#include "../DataStructs/TimingStats.h"

#include "../ESPEasyCore/Controller.h"
#include "../ESPEasyCore/Serial.h"

#include "../Globals/ESPEasy_Scheduler.h"
#include "../Globals/RuntimeData.h"
#include "../Globals/Settings.h"

#include "../Helpers/Misc.h"
#include "../Helpers/Numerical.h"
#include "../Helpers/Rules_calculate.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringParser.h"

// taskIndex = (event->Par1 - 1);   Par1 is here for 1 ... TASKS_MAX
bool validTaskIndexVar(struct EventStruct *event, taskIndex_t& taskIndex)
{
  if (event == nullptr) { return false; }

  if (event->Par1 <= 0) { return false; }
  const taskIndex_t tmp_taskIndex = static_cast<taskIndex_t>(event->Par1 - 1);

  if (!validTaskIndex(tmp_taskIndex)) { return false; }

  taskIndex = tmp_taskIndex;

  return true;
}

// taskIndex = (event->Par1 - 1);   Par1 is here for 1 ... TASKS_MAX
// varNr = event->Par2 - 1;
bool validTaskVars(struct EventStruct *event, taskIndex_t& taskIndex, unsigned int& varNr)
{
  if (event == nullptr) { return false; }

  taskIndex_t tmp_taskIndex;
  if (!validTaskIndexVar(event, tmp_taskIndex)) { return false; }

  varNr = 0;

  if (event->Par2 > 0 && event->Par2 <= VARS_PER_TASK) {
    varNr = event->Par2 - 1;
    taskIndex = tmp_taskIndex;
    return true;
  }

  return false;
}

bool validateAndParseTaskIndexArguments(struct EventStruct * event, const char *Line, taskIndex_t &taskIndex)
{
  if (!validTaskIndexVar(event, taskIndex)) {
    String taskName;
    taskIndex_t tmpTaskIndex = taskIndex;
    if ((event->Par1 <= 0 || event->Par1 >= INVALID_TASK_INDEX) && GetArgv(Line, taskName, 2)) {
      tmpTaskIndex = findTaskIndexByName(taskName, true);
      if (tmpTaskIndex != INVALID_TASK_INDEX) {
        event->Par1 = tmpTaskIndex + 1;
      }
    }
    return validTaskIndexVar(event, taskIndex);
  }
  return true;
}

/**
 * parse TaskName/TaskValue when not numeric for task name and value name and validate values
 */
bool validateAndParseTaskValueArguments(struct EventStruct * event, const char *Line, taskIndex_t &taskIndex, unsigned int &varNr)
{
  if (!validTaskVars(event, taskIndex, varNr) || (event->Par2 <= 0 || event->Par2 >= VARS_PER_TASK))  // Extra check required because of shortcutting in validTaskVars()
  { 
    String taskName;
    taskIndex_t tmpTaskIndex = taskIndex;
    if ((event->Par1 <= 0 || event->Par1 >= INVALID_TASK_INDEX) && GetArgv(Line, taskName, 2)) {
      tmpTaskIndex = findTaskIndexByName(taskName, true);
      if (tmpTaskIndex != INVALID_TASK_INDEX) {
        event->Par1 = tmpTaskIndex + 1;
      }
    }
    String valueName;
    if ((event->Par2 <= 0 || event->Par2 >= VARS_PER_TASK) && event->Par1 - 1 != INVALID_TASK_INDEX && GetArgv(Line, valueName, 3))
    {
      uint8_t tmpVarNr = findDeviceValueIndexByName(valueName, event->Par1 - 1);
      if (tmpVarNr != VARS_PER_TASK) {
        event->Par2 = tmpVarNr + 1;
      }
    }
    if (!validTaskVars(event, taskIndex, varNr)) return false; 
  }

  return true;
}

const __FlashStringHelper * taskValueSet(struct EventStruct *event, const char *Line, taskIndex_t& taskIndex, bool& success)
{
  String TmpStr1;
  unsigned int varNr;

  if (!validateAndParseTaskValueArguments(event, Line, taskIndex, varNr)) {
    success = false;
    return F("INVALID_PARAMETERS");
  }
  if (!Settings.AllowTaskValueSetAllPlugins() && getPluginID_from_TaskIndex(taskIndex) != 33) { // PluginID 33 = Dummy Device
    success = false;
    return F("NOT_A_DUMMY_TASK");
  }
  if (!Settings.TaskDeviceEnabled[taskIndex]) {
    success = false;
    return F("TASK_NOT_ENABLED");
  }
  const uint8_t valueCount = getValueCountForTask(taskIndex);
  if (valueCount <= varNr) {
    success = false;
    return F("INVALID_VAR_INDEX");
  }

  EventStruct tmpEvent(taskIndex);
  if (GetArgv(Line, TmpStr1, 4)) {
    const Sensor_VType sensorType = tmpEvent.getSensorType();

    // FIXME TD-er: Must check if the value has to be computed and not convert to double when sensor type is 64 bit int.

    // Perform calculation with float result.
    double result = 0;

    if (isError(Calculate(TmpStr1, result))) {
      success = false;
      return F("CALCULATION_ERROR");
    }
    UserVar.set(taskIndex, varNr, result, sensorType);
  } else  {
    // TODO: Get Task description and var name
    serialPrintln(formatUserVarNoCheck(&tmpEvent, varNr));
  }
  success = true;
  return return_command_success();
}

const __FlashStringHelper * Command_Task_Clear(struct EventStruct *event, const char *Line)
{
  taskIndex_t  taskIndex;

  if (!validateAndParseTaskIndexArguments(event, Line, taskIndex)) {
    return F("INVALID_PARAMETERS"); 
  }

  taskClear(taskIndex, true);
  return return_command_success();
}

const __FlashStringHelper * Command_Task_ClearAll(struct EventStruct *event, const char *Line)
{
  for (taskIndex_t t = 0; t < TASKS_MAX; t++) {
    taskClear(t, false);
  }
  return return_command_success();
}

const __FlashStringHelper * Command_Task_EnableDisable(struct EventStruct *event, bool enable, const char *Line)
{
  taskIndex_t  taskIndex;
  if (validateAndParseTaskIndexArguments(event, Line, taskIndex)) {
    // This is a command so no guarantee the taskIndex is correct in the event
    event->setTaskIndex(taskIndex);

    #if FEATURE_PLUGIN_PRIORITY
    if (!Settings.isPriorityTask(event->TaskIndex))
    #endif // if FEATURE_PLUGIN_PRIORITY
    {
      if (setTaskEnableStatus(event, enable)) {
        return return_command_success();
      }
    }
    return return_command_failed();
  }
  return F("INVALID_PARAMETERS");
}

#if FEATURE_PLUGIN_PRIORITY
const __FlashStringHelper * Command_PriorityTask_DisableTask(struct EventStruct *event, const char *Line)
{
  taskIndex_t  taskIndex;

  if (validateAndParseTaskIndexArguments(event, Line, taskIndex)) {
    // This is a command so no guarantee the taskIndex is correct in the event
    event->setTaskIndex(taskIndex);

    if (Settings.isPowerManagerTask(event->TaskIndex))
    {
      Settings.setPowerManagerTask(event->TaskIndex, false);
      return return_command_success();
    }
    // Handle other Priotiry task options
    Settings.setTaskEnableReadonly(event->TaskIndex, false);
    return return_command_failed();
  }
  return F("INVALID_PARAMETERS");
}
#endif // if FEATURE_PLUGIN_PRIORITY

const __FlashStringHelper * Command_Task_Disable(struct EventStruct *event, const char *Line)
{
  return Command_Task_EnableDisable(event, false, Line);
}

const __FlashStringHelper * Command_Task_Enable(struct EventStruct *event, const char *Line)
{
  return Command_Task_EnableDisable(event, true, Line);
}

#if FEATURE_PLUGIN_PRIORITY
const __FlashStringHelper * Command_PriorityTask_Disable(struct EventStruct *event, const char *Line)
{
  return Command_PriorityTask_DisableTask(event, Line);
}
#endif

const __FlashStringHelper * Command_Task_ValueSet(struct EventStruct *event, const char *Line)
{
  taskIndex_t taskIndex;
  bool success;
  return taskValueSet(event, Line, taskIndex, success);
}

const __FlashStringHelper * Command_Task_ValueToggle(struct EventStruct *event, const char *Line)
{
  taskIndex_t  taskIndex;
  unsigned int varNr;

  if (!validateAndParseTaskValueArguments(event, Line, taskIndex, varNr)) {
    return F("INVALID_PARAMETERS");
  }
  if (!Settings.TaskDeviceEnabled[taskIndex]) {
    return F("TASK_NOT_ENABLED");
  }

  EventStruct tmpEvent(taskIndex);
  const Sensor_VType sensorType = tmpEvent.getSensorType();
  if (sensorType == Sensor_VType::SENSOR_TYPE_LONG) {
    UserVar.setSensorTypeLong(taskIndex, !UserVar.getSensorTypeLong(taskIndex));
  } else if (isLongOutputDataType(sensorType)) {
    UserVar.setInt32(taskIndex, varNr, !UserVar.getInt32(taskIndex, varNr));
  } else if (isULongOutputDataType(sensorType)) {
    UserVar.setUint32(taskIndex, varNr, !UserVar.getUint32(taskIndex, varNr));
  } else if (isInt64OutputDataType(sensorType)) {
    UserVar.setInt64(taskIndex, varNr, !UserVar.getInt64(taskIndex, varNr));
  } else if (isUInt64OutputDataType(sensorType)) {
    UserVar.setUint64(taskIndex, varNr, !UserVar.getUint64(taskIndex, varNr));
  } else if (isDoubleOutputDataType(sensorType)) {
    const int    result       = lround(UserVar.getDouble(taskIndex, varNr));
    if ((result == 0) || (result == 1)) {
      UserVar.setDouble(taskIndex, varNr, (result == 0) ? 1.0 : 0.0);
    }
  } else {
    const unsigned int uservarIndex = tmpEvent.BaseVarIndex + varNr;
    const int    result       = lround(UserVar[uservarIndex]);

    if ((result == 0) || (result == 1)) {
      UserVar[uservarIndex] = (result == 0) ? 1.0f : 0.0f;
    }
  }
  return return_command_success();
}

const __FlashStringHelper * Command_Task_ValueSetAndRun(struct EventStruct *event, const char *Line)
{
  taskIndex_t taskIndex;
  bool success;
  const __FlashStringHelper * returnvalue = taskValueSet(event, Line, taskIndex, success);
  if (success)
  {
    START_TIMER;
    SensorSendTask(taskIndex);
    STOP_TIMER(SENSOR_SEND_TASK);

    return return_command_success();
  }
  return returnvalue;
}

const __FlashStringHelper * Command_ScheduleTask_Run(struct EventStruct *event, const char* Line)
{
  taskIndex_t  taskIndex;

  if (!validateAndParseTaskIndexArguments(event, Line, taskIndex) || event->Par2 < 0) {
    return F("INVALID_PARAMETERS");
  }
  if (!Settings.TaskDeviceEnabled[taskIndex]) {
    return F("TASK_NOT_ENABLED");
  }

  unsigned int msecFromNow = 0;
  String par3;
  if (GetArgv(Line, par3, 3)) {
    if (validUIntFromString(par3, msecFromNow)) {
      Scheduler.schedule_task_device_timer(taskIndex, millis() + msecFromNow);
      return return_command_success();
    }
  }
  return F("INVALID_PARAMETERS");  
}

const __FlashStringHelper * Command_Task_Run(struct EventStruct *event, const char *Line)
{
  taskIndex_t  taskIndex;

  if (!validateAndParseTaskIndexArguments(event, Line, taskIndex) || event->Par2 < 0) {
    return F("INVALID_PARAMETERS");
  }
  if (!Settings.TaskDeviceEnabled[taskIndex]) {
    return F("TASK_NOT_ENABLED");
  }
  unsigned int unixTime = 0;
  String par3;
  if (GetArgv(Line, par3, 3)) {
    validUIntFromString(par3, unixTime);
  }

  START_TIMER;
  SensorSendTask(taskIndex, unixTime);
  STOP_TIMER(SENSOR_SEND_TASK);

  return return_command_success();
}

const __FlashStringHelper * Command_Task_RemoteConfig(struct EventStruct *event, const char *Line)
{
  struct EventStruct TempEvent(event->TaskIndex);
  String request = Line;

  // FIXME TD-er: Should we call ExecuteCommand here? The command is not parsed like any other call.
  remoteConfig(&TempEvent, request);
  return return_command_success();
}
