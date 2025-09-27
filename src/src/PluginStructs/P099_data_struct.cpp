#include "../PluginStructs/P099_data_struct.h"

#ifdef USES_P099
# include "../ESPEasyCore/ESPEasyNetwork.h"
# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/Scheduler.h"
# include "../Helpers/StringConverter.h"
# include "../Helpers/SystemVariables.h"


# include <XPT2046_Touchscreen.h>

P099_data_struct::P099_data_struct() {
  touchHandler = new (std::nothrow) ESPEasy_TouchHandler(); // Temporary object to be able to call loadTouchObjects
}

P099_data_struct::~P099_data_struct() {
  delete touchscreen;
  delete touchHandler;
}

/**
 * Proper reset and cleanup.
 */
void P099_data_struct::reset() {
  delete touchscreen;
  touchscreen = nullptr;
  delete touchHandler;
  touchHandler = nullptr;

  # ifdef PLUGIN_099_DEBUG
  addLog(LOG_LEVEL_INFO, F("P099 DEBUG Touchscreen reset."));
  # endif // PLUGIN_099_DEBUG
}

/**
 * Initialize data and set up the touchscreen.
 */
bool P099_data_struct::init(struct EventStruct *event,
                            uint8_t             cs,
                            uint8_t             rotation,
                            bool                flipped,
                            uint8_t             z_treshold,
                            uint16_t            ts_x_res,
                            uint16_t            ts_y_res) {
  reset();

  _address_ts_cs = cs;
  _z_treshold    = z_treshold;
  _rotation      = rotation;
  _flipped       = flipped;
  _ts_x_res      = ts_x_res;
  _ts_y_res      = ts_y_res;

  touchHandler = new (std::nothrow) ESPEasy_TouchHandler(static_cast<taskIndex_t>(P099_GET_CONFIG_DISPLAY),
                                                         static_cast<AdaGFXColorDepth>(P099_COLOR_DEPTH));

  if (nullptr != touchHandler) {
    touchHandler->init(event);

    if (touchHandler->touchEnabled()) {
      touchscreen = new (std::nothrow) XPT2046_Touchscreen(_address_ts_cs);

      if (touchscreen != nullptr) {
        touchscreen->setRotation(_rotation);
        touchscreen->setRotationFlipped(_flipped);
        touchscreen->begin();
      }
    }

    // loadTouchObjects(event);
  # ifdef PLUGIN_099_DEBUG
    addLogMove(LOG_LEVEL_INFO,
               concat(concat(F("P099 DEBUG Plugin"), nullptr != touchscreen ? F(" & touchscreen") : F("")), F(" initialized.")));
  } else {
    addLog(LOG_LEVEL_INFO, F("P099 DEBUG Touchscreen initialisation FAILED."));
  # endif // PLUGIN_099_DEBUG
  }
  return isInitialized();
}

/**
 * Properly initialized? then true
 */
bool P099_data_struct::isInitialized() const {
  return touchHandler != nullptr && (!touchHandler->touchEnabled() || touchscreen != nullptr);
}

/**
 * Load the settings onto the webpage
 */
bool P099_data_struct::plugin_webform_load(struct EventStruct *event) {
  if (nullptr != touchHandler) {
    return touchHandler->plugin_webform_load(event);
  }
  return false;
}

/**
 * Save the settings from the web page to flash
 */
bool P099_data_struct::plugin_webform_save(struct EventStruct *event) {
  if (nullptr != touchHandler) {
    const bool result = touchHandler->plugin_webform_save(event);
    P099_SET_CONFIG_VTYPE(touchHandler->get_device_valuecount(event)); // Store 'locally'
    return result;
  }
  return false;
}

/**
 * Every 1/50th second we check if the screen is touched
 */
bool P099_data_struct::plugin_fifty_per_second(struct EventStruct *event) {
  if (isInitialized() && touchHandler->touchEnabled()) {
    if (touched()) {
      int16_t x = 0;
      int16_t y = 0;
      uint8_t z = 0;
      readData(x, y, z);

      if ((z == P099_TOUCH_Z_INVALID) || (z <= _z_treshold)) { // Not past the threshold
        return false;
      }

      int16_t rx = x;             // Keep raw values
      int16_t ry = y;
      scaleRawToCalibrated(x, y); // Map to screen coordinates if so configured

      return touchHandler->plugin_fifty_per_second(event, x, y, x, y, rx, ry, z);
    } else {
      touchHandler->releaseTouch(event);
    }
  }
  return false;
}

/**
 * Handle getting config values, delegated to ESPEasy_TouchHandler
 */
bool P099_data_struct::plugin_get_config_value(struct EventStruct *event,
                                               String            & string) {
  if (nullptr != touchHandler) {
    return touchHandler->plugin_get_config_value(event, string);
  }
  return false;
}

/**
 * Check if the screen is touched.
 */
bool P099_data_struct::touched() {
  if (isInitialized()) {
    return touchscreen->touched();
  }
  return false;
}

/**
 * Read the raw data if the touchscreen is initialized.
 */
void P099_data_struct::readData(int16_t& x, int16_t& y, uint8_t& z) {
  if (isInitialized()) {
    uint16_t cx = x;
    uint16_t cy = y;
    touchscreen->readData(&cx, &cy, &z);
    x = cx;
    y = cy;
    # ifdef PLUGIN_099_DEBUG
    addLog(LOG_LEVEL_INFO, F("P099 DEBUG readData"));
    # endif // PLUGIN_099_DEBUG
  }
}

/**
 * Only set rotation if the touchscreen is initialized.
 */
void P099_data_struct::setRotation(uint8_t n) {
  if (isInitialized()) {
    touchscreen->setRotation(n);
    # ifdef PLUGIN_099_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLogMove(LOG_LEVEL_INFO, concat(F("P099 DEBUG Rotation set: "), (int)n));
    }
    # endif // PLUGIN_099_DEBUG
  }
}

/**
 * Only set rotationFlipped if the touchscreen is initialized.
 */
void P099_data_struct::setRotationFlipped(bool flipped) {
  if (isInitialized()) {
    touchscreen->setRotationFlipped(flipped);
    # ifdef PLUGIN_099_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("P099 DEBUG RotationFlipped set: ");
      log += flipped;
      addLogMove(LOG_LEVEL_INFO, log);
    }
    # endif // PLUGIN_099_DEBUG
  }
}

/**
 * Scale the provided raw coordinates to screen-resolution coordinates if calibration is enabled/configured
 */
void P099_data_struct::scaleRawToCalibrated(int16_t& x,
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
 * Parse and execute the plugin commands, delegated to ESPEasy_TouchHandler
 */
bool P099_data_struct::plugin_write(struct EventStruct *event, const String& string) {
  bool success            = false;
  const String command    = parseString(string, 1);
  const String subcommand = parseString(string, 2);

  if (isInitialized() && equals(command, F("touch"))) {
    # ifdef PLUGIN_099_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO, strformat(F("P099 WRITE arguments Par1: %d, 2: %d, 3: %d, 4: %d"),
                                       event->Par1, event->Par2, event->Par3, event->Par4));
    }
    # endif // ifdef PLUGIN_099_DEBUG

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

#endif // ifdef USES_P099
