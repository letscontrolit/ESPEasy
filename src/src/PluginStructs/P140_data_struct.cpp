#include "../PluginStructs/P140_data_struct.h"

#ifdef USES_P140

# include "../Commands/ExecuteCommand.h"

P140_data_struct::P140_data_struct(struct EventStruct *event) {
  _events = 1 == P140_SEND_EVENTS;
  _exec   = 1 == P140_EXEC_COMMAND;
  _input  = 1 == P140_GET_INPUT;
}

P140_data_struct::~P140_data_struct() {}

bool P140_data_struct::plugin_read(struct EventStruct *event) {
  return false;
}

bool P140_data_struct::plugin_write(struct EventStruct *event, String& string) {
  const String cmd = parseString(string, 1);

  if (equals(cmd, F("cardkb"))) {
    const String subcmd = parseString(string, 2);

    if (equals(subcmd, F("events")) && ((0 == event->Par2) || (1 == event->Par2))) {
      P140_SEND_EVENTS = event->Par2;
      _events          = 1 == event->Par2;
      return true;
    } else
    if (equals(subcmd, F("exec")) && ((0 == event->Par2) || (1 == event->Par2))) {
      P140_EXEC_COMMAND = event->Par2;
      _exec             = 1 == event->Par2;

      if (!(_exec || _input)) {
        clear(); // Clear when both turned off
      }
      return true;
    } else
    if (equals(subcmd, F("input")) && ((0 == event->Par2) || (1 == event->Par2))) {
      P140_GET_INPUT = event->Par2;
      _input         = 1 == event->Par2;

      if (!(_exec || _input)) {
        clear(); // Clear when both turned off
      }
      return true;
    } else
    if (equals(subcmd, F("clear"))) {
      clear(); // Clear buffer
      return true;
    }
  }
  return false;
}

bool P140_data_struct::getBufferValue(String& string) {
  if (_inCounter) {
    if (_inCounter < P140_INPUT_BUFFER_SIZE) {
      _buffer[_inCounter] = 0; // terminate string
    }
    const String tmp(_buffer);
    string = std::move(tmp);
    return true;
  }
  return false;
}

bool P140_data_struct::plugin_ten_per_second(struct EventStruct *event) {
  bool result = false;
  bool update = false;
  bool ok;
  const uint8_t inChar = I2C_read8(P140_I2C_ADDR, &ok);

  if (!ok || (0 == inChar)) {
    return false; // Shortcut
  }

  if ((_exec || _input) && isprint(inChar))
  {
    if (_inCounter < P140_INPUT_BUFFER_SIZE) { // add char to buffer if it still fits
      _buffer[_inCounter++] = inChar;
    }
    update = true;
    # if P140_DEBUG

    if (_inCounter < P140_INPUT_BUFFER_SIZE) {
      _buffer[_inCounter] = 0; // terminate string
    }
    addLog(LOG_LEVEL_INFO, strformat(F("CardKB: Key %d (0x%02X) '%c' command: %s"), inChar, inChar, inChar, _buffer));
    # endif // if P140_DEBUG
  } else

  if ((inChar == '\b') && (_inCounter > 0)) // Correct a typo using BackSpace
  {
    _buffer[--_inCounter] = 0;              // shorten input
    update                = true;
    # if P140_DEBUG
    addLog(LOG_LEVEL_INFO, strformat(F("CardKB: Backspace : %s"), _buffer));
    # endif // if P140_DEBUG
  } else
  if ((_exec || _input) && ((inChar == '\r') || (inChar == '\n'))) { // CR or LF completes input
    // Ignore empty input
    if (_inCounter != 0) {
      _buffer[_inCounter] = 0;                                       // keyboard data completed
      const String cmd(_buffer);

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLog(LOG_LEVEL_INFO, strformat(F("CardKB: Input: %s"), _buffer));
      }

      if (_exec) {
        // Act like we entered a command via serial
        ExecuteCommand_all({ EventValueSource::Enum::VALUE_SOURCE_SERIAL, std::move(cmd) }, true);
      }
      update = true;
      result = true;
    }
  } else
  if (inChar != 0) {
    update = true;
    # if P140_DEBUG
    addLog(LOG_LEVEL_INFO, strformat(F("CardKB: Key: %d (0x%02X) (ignored)"), inChar, inChar));
    # endif // if P140_DEBUG
  }


  if (update) {
    UserVar.setFloat(event->TaskIndex, 0, inChar);
    UserVar.setFloat(event->TaskIndex, 1, _inCounter);

    if (_events || (result && _input)) { // Send events also when using Input mode after pressing Enter
      sendData(event);
    }

    if (result) {
      clear(); // keyboard data processed, clear buffer
    }
  }
  return result;
}

void P140_data_struct::clear() {
  _inCounter = 0;
  _buffer[0] = 0;
}

#endif // ifdef USES_P140
