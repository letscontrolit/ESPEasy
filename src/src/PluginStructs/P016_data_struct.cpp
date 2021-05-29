#include "../PluginStructs/P016_data_struct.h"

#ifdef USES_P016

# include "../Commands/InternalCommands.h"
# include "../Helpers/ESPEasy_Storage.h"
# include <IRutils.h>

P016_data_struct::P016_data_struct() {}

void P016_data_struct::init(struct EventStruct *event, uint16_t CmdInhibitTime) {
  loadCommandLines(event);
  iCmdInhibitTime = CmdInhibitTime;
  iLastCmd        = 0;
  iLastCmdTime    = 0;
}

# ifdef P16_SETTINGS_V1
void P016_data_struct::convertCommandLines(struct EventStruct *event) {
  String log;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    log.reserve(80);
    #  ifdef PLUGIN_016_DEBUG

    log  = F("P016: struct size: ");
    log += sizeof(CommandLines);
    log += '/';
    log += sizeof(tCommandLinesV2);
    log += '/';
    log += sizeof(tCommandLines);
    log += F(" enum: ");
    log += sizeof(decode_type_t);
    addLog(LOG_LEVEL_INFO, log);
    #  endif // ifdef PLUGIN_016_DEBUG
  }
  tCommandLines CommandLinesV1[P16_Nlines];   // holds the CustomTaskSettings V1

  // read V1 data && convert
  LoadCustomTaskSettings(event->TaskIndex, (uint8_t *)&(CommandLinesV1), sizeof(CommandLinesV1));
  uint16_t codeFlags            = 0;
  uint16_t alternativeCodeFlags = 0;

  for (int i = 0; i < P16_Nlines; ++i) {
    if (safe_strncpy(CommandLines[i].Command, CommandLinesV1[i].Command, P16_Nchars)) { // This should never fail.
      codeFlags            = 0;
      alternativeCodeFlags = 0;

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log  = F("P016: converting "); // Still a little logging
        log += i;
        #  ifdef PLUGIN_016_DEBUG
        log += ':';
        #  endif // ifdef PLUGIN_016_DEBUG
      }

      if (CommandLinesV1[i].Code > 0) {
        CommandLines[i].CodeDecodeType = static_cast<decode_type_t>((CommandLinesV1[i].Code >> 24)); // decode_type
        bitWrite(codeFlags, P16_FLAGS_REPEAT, CommandLinesV1[i].Code & (0x1 << P16_CMDBIT_REPEAT));  // Repeat flag
        #  ifdef PLUGIN_016_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          log += F(" type ");
          log += typeToString(CommandLines[i].CodeDecodeType, (CommandLinesV1[i].Code & (0x1 << P16_CMDBIT_REPEAT)) != 0);
          log += F(" code 0x");
          log += uint64ToString(CommandLinesV1[i].Code, 16);
        }
        #  endif // ifdef PLUGIN_016_DEBUG
      }

      if (CommandLinesV1[i].AlternativeCode > 0) {
        CommandLines[i].AlternativeCodeDecodeType = static_cast<decode_type_t>((CommandLinesV1[i].AlternativeCode >> 24)); // decode_type
        bitWrite(alternativeCodeFlags, P16_FLAGS_REPEAT, CommandLinesV1[i].AlternativeCode & (0x1 << P16_CMDBIT_REPEAT));  // Repeat flag
        #  ifdef PLUGIN_016_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          log += F(" alt.type ");
          log +=
            typeToString(CommandLines[i].AlternativeCodeDecodeType, (CommandLinesV1[i].AlternativeCode & (0x1 << P16_CMDBIT_REPEAT)) != 0);
          log += F(" alt.code 0x");
          log += uint64ToString(CommandLinesV1[i].AlternativeCode, 16);
        }
        #  endif // ifdef PLUGIN_016_DEBUG
      }
      CommandLines[i].Code                 = CommandLinesV1[i].Code & 0x7FFFFF;   // Only keep lowest 23 bits
      CommandLines[i].CodeFlags            = codeFlags;
      CommandLines[i].AlternativeCode      = CommandLinesV1[i].AlternativeCode & 0x7FFFFF;
      CommandLines[i].AlternativeCodeFlags = alternativeCodeFlags;
      addLog(LOG_LEVEL_INFO, log);
    }
    delay(0);
  }
}

# endif // ifdef P16_SETTINGS_V1

