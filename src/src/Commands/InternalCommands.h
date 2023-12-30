#ifndef COMMANDS_INTERNALCOMMANDS_H
#define COMMANDS_INTERNALCOMMANDS_H

#include "../../ESPEasy_common.h"

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../Globals/Plugins.h"


// Simple struct to be used in handling commands.
// By packing all into a struct, the macro calling do_command_case generates a lot less code
// resulting in a smaller binary.
struct command_case_data {
  command_case_data(const char         *cmd,
                    struct EventStruct *event,
                    const char         *line);


  String              cmd_lc;
  const char         *cmd;
  struct EventStruct *event;
  const String        line;
  String              status;
  bool                retval = false;
};

class InternalCommands {
public:

  InternalCommands(const char         *cmd,
                   struct EventStruct *event,
                   const char         *line);

private:

  // Typedef for function pointer to be called for handling an internal command.
  typedef String (*command_function)(struct EventStruct *,
                                     const char *);
  typedef const __FlashStringHelper * (*command_function_fs)(struct EventStruct *,
                                                             const char *);


  bool        do_command_case_all(command_function_fs pFunc,
                                  int                 nrArguments);

  bool        do_command_case_all(command_function pFunc,
                                  int              nrArguments);

  bool        do_command_case_all_restricted(command_function_fs pFunc,
                                             int                 nrArguments);

  bool        do_command_case_all_restricted(command_function pFunc,
                                             int              nrArguments);

  static bool do_command_case(command_case_data         & data,
                              command_function_fs         pFunc,
                              int                         nrArguments,
                              EventValueSourceGroup::Enum group);
  static bool do_command_case(command_case_data         & data,
                              command_function            pFunc,
                              int                         nrArguments,
                              EventValueSourceGroup::Enum group);

public:

  /*********************************************************************************************\
  * Wrapper to call all commands
  \*********************************************************************************************/
  bool                     executeInternalCommand();

  const command_case_data& getData() const {
    return _data;
  }

private:

  command_case_data _data;
};

#endif // COMMANDS_INTERNALCOMMANDS_H
