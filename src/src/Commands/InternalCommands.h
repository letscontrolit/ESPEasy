#ifndef COMMANDS_INTERNALCOMMANDS_H
#define COMMANDS_INTERNALCOMMANDS_H

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../Globals/Plugins.h"


bool checkSourceFlags(EventValueSource::Enum source, EventValueSourceGroup::Enum group);

bool checkNrArguments(const char *cmd, const char *Line, int nrArguments);

typedef String (*command_function)(struct EventStruct *, const char *);
bool do_command_case(const String& cmd_lc, const char *cmd, struct EventStruct *event, const char *line, String& status, const String& cmd_test, command_function pFunc, int nrArguments, EventValueSourceGroup::Enum group, bool& retval);


/*********************************************************************************************\
* Registers command
\*********************************************************************************************/
bool executeInternalCommand(const char *cmd, struct EventStruct *event, const char *line, String& status);


// Execute command which may be plugin or internal commands
bool ExecuteCommand_all(EventValueSource::Enum source, const char *Line);

bool ExecuteCommand_all_config(EventValueSource::Enum source, const char *Line);

bool ExecuteCommand_plugin_config(EventValueSource::Enum source, const char *Line);

bool ExecuteCommand_all_config_eventOnly(EventValueSource::Enum source, const char *Line);

bool ExecuteCommand_internal(EventValueSource::Enum source, const char *Line);

bool ExecuteCommand_plugin(EventValueSource::Enum source, const char *Line);

bool ExecuteCommand_plugin(taskIndex_t taskIndex, EventValueSource::Enum source, const char *Line);

bool ExecuteCommand(taskIndex_t taskIndex, EventValueSource::Enum source, const char *Line, bool tryPlugin, bool tryInternal, bool tryRemoteConfig);


#endif // COMMANDS_INTERNALCOMMANDS_H