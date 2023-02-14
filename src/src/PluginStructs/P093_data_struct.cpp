#include "../PluginStructs/P093_data_struct.h"

#ifdef USES_P093

/*
 *
 * Bi-directional communication with the heat pump.
 *
 * Should support all Mitsubishi HP units with CN105 connector.
 *
 * Plugin is based on "Arduino library to control Mitsubishi Heat Pumps" from
 * https://github.com/SwiCago/HeatPump.
 *
 */

P093_data_struct::P093_data_struct(const ESPEasySerialPort port, const int16_t serialRx, const int16_t serialTx, bool includeStatus) :
  _serial(port, serialRx, serialTx),
  _state(NotConnected),
  _fastBaudRate(false),
  _readPos(0),
  _writeTimeout(0),
  _infoModeIndex(0),
  _statusUpdateTimeout(0),
  _tempMode(false),
  _wideVaneAdj(false),
  _valuesInitialized(false),
  _includeStatus(includeStatus) {}

void P093_data_struct::init() {
  setState(Connecting);
}

bool P093_data_struct::sync() {
  if (_statusUpdateTimeout != 0) {
    if (_writeStatus.isDirty()) {
      cancelWaitingAndTransitTo(ApplyingSettings);
      return false;
    }

    if (timeOutReached(_statusUpdateTimeout)) {
      cancelWaitingAndTransitTo(UpdatingStatus);
      return false;
    }
  }

  return readIncommingBytes();
}

bool P093_data_struct::read(String& result) const {
  if (_valuesInitialized == false) {
    return false;
  }

  result.reserve(150);

  // FIXME TD-er: See if this macro can be simpler as it does expand to quite some code which is not changing.
    # define map_list(x, list) findByValue(x, list, sizeof(list) / sizeof(Tuple))

  result  = F("{\"roomTemperature\":");
  result += toString(_currentValues.roomTemperature, 1);
  result += F(",\"wideVane\":\"");
  result += map_list(_currentValues.wideVane, _mappings.wideVane);
  result += F("\",\"power\":\"");
  result += map_list(_currentValues.power, _mappings.power);
  result += F("\",\"mode\":\"");
  result += map_list(_currentValues.mode, _mappings.mode);
  result += F("\",\"fan\":\"");
  result += map_list(_currentValues.fan, _mappings.fan);
  result += F("\",\"vane\":\"");
  result += map_list(_currentValues.vane, _mappings.vane);
  result += F("\",\"iSee\":");
  result += boolToString(_currentValues.iSee);

  if (_includeStatus) {
    result += F(",\"operating\":");
    result += boolToString(_currentValues.operating);
    result += F(",\"compressorFrequency\":");
    result += _currentValues.compressorFrequency;
  }
  result += F(",\"temperature\":");
  result += toString(_currentValues.temperature, 1) + '}';

    # undef map_list

  return true;
}

void P093_data_struct::write(const String& command, const String& value) {
    # define lookup(x, list, placeholder) findByMapping(x, list, sizeof(list) / sizeof(Tuple), placeholder)

  if (equals(command, F("temperature"))) {
    float temperature = 0;

    if (string2float(value, temperature) && (temperature >= 16) && (temperature <= 31)) {
      _wantedSettings.temperature = temperature;
      _writeStatus.set(Temperature);
    }
  } else if ((equals(command, F("power"))) && lookup(value, _mappings.power, _wantedSettings.power)) {
    _writeStatus.set(Power);
  } else if ((equals(command, F("mode"))) && lookup(value, _mappings.mode, _wantedSettings.mode)) {
    _writeStatus.set(Mode);
  } else if ((equals(command, F("fan"))) && lookup(value, _mappings.fan, _wantedSettings.fan)) {
    _writeStatus.set(Fan);
  } else if ((equals(command, F("vane"))) && lookup(value, _mappings.vane, _wantedSettings.vane)) {
    _writeStatus.set(Vane);
  } else if ((equals(command, F("widevane"))) && lookup(value, _mappings.wideVane, _wantedSettings.wideVane)) {
    _writeStatus.set(WideVane);
  }

    # undef lookup
}

void P093_data_struct::setState(P093_data_struct::State newState) {
  if ((newState != _state) && shouldTransition(_state, newState)) {
    State currentState = _state;
    _state = newState;
    didTransition(currentState, newState);
  } else {
# ifdef PLUGIN_093_DEBUG
    addLog(LOG_LEVEL_DEBUG, String(F("M-AC: SS - ignoring ")) +
           stateToString(_state) + F(" -> ") + stateToString(newState));
# endif // ifdef PLUGIN_093_DEBUG
  }
}

