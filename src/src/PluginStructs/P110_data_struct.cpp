#include "../PluginStructs/P110_data_struct.h"

#ifdef USES_P110

P110_data_struct::P110_data_struct(uint8_t i2c_addr, int timing, bool range) : i2cAddress(i2c_addr), timing(timing), range(range) {}

// **************************************************************************/
// Initialize VL53L0X
// **************************************************************************/
bool P110_data_struct::begin() {
  initState = true;

  sensor.setAddress(i2cAddress); // Initialize for configured address

  if (!sensor.init()) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("VL53L0X: Sensor not found, init failed for 0x");
      log += String(i2cAddress, HEX);
      addLogMove(LOG_LEVEL_INFO, log);
      addLog(LOG_LEVEL_INFO, sensor.getInitResult());
    }
    initState = false;
    return initState;
  }

  sensor.setTimeout(500);

  if (range) {
    // lower the return signal rate limit (default is 0.25 MCPS)
    sensor.setSignalRateLimit(0.1);

    // increase laser pulse periods (defaults are 14 and 10 PCLKs)
    sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange,   18);
    sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
  }

  sensor.setMeasurementTimingBudget(timing * 1000);

  initPhase  = P110_initPhases::InitDelay;
  timeToWait = timing + 50;

  return initState;
}

bool P110_data_struct::plugin_fifty_per_second() {
  if (initPhase == P110_initPhases::InitDelay) {
    timeToWait -= 20; // milliseconds

    // String log = F("VL53L0X: remaining wait: ");
    // log += timeToWait;
    // addLogMove(LOG_LEVEL_INFO, log);

    if (timeToWait <= 0) {
      timeToWait = 0;
      initPhase  = P110_initPhases::Ready;
    }
  }
  return true;
}

long P110_data_struct::readDistance() {
  long dist = -1; // Invalid

  if (initPhase != P110_initPhases::Ready) { return dist; }

  # if defined(P110_INFO_LOG) || defined(P110_DEBUG_LOG)
  String log;
  # endif // if defined(P110_INFO_LOG) || defined(P110_DEBUG_LOG)
  # ifdef P110_DEBUG_LOG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    log  = F("VL53L0X  : idx: 0x");
    log += String(i2cAddress, HEX);
    log += F(" init: ");
    log += String(initState, BIN);
    addLogMove(LOG_LEVEL_DEBUG, log);
  }
  # endif // P110_DEBUG_LOG

  if (initState) {
    success = true;
    dist    = sensor.readRangeSingleMillimeters();

    if (sensor.timeoutOccurred()) {
      # ifdef P110_DEBUG_LOG
      addLog(LOG_LEVEL_DEBUG, F("VL53L0X: TIMEOUT"));
      # endif // P110_DEBUG_LOG
      success = false;
    } else if (dist >= 8190) {
      # ifdef P110_DEBUG_LOG
      addLog(LOG_LEVEL_DEBUG, F("VL53L0X: NO MEASUREMENT"));
      # endif // P110_DEBUG_LOG
      success = false;
    }

    # ifdef P110_INFO_LOG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log  = F("VL53L0X: Address: 0x");
      log += String(i2cAddress, HEX);
      log += F(" / Timing: ");
      log += timing;
      log += F(" / Long Range: ");
      log += String(range, BIN);
      log += F(" / Distance: ");
      log += dist;
      addLogMove(LOG_LEVEL_INFO, log);
    }
    # endif // P110_INFO_LOG
  }
  return dist;
}

bool P110_data_struct::isReadSuccessful() {
  return success;
}

#endif // ifdef USES_P110
