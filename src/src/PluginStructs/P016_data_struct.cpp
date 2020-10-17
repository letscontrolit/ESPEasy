#include "P016_data_struct.h"

#ifdef USES_P016

#include "../Commands/InternalCommands.h"
#include "../Helpers/ESPEasy_Storage.h"

P016_data_struct::P016_data_struct() {}

void P016_data_struct::init(taskIndex_t     taskIndex) {
  loadCommandLines(taskIndex);
}

void P016_data_struct::loadCommandLines(taskIndex_t taskIndex) {
    // read data
    LoadCustomTaskSettings(taskIndex, (uint8_t *)&(CommandLines), sizeof(CommandLines));

    for (int i = 0; i < P16_Nlines; ++i) {
      CommandLines[i].Command[P16_Nchars - 1] = 0; // Terminate in case of uninitalized data
    }
}

void P016_data_struct::AddCode(uint32_t  Code) {
  // add received code
  int _index = P16_Nlines;
  if (Code == 0) {
    return;
  }
  for (int i = 0; i < P16_Nlines; ++i) {
    if ((CommandLines[i].Code == Code) || (CommandLines[i].AlternativeCode == Code)) {
      // code already saved
      return;
    }
    if (CommandLines[i].Code == 0) {
      // get first index to add the code
      _index = std::min(i,_index);
    }
  }
  if (_index == P16_Nlines) {
    // no free index
    return;
  }
  CommandLines[_index].Code = Code;
  bCodeChanged = true;
#ifdef PLUGIN_016_DEBUG
  String log = String(F("[P36] AddCode: 0x")) + uint64ToString(Code, 16);
  log += String(F(" to index ")) + String(_index);
  addLog(LOG_LEVEL_INFO, log);
#endif // PLUGIN_016_DEBUG
}

void P016_data_struct::ExecuteCode(uint32_t  Code) {
  if (Code == 0) {
    return;
  }
  for (int i = 0; i < P16_Nlines; ++i) {
    if ((CommandLines[i].Code == Code) || (CommandLines[i].AlternativeCode == Code)) {
      // code already saved
      if (CommandLines[i].Command[0] != 0) {
        ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_NR_VALUES, CommandLines[i].Command);
#ifdef PLUGIN_016_DEBUG
        String log = String(F("[P36] ExecuteCode: 0x")) + uint64ToString(Code, 16);
        log += String(F(" with command: ")) + String(CommandLines[i].Command);
        addLog(LOG_LEVEL_INFO, log);
#endif // PLUGIN_016_DEBUG
      }
      return;
    }
  }
}

#endif // ifdef USES_P016
