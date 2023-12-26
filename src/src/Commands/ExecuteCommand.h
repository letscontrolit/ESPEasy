#ifndef COMMANDS_EXECUTECOMMAND_H
#define COMMANDS_EXECUTECOMMAND_H

#include "../../ESPEasy_common.h"

#include "../DataTypes/EventValueSource.h"
#include "../DataTypes/TaskIndex.h"


// Execute command which may be plugin or internal commands
bool ExecuteCommand_all(EventValueSource::Enum source,
                        const char            *Line);

bool ExecuteCommand_all_config(EventValueSource::Enum source,
                               const char            *Line);

bool ExecuteCommand_plugin_config(EventValueSource::Enum source,
                                  const char            *Line);


bool ExecuteCommand_internal(EventValueSource::Enum source,
                             const char            *Line);


bool ExecuteCommand(taskIndex_t            taskIndex,
                    EventValueSource::Enum source,
                    const char            *Line,
                    bool                   tryPlugin,
                    bool                   tryInternal,
                    bool                   tryRemoteConfig);


#endif // ifndef COMMANDS_EXECUTECOMMAND_H
