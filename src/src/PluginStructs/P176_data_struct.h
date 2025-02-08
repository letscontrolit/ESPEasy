#ifndef PLUGINSTRUCTS_P176_DATA_STRUCT_H
#define PLUGINSTRUCTS_P176_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P176

# include <ESPeasySerial.h>

# define P176_DEBUG 1           // Enable some extra (development) logging

# define P176_HANDLE_CHECKSUM 1 // Implement checksum?
# define P176_FAIL_CHECKSUM   1 // Fail/ignore data on checksum errors (optional)?

# define P176_SERIAL_CONFIG             PCONFIG(0)
# define P176_SERIAL_BAUDRATE           PCONFIG_LONG(1)
# define P176_SERIAL_BUFFER             PCONFIG(3)

# define P176_FLAGS                     PCONFIG_ULONG(0)
# define P176_FLAG_LED_PIN              0  // 8 bit
# define P176_FLAG_LED_INVERTED         8  // 1 bit
# define P176_FLAG_FAIL_CHECKSUM        9  // 1 bit
# define P176_FLAG_DEBUG_LOG            10 // 1 bit
# define P176_FLAG_LOG_QUIET            11 // 1 bit
# define P176_FLAG_READ_UPDATED         12 // 1 bit
# define P176_GET_LED_PIN    get8BitFromUL(P176_FLAGS, P176_FLAG_LED_PIN)
# define P176_SET_LED_PIN(N) set8BitToUL(P176_FLAGS, P176_FLAG_LED_PIN, N)
# define P176_GET_LED_INVERTED    bitRead(P176_FLAGS, P176_FLAG_LED_INVERTED)
# define P176_SET_LED_INVERTED(N) bitWrite(P176_FLAGS, P176_FLAG_LED_INVERTED, N)
# if P176_FAIL_CHECKSUM
#  define P176_GET_FAIL_CHECKSUM    bitRead(P176_FLAGS, P176_FLAG_FAIL_CHECKSUM)
#  define P176_SET_FAIL_CHECKSUM(N) bitWrite(P176_FLAGS, P176_FLAG_FAIL_CHECKSUM, N)
# endif // if P176_FAIL_CHECKSUM
# if P176_HANDLE_CHECKSUM
#  define P176_GET_READ_UPDATED    bitRead(P176_FLAGS, P176_FLAG_READ_UPDATED)
#  define P176_SET_READ_UPDATED(N) bitWrite(P176_FLAGS, P176_FLAG_READ_UPDATED, N)
# endif // if P176_HANDLE_CHECKSUM
# if P176_DEBUG
#  define P176_GET_DEBUG_LOG    bitRead(P176_FLAGS, P176_FLAG_DEBUG_LOG)
#  define P176_SET_DEBUG_LOG(N) bitWrite(P176_FLAGS, P176_FLAG_DEBUG_LOG, N)
# endif // if P176_DEBUG
# define P176_GET_QUIET_LOG    bitRead(P176_FLAGS, P176_FLAG_LOG_QUIET)
# define P176_SET_QUIET_LOG(N) bitWrite(P176_FLAGS, P176_FLAG_LOG_QUIET, N)

# define P176_DEFAULT_BAUDRATE          19200
# define P176_DEFAULT_BUFFER            128
# define P176_DEFAULT_FAIL_CHECKSUM     true

struct P176_data_struct : public PluginTaskData_base {
public:

  P176_data_struct(struct EventStruct *event);

  P176_data_struct() = delete;
  virtual ~P176_data_struct();

  bool   init();

  bool   plugin_read(struct EventStruct *event);
  bool   plugin_fifty_per_second(struct EventStruct *event);
  bool   plugin_get_config_value(struct EventStruct *event,
                                 String            & string);
  size_t getCurrentDataSize() const;
  bool   showCurrentData() const;
  bool   isInitialized() const {
    return nullptr != _serial;
  }

  # if P176_HANDLE_CHECKSUM
  uint32_t getSuccessfulPackets() const {
    return _successCounter;
  }

  uint32_t getChecksumErrors() const {
    return _checksumErrors;
  }

  # endif // if P176_HANDLE_CHECKSUM

private:

  struct VictronValue {
    VictronValue() {}

