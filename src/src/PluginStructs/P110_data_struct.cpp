#include "../PluginStructs/P110_data_struct.h"

#ifdef USES_P110

P110_data_struct::P110_data_struct(uint8_t i2c_addr, int timing, bool range) :
  _i2cAddress(i2c_addr),
  _timing(timing),
  _range(range) {}

// **************************************************************************/
// Initialize VL53L0X
// **************************************************************************/
bool P110_data_struct::begin(uint32_t interval_ms) {
  _timeToWait = 0;
  _initPhase  = P110_initPhases::Undefined;
  sensor.setAddress(_i2cAddress); // Initialize for configured address

  if (!sensor.init()) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLogMove(LOG_LEVEL_INFO, strformat(F("VL53L0X: Sensor not found, init failed for 0x%02x"), _i2cAddress));
      addLog(LOG_LEVEL_INFO, sensor.getInitResult());
    }
    return false;
  }

  sensor.setTimeout(500);

  if (_range) {
    // lower the return signal rate limit (default is 0.25 MCPS)
    sensor.setSignalRateLimit(0.1);

    // increase laser pulse periods (defaults are 14 and 10 PCLKs)
    sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange,   18);
    sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
  }

  sensor.setMeasurementTimingBudget(_timing * 1000);

  _initPhase  = P110_initPhases::InitDelay;
  _timeToWait = millis() + _timing + 50;

  sensor.startContinuous(interval_ms);

  return true;
}

bool P110_data_struct::plugin_fifty_per_second() {
  if (_initPhase == P110_initPhases::InitDelay) {
    if ((_timeToWait != 0) && timeOutReached(_timeToWait)) {
      _timeToWait = 0;
      _initPhase  = P110_initPhases::Ready;
    }
  }
  return true;
}

int16_t P110_data_struct::getDistance() {
  const int res = _distance;
  _distance = P110_DISTANCE_WAITING;
  return res;
}

int16_t P110_data_struct::readDistance() {
  if (_initPhase != P110_initPhases::Ready) {
    return P110_DISTANCE_UNINITIALIZED;
  }

  const uint16_t dist = sensor.readRangeContinuousMillimeters();

  if (dist == 65534) {
    // Just waiting
    // No need to keep sending many logs per second
    return P110_DISTANCE_WAITING;
  }


# ifdef P110_DEBUG_LOG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    addLogMove(LOG_LEVEL_DEBUG, strformat(F("VL53L0X: idx: 0x%02x init: %u"),
                                          _i2cAddress, static_cast<uint8_t>(_initPhase)));
  }
# endif // P110_DEBUG_LOG

  if (sensor.timeoutOccurred()) {
# ifdef P110_DEBUG_LOG
    addLog(LOG_LEVEL_DEBUG, F("VL53L0X: TIMEOUT"));
# endif // P110_DEBUG_LOG
    return P110_DISTANCE_READ_TIMEOUT;
  } else if (dist == 0xFFFF) {
# ifdef P110_DEBUG_LOG
    addLog(LOG_LEVEL_DEBUG, F("VL53L0X: NO MEASUREMENT: 0xFFFF"));
# endif // P110_DEBUG_LOG
    return P110_DISTANCE_READ_ERROR;
  } else if (dist >= 8190u) {
# ifdef P110_DEBUG_LOG
    addLog(LOG_LEVEL_DEBUG, concat(F("VL53L0X: NO MEASUREMENT: "), dist));
# endif // P110_DEBUG_LOG
    return P110_DISTANCE_OUT_OF_RANGE;
  } 

  // Only keep a copy of actual distance readings.
  // Since the distance reading is later called from PLUGIN_READ, 
  // we might have had a new reading inbetween which could be a "still waiting"
  // value and then we lost the actual reading.
  _distance = dist;
  return _distance;
}

bool P110_data_struct::isReadSuccessful() const {
  return _distance >= 0;
}

#endif // ifdef USES_P110
