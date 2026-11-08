#include "../Helpers/CommandArgParser.h"

#include "../Globals/RulesCalculate.h"
#include "../Helpers/Rules_calculate.h"
#include "../Helpers/StringConverter.h"

bool CommandArgParser::isValid() const
{
  if (_hasCommand && !getCommand().isSet()) {
    return false;
  }

  if (_hasSubcommand && !getSubCommand().isSet()) {
    return false;
  }
  return _kv.operator bool();
}

void CommandArgParser::clear()
{
  _hasCommand    = false;
  _hasSubcommand = false;
  _kv.clear();
}

bool CommandArgParser::_init(
  const String& string,
  char          separator,
  bool          keepRawStrings)
{
  const bool argumentsOnly = !_hasCommand;

  if (argumentsOnly) { _hasSubcommand = false; }

  _kv.clear();
  uint8_t indexFind = 0;

  unsigned int string_pos = 0, argc_pos = 0;
  const char  *string_c_str = string.c_str();

  String arg_str;

  if (!argumentsOnly) {
    if (!GetArgv(string_c_str, string.length(), arg_str, ++indexFind, string_pos, argc_pos, separator)) {
      return false;
    }
    _kv = KeyValueStruct(arg_str);
    _kv._key.setCaseFormat(ValueStruct::CaseFormat::ToLower);
  }

  while (GetArgv(string_c_str, string.length(), arg_str, ++indexFind, string_pos, argc_pos, separator))
  {
    if (_hasSubcommand && (indexFind == 2)) {
      // In order to match subcommand, set format to lower case
      ValueStruct v(arg_str);
      v.setCaseFormat(ValueStruct::CaseFormat::ToLower);
      _kv.appendValue(std::move(v));
    } else {
      if (keepRawStrings) { _kv.appendValue(arg_str); }
      else {
        if (!arg_str.isEmpty() && (arg_str[0] == '=')) {
          ESPEASY_RULES_FLOAT_TYPE param{};

          // Starts with an '=', so Calculate starting at next position
          CalculateReturnCode returnCode = Calculate(arg_str.substring(1), param);

          if (!isError(returnCode)) {
            _kv.appendValue(ValueStruct(param));
          } else {
            _kv.appendValue(ValueStruct::makeFromString(arg_str));
          }
        } else {
          if (ContainsAny(arg_str, F(":#[](){},\"'`"))) { _kv.appendValue(arg_str); }
          else {
            _kv.appendValue(ValueStruct::makeFromString(arg_str));
          }
        }
      }
    }
  }

  return isValid();
}

const ValueStruct& CommandArgParser::getCommand() const
{
  return _kv._key;
}

const ValueStruct& CommandArgParser::getSubCommand() const
{
  if (_hasSubcommand && (_kv._values.size() > 0)) {
    return _kv._values[0];
  }
  return INVALID_VALUESTRUCT;
}

bool CommandArgParser::commandEquals(const __FlashStringHelper *cmdStr) const
{
  // Command is always compared using ignore case
  return getCommand().equalsIgnoreCase(cmdStr);
}

bool CommandArgParser::commandEquals(const String& cmdStr) const
{
  // Command is always compared using ignore case
  return getCommand().equalsIgnoreCase(cmdStr);
}

bool CommandArgParser::subCommandEquals(const __FlashStringHelper *cmdStr) const
{
  // SubCommand is always compared using ignore case
  return getSubCommand().equalsIgnoreCase(cmdStr);
}

bool CommandArgParser::subCommandEquals(const String& cmdStr) const
{
  // SubCommand is always compared using ignore case
  return getSubCommand().equalsIgnoreCase(cmdStr);
}

const ValueStruct& CommandArgParser::getArg(uint8_t index) const
{
  if (_hasSubcommand) { ++index; }

  if (index < _kv._values.size()) {
    return _kv._values[index];
  }

  return INVALID_VALUESTRUCT;
}

int64_t CommandArgParser::getArgInt(uint8_t index, int64_t defaultValue) const
{
  return getArg(index).toInt(defaultValue);
}

size_t CommandArgParser::getNrArgs() const {
  if (_hasSubcommand && _kv._values.size()) { return _kv._values.size() - 1; }
  return _kv._values.size();
}

#ifndef BUILD_NO_DEBUG

void CommandArgParser::debug(const __FlashStringHelper *comment, uint8_t logLevel) const
{
  if (!loglevelActiveFor(logLevel)) { return; }
  String logstr = comment;

  if (logstr.isEmpty()) { logstr += F("CommandArgParser"); }
  logstr += ':';

  if (_hasCommand) {
    logstr += concat(
      F(" cmd:"),
      getCommand().toString());
  }

  if (_hasSubcommand) {
    logstr += concat(
      F(" sub:"),
      getSubCommand().toString());
  }

  if (!_kv.operator bool()) {
    logstr += F(" notSet");
  }

  for (uint8_t i = 0; i < getNrArgs(); ++i)
  {
    const ValueStruct& v = getArg(i);
    logstr += strformat(F(" [%d]%s"), i, v.debug().c_str());
  }

  if (!logstr.isEmpty()) {
    addLog(logLevel, logstr);
  }
}

#endif // ifndef BUILD_NO_DEBUG
