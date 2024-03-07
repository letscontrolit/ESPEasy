#include "../PluginStructs/P016_data_struct.h"

#ifdef USES_P016

# include "../Commands/ExecuteCommand.h"
# include "../Helpers/ESPEasy_Storage.h"
# include <IRutils.h>
# ifdef P016_CHECK_HEAP
#  include "src/Helpers/Memory.h"
# endif // ifdef P016_CHECK_HEAP

# ifdef P16_SETTINGS_V1

// Conversion constructor
tCommandLinesV2::tCommandLinesV2(const tCommandLinesV1& lineV1, uint8_t i)
{
  String log;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    #  ifndef PLUGIN_016_DEBUG
    log.reserve(20); // less space needed
    #  else // ifndef PLUGIN_016_DEBUG
    log.reserve(80);
    #  endif // ifndef PLUGIN_016_DEBUG
  }

  log  = F("P016: converting "); // Still a little logging
  log += i;
  #  ifdef PLUGIN_016_DEBUG
  log += ':';
  #  endif // ifdef PLUGIN_016_DEBUG

  memcpy(Command, lineV1.Command, P16_Nchars);
  const uint32_t oldCode = lineV1.Code;

  if (oldCode > 0) {
    CodeDecodeType = static_cast<decode_type_t>((oldCode >> 24));                // decode_type
    bitWrite(CodeFlags, P16_FLAGS_REPEAT, oldCode & (0x1 << P16_CMDBIT_REPEAT)); // Repeat flag
    Code = oldCode & 0x7FFFFF;                                                   // Only keep lowest 23 bits
    #  ifdef PLUGIN_016_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log += F(" type ");
      log += typeToString(CodeDecodeType, (oldCode & (0x1 << P16_CMDBIT_REPEAT)) != 0);
      log += F(" code 0x");
      log += uint64ToString(oldCode, 16);
    }
    #  endif // ifdef PLUGIN_016_DEBUG
  }

  const uint32_t oldAlternativeCode = lineV1.AlternativeCode;

  if (oldAlternativeCode > 0) {
    AlternativeCodeDecodeType = static_cast<decode_type_t>((oldAlternativeCode >> 24));                // decode_type
    bitWrite(AlternativeCodeFlags, P16_FLAGS_REPEAT, oldAlternativeCode & (0x1 << P16_CMDBIT_REPEAT)); // Repeat flag
    AlternativeCode = oldAlternativeCode & 0x7FFFFF;                                                   // Only keep lowest 23 bits
    #  ifdef PLUGIN_016_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log += F(" alt.type ");
      log += typeToString(AlternativeCodeDecodeType, (oldAlternativeCode & (0x1 << P16_CMDBIT_REPEAT)) != 0);
      log += F(" alt.code 0x");
      log += uint64ToString(oldAlternativeCode, 16);
    }
    #  endif // ifdef PLUGIN_016_DEBUG
  }
  addLogMove(LOG_LEVEL_INFO, log);
}

# endif // ifdef P16_SETTINGS_V1


void P016_data_struct::init(struct EventStruct *event, uint16_t CmdInhibitTime) {
  # if P016_FEATURE_COMMAND_HANDLING
  loadCommandLines(event);
  iCmdInhibitTime = CmdInhibitTime;
  iLastCmd        = 0;
  iLastCmdTime    = 0;
  # endif // if P016_FEATURE_COMMAND_HANDLING
}

# if P016_FEATURE_COMMAND_HANDLING
void P016_data_struct::loadCommandLines(struct EventStruct *event) {
  #  ifdef P16_SETTINGS_V1

  // Convert the settings if both versions are defined and PCONFIG(7) != latest version
  if (PCONFIG(7) != P16_SETTINGS_LATEST) {
    addLog(LOG_LEVEL_ERROR, F("P016 IR: Settings conversion, save task settings to store in new format."));
  }
  #  endif // ifdef P16_SETTINGS_V1
  CommandLines.clear(); // Start fresh

  for (uint8_t i = 0; i < P16_Nlines; i++) {
    CommandLines.push_back(tCommandLinesV2());
    loadCommandLine(event, CommandLines[i], i);
  }
}

void P016_data_struct::saveCommandLines(struct EventStruct *event) {
  for (uint8_t i = 0; i < P16_Nlines; i++) {
    saveCommandLine(event, CommandLines[i], i);
  }
}

void P016_data_struct::loadCommandLine(struct EventStruct *event, tCommandLinesV2& line, uint8_t lineNr)
{
  #  ifdef P16_SETTINGS_V1

  if (PCONFIG(7) != P16_SETTINGS_LATEST) {
    loadCommandLinev1(event, line, lineNr);
    return;
  }
  #  endif // ifdef P16_SETTINGS_V1

  const int loadOffset = lineNr * sizeof(tCommandLinesV2);
  LoadFromFile(SettingsType::Enum::CustomTaskSettings_Type,
               event->TaskIndex,
               reinterpret_cast<uint8_t *>(&line),
               sizeof(tCommandLinesV2),
               loadOffset);
  line.Command[P16_Nchars - 1] = 0; // Terminate in case of uninitalized data
}

