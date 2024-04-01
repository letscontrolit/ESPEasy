#include "../Commands/ExecuteCommand.h"

#include "../../_Plugin_Helper.h"

#include "../Commands/Common.h"
#include "../Commands/InternalCommands.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

#include "../ESPEasyCore/Controller.h"

#include "../Helpers/StringConverter.h"
#include "../Helpers/StringParser.h"

ExecuteCommandArgs::ExecuteCommandArgs(EventValueSource::Enum source,
                                       const char            *Line) :
  _taskIndex(INVALID_TASK_INDEX),
  _source(source),
  _Line(Line),
  _tryPlugin(false),
  _tryInternal(false),
  _tryRemoteConfig(false) {}

ExecuteCommandArgs::ExecuteCommandArgs(EventValueSource::Enum source,
                                       const String         & Line) :
  _taskIndex(INVALID_TASK_INDEX),
  _source(source),
  _Line(Line),
  _tryPlugin(false),
  _tryInternal(false),
  _tryRemoteConfig(false) {}

ExecuteCommandArgs::ExecuteCommandArgs(EventValueSource::Enum source,
                                       String              && Line) :
  _taskIndex(INVALID_TASK_INDEX),
  _source(source),
  _Line(Line),
  _tryPlugin(false),
  _tryInternal(false),
  _tryRemoteConfig(false) {}


ExecuteCommandArgs::ExecuteCommandArgs(
  taskIndex_t            taskIndex,
  EventValueSource::Enum source,
  const char            *Line,
  bool                   tryPlugin,
  bool                   tryInternal,
  bool                   tryRemoteConfig) :
  _taskIndex(taskIndex),
  _source(source),
  _Line(Line),
  _tryPlugin(tryPlugin),
  _tryInternal(tryInternal),
  _tryRemoteConfig(tryRemoteConfig) {}

ExecuteCommandArgs::ExecuteCommandArgs(
  taskIndex_t            taskIndex,
  EventValueSource::Enum source,
  const String         & Line,
  bool                   tryPlugin,
  bool                   tryInternal,
  bool                   tryRemoteConfig) :
  _taskIndex(taskIndex),
  _source(source),
  _Line(Line),
  _tryPlugin(tryPlugin),
  _tryInternal(tryInternal),
  _tryRemoteConfig(tryRemoteConfig) {}

ExecuteCommandArgs::ExecuteCommandArgs(
  taskIndex_t            taskIndex,
  EventValueSource::Enum source,
  String              && Line,
  bool                   tryPlugin,
  bool                   tryInternal,
  bool                   tryRemoteConfig) :
  _taskIndex(taskIndex),
  _source(source),
  _Line(std::move(Line)),
  _tryPlugin(tryPlugin),
  _tryInternal(tryInternal),
  _tryRemoteConfig(tryRemoteConfig) {}


std::list<ExecuteCommandArgs> ExecuteCommand_queue;

bool processExecuteCommandQueue()
{
  bool res = false;

  if (!ExecuteCommand_queue.empty()) {
    auto it = ExecuteCommand_queue.front();

    res = ExecuteCommand(std::move(it), false);
    ExecuteCommand_queue.pop_front();
  }
  return res;
}

// Execute command which may be plugin or internal commands
bool ExecuteCommand_all(ExecuteCommandArgs&& args,
                        bool                      addToQueue)
{
  args._tryPlugin = true;
  args._tryInternal = true;
  args._tryRemoteConfig = false;
  return ExecuteCommand(std::move(args), addToQueue);
}

bool ExecuteCommand_all_config(ExecuteCommandArgs&& args,
                               bool                      addToQueue)
{
  args._tryPlugin = true;
  args._tryInternal = true;
  args._tryRemoteConfig = true;
  return ExecuteCommand(std::move(args), addToQueue);
}

bool ExecuteCommand_plugin_config(ExecuteCommandArgs&& args,
                                  bool                      addToQueue)
{
  args._tryPlugin = true;
  args._tryInternal = false;
  args._tryRemoteConfig = true;
  return ExecuteCommand(std::move(args), addToQueue);
}

