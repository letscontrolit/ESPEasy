#include "../PluginStructs/P099_data_struct.h"

#ifdef USES_P099
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Scheduler.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/SystemVariables.h"

#include "../../ESPEasy_fdwdecl.h"

#include <XPT2046_Touchscreen.h>

P099_data_struct::P099_data_struct() : touchscreen(nullptr) {}

P099_data_struct::~P099_data_struct() {
  reset();
}

/**
 * Proper reset and cleanup.
 */
void P099_data_struct::reset() {
  if (touchscreen != nullptr) {
    delete touchscreen;
    touchscreen = nullptr;
  }
#ifdef PLUGIN_099_DEBUG
  addLog(LOG_LEVEL_INFO, F("P099 DEBUG Touchscreen reset."));
#endif // PLUGIN_099_DEBUG
}

/**
 * Initialize data and set up the touchscreen.
 */
bool P099_data_struct::init(taskIndex_t taskIndex,
                            uint8_t     cs,
                            uint8_t     rotation,
                            bool        flipped,
                            uint8_t     z_treshold,
                            bool        send_xy,
                            bool        send_z,
                            bool        useCalibration,
                            uint16_t    ts_x_res,
                            uint16_t    ts_y_res) {
  reset();

  _address_ts_cs  = cs;
  _z_treshold     = z_treshold;
  _rotation       = rotation;
  _flipped        = flipped;
  _send_xy        = send_xy;
  _send_z         = send_z;
  _useCalibration = useCalibration;
  _ts_x_res       = ts_x_res;
  _ts_y_res       = ts_y_res;

  touchscreen = new (std::nothrow) XPT2046_Touchscreen(_address_ts_cs);
  if (touchscreen != nullptr) {
    touchscreen->setRotation(_rotation);
    touchscreen->setRotationFlipped(_flipped);
    touchscreen->begin();
    loadTouchObjects(taskIndex);
#ifdef PLUGIN_099_DEBUG
    addLog(LOG_LEVEL_INFO, F("P099 DEBUG Plugin & touchscreen initialized."));
   } else {
    addLog(LOG_LEVEL_INFO, F("P099 DEBUG Touchscreen initialisation FAILED."));
#endif // PLUGIN_099_DEBUG
  }
  return isInitialized();
}

/**
 * Properly initialized? then true
 */
bool P099_data_struct::isInitialized() const {
  return touchscreen != nullptr;
}

/**
 * Load the touch objects from the settings, and initialize then properly where needed.
 */
