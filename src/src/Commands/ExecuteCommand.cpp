#include "../Commands/ExecuteCommand.h"

#include "../../_Plugin_Helper.h"

#include "../Commands/Common.h"
#include "../Commands/InternalCommands.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

#include "../ESPEasyCore/Controller.h"

#include "../Helpers/StringConverter.h"
#include "../Helpers/StringParser.h"

// Execute command which may be plugin or internal commands
bool ExecuteCommand_all(EventValueSource::Enum source, const char *Line)
{
  return ExecuteCommand(INVALID_TASK_INDEX, source, Line, true, true, false);
}

bool ExecuteCommand_all_config(EventValueSource::Enum source, const char *Line)
{
  return ExecuteCommand(INVALID_TASK_INDEX, source, Line, true, true, true);
}

bool ExecuteCommand_plugin_config(EventValueSource::Enum source, const char *Line)
{
  return ExecuteCommand(INVALID_TASK_INDEX, source, Line, true, false, true);
}


bool ExecuteCommand_internal(EventValueSource::Enum source, const char *Line)
{
  return ExecuteCommand(INVALID_TASK_INDEX, source, Line, false, true, false);
}


bool ExecuteCommand(taskIndex_t            taskIndex,
                    EventValueSource::Enum source,
                    const char            *Line,
                    bool                   tryPlugin,
                    bool                   tryInternal,
                    bool                   tryRemoteConfig)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("ExecuteCommand"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  String cmd;

  // We first try internal commands, which should not have a taskIndex set.
  struct EventStruct TempEvent;

  if (!GetArgv(Line, cmd, 1)) {
    SendStatus(&TempEvent, return_command_failed());
    return false;
  }

  if (tryInternal) {
    // Small optimization for events, which happen frequently
    // FIXME TD-er: Make quick check to see if a command is an internal command, so we don't need to try all
    if (cmd.equalsIgnoreCase(F("event"))) {
      tryPlugin       = false;
      tryRemoteConfig = false;
    }
  }

  TempEvent.Source = source;

  String action(Line);
  action = parseTemplate(action); // parseTemplate before executing the command

  // Split the arguments into Par1...5 of the event.
  // Do not split it in executeInternalCommand, since that one will be called from the scheduler with pre-set events.
  // FIXME TD-er: Why call this for all commands? The CalculateParam function is quite heavy.
  parseCommandString(&TempEvent, action);

  // FIXME TD-er: This part seems a bit strange.
  // It can't schedule a call to PLUGIN_WRITE.
  // Maybe ExecuteCommand can be scheduled?
  delay(0);

#ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    addLogMove(LOG_LEVEL_DEBUG, concat(F("Command: "), cmd));
    addLog(LOG_LEVEL_DEBUG, Line); // for debug purposes add the whole line.
    addLogMove(LOG_LEVEL_DEBUG, strformat(
                 F("Par1: %d Par2: %d Par3: %d Par4: %d Par5: %d"),
                 TempEvent.Par1,
                 TempEvent.Par2,
                 TempEvent.Par3,
                 TempEvent.Par4,
                 TempEvent.Par5));
  }
#endif // ifndef BUILD_NO_DEBUG


  if (tryInternal) {
    InternalCommands internalCommands(cmd.c_str(), &TempEvent, action.c_str());
    bool handled = internalCommands.executeInternalCommand();

    const command_case_data& data = internalCommands.getData();

    if (data.status.length() > 0) {
      delay(0);
      SendStatus(&TempEvent, data.status);
      delay(0);
    }

    if (handled) {
      //      addLog(LOG_LEVEL_INFO, F("executeInternalCommand accepted"));
      return true;
    }
  }

  // When trying a task command, set the task index, even if it is not a valid task index.
  // For example commands from elsewhere may not have a proper task index.
  TempEvent.setTaskIndex(taskIndex);
  checkDeviceVTypeForTask(&TempEvent);

  if (tryPlugin) {
    // Use a tmp string to call PLUGIN_WRITE, since PluginCall may inadvertenly
    // alter the string.
    String tmpAction(action);
    bool   handled = PluginCall(PLUGIN_WRITE, &TempEvent, tmpAction);

    //    if (handled) addLog(LOG_LEVEL_INFO, F("PLUGIN_WRITE accepted"));

    #ifndef BUILD_NO_DEBUG

    if (!tmpAction.equals(action)) {
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        String log = F("PLUGIN_WRITE altered the string: ");
        log += action;
        log += F(" to: ");
        log += tmpAction;
        addLogMove(LOG_LEVEL_ERROR, log);
      }
    }
    #endif // ifndef BUILD_NO_DEBUG

    if (!handled) {
      // Try a controller
      handled = CPluginCall(CPlugin::Function::CPLUGIN_WRITE, &TempEvent, tmpAction);

      //      if (handled) addLog(LOG_LEVEL_INFO, F("CPLUGIN_WRITE accepted"));
    }

    if (handled) {
      SendStatus(&TempEvent, return_command_success());
      return true;
    }
  }

  if (tryRemoteConfig) {
    if (remoteConfig(&TempEvent, action)) {
      SendStatus(&TempEvent, return_command_success());

      //      addLog(LOG_LEVEL_INFO, F("remoteConfig accepted"));

      return true;
    }
  }
  const String errorUnknown = concat(F("Command unknown: "), action);
  addLog(LOG_LEVEL_INFO, errorUnknown);
  SendStatus(&TempEvent, errorUnknown);
  delay(0);
  return false;
}
