#include "../Helpers/BusCmd_Helper.h"

#include <GPIO_Direct_Access.h>
#include "../Globals/RulesCalculate.h"

// These commands (not case-sensitive) must have the same order as the BusCmd_Commands_e enum class
const char BusCmd_commands[] PROGMEM =
  "n|g|p|r|w|s|t|e|c|v|d|a|z|i|l|"
  #if FEATURE_BUSCMD_STRING && FEATURE_STRING_VARIABLES
  "m|"
  #endif // if FEATURE_BUSCMD_STRING && FEATURE_STRING_VARIABLES
;
const char BusCmd_commandsLong[] PROGMEM =
  "nop|get|put|read|write|read16|write16|eval|calc|value|delay|enable|reset|if|let|"
  #if FEATURE_BUSCMD_STRING && FEATURE_STRING_VARIABLES
  "letstr|"
  #endif // if FEATURE_BUSCMD_STRING && FEATURE_STRING_VARIABLES
;

// Supported data formats, _ == undefined, not processed
const char BusCmd_dataFormats[] PROGMEM =
  "_|"
  "u8|"
  "u16|"
  "u24|"
  "u32|"
  "u16le|"
  "u24le|"
  "u32le|"
  "8|"
  "16|"
  "24|"
  "32|"
  "16le|"
  "24le|"
  "32le|"
  "b|"
  "w|"
  #if FEATURE_BUSCMD_STRING
  "str|"
  #endif // if FEATURE_BUSCMD_STRING
;

/**
 * Constructor BusCmd_Command_struct
 */
BusCmd_Command_struct::BusCmd_Command_struct(BusCmd_Command_e    _command,
                                             BusCmd_DataFormat_e _format,
                                             uint16_t            _reg,
                                             int64_t             _data,
                                             uint32_t            _len,
                                             String              _calculation,
                                             String              _variable)
  :command(_command), format(_format), reg(_reg), len(_len), calculation(_calculation), variable(_variable) {
  switch (format) {
    case BusCmd_DataFormat_e::undefined: d0_uint32_t = (uint32_t)_data; break; // Special case
    case BusCmd_DataFormat_e::uint8_t: d0_uint8_t    = (uint8_t)_data; break;
    case BusCmd_DataFormat_e::uint16_t:
    case BusCmd_DataFormat_e::uint16_t_LE: d0_uint16_t = (uint16_t)_data; break;
    case BusCmd_DataFormat_e::uint24_t:
    case BusCmd_DataFormat_e::uint32_t:
    case BusCmd_DataFormat_e::uint24_t_LE:
    case BusCmd_DataFormat_e::uint32_t_LE: d0_uint32_t = (uint32_t)_data; break;
    case BusCmd_DataFormat_e::int8_t: d0_int8_t        = _data; break;
    case BusCmd_DataFormat_e::int16_t:
    case BusCmd_DataFormat_e::int16_t_LE: d0_int16_t = _data; break;
    case BusCmd_DataFormat_e::int24_t:
    case BusCmd_DataFormat_e::int32_t:
    case BusCmd_DataFormat_e::int24_t_LE:
    case BusCmd_DataFormat_e::int32_t_LE: d0_int32_t = _data; break;
    case BusCmd_DataFormat_e::bytes:
    case BusCmd_DataFormat_e::words:
    #if FEATURE_BUSCMD_STRING
    case BusCmd_DataFormat_e::string:
    #endif // if FEATURE_BUSCMD_STRING
      break;
  }
}

/**
 * Destructor BusCmd_Command_struct
 */
BusCmd_Command_struct::~BusCmd_Command_struct() {
  data_b.clear();
  data_w.clear();
  calculation.clear();
}

int64_t BusCmd_Command_struct::getIntValue() {
  int64_t data{};

  switch (format) {
    case BusCmd_DataFormat_e::undefined: break;
    case BusCmd_DataFormat_e::uint8_t: data = d0_uint8_t; break;
    case BusCmd_DataFormat_e::uint16_t:
    case BusCmd_DataFormat_e::uint16_t_LE: data = d0_uint16_t; break;
    case BusCmd_DataFormat_e::uint24_t:
    case BusCmd_DataFormat_e::uint32_t:
    case BusCmd_DataFormat_e::uint24_t_LE:
    case BusCmd_DataFormat_e::uint32_t_LE: data = d0_uint32_t; break;
    case BusCmd_DataFormat_e::int8_t: data      = d0_int8_t; break;
    case BusCmd_DataFormat_e::int16_t:
    case BusCmd_DataFormat_e::int16_t_LE: data = d0_int16_t; break;
    case BusCmd_DataFormat_e::int24_t:
    case BusCmd_DataFormat_e::int32_t:
    case BusCmd_DataFormat_e::int24_t_LE:
    case BusCmd_DataFormat_e::int32_t_LE: data = d0_int32_t; break;
    case BusCmd_DataFormat_e::bytes:
    case BusCmd_DataFormat_e::words:
    #if FEATURE_BUSCMD_STRING
    case BusCmd_DataFormat_e::string:
    #endif // if FEATURE_BUSCMD_STRING
      break;
  }
  return data;
}

String BusCmd_Command_struct::getHexValue(const bool withPrefix) {
  return withPrefix ? concat(F("0x"), getHexValue()) : getHexValue();
}

String BusCmd_Command_struct::getHexValue() {
  uint64_t data64 = getUIntValue();

  if (BusCmd_DataFormat_e::bytes == format) {
    return formatToHex_array(&data_b[0], data_b.size());
  } else if (BusCmd_DataFormat_e::words == format) {
    return formatToHex_wordarray(&data_w[0], data_w.size());
    #if FEATURE_BUSCMD_STRING
  } else if (BusCmd_DataFormat_e::string == format) {
    return EMPTY_STRING;
    #endif // if FEATURE_BUSCMD_STRING
  }
  return formatToHex(data64);
}

#if FEATURE_BUSCMD_STRING
String BusCmd_Command_struct::getString() {
  if (BusCmd_DataFormat_e::string == format) {
    return variable;
  }
  return EMPTY_STRING;
}

#endif // if FEATURE_BUSCMD_STRING

#ifndef LIMIT_BUILD_SIZE
String BusCmd_Command_struct::toString() {
  char cmd[8]{};
  char cmdS[3]{};
  char fmt[6]{};

  GetTextIndexed(cmd,  sizeof(cmd),  static_cast<uint32_t>(command), BusCmd_commandsLong);
  GetTextIndexed(cmdS, sizeof(cmdS), static_cast<uint32_t>(command), BusCmd_commands);
  GetTextIndexed(fmt,  sizeof(fmt),  static_cast<uint32_t>(format),  BusCmd_dataFormats);
  String data;

  if ((BusCmd_DataFormat_e::bytes == format) || (BusCmd_DataFormat_e::words == format)) {
    data = getHexValue(true);
  }

  int64_t val{};

  if ((BusCmd_Command_e::Delay == command) || (BusCmd_Command_e::Value == command)) {
    val = d0_uint32_t;
  } else {
    val = getIntValue();
  }
  String result = strformat(F("cmd: '%s' (%s), fmt: '%s', reg: 0x%x, data: %d (%s), len: %lu"),
                            cmd, cmdS, fmt, reg, static_cast<int32_t>(val), formatToHex(static_cast<uint32_t>(val)).c_str(), len);

  if (!data.isEmpty()) {
    result = concat(result, strformat(F(", data_b/w: %s"), data.c_str()));
  }

  if (!variable.isEmpty()) {
    result = concat(result, strformat(
                      # if FEATURE_BUSCMD_STRING
                      BusCmd_DataFormat_e::string == format ? F(", string: %s") :
                      # endif // if FEATURE_BUSCMD_STRING
                      F(", variable: %s"), variable.c_str()));
  }

  if (!calculation.isEmpty()) {
    result = concat(result, strformat(F(", calculation: %s"), calculation.c_str()));
  }
  return result;
}

