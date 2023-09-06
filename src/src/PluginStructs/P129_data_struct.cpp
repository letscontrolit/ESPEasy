#include "../PluginStructs/P129_data_struct.h"

#ifdef USES_P129
#include <GPIO_Direct_Access.h>

// **************************************************************************/
// Constructor
// **************************************************************************/
P129_data_struct::P129_data_struct(int8_t  dataPin,
                                   int8_t  clockPin,
                                   int8_t  enablePin,
                                   int8_t  loadPin,
                                   uint8_t chipCount)
  : _dataPin(dataPin), _clockPin(clockPin), _enablePin(enablePin), _loadPin(loadPin), _chipCount(chipCount) {}


bool P129_data_struct::plugin_init(struct EventStruct *event) {
  if (isInitialized()) {
    for (uint8_t i = 0; i < P129_MAX_CHIP_COUNT; i++) { // Clear entire buffer
      readBuffer[i] = 0;
    }

    // Prepare all used GPIO pins
    if (validGpio(_enablePin)) { pinMode(_enablePin, OUTPUT); }
    pinMode(_loadPin,  OUTPUT);
    pinMode(_clockPin, OUTPUT);
    pinMode(_dataPin,  INPUT);
    DIRECT_pinWrite(_loadPin, HIGH);

    if (validGpio(_enablePin)) { DIRECT_pinWrite(_enablePin, HIGH); }

    return true;
  }
  return false;
}

uint32_t P129_data_struct::getChannelState(uint8_t offset,
                                           uint8_t size) const {
  uint32_t result    = 0u;
  uint16_t sft       = 0u;
  const uint8_t last = offset + size;

  for (uint8_t ofs = offset; ofs < last; ofs++, sft++) {
    result += (readBuffer[ofs] << (8 * sft));
  }

  # ifdef P129_DEBUG_LOG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("SHIFTIN: getChannelState offset: ");
    log += offset;
    log += F(", size: ");
    log += size;
    log += F(", result: ");
    log += result;
    log += F("/0x");
    log += String(result, HEX);
    addLogMove(LOG_LEVEL_DEBUG, log);
  }
  # endif // ifdef P129_DEBUG_LOG

  return result;
}

bool P129_data_struct::plugin_read(struct EventStruct *event) {
  const uint16_t last = P129_CONFIG_SHOW_OFFSET + (VARS_PER_TASK * 4);
  uint8_t varNr       = 0;

  for (uint16_t index = P129_CONFIG_SHOW_OFFSET; index < _chipCount && index < last && varNr < VARS_PER_TASK; index += 4, varNr++) {
    uint32_t result = getChannelState(index, min(VARS_PER_TASK, _chipCount - index));
    UserVar.setUint32(event->TaskIndex, varNr, result);
  }
  return true;
}

