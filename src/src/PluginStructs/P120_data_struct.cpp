#include "../PluginStructs/P120_data_struct.h"

#if defined(USES_P120) || defined(USES_P125)

// **************************************************************************/
// Constructor I2C
// **************************************************************************/
P120_data_struct::P120_data_struct(
  uint8_t i2c_addr,
  uint8_t aSize)
  : _i2c_addr(i2c_addr), _aSize(aSize)
{
  i2c_mode = true;
  initialization();
}

// **************************************************************************/
// Constructor SPI
// **************************************************************************/
P120_data_struct::P120_data_struct(
  int     cs_pin,
  uint8_t aSize)
  : _cs_pin(cs_pin), _aSize(aSize)
{
  i2c_mode = false;
  initialization();
}

// **************************************************************************/
// Common initialization
// **************************************************************************/
void P120_data_struct::initialization() {
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
      if (i2c_mode) {
        #  ifdef USES_P120
        log  = F("ADXL345: i2caddress: 0x");
        log += String(_i2cAddress, HEX);
        #  endif // ifdef USES_P120
      } else {
        #  ifdef USES_P125
        log  = F("ADXL345: CS-pin: ");
        log += _cs_pin;
        #  endif // ifdef USES_P125
      }
      log += F(", initialized: ");
      log += String(initialized() ? F("true") : F("false"));
      log += F(", ID=0x");
      log += String(adxl345->getDevID(), HEX);
      addLogMove(LOG_LEVEL_DEBUG, log);
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
      addLogMove(LOG_LEVEL_DEBUG, log);
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
        addLogMove(LOG_LEVEL_DEBUG, log);
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
  if (i2c_mode) {
    adxl345 = new (std::nothrow) ADXL345(_i2c_addr); // Init using I2C
  } else {
    adxl345 = new (std::nothrow) ADXL345(_cs_pin);   // Init using SPI
  }

  if (initialized()) {
    uint8_t act = 0, freeFall = 0, singleTap = 0, doubleTap = 0;
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String  log = F("ADXL345: Initializing sensor for ");

      if (i2c_mode) {
        log += F("I2C");
      } else {
        log += F("SPI");
      }
      log += F("...");
      addLogMove(LOG_LEVEL_INFO, log);
    }
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

  #ifndef BUILD_NO_DEBUG
  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log;

    if (log.reserve(25)) {
      if (i2c_mode) {
        # ifdef USES_P120
        log  = F("ADXL345: Address: 0x");
        log += String(_i2c_addr, HEX);
        # endif // ifdef USES_P120
      } else {
        # ifdef USES_P125
        log  = F("ADXL345: CS-pin: ");
        log += _cs_pin;
        # endif // ifdef USES_P125
      }
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
  }
  #endif

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

// *******************************************************************
// Load the configuration interface
// *******************************************************************
bool P120_data_struct::plugin_webform_load(struct EventStruct *event) {
  if (!i2c_mode) {
    addFormSubHeader(F("Device Settings"));
  }

  // Range
  {
    const __FlashStringHelper *rangeOptions[] = {
      F("2g"),
      F("4g"),
      F("8g"),
      F("16g (default)") };
    int rangeValues[] = { P120_RANGE_2G, P120_RANGE_4G, P120_RANGE_8G, P120_RANGE_16G };
    addFormSelector(F("Range"), F("p120_range"), 4, rangeOptions, rangeValues,
                    get2BitFromUL(P120_CONFIG_FLAGS1, P120_FLAGS1_RANGE));
  }

  // Axis selection
  {
    addFormCheckBox(F("X-axis activity sensing"), F("p120_activity_x"), bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_X) == 1);
    addFormCheckBox(F("Y-axis activity sensing"), F("p120_activity_y"), bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_Y) == 1);
    addFormCheckBox(F("Z-axis activity sensing"), F("p120_activity_z"), bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_Z) == 1);
    addFormNumericBox(F("Activity treshold"), F("p120_activity_treshold"),
                      get8BitFromUL(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_TRESHOLD), 1, 255);
    addUnit(F("1..255 * 62.5 mg"));
    addFormNumericBox(F("In-activity treshold"), F("p120_inactivity_treshold"),
                      get8BitFromUL(P120_CONFIG_FLAGS1, P120_FLAGS1_INACTIVITY_TRESHOLD), 1, 255);
    addUnit(F("1..255 * 62.5 mg"));
  }

  // Activity logging and send events for (in)activity
  {
    addFormCheckBox(F("Enable (in)activity events"), F("p120_send_activity"),
                    bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_SEND_ACTIVITY) == 1);
    addFormCheckBox(F("Log sensor activity (INFO)"), F("p120_log_activity"),
                    bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_LOG_ACTIVITY) == 1);
    addFormCheckBox(F("Events with raw measurements"), F("p120_raw_measurement"),
                    bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_EVENT_RAW_VALUES) == 1);
  }

  // Tap detection
  {
    addFormSubHeader(F("Tap detection"));

    addFormCheckBox(F("X-axis"), F("p120_tap_x"), bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_TAP_X) == 1);
    addFormCheckBox(F("Y-axis"), F("p120_tap_y"), bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_TAP_Y) == 1);
    addFormCheckBox(F("Z-axis"), F("p120_tap_z"), bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_TAP_Z) == 1);
    addFormNote(F("Also enables taskname#Tapped event."));
    addFormNumericBox(F("Tap treshold"), F("p120_tap_treshold"),
                      get8BitFromUL(P120_CONFIG_FLAGS2, P120_FLAGS2_TAP_TRESHOLD), 1, 255);
    addUnit(F("1..255 * 62.5 mg"));
    addFormNumericBox(F("Tap duration"), F("p120_tap_duration"),
                      get8BitFromUL(P120_CONFIG_FLAGS2, P120_FLAGS2_TAP_DURATION), 1, 255);
    addUnit(F("1..255 * 625 &micro;s"));
  }

  // Double-tap detection
  {
    addFormCheckBox(F("Enable double-tap detection"), F("p120_dbl_tap"), bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_DBL_TAP) == 1);
    addFormNote(F("Also enables taskname#DoubleTapped event."));
    addFormNumericBox(F("Double-tap latency"), F("p120_dbl_tap_latency"),
                      get8BitFromUL(P120_CONFIG_FLAGS2, P120_FLAGS2_DBL_TAP_LATENCY), 1, 255);
    addUnit(F("1..255 * 1.25 ms"));
    addFormNumericBox(F("Double-tap window"), F("p120_dbl_tap_window"),
                      get8BitFromUL(P120_CONFIG_FLAGS2, P120_FLAGS2_DBL_TAP_WINDOW), 1, 255);
    addUnit(F("1..255 * 1.25 ms"));
  }

  // Free-fall detection
  {
    addFormSubHeader(F("Free-fall detection"));

    addFormCheckBox(F("Enable free-fall detection"), F("p120_free_fall"), bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_FREE_FALL) == 1);
    addFormNote(F("Also enables taskname#FreeFall event."));
    addFormNumericBox(F("Free-fall treshold"), F("p120_free_fall_treshold"),
                      get8BitFromUL(P120_CONFIG_FLAGS3, P120_FLAGS3_FREEFALL_TRESHOLD), 1, 255);
    addUnit(F("1..255 * 62.5 mg"));
    addFormNumericBox(F("Free-fall duration"), F("p120_free_fall_duration"),
                      get8BitFromUL(P120_CONFIG_FLAGS3, P120_FLAGS3_FREEFALL_DURATION), 1, 255);
    addUnit(F("1..255 * 625 &micro;s"));
  }

  // Axis Offsets (calibration)
  {
    addFormSubHeader(F("Axis calibration"));
    addFormNumericBox(F("X-Axis offset"), F("p120_offset_x"),
                      get8BitFromUL(P120_CONFIG_FLAGS4, P120_FLAGS4_OFFSET_X) - 0x80, -127, 127);
    addUnit(F("-127..127 * 15.6 mg"));
    addFormNumericBox(F("Y-Axis offset"), F("p120_offset_y"),
                      get8BitFromUL(P120_CONFIG_FLAGS4, P120_FLAGS4_OFFSET_Y) - 0x80, -127, 127);
    addUnit(F("-127..127 * 15.6 mg"));
    addFormNumericBox(F("Z-Axis offset"), F("p120_offset_z"),
                      get8BitFromUL(P120_CONFIG_FLAGS4, P120_FLAGS4_OFFSET_Z) - 0x80, -127, 127);
    addUnit(F("-127..127 * 15.6 mg"));
  }

  // Data retrieval options
  {
    addFormSubHeader(F("Data retrieval"));

    addFormNumericBox(F("Averaging buffer size"), F("p120_average_buf"), P120_AVERAGE_BUFFER, 1, 100);
    addUnit(F("1..100"));

    const __FlashStringHelper *frequencyOptions[] = {
      F("10"),
      F("50") };
    int frequencyValues[] = { P120_FREQUENCY_10, P120_FREQUENCY_50 };
    addFormSelector(F("Measuring frequency"), F("p120_frequency"), 2, frequencyOptions, frequencyValues, P120_FREQUENCY);
    addUnit(F("Hz"));
    addFormNote(F("Values X/Y/Z are updated 1x per second, Controller updates &amp; Value-events are based on 'Interval' setting."));
  }

  return true;
}

