#include "../PluginStructs/P123_data_struct.h"

#ifdef USES_P123

# include "../Helpers/AdafruitGFX_helper.h"

const __FlashStringHelper* toString(P123_TouchType_e tType) {
  switch (tType) {
    case P123_TouchType_e::FT62x6: return F("FT62x6 (0x38)");
    case P123_TouchType_e::GT911_1: return F("GT911 (0x5D)");
    case P123_TouchType_e::GT911_2: return F("GT911 (0x14)");
    case P123_TouchType_e::CST820: return F("CST820 (0x15)");
    case P123_TouchType_e::CST226: return F("CST226 (0x5A)");
    case P123_TouchType_e::AXS15231: return F("AXS15231 (0x3B)");
    case P123_TouchType_e::Automatic: return F("Auto-detect");
  }
  return F("");
}

/**
 * Order must match enum P123_TouchType_e, with Automatic (-1) ignored
 */
const uint8_t P123_i2cAddressValues[] = { FT6X36_ADDR, GT911_ADDR1, GT911_ADDR2, CST820_ADDR, CST226_ADDR, AXS15231_ADDR };

bool P123_data_struct::plugin_i2c_has_address(const int Par1) {
  return intArrayContains(NR_ELEMENTS(P123_i2cAddressValues), P123_i2cAddressValues, Par1);
}

uint8_t P123_data_struct::plugin_i2c_address(P123_TouchType_e touchType) {
  const int tType = static_cast<int>(touchType);

  if ((tType >= 0) && (tType < NR_ELEMENTS(P123_i2cAddressValues))) {
    return P123_i2cAddressValues[tType];
  }
  return 0u;
}

P123_TouchType_e P123_data_struct::getTouchType() {
  if (nullptr != touchscreen) {
    const int stype = touchscreen->sensorType();

    switch (stype) {
      case CT_TYPE_FT6X36: return P123_TouchType_e::FT62x6;
      case CT_TYPE_GT911:

        if (GT911_ADDR1 == touchscreen->getI2CAddress()) {
          return P123_TouchType_e::GT911_1;
        } else {
          return P123_TouchType_e::GT911_2;
        }
      case CT_TYPE_CST820: return P123_TouchType_e::CST820;
      case CT_TYPE_CST226: return P123_TouchType_e::CST226;
      case CT_TYPE_AXS15231: return P123_TouchType_e::AXS15231;
    }
  }
  return P123_TouchType_e::Automatic;
}

int P123_data_struct::getBBCapTouchType(P123_TouchType_e touchType) {
  switch (touchType) {
    case P123_TouchType_e::Automatic: return CT_TYPE_UNKNOWN;
    case P123_TouchType_e::FT62x6: return CT_TYPE_FT6X36;
    case P123_TouchType_e::GT911_1: // Fall through
    case P123_TouchType_e::GT911_2: return CT_TYPE_GT911;
    case P123_TouchType_e::CST820: return CT_TYPE_CST820;
    case P123_TouchType_e::CST226: return CT_TYPE_CST226;
    case P123_TouchType_e::AXS15231: return CT_TYPE_AXS15231;
  }
  return CT_TYPE_UNKNOWN;
}

/**
 * Constructor
 */
P123_data_struct::P123_data_struct(P123_TouchType_e touchType)
  : _touchType(touchType) {
  touchHandler = new (std::nothrow) ESPEasy_TouchHandler(); // Temporary object to be able to call loadTouchObjects
  _i2caddr     = plugin_i2c_address(_touchType);
}

/**
 * Destructor
 */
P123_data_struct::~P123_data_struct() {
  reset();
}

/**
 * Proper reset and cleanup.
 */
void P123_data_struct::reset() {
  # ifdef PLUGIN_123_DEBUG
  addLog(LOG_LEVEL_INFO, F("P123 DEBUG Touchscreen reset."));
  # endif // PLUGIN_123_DEBUG

  delete touchscreen;
  touchscreen = nullptr;
  delete touchHandler;
  touchHandler = nullptr;
}

/**
 * Initialize data and set up the touchscreen.
 */
