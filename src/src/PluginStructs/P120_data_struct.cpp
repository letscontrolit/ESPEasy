#include "../PluginStructs/P120_data_struct.h"

#if defined(USES_P120) || defined(USES_P125)

# define P120_RAD_TO_DEG        57.295779f // 180.0/M_PI


P120_data_struct::P120_data_struct(uint8_t aSize)
  : _aSize(aSize)
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
  if (adxl345 != nullptr) {
    delete adxl345;
    adxl345 = nullptr;
  }
}

void P120_data_struct::setI2Caddress(uint8_t i2c_addr)
{
  _i2c_addr = i2c_addr;
  i2c_mode  = true;
}

void P120_data_struct::setSPI_CSpin(int cs_pin)
{
  _cs_pin  = cs_pin;
  i2c_mode = false;
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
bool P120_data_struct::read_data(struct EventStruct *event) const
{
  float pitch, roll;

  getPitchRoll(event, pitch, roll);
  float X, Y, Z;

  if (!get_XYZ(X, Y, Z)) { return false; }

  // At 2.5V the sensor has 256 LSB/g
  // However with higher voltage the sensitivity may change
  // Assume on average we're at 1g environment, regardless the sensor orientation.
  // Thus we need to compute the length of the vector and use that as scale factor for 1g.
  float scaleFactor_g = sqrtf(X * X + Y * Y + Z * Z);

  if (last_scale_factor_g > 0.1f) {
    // IIR filter to adapt factor slowly for variations in the supply voltage.
    // This will also prevent "shocks" to affect the scale factor a lot.
    scaleFactor_g += last_scale_factor_g * 15;
    scaleFactor_g /= 16;
  }
  last_scale_factor_g = scaleFactor_g;

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    if (i < P120_NR_OUTPUT_VALUES) {
      const uint8_t pconfigIndex = i + P120_QUERY1_CONFIG_POS;
      float value                = 0.0f;

      switch (static_cast<valueType>(PCONFIG(pconfigIndex))) {
        case valueType::Empty:
          break;
        case valueType::X_RAW:
          value = X;
          break;
        case valueType::Y_RAW:
          value = Y;
          break;
        case valueType::Z_RAW:
          value = Z;
          break;
        case valueType::X_g:
          value = X / scaleFactor_g;
          break;
        case valueType::Y_g:
          value = Y / scaleFactor_g;
          break;
        case valueType::Z_g:
          value = Z / scaleFactor_g;
          break;
        case valueType::Pitch:
          value = pitch;
          break;
        case valueType::Roll:
          value = roll;
          break;
        case valueType::NR_ValueTypes:
          break;
      }
      UserVar[event->BaseVarIndex + i] = value;
    }
  }
  return true;
}

bool P120_data_struct::get_XYZ(float& X, float& Y, float& Z) const
{
  X = 0.0f;
  Y = 0.0f;
  Z = 0.0f;

  if (initialized()) {
    for (uint8_t n = 0; n <= _aMax; n++) {
      X += _XA[n];
      Y += _YA[n];
      Z += _ZA[n];
    }

    if (_aMax > 0) {
      X /= _aMax; // Average available measurements
      Y /= _aMax;
      Z /= _aMax;
    }

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
    return true;
  }
  return false;
}