// *******************************************************************
// Save the configuration interface
// *******************************************************************
bool P120_data_struct::plugin_webform_save(struct EventStruct *event) {
  P120_FREQUENCY = getFormItemInt(F("p120_frequency"));
  uint32_t flags = 0ul;

  set2BitToUL(flags, P120_FLAGS1_RANGE, getFormItemInt(F("p120_range")));
  bitWrite(flags, P120_FLAGS1_ACTIVITY_X,       isFormItemChecked(F("p120_activity_x")));
  bitWrite(flags, P120_FLAGS1_ACTIVITY_Y,       isFormItemChecked(F("p120_activity_y")));
  bitWrite(flags, P120_FLAGS1_ACTIVITY_Z,       isFormItemChecked(F("p120_activity_z")));
  bitWrite(flags, P120_FLAGS1_TAP_X,            isFormItemChecked(F("p120_tap_x")));
  bitWrite(flags, P120_FLAGS1_TAP_Y,            isFormItemChecked(F("p120_tap_y")));
  bitWrite(flags, P120_FLAGS1_TAP_Z,            isFormItemChecked(F("p120_tap_z")));
  bitWrite(flags, P120_FLAGS1_DBL_TAP,          isFormItemChecked(F("p120_dbl_tap")));
  bitWrite(flags, P120_FLAGS1_FREE_FALL,        isFormItemChecked(F("p120_free_fall")));
  bitWrite(flags, P120_FLAGS1_SEND_ACTIVITY,    isFormItemChecked(F("p120_send_activity")));
  bitWrite(flags, P120_FLAGS1_LOG_ACTIVITY,     isFormItemChecked(F("p120_log_activity")));
  bitWrite(flags, P120_FLAGS1_EVENT_RAW_VALUES, isFormItemChecked(F("p120_raw_measurement")));
  set8BitToUL(flags, P120_FLAGS1_ACTIVITY_TRESHOLD,   getFormItemInt(F("p120_activity_treshold")));
  set8BitToUL(flags, P120_FLAGS1_INACTIVITY_TRESHOLD, getFormItemInt(F("p120_inactivity_treshold")));
  P120_CONFIG_FLAGS1 = flags;

  flags = 0ul;
  set8BitToUL(flags, P120_FLAGS2_TAP_TRESHOLD,    getFormItemInt(F("p120_tap_treshold")));
  set8BitToUL(flags, P120_FLAGS2_TAP_DURATION,    getFormItemInt(F("p120_tap_duration")));
  set8BitToUL(flags, P120_FLAGS2_DBL_TAP_LATENCY, getFormItemInt(F("p120_dbl_tap_latency")));
  set8BitToUL(flags, P120_FLAGS2_DBL_TAP_WINDOW,  getFormItemInt(F("p120_dbl_tap_window")));
  P120_CONFIG_FLAGS2 = flags;

  flags = 0ul;
  set8BitToUL(flags, P120_FLAGS3_FREEFALL_TRESHOLD, getFormItemInt(F("p120_free_fall_treshold")));
  set8BitToUL(flags, P120_FLAGS3_FREEFALL_DURATION, getFormItemInt(F("p120_free_fall_duration")));
  P120_CONFIG_FLAGS3 = flags;

  flags = 0ul;
  set8BitToUL(flags, P120_FLAGS4_OFFSET_X, getFormItemInt(F("p120_offset_x")) + 0x80);
  set8BitToUL(flags, P120_FLAGS4_OFFSET_Y, getFormItemInt(F("p120_offset_y")) + 0x80);
  set8BitToUL(flags, P120_FLAGS4_OFFSET_Z, getFormItemInt(F("p120_offset_z")) + 0x80);
  P120_CONFIG_FLAGS4 = flags;

  return true;
}

