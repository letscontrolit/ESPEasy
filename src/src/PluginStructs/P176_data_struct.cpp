#include "../PluginStructs/P176_data_struct.h"

#ifdef USES_P176

# include <GPIO_Direct_Access.h>

/**************************************************************************
* Constructor
**************************************************************************/
P176_data_struct::P176_data_struct(struct EventStruct *event) {
  _port         = static_cast<ESPEasySerialPort>(CONFIG_PORT);
  _baud         = P176_SERIAL_BAUDRATE;
  _config       = P176_SERIAL_CONFIG;
  _serialBuffer = P176_SERIAL_BUFFER;
  _rxPin        = CONFIG_PIN1;
  _txPin        = CONFIG_PIN2;
  _ledPin       = P176_GET_LED_PIN;
  _ledInverted  = P176_GET_LED_INVERTED;
  # if P176_HANDLE_CHECKSUM && P176_FAIL_CHECKSUM
  _failChecksum = P176_GET_FAIL_CHECKSUM;
  # endif // if P176_HANDLE_CHECKSUM && P176_FAIL_CHECKSUM
  # if P176_HANDLE_CHECKSUM
  _readUpdated = P176_GET_READ_UPDATED;
  # endif // if P176_HANDLE_CHECKSUM
  # if P176_DEBUG
  _debugLog = P176_GET_DEBUG_LOG;
  # endif // if P176_DEBUG
  _logQuiet = P176_GET_QUIET_LOG;
  _dataLine.reserve(44); // Should be max. line-length sent, including CR/LF
}

P176_data_struct::~P176_data_struct() {
  delete _serial;
}

bool P176_data_struct::init() {
  if (ESPEasySerialPort::not_set != _port) {
    _serial = new (std::nothrow) ESPeasySerial(_port, _rxPin, _txPin, false, _serialBuffer);

    if (nullptr != _serial) {
      # if defined(ESP8266)
      _serial->begin(_baud, (SerialConfig)_config);
      # elif defined(ESP32)
      _serial->begin(_baud, _config);
      # endif // if defined(ESP8266)
      addLog(LOG_LEVEL_INFO, F("Victron: Serial port started"));

      if (validGpio(_ledPin)) {
        DIRECT_PINMODE_OUTPUT(_ledPin);
        DIRECT_pinWrite(_ledPin, _ledInverted ? 1 : 0); // Led off
      } else {
        _ledPin = -1;
      }
    }
  }

  return isInitialized();
}

/*****************************************************
* plugin_read
*****************************************************/
bool P176_data_struct::plugin_read(struct EventStruct *event) {
  bool success = false;

  if (isInitialized()) {
    for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
      String key = getTaskValueName(event->TaskIndex, i);
      key.toLowerCase();
      VictronValue value;

      if (getReceivedValue(key, value) && value.isNumeric()) {
        UserVar.setFloat(event->TaskIndex, i, value.getNumValue());
        success = true;
      }
    }
    # if P176_HANDLE_CHECKSUM

    if (success && (!_readUpdated || (_readUpdated &&
                                      (_successCounter > 0) && (_successCounter != _lastReadCounter)))) {
      _lastReadCounter = _successCounter;
    } else {
      success = false;
    }
    # endif // if P176_HANDLE_CHECKSUM
  }

  return success;
}

// In order of most values included
// mV, mAh
const char p176_factor_1000[] PROGMEM =
  "v|v2|v3|vs|vm|vpv|i|i2|i3|il|ce|h1|h2|h3|h6|h7|h8|h15|h16"
;

// 0.01 kWh, 0.01 V
const char p176_factor_0_01[] PROGMEM =
  "h17|h18|h19|h20|h22|ac_out_v|dc_in_v|"
;

// promille
const char p176_factor_10[] PROGMEM =
  "dm|soc|"
;

// 0.1 A
const char p176_factor_0_1[] PROGMEM =
  "ac_out_i|dc_in_i|"
;

float P176_data_struct::getKeyFactor(const String& key,
                                     int32_t     & nrDecimals) const {
  nrDecimals = 0;

  if (key.isEmpty()) { return 1.0f; }

  if (GetCommandCode(key.c_str(), p176_factor_1000) > -1) {
    nrDecimals = 3;
    return 0.001f;
  }

  if (GetCommandCode(key.c_str(), p176_factor_0_01) > -1) {
    return 100.0f;
  }

  if (GetCommandCode(key.c_str(), p176_factor_10) > -1) {
    nrDecimals = 1;
    return 0.1f;
  }

  if (GetCommandCode(key.c_str(), p176_factor_0_1) > -1) {
    return 10.0f;
  }

  return 1.0f;
}

