#include "../PluginStructs/P120_data_struct.h"

#ifdef USES_P120

// **************************************************************************/
// Constructor
// **************************************************************************/
P120_data_struct::P120_data_struct(
  uint8_t i2c_addr,
  uint8_t aSize
  )
  : _i2c_addr(i2c_addr), _aSize(aSize)
{
  if (_aSize == 0) { _aSize = 1; }
  _XA.resize(_aSize, 0);
  _YA.resize(_aSize, 0);
  _ZA.resize(_aSize, 0);
  _aUsed = 0;
  _aMax  = 0;
}

// **************************************************************************/
// Destructor
// **************************************************************************/
P120_data_struct::~P120_data_struct() {
  if (initialized()) {
    delete adxl345;
    adxl345 = nullptr;
  }
}

// **************************************************************************/
// Initialize sensor and read data from ADXL345
// **************************************************************************/
bool P120_data_struct::read_sensor(struct EventStruct *event) {
  # if PLUGIN_120_DEBUG
  String log;
  # endif // if PLUGIN_120_DEBUG

  if (!initialized()) {
    init_sensor(event);
    # if PLUGIN_120_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG) &&
        log.reserve(55)) {
      log  = F("ADXL345: i2caddress: 0x");
      log += String(_i2cAddress, HEX);
      log += F(", initialized: ");
      log += String(initialized() ? F("true") : F("false"));
      log += F(", ID=0x");
      log += String(adxl345->getDevID(), HEX);
      addLog(LOG_LEVEL_DEBUG, log);
    }
    # endif // if PLUGIN_120_DEBUG
  }

  if (initialized()) {
    _x = 0; _y = 0; _z = 0;
    adxl345->readAccel(&_x, &_y, &_z);
    _XA[_aUsed] = _x;
    _YA[_aUsed] = _y;
    _ZA[_aUsed] = _z;

    _aUsed++;

    if ((_aMax < _aUsed) && (_aUsed < _aSize)) {
      _aMax = _aUsed;
    }

    if (_aUsed == _aSize) {
      _aUsed = 0;
    }

    # if PLUGIN_120_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG) &&
        log.reserve(40)) {
      log  = F("ADXL345: X: ");
      log += _x;
      log += F(", Y: ");
      log += _y;
      log += F(", Z: ");
      log += _z;
      addLog(LOG_LEVEL_DEBUG, log);
    }
    # endif // if PLUGIN_120_DEBUG

    sensor_check_interrupt(event); // Process any interrupt

    return true;
  }
  return false;
}

// **************************************************************************/
// Average the measurements and return the results
// **************************************************************************/
bool P120_data_struct::read_data(struct EventStruct *event, int& X, int& Y, int& Z) {
  X = 0;
  Y = 0;
  Z = 0;

  if (initialized()) {
    for (uint8_t n = 0; n <= _aMax; n++) {
      X += _XA[n];
      Y += _YA[n];
      Z += _ZA[n];
    }

    X /= _aMax; // Average available measurements
    Y /= _aMax;
    Z /= _aMax;

    # if PLUGIN_120_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log;

      if (log.reserve(40)) {
        log  = F("ADXL345: averages, X: ");
        log += X;
        log += F(", Y: ");
        log += Y;
        log += F(", Z: ");
        log += Z;
        addLog(LOG_LEVEL_DEBUG, log);
      }
    }
    # endif // if PLUGIN_120_DEBUG
  }
  return initialized();
}