#endif // ifndef LIMIT_BUILD_SIZE

/**
 * Constructor BusCmd_Buffer
 */
BusCmd_Buffer::BusCmd_Buffer(const String& name,
                             const String& line)
  :cacheName(name), commandSet(line) {}

/**
 *  Constructor BusCmd_Helper_struct
 */
BusCmd_Helper_struct::BusCmd_Helper_struct(IBusCmd_Handler*busCmd_Handler,
                                           taskIndex_t     taskIndex,
                                           int16_t         enPin,
                                           int16_t         rstPin,
                                           uint8_t         loopLimit)
  : _iBusCmd_Handler(busCmd_Handler), _taskIndex(taskIndex), _enPin(enPin), _rstPin(rstPin), _loopLimit(loopLimit) {}

/**
 *  Destructor BusCmd_Helper_struct
 */
BusCmd_Helper_struct::~BusCmd_Helper_struct() {
  _commandCache.clear();
  _commands.clear();
  _buffer.clear();
}

const __FlashStringHelper * BusCmd_Helper_struct::cacheSuffix(BusCmd_CommandSource_e source) {
  switch (source) {
    case BusCmd_CommandSource_e::PluginIdle:
    case BusCmd_CommandSource_e::PluginGetConfigVar:
    case BusCmd_CommandSource_e::PluginRead: return F("");
    case BusCmd_CommandSource_e::PluginOncePerSecond: return F("_1ps");
    case BusCmd_CommandSource_e::PluginTenPerSecond: return F("_10ps");
    case BusCmd_CommandSource_e::PluginFiftyPerSecond: return F("_50ps");
  }
  return F("");
}

void BusCmd_Helper_struct::setBuffer(uint8_t       index,
                                     const String& name,
                                     const String& line) {
  if (_buffer.size() < index + 1u) {
    _buffer.resize(max(index + 1, VARS_PER_TASK), BusCmd_Buffer(EMPTY_STRING, EMPTY_STRING));
  }
  _buffer[index] = BusCmd_Buffer(name, line);
}

/*********************************************************************************************
 * Handle I2C-command processing, optionally stored to command cache
 * Format: <cmd>.<dataformat>[.<len>][.<register>][.<data>];...
 ********************************************************************************************/
std::vector<BusCmd_Command_struct>BusCmd_Helper_struct::parseBusCmdCommands(const String& name,
                                                                            const String& line) {
  return parseBusCmdCommands(name, line, false);
}