bool P093_data_struct::shouldTransition(P093_data_struct::State from, P093_data_struct::State to) {
  switch (to) {
    case Connecting:
      return from == NotConnected || from == ReadTimeout;

    case Connected:
      return from == Connecting;

    case UpdatingStatus:
      return from == Connected || from == StatusUpdated || from == WaitingForScheduledStatusUpdate;

    case StatusUpdated:
      return from == UpdatingStatus;

    case ScheduleNextStatusUpdate:
      return from == StatusUpdated || from == SettingsApplied;

    case WaitingForScheduledStatusUpdate:
      return from == ScheduleNextStatusUpdate;

    case ApplyingSettings:
      return from == WaitingForScheduledStatusUpdate;

    case SettingsApplied:
      return from == ApplyingSettings;

    case ReadTimeout:
      return from == UpdatingStatus || from == ApplyingSettings || from == Connecting;

    default:
      return false;
  }
}

void P093_data_struct::didTransition(P093_data_struct::State from, P093_data_struct::State to) {
# ifdef PLUGIN_093_DEBUG
  addLog(LOG_LEVEL_DEBUG, String(F("M-AC: didTransition: ")) +
         stateToString(from) + " -> " + stateToString(to));
# endif // ifdef PLUGIN_093_DEBUG

  switch (to) {
    case ReadTimeout:

      // Try to connect using different boud rate if we don't get response while connecting.
      if (from == Connecting) {
        _fastBaudRate = !_fastBaudRate;
      }
      setState(Connecting);
      break;

    case Connecting:
      connect();
      break;

    case Connected:
      responseReceived();
      _infoModeIndex = 0;
      setState(UpdatingStatus);
      break;

    case UpdatingStatus:
      updateStatus();
      break;

    case StatusUpdated: {
      responseReceived();
      const int infoModeCount = _includeStatus ? sizeof(INFOMODE) : sizeof(INFOMODE) - 1;
      _infoModeIndex = (_infoModeIndex + 1) % infoModeCount;

      if (_infoModeIndex != 0) {
        setState(UpdatingStatus);
      } else {
        _valuesInitialized = true;
        setState(ScheduleNextStatusUpdate);
      }
      break;
    }

    case ScheduleNextStatusUpdate:
      _statusUpdateTimeout = millis() + 1000;
      setState(WaitingForScheduledStatusUpdate);
      break;

    case ApplyingSettings:
      // Let's be optimistic and apply local changes immediately so potential
      // read operation will fetch latest settings even if they are in the
      // process of being applied. If settings will fail to apply for some reason
      // then next status update will re-set correct values (values from AC unit).
      applySettingsLocally();
      applySettings();
      _writeStatus.clear();
      break;

    case SettingsApplied:
      responseReceived();
      setState(ScheduleNextStatusUpdate);
      break;

    default:
      break;
  }
}

void P093_data_struct::applySettingsLocally() {
  if (_writeStatus.isDirty(Power)) {
    _currentValues.power = _wantedSettings.power;
  }

  if (_writeStatus.isDirty(Mode)) {
    _currentValues.mode = _wantedSettings.mode;
  }

  if (_writeStatus.isDirty(Temperature)) {
    _currentValues.temperature = _wantedSettings.temperature;
  }

  if (_writeStatus.isDirty(Fan)) {
    _currentValues.fan = _wantedSettings.fan;
  }

  if (_writeStatus.isDirty(Vane)) {
    _currentValues.vane = _wantedSettings.vane;
  }

  if (_writeStatus.isDirty(WideVane)) {
    _currentValues.wideVane = _wantedSettings.wideVane;
  }
}

void P093_data_struct::cancelWaitingAndTransitTo(P093_data_struct::State state) {
  _statusUpdateTimeout = 0;
  setState(state);
}

void P093_data_struct::responseReceived() {
  _writeTimeout = 0;
}

void P093_data_struct::updateStatus() {
  # ifdef PLUGIN_093_DEBUG
  addLog(LOG_LEVEL_DEBUG, String(F("M-AC: US: ")) + _infoModeIndex);
  #endif

  uint8_t packet[PACKET_LEN] = { 0xfc, 0x42, 0x01, 0x30, 0x10 };

  packet[5] = INFOMODE[_infoModeIndex];
  memset(packet + 6, 0, 15);
  packet[21] = checkSum(packet, 21);

  sendPacket(packet, PACKET_LEN);
}