bool P120_data_struct::getPitchRoll(struct EventStruct *event, float& pitch, float& roll) const
{
  float X, Y, Z;

  if (get_XYZ(X, Y, Z)) {
    roll  = atan2(-Y, Z);
    pitch = atan2(X, sqrt(Y * Y + Z * Z));

    if (!bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_ANGLE_IN_RAD)) {
      roll  *= P120_RAD_TO_DEG;
      pitch *= P120_RAD_TO_DEG;
    }
    return true;
  }
  roll  = 0.0f;
  pitch = 0.0f;
  return false;
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
      String log = F("ADXL345: Initializing sensor for ");

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

    // FIXME TD-er: This offset only has 1/4th the resolution of the raw data
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

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log;

    if (log.reserve(25)) {
      if (i2c_mode) {
        #  ifdef USES_P120
        log  = F("ADXL345: Address: 0x");
        log += String(_i2c_addr, HEX);
        #  endif // ifdef USES_P120
      } else {
        #  ifdef USES_P125
        log  = F("ADXL345: CS-pin: ");
        log += _cs_pin;
        #  endif // ifdef USES_P125
      }
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
  }
  # endif // ifndef BUILD_NO_DEBUG

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
bool P120_data_struct::plugin_webform_loadOutputSelector(struct EventStruct *event) {
  {
    const __FlashStringHelper *options[P120_NR_OUTPUT_OPTIONS];

    for (uint8_t i = 0; i < P120_NR_OUTPUT_OPTIONS; ++i) {
      options[i] = P120_data_struct::valuename(i, true);
    }

    for (uint8_t i = 0; i < P120_NR_OUTPUT_VALUES; ++i) {
      const uint8_t pconfigIndex = i + P120_QUERY1_CONFIG_POS;
      sensorTypeHelper_loadOutputSelector(event, pconfigIndex, i, P120_NR_OUTPUT_OPTIONS, options);
    }
  }
  addFormSeparator(4);
  {
    const __FlashStringHelper *options[] = {
      F("Degrees"),
      F("Radians") };
    const int values[] = { 0,
                           1 };
    const int choice = bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_ANGLE_IN_RAD);
    addFormSelector(F("Angle Units"), F("angle_rad"), 2, options, values, choice);
  }
  return true;
}