std::vector<BusCmd_Command_struct>BusCmd_Helper_struct::parseBusCmdCommands(const String& name,
                                                                            const String& line,
                                                                            const bool    update) {
  std::vector<BusCmd_Command_struct> commands;

  if ((nullptr == _iBusCmd_Handler) || !_iBusCmd_Handler->init()) { // Handler is required and initialized
    return commands;
  }

  const String key = parseString(name, 1);
  String keyPostfix;

  if (!key.isEmpty() && (_commandCache.count(key) == 1) && !update) {
    commands = _commandCache.find(key)->second;

    if (loglevelActiveFor(LOG_LEVEL_INFO) && _showLog && ((BusCmd_CommandSource_e::PluginRead == _commandSource) ||
                                                          (BusCmd_CommandSource_e::PluginGetConfigVar == _commandSource))) {
      addLog(LOG_LEVEL_INFO, strformat(F("BUSCMD: Retrieve '%s' from cache with %d commands."), name.c_str(), commands.size()));
    }
  }

  if (!line.isEmpty() && ((commands.empty()) || update) && ((BusCmd_CommandSource_e::PluginRead == _commandSource) ||
                                                            (BusCmd_CommandSource_e::PluginGetConfigVar == _commandSource))) {
    int evt = 1;

    while (evt > 0) {
      keyPostfix.clear();
      String evtAll = parseStringKeepCaseNoTrim(line, evt, BUSCMD_EVENT_SEPARATOR);

      if (evtAll.equalsIgnoreCase(F("1ps"))) { // PLUGIN_ONCE_A_SECOND
        ++evt;
        keyPostfix = BusCmd_Helper_struct::cacheSuffix(BusCmd_CommandSource_e::PluginOncePerSecond);
        _has1ps    = true;
      } else
      if (evtAll.equalsIgnoreCase(F("10ps"))) { // PLUGIN_TEN_PER_SECOND
        ++evt;
        keyPostfix = BusCmd_Helper_struct::cacheSuffix(BusCmd_CommandSource_e::PluginTenPerSecond);
        _has10ps   = true;
      } else
      if (evtAll.equalsIgnoreCase(F("50ps"))) { // PLUGIN_FIFTY_PER_SECOND
        ++evt;
        keyPostfix = BusCmd_Helper_struct::cacheSuffix(BusCmd_CommandSource_e::PluginFiftyPerSecond);
        _has50ps   = true;
      }

      if (!keyPostfix.isEmpty()) { // Next part is I2C command sequence
        evtAll = parseStringKeepCaseNoTrim(line, evt, BUSCMD_EVENT_SEPARATOR);
      }

      if (!evtAll.isEmpty()) {
        ++evt;
        commands.clear();
      } else {
        evt = 0;
      }

      // parse line
      int idx = 1;

      while (idx > 0 && evt > 0) {
        const String cmdAll = parseStringKeepCaseNoTrim(evtAll, idx, BUSCMD_COMMAND_SEPARATOR);
        bool addCmd         = true;

        if (!cmdAll.isEmpty()) {
          std::vector<String> args;

          uint8_t i    = 1;
          String  arg0 = parseStringKeepCaseNoTrim(cmdAll, i, BUSCMD_ARGUMENT_SEPARATOR);

          while (i < 5 || !arg0.isEmpty()) { // Read at least 4 arguments
            args.push_back(arg0);
            ++i;
            arg0 = parseStringKeepCaseNoTrim(cmdAll, i, BUSCMD_ARGUMENT_SEPARATOR);
          }

          #ifndef LIMIT_BUILD_SIZE

          if (loglevelActiveFor(LOG_LEVEL_INFO) && _showLog && ((BusCmd_CommandSource_e::PluginRead == _commandSource) ||
                                                                (BusCmd_CommandSource_e::PluginGetConfigVar == _commandSource))) {
            addLog(LOG_LEVEL_INFO, strformat(F("BUSCMD: Arguments parsed: %d (%s)"), args.size(), cmdAll.c_str()));
          }
          #endif // ifndef LIMIT_BUILD_SIZE

          args[0].toLowerCase();
          String sFmt = args[1];
          sFmt.toLowerCase();
          const int arg2i = GetCommandCode(sFmt.c_str(), BusCmd_dataFormats);
          int arg1i       = GetCommandCode(args[0].c_str(), BusCmd_commands);

          if (arg1i < 0) {
            arg1i = GetCommandCode(args[0].c_str(), BusCmd_commandsLong);
          }
          uint8_t arg = 2;

          uint16_t reg            = 0;
          uint32_t len            = 0;
          BusCmd_DataFormat_e fmt = BusCmd_DataFormat_e::uint8_t;
          String calculation;
          String variable;

          if (arg2i > -1) {
            fmt = static_cast<BusCmd_DataFormat_e>(arg2i);
          }

          if ((BusCmd_DataFormat_e::bytes == fmt) || (BusCmd_DataFormat_e::words == fmt)
              #if FEATURE_BUSCMD_STRING
              || (BusCmd_DataFormat_e::string == fmt)
              #endif // if FEATURE_BUSCMD_STRING
              ) {
            if (!validUIntFromString(args[arg], len) && !args[arg].isEmpty()) {
              variable = args[arg];
              len      = std::numeric_limits<uint32_t>::max();
            }
            ++arg;
          }

          if (arg1i > -1) {
            BusCmd_Command_e cmd = static_cast<BusCmd_Command_e>(arg1i);
            int64_t val          = 0;
            validInt64FromString(args[arg], val);

            switch (cmd) {
              case BusCmd_Command_e::NOP: break;
              case BusCmd_Command_e::Read:  // get - g.<format>
                break;
              case BusCmd_Command_e::Write: // put - p.<format>.<value>

                #if FEATURE_BUSCMD_STRING

                if (BusCmd_DataFormat_e::string == fmt) {
                  variable = args[arg];
                }
                #endif // if FEATURE_BUSCMD_STRING
                break;
              case BusCmd_Command_e::Calculate: // calc - c.<calculation>
              case BusCmd_Command_e::If:        // if - i.<calculation>[.skip]
                fmt         = BusCmd_DataFormat_e::undefined;
                val         = 0;
                calculation = args[arg - 1];
                stripEscapeCharacters(calculation);

                if ((BusCmd_Command_e::If == cmd) && !args[arg].isEmpty()) {
                  validUIntFromString(args[arg], len); // destination, 0 = exit, 1.. skip n commands
                }
                break;
              case BusCmd_Command_e::Let:              // let - l.<variable>.<calculation>
              #if FEATURE_BUSCMD_STRING && FEATURE_STRING_VARIABLES
              case BusCmd_Command_e::LetStr:           // letstr - m.<variable>.<calculation>
              #endif // if FEATURE_BUSCMD_STRING && FEATURE_STRING_VARIABLES
                fmt         = BusCmd_DataFormat_e::undefined;
                val         = 0;
                variable    = args[arg - 1];
                calculation = args[arg];
                stripEscapeCharacters(variable);
                stripEscapeCharacters(calculation);
                break;
              case BusCmd_Command_e::Eval:  // eval - e
                fmt = BusCmd_DataFormat_e::undefined;
                break;
              case BusCmd_Command_e::Value: // value - v.<valueIndex>
              case BusCmd_Command_e::Delay: // delay - d.<ms>
              {
                fmt = BusCmd_DataFormat_e::undefined;
                const bool isInt = validInt64FromString(args[arg - 1], val);

                if ((BusCmd_Command_e::Value == cmd) && !isInt) {
                  val = findDeviceValueIndexByName(args[arg - 1], _taskIndex);
                }
                break;
              }
              case BusCmd_Command_e::RegisterRead:   // read - r.<format>.<reg>
              case BusCmd_Command_e::Register16Read: // read16 - s.<format>.<reg16>
                reg = val;
                val = 0;
                break;
              case BusCmd_Command_e::RegisterWrite:   // write - w.<format>.<reg>.<data>
              case BusCmd_Command_e::Register16Write: // write16 - t.<format>.<reg16>.<data>
                reg = val;
                val = 0;

                if (!((BusCmd_DataFormat_e::bytes == fmt) || (BusCmd_DataFormat_e::words == fmt)
                      #if FEATURE_BUSCMD_STRING
                      || (BusCmd_DataFormat_e::string == fmt)
                      #endif // if FEATURE_BUSCMD_STRING
                      )) {
                  ++arg;

                  validInt64FromString(args[arg], val);
                }

                #if FEATURE_BUSCMD_STRING

                if (BusCmd_DataFormat_e::string == fmt) {
                  ++arg;
                  variable = args[arg];
                }
                #endif // if FEATURE_BUSCMD_STRING
                break;
              case BusCmd_Command_e::EnableGPIO: // enable - l.<state>
              case BusCmd_Command_e::ResetGPIO:  // reset - z.<state>.<msec>
                fmt = BusCmd_DataFormat_e::undefined;
                val = 0;
                validInt64FromString(args[arg - 1], val);

                if ((val < 0) || (val > 1)) { // Range check
                  addCmd = false;
                } else {
                  reg = val;                  // State

                  if (BusCmd_Command_e::ResetGPIO == cmd) {
                    val = 0;

                    if (!validInt64FromString(args[arg], val) || (val < 0)) { // msec
                      addCmd = false;
                    }
                  }
                }
                break;
            }

            if (addCmd) {
              commands.push_back(BusCmd_Command_struct(cmd, fmt, reg, val, len, calculation, variable));

              if (((BusCmd_Command_e::Write == cmd) || (BusCmd_Command_e::RegisterWrite == cmd) ||
                   (BusCmd_Command_e::Register16Write == cmd)) &&
                  ((BusCmd_DataFormat_e::bytes == fmt) || (BusCmd_DataFormat_e::words == fmt))) {
                const size_t currentCommandsIdx = commands.size() - 1;

                while (arg < args.size()) {
                  uint32_t val = 0;

                  if (validUIntFromString(args[arg], val)) {
                    if (BusCmd_DataFormat_e::bytes == fmt) { // bytes
                      commands[currentCommandsIdx].data_b.push_back((uint8_t)val);
                    } else {                                 // words
                      commands[currentCommandsIdx].data_w.push_back((uint16_t)val);
                    }
                  }
                  ++arg;
                }
              }
            }
          }
          ++idx;   // next
        } else {
          idx = 0; // done
        }
      }

      if (evt > 0) {
        #ifndef LIMIT_BUILD_SIZE

        if (loglevelActiveFor(LOG_LEVEL_INFO) && _showLog) {
          for (auto it = commands.begin(); it != commands.end(); ++it) {
            addLog(LOG_LEVEL_INFO, strformat(F("BUSCMD: Parsing command: %s, name: %s"), it->toString().c_str(), name.c_str()));
          }
        }
        #endif // ifndef LIMIT_BUILD_SIZE

        if (!key.isEmpty()) {
          _commandCache[concat(key, keyPostfix)] = commands;

          if (loglevelActiveFor(LOG_LEVEL_INFO) && _showLog) {
            addLog(LOG_LEVEL_INFO, strformat(F("BUSCMD: Insert '%s%s' into cache with %d commands."),
                                             name.c_str(), keyPostfix.c_str(), commands.size()));
          }
        }
      }
    }
  }

  return commands;
}

