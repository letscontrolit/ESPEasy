#include "../PluginStructs/P177_data_struct.h"

#ifdef USES_P177


P177_data_struct::P177_data_struct(struct EventStruct *event) {
  _sendEvents     = P177_GENERATE_EVENTS == 1;
  _rawData        = P177_RAW_DATA == 1;
  _ignoreNegative = P177_IGNORE_NEGATIVE == 1;
}

P177_data_struct::~P177_data_struct() {}

bool P177_data_struct::plugin_ten_per_second(struct EventStruct *event) {
  bool is_ok;
  uint8_t byteData;

  if (P177_SensorMode_e::IdleMode == _sensorMode) {
    _cycles--;

    if (_cycles) { // Skip until counted down to 0
      return false;
    }

    // we're ignoring the 'bit 3' status info, as it's unclear if it's 0-based or 1-based bit 3...
    // a 100 msec 'delay' (10/sec) after the start of a read should be enough to stabilize the data

    # if P177_CHECK_READY_BIT
    byteData = I2C_read8_reg(P177_I2C_ADDR, P177_COMMAND_REG, &is_ok);

    if (is_ok && (0 == bitRead(byteData, 3))) // Not currently reading
    # endif // if P177_CHECK_READY_BIT
    {
      I2C_write8_reg(P177_I2C_ADDR, P177_COMMAND_REG, P177_START_READ);
      _sensorMode = P177_SensorMode_e::SensingMode;
      _cycles     = P177_SKIP_CYCLES; // Restart timer after a read is started
    }
    # if P177_LOG
    addLog(LOG_LEVEL_INFO, strformat(F("P177 : 1 Sensormode: %d, ok: %d, status: 0x%02x"),
                                     static_cast<uint8_t>(_sensorMode), is_ok, byteData));
    # endif // if P177_LOG
    return false;
  }

  if (P177_SensorMode_e::SensingMode == _sensorMode) {
    byteData = I2C_read8_reg(P177_I2C_ADDR, P177_COMMAND_REG, &is_ok);

    # if P177_LOG
    addLog(LOG_LEVEL_INFO, strformat(F("P177 : 2 Sensormode: %d, ok: %d, status: 0x%02x"),
                                     static_cast<uint8_t>(_sensorMode), is_ok, byteData));
    # endif // if P177_LOG

    // if (!is_ok || (1 == bitRead(byteData, 3))) { // Currently reading
    //   return false;
    // }
    _sensorMode = P177_SensorMode_e::ReportingMode;
  }

  # if P177_LOG
  addLog(LOG_LEVEL_INFO, concat(F("P177 : 3 Sensormode: "), static_cast<uint8_t>(_sensorMode)));
  # endif // if P177_LOG
  uint32_t prData{};
  uint32_t tmData{};

  # if P177_LOG
  String log = F("P177 : RAW data: ");
  # endif // if P177_LOG
  size_t i = 0;

  for (; i < 3; ++i) { // first 3 bytes pressure
    prData <<= 8;
    byteData = I2C_read8_reg(P177_I2C_ADDR, P177_DATA_REG_1 + i);
    prData  += byteData;
    # if P177_LOG
    log += strformat(F("0x%02X "), byteData);
    # endif // if P177_LOG
  }
  # if P177_LOG
  log += strformat(F("prData: 0x%06X, "), prData);
  # endif // if P177_LOG

  for (; i < P177_DATA_BYTES; ++i) { // 2 more bytes temperature
    tmData <<= 8;
    byteData = I2C_read8_reg(P177_I2C_ADDR, P177_DATA_REG_1 + i);
    tmData  += byteData;
    # if P177_LOG
    log += strformat(F("0x%02X "), byteData);
    # endif // if P177_LOG
  }

  # if P177_LOG
  log += strformat(F(" tmData: 0x%04X"), tmData);
  addLog(LOG_LEVEL_INFO, log);
  # endif // if P177_LOG

  _sensorMode = P177_SensorMode_e::IdleMode; // Start a new cycle
  const bool pressureChanged = (prData != _rawPressure);

  if (!pressureChanged && (tmData == _rawTemperature)) {
    return false;
  }
  _rawPressure    = prData;
  _rawTemperature = tmData;
  _updated        = true;

  if (_sendEvents && pressureChanged) {
    plugin_read(event);
    sendData(event);
    _updated = true; // Is reset in plugin_read()
  }
  return true;
}

bool P177_data_struct::plugin_read(struct EventStruct *event) {
  if (!_updated) {
    // Needed to get the 1st value, or can get the latest value when sampled too frequently.
    plugin_ten_per_second(event);
  }

  if (_updated) {
    int tmpPressure                       = _rawPressure; // Unsigned to signed
    int tmpTemperature                    = _rawTemperature;
    ESPEASY_RULES_FLOAT_TYPE _pressure    = tmpPressure;
    ESPEASY_RULES_FLOAT_TYPE _temperature = tmpTemperature;

    if (tmpPressure >= (1 << 23)) { // Negative value?
      if (_ignoreNegative) {
        _pressure = 0.0;
      } else {
        _pressure = tmpPressure - (1 << 24);
      }
    }

    if (tmpTemperature >= (1 << 15)) { // Negative value?
      _temperature = tmpTemperature - (1 << 16);
    }

    if (!essentiallyZero(_temperature)) {
      _temperature /= 256.0; // Move less significant bits
    }

    if (!_rawData) {
      _pressure *= (P177_PRESSURE_SCALE_FACTOR / static_cast<ESPEASY_RULES_FLOAT_TYPE>(1 << 23));

      if (P177_TEMPERATURE_OFFSET != 0) {
        _temperature += (P177_TEMPERATURE_OFFSET / 10.0); // In 0.1 degree steps
      }
    }

    UserVar.setFloat(event->TaskIndex, 0, _temperature);
    UserVar.setFloat(event->TaskIndex, 1, _pressure);
    _updated = false;
    return true;
  }
  return false;
}

#endif // ifdef USES_P177