void P099_data_struct::loadTouchObjects(taskIndex_t taskIndex) {
#ifdef PLUGIN_099_DEBUG
  String log = F("P099 DEBUG loadTouchObjects size: ");
  log += sizeof(StoredSettings);
  addLog(LOG_LEVEL_INFO, log);
#endif // PLUGIN_099_DEBUG
  LoadCustomTaskSettings(taskIndex, (uint8_t *)&(StoredSettings), sizeof(StoredSettings));

  for (int i = 0; i < P099_MaxObjectCount; i++) {
    StoredSettings.TouchObjects[i].objectname[P099_MaxObjectNameLength - 1] = 0; // Terminate strings in case of uninitialized data
    SurfaceAreas[i] = 0; // Reset
    TouchTimers[i]  = 0;
    TouchStates[i]  = false;
  }
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
void P099_data_struct::readData(uint16_t *x, uint16_t *y, uint8_t *z) {
  if (isInitialized()) {
    touchscreen->readData(x, y, z);
#ifdef PLUGIN_099_DEBUG
    addLog(LOG_LEVEL_INFO, F("P099 DEBUG readData"));
#endif // PLUGIN_099_DEBUG
  }
}

/**
 * Only set rotation if the touchscreen is initialized.
 */
void P099_data_struct::setRotation(uint8_t n) {
  if (isInitialized()) {
    touchscreen->setRotation(n);
#ifdef PLUGIN_099_DEBUG
    String log = F("P099 DEBUG Rotation set: ");
    log += n;
    addLog(LOG_LEVEL_INFO, log);
#endif // PLUGIN_099_DEBUG
  }
}

/**
 * Only set rotationFlipped if the touchscreen is initialized.
 */
void P099_data_struct::setRotationFlipped(bool flipped) {
  if (isInitialized()) {
    touchscreen->setRotationFlipped(flipped);
#ifdef PLUGIN_099_DEBUG
    String log = F("P099 DEBUG RotationFlipped set: ");
    log += flipped;
    addLog(LOG_LEVEL_INFO, log);
#endif // PLUGIN_099_DEBUG
  }
}

/**
 * Determine if calibration is enabled and usable.
 */
bool P099_data_struct::isCalibrationActive() {
  return    _useCalibration
         && StoredSettings.Calibration.top_left.x > 0
         && StoredSettings.Calibration.top_left.y > 0
         && StoredSettings.Calibration.bottom_right.x > 0
         && StoredSettings.Calibration.bottom_right.y > 0; // Enabled and all values != 0 => Active
}

/**
 * Check within the list of defined objects if we touched one of them.
 * The smallest matching surface is selected if multiple objects overlap.
 * Returns state, and sets selectedObjectName to the best matching object
 */
bool P099_data_struct::isValidAndTouchedTouchObject(uint16_t x, uint16_t y, String &selectedObjectName, int &selectedObjectIndex, uint8_t checkObjectCount) {
  uint32_t lastObjectArea = 0;
  bool     selected = false;
  for (uint8_t objectNr = 0; objectNr < checkObjectCount; objectNr++) {
    String objectName = String(StoredSettings.TouchObjects[objectNr].objectname);
    if ( objectName.length() > 0
      && objectName.substring(0,1 ) != F("_")         // Ignore if name starts with an underscore
      && StoredSettings.TouchObjects[objectNr].bottom_right.x > 0
      && StoredSettings.TouchObjects[objectNr].bottom_right.y > 0) { // Not initial could be valid

      if (SurfaceAreas[objectNr] == 0) { // Need to calculate the surface area
        SurfaceAreas[objectNr] = (StoredSettings.TouchObjects[objectNr].bottom_right.x - StoredSettings.TouchObjects[objectNr].top_left.x) * (StoredSettings.TouchObjects[objectNr].bottom_right.y - StoredSettings.TouchObjects[objectNr].top_left.y);
      }

      if ( StoredSettings.TouchObjects[objectNr].top_left.x <= x
        && StoredSettings.TouchObjects[objectNr].top_left.y <= y
        && StoredSettings.TouchObjects[objectNr].bottom_right.x >= x
        && StoredSettings.TouchObjects[objectNr].bottom_right.y >= y
        && (lastObjectArea == 0 
          || SurfaceAreas[objectNr] < lastObjectArea)) { // Select smallest area that fits the coordinates
        selectedObjectName  = objectName;
        selectedObjectIndex = objectNr;
        lastObjectArea      = SurfaceAreas[objectNr];
        selected            = true;
      }
#ifdef PLUGIN_099_DEBUG
      String log = F("P099 DEBUG Touched: obj: ");
      log += objectName;
      log += ',';
      log += StoredSettings.TouchObjects[objectNr].top_left.x;
      log += ',';
      log += StoredSettings.TouchObjects[objectNr].top_left.y;
      log += ',';
      log += StoredSettings.TouchObjects[objectNr].bottom_right.x;
      log += ',';
      log += StoredSettings.TouchObjects[objectNr].bottom_right.y;
      log += F(" surface:");
      log += SurfaceAreas[objectNr];
      log += F(" x,y:");
      log += x;
      log += ',';
      log += y;
      log += F(" sel:");
      log += selectedObjectName;
      log += '/';
      log += selectedObjectIndex;
      addLog(LOG_LEVEL_INFO, log);
#endif // PLUGIN_099_DEBUG
    }
  }
  return selected;
}

/**
 * Set the enabled/disabled state by inserting or deleting an underscore '_' as the first character of the object name.
 * Checks if the name doesn't exceed the max. length.
 */
bool P099_data_struct::setTouchObjectState(String touchObject, bool state, uint8_t checkObjectCount) {
  if (touchObject.length() == 0 || touchObject.substring(0, 1) == F("_")) return false;
  String findObject = (state ? F("_") : F("")); // When enabling, try to find a disabled object
  findObject += touchObject;
  String thisObject;
  bool   success = false;
  thisObject.reserve(P099_MaxObjectNameLength);
  for (uint8_t objectNr = 0; objectNr < checkObjectCount; objectNr++) {
    thisObject = String(StoredSettings.TouchObjects[objectNr].objectname);
    if (thisObject.length() > 0 && findObject.equalsIgnoreCase(thisObject)) {
      if (state) {
        success = safe_strncpy(StoredSettings.TouchObjects[objectNr].objectname, thisObject.substring(1), P099_MaxObjectNameLength); // Keep original character casing
      } else {
        if (thisObject.length() < P099_MaxObjectNameLength - 2) { // Leave room for the underscore and the terminating 0.
          String disabledObject = F("_");
          disabledObject += thisObject;
          success = safe_strncpy(StoredSettings.TouchObjects[objectNr].objectname, disabledObject, P099_MaxObjectNameLength);
        }
      }
      StoredSettings.TouchObjects[objectNr].objectname[P099_MaxObjectNameLength - 1] = 0; // Just to be safe
#ifdef PLUGIN_099_DEBUG
      String log = F("P099 setTouchObjectState: obj: ");
      log += thisObject;
      if (success) {
      log += F(", new state: ");
      log += (state ? F("en") : F("dis"));
      log += F("abled.");
      } else {
        log += F("failed!");
      }
      addLog(LOG_LEVEL_INFO, log);
#endif // PLUGIN_099_DEBUG
      // break; // Only first one found is processed
    }
  }
  return success;
}

/**
 * Scale the provided raw coordinates to screen-resolution coordinates if calibration is enabled/configured
 */
void P099_data_struct::scaleRawToCalibrated(uint16_t &x, uint16_t &y) {
  if (isCalibrationActive()) {
    uint16_t lx = x - StoredSettings.Calibration.top_left.x;
    if (lx <= 0) {
      x = 0;
    } else {
      if (lx > StoredSettings.Calibration.bottom_right.x) {
        lx = StoredSettings.Calibration.bottom_right.x;
      }
      float x_fact = static_cast<float>(StoredSettings.Calibration.bottom_right.x - StoredSettings.Calibration.top_left.x) / static_cast<float>(_ts_x_res);
      x = static_cast<uint16_t>(round(lx / x_fact));
    }
    uint16_t ly = y - StoredSettings.Calibration.top_left.y;
    if (ly <= 0) {
      y = 0;
    } else {
      if (ly > StoredSettings.Calibration.bottom_right.y) {
        ly = StoredSettings.Calibration.bottom_right.y;
      }
      float y_fact = (StoredSettings.Calibration.bottom_right.y - StoredSettings.Calibration.top_left.y) / _ts_y_res;
      y = static_cast<uint16_t>(round(ly / y_fact));
    }
  }
}

#endif  // ifdef USES_P099