# endif // if P016_FEATURE_COMMAND_HANDLING

# ifdef P16_SETTINGS_V1
void P016_data_struct::loadCommandLinev1(struct EventStruct *event, tCommandLinesV2& line, uint8_t lineNr)
{
  tCommandLinesV1 lineV1;

  {
    const int loadOffsetV1 = lineNr * sizeof(tCommandLinesV1);
    LoadFromFile(SettingsType::Enum::CustomTaskSettings_Type,
                 event->TaskIndex,
                 reinterpret_cast<uint8_t *>(&lineV1),
                 sizeof(tCommandLinesV2),
                 loadOffsetV1);
  }

  line                         = tCommandLinesV2(lineV1, lineNr);
  line.Command[P16_Nchars - 1] = 0;
}

# endif // ifdef P16_SETTINGS_V1

# if P016_FEATURE_COMMAND_HANDLING
void P016_data_struct::saveCommandLine(struct EventStruct *event, const tCommandLinesV2& line, uint8_t lineNr)
{
  const int saveOffset = lineNr * sizeof(tCommandLinesV2);

  SaveToFile(SettingsType::Enum::CustomTaskSettings_Type,
             event->TaskIndex,
             reinterpret_cast<const uint8_t *>(&line),
             sizeof(tCommandLinesV2),
             saveOffset);
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
  #  ifdef PLUGIN_016_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLogMove(LOG_LEVEL_INFO, strformat(
                 F("[P016] AddCode: %s code: 0x%s to index %d"),
                 typeToString(DecodeType, bitRead(CodeFlags, P16_FLAGS_REPEAT)).c_str(),
                 uint64ToString(Code, 16).c_str(),
                 _index));
  }
  #  endif // PLUGIN_016_DEBUG
}

bool P016_data_struct::ExecuteCode(uint64_t Code, decode_type_t DecodeType, uint16_t CodeFlags) {
  if (Code == 0) {
    return false;
  }

  if ((iLastCmd == Code) && (iLastDecodeType == DecodeType)) {
    // same code as before
    if (iCmdInhibitTime > timePassedSince(iLastCmdTime)) {
      // inhibit time not ellapsed
      return false;
    }
  }

  const unsigned int nr_CommandLines = CommandLines.size();

  for (unsigned int i = 0; i < nr_CommandLines; ++i) {
    if (validateCode(i, Code, DecodeType, CodeFlags)) {
      // code already saved
      iLastCmd        = Code;
      iLastDecodeType = DecodeType;
      iLastCodeFlags  = CodeFlags;
      iLastCmdTime    = millis();

      if (CommandLines[i].Command[0] != 0) {
        #  ifdef P016_CHECK_HEAP
        CheckHeap(F("Before ExecuteCommand_all:"));
        #  endif // ifdef P016_CHECK_HEAP
        ExecuteCommand_all(
          { EventValueSource::Enum::VALUE_SOURCE_SYSTEM, CommandLines[i].Command }, true);
        #  ifdef P016_CHECK_HEAP
        CheckHeap(F("After ExecuteCommand_all:"));
        #  endif // ifdef P016_CHECK_HEAP
        #  ifdef PLUGIN_016_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLogMove(LOG_LEVEL_INFO, strformat(
                       F("[P016] Execute added: %s Code: 0x%s with command %d: {%s}"),
                       typeToString(DecodeType, bitRead(CodeFlags, P16_FLAGS_REPEAT)).c_str(),
                       uint64ToString(Code, 16).c_str(),
                       (i + 1),
                       CommandLines[i].Command));
        }
        #  endif // PLUGIN_016_DEBUG
        return true;
      }
    }
  }
  #  ifdef PLUGIN_016_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
    addLogMove(LOG_LEVEL_ERROR, strformat(
                 F("[P016] ValidateCode failed: %s Code: 0x%s"),
                 typeToString(DecodeType, bitRead(CodeFlags, P16_FLAGS_REPEAT)).c_str(),
                 uint64ToString(Code, 16).c_str()));
  }
  #  endif // PLUGIN_016_DEBUG
  return false;
}

bool P016_data_struct::validateCode(int i, uint64_t Code, decode_type_t DecodeType, uint16_t CodeFlags) {
  if ((i >= static_cast<int>(CommandLines.size())) || (i < 0)) { return false; }
  return ((CommandLines[i].Code == Code)
          && (CommandLines[i].CodeDecodeType == DecodeType)
          && (CommandLines[i].CodeFlags == CodeFlags))
         || ((CommandLines[i].AlternativeCode == Code)
             && (CommandLines[i].AlternativeCodeDecodeType == DecodeType)
             && (CommandLines[i].AlternativeCodeFlags == CodeFlags));
}

# endif // if P016_FEATURE_COMMAND_HANDLING
# ifdef P016_CHECK_HEAP
void P016_data_struct::CheckHeap(String dbgtxt) {
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLogMove(LOG_LEVEL_INFO, strformat(
                 F("P016: %s FreeMem: %d FreeStack:%d"),
                 dbgtxt.c_str(), FreeMem(), getCurrentFreeStack()));
  }
}

# endif // ifdef P016_CHECK_HEAP

#endif // ifdef USES_P016
