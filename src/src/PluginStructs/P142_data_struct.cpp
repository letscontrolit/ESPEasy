#include "../PluginStructs/P142_data_struct.h"

#ifdef USES_P142

const __FlashStringHelper* Plugin_142_valuename(uint8_t value_nr,
                                                bool    displayString) {
  const __FlashStringHelper *strings[] { /*** ATTENTION: Don't change order of values as these are stored as user-selected!!! ***/
    F("Angle"), F("angle"),              /*** And the order has to match for this list, P142_output_options and getOutputValue() ***/
    F("Magnitude"), F("magnitude"),
    F("Rpm"), F("rpm"),
    F("ReadAngle"), F("readangle"),
    F("RawAngle"), F("rawangle"),
    F("Automatic Gain Control"), F("agc"),
    F("Has magnet"), F("hasmagnet"),
  };
  const size_t index         = (2 * value_nr) + (displayString ? 0 : 1);
  constexpr size_t nrStrings = NR_ELEMENTS(strings);

  if (index < nrStrings) {
    return strings[index];
  }
  return F("");
}

/**************************************************************************
* Constructor
**************************************************************************/
P142_data_struct::P142_data_struct(struct EventStruct *event) {
  _powerMode     = P142_GET_POWER_MODE;
  _hysteresis    = P142_GET_HYSTERESIS;
  _slowFilter    = P142_GET_SLOW_FILTER;
  _fastFilter    = P142_GET_FAST_FILTER;
  _angleOffset   = P142_PRESET_ANGLE;
  _startPosition = P142_START_POSITION;
  _maxPosition   = P142_MAX_POSITION;
}

P142_data_struct::~P142_data_struct() {
  delete as5600;
}

bool P142_data_struct::init(struct EventStruct *event) {
  //
  as5600 = new (std::nothrow) AS5600();

  if ((nullptr != as5600) && as5600->begin()) {
    initialized = true;

    as5600->setPowerMode(_powerMode);
    as5600->setHysteresis(_hysteresis);
    as5600->setSlowFilter(_slowFilter);
    as5600->setFastFilter(_fastFilter);
    as5600->setZPosition(_startPosition);
    as5600->setMPosition(_maxPosition);
    as5600->setOffset(_angleOffset);
  }

  addLog(LOG_LEVEL_INFO, concat(F("AS5600: Initialization "), isInitialized() ? F("succeeded") : F("failed")));
  return isInitialized();
}

/*****************************************************
* plugin_read
*****************************************************/
bool P142_data_struct::plugin_read(struct EventStruct *event) {
  if (isInitialized() && _dataAvailable && (_hasMagnet || !P142_GET_CHECK_MAGNET)) {
    if (setOutputValues(event)) {
      _dataAvailable = false;
      return true;
    }
  }
  return false;
}

bool P142_data_struct::setOutputValues(struct EventStruct *event) {
  bool result             = false;
  const int8_t valueCount = P142_NR_OUTPUT_VALUES;

  result = !P142_GET_UPDATE_DIFF_ONLY;

  for (int8_t i = 0; i < valueCount; ++i) {
    const uint8_t pconfigIndex = i + P142_QUERY1_CONFIG_POS;
    bool isChanged             = false;
    UserVar.setFloat(event->TaskIndex, i, getOutputValue(event, PCONFIG(pconfigIndex), UserVar[event->BaseVarIndex + i], isChanged));

    result |= isChanged;
  }
  return result;
}

float P142_data_struct::getOutputValue(struct EventStruct *event, int idx, float currentValue, bool& changed) {
  float result = NAN;

  if (isInitialized()) {
    if (speedCount > 0) { --speedCount; }

    switch (idx) {
      case P142_OUTPUT_ANGLE:
        result = _angle;
        break;
      case P142_OUTPUT_MAGNET:
        result = _magnitude;
        break;
      case P142_OUTPUT_RPM:
        result = speedCount == 0 ? _speed : currentValue;
        break;
      case P142_OUTPUT_RAW_ANGLE:
        result = _rawAngle;
        break;
      case P142_OUTPUT_READ_ANGLE:
        result = _readAngle;
        break;
      case P142_OUTPUT_AGC:
        result = _agc;
        break;
      case P142_OUTPUT_HAS_MAGNET:
        result = _hasMagnet;
        break;
    }
    changed = result != NAN && !essentiallyEqual(currentValue, result);
    return result;
  }
  return result;
}