    VictronValue(const String & name,
                 const  String& value,
                 const float  & factor,
                 const int32_t& nrDecimals)
      :_name(name), _factor(factor), _nrDecimals(nrDecimals)
    {
      set(value);
    }

    String getName() const {
      return _name;
    }

    void set(const String& value) {
# if P176_FAIL_CHECKSUM
      _valueTemp = value;
# else // if P176_FAIL_CHECKSUM
      update(value);
# endif // if P176_FAIL_CHECKSUM
    }

private:

    void update(const String& value) {
      _changed = !_value.equals(value);
      _value   = value;

      if (_changed) {
        int32_t iValue = 0;
        _isNumeric = true;

        if (validIntFromString(_value, iValue)) {
          _numValue = iValue * _factor;
        } else
        if (_value.equalsIgnoreCase(F("ON"))) {  // Can be ON or On
          _numValue = 1.0f;
        } else
        if (_value.equalsIgnoreCase(F("OFF"))) { // Can be OFF or Off
          _numValue = 0.0f;
        } else {
          _isNumeric = false;
        }
        _changed = false;
      }
    }

public:

    bool isNumeric() const {
      return _isNumeric;
    }

    String getValue() const {
      if (!_isNumeric) {
        return _value;
      }

      return toString(_numValue, _nrDecimals);
    }

    float getNumValue() const {
      return _numValue;
    }

    String getRawValue() const {
      return _value;
    }

# if P176_FAIL_CHECKSUM
    bool commitTempData(bool checksumSuccess) {
      if (_valueTemp.isEmpty()) {
        return false;
      }

      if (checksumSuccess) {
        update(_valueTemp);
      }
      _valueTemp.clear();
      return checksumSuccess;
    }

# endif // if P176_FAIL_CHECKSUM

private:

    String _name;
    String _value;
# if P176_FAIL_CHECKSUM
    String _valueTemp;
# endif // if P176_FAIL_CHECKSUM
    float   _factor{};
    float   _numValue{};
    int32_t _nrDecimals{};
    bool    _changed   = true;
    bool    _isNumeric = true;
  };

  float getKeyFactor(const String& key,
                     int32_t     & nrDecimals) const;
  bool  getReceivedValue(const String& key,
                         VictronValue& value) const;
  bool  handleSerial();
  void  processBuffer(const String& message);
  # if P176_FAIL_CHECKSUM
  bool  commitTempData(bool checksumSuccess);
  # endif // if P176_FAIL_CHECKSUM

  ESPeasySerial *_serial = nullptr;

  # if P176_HANDLE_CHECKSUM
  uint32_t _checksumErrors     = 0;
  uint32_t _checksumDelta      = 0;
  uint32_t _successCounter     = 0;
  uint32_t _lastSuccessCounter = 0;
  uint32_t _lastReadCounter    = 0;
  bool     _readUpdated        = false;
  # endif // if P176_HANDLE_CHECKSUM
  int               _baud         = P176_DEFAULT_BAUDRATE;
  unsigned int      _serialBuffer = P176_DEFAULT_BUFFER;
  String            _dataLine;
  int8_t            _ledPin      = -1;
  bool              _ledInverted = false;
  ESPEasySerialPort _port        = ESPEasySerialPort::not_set;
  uint8_t           _config      = 0;
  int8_t            _rxPin       = -1;
  int8_t            _txPin       = -1;
  # if P176_DEBUG
  bool _debugLog = false;
  # endif // if P176_DEBUG
  bool _logQuiet = false;

  # if P176_HANDLE_CHECKSUM
  enum class Checksum_state_e: uint8_t {
    Undefined = 0,
    Starting,
    Counting,
    ValidateNext,
    Validating,
    Error,
  };
  Checksum_state_e _checksumState = Checksum_state_e::Undefined;
  uint8_t          _checksum      = 0;
  bool             _failChecksum  = false;
  # endif // if P176_HANDLE_CHECKSUM

  // Key is stored in lower-case as PLUGIN_GET_CONFIG_VALUE passes in the variable name in lower-case
  std::map<String, VictronValue>_data{};
};

#endif // ifdef USES_P176
#endif // ifndef PLUGINSTRUCTS_P176_DATA_STRUCT_H
