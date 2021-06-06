#ifndef COMMANDS_INTERNALCOMMANDS_H
#define COMMANDS_INTERNALCOMMANDS_H

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../Globals/Plugins.h"

#include <Arduino.h>


bool checkSourceFlags(EventValueSource::Enum source, EventValueSourceGroup::Enum group);

bool checkNrArguments(const char *cmd, const char *Line, int nrArguments);


// Typedef for function pointer to be called for handling an internal command.
typedef String (*command_function)(struct EventStruct *, const char *);
typedef const __FlashStringHelper * (*command_function_fs)(struct EventStruct *, const char *);
// Simple struct to be used in handling commands.
// By packing all into a struct, the macro calling do_command_case generates a lot less code
// resulting in a smaller binary.
struct command_case_data {

    command_case_data(const char *cmd, struct EventStruct *event, const char *line);


    String cmd_lc;
    const char  *cmd;
    struct EventStruct *event;
    const char *line;
    String status;
    bool retval = false;

};

bool do_command_case(command_case_data& data, const String& cmd_test, command_function_fs pFunc, int nrArguments, EventValueSourceGroup::Enum group);
bool do_command_case(command_case_data& data, const String& cmd_test, command_function pFunc, int nrArguments, EventValueSourceGroup::Enum group);


/*********************************************************************************************\
* Registers command
\*********************************************************************************************/
bool executeInternalCommand(command_case_data & data);


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