/*****************************************************
* plugin_fifty_per_second
*****************************************************/
bool P176_data_struct::plugin_fifty_per_second(struct EventStruct *event) {
  bool success = false;

  if (isInitialized()) {
    if (handleSerial()) { // Read and process data
      plugin_read(event); // Update task values
    }
    success = true;
  }

  return success;
}

/*****************************************************
* plugin_get_config_value
*****************************************************/
bool P176_data_struct::plugin_get_config_value(struct EventStruct *event,
                                               String            & string) {
  bool success = false;

  if (isInitialized()) {
    const String key = parseString(string, 1, '.'); // Decimal point as separator, by convention
    VictronValue value;

    # if P176_HANDLE_CHECKSUM

    if (equals(key, F("successcount"))) {
      string  = _successCounter;
      success = true;
    } else
    if (equals(key, F("updated"))) {
      string              = _successCounter != _lastSuccessCounter;
      _lastSuccessCounter = _successCounter;
      success             = true;
    } else
    if (equals(key, F("errorcount"))) {
      string  = _checksumErrors;
      success = true;
    } else
    # endif // if P176_HANDLE_CHECKSUM

    if (getReceivedValue(key, value)) {
      string  = value.getValue();
      success = true;
    }
  }

  return success;
}

/*****************************************************
* getCurrentDataSize
*****************************************************/
size_t P176_data_struct::getCurrentDataSize() const {
  return _data.size();
}

/*****************************************************
* showCurrentData
*****************************************************/
bool P176_data_struct::showCurrentData() const {
  bool success = false;

  if (_data.size() > 0) {
    html_table(EMPTY_STRING); // Sub-table
    html_table_header(F("Name"),  125);
    html_table_header(F("Data"),  250);
    html_table_header(F("Value"), 125);

    for (auto it = _data.begin(); it != _data.end(); ++it) {
      html_TR_TD();
      addHtml(it->second.getName());
      html_TD();
      addHtml(it->second.getRawValue());
      html_TD();

      if (it->second.isNumeric()) {
        addHtml(it->second.getValue());
      }
    }
    html_end_table();
  }
  return success;
}

// Support functions

/*****************************************************
* getReceivedValue
*****************************************************/
bool P176_data_struct::getReceivedValue(const String& key,
                                        VictronValue& value) const {
  bool success = false;

  if (!key.isEmpty()) { // Find is case-sensitive
    # ifdef ESP8266
    const auto it = _data.find(key);

    if (it != _data.end()) {
      value = it->second;
    # else // ifdef ESP8266

    if (_data.contains(key)) {
      value = _data.at(key);
    # endif // ifdef ESP8266
      success = true;
    }

    # if P176_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO) && _debugLog) {
      addLog(LOG_LEVEL_INFO, strformat(F("P176 : getReceivedValue Key:%s, value:%s"),
                                       key.c_str(), value.getRawValue().c_str()));
    }
    # endif // if P176_DEBUG
  }
  return success;
}

