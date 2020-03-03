#ifdef USES_P092

//#######################################################################################################
//################################ Plugin 090: Mitsubishi Heat Pump #####################################
//#######################################################################################################

//uncomment one of the following as needed
//#ifdef PLUGIN_BUILD_DEVELOPMENT
//#ifdef PLUGIN_BUILD_TESTING

#define PLUGIN_092
#define PLUGIN_ID_092         92
#define PLUGIN_NAME_092       "Mitsubishi AC [TESTING]"
#define PLUGIN_VALUENAME1_092 "settings"

#define PLUGIN_092_DEBUG

static const uint8_t PACKET_LEN = 22;
static const uint8_t READ_BUFFER_LEN = 32;

static const uint8_t INFOMODE[] = {
  0x02, // request a settings packet
  0x03  // request the current room temp
};

struct P092_data_struct : public PluginTaskData_base {
  P092_data_struct(const int16_t serialRx, const int16_t serialTx) :
    _serial(new ESPeasySerial(serialRx, serialTx)),
    _state(NotConnected),
    _fastBaudRate(false),
    _readPos(0),
    _writeTimeout(0),
    _infoModeIndex(0),
    _statusUpdateTimeout(0),
    _tempMode(false),
    _wideVaneAdj(false),
    _valuesInitialized(false) {

    setState(Connecting);
  }

  virtual ~P092_data_struct() {
    delete _serial;
  }