/*********************************************************************************************
 * Execute I2C commands
 ********************************************************************************************/
bool BusCmd_Helper_struct::executeBusCmdCommands() {
  bool result = false;

  if ((nullptr == _iBusCmd_Handler) || !_iBusCmd_Handler->init()) {
    return result;
  }

  if (BusCmd_CommandState_e::Processing == _commandState) {
    _it = _commands.begin();
  }

  if ((BusCmd_CommandState_e::Processing == _commandState) &&
      (_taskIndex != INVALID_TASK_INDEX) &&
      (_varIndex != INVALID_TASKVAR_INDEX) &&
      !_valueIsSet) {
    _value = UserVar.getFloat(_taskIndex, _varIndex);
  }

  if (BusCmd_CommandState_e::WaitingForDelay == _commandState) { // Returning from a delay
    _commandState = BusCmd_CommandState_e::Processing;

    if (validGpio(_rstPin) && (BusCmd_Command_e::ResetGPIO == _lastCommand)) {
      DIRECT_pinWrite(_rstPin, _lastReg ? LOW : HIGH); // Revert ResetGPIO state
    }
  }
  uint32_t toSkip{};                                   // Skip nr of commands after if.<cond>.<skip>

  while (_it != _commands.end() && BusCmd_CommandState_e::Processing == _commandState) {
    bool wideReg = false;
    uint32_t du32{};
    uint16_t du16{};
    uint32_t _len = _it->len;

    if (_len == std::numeric_limits<uint32_t>::max()) {
      String _var(replacePluginValues(_it->variable)); // preprocessing
      const String _variable = parseTemplate(_var);
      ESPEASY_RULES_FLOAT_TYPE tmp{};

      if (Calculate(_variable, tmp) == CalculateReturnCode::OK) {
        _len = tmp;
      } else {
        _len = 0u;
      }
    }
    _lastCommand = _it->command; // We only need this for long ResetGPIO pulses
    _lastReg     = _it->reg;

    switch (_it->command) {
      case BusCmd_Command_e::NOP: break;
      case BusCmd_Command_e::Read:

        switch (_it->format) {
          case BusCmd_DataFormat_e::undefined: break;
          case BusCmd_DataFormat_e::uint8_t:
          case BusCmd_DataFormat_e::int8_t:
            _it->d0_uint8_t = _iBusCmd_Handler->read8u();
            break;
          case BusCmd_DataFormat_e::uint16_t:
          case BusCmd_DataFormat_e::int16_t:
            _it->d0_uint16_t = _iBusCmd_Handler->read16u();
            break;
          case BusCmd_DataFormat_e::uint24_t:
            _it->d0_uint32_t = _iBusCmd_Handler->read24u();
            break;
          case BusCmd_DataFormat_e::uint32_t: // Fall through
          case BusCmd_DataFormat_e::int32_t:
            _it->d0_uint32_t = _iBusCmd_Handler->read32u();
            break;
          case BusCmd_DataFormat_e::uint16_t_LE:
            du16             = _iBusCmd_Handler->read16u();
            _it->d0_uint16_t = (du16 << 8) | (du16 >> 8);
            break;
          case BusCmd_DataFormat_e::uint24_t_LE:
            du32             = _iBusCmd_Handler->read24u();
            _it->d0_uint32_t = ((du32 & 0xFF0000) >> 16) | (du32 & 0xFF00) | ((du32 & 0xFF) << 16);
            break;
          case BusCmd_DataFormat_e::uint32_t_LE:
            du32             = _iBusCmd_Handler->read32u();
            _it->d0_uint32_t = ((du32 & 0xFF000000) >> 24) | ((du32 & 0xFF0000) >> 16) | ((du32 & 0xFF00) << 8) | ((du32 & 0xFF) << 24);
            break;
          case BusCmd_DataFormat_e::int16_t_LE:
          {
            du16 = _iBusCmd_Handler->read16u();
            const bool isNeg = bitRead(du16, 15);
            du16            &= ((1 << 15) - 1);
            du16             = ((du16 << 8) | (du16 >> 8)) | (isNeg ? (1 << 15) : 0);
            _it->d0_uint16_t = du16;
            break;
          }
          case BusCmd_DataFormat_e::int24_t:
          {
            du32 = _iBusCmd_Handler->read24u();
            const bool isNeg = bitRead(du32, 23);
            du32            &= ((1 << 23) - 1);
            _it->d0_uint32_t = du32 | (isNeg ? (1 << 31) : 0);
            break;
          }
          case BusCmd_DataFormat_e::int24_t_LE:
          {
            du32 = _iBusCmd_Handler->read24u();
            const bool isNeg = bitRead(du32, 23);
            du32            &= ((1 << 23) - 1);
            _it->d0_uint32_t = (((du32 & 0xFF0000) >> 16) | (du32 & 0xFF00) | ((du32 & 0xFF) << 16))
                               | (isNeg ? (1 << 31) : 0);
            break;
          }
          case BusCmd_DataFormat_e::int32_t_LE:
          {
            du32 = _iBusCmd_Handler->read32u();
            const bool isNeg = bitRead(du32, 31);
            du32            &= ((uint32_t)(1 << 31u) - 1);
            _it->d0_uint32_t = (((du32 & 0xFF000000) >> 24) |
                                ((du32 & 0xFF0000) >> 16) |
                                ((du32 & 0xFF00) << 8) |
                                ((du32 & 0xFF) << 24))
                               | (isNeg ? (1 << 31) : 0);
            break;
          }
          case BusCmd_DataFormat_e::bytes:
            _it->data_b = _iBusCmd_Handler->read8uB(_len);
            break;
          case BusCmd_DataFormat_e::words:
            _it->data_w = _iBusCmd_Handler->read16uW(_len);
            break;
          #if FEATURE_BUSCMD_STRING
          case BusCmd_DataFormat_e::string:
            _it->variable = _iBusCmd_Handler->readString(_len);
            break;
          #endif // if FEATURE_BUSCMD_STRING
        }
        result = true;
        break;
      case BusCmd_Command_e::Write:

        switch (_it->format) {
          case BusCmd_DataFormat_e::undefined: break;
          case BusCmd_DataFormat_e::uint8_t:
          case BusCmd_DataFormat_e::int8_t:
            result = _iBusCmd_Handler->write8u(_it->d0_uint8_t);
            break;
          case BusCmd_DataFormat_e::uint16_t:
          case BusCmd_DataFormat_e::int16_t:
            result = _iBusCmd_Handler->write16u(_it->d0_uint16_t);
            break;
          case BusCmd_DataFormat_e::uint24_t:
            result = _iBusCmd_Handler->write24u(_it->d0_uint32_t);
            break;
          case BusCmd_DataFormat_e::uint32_t:
          case BusCmd_DataFormat_e::int32_t:
            result = _iBusCmd_Handler->write32u(_it->d0_uint32_t);
            break;
          case BusCmd_DataFormat_e::uint16_t_LE:
            result = _iBusCmd_Handler->write16u((_it->d0_uint16_t << 8) | (_it->d0_uint16_t >> 8));
            break;
          case BusCmd_DataFormat_e::uint24_t_LE:
            du32   = ((_it->d0_uint32_t & 0xFF0000) >> 16) | (_it->d0_uint32_t & 0xFF00) | ((_it->d0_uint32_t & 0xFF) << 16);
            result = _iBusCmd_Handler->write24u(du32);
            break;
          case BusCmd_DataFormat_e::uint32_t_LE:
            du32 = ((_it->d0_uint32_t & 0xFF000000) >> 24) | ((_it->d0_uint32_t & 0xFF0000) >> 16) |
                   (_it->d0_uint32_t & 0xFF00) | ((_it->d0_uint32_t & 0xFF) << 16);
            result = _iBusCmd_Handler->write32u(du32);
            break;
          case BusCmd_DataFormat_e::int16_t_LE:
          {
            du16 = _it->d0_uint16_t;
            const bool isNeg = bitRead(du16, 15);
            du16  &= ((1 << 15) - 1);
            du16   = ((du16 << 8) | (du16 >> 8)) | (isNeg ? (1 << 15) : 0);
            result = _iBusCmd_Handler->write16u(du16);
            break;
          }
          case BusCmd_DataFormat_e::int24_t:
            result = _iBusCmd_Handler->write24u(_it->d0_uint32_t);
            break;
          case BusCmd_DataFormat_e::int24_t_LE:
          {
            du32 = _it->d0_uint32_t;
            const bool isNeg = bitRead(du32, 31);
            du32 &= ((1 << 23) - 1);
            du32  = (((du32 & 0xFF0000) >> 16) | (du32 & 0xFF00) | ((du32 & 0xFF) << 16))
                    | isNeg ? (1 << 23) : 0;
            result = _iBusCmd_Handler->write24u(du32);
            break;
          }
          case BusCmd_DataFormat_e::int32_t_LE:
          {
            du32 = _it->d0_uint32_t;
            const bool isNeg = bitRead(du32, 31);
            du32 = (((du32 & 0xFF000000) >> 24) |
                    ((du32 & 0xFF0000) >> 16) |
                    (du32 & 0xFF00) |
                    ((du32 & 0xFF) << 16))
                   | (isNeg ? (1 << 31) : 0);
            result = _iBusCmd_Handler->write32u(du32);
            break;
          }
          case BusCmd_DataFormat_e::bytes:
            result = _iBusCmd_Handler->write8uB(_it->data_b);
            break;
          case BusCmd_DataFormat_e::words:
            result = _iBusCmd_Handler->write16uW(_it->data_w);
            break;
          #if FEATURE_BUSCMD_STRING
          case BusCmd_DataFormat_e::string:
            result = _iBusCmd_Handler->writeString(_it->variable);
            break;
          #endif // if FEATURE_BUSCMD_STRING
        }
        break;
      case BusCmd_Command_e::Register16Read:
        wideReg = true; // fall through
      case BusCmd_Command_e::RegisterRead:

        switch (_it->format) {
          case BusCmd_DataFormat_e::undefined: break;
          case BusCmd_DataFormat_e::uint8_t:
          case BusCmd_DataFormat_e::int8_t:
            _it->d0_uint8_t = _iBusCmd_Handler->read8uREG(_it->reg, wideReg);
            break;
          case BusCmd_DataFormat_e::uint16_t:
          case BusCmd_DataFormat_e::int16_t:
            _it->d0_uint16_t = _iBusCmd_Handler->read16uREG(_it->reg, wideReg);
            break;
          case BusCmd_DataFormat_e::uint24_t:
            _it->d0_uint32_t = _iBusCmd_Handler->read24uREG(_it->reg, wideReg);
            break;
          case BusCmd_DataFormat_e::uint32_t: // Fall through
          case BusCmd_DataFormat_e::int32_t:
            _it->d0_uint32_t = _iBusCmd_Handler->read32uREG(_it->reg, wideReg);
            break;
          case BusCmd_DataFormat_e::uint16_t_LE:
            du16             = _iBusCmd_Handler->read16uREG(_it->reg, wideReg);
            _it->d0_uint16_t = (du16 << 8) | (du16 >> 8);
            break;
          case BusCmd_DataFormat_e::uint24_t_LE:
            du32             = _iBusCmd_Handler->read24uREG(_it->reg, wideReg);
            _it->d0_uint32_t = ((du32 & 0xFF0000) >> 16) | (du32 & 0xFF00) | ((du32 & 0xFF) << 16);
            break;
          case BusCmd_DataFormat_e::uint32_t_LE:
            du32             = _iBusCmd_Handler->read32uREG(_it->reg, wideReg);
            _it->d0_uint32_t = ((du32 & 0xFF000000) >> 24) | ((du32 & 0xFF0000) >> 16) | ((du32 & 0xFF00) << 8) | ((du32 & 0xFF) << 24);
            break;
          case BusCmd_DataFormat_e::int16_t_LE:
          {
            du16 = _iBusCmd_Handler->read16uREG(_it->reg, wideReg);
            const bool isNeg = bitRead(du16, 15);
            du16            &= ((1 << 15) - 1);
            du16             = ((du16 << 8) | (du16 >> 8)) | (isNeg ? (1 << 15) : 0);
            _it->d0_uint16_t = du16;
            break;
          }
          case BusCmd_DataFormat_e::int24_t:
          {
            du32 = _iBusCmd_Handler->read24uREG(_it->reg, wideReg);
            const bool isNeg = bitRead(du32, 23);
            du32            &= ((1 << 23) - 1);
            _it->d0_uint32_t = du32 | (isNeg ? (1 << 31) : 0);
            break;
          }
          case BusCmd_DataFormat_e::int24_t_LE:
          {
            du32 = _iBusCmd_Handler->read24uREG(_it->reg, wideReg);
            const bool isNeg = bitRead(du32, 23);
            du32            &= ((1 << 23) - 1);
            _it->d0_uint32_t = (((du32 & 0xFF0000) >> 16) | (du32 & 0xFF00) | ((du32 & 0xFF) << 16))
                               | (isNeg ? (1 << 31) : 0);
            break;
          }
          case BusCmd_DataFormat_e::int32_t_LE:
          {
            du32 = _iBusCmd_Handler->read32uREG(_it->reg, wideReg);
            const bool isNeg = bitRead(du32, 31);
            du32            &= ((uint32_t)(1 << 31u) - 1);
            _it->d0_uint32_t = (((du32 & 0xFF000000) >> 24) |
                                ((du32 & 0xFF0000) >> 16) |
                                ((du32 & 0xFF00) << 8) |
                                ((du32 & 0xFF) << 24))
                               | (isNeg ? (1 << 31) : 0);
            break;
          }
          case BusCmd_DataFormat_e::bytes:
            _it->data_b = _iBusCmd_Handler->read8uBREG(_it->reg, _len, wideReg);
            break;
          case BusCmd_DataFormat_e::words:
            _it->data_w = _iBusCmd_Handler->read16uWREG(_it->reg, _len, wideReg);
            break;
          #if FEATURE_BUSCMD_STRING
          case BusCmd_DataFormat_e::string:
            _it->variable = _iBusCmd_Handler->readStringREG(_it->reg, _len, wideReg);
            break;
          #endif // if FEATURE_BUSCMD_STRING
        }
        result = true;
        break;
      case BusCmd_Command_e::Register16Write:
        wideReg = true; // fall through
      case BusCmd_Command_e::RegisterWrite:

        switch (_it->format) {
          case BusCmd_DataFormat_e::undefined: break;
          case BusCmd_DataFormat_e::uint8_t:
          case BusCmd_DataFormat_e::int8_t:
            result = _iBusCmd_Handler->write8uREG(_it->reg, _it->d0_uint8_t, wideReg);
            break;
          case BusCmd_DataFormat_e::uint16_t:
          case BusCmd_DataFormat_e::int16_t:
            result = _iBusCmd_Handler->write16uREG(_it->reg, _it->d0_uint16_t, wideReg);
            break;
          case BusCmd_DataFormat_e::uint24_t:
            result = _iBusCmd_Handler->write24uREG(_it->reg, _it->d0_uint32_t, wideReg);
            break;
          case BusCmd_DataFormat_e::uint32_t:
          case BusCmd_DataFormat_e::int32_t:
            result = _iBusCmd_Handler->write32uREG(_it->reg, _it->d0_uint32_t, wideReg);
            break;
          case BusCmd_DataFormat_e::uint16_t_LE:
            result = _iBusCmd_Handler->write16uREG(_it->reg, (_it->d0_uint16_t << 8) | (_it->d0_uint16_t >> 8), wideReg);
            break;
          case BusCmd_DataFormat_e::uint24_t_LE:
            du32   = ((_it->d0_uint32_t & 0xFF0000) >> 16) | (_it->d0_uint32_t & 0xFF00) | ((_it->d0_uint32_t & 0xFF) << 16);
            result = _iBusCmd_Handler->write24uREG(_it->reg, du32, wideReg);
            break;
          case BusCmd_DataFormat_e::uint32_t_LE:
            du32 = ((_it->d0_uint32_t & 0xFF000000) >> 24) | ((_it->d0_uint32_t & 0xFF0000) >> 16) |
                   (_it->d0_uint32_t & 0xFF00) | ((_it->d0_uint32_t & 0xFF) << 16);
            result = _iBusCmd_Handler->write32uREG(_it->reg, du32, wideReg);
            break;
          case BusCmd_DataFormat_e::int16_t_LE:
          {
            du16 = _it->d0_uint16_t;
            const bool isNeg = bitRead(du16, 15);
            du16  &= ((1 << 15) - 1);
            du16   = ((du16 << 8) | (du16 >> 8)) | (isNeg ? (1 << 15) : 0);
            result = _iBusCmd_Handler->write16uREG(_it->reg, du16, wideReg);
            break;
          }
          case BusCmd_DataFormat_e::int24_t:
            result = _iBusCmd_Handler->write24uREG(_it->reg, _it->d0_uint32_t, wideReg);
            break;
          case BusCmd_DataFormat_e::int24_t_LE:
          {
            du32 = _it->d0_uint32_t;
            const bool isNeg = bitRead(du32, 31);
            du32 &= ((1 << 23) - 1);
            du32  = (((du32 & 0xFF0000) >> 16) | (du32 & 0xFF00) | ((du32 & 0xFF) << 16))
                    | isNeg ? (1 << 23) : 0;
            result = _iBusCmd_Handler->write24uREG(_it->reg, du32, wideReg);
            break;
          }
          case BusCmd_DataFormat_e::int32_t_LE:
          {
            du32 = _it->d0_uint32_t;
            const bool isNeg = bitRead(du32, 31);
            du32 = (((du32 & 0xFF000000) >> 24) |
                    ((du32 & 0xFF0000) >> 16) |
                    (du32 & 0xFF00) |
                    ((du32 & 0xFF) << 16))
                   | (isNeg ? (1 << 31) : 0);
            result = _iBusCmd_Handler->write32uREG(_it->reg, du32, wideReg);
            break;
          }
          case BusCmd_DataFormat_e::bytes:
            result = _iBusCmd_Handler->write8uBREG(_it->reg, _it->data_b, wideReg);
            break;
          case BusCmd_DataFormat_e::words:
            result = _iBusCmd_Handler->write16uWREG(_it->reg, _it->data_w, wideReg);
            break;
          #if FEATURE_BUSCMD_STRING
          case BusCmd_DataFormat_e::string:
            result = _iBusCmd_Handler->writeStringReg(_it->reg, _it->variable, wideReg);
            break;
          #endif // if FEATURE_BUSCMD_STRING
        }
        break;
      case BusCmd_Command_e::Value:

        if ((_it->d0_uint32_t > 0) && (_it->d0_uint32_t <= VARS_PER_TASK)) {
          UserVar.setFloat(_taskIndex, _it->d0_uint32_t - 1, _value);
          _valueIsSet = true;
        }
        result = true;
        break;
      case BusCmd_Command_e::Calculate:
      case BusCmd_Command_e::If:
      case BusCmd_Command_e::Let:
      #if FEATURE_BUSCMD_STRING && FEATURE_STRING_VARIABLES
      case BusCmd_Command_e::LetStr:
      #endif // if FEATURE_BUSCMD_STRING && FEATURE_STRING_VARIABLES

        result = true;

        if (!_it->calculation.isEmpty()) {
          String toCalc(replacePluginValues(_it->calculation));
          const String newCalc = parseTemplate(toCalc); // Process like rules
          ESPEASY_RULES_FLOAT_TYPE tmp{};

          #if FEATURE_BUSCMD_STRING && FEATURE_STRING_VARIABLES

          if ((BusCmd_Command_e::LetStr == _it->command)) {
            String toVar(replacePluginValues(_it->variable));
            const String newVar = parseTemplate(toVar); // Process like rules

            if (!newVar.isEmpty()) {
              setCustomStringVar(newVar, newCalc);      // Assign string value to a string variable

              if (loglevelActiveFor(LOG_LEVEL_INFO) && _showLog && ((BusCmd_CommandSource_e::PluginRead == _commandSource) ||
                                                                    (BusCmd_CommandSource_e::PluginGetConfigVar == _commandSource))) {
                addLog(LOG_LEVEL_INFO, strformat(F("BUSCMD: Calculation: %s -> LetStr,%s,%s"),
                                                 toCalc.c_str(), newVar.c_str(), newCalc.c_str()));
              }
            }
          } else
          #endif // if FEATURE_BUSCMD_STRING && FEATURE_STRING_VARIABLES

          if (Calculate(newCalc, tmp) == CalculateReturnCode::OK) {
            if (loglevelActiveFor(LOG_LEVEL_INFO) && _showLog && ((BusCmd_CommandSource_e::PluginRead == _commandSource) ||
                                                                  (BusCmd_CommandSource_e::PluginGetConfigVar == _commandSource))) {
              addLog(LOG_LEVEL_INFO, strformat(F("BUSCMD: Calculation: %s, result: %s"), toCalc.c_str(), doubleToString(tmp).c_str()));
            }

            if (BusCmd_Command_e::If == _it->command) {
              if (essentiallyZero(tmp)) { // 0 = false => cancel execution
                if (0 == _it->len) {
                  _commandState = BusCmd_CommandState_e::ConditionalExit;
                  result        = false;  // PLUGIN_READ failed
                } else {
                  // skip <len> commands
                  toSkip = _it->len;
                }
              }
            } else if (BusCmd_Command_e::Let == _it->command) {
              String toVar(replacePluginValues(_it->variable));
              const String newVar = parseTemplate(toVar); // Process like rules

              if (!newVar.isEmpty() && ExtraTaskSettings.checkInvalidCharInNames(newVar.c_str())) {
                setCustomFloatVar(newVar, tmp);
              } else {
                result = false;
              }
            } else {
              _value = tmp;
            }
          }
        }
        break;
      case BusCmd_Command_e::Eval:

        if (_it != _commands.begin()) { // No previous entry
          _evalCommand = _it;
          --_evalCommand;               // Previous command holds the desired value
          _evalIsSet = true;
        }
        result = true;
        break;
      case BusCmd_Command_e::Delay:
      {
        const uint32_t mx = ((BusCmd_CommandSource_e::PluginOncePerSecond == _commandSource) ||
                             (BusCmd_CommandSource_e::PluginTenPerSecond == _commandSource) ||
                             (BusCmd_CommandSource_e::PluginFiftyPerSecond == _commandSource) ||
                             (BusCmd_CommandSource_e::PluginGetConfigVar == _commandSource)) ? 10u : 500u;
        const uint32_t ms = min(_it->d0_uint32_t, mx); // Reasonable limit, max 10 msec for repeating events

        if (ms <= 10) {
          delay(ms);
          result = true;
        } else {
          _commandState = BusCmd_CommandState_e::StartingDelay;
          Scheduler.schedule_task_device_timer(_taskIndex, millis() + ms);
          result = false; // Don't publish result yet
        }
        break;
      }
      case BusCmd_Command_e::EnableGPIO:

        if (validGpio(_enPin)) {
          DIRECT_pinWrite(_enPin, _it->reg ? HIGH : LOW);
          result = true;
        }
        break;
      case BusCmd_Command_e::ResetGPIO:

        if (validGpio(_rstPin)) {
          const uint32_t ms = min(_it->d0_uint32_t, static_cast<uint32_t>(500u)); // Reasonable limit

          DIRECT_pinWrite(_rstPin, _it->reg ? HIGH : LOW);                        // Set ResetGPIO state

          if (ms <= 10) {
            delay(ms);
            DIRECT_pinWrite(_rstPin, _it->reg ? LOW : HIGH); // Revert ResetGPIO state
            result = true;
          } else {
            _commandState = BusCmd_CommandState_e::StartingDelay;
            Scheduler.schedule_task_device_timer(_taskIndex, millis() + ms);
            result = false; // Don't publish result yet
          }
        }
        break;
    }

    if ((BusCmd_CommandState_e::Processing == _commandState) && !_valueIsSet) {
      switch (_it->format) {
        case BusCmd_DataFormat_e::undefined: break;
        case BusCmd_DataFormat_e::uint8_t:
        case BusCmd_DataFormat_e::uint16_t:
        case BusCmd_DataFormat_e::uint16_t_LE:
        case BusCmd_DataFormat_e::uint24_t:
        case BusCmd_DataFormat_e::uint32_t:
        case BusCmd_DataFormat_e::uint24_t_LE:
        case BusCmd_DataFormat_e::uint32_t_LE:
          _value = _it->getUIntValue();
          break;
        case BusCmd_DataFormat_e::int8_t:
        case BusCmd_DataFormat_e::int16_t:
        case BusCmd_DataFormat_e::int16_t_LE:
        case BusCmd_DataFormat_e::int24_t:
        case BusCmd_DataFormat_e::int32_t:
        case BusCmd_DataFormat_e::int24_t_LE:
        case BusCmd_DataFormat_e::int32_t_LE:
          _value = _it->getIntValue();
          break;
        case BusCmd_DataFormat_e::bytes:
        case BusCmd_DataFormat_e::words:
          break;
        #if FEATURE_BUSCMD_STRING
        case BusCmd_DataFormat_e::string:
          # if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
          validDoubleFromString(_it->variable, _value);
          # else // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
          validFloatFromString(_it->variable, _value);
          # endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
          break;
        #endif // if FEATURE_BUSCMD_STRING
      }
    }
#ifndef LIMIT_BUILD_SIZE
    if (loglevelActiveFor(LOG_LEVEL_INFO) && _showLog && ((BusCmd_CommandSource_e::PluginRead == _commandSource) ||
                                                          (BusCmd_CommandSource_e::PluginGetConfigVar == _commandSource))) {
      #if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
      const String valStr = doubleToString(_value, 2, true);
      #else // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
      const String valStr = toString(_value, 2, true);
      #endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE

      addLog(LOG_LEVEL_INFO, strformat(F("BUSCMD: Executing command: %s, value[%d]:(%c): %s"),
                                       _it->toString().c_str(), _varIndex, _valueIsSet ? 't' : 'f', valStr.c_str()));
    }
#endif
    ++_it; // Next command

    while (toSkip > 0 && _it != _commands.end()) {
      ++_it;
      --toSkip; // Skip commands, forward-only, for now
    }
  }

  if ((BusCmd_CommandState_e::Processing == _commandState) &&
      (_taskIndex != INVALID_TASK_INDEX) &&
      (_varIndex != INVALID_TASKVAR_INDEX) &&
      !_valueIsSet &&
      (BusCmd_CommandSource_e::PluginRead == _commandSource)) {
    UserVar.setFloat(_taskIndex, _varIndex, _value);
  }

  return result;
}

