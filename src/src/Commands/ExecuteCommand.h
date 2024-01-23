#ifndef COMMANDS_EXECUTECOMMAND_H
#define COMMANDS_EXECUTECOMMAND_H

#include "../../ESPEasy_common.h"

#include "../DataTypes/EventValueSource.h"
#include "../DataTypes/TaskIndex.h"

#include <list>

struct ExecuteCommandArgs {
  ExecuteCommandArgs(EventValueSource::Enum source,
                     const char            *Line);

  ExecuteCommandArgs(EventValueSource::Enum source,
                     const String         & Line);

  ExecuteCommandArgs(EventValueSource::Enum source,
                     String              && Line);


  ExecuteCommandArgs(taskIndex_t            taskIndex,
                     EventValueSource::Enum source,
                     const char            *Line,
                     bool                   tryPlugin,
                     bool                   tryInternal,
                     bool                   tryRemoteConfig);

  ExecuteCommandArgs(taskIndex_t            taskIndex,
                     EventValueSource::Enum source,
                     const String         & Line,
                     bool                   tryPlugin,
                     bool                   tryInternal,
                     bool                   tryRemoteConfig);

  ExecuteCommandArgs(taskIndex_t            taskIndex,
                     EventValueSource::Enum source,
                     String              && Line,
                     bool                   tryPlugin,
                     bool                   tryInternal,
                     bool                   tryRemoteConfig);

  taskIndex_t            _taskIndex = INVALID_TASK_INDEX;
  EventValueSource::Enum _source    = EventValueSource::Enum::VALUE_SOURCE_NOT_SET;
  String                 _Line;
  bool                   _tryPlugin       = false;
  bool                   _tryInternal     = false;
  bool                   _tryRemoteConfig = false;
};

extern std::list<ExecuteCommandArgs> ExecuteCommand_queue;

bool processExecuteCommandQueue();

// Execute command which may be plugin or internal commands
bool ExecuteCommand_all(ExecuteCommandArgs&& args,
                        bool                      addToQueue = false);

bool ExecuteCommand_all_config(ExecuteCommandArgs&& args,
                               bool                      addToQueue = false);

bool ExecuteCommand_plugin_config(ExecuteCommandArgs&& args,
                                  bool                      addToQueue = false);


bool ExecuteCommand_internal(ExecuteCommandArgs&& args,
                             bool                      addToQueue = false);


bool ExecuteCommand(taskIndex_t            taskIndex,
                    EventValueSource::Enum source,
                    const char            *Line,
                    bool                   tryPlugin,
                    bool                   tryInternal,
                    bool                   tryRemoteConfig,
                    bool                   addToQueue = false);

bool ExecuteCommand(ExecuteCommandArgs&& args, bool addToQueue);


#endif // ifndef COMMANDS_EXECUTECOMMAND_H