  boolean sync() {
    if (_statusUpdateTimeout != 0) {
      if (_wantedSettings != _currentValues) {
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

  boolean read(String& result) const {
    if (_valuesInitialized == false) {
      return false;
    }

    #define map(x, list) findByValue(x, list, sizeof(list) / sizeof(Tuple))

    result = F("{\"roomTemperature\":");
    result += toString(_currentValues.roomTemperature, 1);
    result += F(",\"wideVane\":\"");
    result += map(_currentValues.wideVane, _mappings.wideVane);
    result += F("\",\"power\":\"");
    result += map(_currentValues.power, _mappings.power);
    result += F("\",\"mode\":\"");
    result += map(_currentValues.mode, _mappings.mode);
    result += F("\",\"fan\":\"");
    result += map(_currentValues.fan, _mappings.fan);
    result += F("\",\"vane\":\"");
    result += map(_currentValues.vane, _mappings.vane);
    result += F("\",\"iSee\":");
    result += boolToString(_currentValues.iSee);
    result += F(",\"temperature\":");
    result += toString(_currentValues.temperature, 1) + '}';

    #undef map

    return true;
  }

  bool write(const String& command, const String& value) {
    bool success = false;

    if (command.isEmpty() || _valuesInitialized == false) {
      return success;
    }

    #define lookup(x, list, placeholder) findByMapping(x, list, sizeof(list) / sizeof(Tuple), placeholder);

    switch (command[0]) {
      case 't':
        if (command == F("temperature")) {
          success = string2float(value, _wantedSettings.temperature);
        }
        break;
      case 'p':
        if (command == F("power")) {
          success = lookup(value, _mappings.power, _wantedSettings.power);
        }
        break;
      case 'm':
        if (command == F("mode")) {
          success = lookup(value, _mappings.mode, _wantedSettings.mode);
        }
        break;
      case 'f':
        if (command == F("fan")) {
          success = lookup(value, _mappings.fan, _wantedSettings.fan);
        }
        break;
      case 'v':
        if (command == F("vane")) {
          success = lookup(value, _mappings.vane, _wantedSettings.vane);
        }
        break;
      case 'w':
        if (command == F("widevane")) {
          success = lookup(value, _mappings.wideVane, _wantedSettings.wideVane);
        }
        break;
    }

    #undef lookup

    return success;
  }

private:
  struct Tuple {
    uint8_t value;
    String mapping;
  };

  struct Mappings {
    Tuple power[2];
    Tuple mode[5];
    Tuple fan[6];
    Tuple vane[7];
    Tuple wideVane[7];

    Mappings() :
      power {
        { 0x00, F("OFF") },
        { 0x01, F("ON") }
      },
      mode {
        { 0x01, F("HEAT") },
        { 0x02, F("DRY") },
        { 0x03, F("COOL") },
        { 0x07, F("FAN") },
        { 0x08, F("AUTO") }
      },
      fan {
        { 0x00, F("AUTO") },
        { 0x01, F("QUIET") },
        { 0x02, F("1") },
        { 0x03, F("2") },
        { 0x05, F("3") },
        { 0x06, F("4") }
      },
      vane {
        { 0x00, F("AUTO") },
        { 0x01, F("1") },
        { 0x02, F("2") },
        { 0x03, F("3") },
        { 0x04, F("4") },
        { 0x05, F("5") },
        { 0x07, F("SWING") }
      },
      wideVane {
        { 0x01, F("<<") },
        { 0x02, F("<") },
        { 0x03, F("|") },
        { 0x04, F(">") },
        { 0x05, F(">>") },
        { 0x08, F("<>") },
        { 0x0C, F("SWING") }
      } {
    }
  };

  enum State {
    Invalid = -1,
    NotConnected = 0,
    Connecting,
    Connected,
    UpdatingStatus,
    StatusUpdated,
    ScheduleNextStatusUpdate,
    WaitingForScheduledStatusUpdate,
    ApplyingSettings,
    SettingsApplied,
    ReadTimeout
  };

  struct Values {
    uint8_t power;
    boolean iSee;
    uint8_t mode;
    float temperature;
    uint8_t fan;
    uint8_t vane;
    uint8_t wideVane;
    float roomTemperature;

    Values() :
      power(0),
      iSee(false),
      mode(0),
      temperature(0),
      fan(0),
      vane(0),
      wideVane(0),
      roomTemperature(0) {
    }

    boolean operator!=(const Values& rhs) const {
      return power != rhs.power ||
        mode != rhs.mode ||
        temperature != rhs.temperature ||
        fan != rhs.fan ||
        vane != rhs.vane ||
        wideVane != rhs.wideVane ||
        iSee != rhs.iSee ||
        roomTemperature != rhs.roomTemperature;
    }
  };

private:
  void setState(State newState) {
    if (newState != _state && shouldTransition(_state, newState)) {
      State currentState = _state;
      _state = newState;
      didTransition(currentState, newState);
    } else {
      addLog(LOG_LEVEL_DEBUG, String(F("M-AC: SS - ignoring ")) +
        stateToString(_state) + F(" -> ") + stateToString(newState));
    }
  }

  static boolean shouldTransition(State from, State to) {
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
        return from == UpdatingStatus || from == Connecting;

      default:
        return false;
    }
  }

  void didTransition(State from, State to) {
    addLog(LOG_LEVEL_DEBUG, String(F("M-AC: didTransition: ")) +
      stateToString(from) + " -> " + stateToString(to));

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

      case StatusUpdated:
        responseReceived();
        _infoModeIndex = (_infoModeIndex + 1) % sizeof(INFOMODE);
        if (_infoModeIndex != 0) {
          setState(UpdatingStatus);
        } else {
          if (_valuesInitialized == false) {
            _wantedSettings = _currentValues;
            _valuesInitialized = true;
          }
          setState(ScheduleNextStatusUpdate);
        }
        break;

      case ScheduleNextStatusUpdate:
        _statusUpdateTimeout = millis() + 1000;
        setState(WaitingForScheduledStatusUpdate);
        break;

      case ApplyingSettings:
        applySettings();
        break;

      case SettingsApplied:
        responseReceived();
        _currentValues = _wantedSettings;
        setState(ScheduleNextStatusUpdate);
        break;

      default:
        break;
    }
  }

  void cancelWaitingAndTransitTo(State state) {
    _statusUpdateTimeout = 0;
    setState(state);
  }

  void responseReceived() {
    _writeTimeout = 0;
  }

  void updateStatus() {
    addLog(LOG_LEVEL_DEBUG, String(F("M-AC: US: ")) + _infoModeIndex);

    uint8_t packet[PACKET_LEN] = { 0xfc, 0x42, 0x01, 0x30, 0x10 };

    packet[5] = INFOMODE[_infoModeIndex];
    memset(packet + 6, 0, 15);
    packet[21] = checkSum(packet, 21);

    sendPacket(packet, PACKET_LEN);
  }

  void applySettings() {
    uint8_t packet[PACKET_LEN] = { 0xfc, 0x41, 0x01, 0x30, 0x10, 0x01 };
    memset(packet + 6, 0, 15);

    if (_wantedSettings.power != _currentValues.power) {
      packet[8] = _wantedSettings.power;
      packet[6] |= 0x01;
    }

    if (_wantedSettings.mode != _currentValues.mode) {
      packet[9] = _wantedSettings.mode;
      packet[6] |= 0x02;
    }

    if (_wantedSettings.temperature != _currentValues.temperature) {
      packet[6] |= 0x04;
      if (_tempMode) {
        packet[19] = (uint8_t)(_wantedSettings.temperature * 2.0f + 128.0f);
      } else {
        //packet[10] = TEMP[lookupByteMapIndex(TEMP_MAP, 16, settings.temperature)];
      }
    }

    if (_wantedSettings.fan != _currentValues.fan) {
      packet[11] = _wantedSettings.fan;
      packet[6] |= 0x08;
    }

    if (_wantedSettings.vane != _currentValues.vane) {
      packet[12] = _wantedSettings.vane;
      packet[6] |= 0x10;
    }

    if (_wantedSettings.wideVane!= _currentValues.wideVane) {
      packet[18] = _wantedSettings.wideVane | (_wideVaneAdj ? 0x80 : 0x00);
      packet[7] |= 0x01;
    }

    packet[21] = checkSum(packet, 21);

    sendPacket(packet, PACKET_LEN);
  }

  void connect() {
    addLog(LOG_LEVEL_DEBUG, String(F("M-AC: Connect ")) + getBaudRate());

    _serial->begin(getBaudRate(), SERIAL_8E1);
    const uint8_t buffer[] = { 0xfc, 0x5a, 0x01, 0x30, 0x02, 0xca, 0x01, 0xa8 };
    sendPacket(buffer, sizeof(buffer));
  }

  unsigned long getBaudRate() const {
    return _fastBaudRate ? 9600 : 2400;
  }

  void sendPacket(const uint8_t *packet, size_t size) {
    addLog(LOG_LEVEL_DEBUG_MORE, dumpOutgoingPacket(packet, size));
    _serial->write(packet, size);
    _writeTimeout = millis() + 2000;
  }

  void addByteToReadBuffer(uint8_t value) {
    if (_readPos < READ_BUFFER_LEN) {
      _readBuffer[_readPos] = value;
      ++_readPos;
    } else {
      addLog(LOG_LEVEL_DEBUG, F("M-AC: ABTRB(0)"));
      _readPos = 0;
    }
  }

  boolean readIncommingBytes() {
    if (_writeTimeout != 0 && timeOutReached(_writeTimeout)) {
      _writeTimeout = 0;
      _readPos = 0;
      setState(ReadTimeout);
      return false;
    }

    static const uint8_t DATA_LEN_INDEX = 4;

    while(_serial->available() > 0) {
      uint8_t value = _serial->read();

      if (_readPos == 0) {
        // Wait for start byte.
        if (value == 0xfc) {
          addByteToReadBuffer(value);
        } else {
          addLog(LOG_LEVEL_DEBUG, String(F("M-AC: RIB(0) ")) + formatToHex(value));
        }
      } else if ((_readPos <= DATA_LEN_INDEX) || (_readPos <= DATA_LEN_INDEX + _readBuffer[DATA_LEN_INDEX])) {
        // Read header + data part - data length is at index 4.
        addByteToReadBuffer(value);
      } else {
        // Done, last byte is checksum.
        uint8_t length = _readPos;
        _readPos = 0;
        return processIncomingPacket(_readBuffer, length, value);
      }
    }

    return false;
  }

  boolean processIncomingPacket(const uint8_t* packet, uint8_t length, uint8_t checksum) {
    State state = checkIncomingPacket(packet, length, checksum);
    switch (state) {
      case StatusUpdated: {
        static const uint8_t dataPartOffset = 5;
        if (length <= dataPartOffset) {
          return false;
        }

        Values values = _currentValues;
        if (parseValues(_readBuffer + dataPartOffset, length - dataPartOffset)) {
          setState(StatusUpdated);
          return values != _currentValues;
        }

        break;
      }

      case SettingsApplied: {
        Values values = _currentValues;
        setState(SettingsApplied);
        return values != _currentValues;;
      }

      default:
        if (state != Invalid) {
          setState(state);
        }
        break;
    }

    return false;
  }

 boolean parseValues(const uint8_t* data, size_t length) {
    if (length == 0) {
      addLog(LOG_LEVEL_DEBUG, F("M-AC: PV(0)"));
      return false;
    }

    switch(data[0]) {
      case 0x02:
        if (length > 11) {
          _currentValues.power = data[3];
          _currentValues.iSee  = data[4] > 0x08 ? true : false;
          _currentValues.mode  = _currentValues.iSee ? (data[4] - 0x08) : data[4];

          if (data[11] != 0x00) {
            _currentValues.temperature = ((float)data[11] - 128.0f) / 2.0f;
            _tempMode = true;
          } else {
            _currentValues.temperature = data[5];
          }

          _currentValues.fan         = data[6];
          _currentValues.vane        = data[7];
          _currentValues.wideVane    = data[10] & 0x0F;
          _wideVaneAdj = (data[10] & 0xF0) == 0x80 ? true : false;

          return true;
        }

      case 0x03:
        if (length > 6) {
          if(data[6] != 0x00) {
            _currentValues.roomTemperature = ((float)data[6] - 128.0f) / 2.0f;
          } else {
            _currentValues.roomTemperature = data[3];
          }
          return true;
        }
    }

    addLog(LOG_LEVEL_DEBUG, F("M-AC: PV(1)"));
    return false;
  }

  static State checkIncomingPacket(const uint8_t* packet, uint8_t length, uint8_t checksum) {
    addLog(LOG_LEVEL_DEBUG_MORE, dumpIncomingPacket(packet, length));

    if (packet[2] != 0x01 || packet[3] != 0x30) {
      addLog(LOG_LEVEL_DEBUG, F("M-AC: CIP(0)"));
      return Invalid;
    }

    uint8_t calculatedChecksum = checkSum(packet, length);
    if (calculatedChecksum != checksum) {
      addLog(LOG_LEVEL_DEBUG, String(F("M-AC: CIP(1) ")) + calculatedChecksum);
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

  static uint8_t checkSum(const uint8_t* bytes, size_t length) {
    uint8_t sum = 0;
    for (size_t i = 0; i < length; ++i) {
      sum += bytes[i];
    }
    return (0xfc - sum) & 0xff;
  }

  static String findByValue(uint8_t value, const Tuple list[], size_t count) {
    for (size_t index = 0; index < count; ++index) {
      const Tuple& tuple = list[index];
      if (value == tuple.value) {
        return tuple.mapping;
      }
    }
    return list[0].mapping;
  }

  static boolean findByMapping(const String& mapping, const Tuple list[], size_t count, uint8_t& value) {
    for (size_t index = 0; index < count; ++index) {
      const Tuple& tuple = list[index];
      if (mapping == tuple.mapping) {
        value = tuple.value;
        return true;
      }
    }
    return false;
  }

  #ifdef PLUGIN_092_DEBUG
  static String stateToString(State state) {
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
    return String(F("<unknown> ")) + state;
  }

  static void dumpPacket(const uint8_t* packet, size_t length, String& result) {
    for (size_t idx = 0; idx < length; ++idx) {
      result += formatToHex(packet[idx], F(""));
      result += ' ';
    }
  }

  static String dumpOutgoingPacket(const uint8_t* packet, size_t length) {
    String message = F("M-AC - OUT: ");
    dumpPacket(packet, length, message);
    return message;
  }

  static String dumpIncomingPacket(const uint8_t* packet, int length) {
    String message = F("M-AC - IN: ");
    dumpPacket(packet, length, message);
    return message;
  }
  #endif

private:
  ESPeasySerial* _serial;
  State _state;
  boolean _fastBaudRate;
  uint8_t _readBuffer[READ_BUFFER_LEN];
  uint8_t _readPos;
  unsigned long _writeTimeout;
  Values _currentValues;
  Values _wantedSettings;
  uint8_t _infoModeIndex;
  unsigned long _statusUpdateTimeout;
  boolean _tempMode;
  boolean _wideVaneAdj;
  boolean _valuesInitialized;
  const Mappings _mappings;
};

boolean Plugin_092(byte function, struct EventStruct *event, String& string) {
  boolean success = false;

  switch (function) {

    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number = PLUGIN_ID_092;
      Device[deviceCount].Type = DEVICE_TYPE_DUAL;
      Device[deviceCount].VType = SENSOR_TYPE_STRING;
      Device[deviceCount].ValueCount = 1;
      Device[deviceCount].SendDataOption = true;
      Device[deviceCount].TimerOption = true;
      Device[deviceCount].TimerOptional = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_092);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES: {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_092));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES: {
      serialHelper_getGpioNames(event);
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG: {
      string += serialHelper_getSerialTypeLabel(event);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      serialHelper_webformLoad(event);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      serialHelper_webformSave(event);
      success = true;
      break;
    }

    case PLUGIN_INIT: {
      initPluginTaskData(event->TaskIndex, new P092_data_struct(CONFIG_PIN1, CONFIG_PIN2));
      success = getPluginTaskData(event->TaskIndex) != nullptr;
      break;
    }

    case PLUGIN_READ: {
      P092_data_struct* heatPump = static_cast<P092_data_struct*>(getPluginTaskData(event->TaskIndex));
      if (heatPump != nullptr) {
        success = heatPump->read(event->String2);
      }
      break;
    }

    case PLUGIN_WRITE: {
      //P092_data_struct* heatPump = static_cast<P092_data_struct*>(getPluginTaskData(event->TaskIndex));
      //if (heatPump != nullptr) {
    //    success = heatPump->write(parseString(string, 1), parseStringKeepCase(string, 2));
    //  }
      break;
    }

    case PLUGIN_EXIT: {
      clearPluginTaskData(event->TaskIndex);
      success = true;
      break;
    }

    case PLUGIN_TEN_PER_SECOND: {
      P092_data_struct* heatPump = static_cast<P092_data_struct*>(getPluginTaskData(event->TaskIndex));
      if (heatPump != nullptr && heatPump->sync()) {
        schedule_task_device_timer(event->TaskIndex, millis() + 10);
      }
      break;
    }
  }

  return success;
}

#endif  // USES_P092
