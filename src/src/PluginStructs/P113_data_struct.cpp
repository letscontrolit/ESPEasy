#include "../PluginStructs/P113_data_struct.h"

#ifdef USES_P113

P113_data_struct::P113_data_struct(uint8_t i2c_addr, int timing, bool range) : i2cAddress(i2c_addr), timing(timing), range(range) {}

// **************************************************************************/
// Initialize VL53L1X
// **************************************************************************/
bool P113_data_struct::begin() {
  initState = true;

  sensor.setI2CAddress(i2cAddress); // Initialize for configured address

  if (sensor.begin() != 0) {
    String log = F("VL53L1X: Sensor not found, init failed for 0x");
    log += String(i2cAddress, HEX);
    addLog(LOG_LEVEL_INFO, log);
    initState = false;
    return initState;
  }

  sensor.setTimingBudgetInMs(timing);

  if (range) {
    sensor.setDistanceModeLong();
  } else {
    sensor.setDistanceModeShort();
  }

  return initState;
}

bool P113_data_struct::startRead() {
  if (initState && !readActive) {
    sensor.startRanging();
    readActive = true;
    distance   = -1;
  }
  return readActive;
}

bool P113_data_struct::readAvailable() {
  bool ready = sensor.checkForDataReady();

  if (ready) {
    distance = sensor.getDistance();
    sensor.clearInterrupt();
    sensor.stopRanging();

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
    log  = F("VL53L1X  : idx: 0x");
    log += String(i2cAddress, HEX);
    log += F(" init: ");
    log += String(initState, BIN);
    addLog(LOG_LEVEL_DEBUG, log);
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
  log  = F("VL53L1X: Address: 0x");
  log += String(i2cAddress, HEX);
  log += F(" / Timing: ");
  log += String(timing, DEC);
  log += F(" / Long Range: ");
  log += String(range, BIN);
  log += F(" / Distance: ");
  log += distance;
  addLog(LOG_LEVEL_INFO, log);
  # endif // P113_DEBUG

  return distance;
}

uint16_t P113_data_struct::readAmbient() {
  return sensor.getAmbientRate();
}

bool P113_data_struct::isReadSuccessful() {
  return success;
}

#endif // ifdef USES_P113
