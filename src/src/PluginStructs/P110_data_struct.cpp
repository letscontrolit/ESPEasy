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

bool P110_data_struct::check_reading_ready(struct EventStruct *event) {
  if (_initPhase == P110_initPhases::InitDelay) {
    if ((_timeToWait != 0) && timeOutReached(_timeToWait)) {
      _timeToWait = 0;
      _initPhase  = P110_initPhases::Ready;
    }
  } else {
    if (readDistance() >= 0) {
      Scheduler.schedule_task_device_timer(event->TaskIndex, millis());
    }
  }
  return true;
}

bool P110_data_struct::plugin_read(struct EventStruct *event) {
  bool success = false;

  if (isReadSuccessful()) {
    const float new_distance  = getDistance();
    const float prev_distance = _prev_distance;

    const bool first_sample = (prev_distance < 0.0f);

    const float estimator = (_filtered < 0.0f)
            ? new_distance
            : _filtered;

    const float ratio_filtered = 16;
    const float ratio_newVal = _prev_newval_ratio;
    _prev_newval_ratio = std::abs(estimator - new_distance);

    _filtered = 
      ((ratio_filtered * estimator) + (ratio_newVal * new_distance))
      / (ratio_filtered + ratio_newVal);

    const float dist   = _filtered;
    const float p_dist = prev_distance;

    // Check trend:
    //  0 = equal
    // -1 = move closer
    //  1 = move away

    const int16_t displacement = first_sample ? 0 : roundf(dist - p_dist);
    const int16_t disp_dir     = (displacement == 0)
            ? 0
            : (displacement > 0) ? 1 : -1;

    //const bool direction_changed = disp_dir != static_cast<int16_t>(UserVar.getFloat(event->TaskIndex, 1));
    const bool triggered         =
      //      direction_changed ||
      (std::abs(displacement) > P110_DELTA);

        # ifdef P110_INFO_LOG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO, strformat(F("VL53L0x: Perform read: trig: %d, prev: %d, dist: %d"), triggered, p_dist, dist));
    }
        # endif // ifdef P110_INFO_LOG
    // Value is classified as invalid when > 8190, so no conversion or 'split' needed
    UserVar.setFloat(event->TaskIndex, 0, _filtered);
    UserVar.setFloat(event->TaskIndex, 1, disp_dir);       // Trend of value

    if (first_sample || triggered || (P110_SEND_ALWAYS == 1)) {
      // Update the "previous" distance.
      _prev_distance = _filtered;
      success        = true;
    }
  }
  return success;
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

  int16_t dist{};

  if (sensor.asyncReadRangeContinuousMillimeters(dist)) {
    if ((dist >= 0) && (dist < 8192)) {
      // Only keep a copy of valid distance readings.
      // Since the distance reading is later called from PLUGIN_READ,
      // we might have had a new reading inbetween which could be a "still waiting"
      // value and then we lost the actual reading.

      _distance = dist;
      return _distance;
    }
  }

  if (dist == VL53L0X_WAITING) {
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
  } else if (dist >= 8190) {
# ifdef P110_DEBUG_LOG
    addLog(LOG_LEVEL_DEBUG, concat(F("VL53L0X: NO MEASUREMENT: "), dist));
# endif // P110_DEBUG_LOG
    return P110_DISTANCE_OUT_OF_RANGE;
  }

# ifdef P110_DEBUG_LOG
  addLog(LOG_LEVEL_DEBUG, F("VL53L0X: NO MEASUREMENT: 0xFFFF"));
# endif // P110_DEBUG_LOG
  return P110_DISTANCE_READ_ERROR;
}

bool P110_data_struct::isReadSuccessful() const {
  return _distance >= 0;
}

#endif // ifdef USES_P110