String BusCmd_Helper_struct::replacePluginValues(const String& inVar) {
  String result(inVar);

  result.replace(F("%pvalue%"), toString(_value)); // %pvalue%

  if (_evalIsSet) {
    result.replace(F("%value%"),
                   #if FEATURE_BUSCMD_STRING
                   BusCmd_DataFormat_e::string == _evalCommand->format ? _evalCommand->getString() :
                   #endif // if FEATURE_BUSCMD_STRING
                   toString(_evalCommand->getIntValue()));     // %value%
    result.replace(F("%h%"), _evalCommand->getHexValue(true)); // %h%

    if (BusCmd_DataFormat_e::bytes == _evalCommand->format) {
      if (result.indexOf(F("%b")) > -1) {
        for (uint8_t i = 0; i < _evalCommand->data_b.size(); ++i) {
          result.replace(strformat(F("%%b%d%%"), i),  String(_evalCommand->data_b[i]));                   // %b<n>%
          result.replace(strformat(F("%%bx%d%%"), i), formatToHex_no_prefix(_evalCommand->data_b[i], 2)); // %bx<n>%
        }
      }
    } else if (BusCmd_DataFormat_e::words == _evalCommand->format) {
      if (result.indexOf(F("%w")) > -1) {
        for (uint8_t i = 0; i < _evalCommand->data_w.size(); ++i) {
          result.replace(strformat(F("%%w%d%%"), i),  String(_evalCommand->data_w[i]));                   // %w<n>%
          result.replace(strformat(F("%%wx%d%%"), i), formatToHex_no_prefix(_evalCommand->data_w[i], 4)); // %wx<n>%
        }
      }
    }
  }

  return result;
}

