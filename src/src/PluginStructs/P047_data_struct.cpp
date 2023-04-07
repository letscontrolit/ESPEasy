#include "../PluginStructs/P047_data_struct.h"


#ifdef USES_P047

// **************************************************************************/
// Constructor
// **************************************************************************/
P047_data_struct::P047_data_struct(uint8_t address,
                                   uint8_t model) :
  _address(address), _model(static_cast<P047_SensorModels>(model)) {}

// **************************************************************************/
// PLUGIN_READ
// **************************************************************************/
bool P047_data_struct::plugin_read(struct EventStruct *event) {
  bool success = false;

  if (P047_SENSOR_SLEEP && (P047_ReadMode::ReadStarted != _readMode)) {
    // wake sensor when not reading
    getVersion();
    delayBackground(20); // Seems acceptable to have this relatively short delay
    # ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_DEBUG, F("SoilMoisture->wake"));
    # endif // ifndef BUILD_NO_DEBUG
  }

  switch (_readMode) {
    case P047_ReadMode::NotReading:
    {
      if (P047_CHECK_VERSION) {
        // get sensor version to check if sensor is present
        _sensorVersion = getVersion();

        if (!((_sensorVersion == 0x22) || (_sensorVersion == 0x23) || (_sensorVersion == 0x24) || (_sensorVersion == 0x25) ||
              (_sensorVersion == 0x26))) {
          // invalid sensor
          addLog(LOG_LEVEL_INFO, F("SoilMoisture: Bad Version, no Sensor?"));
          resetSensor();
        }
      }

      // check if we want to change the sensor address
      if (P047_CHANGE_ADDR && (P047_I2C_ADDR != P047_NEW_ADDR) && (0 != P047_NEW_ADDR)) {
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLog(LOG_LEVEL_INFO, concat(F("SoilMoisture: Change Address: "), formatToHex(P047_I2C_ADDR, HEX)) +
                 concat(F(" -> "), formatToHex(P047_NEW_ADDR)));
        }

        if (changeAddress(P047_NEW_ADDR)) {
          P047_I2C_ADDR = P047_NEW_ADDR;
        }
        P047_CHANGE_ADDR = false;
      }

      // start light measurement
      startMeasure();

      _readMode = P047_ReadMode::ReadStarted;

      // 2 s delay ...we need this delay, otherwise we get only the last reading...
      // delayBackground(2000);
      // Schedule the next task-run 2 seconds from now
      Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 2000);
      break;
    }

    case P047_ReadMode::ReadStarted:
    {
      // Get the values
      const float temperature = readTemperature();
      const float moisture    = readMoisture();
      const float light       = readLight();

      if ((temperature > 100.0f) || (temperature < -40.0f) || (moisture > 800.0f) || (moisture < 1.0f) || (light > 65535.0f) ||
          (light < 0.0f)) {
        addLog(LOG_LEVEL_INFO, F("SoilMoisture: Bad Reading, resetting Sensor..."));
        resetSensor();
      }
      else
      {
        UserVar[event->BaseVarIndex]     = temperature;
        UserVar[event->BaseVarIndex + 1] = moisture;
        UserVar[event->BaseVarIndex + 2] = light;

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = concat(F("SoilMoisture: Address: "), formatToHex(P047_I2C_ADDR));

          if (P047_CHECK_VERSION) {
            log += concat(F(" Version: "), formatToHex(_sensorVersion));
          }
          addLogMove(LOG_LEVEL_INFO, log);
          addLogMove(LOG_LEVEL_INFO, concat(F("SoilMoisture: Temperature: "), formatUserVarNoCheck(event, 0)));
          addLogMove(LOG_LEVEL_INFO, concat(F("SoilMoisture: Moisture: "), static_cast<int>(moisture)));

          if (P047_MODEL_CATNIP == _model) {
            addLogMove(LOG_LEVEL_INFO, concat(F("SoilMoisture: Light: "), formatUserVarNoCheck(event, 2)));
          }
        }

        if (P047_SENSOR_SLEEP) {
          // send sensor to sleep
          setToSleep();
          # ifndef BUILD_NO_DEBUG
          addLog(LOG_LEVEL_DEBUG, F("SoilMoisture->sleep"));
          # endif // ifndef BUILD_NO_DEBUG
        }
        success = true;
      }
      _readMode = P047_ReadMode::NotReading;

      break;
    }
  }
  return success;
}

