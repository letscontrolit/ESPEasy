#include "../PluginStructs/P150_data_struct.h"

#ifdef USES_P150

P150_data_struct::P150_data_struct(struct EventStruct *event) {
  _deviceAddress     = P150_I2C_ADDRESS;
  _config            = P150_FLAGS;
  _temperatureOffset = P150_TEMPERATURE_OFFSET / 10.0f; // Per 0.1 degree
}

/**
 * init(): Initialize sensor
 * - Set config
 * - Set temperature offset if <> 0
 * Will fail if no sensor is found, or the device id doesn't match (0x0117)
 */
bool P150_data_struct::init(struct EventStruct *event) {
  // make sure the TMP will acknowledge over I2C
  Wire.beginTransmission(_deviceAddress);

  if (Wire.endTransmission() != 0) {
    return false;
  }

  uint16_t deviceID = I2C_read16_reg(_deviceAddress, TMP117_DEVICE_ID); // reads registers into rawData

  // make sure the device ID reported by the TMP is correct
  // should always be 0x0117
  if (deviceID != TMP117_DEVICE_ID_VALUE) {
    addLog(LOG_LEVEL_ERROR, concat(F("TMP117: Device ID mismatch: "), formatToHex(deviceID, 4)));
    return false;
  }

  uint16_t config = I2C_read16_reg(_deviceAddress, TMP117_CONFIGURATION); // Read current configuration

  config |= _config;                                                      // Apply config settings
  I2C_write16_reg(_deviceAddress, TMP117_CONFIGURATION, config);          // Update config

  if (!essentiallyZero(_temperatureOffset)) {
    setTemperatureOffset(_temperatureOffset);                             // set offset
  }

  return true;                                                            // returns true if all the checks pass
}

/**
 * plugin_read(): Read sensor if data is available
 * Will only return true if data is actually available and valid
 */
bool P150_data_struct::plugin_read(struct EventStruct *event) {
  if (dataReady()) {
    readTemp();

    if (definitelyGreaterThan(_finalTempC, -256)) {
      UserVar[event->BaseVarIndex]     = _finalTempC;
      UserVar[event->BaseVarIndex + 1] = _digitalTempC;
      return true;
    }
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
  int16_t resolutionOffset = offset / TMP117_RESOLUTION; // Divides by the resolution to send the correct value to the

  // sensor

  I2C_write16_reg(_deviceAddress, TMP117_TEMP_OFFSET, resolutionOffset); // Writes to the offset temperature register with the new offset
                                                                         // value
}

/**
 * Private: dataReady()
 * Check if sensor os ready processing the measurement(s)
 */
bool P150_data_struct::dataReady() {
  uint16_t response = I2C_read16_reg(_deviceAddress, TMP117_CONFIGURATION);

  // If statement to see if the 13th bit of the register is 1 or not
  return response & 1 << 13;
}

#endif // ifdef USES_P150