void P093_data_struct::applySettings() {
  uint8_t packet[PACKET_LEN] = { 0xfc, 0x41, 0x01, 0x30, 0x10, 0x01 };

  memset(packet + 6, 0, 15);

  if (_writeStatus.isDirty(Power)) {
    packet[8]  = _wantedSettings.power;
    packet[6] |= 0x01;
  }

  if (_writeStatus.isDirty(Mode)) {
    packet[9]  = _wantedSettings.mode;
    packet[6] |= 0x02;
  }

  if (_writeStatus.isDirty(Temperature)) {
    packet[6] |= 0x04;

    if (_tempMode) {
      packet[19] = static_cast<uint8_t>(_wantedSettings.temperature * 2.0f + 128.0f);
    } else {
      packet[10] = 31 - _wantedSettings.temperature;
    }
  }

  if (_writeStatus.isDirty(Fan)) {
    packet[11] = _wantedSettings.fan;
    packet[6] |= 0x08;
  }

  if (_writeStatus.isDirty(Vane)) {
    packet[12] = _wantedSettings.vane;
    packet[6] |= 0x10;
  }

  if (_writeStatus.isDirty(WideVane)) {
    packet[18] = _wantedSettings.wideVane | (_wideVaneAdj ? 0x80 : 0x00);
    packet[7] |= 0x01;
  }

  packet[21] = checkSum(packet, 21);

  sendPacket(packet, PACKET_LEN);
}

void P093_data_struct::connect() {
  # ifdef PLUGIN_093_DEBUG
  addLog(LOG_LEVEL_DEBUG, String(F("M-AC: Connect ")) + getBaudRate());
  #endif

  _serial.begin(getBaudRate(), SERIAL_8E1);
  const uint8_t buffer[] = { 0xfc, 0x5a, 0x01, 0x30, 0x02, 0xca, 0x01, 0xa8 };

  sendPacket(buffer, sizeof(buffer));
}

unsigned long P093_data_struct::getBaudRate() const {
  return _fastBaudRate ? 9600 : 2400;
}

void P093_data_struct::sendPacket(const uint8_t *packet, size_t size) {
# ifdef PLUGIN_093_DEBUG
  addLog(LOG_LEVEL_DEBUG_MORE, dumpOutgoingPacket(packet, size));
# endif // ifdef PLUGIN_093_DEBUG

  _serial.write(packet, size);
  _writeTimeout = millis() + 2000;
}

void P093_data_struct::addByteToReadBuffer(uint8_t value) {
  if (_readPos < READ_BUFFER_LEN) {
    _readBuffer[_readPos] = value;
    ++_readPos;
  } else {
    # ifdef PLUGIN_093_DEBUG
    addLog(LOG_LEVEL_DEBUG, F("M-AC: ABTRB(0)"));
    #endif
    _readPos = 0;
  }
}

bool P093_data_struct::readIncommingBytes() {
  if ((_writeTimeout != 0) && timeOutReached(_writeTimeout)) {
    _writeTimeout = 0;
    _readPos      = 0;
    setState(ReadTimeout);
    return false;
  }

  static const uint8_t DATA_LEN_INDEX = 4;

  while (_serial.available() > 0) {
    uint8_t value = _serial.read();

    if (_readPos == 0) {
      // Wait for start uint8_t.
      if (value == 0xfc) {
        addByteToReadBuffer(value);
      } else {
        # ifdef PLUGIN_093_DEBUG
        addLog(LOG_LEVEL_DEBUG, String(F("M-AC: RIB(0) ")) + formatToHex(value));
        #endif
      }
    } else if ((_readPos <= DATA_LEN_INDEX) || (_readPos <= DATA_LEN_INDEX + _readBuffer[DATA_LEN_INDEX])) {
      // Read header + data part - data length is at index 4.
      addByteToReadBuffer(value);
    } else {
      // Done, last uint8_t is checksum.
      uint8_t length = _readPos;
      _readPos = 0;
      return processIncomingPacket(_readBuffer, length, value);
    }
  }

  return false;
}

bool P093_data_struct::processIncomingPacket(const uint8_t *packet, uint8_t length, uint8_t checksum) {
  P093_data_struct::State state = checkIncomingPacket(packet, length, checksum);

  if (state == StatusUpdated) {
    static const uint8_t dataPartOffset = 5;

    if (length <= dataPartOffset) {
      return false;
    }

    Values values = _currentValues;

    if (parseValues(_readBuffer + dataPartOffset, length - dataPartOffset)) {
      setState(StatusUpdated);
      return values != _currentValues;
    }
  } else if (state != Invalid) {
    setState(state);
  }

  return false;
}