// **************************************************************************/
// Initialize ADXL345
// **************************************************************************/
bool P120_data_struct::init_sensor(struct EventStruct *event) {
  adxl345 = new (std::nothrow) ADXL345(_i2c_addr);

  if (initialized()) {
    uint8_t act = 0, freeFall = 0, singleTap = 0, doubleTap = 0;
    addLog(LOG_LEVEL_INFO, F("ADXL345: Initializing sensor..."));
    adxl345->powerOn();
    adxl345->setRangeSetting(2 ^ (get2BitFromUL(P120_CONFIG_FLAGS1, P120_FLAGS1_RANGE) + 1)); // Range is stored in 2 bits, only 4 possible
                                                                                              // options
    // Activity triggering
    // Inactivity triggering, same axis
    adxl345->setActivityXYZ(bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_X),
                            bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_Y),
                            bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_Z));
    adxl345->setInactivityXYZ(bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_X),
                              bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_Y),
                              bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_Z));

    if (bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_X) ||
                              bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_Y) ||
                              bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_Z)) {
      adxl345->setActivityThreshold(get8BitFromUL(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_TRESHOLD));
      adxl345->setInactivityThreshold(get8BitFromUL(P120_CONFIG_FLAGS1, P120_FLAGS1_INACTIVITY_TRESHOLD));
      act = 1;
    }

    // Axis Offsets
    adxl345->setAxisOffset(get8BitFromUL(P120_CONFIG_FLAGS4, P120_FLAGS4_OFFSET_X) - 0x80,
                           get8BitFromUL(P120_CONFIG_FLAGS4, P120_FLAGS4_OFFSET_Y) - 0x80,
                           get8BitFromUL(P120_CONFIG_FLAGS4, P120_FLAGS4_OFFSET_Z) - 0x80);

    // Tap triggering
    adxl345->setTapDetectionOnXYZ(bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_TAP_X),
                                  bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_TAP_Y),
                                  bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_TAP_Z));

    // Tap detection
    if (bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_TAP_X) ||
                                  bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_TAP_Y) ||
                                  bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_TAP_Z)) {
      adxl345->setTapThreshold(get8BitFromUL(P120_CONFIG_FLAGS2, P120_FLAGS2_TAP_TRESHOLD));
      adxl345->setTapDuration(get8BitFromUL(P120_CONFIG_FLAGS2, P120_FLAGS2_TAP_DURATION));
      singleTap = 1;

      // Double-tap detection
      if (bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_DBL_TAP)) {
        adxl345->setDoubleTapLatency(get8BitFromUL(P120_CONFIG_FLAGS2, P120_FLAGS2_DBL_TAP_LATENCY));
        adxl345->setDoubleTapWindow(get8BitFromUL(P120_CONFIG_FLAGS2, P120_FLAGS2_DBL_TAP_WINDOW));
        doubleTap = 1;
      } else {
        adxl345->setDoubleTapLatency(0); // Off
        adxl345->setDoubleTapWindow(0);
      }
    } else {
      adxl345->setTapThreshold(0); // Off
      adxl345->setTapDuration(0);
      adxl345->setDoubleTapLatency(0);
      adxl345->setDoubleTapWindow(0);
    }

    // Free-fall detection
    if (bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_FREE_FALL)) {
      adxl345->setFreeFallThreshold(get8BitFromUL(P120_CONFIG_FLAGS3, P120_FLAGS3_FREEFALL_TRESHOLD));
      adxl345->setFreeFallDuration(get8BitFromUL(P120_CONFIG_FLAGS3,  P120_FLAGS3_FREEFALL_DURATION));
      freeFall = 1;
    } else {
      adxl345->setFreeFallThreshold(0);
      adxl345->setFreeFallDuration(0);
    }

    // Enable interrupts
    adxl345->setImportantInterruptMapping(singleTap, doubleTap, freeFall, act, act);
    adxl345->ActivityINT(act);
    adxl345->InactivityINT(act);
    adxl345->singleTapINT(singleTap);
    adxl345->doubleTapINT(doubleTap);
    adxl345->FreeFallINT(freeFall);

    addLog(LOG_LEVEL_INFO, F("ADXL345: Initialization done."));
  } else {
    addLog(LOG_LEVEL_ERROR, F("ADXL345: Initialization of sensor failed."));
    return false;
  }

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log;

    if (log.reserve(25)) {
      log  = F("ADXL345: Address: 0x");
      log += String(_i2c_addr, HEX);
      addLog(LOG_LEVEL_DEBUG, log);
    }
  }

  return true;
}