bool BusCmd_Helper_struct::parseAndExecute(BusCmd_CommandSource_e source,
                                           const String         & line,
                                           const String         & logFormat) {
  bool result = false;

  _commandSource = source;
  _commands      = parseBusCmdCommands(EMPTY_STRING, line);

  if (!_commands.empty()) {
    if (loglevelActiveFor(LOG_LEVEL_INFO) && _showLog) {
      addLog(LOG_LEVEL_INFO, strformat(logFormat, _commands.size()));
    }
    _commandState = BusCmd_CommandState_e::Processing;
    result        = executeBusCmdCommands();
    _commands.clear();
    _commandState = BusCmd_CommandState_e::Idle;
  }
  _commandSource = BusCmd_CommandSource_e::PluginIdle;
  return result;
}

/**
 * Event handlers
 */
bool BusCmd_Helper_struct::plugin_once_a_second(struct EventStruct *event) {
  if (((BusCmd_CommandSource_e::PluginIdle != _commandSource) &&
       (BusCmd_CommandSource_e::PluginOncePerSecond != _commandSource)) ||
      !_has1ps) {
    return false;
  }
  _commandSource = BusCmd_CommandSource_e::PluginOncePerSecond;
  return processCommands(event);
}

bool BusCmd_Helper_struct::plugin_ten_per_second(struct EventStruct *event) {
  if (((BusCmd_CommandSource_e::PluginIdle != _commandSource) &&
       (BusCmd_CommandSource_e::PluginTenPerSecond != _commandSource)) ||
      !_has10ps) {
    return false;
  }
  _commandSource = BusCmd_CommandSource_e::PluginTenPerSecond;
  return processCommands(event);
}