/*****************************************************
* handleSerial
*****************************************************/
bool P176_data_struct::handleSerial() {
  bool enough    = false;
  bool result    = false; // True for a successfully received packet, with a correct checksum or _failChecksum = false
  int  available = _serial->available();
  uint8_t ch;

  do {
    if (available > 0) {
      if (_ledPin != -1) {
        DIRECT_pinWrite(_ledPin, _ledInverted ? 0 : 1);
      }

      ch = static_cast<uint8_t>(_serial->read());
      available--;

      # if P176_HANDLE_CHECKSUM
      _checksum += ch;
      # endif // if P176_HANDLE_CHECKSUM

      switch (ch) {
        case '\r': // Ignore as it'll be stripped off
          break;
        case '\n':
          // no need to append before processing, as it's stripped off
          processBuffer(_dataLine); // Store data
          _dataLine.clear();
          enough = true;

          # if P176_HANDLE_CHECKSUM
          #  if P176_DEBUG

          if (loglevelActiveFor(LOG_LEVEL_INFO) && _debugLog) {
            addLog(LOG_LEVEL_INFO, strformat(F("P176 : Checksum state: %d"),
                                             static_cast<uint8_t>(_checksumState)));
          }
          #  endif // if P176_DEBUG

          if (Checksum_state_e::ValidateNext == _checksumState) {
            _checksumState = Checksum_state_e::Validating;
          } else
          if (Checksum_state_e::Starting == _checksumState) { // Start counting after a Checksum (aka 'end of packet') was received
            _checksumState = Checksum_state_e::Counting;
            _checksum      = 0;
            #  if P176_DEBUG

            if (loglevelActiveFor(LOG_LEVEL_INFO) && _debugLog) {
              addLog(LOG_LEVEL_INFO, F("P176 : Start counting for checksum"));
            }
            #  endif // if P176_DEBUG
          }
          # endif // if P176_HANDLE_CHECKSUM
          break;
        case '\t':
          # if P176_HANDLE_CHECKSUM

          if (equals(_dataLine, F("Checksum"))) {
            if (Checksum_state_e::Counting == _checksumState) {
              _checksumState = Checksum_state_e::ValidateNext;
              #  if P176_DEBUG

              if (loglevelActiveFor(LOG_LEVEL_INFO) && _debugLog) {
                addLog(LOG_LEVEL_INFO, F("P176 : Validate next checksum"));
              }
              #  endif // if P176_DEBUG
            } else {
              _checksumState = Checksum_state_e::Starting;
              #  if P176_FAIL_CHECKSUM

              commitTempData(!_failChecksum); // Discard any data received so far, as we don't know their checksum status
              #  endif // if P176_FAIL_CHECKSUM
            }
          }
          # endif // if P176_HANDLE_CHECKSUM
          _dataLine += static_cast<char>(ch); // append after
          break;
        default:
          _dataLine += static_cast<char>(ch);
          break;
      }

      # if P176_HANDLE_CHECKSUM

      if (Checksum_state_e::Validating == _checksumState) {
        if (_checksum != 0) {
          _checksumState = Checksum_state_e::Error; // Error if resulting checksum isn't 0
          _checksumErrors++;
          _checksumDelta = 0;

          if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
            addLog(LOG_LEVEL_ERROR, strformat(F("Victron: Checksum error, expected 0 but got %d (success: %d errors: %d)"),
                                              _checksum, _successCounter, _checksumErrors));
          }
        } else {
          _checksumState = Checksum_state_e::Counting; // New packet is expected, start counting immediately
          _successCounter++;
          _checksumDelta++;
          result = true;

          if (_checksumDelta >= 50) {
            _checksumErrors = 0;
          }


          if (loglevelActiveFor(LOG_LEVEL_INFO) && !_logQuiet) {
            addLog(LOG_LEVEL_INFO, F("Victron: Checksum validated Ok"));
          }
        }
        _checksum = 0; // Clean start

        #  if P176_FAIL_CHECKSUM

        const bool checksumSuccess = (Checksum_state_e::Error != _checksumState) || !_failChecksum;
        commitTempData(checksumSuccess);

        if (checksumSuccess) {
          result = true; // In case of a failed checksum
        }
        #  endif // if P176_FAIL_CHECKSUM
      }
      # endif // if P176_HANDLE_CHECKSUM

      if (_ledPin != -1) {
        DIRECT_pinWrite(_ledPin, _ledInverted ? 1 : 0);
      }
    } else {
      available = _serial->available();
      enough    = available <= 0;
    }
  } while (!enough);
  return result;
}

/*****************************************************
* processBuffer
*****************************************************/
void P176_data_struct::processBuffer(const String& message) {
  if (message.isEmpty()) { return; }
  const String name  = parseStringKeepCase(message, 1, '\t');
  const String value = parseStringToEndKeepCase(message, 2, '\t');
  String key(name);
  key.toLowerCase();

  # if P176_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO) && _debugLog) {
    addLog(LOG_LEVEL_INFO, strformat(F("P176 : Processing data '%s\t%s'"
                                       #  if P176_HANDLE_CHECKSUM
                                       ", checksum: %d"
                                       #  endif // if P176_HANDLE_CHECKSUM
                                       ), name.c_str(), value.c_str()
                                     #  if P176_HANDLE_CHECKSUM
                                     , _checksum
                                     #  endif // if P176_HANDLE_CHECKSUM
                                     ));
  }
  # endif // if P176_DEBUG

  if (!key.isEmpty() && !value.isEmpty() && !equals(key, F("checksum"))) {
    auto it = _data.find(key);

    if (it == _data.end()) {
      int32_t nrDecimals{};
      float   factor = getKeyFactor(key, nrDecimals);
      VictronValue val(name, value, factor, nrDecimals);
      _data[key] = std::move(val);
    } else {
      it->second.set(value);
    }
  }
}

# if P176_FAIL_CHECKSUM

/*****************************************************
* commitTempData
*****************************************************/
bool P176_data_struct::commitTempData(bool checksumSuccess) {
  size_t nrChanged = 0;

  for (auto it = _data.begin(); it != _data.end(); ++it) {
    if (it->second.commitTempData(checksumSuccess)) {
      ++nrChanged;
    }
  }
  #  if P176_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO) && _debugLog) {
    addLog(LOG_LEVEL_INFO, strformat(F("P176 : Moving %d _temp items to _data"), nrChanged));
  }
  #  endif // if P176_DEBUG
  return nrChanged != 0;
}

# endif // if P176_FAIL_CHECKSUM

#endif // ifdef USES_P176
