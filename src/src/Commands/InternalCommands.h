#ifndef COMMANDS_INTERNALCOMMANDS_H
#define COMMANDS_INTERNALCOMMANDS_H

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../Globals/Plugins.h"


bool checkNrArguments(const char *cmd, const char *Line, int nrArguments);

typedef String (*command_function)(struct EventStruct *, const char *);
bool do_command_case(const String& cmd_lc, const char *cmd, struct EventStruct *event, const char *line, String& status, const String& cmd_test, command_function pFunc, int nrArguments, bool& retval);


/*********************************************************************************************\
* Registers command
\*********************************************************************************************/
bool executeInternalCommand(const char *cmd, struct EventStruct *event, const char *line, String& status);


// Execute command which may be plugin or internal commands
bool ExecuteCommand_all(byte source, const char *Line);

bool ExecuteCommand_all_config(byte source, const char *Line);

bool ExecuteCommand_plugin_config(byte source, const char *Line);

bool ExecuteCommand_all_config_eventOnly(byte source, const char *Line);

bool ExecuteCommand_internal(byte source, const char *Line);

bool ExecuteCommand_plugin(byte source, const char *Line);

bool ExecuteCommand_plugin(taskIndex_t taskIndex, byte source, const char *Line);

bool ExecuteCommand(taskIndex_t taskIndex, byte source, const char *Line, bool tryPlugin, bool tryInternal, bool tryRemoteConfig);


#endif // COMMANDS_INTERNALCOMMANDS_H