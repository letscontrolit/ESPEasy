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
  if (nullptr != shift) {
    delete shift;
    shift = nullptr;
  }
}

const uint32_t P126_data_struct::getChannelState(uint8_t offset, uint8_t size) const {
  uint32_t result       = 0u;
  const uint8_t *pvalue = shift->getAll();
  const uint8_t  last   = offset + size;

  if (nullptr != pvalue) {
    uint16_t sft = 0u;

    for (uint8_t ofs = offset; ofs < last; ofs++, sft++) {
      result += (pvalue[ofs] << (8 * sft));
    }

    # ifdef P126_DEBUG_LOG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log = F("74hc595: getChannelState offset: ");
      log += offset;
      log += F(", size: ");
      log += size;
      log += F(", result: ");
      log += result;
      log += F("/0x");
      log += String(result, HEX);
      addLog(LOG_LEVEL_DEBUG, log);
    }
    # endif // ifdef P126_DEBUG_LOG
  }
  return result;
}

bool P126_data_struct::plugin_read(struct EventStruct *event) {
  const uint16_t last = P126_CONFIG_SHOW_OFFSET + (VARS_PER_TASK * 4);
  uint8_t varNr       = 0;

  for (uint16_t index = P126_CONFIG_SHOW_OFFSET; index < _chipCount && index < last && varNr < VARS_PER_TASK; index += 4, varNr++) {
    uint32_t result = getChannelState(index, min(VARS_PER_TASK, _chipCount - index));
    UserVar.setUint32(event->TaskIndex, varNr, result);
  }
  return true;
}

bool P126_data_struct::plugin_write(struct EventStruct *event,
                                    const String      & string) {
  bool   success = false;
  String command = parseString(string, 1);

  if (command.startsWith(F("74hc"))) {
    if (command.equals(F("74hcset")) || command.equals(F("74hcsetnoupdate"))) {
      const uint8_t  pin   = event->Par1;
      const uint16_t value = event->Par2;

      if (validChannel(pin)) {
        shift->set(pin - 1, value, command.equals(F("74hcset")));
        success = true;
      }
      # ifdef P126_DEBUG_LOG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        String log = command;
        log += F(", pin: ");
        log += pin;
        log += F(", value: ");
        log += value;
        addLog(LOG_LEVEL_DEBUG, log);
      }
      # endif // ifdef P126_DEBUG_LOG
    } else if (command.equals(F("74hcupdate"))) {
      shift->updateRegisters();
      success = true;
    } else if (command.equals(F("74hcsetall")) || command.equals(F("74hcsetallnoupdate"))) {
      success = true;
      std::vector<uint8_t> value;
      value.resize(_chipCount, 0);             // Initialize vector to 0's

      const uint8_t *pvalue = shift->getAll(); // Get current state

      for (uint8_t i = 0; i < _chipCount; i++) {
        value[i] = pvalue[i];
      }

      uint32_t par   = 0u;
      uint8_t  param = 2; // Start with an offset
      uint8_t  width = 4;
      uint8_t  idx   = 0;
      String   arg   = parseString(string, param);

      while (!arg.isEmpty() && idx < _chipCount && success) {
        int colon = arg.indexOf(':'); // First colon: Chip-index, range 1.._chipCount
        int itmp  = 0;

        if (colon != -1) {
          String cis = arg.substring(0, colon);
          arg = arg.substring(colon + 1);

          if (!cis.isEmpty() && validIntFromString(cis, itmp) && (itmp > 0) && (itmp <= _chipCount)) {
            idx = itmp - 1;       // Actual range is 0.._chipCount - 1
          } else {
            success = false;      // Cancel entire operation on error
          }
        }
        colon = arg.indexOf(':'); // Second colon: data width, range 1..4 bytes
        width = 4;                // Set default data width to 4 = 32 bits

        if (colon != -1) {
          String lis = arg.substring(0, colon);
          arg = arg.substring(colon + 1);

          if (!lis.isEmpty() && validIntFromString(lis, itmp) && (itmp > 0) && (itmp <= 4)) {
            width = itmp;
          } else {
            success = false; // Cancel entire operation on error
          }
        }
        int64_t tmp = 0;
        par = 0u; // reset

        if (validInt64FromString(arg, tmp)) {
          par = static_cast<uint32_t>(tmp);
        }

        param++; // Process next argument
        arg = parseString(string, param);

        # ifdef P126_DEBUG_LOG

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = command;
          log += F(": arg: ");
          log += arg;
          log += F(", tmp: ");
          log += ull2String(tmp);
          log += F("/0x");
          log += ull2String(tmp, HEX);
          log += F(", par: ");
          log += par;
          log += F("/0x");
          log += ull2String(par, HEX);
          log += F(", chip:");
          log += idx;
          log += F(", width:");
          log += width;
          addLog(LOG_LEVEL_INFO, log);
        }
        # endif // ifdef P126_DEBUG_LOG

        for (uint8_t n = 0; n < width && idx < _chipCount; n++, idx++) {
          value[idx] = ((par >> (n * 8)) & 0xff);

          # ifdef P126_DEBUG_LOG

          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            String log = command;
            log += F(": value[");
            log += idx;
            log += F("] : ");
            log += value[idx];
            log += F("/0x");
            log += String(value[idx], HEX);
            log += F(", n * 8: ");
            log += n;
            log += '/';
            log += n * 8;
            addLog(LOG_LEVEL_DEBUG, log);
          }
          # endif // ifdef P126_DEBUG_LOG
        }
      }

      if (success) {
        shift->setAll(&value[0], command.equals(F("74hcsetall")));
      }
    } else if (command.equals(F("74hcsetalllow"))) {
      shift->setAllLow();
      success = true;
    } else if (command.equals(F("74hcsetallhigh"))) {
      shift->setAllHigh();
      success = true;
    } else if (command.equals(F("74hcsetoffset"))) {
      if ((event->Par1 >= 0) && (event->Par1 <= P126_MAX_SHOW_OFFSET)) {
        P126_CONFIG_SHOW_OFFSET = event->Par1;

        if (P126_CONFIG_SHOW_OFFSET >= P126_CONFIG_CHIP_COUNT) {
          P126_CONFIG_SHOW_OFFSET = 0;
        }
        P126_CONFIG_SHOW_OFFSET -= (P126_CONFIG_SHOW_OFFSET % 4);

        if ((P126_CONFIG_SHOW_OFFSET > P126_CONFIG_CHIP_COUNT - 4) && (P126_CONFIG_CHIP_COUNT < P126_MAX_SHOW_OFFSET)) {
          P126_CONFIG_SHOW_OFFSET -= 4;
        }
        success = true;
      }
    # ifdef P126_SHOW_VALUES
    } else if (command.equals(F("74hcsethexbin"))) {
      if ((event->Par1 == 0) || (event->Par1 == 1)) {
        uint32_t lSettings = P126_CONFIG_FLAGS;
        bitWrite(lSettings, P126_FLAGS_VALUES_DISPLAY, event->Par1 == 1);
        P126_CONFIG_FLAGS = lSettings;
        success           = true;
      }
    # endif // ifdef P126_SHOW_VALUES
    }
    # ifdef P126_DEBUG_LOG

    if (success) {
      addLog(LOG_LEVEL_DEBUG, command);
    }
    # endif // ifdef P126_DEBUG_LOG
  }
  return success;
}

#endif // ifdef USES_P126