bool BusCmd_Helper_struct::plugin_fifty_per_second(struct EventStruct *event) {
  if (((BusCmd_CommandSource_e::PluginIdle != _commandSource) &&
       (BusCmd_CommandSource_e::PluginFiftyPerSecond != _commandSource)) ||
      !_has50ps) {
    return false;
  }
  _commandSource = BusCmd_CommandSource_e::PluginFiftyPerSecond;
  return processCommands(event);
}

bool BusCmd_Helper_struct::plugin_read(struct EventStruct *event) {
  if ((BusCmd_CommandSource_e::PluginIdle != _commandSource) &&
      (BusCmd_CommandSource_e::PluginRead != _commandSource)) {
    return false;
  }
  _commandSource = BusCmd_CommandSource_e::PluginRead;
  return processCommands(event);
}

bool BusCmd_Helper_struct::processCommands(struct EventStruct *event) {
  bool result       = true;
  BusCmd_Buffer buf = BusCmd_Buffer(EMPTY_STRING, EMPTY_STRING);

  if (BusCmd_CommandState_e::Idle == _commandState) {
    _commandState = BusCmd_CommandState_e::Processing;
    _loop         = 0;
    _loopMax      = _loopLimit;
    _value        = 0.0;
    _valueIsSet   = false; // init

    // loadStrings(event);
    buf = _buffer[_loop];

    if (!(buf.cacheName.isEmpty() && buf.commandSet.isEmpty())) {
      const String cacheName = buf.cacheName.isEmpty()
                               ? EMPTY_STRING
                               : concat(buf.cacheName, BusCmd_Helper_struct::cacheSuffix(_commandSource));

      _commands = parseBusCmdCommands(cacheName, // PluginOnce/Ten/Fifty-PerSecond must come from cache
                                      (BusCmd_CommandSource_e::PluginRead == _commandSource) ||
                                      (BusCmd_CommandSource_e::PluginGetConfigVar == _commandSource) ? buf.commandSet : EMPTY_STRING);
    }
    _varIndex = _loop;
  }

  while (BusCmd_CommandState_e::StartingDelay != _commandState && BusCmd_CommandState_e::ConditionalExit != _commandState &&
         _loop < _loopMax) {
    if (!_commands.empty()) {
      result &= executeBusCmdCommands();
    }

    if (BusCmd_CommandState_e::Processing == _commandState) {
      ++_loop;
      buf = _buffer[_loop];
      const String cacheName = buf.cacheName.isEmpty()
                               ? EMPTY_STRING
                               : concat(buf.cacheName, BusCmd_Helper_struct::cacheSuffix(_commandSource));

      _commands = parseBusCmdCommands(cacheName, // PluginOnce/Ten/Fifty-PerSecond must come from cache
                                      (BusCmd_CommandSource_e::PluginRead == _commandSource) ||
                                      (BusCmd_CommandSource_e::PluginGetConfigVar == _commandSource) ? buf.commandSet : EMPTY_STRING);
      _varIndex = _loop;
    }
  }

  if ((BusCmd_CommandState_e::Processing == _commandState) || (BusCmd_CommandState_e::ConditionalExit == _commandState)) {
    if (_valueIsSet && (BusCmd_CommandSource_e::PluginRead != _commandSource) && (BusCmd_CommandState_e::ConditionalExit != _commandState)) {
      sendData(event); // Publish changes on succesful Value command from another action than PLUGIN_READ
    }
    _commandState = BusCmd_CommandState_e::Idle;
    _commands.clear();
    _valueIsSet    = false; // reset afterward
    _commandSource = BusCmd_CommandSource_e::PluginIdle;
  } else if (BusCmd_CommandState_e::StartingDelay == _commandState) {
    _commandState = BusCmd_CommandState_e::WaitingForDelay;
  }

  return result;
}

