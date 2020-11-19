#include "P016_data_struct.h"

#ifdef USES_P016

#include "../Commands/InternalCommands.h"
#include "../Helpers/ESPEasy_Storage.h"

P016_data_struct::P016_data_struct() {}

void P016_data_struct::init(taskIndex_t taskIndex, uint16_t CmdInhibitTime) {
  loadCommandLines(taskIndex);
  iCmdInhibitTime = CmdInhibitTime;
  iLastCmd = 0;
  iLastCmdTime = 0;
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
  String log;
  log.reserve(45); // estimated
  log = F("[P36] AddCode: 0x");
  log += uint64ToString(Code, 16);
  log += F(" to index ");
  log += _index;
  addLog(LOG_LEVEL_INFO, log);
#endif // PLUGIN_016_DEBUG
}

void P016_data_struct::ExecuteCode(uint32_t  Code) {
  if (Code == 0) {
    return;
  }
  uint32_t _now = millis();
  if (iLastCmd == Code) {
    // same code as before
    if (iCmdInhibitTime > (int32_t)(_now - iLastCmdTime)) {
      // inhibit time not ellapsed
      return;
    }
  }
  for (int i = 0; i < P16_Nlines; ++i) {
    if ((CommandLines[i].Code == Code) || (CommandLines[i].AlternativeCode == Code)) {
      // code already saved
      iLastCmd = Code;
      iLastCmdTime = _now;

      if (CommandLines[i].Command[0] != 0) {
        bool _success = ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_SYSTEM, CommandLines[i].Command);
#ifdef PLUGIN_016_DEBUG
        String log;
        log.reserve(128); // estimated
        log = F("[P36] ExecuteCode: 0x");
        log += uint64ToString(Code, 16);
        log += F(" with command ");
        log += (i+1);
        log += F(": {");
        log += String(CommandLines[i].Command);
        log += '}';
        if (!_success) {
          log += F(" FAILED!");
        }
        addLog(LOG_LEVEL_INFO, log);
#endif // PLUGIN_016_DEBUG
      }
      return;
    }
  }
}

#endif // ifdef USES_P016