bool P129_data_struct::plugin_write(struct EventStruct *event,
                                    const String      & string) {
  bool success = false;

  const String command = parseString(string, 1);

  if (equals(command, F("shiftin"))) {
    const String subcommand = parseString(string, 2);

    if (equals(subcommand, F("pinevent"))) { // ShiftIn,pinevent,<pin>,<0|1>
      const uint8_t pin   = event->Par2 - 1;
      const uint8_t value = event->Par3;

      if (validChannel(pin + 1) && ((value == 0) || (value == 1))) {
        const uint8_t ulong = pin / 32;
        const uint8_t bit   = pin % 32;
        uint32_t lSettings  = PCONFIG_ULONG(ulong);

        bitWrite(lSettings, bit, value);
        PCONFIG_ULONG(ulong) = lSettings;
        success              = true;
        # ifdef P129_DEBUG_LOG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          String log = command;
          log += F(", pin: ");
          log += event->Par2;
          log += F(", value: ");
          log += value;
          log += F(", config: ");
          log += ulong;
          log += F(", bit: ");
          log += bit;
          addLogMove(LOG_LEVEL_DEBUG, log);
        }
        # endif // ifdef P129_DEBUG_LOG
      }
    } else if (equals(subcommand, F("chipevent"))) { // ShiftIn,chipevent,<chip>,<0|1>
      const int8_t chip  = event->Par2 - 1;
      const uint8_t value = event->Par3;

      if ((chip >= 0) && (chip < P129_CONFIG_CHIP_COUNT) && ((value == 0) || (value == 1))) {
        const uint8_t ulong = chip / 4;
        const uint8_t bit   = (chip % 4) * 8;
        uint32_t lSettings  = PCONFIG_ULONG(ulong);

        set8BitToUL(lSettings, bit, value == 1 ? 0xFF : 0x00);
        PCONFIG_ULONG(ulong) = lSettings;
        success              = true;
        # ifdef P129_DEBUG_LOG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          String log = command;
          log += F(", chip: ");
          log += event->Par2;
          log += F(", value: ");
          log += value;
          log += F(", config: ");
          log += ulong;
          log += F(", bit: ");
          log += bit;
          addLogMove(LOG_LEVEL_DEBUG, log);
        }
        # endif // ifdef P129_DEBUG_LOG
      }
    } else if (equals(subcommand, F("setchipcount"))) { // ShiftIn,setchipcount,<count>
      if ((event->Par2 >= 1) && (event->Par2 <= P129_MAX_CHIP_COUNT)) {
        P129_CONFIG_CHIP_COUNT = event->Par2;
        _chipCount             = event->Par2;
        success                = true;
      }
    } else if (equals(subcommand, F("samplefrequency"))) { // ShiftIn,samplefrequency,<0|1>
      if ((event->Par2 == 0) || (event->Par2 == 1)) {
        uint32_t lSettings = P129_CONFIG_FLAGS;
        bitWrite(lSettings, P129_FLAGS_READ_FREQUENCY, event->Par2 == 1);
        P129_CONFIG_FLAGS = lSettings;
        success           = true;
      }
    } else if (equals(subcommand, F("eventperpin"))) { // ShiftIn,eventperpin,<0|1>
      if ((event->Par2 == 0) || (event->Par2 == 1)) {
        uint32_t lSettings = P129_CONFIG_FLAGS;
        bitWrite(lSettings, P129_FLAGS_SEPARATE_EVENTS, event->Par2 == 1);
        P129_CONFIG_FLAGS = lSettings;
        success           = true;
      }
    }
    # ifdef P129_DEBUG_LOG

    if (success) {
      addLog(LOG_LEVEL_DEBUG, string);
    }
    # endif // ifdef P129_DEBUG_LOG
  }
  return success;
}

bool P129_data_struct::plugin_readData(struct EventStruct *event) {
  if (isInitialized()) {
    DIRECT_pinWrite(_loadPin, LOW);
    delayMicroseconds(5);
    DIRECT_pinWrite(_loadPin, HIGH);
    delayMicroseconds(5);

    if (validGpio(_enablePin)) { DIRECT_pinWrite(_enablePin, LOW); }

    for (uint8_t i = 0; i < P129_CONFIG_CHIP_COUNT; i++) {
      prevBuffer[i] = readBuffer[i];
      readBuffer[i] = shiftIn(static_cast<uint8_t>(_dataPin), static_cast<uint8_t>(_clockPin), MSBFIRST);
    }

    if (validGpio(_enablePin)) { DIRECT_pinWrite(_enablePin, HIGH); }
    delay(0);

    checkDiff(event);

    return true;
  }
  return false;
}

void P129_data_struct::checkDiff(struct EventStruct *event) {
  for (uint8_t i = 0; i < P129_CONFIG_CHIP_COUNT; i += 4) {
    if (PCONFIG_ULONG(i / 4) != 0) { // Any input event enabled?
      const uint32_t read = readBuffer[i + 3] << 24 | readBuffer[i + 2] << 16 | readBuffer[i + 1] << 8 | readBuffer[i + 0];
      const uint32_t prev = prevBuffer[i + 3] << 24 | prevBuffer[i + 2] << 16 | prevBuffer[i + 1] << 8 | prevBuffer[i + 0];

      for (uint8_t j = 0; j < 32; j++) {                                                  // Check all 32 bits
        if (bitRead(PCONFIG_ULONG(i / 4), j) && (bitRead(read, j) != bitRead(prev, j))) { // Event enabled and bit changed?
          sendInputEvent(event, i, j, bitRead(read, j));                                  // Send out new state
        }
      }
    }
    delay(0);
  }
}

void P129_data_struct::sendInputEvent(struct EventStruct *event,
                                      uint8_t             group,
                                      uint8_t             bit,
                                      uint8_t             state) {
  String send;
  const uint8_t pin  = (group * 8) + bit + 1; // 1..128
  const uint8_t chip = group + (bit / 8) + 1; // 1..16
  const uint8_t port = (bit % 8);             // 0..7

  send.reserve(40);
  send += getTaskDeviceName(event->TaskIndex);

  if (P129_CONFIG_FLAGS_GET_SEPARATE_EVENTS) {
    send += '#';
    send += pin;
  }
  send += '=';
  send += state;
  send += ',';
  send += chip;
  send += ',';
  send += port;
  send += ',';
  send += pin;
  eventQueue.addMove(std::move(send));
}

#endif // ifdef USES_P129