bool ExecuteCommand_internal(ExecuteCommandArgs&& args,
                             bool                      addToQueue)
{
  args._tryPlugin = false;
  args._tryInternal = true;
  args._tryRemoteConfig = false;
  return ExecuteCommand(std::move(args), addToQueue);
}

bool ExecuteCommand(taskIndex_t            taskIndex,
                    EventValueSource::Enum source,
                    const char            *Line,
                    bool                   tryPlugin,
                    bool                   tryInternal,
                    bool                   tryRemoteConfig,
                    bool                   addToQueue)
{
  ExecuteCommandArgs args(taskIndex,
                          source,
                          Line,
                          tryPlugin,
                          tryInternal,
                          tryRemoteConfig);
  return ExecuteCommand(std::move(args), addToQueue);
}

bool ExecuteCommand(ExecuteCommandArgs&& args, bool addToQueue)
{
  if (addToQueue) {
    ExecuteCommand_queue.emplace_back(std::move(args));
    return false; // What should be returned here?
  }
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("ExecuteCommand"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  String cmd;

  // We first try internal commands, which should not have a taskIndex set.
  struct EventStruct TempEvent;

  if (!GetArgv(args._Line.c_str(), cmd, 1)) {
    SendStatus(&TempEvent, return_command_failed());
    return false;
  }

  if (args._tryInternal) {
    // Small optimization for events, which happen frequently
    // FIXME TD-er: Make quick check to see if a command is an internal command, so we don't need to try all
    if (cmd.equalsIgnoreCase(F("event"))) {
      args._tryPlugin       = false;
      args._tryRemoteConfig = false;
    }
  }

  TempEvent.Source = args._source;

  #ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    addLogMove(LOG_LEVEL_DEBUG, concat(F("Command: "), cmd));
    addLog(LOG_LEVEL_DEBUG, args._Line); // for debug purposes add the whole line.
  }
  #endif

  args._Line = parseTemplate(args._Line); // parseTemplate before executing the command

  // Split the arguments into Par1...5 of the event.
  // Do not split it in executeInternalCommand, since that one will be called from the scheduler with pre-set events.
  // FIXME TD-er: Why call this for all commands? The CalculateParam function is quite heavy.
  parseCommandString(&TempEvent, args._Line);

  // FIXME TD-er: This part seems a bit strange.
  // It can't schedule a call to PLUGIN_WRITE.
  // Maybe ExecuteCommand can be scheduled?
  delay(0);

#ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    addLogMove(LOG_LEVEL_DEBUG, strformat(
                 F("Par1: %d Par2: %d Par3: %d Par4: %d Par5: %d"),
                 TempEvent.Par1,
                 TempEvent.Par2,
                 TempEvent.Par3,
                 TempEvent.Par4,
                 TempEvent.Par5));
  }
#endif // ifndef BUILD_NO_DEBUG


  if (args._tryInternal) {
    InternalCommands internalCommands(cmd.c_str(), &TempEvent, args._Line.c_str());
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
  TempEvent.setTaskIndex(args._taskIndex);
  checkDeviceVTypeForTask(&TempEvent);

  if (args._tryPlugin) {
    // Use a tmp string to call PLUGIN_WRITE, since PluginCall may inadvertenly
    // alter the string.
    String tmpAction(args._Line);
    bool   handled = PluginCall(PLUGIN_WRITE, &TempEvent, tmpAction);

    //    if (handled) addLog(LOG_LEVEL_INFO, F("PLUGIN_WRITE accepted"));

    #ifndef BUILD_NO_DEBUG

    if (!tmpAction.equals(args._Line)) {
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        String log = F("PLUGIN_WRITE altered the string: ");
        log += args._Line;
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

  if (args._tryRemoteConfig) {
    if (remoteConfig(&TempEvent, args._Line)) {
      SendStatus(&TempEvent, return_command_success());

      //      addLog(LOG_LEVEL_INFO, F("remoteConfig accepted"));

      return true;
    }
  }
  String errorUnknown = concat(F("Command unknown: "), args._Line);
  SendStatus(&TempEvent, errorUnknown);
  addLogMove(LOG_LEVEL_INFO, errorUnknown);
  delay(0);
  return false;
}