void BusCmd_Helper_struct::setCommands(std::vector<BusCmd_Command_struct>commands,
                                       taskVarIndex_t                    taskVarIndex,
                                       uint8_t                           loopStart,
                                       uint8_t                           loopMax,
                                       BusCmd_CommandState_e             commandState) {
  _commands     = commands;
  _varIndex     = taskVarIndex;
  _loop         = loopStart;
  _loopMax      = loopMax;
  _commandState = commandState;
}

#ifndef LIMIT_BUILD_SIZE
bool BusCmd_Helper_struct::plugin_get_config(struct EventStruct *event,
                                             String            & string) {
  const String cmd = parseString(string, 1);

  if (cmd.isEmpty() || (string.indexOf(',') == -1)) { return false; }

  if (equals(cmd, F("log"))) { // Get current logging state
    string = String(_showLog ? 1 : 0);
    return true;
  }

  if (BusCmd_CommandSource_e::PluginIdle != _commandSource) { // Execute commands when not busy
    return false;
  }
  bool success = false;
  std::vector<BusCmd_Command_struct> cmds;
  taskVarIndex_t taskVar = INVALID_TASKVAR_INDEX;

  _commandSource = BusCmd_CommandSource_e::PluginGetConfigVar;
  cmds           = parseBusCmdCommands(EMPTY_STRING, parseStringKeepCaseNoTrim(string, 2));

  if (!cmds.empty() && (BusCmd_CommandState_e::Idle == _commandState)) {
    setCommands(cmds,
                taskVar,
                0,
                1, // Process single entry
                BusCmd_CommandState_e::Processing);
    success = processCommands(event);

    if (success) {
      string = strformat(F("%g"), _value);
    }
  }
  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) { // Not logging log value or early return
    addLog(LOG_LEVEL_DEBUG, strformat(F("BUSCMD: GET_CONFIG, result: %s, success: %d, value: %g, cmds: %d"),
                                      string.c_str(),
                                      success,
                                      _value,
                                      cmds.size()));
  }
  # endif // ifndef BUILD_NO_DEBUG

  return success;
}

#endif // ifndef LIMIT_BUILD_SIZE