bool P123_data_struct::init(struct EventStruct *event) {
  _rotation = P123_CONFIG_ROTATION;
  _ts_x_res = P123_CONFIG_X_RES;
  _ts_y_res = P123_CONFIG_Y_RES;

  reset();

  touchHandler = new (std::nothrow) ESPEasy_TouchHandler(static_cast<taskIndex_t>(P123_CONFIG_DISPLAY_TASK),
                                                         static_cast<AdaGFXColorDepth>(P123_COLOR_DEPTH));

  if (nullptr != touchHandler) {
    touchHandler->init(event);

    if (touchHandler->touchEnabled()) {
      touchscreen = new (std::nothrow) BBCapTouch();

      if (nullptr != touchscreen) {
        touchscreen->setThreshold(P123_CONFIG_THRESHOLD);

        P123_TouchType_e touchType = static_cast<P123_TouchType_e>(P123_GET_TOUCH_TYPE);

        if (P123_TouchType_e::Automatic != touchType) { // Manual override
          touchscreen->sensorType(getBBCapTouchType(touchType));
          touchscreen->setI2CAddress(plugin_i2c_address(touchType));
        }

        if (touchscreen->init(-1, -1, P123_RESETPIN, P123_INTERRUPTPIN) != CT_SUCCESS) {
          delete touchscreen;
          touchscreen = nullptr;
        } else {
          setRotation(_rotation);
        }
      }
    }

  # ifdef PLUGIN_123_DEBUG
    addLogMove(LOG_LEVEL_INFO,
               concat(concat(F("P123 DEBUG Plugin"), nullptr != touchscreen ? F(" & touchscreen") : F("")), F(" initialized.")));
  } else {
    addLogMove(LOG_LEVEL_INFO, F("P123 DEBUG Touchscreen initialization FAILED."));
  # endif // PLUGIN_123_DEBUG
  }
  return isInitialized();
}

/**
 * mode: -2 = clear buttons in group, -3 = clear all buttongroups, -1 = draw buttons in group, 0 = initialize buttons
 */
void P123_data_struct::displayButtonGroup(struct EventStruct *event,
                                          int16_t             buttonGroup,
                                          int8_t              mode) {
  # if TOUCH_FEATURE_EXTENDED_TOUCH

  if (nullptr != touchHandler) {
    touchHandler->displayButtonGroup(event, buttonGroup, mode);
  }
  # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
}

/**
 * (Re)Display a button
 */
bool P123_data_struct::displayButton(struct EventStruct *event,
                                     const int8_t      & buttonNr,
                                     int16_t             buttonGroup,
                                     int8_t              mode) {
  # if TOUCH_FEATURE_EXTENDED_TOUCH

  if (nullptr != touchHandler) {
    return touchHandler->displayButton(event, buttonNr, buttonGroup, mode);
  }
  # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
  return false;
}

/**
 * Properly initialized? then true
 */
bool P123_data_struct::isInitialized() const {
  return touchHandler != nullptr && (!touchHandler->touchEnabled() || touchscreen != nullptr);
}

/**
 * Load the settings onto the webpage
 */
bool P123_data_struct::plugin_webform_load(struct EventStruct *event) {
  if (nullptr != touchHandler) {
    return touchHandler->plugin_webform_load(event);
  }
  return false;
}

/**
 * Save the settings from the web page to flash
 */
bool P123_data_struct::plugin_webform_save(struct EventStruct *event) {
  if (nullptr != touchHandler) {
    const bool result = touchHandler->plugin_webform_save(event);
    P123_CONFIG_VTYPE = touchHandler->get_device_valuecount(event); // Store 'locally'
    return result;
  }
  return false;
}

/**
 * Parse and execute the plugin commands, delegated to ESPEasy_TouchHandler
 */
bool P123_data_struct::plugin_write(struct EventStruct *event,
                                    const String      & string) {
  bool   success = false;
  String command;
  String subcommand;

  command    = parseString(string, 1);
  subcommand = parseString(string, 2);

  if (isInitialized() && equals(command, F("touch"))) {
    # ifdef PLUGIN_123_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO, strformat(F("P123 WRITE arguments Par1: %d, 2: %d, 3: %d, 4: %d"),
                                       event->Par1, event->Par2, event->Par3, event->Par4));
    }
    # endif // ifdef PLUGIN_123_DEBUG

    if (equals(subcommand, F("rot"))) {         // touch,rot,<0..3> : Set rotation to 0, 90, 180, 270 degrees
      setRotation(static_cast<uint8_t>(event->Par2 % 4));
      success = true;
    } else if (equals(subcommand, F("flip"))) { // touch,flip,<0|1> : Flip rotation by 0 or 180 degrees
      setRotationFlipped(event->Par2 > 0);
      success = true;
    } else {                                    // Rest of the commands handled by ESPEasy_TouchHandler
      success = touchHandler->plugin_write(event, string);
    }
  }
  return success;
}