bool P120_data_struct::plugin_webform_load(struct EventStruct *event) {
  // Range
  {
    const __FlashStringHelper *rangeOptions[] = {
      F("2g"),
      F("4g"),
      F("8g"),
      F("16g (default)") };
    int rangeValues[] = { P120_RANGE_2G, P120_RANGE_4G, P120_RANGE_8G, P120_RANGE_16G };
    addFormSelector(F("Range"), F("range"), 4, rangeOptions, rangeValues,
                    get2BitFromUL(P120_CONFIG_FLAGS1, P120_FLAGS1_RANGE));
  }

  // Axis selection
  {
    addFormCheckBox(F("X-axis activity sensing"), F("act_x"), bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_X) == 1);
    addFormCheckBox(F("Y-axis activity sensing"), F("act_y"), bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_Y) == 1);
    addFormCheckBox(F("Z-axis activity sensing"), F("act_z"), bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_Z) == 1);
    addFormNumericBox(F("Activity treshold"), F("act_thres"),
                      get8BitFromUL(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_TRESHOLD), 1, 255);
    addUnit(F("1..255 * 62.5 mg"));
    addFormNumericBox(F("In-activity treshold"), F("inact_thres"),
                      get8BitFromUL(P120_CONFIG_FLAGS1, P120_FLAGS1_INACTIVITY_TRESHOLD), 1, 255);
    addUnit(F("1..255 * 62.5 mg"));
  }

  // Activity logging and send events for (in)activity
  {
    addFormCheckBox(F("Enable (in)activity events"), F("send_act"),
                    bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_SEND_ACTIVITY) == 1);
    addFormCheckBox(F("Log sensor activity (INFO)"), F("log_act"),
                    bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_LOG_ACTIVITY) == 1);
    addFormCheckBox(F("Events with raw measurements"), F("raw_measure"),
                    bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_EVENT_RAW_VALUES) == 1);
  }

  // Tap detection
  {
    addFormSubHeader(F("Tap detection"));

    addFormCheckBox(F("X-axis"), F("tap_x"), bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_TAP_X) == 1);
    addFormCheckBox(F("Y-axis"), F("tap_y"), bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_TAP_Y) == 1);
    addFormCheckBox(F("Z-axis"), F("tap_z"), bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_TAP_Z) == 1);
    addFormNote(F("Also enables taskname#Tapped event."));
    addFormNumericBox(F("Tap treshold"), F("tap_thres"),
                      get8BitFromUL(P120_CONFIG_FLAGS2, P120_FLAGS2_TAP_TRESHOLD), 1, 255);
    addUnit(F("1..255 * 62.5 mg"));
    addFormNumericBox(F("Tap duration"), F("tap_dur"),
                      get8BitFromUL(P120_CONFIG_FLAGS2, P120_FLAGS2_TAP_DURATION), 1, 255);
    addUnit(F("1..255 * 625 &micro;s"));
  }

  // Double-tap detection
  {
    addFormCheckBox(F("Enable double-tap detection"), F("dbl_tap"), bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_DBL_TAP) == 1);
    addFormNote(F("Also enables taskname#DoubleTapped event."));
    addFormNumericBox(F("Double-tap latency"), F("dbl_tap_latency"),
                      get8BitFromUL(P120_CONFIG_FLAGS2, P120_FLAGS2_DBL_TAP_LATENCY), 1, 255);
    addUnit(F("1..255 * 1.25 ms"));
    addFormNumericBox(F("Double-tap window"), F("dbl_tap_window"),
                      get8BitFromUL(P120_CONFIG_FLAGS2, P120_FLAGS2_DBL_TAP_WINDOW), 1, 255);
    addUnit(F("1..255 * 1.25 ms"));
  }

  // Free-fall detection
  {
    addFormSubHeader(F("Free-fall detection"));

    addFormCheckBox(F("Enable free-fall detection"), F("fr_fall"), bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_FREE_FALL) == 1);
    addFormNote(F("Also enables taskname#FreeFall event."));
    addFormNumericBox(F("Free-fall treshold"), F("fr_fall_thres"),
                      get8BitFromUL(P120_CONFIG_FLAGS3, P120_FLAGS3_FREEFALL_TRESHOLD), 1, 255);
    addUnit(F("1..255 * 62.5 mg"));
    addFormNumericBox(F("Free-fall duration"), F("fr_fall_dur"),
                      get8BitFromUL(P120_CONFIG_FLAGS3, P120_FLAGS3_FREEFALL_DURATION), 1, 255);
    addUnit(F("1..255 * 625 &micro;s"));
  }

  // Axis Offsets (calibration)
  {
    addFormSubHeader(F("Axis calibration"));
    addFormNumericBox(F("X-Axis offset"), F("off_x"),
                      get8BitFromUL(P120_CONFIG_FLAGS4, P120_FLAGS4_OFFSET_X) - 0x80, -127, 127);
    addUnit(F("-127..127 * 15.6 mg"));
    addFormNumericBox(F("Y-Axis offset"), F("off_y"),
                      get8BitFromUL(P120_CONFIG_FLAGS4, P120_FLAGS4_OFFSET_Y) - 0x80, -127, 127);
    addUnit(F("-127..127 * 15.6 mg"));
    addFormNumericBox(F("Z-Axis offset"), F("off_z"),
                      get8BitFromUL(P120_CONFIG_FLAGS4, P120_FLAGS4_OFFSET_Z) - 0x80, -127, 127);
    addUnit(F("-127..127 * 15.6 mg"));
  }

  // Data retrieval options
  {
    addFormSubHeader(F("Data retrieval"));

    addFormNumericBox(F("Averaging buffer size"), F("average_buf"), P120_AVERAGE_BUFFER, 1, 100);
    addUnit(F("1..100"));

    const __FlashStringHelper *frequencyOptions[] = {
      F("10"),
      F("50") };
    int frequencyValues[] = { P120_FREQUENCY_10, P120_FREQUENCY_50 };
    addFormSelector(F("Measuring frequency"), F("frequency"), 2, frequencyOptions, frequencyValues, P120_FREQUENCY);
    addUnit(F("Hz"));
    addFormNote(F("Values X/Y/Z are updated 1x per second, Controller updates &amp; Value-events are based on 'Interval' setting."));
  }

  return true;
}

