#ifndef HELPERS_WEBSERVER_COMMANDHELPER_H
#define HELPERS_WEBSERVER_COMMANDHELPER_H

#include "../DataTypes/EventValueSource.h"


enum class HandledWebCommand_result {
    NoCommand = 0,
    CommandHandled,
    Unknown_or_restricted_command,
    IP_not_allowed
};

// Check if we got a command as argument and try to execute it.
HandledWebCommand_result handle_command_from_web(EventValueSource::Enum source, String& webrequest);

#endif // HELPERS_WEBSERVER_COMMANDHELPER_H