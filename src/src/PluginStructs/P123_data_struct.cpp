#include "../PluginStructs/P123_data_struct.h"

#ifdef USES_P123

# include "../ESPEasyCore/ESPEasyNetwork.h"

# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/Scheduler.h"
# include "../Helpers/StringConverter.h"
# include "../Helpers/SystemVariables.h"

# include "../Commands/InternalCommands.h"

/**
 * Constructor
 */
P123_data_struct::P123_data_struct()
  : touchscreen(nullptr) {
  touchHandler = new (std::nothrow) ESPEasy_TouchHandler(); // Temporary object to be able to call loadTouchObjects
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
  addLogMove(LOG_LEVEL_INFO, F("P123 DEBUG Touchscreen reset."));
  # endif // PLUGIN_123_DEBUG

  if (nullptr != touchscreen) { delete touchscreen; }
  touchscreen = nullptr;

  if (nullptr != touchHandler) { delete touchHandler; }
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

  touchscreen = new (std::nothrow) Adafruit_FT6206();

  if (nullptr != touchscreen) {
    if (nullptr != touchHandler) { delete touchHandler; }
    touchHandler = new (std::nothrow) ESPEasy_TouchHandler(P123_CONFIG_DISPLAY_TASK,
                                                           static_cast<AdaGFXColorDepth>(P123_COLOR_DEPTH));
  }

  if (isInitialized()) {
    touchscreen->begin(P123_CONFIG_THRESHOLD);

    touchHandler->init(event);

  # ifdef PLUGIN_123_DEBUG
    addLogMove(LOG_LEVEL_INFO, F("P123 DEBUG Plugin & touchscreen initialized."));
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
  return touchscreen != nullptr && touchHandler != nullptr;
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
    return touchHandler->plugin_webform_save(event);
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

  if (isInitialized() && command.equals(F("touch"))) {
    # ifdef PLUGIN_123_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log(concat(F("P123 WRITE arguments Par1:"), event->Par1));
      log += concat(F(", 2: "), event->Par2);
      log += concat(F(", 3: "), event->Par3);
      log += concat(F(", 4: "), event->Par4);
      addLog(LOG_LEVEL_INFO, log);
    }
    # endif // ifdef PLUGIN_123_DEBUG

    if (subcommand.equals(F("rot"))) {         // touch,rot,<0..3> : Set rotation to 0, 90, 180, 270 degrees
      setRotation(static_cast<uint8_t>(event->Par2 % 4));
      success = true;
    } else if (subcommand.equals(F("flip"))) { // touch,flip,<0|1> : Flip rotation by 0 or 180 degrees
      setRotationFlipped(event->Par2 > 0);
      success = true;
    } else {                                   // Rest of the commands handled by ESPEasy_TouchHandler
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
    return touchscreen->touched();
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
    FT_Point p = touchscreen->getPoint();

    int16_t _x = p.x;
    int16_t _y = p.y;

    // Rotate, as the driver doesn't provide that, use native touch-panel resolution
    switch (_rotation) {
      case P123_ROTATION_90:

        if (touchHandler->_flipped) {
          p.x = map(_y, 0, P123_TOUCH_Y_NATIVE, P123_TOUCH_Y_NATIVE, 0);
          p.y = _x;
        } else {
          p.x = _y;
          p.y = map(_x, 0, P123_TOUCH_X_NATIVE, P123_TOUCH_X_NATIVE, 0);
        }
        break;
      case P123_ROTATION_180:

        if (!touchHandler->_flipped) { // Change only when not flipped
          p.x = map(_x, 0, P123_TOUCH_X_NATIVE, P123_TOUCH_X_NATIVE, 0);
          p.y = map(_y, 0, P123_TOUCH_Y_NATIVE, P123_TOUCH_Y_NATIVE, 0);
        }
        break;
      case P123_ROTATION_270:

        if (touchHandler->_flipped) {
          p.x = _y;
          p.y = map(_x, 0, P123_TOUCH_X_NATIVE, P123_TOUCH_X_NATIVE, 0);
        } else {
          p.x = map(_y, 0, P123_TOUCH_Y_NATIVE, P123_TOUCH_Y_NATIVE, 0);
          p.y = _x;
        }
        break;
      case P123_ROTATION_0:
      default:

        if (touchHandler->_flipped) {
          p.x = map(p.x, 0, P123_TOUCH_X_NATIVE, P123_TOUCH_X_NATIVE, 0);
          p.y = map(p.y, 0, P123_TOUCH_Y_NATIVE, P123_TOUCH_Y_NATIVE, 0);
        }
        break;
    }

    x  = p.x;
    y  = p.y;
    z  = p.z;
    ox = _x;
    oy = _y;
  }
}

/**
 * Set rotation
 */
void P123_data_struct::setRotation(uint8_t n) {
  _rotation = n;
  # ifdef PLUGIN_123_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLogMove(LOG_LEVEL_INFO, concat(F("P123 DEBUG Rotation set: "), n));
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
    addLogMove(LOG_LEVEL_INFO, concat(F("P123 DEBUG RotationFlipped set: "), flipped));
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
  return false;
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
int16_t P123_data_struct::getButtonGroup() {
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
bool P123_data_struct::incrementButtonGroup(struct EventStruct *event) {
  # if TOUCH_FEATURE_EXTENDED_TOUCH

  if (nullptr != touchHandler) {
    return touchHandler->incrementButtonGroup(event);
  }
  # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
  return false;
}

/**
 * Decrement button group, if max. group > 0 then min. group = 1
 */
bool P123_data_struct::decrementButtonGroup(struct EventStruct *event) {
  # if TOUCH_FEATURE_EXTENDED_TOUCH

  if (nullptr != touchHandler) {
    return touchHandler->decrementButtonGroup(event);
  }
  # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
  return false;
}

/**
 * Increment button group page (+10), if max. group > 0 then min. group page (+10) = 1
 */
bool P123_data_struct::incrementButtonPage(struct EventStruct *event) {
  # if TOUCH_FEATURE_EXTENDED_TOUCH

  if (nullptr != touchHandler) {
    return touchHandler->incrementButtonPage(event);
  }
  # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
  return false;
}

/**
 * Decrement button group page (-10), if max. group > 0 then min. group = 1
 */
bool P123_data_struct::decrementButtonPage(struct EventStruct *event) {
  # if TOUCH_FEATURE_EXTENDED_TOUCH

  if (nullptr != touchHandler) {
    return touchHandler->decrementButtonPage(event);
  }
  # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
  return false;
}

#endif // ifdef USES_P123