// *******************************************************************
// Save the configuration interface
// *******************************************************************
bool P120_data_struct::plugin_webform_save(struct EventStruct *event) {
  for (uint8_t i = 0; i < P120_NR_OUTPUT_VALUES; ++i) {
    const uint8_t pconfigIndex = i + P120_QUERY1_CONFIG_POS;
    const uint8_t choice       = PCONFIG(pconfigIndex);
    sensorTypeHelper_saveOutputSelector(event, pconfigIndex, i, P120_data_struct::valuename(choice, false));
  }


  P120_FREQUENCY = getFormItemInt(F("frequency"));
  uint32_t flags = 0ul;

  set2BitToUL(flags, P120_FLAGS1_RANGE, getFormItemInt(F("range")));
  bitWrite(flags, P120_FLAGS1_ACTIVITY_X,       isFormItemChecked(F("act_x")));
  bitWrite(flags, P120_FLAGS1_ACTIVITY_Y,       isFormItemChecked(F("act_y")));
  bitWrite(flags, P120_FLAGS1_ACTIVITY_Z,       isFormItemChecked(F("act_z")));
  bitWrite(flags, P120_FLAGS1_TAP_X,            isFormItemChecked(F("tap_x")));
  bitWrite(flags, P120_FLAGS1_TAP_Y,            isFormItemChecked(F("tap_y")));
  bitWrite(flags, P120_FLAGS1_TAP_Z,            isFormItemChecked(F("tap_z")));
  bitWrite(flags, P120_FLAGS1_DBL_TAP,          isFormItemChecked(F("dbl_tap")));
  bitWrite(flags, P120_FLAGS1_FREE_FALL,        isFormItemChecked(F("fr_fall")));
  bitWrite(flags, P120_FLAGS1_SEND_ACTIVITY,    isFormItemChecked(F("send_act")));
  bitWrite(flags, P120_FLAGS1_LOG_ACTIVITY,     isFormItemChecked(F("log_act")));
  bitWrite(flags, P120_FLAGS1_EVENT_RAW_VALUES, isFormItemChecked(F("raw_measure")));
  bitWrite(flags, P120_FLAGS1_ANGLE_IN_RAD,     getFormItemInt(F("angle_rad")));
  set8BitToUL(flags, P120_FLAGS1_ACTIVITY_TRESHOLD,   getFormItemInt(F("act_thres")));
  set8BitToUL(flags, P120_FLAGS1_INACTIVITY_TRESHOLD, getFormItemInt(F("inact_thres")));
  P120_CONFIG_FLAGS1 = flags;

  flags = 0ul;
  set8BitToUL(flags, P120_FLAGS2_TAP_TRESHOLD,    getFormItemInt(F("tap_thres")));
  set8BitToUL(flags, P120_FLAGS2_TAP_DURATION,    getFormItemInt(F("tap_dur")));
  set8BitToUL(flags, P120_FLAGS2_DBL_TAP_LATENCY, getFormItemInt(F("dbl_tap_latency")));
  set8BitToUL(flags, P120_FLAGS2_DBL_TAP_WINDOW,  getFormItemInt(F("dbl_tap_window")));
  P120_CONFIG_FLAGS2 = flags;

  flags = 0ul;
  set8BitToUL(flags, P120_FLAGS3_FREEFALL_TRESHOLD, getFormItemInt(F("fr_fall_thres")));
  set8BitToUL(flags, P120_FLAGS3_FREEFALL_DURATION, getFormItemInt(F("fr_fall_dur")));
  P120_CONFIG_FLAGS3 = flags;

  flags = 0ul;
  set8BitToUL(flags, P120_FLAGS4_OFFSET_X, getFormItemInt(F("off_x")) + 0x80);
  set8BitToUL(flags, P120_FLAGS4_OFFSET_Y, getFormItemInt(F("off_y")) + 0x80);
  set8BitToUL(flags, P120_FLAGS4_OFFSET_Z, getFormItemInt(F("off_z")) + 0x80);
  P120_CONFIG_FLAGS4 = flags;

  return true;
}

