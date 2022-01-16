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

uint32_t P126_data_struct::getChannelState(uint8_t offset, uint8_t size) {
  uint32_t result = 0u;
  uint8_t *pvalue = shift->getAll();

  if (nullptr != pvalue) {
    for (uint8_t o = offset; o < offset + size; o++) {
      result += (pvalue[o] << (8 * o));
    }
  }
  return result;
}

bool P126_data_struct::plugin_read(struct EventStruct *event) {
  for (uint8_t index = 0; index < _chipCount && index < (VARS_PER_TASK * 4); index += 4) {
    uint32_t result = getChannelState(index, min(VARS_PER_TASK, _chipCount - index));
    UserVar.setUint32(event->TaskIndex, index / 4, result);
  }
  return true;
}

bool P126_data_struct::plugin_write(struct EventStruct *event,
                                    const String      & string) {
  bool   success = false;
  String command = parseString(string, 1);

  if (command.equals(F("74hcset"))) {
    const uint8_t pin   = event->Par1;
    const uint8_t value = event->Par2;

    if (validChannel(pin)) {
      shift->set(pin, value);
      success = true;
    }
    # ifdef P126_DEBUG_LOG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log = F("74hcset, pin: ");
      log += pin;
      log += F(", value: ");
      log += value;
      addLog(LOG_LEVEL_DEBUG, log);
    }
    # endif // ifdef P126_DEBUG_LOG
  } else if (command.equals(F("74hcsetnoupdate"))) {
    const uint8_t pin   = event->Par1;
    const uint8_t value = event->Par2;

    if (validChannel(pin)) {
      shift->setNoUpdate(pin, value);
      success = true;
    }
    # ifdef P126_DEBUG_LOG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log = F("74hcsetnoupdate, pin: ");
      log += pin;
      log += F(", value: ");
      log += value;
      addLog(LOG_LEVEL_DEBUG, log);
    }
    # endif // ifdef P126_DEBUG_LOG
  } else if (command.equals(F("74hcupdate"))) {
    shift->updateRegisters();
    success = true;
    # ifdef P126_DEBUG_LOG

    addLog(LOG_LEVEL_DEBUG, F("74hcupdate"));
    # endif // ifdef P126_DEBUG_LOG
  } else if (command.equals(F("74hcsetall"))) {
    std::vector<uint8_t> value;
    value.resize(_chipCount, 0); // Initialize vector to 0's
    uint32_t par = 0u;
    uint8_t  lp  = 255;

    for (uint8_t i = 0; i < _chipCount; i++) {
      uint8_t p = i / 4;

      if (p != lp) {
        String  arg = parseString(string, p + 2);
        int64_t tmp = 0;
        par = 0u; // reset

        if (validInt64FromString(arg, tmp)) {
          par = static_cast<uint32_t>(tmp);
        }
        lp = p;

        # ifdef P126_DEBUG_LOG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          String log = F("74hcsetall: arg: ");
          log += arg;
          log += F(", tmp: ");
          log += ull2String(tmp);
          log += F("/0x");
          log += ull2String(tmp, HEX);
          log += F(", par: ");
          log += par;
          log += F("/0x");
          log += ull2String(par, HEX);
          addLog(LOG_LEVEL_DEBUG, log);
        }
        # endif // ifdef P126_DEBUG_LOG
      }
      value[i] = ((par >> ((i % 4) * 8)) & 0xff);

      # ifdef P126_DEBUG_LOG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        String log = F("74hcsetall: value[");
        log += i;
        log += F("] : ");
        log += value[i];
        log += F("/0x");
        log += String(value[i], HEX);
        log += F(", i % 4 * 8 : ");
        log += i % 4;
        log += '/';
        log += (i % 4) * 8;
        addLog(LOG_LEVEL_DEBUG, log);
      }
      # endif // ifdef P126_DEBUG_LOG
    }

    shift->setAll(&value[0]);
    success = true;
  } else if (command.equals(F("74hcsetalllow"))) {
    shift->setAllLow();
    # ifdef P126_DEBUG_LOG
    addLog(LOG_LEVEL_DEBUG, F("74hcsetalllow"));
    # endif // ifdef P126_DEBUG_LOG
    success = true;
  } else if (command.equals(F("74hcsetallhigh"))) {
    shift->setAllHigh();
    # ifdef P126_DEBUG_LOG
    addLog(LOG_LEVEL_DEBUG, F("74hcsetallhigh"));
    # endif // ifdef P126_DEBUG_LOG
    success = true;
  }
  return success;
}

#endif // ifdef USES_P126
