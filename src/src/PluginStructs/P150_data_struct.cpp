#include "../PluginStructs/P150_data_struct.h"

#ifdef USES_P150

P150_data_struct::P150_data_struct(struct EventStruct *event) {
  _deviceAddress     = P150_I2C_ADDRESS;
  _config            = P150_CONFIG;
  _temperatureOffset = P150_TEMPERATURE_OFFSET / 10.0f; // Per 0.1 degree
  _rawEnabled        = P150_GET_OPT_ENABLE_RAW;
  _logEnabled        = P150_GET_OPT_ENABLE_LOG;
  # if P150_USE_EXTRA_LOG
  _extraLog = P150_GET_OPT_EXTRA_LOG;
  # endif // if P150_USE_EXTRA_LOG
}

/**
 * init(): Initialize sensor
 * - Set config
 * - Set temperature offset if <> 0
 * Will fail if no sensor is found, or the device id doesn't match (0x0117)
 */
bool P150_data_struct::init() {
  // make sure the TMP will acknowledge over I2C
  if ((_deviceAddress < 0) || (I2C_wakeup(_deviceAddress) != 0)) {
    return false;
  }

  uint16_t deviceID = I2C_read16_reg(_deviceAddress, TMP117_DEVICE_ID); // reads registers into rawData

  // make sure the device ID reported by the TMP is correct
  // should always be 0x0117
  if (deviceID != TMP117_DEVICE_ID_VALUE) {
    # ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_ERROR, concat(F("TMP117: Device ID mismatch: "), formatToHex(deviceID, 4)));
    # endif // ifndef BUILD_NO_DEBUG
    return false;
  }

  setConfig();

  if (!essentiallyZero(_temperatureOffset)) {
    setTemperatureOffset(_temperatureOffset); // set offset
  }

  return true;                                // returns true if all the checks pass
}

/**
 * Set configuration, also used to start a one-shot conversion
 */
void P150_data_struct::setConfig() {
  uint16_t config = I2C_read16_reg(_deviceAddress, TMP117_CONFIGURATION); // Read current configuration

  config &= P150_CONFIG_RESET_MASK;                                       // Reset the bits we want to update
  config |= _config;                                                      // Apply (new) config settings
  I2C_write16_reg(_deviceAddress, TMP117_CONFIGURATION, config);          // Update config
}

/**
 * plugin_read(): Fetch last read values
 */
bool P150_data_struct::plugin_read(struct EventStruct *event) {
  if (_readValid && definitelyGreaterThan(_finalTempC, -256.0f)) {
    UserVar.setFloat(event->TaskIndex, 0, _finalTempC);
    UserVar.setFloat(event->TaskIndex, 1, _digitalTempC);

    if (_logEnabled && loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = strformat(F("TMP117: Temperature: %sC"), formatUserVarNoCheck(event, 0).c_str());

      if (_rawEnabled) {
        log += concat(F(", Raw: "), formatUserVarNoCheck(event, 1));
      }
      addLogMove(LOG_LEVEL_INFO, log);
    }

    if (P150_GET_CONF_CONVERSION_MODE == P150_CONVERSION_ONE_SHOT) {
      setConfig(); // Start the next one-shot measurement
    }
    return true;
  }
  return false;
}

/**
 * plugin_once_a_second(): Read sensor if data is available
 * Will return true if data is actually available and valid
 */
bool P150_data_struct::plugin_once_a_second(struct EventStruct *event) {
  if (dataReady()) {
    readTemp();
    _readValid = true;

    # if P150_USE_EXTRA_LOG

    if (_extraLog && loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = strformat(F("TMP117: Read temp: %.2fC, raw: %d"),
                             _finalTempC,
                             _digitalTempC);

      if (P150_GET_CONF_CONVERSION_MODE == P150_CONVERSION_ONE_SHOT) {
        log += F(" (one-shot)");
      }
      addLog(LOG_LEVEL_INFO, log);
    }
    # endif // if P150_USE_EXTRA_LOG
  }
  return false;
}

/**
 * Private: readTemp()
 * Read raw temperature value and calculate final temperature
 */
float P150_data_struct::readTemp() {
  _digitalTempC = I2C_read16_reg(_deviceAddress, TMP117_TEMP_RESULT);

  _finalTempC = _digitalTempC * TMP117_RESOLUTION; // Multiplied by the resolution for digital to final temp

  return _finalTempC;
}

void P150_data_struct::setTemperatureOffset(float offset) {
  const int16_t resolutionOffset = offset / TMP117_RESOLUTION;           // Divide by resolution to send to the sensor

  I2C_write16_reg(_deviceAddress, TMP117_TEMP_OFFSET, resolutionOffset); // Write to the offset temperature register
}

/**
 * Private: dataReady()
 * Check if sensor os ready processing the measurement(s)
 */
bool P150_data_struct::dataReady() {
  const uint16_t response = I2C_read16_reg(_deviceAddress, TMP117_CONFIGURATION);

  // If statement to see if the 13th bit of the register is 1 or not
  return bitRead(response, 13);
}

#endif // ifdef USES_P150