// *******************************************************************
// Set defaults for the configuration interface
// *******************************************************************
bool P120_data_struct::plugin_set_defaults(struct EventStruct *event) {
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

  PCONFIG(P120_SENSOR_TYPE_INDEX)     = static_cast<int16_t>(Sensor_VType::SENSOR_TYPE_TRIPLE);
  PCONFIG(P120_QUERY1_CONFIG_POS + 0) = 1; // "X RAW"
  PCONFIG(P120_QUERY1_CONFIG_POS + 1) = 2; // "Y RAW"
  PCONFIG(P120_QUERY1_CONFIG_POS + 2) = 3; // "Z RAW"
  PCONFIG(P120_QUERY1_CONFIG_POS + 3) = 0; // "Empty"

  return true;
}

bool P120_data_struct::plugin_get_config_value(struct EventStruct *event, String& string) const
{
  if (initialized()) {
    const bool isPitch =  string.equalsIgnoreCase(F("pitch"));
    const bool isRoll  =  string.equalsIgnoreCase(F("roll"));

    if (isPitch || isRoll) {
      float pitch, roll;

      if (getPitchRoll(event, pitch, roll)) {
        // FIXME TD-er: Use Cache.getTaskDeviceValueDecimals to determine nr decimals
        const int nrDecimals = 2;
        string = toString(isRoll ? roll : pitch, nrDecimals);
        return true;
      }
    }
  }
  return false;
}

void P120_data_struct::plugin_get_device_value_names(struct EventStruct *event)
{
  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    if (i < P120_NR_OUTPUT_VALUES) {
      const uint8_t pconfigIndex = i + P120_QUERY1_CONFIG_POS;
      uint8_t choice             = PCONFIG(pconfigIndex);
      safe_strncpy(
        ExtraTaskSettings.TaskDeviceValueNames[i],
        P120_data_struct::valuename(choice, false),
        sizeof(ExtraTaskSettings.TaskDeviceValueNames[i]));

      // Set decimals for RAW values to 0, Others to 2 decimals
      if (choice <= 3) {
        ExtraTaskSettings.TaskDeviceValueDecimals[i] = 0;
      }
    } else {
      ZERO_FILL(ExtraTaskSettings.TaskDeviceValueNames[i]);
    }
  }
}

const __FlashStringHelper * P120_data_struct::valuename(uint8_t value_nr, bool displayString) {
  switch (static_cast<valueType>(value_nr)) {
    case valueType::Empty:    return displayString ? F("Empty") : F("");
    case valueType::X_RAW:    return displayString ? F("X RAW") : F("X");
    case valueType::Y_RAW:    return displayString ? F("Y RAW") : F("Y");
    case valueType::Z_RAW:    return displayString ? F("Z RAW") : F("Z");
    case valueType::X_g:      return displayString ? F("X (g)") : F("X");
    case valueType::Y_g:      return displayString ? F("Y (g)") : F("Y");
    case valueType::Z_g:      return displayString ? F("Z (g)") : F("Z");
    case valueType::Pitch:    return displayString ? F("Pitch Angle") : F("Pitch");
    case valueType::Roll:     return displayString ? F("Roll Angle")  : F("Roll");
    case valueType::NR_ValueTypes:
      break;
  }
  return F("");
}

bool P120_data_struct::isXYZ(valueType vtype)
{
  return vtype >= valueType::X_RAW &&  vtype <= valueType::Z_g;
}

#endif // if defined(USES_P120) || defined(USES_P125)