// **************************************************************************/
// Read temperature
// **************************************************************************/
float P047_data_struct::readTemperature() {
  if (P047_MODEL_CATNIP == _model) {
    return I2C_readS16_reg(_address, P047_CATNIP_GET_TEMPERATURE) / 10.0f;
  } else {
    return static_cast<float>(I2C_read8_reg(_address, P047_BEFLE_GET_TEMPERATURE));
  }
}

// **************************************************************************/
// Read light
// **************************************************************************/
float P047_data_struct::readLight() {
  if (P047_MODEL_CATNIP == _model) {
    return I2C_read16_reg(_address, P047_CATNIP_GET_LIGHT);
  }

  // Not supported by BeFlE sensor
  return 0.0f;
}

// **************************************************************************/
// Read moisture
// **************************************************************************/
unsigned int P047_data_struct::readMoisture() {
  if (P047_MODEL_CATNIP == _model) {
    return I2C_read16_reg(_address, P047_CATNIP_GET_CAPACITANCE);
  } else {
    return I2C_read16_reg(_address, P047_BEFLE_GET_CAPACITANCE) >> 8; // Get averaged value
  }
}

// Read Sensor Version
uint8_t P047_data_struct::getVersion() {
  if (P047_MODEL_CATNIP == _model) {
    return I2C_read8_reg(_address, P047_CATNIP_GET_VERSION);
  }

  // Not supported by BeFlE sensor
  return 0;
}

/*----------------------------------------------------------------------*
* Change I2C address of the sensor to the provided address (1..127)    *
* and do a reset after it in order for the new address to become       *
* effective if second parameter is true.                               *
* Method returns true if the new address is set successfully on sensor.*
*----------------------------------------------------------------------*/
bool P047_data_struct::changeAddress(uint8_t new_i2cAddr) {
  uint8_t command;

  if (P047_MODEL_CATNIP == _model) {
    command = P047_CATNIP_SET_ADDRESS;
  } else {
    command = P047_BEFLE_SET_ADDRESS;
  }
  I2C_write8_reg(_address, command, new_i2cAddr);
  I2C_write8_reg(_address, command, new_i2cAddr);

  if (resetSensor()) {
    delayBackground(1000); // FIXME TD-er: Why is this blocking call needed?
  }
  _address = new_i2cAddr;
  return checkAddress(new_i2cAddr);
}

bool P047_data_struct::checkAddress(uint8_t new_i2cAddr) {
  if (P047_MODEL_CATNIP == _model) {
    return I2C_read8_reg(_address, P047_CATNIP_GET_ADDRESS) == new_i2cAddr;
  }

  // Not supported by BeFlE sensor
  return true;
}

// **************************************************************************/
// Reset sensor
// **************************************************************************/
bool P047_data_struct::resetSensor() {
  if (P047_MODEL_CATNIP == _model) {
    I2C_write8(_address, P047_CATNIP_RESET);
    return true;
  }

  // Not supported by BeFlE sensor
  return false;
}

void P047_data_struct::setToSleep() {
  if (P047_MODEL_CATNIP == _model) {
    I2C_write8(_address, P047_CATNIP_SLEEP);
  }

  // Not supported by BeFlE sensor
}

void P047_data_struct::startMeasure() {
  if (P047_MODEL_CATNIP == _model) {
    I2C_write8(_address, P047_CATNIP_MEASURE_LIGHT);
  }

  // Not supported by BeFlE sensor
}

#endif // ifdef USES_P047