// *******************************************************************
// Set defaultss for the configuration interface
// *******************************************************************
bool P120_data_struct::plugin_set_defaults(struct EventStruct *event) {
  bool success = false;

  uint32_t flags = 0ul;

  set2BitToUL(flags, P120_FLAGS1_RANGE, P120_RANGE_16G); // Default to 16g range for highest resolution
  bitSet(flags, P120_FLAGS1_ACTIVITY_X);                 // Detect activity on all axes
  bitSet(flags, P120_FLAGS1_ACTIVITY_Y);
  bitSet(flags, P120_FLAGS1_ACTIVITY_Z);
  set8BitToUL(flags, P120_FLAGS1_ACTIVITY_TRESHOLD,   P120_DEFAULT_ACTIVITY_TRESHOLD);
  set8BitToUL(flags, P120_FLAGS1_INACTIVITY_TRESHOLD, P120_DEFAULT_INACTIVITY_TRESHOLD);
  P120_CONFIG_FLAGS1 = flags;

  flags = 0ul;
  set8BitToUL(flags, P120_FLAGS2_TAP_TRESHOLD,    P120_DEFAULT_TAP_TRESHOLD);
  set8BitToUL(flags, P120_FLAGS2_TAP_DURATION,    P120_DEFAULT_TAP_DURATION);
  set8BitToUL(flags, P120_FLAGS2_DBL_TAP_LATENCY, P120_DEFAULT_DBL_TAP_LATENCY);
  set8BitToUL(flags, P120_FLAGS2_DBL_TAP_WINDOW,  P120_DEFAULT_DBL_TAP_WINDOW);
  P120_CONFIG_FLAGS2 = flags;

  flags = 0ul;
  set8BitToUL(flags, P120_FLAGS3_FREEFALL_TRESHOLD, P120_DEFAULT_FREEFALL_TRESHOLD);
  set8BitToUL(flags, P120_FLAGS3_FREEFALL_DURATION, P120_DEFAULT_FREEFALL_DURATION);
  P120_CONFIG_FLAGS3 = flags;

  flags = 0ul;
  set8BitToUL(flags, P120_FLAGS4_OFFSET_X, 0 + 0x80); // Offset 0 by default
  set8BitToUL(flags, P120_FLAGS4_OFFSET_Y, 0 + 0x80);
  set8BitToUL(flags, P120_FLAGS4_OFFSET_Z, 0 + 0x80);
  P120_CONFIG_FLAGS4 = flags;

  // No decimals plausible, as the outputs from the sensor are of type int
  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    ExtraTaskSettings.TaskDeviceValueDecimals[i] = 0;
  }

  success = true;
  return success;
}

#endif // if defined(USES_P120) || defined(USES_P125)
