#include "P062_data_struct.h"


#ifdef USES_P062
#include "../Helpers/ESPEasy_Storage.h"

P062_data_struct::P062_data_struct() {
#ifdef PLUGIN_062_DEBUG
  addLog(LOG_LEVEL_INFO, F("P062_data_struct constructor"));
#endif
  clearCalibrationData(); // Reset
}

bool P062_data_struct::init(taskIndex_t taskIndex,
                            uint8_t i2c_addr,
                            bool scancode,
                            bool keepCalibrationData) {
#ifdef PLUGIN_062_DEBUG
  addLog(LOG_LEVEL_INFO, F("P062_data_struct init()"));
#endif
  _i2c_addr            = i2c_addr;
  _use_scancode        = scancode;
  _keepCalibrationData = keepCalibrationData;

  if (!keypad) {
    keypad = new Adafruit_MPR121();
  }
  if (keypad) {
    keypad->begin(_i2c_addr);
    loadTouchObjects(taskIndex);
    return true;
  }
  return false;
}

void P062_data_struct::updateCalibration(uint8_t t) {
  if (t >= P062_MaxTouchObjects) return;
  if (_keepCalibrationData) {
    uint16_t current = keypad->filteredData(t);
    CalibrationData.CalibrationValues[t].current = current;
    if (CalibrationData.CalibrationValues[t].min == 0 || current < CalibrationData.CalibrationValues[t].min) {
      CalibrationData.CalibrationValues[t].min = current;
    }
    if (CalibrationData.CalibrationValues[t].max == 0 || current > CalibrationData.CalibrationValues[t].max) {
      CalibrationData.CalibrationValues[t].max = current;
    }
  }
}

bool P062_data_struct::readKey(uint16_t& key) {
  if (!keypad) return false;
  key = keypad->touched();

  if (key)
  {
    uint16_t colMask = 0x01;

    for (byte col = 1; col <= 12; col++)
    {
      if (key & colMask) // this key pressed?
      {
        updateCalibration(col - 1);
        if (_use_scancode) {
          key = col;
          break;
        }
      }
      colMask <<= 1;
    }
  }

  if (keyLast != key)
  {
    keyLast = key;
    return true;
  }
  return false;
}

/**
 * Set all tresholds at once
 */
void P062_data_struct::setThresholds(uint8_t touch, uint8_t release) {
  keypad->setThresholds(touch, release);
}

/**
 * Set a single treshold
 */
void P062_data_struct::setThreshold(uint8_t t, uint8_t touch, uint8_t release) {
  keypad->setThreshold(t, touch, release);
}

/**
 * Load the touch objects from the settings, and initialize then properly where needed.
 */
void P062_data_struct::loadTouchObjects(taskIndex_t taskIndex) {
#ifdef PLUGIN_062_DEBUG
  String log = F("P062 DEBUG loadTouchObjects size: ");
  log += sizeof(StoredSettings);
  addLog(LOG_LEVEL_INFO, log);
#endif // PLUGIN_062_DEBUG
  LoadCustomTaskSettings(taskIndex, (uint8_t *)&(StoredSettings), sizeof(StoredSettings));
}

/**
 * Get the Calibration data for 1 touch object, return false if all zeroes or invalid input for t.
 */
bool P062_data_struct::getCalibrationData(uint8_t t, uint16_t *current, uint16_t *min, uint16_t *max) {
  if (t >= P062_MaxTouchObjects) return false;
  *current = CalibrationData.CalibrationValues[t].current;
  *min     = CalibrationData.CalibrationValues[t].min;
  *max     = CalibrationData.CalibrationValues[t].max;
  return (*current + *min + *max) > 0;
}

/**
 * Reset the touch data.
 */
void P062_data_struct::clearCalibrationData() {
  for (uint8_t t = 0; t < P062_MaxTouchObjects; t++) {
    CalibrationData.CalibrationValues[t].current = 0;
    CalibrationData.CalibrationValues[t].min     = 0;
    CalibrationData.CalibrationValues[t].max     = 0;
  }
}
#endif // ifdef USES_P062
