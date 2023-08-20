#include "../PluginStructs/P113_data_struct.h"

#ifdef USES_P113

P113_data_struct::P113_data_struct(uint8_t i2c_addr, int timing, bool range) : i2cAddress(i2c_addr), timing(timing), range(range) {
  sensor = new (std::nothrow) SFEVL53L1X();
}

P113_data_struct::~P113_data_struct() {
  if (nullptr != sensor) {
    delete sensor;
  }
}

// **************************************************************************/
// Initialize VL53L1X
// **************************************************************************/
bool P113_data_struct::begin() {
  initState = nullptr != sensor;

  if (initState) {
    uint16_t id = sensor->getID();

    // FIXME 2023-08-11 tonhuisman: Disabled, as it seems to mess up the sensor
    // sensor->setI2CAddress(i2cAddress); // Initialize for configured address

    uint8_t res = sensor->begin();

    if (res) { // 0/false is NO-ERROR
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        addLogMove(LOG_LEVEL_ERROR, strformat(F("VL53L1X: Sensor not found, init failed for 0x%02x, id: 0x%04X status: %d"),
                                              i2cAddress, id, res));
      }
      initState = false;
      return initState;
    }

    sensor->setTimingBudgetInMs(timing);

    if (range) {
      sensor->setDistanceModeLong();
    } else {
      sensor->setDistanceModeShort();
    }

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLogMove(LOG_LEVEL_INFO, strformat(F("VL53L1X: Sensor initialized at address 0x%02x, id: 0x%04x"), i2cAddress, id));
    }
  }

  return initState;
}

bool P113_data_struct::startRead() {
  if (initState && !readActive && (nullptr != sensor)) {
    sensor->startRanging();
    readActive = true;
    distance   = -1;
  }
  return readActive;
}

bool P113_data_struct::readAvailable() {
  bool ready = (nullptr != sensor) && sensor->checkForDataReady();

  if (ready) {
    distance = sensor->getDistance();
    sensor->clearInterrupt();
    sensor->stopRanging();

    // readActive = false;
  }
  return ready;
}

uint16_t P113_data_struct::readDistance() {
  success = false;

  # if defined(P113_DEBUG) || defined(P113_DEBUG_DEBUG)
  String log;
  # endif // if defined(P113_DEBUG) || defined(P113_DEBUG_DEBUG)
  # ifdef P113_DEBUG_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    addLogMove(LOG_LEVEL_DEBUG, strformat(F("VL53L1X  : idx: 0x%x init: %d"), i2cAddress, initState ? 1 : 0));
  }
  # endif // P113_DEBUG_DEBUG

  success    = true;
  readActive = false;

  if (distance >= 8190) {
    # ifdef P113_DEBUG_DEBUG
    addLog(LOG_LEVEL_DEBUG, "VL53L1X: NO MEASUREMENT");
    # endif // P113_DEBUG_DEBUG
    success = false;
  }

  # ifdef P113_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLogMove(LOG_LEVEL_INFO, strformat(F("VL53L1X: Address: 0x%02x / Timing: %d / Long Range: %d / Distance: %d"),
                                         i2cAddress, timing, range ? 1 : 0, distance));
  }
  # endif // P113_DEBUG

  return distance;
}

uint16_t P113_data_struct::readAmbient() {
  if (nullptr == sensor) {
    return 0u;
  }
  return sensor->getAmbientRate();
}

bool P113_data_struct::isReadSuccessful() {
  return success;
}

#endif // ifdef USES_P113