void P016_data_struct::loadCommandLines(struct EventStruct *event) {
  # if defined(P16_SETTINGS_V2) && defined(P16_SETTINGS_V1)

  // Convert the settings if both versions are defined and PCONFIG(7) != latest version
  if (PCONFIG(7) != P16_SETTINGS_LATEST) {
    addLog(LOG_LEVEL_ERROR, F("P016 IR: Settings conversion, save task settings to store in new format."));

    convertCommandLines(event);
  } else {
  # endif // if defined(P16_SETTINGS_V2) && defined(P16_SETTINGS_V1)
  // read V2 settings data
  LoadCustomTaskSettings(event->TaskIndex, (uint8_t *)&(CommandLines), sizeof(CommandLines));
  # if defined(P16_SETTINGS_V2) && defined(P16_SETTINGS_V1)
}

  # endif // if defined(P16_SETTINGS_V2) && defined(P16_SETTINGS_V1)

  for (int i = 0; i < P16_Nlines; ++i) {
    CommandLines[i].Command[P16_Nchars - 1] = 0; // Terminate in case of uninitalized data
  }
}

void P016_data_struct::AddCode(uint64_t Code, decode_type_t DecodeType, uint16_t CodeFlags) {
  // add received code
  int _index = P16_Nlines;

  if (Code == 0) {
    return;
  }

  for (int i = 0; i < P16_Nlines; ++i) {
    if (validateCode(i, Code, DecodeType, CodeFlags)) {
      // code already saved
      return;
    }

    if (CommandLines[i].Code == 0) {
      // get first index to add the code
      _index = std::min(i, _index);
    }
  }

  if (_index == P16_Nlines) {
    // no free index
    return;
  }
  CommandLines[_index].Code           = Code;
  CommandLines[_index].CodeDecodeType = DecodeType;
  CommandLines[_index].CodeFlags      = CodeFlags;
  bCodeChanged                        = true;
  # ifdef PLUGIN_016_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    log.reserve(80); // estimated
    log  = F("[P36] AddCode: ");
    log += typeToString(DecodeType, bitRead(CodeFlags, P16_FLAGS_REPEAT));
    log += F(" code: 0x");
    log += uint64ToString(Code, 16);
    log += F(" to index ");
    log += _index;
    addLog(LOG_LEVEL_INFO, log);
  }
  # endif // PLUGIN_016_DEBUG
}

void P016_data_struct::ExecuteCode(uint64_t Code, decode_type_t DecodeType, uint16_t CodeFlags) {
  if (Code == 0) {
    return;
  }

  if ((iLastCmd == Code) && (iLastDecodeType == DecodeType)) {
    // same code as before
    if (iCmdInhibitTime > timePassedSince(iLastCmdTime)) {
      // inhibit time not ellapsed
      return;
    }
  }

  for (int i = 0; i < P16_Nlines; ++i) {
    if (validateCode(i, Code, DecodeType, CodeFlags)) {
      // code already saved
      iLastCmd        = Code;
      iLastDecodeType = DecodeType;
      iLastCodeFlags  = CodeFlags;
      iLastCmdTime    = millis();

      if (CommandLines[i].Command[0] != 0) {
        # ifdef PLUGIN_016_DEBUG
        bool _success =
        # endif // ifdef PLUGIN_016_DEBUG
        ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_SYSTEM, CommandLines[i].Command);
        # ifdef PLUGIN_016_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log;
          log.reserve(128); // estimated
          log  = F("[P36] Execute: ");
          log += typeToString(DecodeType, bitRead(CodeFlags, P16_FLAGS_REPEAT));
          log += F(" Code: 0x");
          log += uint64ToString(Code, 16);
          log += F(" with command ");
          log += (i + 1);
          log += F(": {");
          log += String(CommandLines[i].Command);
          log += '}';

          if (!_success) {
            log += F(" FAILED!");
          }
          addLog(LOG_LEVEL_INFO, log);
        }
        # endif // PLUGIN_016_DEBUG
      }
      return;
    }
  }
}

bool P016_data_struct::validateCode(int i, uint64_t Code, decode_type_t DecodeType, uint16_t CodeFlags) {
  return ((CommandLines[i].Code == Code)
          && (CommandLines[i].CodeDecodeType == DecodeType)
          && (CommandLines[i].CodeFlags == CodeFlags))
         || ((CommandLines[i].AlternativeCode == Code)
             && (CommandLines[i].AlternativeCodeDecodeType == DecodeType)
             && (CommandLines[i].AlternativeCodeFlags == CodeFlags));
}

#endif // ifdef USES_P016