/**
 * Every 1/50th second we check if the screen is touched
 */
bool P123_data_struct::plugin_fifty_per_second(struct EventStruct *event) {
  if (isInitialized() && touchHandler->touchEnabled()) {
    if (touched()) {
      int16_t x  = 0;
      int16_t y  = 0;
      int16_t z  = 0;
      int16_t ox = 0;
      int16_t oy = 0;
      readData(x, y, z, ox, oy);

      int16_t rx = x;             // Keep raw values
      int16_t ry = y;
      scaleRawToCalibrated(x, y); // Map to screen coordinates if so configured

      return touchHandler->plugin_fifty_per_second(event, x, y, ox, oy, rx, ry, z);
    } else {
      touchHandler->releaseTouch(event);
    }
  }
  return false;
}

/**
 * Handle getting config values, delegated to ESPEasy_TouchHandler
 */
bool P123_data_struct::plugin_get_config_value(struct EventStruct *event,
                                               String            & string) {
  if (nullptr != touchHandler) {
    return touchHandler->plugin_get_config_value(event, string);
  }
  return false;
}

/**
 * Load the touch objects from the settings, and initialize then properly where needed.
 */
void P123_data_struct::loadTouchObjects(struct EventStruct *event) {
  if (nullptr != touchHandler) {
    touchHandler->loadTouchObjects(event);
  }
}

/**
 * Check if the screen is touched.
 */
bool P123_data_struct::touched() {
  if (isInitialized()) {
    return touchscreen->getSamples(&touchInfo) > 0; // 1 or more points touched
  }
  return false;
}

/**
 * Read the raw data if the touchscreen is initialized.
 */
void P123_data_struct::readData(int16_t& x,
                                int16_t& y,
                                int16_t& z,
                                int16_t& ox,
                                int16_t& oy) {
  if (isInitialized()) {
    x  = touchInfo.x[0]; // Only 1 point used for now.
    y  = touchInfo.y[0];
    z  = touchInfo.pressure[0];
    ox = touchInfo.x[0]; // Change of touch driver has made these arguments obsolete, but in use for touchHandler...
    oy = touchInfo.y[0];
  }
}

/**
 * Set rotation
 */
void P123_data_struct::setRotation(uint8_t n) {
  _rotation = n;

  if (isInitialized()) {
    const bool fl = touchHandler->_flipped;

    switch (_rotation) { // Rotation is handled by touch driver, flipped handled here
      case 0: touchscreen->setOrientation(fl ? 180 : 0, _ts_x_res, _ts_y_res); break;
      case 1: touchscreen->setOrientation(fl ? 270 : 90, _ts_x_res, _ts_y_res); break;
      case 2: touchscreen->setOrientation(fl ? 0 : 180, _ts_x_res, _ts_y_res); break;
      case 3: touchscreen->setOrientation(fl ? 90 : 270, _ts_x_res, _ts_y_res); break;
    }
  }
  # ifdef PLUGIN_123_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLogMove(LOG_LEVEL_INFO, concat(F("P123 DEBUG Rotation set: "), _rotation));
  }
  # endif // PLUGIN_123_DEBUG
}

/**
 * Set rotationFlipped
 */
void P123_data_struct::setRotationFlipped(bool flipped) {
  touchHandler->_flipped = flipped;
  # ifdef PLUGIN_123_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLogMove(LOG_LEVEL_INFO, concat(F("P123 DEBUG RotationFlipped set: "), boolToString(flipped)));
  }
  # endif // PLUGIN_123_DEBUG
}

/**
 * Check within the list of defined objects if we touched one of them.
 * The smallest matching surface is selected if multiple objects overlap.
 * Returns state, and sets selectedObjectName to the best matching object
 */
bool P123_data_struct::isValidAndTouchedTouchObject(const int16_t& x,
                                                    const int16_t& y,
                                                    String       & selectedObjectName,
                                                    int8_t       & selectedObjectIndex) {
  if (nullptr != touchHandler) {
    return touchHandler->isValidAndTouchedTouchObject(x, y, selectedObjectName, selectedObjectIndex);
  }
  return false;
}

/**
 * Get the index of a touch object by name or number
 */
int8_t P123_data_struct::getTouchObjectIndex(struct EventStruct *event,
                                             const String      & touchObject,
                                             bool                isButton) {
  if (nullptr != touchHandler) {
    return touchHandler->getTouchObjectIndex(event, touchObject, isButton);
  }
  return -1;
}

/**
 * Set the enabled/disabled state of an object.
 */
