#include "../PluginStructs/P126_data_struct.h"

#ifdef USES_P126

// **************************************************************************/
// Constructor
// **************************************************************************/
P126_data_struct::P126_data_struct(int8_t  dataPin,
                                   int8_t  clockPin,
                                   int8_t  latchPin,
                                   uint8_t chipCount)
  : _dataPin(dataPin), _clockPin(clockPin), _latchPin(latchPin), _chipCount(chipCount) {
  shift = new (std::nothrow) ShiftRegister74HC595_NonTemplate(_chipCount, _dataPin, _clockPin, _latchPin);
}

// **************************************************************************/
// Destructor
// **************************************************************************/
P126_data_struct::~P126_data_struct() {
  delete shift;
  shift = nullptr;
}

bool P126_data_struct::plugin_init(struct EventStruct *event) {
  if (P126_CONFIG_FLAGS_GET_VALUES_RESTORE) { // Restore only when enabled
    uint8_t idx = P126_CONFIG_SHOW_OFFSET;
    std::vector<uint8_t> value;

    value.resize(_chipCount, 0);             // Initialize vector to 0's

    const uint8_t *pvalue = shift->getAll(); // Get current state

    for (uint8_t i = 0; i < _chipCount; ++i) {
      value[i] = pvalue[i];
    }

    const uint16_t maxVar = min(static_cast<uint8_t>(VARS_PER_TASK),
                                static_cast<uint8_t>(ceilf((P126_CONFIG_CHIP_COUNT - P126_CONFIG_SHOW_OFFSET) / 4.0f)));
    uint32_t par;

    for (uint16_t varNr = 0; varNr < maxVar; ++varNr) {
      par = UserVar.getUint32(event->TaskIndex, varNr);

      for (uint8_t n = 0; n < 4 && idx < _chipCount; ++n, ++idx) {
        value[idx] = ((par >> (n * 8)) & 0xff);

        # ifdef P126_DEBUG_LOG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          addLogMove(LOG_LEVEL_DEBUG, strformat(F("SHIFTOUT: plugin_init: value[%d] : %d/0x%02x, n * 8: %d/%d, varNr: %d"),
                                                idx, value[idx], value[idx], n, n * 8, varNr));
        }
        # endif // ifdef P126_DEBUG_LOG
      }
    }
    shift->setAll(&value[0], false); // DO NOT SEND OUTPUT TO REGISTERS
  }
  return true;
}

uint32_t P126_data_struct::getChannelState(uint8_t offset, uint8_t size) const {
  uint32_t result       = 0u;
  const uint8_t *pvalue = shift->getAll();
  const uint8_t  last   = offset + size;

  if (nullptr != pvalue) {
    uint16_t sft = 0u;

    for (uint8_t ofs = offset; ofs < last; ++ofs, ++sft) {
      result += (pvalue[ofs] << (8 * sft));
    }

    # ifdef P126_DEBUG_LOG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      addLogMove(LOG_LEVEL_DEBUG, strformat(F("SHIFTOUT: getChannelState offset: %d, size: %d, result: %d/0x%x"),
                                            offset, size, result, result));
    }
    # endif // ifdef P126_DEBUG_LOG
  }
  return result;
}

bool P126_data_struct::plugin_read(struct EventStruct *event) {
  const uint16_t last = P126_CONFIG_SHOW_OFFSET + (VARS_PER_TASK * 4);
  uint8_t varNr       = 0;

  bool changed{};

  for (uint16_t index = P126_CONFIG_SHOW_OFFSET; index < _chipCount && index < last && varNr < VARS_PER_TASK; index += 4, ++varNr) {
    const uint32_t result = getChannelState(index, min(VARS_PER_TASK, _chipCount - index));

    if (UserVar.getUint32(event->TaskIndex, varNr) != result) { changed = true; }
    UserVar.setUint32(event->TaskIndex, varNr, result);
  }
  return changed;
}

