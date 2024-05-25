#include "../PluginStructs/P170_data_struct.h"

#ifdef USES_P170

/**************************************************************************
* Constructor
**************************************************************************/
P170_data_struct::P170_data_struct(uint8_t level,
                                   bool    log)
  : _level(level), _log(log) {}

/**************************************************************************
* Start the plugin
**************************************************************************/
bool P170_data_struct::init(struct EventStruct *event) {
  if (!Settings.CheckI2Cdevice() || (0 == I2C_wakeup(P170_I2C_ADDRESS_HIGH))) { // Also fail if second address isn't found
    initialized = true;

    if (0 == Settings.TaskDeviceTimer[event->TaskIndex]) {
      Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + P170_DEFAULT_INTERVAL); // Schedule
    }
  } else {
    addLog(LOG_LEVEL_ERROR, F("LQLVL: Initialization error."));
  }
  return isInitialized();
}

/*****************************************************
* plugin_read
*****************************************************/
bool P170_data_struct::plugin_read(struct EventStruct *event)           {
  bool success = false;

  if (isInitialized()) {
    memset(data, 0, sizeof(data));

    if (readLowSteps() &&
        readHighSteps()) {
      steps = getSteps();
      level = steps * P170_MM_PER_STEP;

      UserVar.setFloat(event->TaskIndex, 0, level);
      UserVar.setFloat(event->TaskIndex, 1, steps);

      if ((P170_TRIGGER_LOW_LEVEL > 0) && (level < P170_TRIGGER_LOW_LEVEL)) {
        if (!P170_TRIGGER_ONCE || !lowlevel) {
          eventQueue.add(event->TaskIndex, F("LowLevel"), level);
          lowlevel = true;
        }

        if (level >= P170_TRIGGER_LOW_LEVEL) {
          lowlevel = false;
        }
      }

      if ((P170_TRIGGER_HIGH_LEVEL > 0) && (level > P170_TRIGGER_HIGH_LEVEL)) {
        if (!P170_TRIGGER_ONCE || !highlevel) {
          eventQueue.add(event->TaskIndex, F("HighLevel"), level);
          highlevel = true;
        }

        if (level <= P170_TRIGGER_HIGH_LEVEL) {
          highlevel = false;
        }
      }

      if (0 == Settings.TaskDeviceTimer[event->TaskIndex]) {
        Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + P170_DEFAULT_INTERVAL); // Schedule again
      } else {
        success = true;
      }
    } else {
      addLog(LOG_LEVEL_ERROR, F("LQLVL: Read error."));
    }
  }

  return success;
}

/*****************************************************
* count the number of steps
*****************************************************/
uint8_t P170_data_struct::getSteps() {
  uint8_t result = 0;
  uint8_t max    = 0;

  for (int i = 0; i < P170_TOTAL_STEPS; ++i) {
    if (data[i] >= _level) {
      ++result;
    }

    if (data[i] > max) {
      max = data[i];
    }
  }

  if (_log) {
    String log;
    log.reserve(80);

    for (int i = 0; i < P170_TOTAL_STEPS; ++i) {
      log += data[i];
      log += ',';
    }
    addLog(LOG_LEVEL_INFO, strformat(F("LQLVL: Max level: %d, data: %s result: %d"), max, log.c_str(), result));
  }

  return result;
}

/*****************************************************
* read high part of the data (12 bytes)
*****************************************************/
bool P170_data_struct::readHighSteps() {
  const uint32_t start   = millis();
  const uint8_t  _addr   = P170_I2C_ADDRESS_HIGH;
  const uint8_t  _offset = P170_LOW_STEPS;
  const uint8_t  _size   = P170_HIGH_STEPS;
  bool success           = false;

  Wire.requestFrom(_addr, _size);

  while (_size != Wire.available() && 20 < timePassedSince(start)) {} // Limit waiting time when no device connected

  if (_size == Wire.available()) {
    success = true;

    for (uint8_t i = 0; i < _size; i++) {
      data[i + _offset] = Wire.read(); // receive a byte as character
    }
  }
  return success;
}

/*****************************************************
* read low part of the data (8 bytes)
*****************************************************/
bool P170_data_struct::readLowSteps() {
  const uint32_t start   = millis();
  const uint8_t  _addr   = P170_I2C_ADDRESS;
  const uint8_t  _offset = 0;
  const uint8_t  _size   = P170_LOW_STEPS;
  bool success           = false;

  Wire.requestFrom(_addr, _size);

  while (_size != Wire.available() && 20 < timePassedSince(start)) {} // Limit waiting time when no device connected

  if (_size == Wire.available()) {
    success = true;

    for (uint8_t i = 0; i < _size; i++) {
      data[i + _offset] = Wire.read(); // receive a byte as character
    }
  }
  return success;
}

#endif // ifdef USES_P170