bool P123_data_struct::setTouchObjectState(struct EventStruct *event,
                                           const String      & touchObject,
                                           bool                state) {
  if (nullptr != touchHandler) {
    return touchHandler->setTouchObjectState(event, touchObject, state);
  }
  return false;
}

/**
 * Set the on/off state of a touch-button object.
 */
bool P123_data_struct::setTouchButtonOnOff(struct EventStruct *event,
                                           const String      & touchObject,
                                           bool                state) {
  if (nullptr != touchHandler) {
    return touchHandler->setTouchButtonOnOff(event, touchObject, state);
  }
  return false;
}

/**
 * Scale the provided raw coordinates to screen-resolution coordinates if calibration is enabled/configured
 */
void P123_data_struct::scaleRawToCalibrated(int16_t& x,
                                            int16_t& y) {
  if ((nullptr != touchHandler) && touchHandler->isCalibrationActive()) {
    int16_t lx = x - touchHandler->Touch_Settings.top_left.x;

    if (lx <= 0) {
      x = 0;
    } else {
      if (lx > touchHandler->Touch_Settings.bottom_right.x) {
        lx = touchHandler->Touch_Settings.bottom_right.x;
      }
      float x_fact = static_cast<float>(touchHandler->Touch_Settings.bottom_right.x - touchHandler->Touch_Settings.top_left.x) /
                     static_cast<float>(_ts_x_res);
      x = static_cast<int16_t>(round(lx / x_fact));
    }
    int16_t ly = y - touchHandler->Touch_Settings.top_left.y;

    if (ly <= 0) {
      y = 0;
    } else {
      if (ly > touchHandler->Touch_Settings.bottom_right.y) {
        ly = touchHandler->Touch_Settings.bottom_right.y;
      }
      float y_fact = (touchHandler->Touch_Settings.bottom_right.y - touchHandler->Touch_Settings.top_left.y) / _ts_y_res;
      y = static_cast<int16_t>(round(ly / y_fact));
    }
  }
}

/**
 * Get the current button group
 */
int16_t P123_data_struct::getButtonGroup() const {
  if (nullptr != touchHandler) {
    return touchHandler->getButtonGroup();
  }
  return 0;
}

/**
 * Check if a valid button group, optionally ignoring group 0
 */
bool P123_data_struct::validButtonGroup(int16_t buttonGroup,
                                        bool    ignoreZero) {
  # if TOUCH_FEATURE_EXTENDED_TOUCH

  if (nullptr != touchHandler) {
    return touchHandler->validButtonGroup(buttonGroup, ignoreZero);
  }
  # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
  return false;
}

/**
 * Set the desired button group, must be between the minimum and maximum found values
 */
bool P123_data_struct::setButtonGroup(struct EventStruct *event,
                                      int16_t             buttonGroup) {
  # if TOUCH_FEATURE_EXTENDED_TOUCH

  if (nullptr != touchHandler) {
    return touchHandler->setButtonGroup(event, buttonGroup);
  }
  # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
  return false;
}

/**
 * Increment button group, if max. group > 0 then min. group = 1
 */
bool P123_data_struct::nextButtonGroup(struct EventStruct *event) {
  # if TOUCH_FEATURE_EXTENDED_TOUCH

  if (nullptr != touchHandler) {
    return touchHandler->nextButtonGroup(event);
  }
  # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
  return false;
}

/**
 * Decrement button group, if max. group > 0 then min. group = 1
 */
bool P123_data_struct::prevButtonGroup(struct EventStruct *event) {
  # if TOUCH_FEATURE_EXTENDED_TOUCH

  if (nullptr != touchHandler) {
    return touchHandler->prevButtonGroup(event);
  }
  # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
  return false;
}

/**
 * Increment button group page (+10), if max. group > 0 then min. group page (+10) = 1
 */
bool P123_data_struct::nextButtonPage(struct EventStruct *event) {
  # if TOUCH_FEATURE_EXTENDED_TOUCH

  if (nullptr != touchHandler) {
    return touchHandler->nextButtonPage(event);
  }
  # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
  return false;
}

/**
 * Decrement button group page (-10), if max. group > 0 then min. group = 1
 */
bool P123_data_struct::prevButtonPage(struct EventStruct *event) {
  # if TOUCH_FEATURE_EXTENDED_TOUCH

  if (nullptr != touchHandler) {
    return touchHandler->prevButtonPage(event);
  }
  # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
  return false;
}

#endif // ifdef USES_P123