bool P093_data_struct::parseValues(const uint8_t *data, size_t length) {
  if (length == 0) {
    # ifdef PLUGIN_093_DEBUG
    addLog(LOG_LEVEL_DEBUG, F("M-AC: PV(0)"));
    #endif
    return false;
  }

  switch (data[0]) {
    case 0x02:

      if (length > 11) {
        _currentValues.power = data[3];
        _currentValues.iSee  = data[4] > 0x08 ? true : false;
        _currentValues.mode  = _currentValues.iSee ? (data[4] - 0x08) : data[4];

        if (data[11] != 0x00) {
          _currentValues.temperature = (static_cast<float>(data[11]) - 128.0f) / 2.0f;
          _tempMode                  = true;
        } else {
          _currentValues.temperature = 31 - data[5];
        }

        _currentValues.fan      = data[6];
        _currentValues.vane     = data[7];
        _currentValues.wideVane = data[10] & 0x0F;
        _wideVaneAdj            = (data[10] & 0xF0) == 0x80 ? true : false;

        return true;
      }
      break;

    case 0x03:

      if (length > 6) {
        if (data[6] != 0x00) {
          _currentValues.roomTemperature = (static_cast<float>(data[6]) - 128.0f) / 2.0f;
        } else {
          _currentValues.roomTemperature = data[3] + 10;
        }
        return true;
      }
      break;

    case 0x06:

      if (length > 4) {
        _currentValues.operating           = data[4];
        _currentValues.compressorFrequency = data[3];
        return true;
      }
      break;
  }
  # ifdef PLUGIN_093_DEBUG
  addLog(LOG_LEVEL_DEBUG, F("M-AC: PV(1)"));
  #endif
  return false;
}

P093_data_struct::State P093_data_struct::checkIncomingPacket(const uint8_t *packet, uint8_t length, uint8_t checksum) {
# ifdef PLUGIN_093_DEBUG
  addLog(LOG_LEVEL_DEBUG_MORE, dumpIncomingPacket(packet, length));
# endif // ifdef PLUGIN_093_DEBUG

  if ((packet[2] != 0x01) || (packet[3] != 0x30)) {
    # ifdef PLUGIN_093_DEBUG
    addLog(LOG_LEVEL_DEBUG, F("M-AC: CIP(0)"));
    #endif
    return Invalid;
  }

  uint8_t calculatedChecksum = checkSum(packet, length);

  if (calculatedChecksum != checksum) {
    # ifdef PLUGIN_093_DEBUG
    addLog(LOG_LEVEL_DEBUG, String(F("M-AC: CIP(1) ")) + calculatedChecksum);
    #endif
    return Invalid;
  }

  switch (packet[1]) {
    case 0x61:
      return SettingsApplied;
    case 0x62:
      return StatusUpdated;
    case 0x7a:
      return Connected;
    default:
      return Invalid;
  }
}

uint8_t P093_data_struct::checkSum(const uint8_t *bytes, size_t length) {
  uint8_t sum = 0;

  for (size_t i = 0; i < length; ++i) {
    sum += bytes[i];
  }
  return (0xfc - sum) & 0xff;
}

const __FlashStringHelper * P093_data_struct::findByValue(uint8_t value, const Tuple list[], size_t count) {
  for (size_t index = 0; index < count; ++index) {
    const Tuple& tuple = list[index];

    if (value == tuple.value) {
      return tuple.mapping;
    }
  }
  return list[0].mapping;
}

bool P093_data_struct::findByMapping(const String& mapping, const Tuple list[], size_t count, uint8_t& value) {
  for (size_t index = 0; index < count; ++index) {
    const Tuple& tuple = list[index];

    if (mapping.equals(tuple.mapping)) {
      value = tuple.value;
      return true;
    }
  }
  return false;
}

  # ifdef PLUGIN_093_DEBUG
const __FlashStringHelper * P093_data_struct::stateToString_f(P093_data_struct::State state) {
  switch (state) {
    case Invalid: return F("Invalid");
    case NotConnected: return F("NotConnected");
    case Connecting: return F("Connecting");
    case Connected: return F("Connected");
    case UpdatingStatus: return F("UpdatingStatus");
    case StatusUpdated: return F("StatusUpdated");
    case ReadTimeout: return F("ReadTimeout");
    case ScheduleNextStatusUpdate: return F("ScheduleNextStatusUpdate");
    case WaitingForScheduledStatusUpdate: return F("WaitingForScheduledStatusUpdate");
    case ApplyingSettings: return F("ApplyingSettings");
    case SettingsApplied: return F("SettingsApplied");
  }
  return F("");
}

String P093_data_struct::stateToString(P093_data_struct::State state) {
  String res = stateToString_f(state);

  if (res.isEmpty()) {
    return String(F("<unknown> ")) + state;
  }
  return res;
}

void P093_data_struct::dumpPacket(const uint8_t *packet, size_t length, String& result) {
  for (size_t idx = 0; idx < length; ++idx) {
    result += formatToHex(packet[idx], F(""));
    result += ' ';
  }
}

String P093_data_struct::dumpOutgoingPacket(const uint8_t *packet, size_t length) {
  String message = F("M-AC - OUT: ");

  dumpPacket(packet, length, message);
  return message;
}

String P093_data_struct::dumpIncomingPacket(const uint8_t *packet, int length) {
  String message = F("M-AC - IN: ");

  dumpPacket(packet, length, message);
  return message;
}

  # endif // ifdef PLUGIN_093_DEBUG


#endif // ifdef USES_P093
