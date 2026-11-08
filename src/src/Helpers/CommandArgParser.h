#pragma once

#include "../../ESPEasy_common.h"

#include "../DataStructs/KeyValueStruct.h"

struct CommandArgParser {

  CommandArgParser() = default;

  bool isValid() const;

  operator bool() const {
    return isValid();
  }

  void clear();

  // consider given string as a list of:
  // - command
  // - 0 or more arguments
  // Set keepRawStrings to true when some parameters might need
  // special attention, like combinations of numericals and ':'
  bool readCommand(
    const String& string,
    char          separator      = ',',
    bool          keepRawStrings = false)
  {
    _hasCommand    = true;
    _hasSubcommand = false;
    return _init(string, separator, keepRawStrings);
  }

  bool readCommandAndMatch(
    const String             & string,
    const __FlashStringHelper *cmdToMatch,
    char                       separator      = ',',
    bool                       keepRawStrings = false)
  {
    _hasCommand    = true;
    _hasSubcommand = false;
    return
      _init(string, separator, keepRawStrings) &&
      commandEquals(cmdToMatch);
  }

  // consider given string as a list of:
  // - command
  // - subcommmand
  // - 0 or more arguments
  bool readCommandSubCommand(
    const String& string,
    char          separator      = ',',
    bool          keepRawStrings = false)
  {
    _hasCommand    = true;
    _hasSubcommand = true;
    return _init(string, separator, keepRawStrings);
  }

  bool readCommandSubCommandAndMatch(
    const String             & string,
    const __FlashStringHelper *cmdToMatch,
    char                       separator      = ',',
    bool                       keepRawStrings = false)
  {
    _hasCommand    = true;
    _hasSubcommand = true;
    return
      _init(string, separator, keepRawStrings) &&
      commandEquals(cmdToMatch);
  }

  // consider given string as a list of:
  // - 0 or more arguments
  bool readArgumentsOnly(
    const String& string,
    char          separator      = ',',
    bool          keepRawStrings = false)
  {
    _hasCommand    = false;
    _hasSubcommand = false;
    return _init(string, separator, keepRawStrings);
  }

private:

  bool _init(
    const String& string,
    char          separator,
    bool          keepRawStrings);

public:

  // When initialized with readCommand or readCommandSubCommand, returns the command
  // Otherwise return an unset ValueStruct
  const ValueStruct& getCommand() const;

  // When initialized with readCommandSubCommand, returns the subcommand
  // Otherwise return an unset ValueStruct
  const ValueStruct& getSubCommand() const;

  bool               commandEquals(const __FlashStringHelper *cmdStr) const;
  bool               commandEquals(const String& cmdStr) const;

  bool               subCommandEquals(const __FlashStringHelper *cmdStr) const;
  bool               subCommandEquals(const String& cmdStr) const;

  // Returns found argument, or an uninitialized ValueStruct when index refers to a non-existent
  // N.B. When initialized with readCommandSubCommand, index 0 is the first argument after the subcommand.
  // Otherwise index 0 is the first argument
  const ValueStruct& getArg(uint8_t index) const;

  // Returns int-representation of the requested argument or defaultValue if argument is not present or not valid.
  int64_t            getArgInt(uint8_t index,
                               int64_t defaultValue = 0) const;

  size_t             getNrArgs() const;

  bool               hasArgs() const { return getNrArgs() > 0; }

#ifndef BUILD_NO_DEBUG
  void               debug(const __FlashStringHelper *comment,
                           uint8_t                    logLevel = LOG_LEVEL_DEBUG) const;
#endif // ifndef BUILD_NO_DEBUG

private:

  KeyValueStruct _kv;

  bool _hasCommand{};
  bool _hasSubcommand{};

};