bool P142_data_struct::plugin_ten_per_second(struct EventStruct *event) {
  if (isInitialized()) {
    bool  send  = false;
    float speed = as5600->getAngularSpeed(AS5600_MODE_RPM);

    if (speedCount > 0) { --speedCount; }

    if ((0 == speedCount) && !essentiallyEqual(_speed, speed)) {
      _speed         = speed;
      _dataAvailable = true;
    }
    const uint16_t rawAngle  = as5600->rawAngle();
    const uint16_t readAngle = as5600->readAngle();
    const uint8_t  agc       = as5600->readAGC();
    const float    angle     = (AS5600_MODE_DEGREES == P142_GET_OUTPUT_MODE ? AS5600_RAW_TO_DEGREES : AS5600_RAW_TO_RADIANS) * _readAngle;
    const uint16_t magnitude = as5600->readMagnitude();
    const bool     hasMagnet = as5600->detectMagnet();

    if (_rawAngle != rawAngle) {
      _rawAngle      = rawAngle;
      _dataAvailable = true;
    }

    if (_readAngle != readAngle) {
      _readAngle     = readAngle;
      _dataAvailable = true;
    }

    if (!essentiallyEqual(_angle, angle)) {
      _angle         = angle;
      _dataAvailable = true;
    }

    if (_magnitude != magnitude) {
      _magnitude     = magnitude;
      _dataAvailable = true;
    }

    if (hasMagnet != _hasMagnet) {
      _hasMagnet     = hasMagnet;
      _dataAvailable = true;
    }

    if (agc != _agc) {
      _agc           = agc;
      _dataAvailable = true;
    }

    send = _dataAvailable &&
           (_hasMagnet || !P142_GET_CHECK_MAGNET) &&
           setOutputValues(event);

    if (send) {
      sendData(event);
      _dataAvailable = false;
    }

    if (P142_GET_ENABLE_LOG) {
      addLog(LOG_LEVEL_INFO, strformat(F("AS5600: Read: %d, Angle: %.2f, magnitude: %d, speed: %.2f, hasMagnet: %d"),
                                       _readAngle, _angle, _magnitude, _speed, _hasMagnet));
    }
    return true;
  }
  return false;
}

const char P142_subcommands[] PROGMEM = "dir|";

enum class P142_subcmd_e : int8_t {
  invalid = -1,
  dir     = 0,
};

/*****************************************************
* plugin_write
*****************************************************/
bool P142_data_struct::plugin_write(struct EventStruct *event,
                                    String            & string) {
  bool success = false;

  const String command = parseString(string, 1);

  if (isInitialized() && equals(command, F("as5600"))) {
    const String subcommand   = parseString(string, 2);
    const int    subcommand_i = GetCommandCode(subcommand.c_str(), P142_subcommands);

    if (subcommand_i < 0) { return false; } // Fail fast

    const P142_subcmd_e subcmd = static_cast<P142_subcmd_e>(subcommand_i);

    switch (subcmd) {
      case P142_subcmd_e::invalid:
        break;
      case P142_subcmd_e::dir:

        if ((event->Par2 >= 0) && (event->Par2 <= 1)) {
          as5600->setDirection(event->Par2);
          success = true;
        }
        break;
    }
  }
  return success;
}

/*****************************************************
* plugin_get_config_value
*****************************************************/
bool P142_data_struct::plugin_get_config_value(struct EventStruct *event,
                                               String            & string) {
  bool success = false;
  bool changed;

  const String var = parseString(string, 1);

  for (uint8_t v = 0; v < P142_NR_OUTPUT_OPTIONS && !success; ++v) {
    if (var.equals(Plugin_142_valuename(v, false))) {
      # ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        addLog(LOG_LEVEL_DEBUG, strformat(F("AS5600: Get Config Value: %s: %.2f"),
                                          string.c_str(), getOutputValue(event, v, 0.0f, changed)));
      }
      # endif // ifndef BUILD_NO_DEBUG
      string  = toString(getOutputValue(event, v, 0.0f, changed));
      success = true;
      break;
    }
  }

  return success;
}

#endif // ifdef USES_P142
