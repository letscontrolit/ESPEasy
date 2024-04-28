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
      addLogMove(LOG_LEVEL_INFO, strformat(F("VL53L0X: Sensor not found, init failed for 0x%02x"), i2cAddress));
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

  # ifdef P110_DEBUG_LOG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    addLogMove(LOG_LEVEL_DEBUG, strformat(F("VL53L0X  : idx: 0x%02x init: %d"), i2cAddress, initState ? 1 : 0));
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
      addLogMove(LOG_LEVEL_INFO,
                 strformat(F("VL53L0X: Address: 0x%02x / Timing: %d / Long Range: %d / Distance: %d"),
                           i2cAddress, timing, range ? 1 : 0, dist));
    }
    # endif // P110_INFO_LOG
  }
  return dist;
}

bool P110_data_struct::isReadSuccessful() {
  return success;
}

#endif // ifdef USES_P110