bool P126_data_struct::plugin_write(struct EventStruct *event,
                                    const String      & string) {
  bool success = false;
  bool updated = false;

  CommandArgParser parsedCmd;

  if (!parsedCmd.readCommandSubCommandAndMatch(string, F("shiftout"))) {
    return false;
  }

# ifdef P126_DEBUG_LOG
  parsedCmd.debug(F("init success"), LOG_LEVEL_INFO);
# endif

  const bool hc_update = parsedCmd.getSubCommand().toString().indexOf(F("noupdate")) == -1;

  if (parsedCmd.subCommandEquals(F("set")) ||
      parsedCmd.subCommandEquals(F("setnoupdate"))) {
    const uint8_t  pin   = parsedCmd.getArgInt(0, 0);
    const uint16_t value = parsedCmd.getArgInt(1, -1);

    if (validChannel(pin) && ((value == 0) || (value == 1))) {
      shift->set(pin - 1, value, hc_update);

      if (hc_update) { updated = true; }
      success = true;
    }
# ifdef P126_DEBUG_LOG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      addLogMove(LOG_LEVEL_DEBUG, strformat(
                   F("%s, pin: %d, value: %d"),
                   parsedCmd.getCommand().toString().c_str(),
                   pin,
                   value));
    }
# endif // ifdef P126_DEBUG_LOG
  } else if (parsedCmd.subCommandEquals(F("update"))) {
    shift->updateRegisters();
    updated = true;
    success = true;
  } else if (parsedCmd.subCommandEquals(F("setall")) ||
             parsedCmd.subCommandEquals(F("setallnoupdate"))) {
    success = true;
    std::vector<uint8_t> value;
    value.resize(_chipCount, 0);             // Initialize vector to 0's

    const uint8_t *pvalue = shift->getAll(); // Get current state

    for (uint8_t i = 0; i < _chipCount; ++i) {
      value[i] = pvalue[i];
    }

    uint32_t par   = 0u;
    uint8_t  param = 0; // Argument index
    uint8_t  width = 4;
    uint8_t  idx   = 0;
    String   arg   = parsedCmd.getArg(param).toString();

    while (!arg.isEmpty() && idx < _chipCount && success) {
      CommandArgParser cwv;

      if (!cwv.readArgumentsOnly(arg, ':'))
      {
        success = false;
        continue;
      }

# ifdef P126_DEBUG_LOG
      cwv.debug(F("Chip:Width:Val"), LOG_LEVEL_INFO);
# endif

      // First argument: Chip-index, range 1.._chipCount
      const auto chipIndex = cwv.getArgInt(0);

      if ((chipIndex > 0) && (chipIndex <= _chipCount)) {
        idx = chipIndex - 1; // Actual range is 0.._chipCount - 1
      } else {
        success = false;     // Cancel entire operation on error
        continue;
      }

      // Second argument: data width, range 1..4 bytes, default: 4
      width = cwv.getArgInt(1, 4);

      if ((width <= 0) && (width > 4)) {
        success = false; // Cancel entire operation on error
        continue;
      }


      // Third argument: value
      const auto tmp = cwv.getArgInt(2, -1);

      if (tmp < 0)
      {
        success = false; // Cancel entire operation on error
        continue;
      }
      const uint32_t nrBits = 8 * width;
      const uint32_t mask   = std::numeric_limits<uint32_t>::max() >> (32 - nrBits);
      par = static_cast<uint32_t>(tmp) & mask;

        # ifdef P126_DEBUG_LOG

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {

        addLogMove(
          LOG_LEVEL_INFO,
          strformat(
            F("%s: arg: %s, tmp: %s/%s, par: %u/%s, chip:%d, width:%d"),
            parsedCmd.getCommand().toString().c_str(),
            arg.c_str(),
            ull2String(tmp).c_str(),
            ull2String(tmp, HEX).c_str(),
            par,
            ull2String(par, HEX).c_str(),
            static_cast<int>(idx),
            static_cast<int>(width)));
      }
        # endif // ifdef P126_DEBUG_LOG

      param++; // Process next argument
      arg = parsedCmd.getArg(param).toString();

      for (uint8_t n = 0; n < width && idx < _chipCount; ++n, ++idx) {
        value[idx] = ((par >> (n * 8)) & 0xff);

          # ifdef P126_DEBUG_LOG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          addLogMove(LOG_LEVEL_DEBUG,
                     strformat(F("%s: value[%d] : %d/0x%x, n * 8: %d/%d"),
                               parsedCmd.getCommand().toString().c_str(),
                               idx,
                               value[idx],
                               value[idx],
                               n,
                               n * 8));
        }
          # endif // ifdef P126_DEBUG_LOG
      }
    }

    if (success) {
      shift->setAll(&value[0], hc_update);

      if (hc_update) { updated = true; }
    }
  } else if (parsedCmd.subCommandEquals(F("setalllow"))) {
    shift->setAllLow();
    updated = true;
    success = true;
  } else if (parsedCmd.subCommandEquals(F("setallhigh"))) {
    shift->setAllHigh();
    updated = true;
    success = true;
  } else if (parsedCmd.subCommandEquals(F("setoffset"))) {
    const int chipOffset = parsedCmd.getArgInt(0, -1);

    if ((chipOffset >= 0) && (chipOffset <= P126_MAX_SHOW_OFFSET)) {
      uint8_t previousOffset = P126_CONFIG_SHOW_OFFSET;
      P126_CONFIG_SHOW_OFFSET = chipOffset;

      if (P126_CONFIG_SHOW_OFFSET >= P126_CONFIG_CHIP_COUNT) {
        P126_CONFIG_SHOW_OFFSET = 0;
      }
      P126_CONFIG_SHOW_OFFSET -= (P126_CONFIG_SHOW_OFFSET % 4);

      if ((P126_CONFIG_CHIP_COUNT > 4) &&
          (P126_CONFIG_SHOW_OFFSET > P126_CONFIG_CHIP_COUNT - 4) &&
          (P126_CONFIG_CHIP_COUNT < P126_MAX_SHOW_OFFSET)) {
        P126_CONFIG_SHOW_OFFSET -= 4;
      }

      // Reset State_A..D values when changing the offset
      if ((previousOffset != P126_CONFIG_SHOW_OFFSET) && P126_CONFIG_FLAGS_GET_VALUES_RESTORE) {
        for (uint8_t varNr = 0; varNr < VARS_PER_TASK; ++varNr) {
          UserVar.setUint32(event->TaskIndex, varNr, 0u);
        }
          # ifdef P126_DEBUG_LOG
        addLog(LOG_LEVEL_INFO, F("SHIFTOUT: 'Offset for display' changed: state values reset."));
          # endif // ifdef P126_DEBUG_LOG
      }
      success = true;
    }
  } else if (parsedCmd.subCommandEquals(F("setchipcount"))) {
    const int chipCount = parsedCmd.getArgInt(0, -1);

    if ((chipCount >= 1) && (chipCount <= P126_MAX_CHIP_COUNT)) {
      P126_CONFIG_CHIP_COUNT = chipCount;
      _chipCount             = chipCount;
      shift->setSize(P126_CONFIG_CHIP_COUNT);
      success = true;
    }
    # ifdef P126_SHOW_VALUES
  } else if (parsedCmd.subCommandEquals(F("sethexbin"))) {
    const int HexBin = parsedCmd.getArgInt(0, -1);

    if ((HexBin == 0) || (HexBin == 1)) {
      uint32_t lSettings = P126_CONFIG_FLAGS;
      bitWrite(lSettings, P126_FLAGS_VALUES_DISPLAY, HexBin == 1);
      P126_CONFIG_FLAGS = lSettings;
      success           = true;
    }
    # endif // ifdef P126_SHOW_VALUES
  }
    # ifdef P126_DEBUG_LOG

  if (success) {
    addLog(LOG_LEVEL_DEBUG, string);
  }
    # endif // ifdef P126_DEBUG_LOG

  // TODO TD-er: Do we need to schedule a taskRun to send out events and/or send data to controller(s), or just a plugin_read(event) ??
  if (updated) { // plugin_read(event);
    Scheduler.schedule_task_device_timer(event->TaskIndex, 10);
  }
  return success;
}

#endif // ifdef USES_P126