/* Look for Interrupts and Triggered Action    */
void P120_data_struct::sensor_check_interrupt(struct EventStruct *event) {
  // getInterruptSource clears all triggered actions after returning value
  // Do not call again until you need to recheck for triggered actions
  uint8_t interrupts = adxl345->getInterruptSource();
  String  payload;

  payload.reserve(30);

  // Free Fall Detection
  if (adxl345->triggered(interrupts, ADXL345_FREE_FALL)) {
    if (bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_LOG_ACTIVITY)) {
      addLog(LOG_LEVEL_INFO, F("ADXL345: *** FREE FALL ***"));
    }
    payload = F("FreeFall=");
    appendPayloadXYZ(event, payload, 1u, 1u, 1u); // Use all values
    send_task_event(event, payload);
  }

  // Inactivity
  if (adxl345->triggered(interrupts, ADXL345_INACTIVITY) &&
      bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_SEND_ACTIVITY)) {
    if (!inactivityTriggered) {
      if (bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_LOG_ACTIVITY)) {
        addLog(LOG_LEVEL_INFO, F("ADXL345: *** INACTIVITY ***"));
      }
      payload = F("Inactivity=");
      appendPayloadXYZ(event, payload,
                       bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_X),
                       bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_Y),
                       bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_Z));
      send_task_event(event, payload);
    }
    inactivityTriggered = true;
    activityTriggered   = false;
  }

  // Activity
  if (adxl345->triggered(interrupts, ADXL345_ACTIVITY) &&
      bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_SEND_ACTIVITY)) {
    if (!activityTriggered) {
      if (bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_LOG_ACTIVITY)) {
        addLog(LOG_LEVEL_INFO, F("ADXL345: *** ACTIVITY ***"));
      }
      payload = F("Activity=");
      appendPayloadXYZ(event, payload,
                       bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_X),
                       bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_Y),
                       bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_Z));
      send_task_event(event, payload);
    }
    activityTriggered   = true;
    inactivityTriggered = false;
  }

  // Double Tap Detection
  // Tap Detection
  if (adxl345->triggered(interrupts, ADXL345_DOUBLE_TAP) ||
      (adxl345->triggered(interrupts, ADXL345_SINGLE_TAP))) {
    if (adxl345->triggered(interrupts, ADXL345_SINGLE_TAP)) {
      if (bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_LOG_ACTIVITY)) {
        addLog(LOG_LEVEL_INFO, F("ADXL345: *** TAP ***"));
      }
      payload = F("Tapped");
    }

    if (adxl345->triggered(interrupts, ADXL345_DOUBLE_TAP)) {      // tonhuisman: Double-tap overrides single-tap event
      if (bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_LOG_ACTIVITY)) { // This is on purpose and as intended!
        addLog(LOG_LEVEL_INFO, F("ADXL345: *** DOUBLE TAP ***"));
      }
      payload = F("DoubleTapped");
    }
    payload += '=';

    appendPayloadXYZ(event, payload,
                     bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_TAP_X),
                     bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_TAP_Y),
                     bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_TAP_Z));
    send_task_event(event, payload);
  }
}

// *******************************************************************
// Append X, Y and Z arguments where configured
// *******************************************************************
void P120_data_struct::appendPayloadXYZ(struct EventStruct *event, String& payload,
                                        uint8_t useX,
                                        uint8_t useY,
                                        uint8_t useZ) {
  if (useX == 1) {
    if (bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_EVENT_RAW_VALUES)) {
      payload += _x;
    } else {
      payload += formatUserVarNoCheck(event, 0);
    }
  } else {
    payload += 0;
  }
  payload += ',';

  if (useY == 1) {
    if (bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_EVENT_RAW_VALUES)) {
      payload += _y;
    } else {
      payload += formatUserVarNoCheck(event, 1);
    }
  } else {
    payload += 0;
  }
  payload += ',';

  if (useZ == 1) {
    if (bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_EVENT_RAW_VALUES)) {
      payload += _z;
    } else {
      payload += formatUserVarNoCheck(event, 2);
    }
  } else {
    payload += 0;
  }
}

// *******************************************************************
// Send out an event for the current event, aith provided payload
// *******************************************************************
void P120_data_struct::send_task_event(struct EventStruct *event,
                                       String            & eventPayload) {
  if (Settings.UseRules &&
      !eventPayload.isEmpty()) {
    String RuleEvent;
    RuleEvent += getTaskDeviceName(event->TaskIndex);
    RuleEvent += '#';
    RuleEvent += eventPayload;
    eventQueue.addMove(std::move(RuleEvent));
  }
}

#endif // ifdef USES_P120
