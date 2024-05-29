#include "../Helpers/ESPEasy_TouchHandler.h"

#ifdef PLUGIN_USES_TOUCHHANDLER

# include "../Commands/ExecuteCommand.h"

/****************************************************************************
 * toString: Display-value for the touch action
 ***************************************************************************/
# if TOUCH_FEATURE_EXTENDED_TOUCH
const __FlashStringHelper* toString(Touch_action_e action) {
  switch (action) {
    case Touch_action_e::Default: return F("Default");
    case Touch_action_e::ActivateGroup: return F("Activate Group");
    case Touch_action_e::IncrementGroup: return F("Next Group");
    case Touch_action_e::DecrementGroup: return F("Previous Group");
    case Touch_action_e::IncrementPage: return F("Next Page (+10)");
    case Touch_action_e::DecrementPage: return F("Previous Page (-10)");
  }
  return F("Unsupported!");
}

# endif // if TOUCH_FEATURE_EXTENDED_TOUCH

/****************************************************************************
 * toString: Display-value for the swipe action
 ***************************************************************************/
# if TOUCH_FEATURE_EXTENDED_TOUCH && TOUCH_FEATURE_SWIPE
const __FlashStringHelper* toString(Swipe_action_e action) {
  switch (action) {
    case Swipe_action_e::Up: return F("Up");
    case Swipe_action_e::UpRight: return F("Up-Right");
    case Swipe_action_e::Right: return F("Right");
    case Swipe_action_e::RightDown: return F("Right-Down");
    case Swipe_action_e::Down: return F("Down");
    case Swipe_action_e::DownLeft: return F("Down-Left");
    case Swipe_action_e::Left: return F("Left");
    case Swipe_action_e::LeftUp: return F("Left-Up");
    case Swipe_action_e::None: return F("None");
    case Swipe_action_e::SwipeAction_MAX: break;
  }
  return F("Unknown");
}

# endif // if TOUCH_FEATURE_EXTENDED_TOUCH && TOUCH_FEATURE_SWIPE

/**
 * Constructors
 */
ESPEasy_TouchHandler::ESPEasy_TouchHandler() {}

ESPEasy_TouchHandler::ESPEasy_TouchHandler(const taskIndex_t     & displayTask,
                                           const AdaGFXColorDepth& colorDepth)
  : _displayTask(displayTask), _colorDepth(colorDepth) {}

/**
 * Destructor
 */
ESPEasy_TouchHandler::~ESPEasy_TouchHandler() {}

/**
 * Load the touch objects from the settings, and initialize them properly where needed.
 */
void ESPEasy_TouchHandler::loadTouchObjects(struct EventStruct *event) {
  # ifdef TOUCH_DEBUG
  addLog(LOG_LEVEL_INFO, F("TOUCH DEBUG loadTouchObjects"));
  # endif // TOUCH_DEBUG
  LoadCustomTaskSettings(event->TaskIndex, settingsArray, TOUCH_ARRAY_SIZE, 0);

  lastObjectIndex = TOUCH_OBJECT_INDEX_START - 1; // START must be > 0!!!

  objectCount = 0;
  _buttonGroups.clear();                          // Clear groups
  _buttonGroups.insert(0u);                       // Always have group 0

  for (uint8_t i = TOUCH_OBJECT_INDEX_END; i >= TOUCH_OBJECT_INDEX_START; --i) {
    if (!settingsArray[i].isEmpty() && (lastObjectIndex < TOUCH_OBJECT_INDEX_START)) {
      lastObjectIndex = i;
      objectCount++; // Count actual number of objects
    }
  }

  // Get calibration and common settings
  Touch_Settings.calibrationEnabled = parseStringToInt(settingsArray[TOUCH_CALIBRATION_START],
                                                       TOUCH_CALIBRATION_ENABLED, TOUCH_SETTINGS_SEPARATOR) == 1;
  Touch_Settings.logEnabled = parseStringToInt(settingsArray[TOUCH_CALIBRATION_START],
                                               TOUCH_CALIBRATION_LOG_ENABLED, TOUCH_SETTINGS_SEPARATOR) == 1;
  int lSettings = 0;

  bitWrite(lSettings, TOUCH_FLAGS_SEND_XY,          TOUCH_TS_SEND_XY); // Defaults initialized
  bitWrite(lSettings, TOUCH_FLAGS_SEND_Z,           TOUCH_TS_SEND_Z);
  bitWrite(lSettings, TOUCH_FLAGS_SEND_OBJECTNAME,  TOUCH_TS_SEND_OBJECTNAME);
  bitWrite(lSettings, TOUCH_FLAGS_DEDUPLICATE,      TOUCH_TS_DEDUPLICATE);
  bitWrite(lSettings, TOUCH_FLAGS_INIT_OBJECTEVENT, TOUCH_TS_INIT_OBJECTEVENT);
  Touch_Settings.flags = parseStringToInt(settingsArray[TOUCH_CALIBRATION_START],
                                          TOUCH_COMMON_FLAGS, TOUCH_SETTINGS_SEPARATOR, lSettings);
  Touch_Settings.top_left.x     = parseStringToInt(settingsArray[TOUCH_CALIBRATION_START], TOUCH_CALIBRATION_TOP_X, TOUCH_SETTINGS_SEPARATOR);
  Touch_Settings.top_left.y     = parseStringToInt(settingsArray[TOUCH_CALIBRATION_START], TOUCH_CALIBRATION_TOP_Y, TOUCH_SETTINGS_SEPARATOR);
  Touch_Settings.bottom_right.x = parseStringToInt(settingsArray[TOUCH_CALIBRATION_START],
                                                   TOUCH_CALIBRATION_BOTTOM_X,
                                                   TOUCH_SETTINGS_SEPARATOR);
  Touch_Settings.bottom_right.y = parseStringToInt(settingsArray[TOUCH_CALIBRATION_START],
                                                   TOUCH_CALIBRATION_BOTTOM_Y,
                                                   TOUCH_SETTINGS_SEPARATOR);
  Touch_Settings.debounceMs = parseStringToInt(settingsArray[TOUCH_CALIBRATION_START], TOUCH_COMMON_DEBOUNCE_MS, TOUCH_SETTINGS_SEPARATOR,
                                               TOUCH_DEBOUNCE_MILLIS);
  # if TOUCH_FEATURE_EXTENDED_TOUCH
  Touch_Settings.colorOn = parseStringToInt(settingsArray[TOUCH_CALIBRATION_START],
                                            TOUCH_COMMON_DEF_COLOR_ON, TOUCH_SETTINGS_SEPARATOR);
  Touch_Settings.colorOff = parseStringToInt(settingsArray[TOUCH_CALIBRATION_START],
                                             TOUCH_COMMON_DEF_COLOR_OFF, TOUCH_SETTINGS_SEPARATOR);
  Touch_Settings.colorBorder = parseStringToInt(settingsArray[TOUCH_CALIBRATION_START],
                                                TOUCH_COMMON_DEF_COLOR_BORDER, TOUCH_SETTINGS_SEPARATOR);
  Touch_Settings.colorCaption = parseStringToInt(settingsArray[TOUCH_CALIBRATION_START],
                                                 TOUCH_COMMON_DEF_COLOR_CAPTION, TOUCH_SETTINGS_SEPARATOR);
  Touch_Settings.colorDisabled = parseStringToInt(settingsArray[TOUCH_CALIBRATION_START],
                                                  TOUCH_COMMON_DEF_COLOR_DISABLED, TOUCH_SETTINGS_SEPARATOR);
  Touch_Settings.colorDisabledCaption = parseStringToInt(settingsArray[TOUCH_CALIBRATION_START],
                                                         TOUCH_COMMON_DEF_COLOR_DISABCAPT, TOUCH_SETTINGS_SEPARATOR);

  if ((Touch_Settings.colorOn              == 0u) && // Validate and set defaults
      (Touch_Settings.colorOff             == 0u) &&
      (Touch_Settings.colorCaption         == 0u) &&
      (Touch_Settings.colorBorder          == 0u) &&
      (Touch_Settings.colorDisabled        == 0u) &&
      (Touch_Settings.colorDisabledCaption == 0u)) {
    Touch_Settings.colorOn              = TOUCH_DEFAULT_COLOR_ON;
    Touch_Settings.colorOff             = TOUCH_DEFAULT_COLOR_OFF;
    Touch_Settings.colorCaption         = TOUCH_DEFAULT_COLOR_CAPTION;
    Touch_Settings.colorBorder          = TOUCH_DEFAULT_COLOR_BORDER;
    Touch_Settings.colorDisabled        = TOUCH_DEFAULT_COLOR_DISABLED;
    Touch_Settings.colorDisabledCaption = TOUCH_DEFAULT_COLOR_DISABLED_CAPTION;
  }
  #  if TOUCH_FEATURE_SWIPE
  Touch_Settings.swipeMinimal = parseStringToInt(settingsArray[TOUCH_CALIBRATION_START],
                                                 TOUCH_COMMON_SWIPE_MINIMAL, TOUCH_SETTINGS_SEPARATOR, TOUCH_DEF_SWIPE_MINIMAL);
  Touch_Settings.swipeMargin = parseStringToInt(settingsArray[TOUCH_CALIBRATION_START],
                                                TOUCH_COMMON_SWIPE_MARGIN, TOUCH_SETTINGS_SEPARATOR, TOUCH_DEF_SWIPE_MARGIN);
  #  endif // if TOUCH_FEATURE_SWIPE
  # endif // if TOUCH_FEATURE_EXTENDED_TOUCH

  settingsArray[TOUCH_CALIBRATION_START].clear(); // Free a little memory

  // Buffer some settings, mostly for readability, but also to be able to set from write command
  _flipped     = bitRead(Touch_Settings.flags, TOUCH_FLAGS_ROTATION_FLIPPED);
  _deduplicate = bitRead(Touch_Settings.flags, TOUCH_FLAGS_DEDUPLICATE);

  TouchObjects.clear();

  if (objectCount > 0) {
    TouchObjects.reserve(objectCount);
    uint8_t t = 0u;

    for (uint8_t i = TOUCH_OBJECT_INDEX_START; i <= lastObjectIndex; ++i) {
      if (!settingsArray[i].isEmpty()) {
        TouchObjects.push_back(tTouchObjects());
        TouchObjects[t].flags          = parseStringToInt(settingsArray[i], TOUCH_OBJECT_FLAGS, TOUCH_SETTINGS_SEPARATOR);
        TouchObjects[t].objectName     = parseStringKeepCase(settingsArray[i], TOUCH_OBJECT_NAME, TOUCH_SETTINGS_SEPARATOR);
        TouchObjects[t].top_left.x     = parseStringToInt(settingsArray[i], TOUCH_OBJECT_COORD_TOP_X, TOUCH_SETTINGS_SEPARATOR);
        TouchObjects[t].top_left.y     = parseStringToInt(settingsArray[i], TOUCH_OBJECT_COORD_TOP_Y, TOUCH_SETTINGS_SEPARATOR);
        TouchObjects[t].width_height.x = parseStringToInt(settingsArray[i], TOUCH_OBJECT_COORD_WIDTH, TOUCH_SETTINGS_SEPARATOR);
        TouchObjects[t].width_height.y = parseStringToInt(settingsArray[i], TOUCH_OBJECT_COORD_HEIGHT, TOUCH_SETTINGS_SEPARATOR);
        # if TOUCH_FEATURE_EXTENDED_TOUCH
        TouchObjects[t].colorOn              = parseStringToInt(settingsArray[i], TOUCH_OBJECT_COLOR_ON, TOUCH_SETTINGS_SEPARATOR);
        TouchObjects[t].colorOff             = parseStringToInt(settingsArray[i], TOUCH_OBJECT_COLOR_OFF, TOUCH_SETTINGS_SEPARATOR);
        TouchObjects[t].colorCaption         = parseStringToInt(settingsArray[i], TOUCH_OBJECT_COLOR_CAPTION, TOUCH_SETTINGS_SEPARATOR);
        TouchObjects[t].captionOn            = parseStringKeepCase(settingsArray[i], TOUCH_OBJECT_CAPTION_ON, TOUCH_SETTINGS_SEPARATOR);
        TouchObjects[t].captionOff           = parseStringKeepCase(settingsArray[i], TOUCH_OBJECT_CAPTION_OFF, TOUCH_SETTINGS_SEPARATOR);
        TouchObjects[t].colorBorder          = parseStringToInt(settingsArray[i], TOUCH_OBJECT_COLOR_BORDER, TOUCH_SETTINGS_SEPARATOR);
        TouchObjects[t].colorDisabled        = parseStringToInt(settingsArray[i], TOUCH_OBJECT_COLOR_DISABLED, TOUCH_SETTINGS_SEPARATOR);
        TouchObjects[t].colorDisabledCaption = parseStringToInt(settingsArray[i], TOUCH_OBJECT_COLOR_DISABCAPT, TOUCH_SETTINGS_SEPARATOR);
        TouchObjects[t].groupFlags           = parseStringToInt(settingsArray[i], TOUCH_OBJECT_GROUPFLAGS, TOUCH_SETTINGS_SEPARATOR);

        const uint8_t g = get8BitFromUL(TouchObjects[t].flags, TOUCH_OBJECT_FLAG_GROUP);

        if (!validButtonGroup(g)) {
          _buttonGroups.insert(g);
        }
        # endif // if TOUCH_FEATURE_EXTENDED_TOUCH

        TouchObjects[t].SurfaceAreas = 0u; // Reset runtime stuff
        TouchObjects[t].TouchTimers  = 0u;
        TouchObjects[t].TouchStates  = 0;

        # if TOUCH_FEATURE_EXTENDED_TOUCH && TOUCH_FEATURE_SWIPE

        // Check if a slider/gauge with range not including 0 is used, then set starting value closest to 0
        if (bitRead(TouchObjects[t].flags, TOUCH_OBJECT_FLAG_SLIDER) && !TouchObjects[t].captionOff.isEmpty()) {
          int16_t _value    = 0;
          int16_t lowRange  = 0;
          int16_t highRange = 100;

          if (parseRangeToInt16(TouchObjects[t].captionOff, lowRange, highRange)) {
            if (lowRange > highRange) {
              if (_value < highRange) {
                _value = highRange;
              } else if (_value > lowRange) {
                _value = lowRange;
              }
            } else {
              if (_value < lowRange) {
                _value = lowRange;
              } else if (_value > highRange) {
                _value = highRange;
              }
            }
            TouchObjects[t].TouchStates = _value;
          }
        }
        # endif // if TOUCH_FEATURE_EXTENDED_TOUCH && TOUCH_FEATURE_SWIPE

        t++;

        settingsArray[i].clear(); // Free a little memory
      }
    }
  }
}

/**
 * init
 */
void ESPEasy_TouchHandler::init(struct EventStruct *event) {
  if (!_settingsLoaded) {
    loadTouchObjects(event);
    _settingsLoaded = true;
  }

  # if TOUCH_FEATURE_EXTENDED_TOUCH
  _touchIgnored = bitRead(Touch_Settings.flags, TOUCH_FLAGS_IGNORE_TOUCH);

  if (bitRead(Touch_Settings.flags, TOUCH_FLAGS_SEND_OBJECTNAME) &&
      bitRead(Touch_Settings.flags, TOUCH_FLAGS_INIT_OBJECTEVENT)) {
    if (_buttonGroups.size() > 1) {                // Multiple groups?
      displayButtonGroup(event, _buttonGroup, -3); // Clear all displayed groups
    }
    _buttonGroup = get8BitFromUL(Touch_Settings.flags, TOUCH_FLAGS_INITIAL_GROUP);
    #  ifdef TOUCH_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLogMove(LOG_LEVEL_INFO, strformat(F("TOUCH DEBUG group: %d, max group: %d"), _buttonGroup, *_buttonGroups.crbegin()));
    }
    #  endif // ifdef TOUCH_DEBUG

    displayButtonGroup(event, _buttonGroup); // Initialize selected group and group 0

    #  ifdef TOUCH_DEBUG
    addLog(LOG_LEVEL_INFO, F("TOUCH DEBUG group done."));
    #  endif // ifdef TOUCH_DEBUG
  }
  # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
}

/**
 * helper function: use parseString() to read an argument, and convert that to an int value
 */
int ESPEasy_TouchHandler::parseStringToInt(const String & string,
                                           const uint8_t& indexFind,
                                           const char   & separator,
                                           const int    & defaultValue) {
  const String parsed = parseStringKeepCase(string, indexFind, separator);
  int32_t result      = defaultValue;

  validIntFromString(parsed, result);

  return result;
}

/**
 * Determine if calibration is enabled and usable.
 */
bool ESPEasy_TouchHandler::isCalibrationActive() {
  return _useCalibration
         && (Touch_Settings.top_left.x != 0 ||
             Touch_Settings.top_left.y != 0 ||
             Touch_Settings.bottom_right.x > Touch_Settings.top_left.x ||
             Touch_Settings.bottom_right.y > Touch_Settings.top_left.y); // Enabled and any value != 0 => Active
}

/**
 * Check within the list of defined objects if we touched one of them.
 * Must be in the current button group or in button group 0.
 * The smallest matching surface is selected if multiple objects overlap.
 * Returns state, sets selectedObjectName to the best matching object name
 * and selectedObjectIndex to the index into the TouchObjects vector.
 */
bool ESPEasy_TouchHandler::isValidAndTouchedTouchObject(const int16_t& x,
                                                        const int16_t& y,
                                                        String       & selectedObjectName,
                                                        int8_t       & selectedObjectIndex) {
  uint32_t lastObjectArea = 0u;
  bool     selected       = false;
  const uint16_t _x       = static_cast<uint16_t>(x);
  const uint16_t _y       = static_cast<uint16_t>(y);

  for (size_t objectNr = 0; objectNr < TouchObjects.size(); ++objectNr) {
    const uint8_t group = get8BitFromUL(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_GROUP);

    if (!TouchObjects[objectNr].objectName.isEmpty()
        && bitRead(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_ENABLED)
        && (TouchObjects[objectNr].width_height.x != 0)
        && (TouchObjects[objectNr].width_height.y != 0) // Not initial could be valid
        && ((group == 0) || (group == _buttonGroup))) { // Group 0 is always active
      if (TouchObjects[objectNr].SurfaceAreas == 0) {   // Need to calculate the surface area
        TouchObjects[objectNr].SurfaceAreas = TouchObjects[objectNr].width_height.x * TouchObjects[objectNr].width_height.y;
      }

      if ((TouchObjects[objectNr].top_left.x <= _x)
          && (TouchObjects[objectNr].top_left.y <= _y)
          && ((TouchObjects[objectNr].width_height.x + TouchObjects[objectNr].top_left.x) >= _x)
          && ((TouchObjects[objectNr].width_height.y + TouchObjects[objectNr].top_left.y) >= _y)
          && ((lastObjectArea == 0) ||
              (TouchObjects[objectNr].SurfaceAreas < lastObjectArea))) { // Select smallest area that fits the coordinates
        selectedObjectName  = TouchObjects[objectNr].objectName;
        selectedObjectIndex = objectNr;
        lastObjectArea      = TouchObjects[objectNr].SurfaceAreas;
        selected            = true;
      }
      # ifdef TOUCH_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        addLog(LOG_LEVEL_DEBUG,
               strformat(F("TOUCH DEBUG Touched: obj: %s,%d,%d,%d,%d surface:%d x,y:%d,%d sel:%s/%d/%c"),
                         TouchObjects[objectNr].objectName.c_str(),
                         TouchObjects[objectNr].top_left.x,
                         TouchObjects[objectNr].top_left.y,
                         TouchObjects[objectNr].width_height.x,
                         TouchObjects[objectNr].width_height.y,
                         TouchObjects[objectNr].SurfaceAreas,
                         x,
                         y,
                         selectedObjectName.c_str(),
                         selectedObjectIndex,
                         selected ? 'T' : 'f'));
      }
      # endif // ifdef TOUCH_DEBUG
    }
  }
  return selected;
}

/**
 * Get either the int value or index of the objectName provided, optionally a button object
 */
int8_t ESPEasy_TouchHandler::getTouchObjectIndex(struct EventStruct *event,
                                                 const String      & touchObject,
                                                 const bool        & isButton) {
  if (touchObject.isEmpty()) { return -1; }

  int32_t index = -1;
  int32_t idx   = -1;

  if ((idx = touchObject.indexOf('.')) > -1) {
    String part = touchObject.substring(0, idx);

    # if TOUCH_FEATURE_EXTENDED_TOUCH

    int32_t grp = -1;

    if (validIntFromString(part, grp) && validButtonGroup(static_cast<int16_t>(grp), false)) {
      part = touchObject.substring(idx + 1);
      int32_t btn = -1;

      if (validIntFromString(part, btn)) {
        idx = 0;

        for (size_t objectNr = 0; objectNr < TouchObjects.size(); ++objectNr) {
          if (!TouchObjects[objectNr].objectName.isEmpty()
              && (get8BitFromUL(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_GROUP) == static_cast<uint8_t>(grp))
              && (!isButton || bitRead(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_BUTTON))) {
            idx++;

            if (idx == btn) {
              return static_cast<int8_t>(objectNr);
            }
          }
        }
      } else {
        return -1; // Invalid button name
      }
    } else {
      return -1;   // Invalid group number
    }
    # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
  }

  // ATTENTION: Any externally provided objectNumber is 1-based, result is 0-based
  if (validIntFromString(touchObject, index) &&
      (index > 0) &&
      (index <= static_cast<int>(TouchObjects.size()))) {
    return static_cast<int8_t>(index - 1);
  }

  for (size_t objectNr = 0; objectNr < TouchObjects.size(); ++objectNr) {
    if (!TouchObjects[objectNr].objectName.isEmpty()
        && touchObject.equalsIgnoreCase(TouchObjects[objectNr].objectName)
        && (!isButton || bitRead(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_BUTTON))) {
      return static_cast<int8_t>(objectNr);
    }
  }
  return -1;
}

/**
 * Set the enabled/disabled state of an object. Will redraw if a button object.
 */
bool ESPEasy_TouchHandler::setTouchObjectState(struct EventStruct *event,
                                               const String      & touchObject,
                                               const bool        & state) {
  if (touchObject.isEmpty()) { return false; }
  bool success = false;

  const int8_t objectNr = getTouchObjectIndex(event, touchObject);

  if (objectNr > -1) {
    success = true;                                                             // Succes if matched object

    if (state != bitRead(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_ENABLED)) {
      bitWrite(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_ENABLED, state); // Store in settings, no save

      // Event when enabling/disabling
      if (bitRead(Touch_Settings.flags, TOUCH_FLAGS_SEND_OBJECTNAME) &&
          bitRead(Touch_Settings.flags, TOUCH_FLAGS_INIT_OBJECTEVENT)) {
        generateObjectEvent(event, objectNr,
                            bitRead(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_SLIDER) ?
                            TouchObjects[objectNr].TouchStates :
                            (TouchObjects[objectNr].TouchStates > 0 ? 1 : 0),
                            state ? -1 : -2); // Redraw only, no activation
      }
    }
    # ifdef TOUCH_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log;
      log.reserve(72);
      log = strformat(F("TOUCH setTouchObjectState: obj: %s/%d"),
                      touchObject.c_str(), objectNr);

      if (success) {
        log += F(", new state: ");
        log += state ? F("en") : F("dis");
        log += F("abled.");
      } else {
        log += F(" failed!");
      }
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
    # endif // ifdef TOUCH_DEBUG
  }

  return success;
}

/**
 * Get the enabled/disabled state of an object.
 */
int8_t ESPEasy_TouchHandler::getTouchObjectState(struct EventStruct *event,
                                                 const String      & touchObject) {
  if (touchObject.isEmpty()) { return false; }
  int8_t result         = -1;
  const int8_t objectNr = getTouchObjectIndex(event, touchObject);

  if (objectNr > -1) {
    result =  bitRead(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_ENABLED) ? 1 : 0;
  }
  return result;
}

/**
 * Set the on/off state of an enabled touch-button object. Will generate an event if so configured.
 */
bool ESPEasy_TouchHandler::setTouchButtonOnOff(struct EventStruct *event,
                                               const String      & touchObject,
                                               const bool        & state) {
  if (touchObject.isEmpty()) { return false; }
  bool success          = false;
  const int8_t objectNr = getTouchObjectIndex(event, touchObject, true);

  if ((objectNr > -1)
      && bitRead(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_ENABLED)
      && bitRead(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_BUTTON)) {
    success = true; // Always success if matched button

    if (state != TouchObjects[objectNr].TouchStates) {
      TouchObjects[objectNr].TouchStates = state;

      // Send event like it was pressed
      if (bitRead(Touch_Settings.flags, TOUCH_FLAGS_SEND_OBJECTNAME) &&
          bitRead(Touch_Settings.flags, TOUCH_FLAGS_INIT_OBJECTEVENT)) {
        generateObjectEvent(event, objectNr, state ? 1 : 0);
      }
    }
    # ifdef TOUCH_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log;
      log.reserve(72);
      log = strformat(F("TOUCH setTouchButtonOnOff: obj: %s/%d, new state: "),
                      touchObject.c_str(), objectNr);
      log += state ? F("on") : F("off");
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
    # endif // ifdef TOUCH_DEBUG
  }
  return success;
}

/**
 * Get the on/off state of an enabled touch-button object.
 */
int16_t ESPEasy_TouchHandler::getTouchObjectValue(struct EventStruct *event,
                                                  const String      & touchObject) {
  if (touchObject.isEmpty()) { return -1; }
  int16_t result        = -1; // invalid object
  const int8_t objectNr = getTouchObjectIndex(event, touchObject);

  if ((objectNr > -1)
      && bitRead(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_ENABLED)) {
    if (bitRead(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_BUTTON)) {
      result = TouchObjects[objectNr].TouchStates > 0 &&
               !bitRead(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_INVERTED) ? 1 : 0;
    } else {
      result = TouchObjects[objectNr].TouchStates;
    }
  }
  return result;
}

/**
 * Set the value of any enabled touch-object. Will generate an event if so configured.
 */
bool ESPEasy_TouchHandler::setTouchObjectValue(struct EventStruct *event,
                                               const String      & touchObject,
                                               const int16_t     & value) {
  if (touchObject.isEmpty()) { return false; }
  bool success          = false;
  const int8_t objectNr = getTouchObjectIndex(event, touchObject, false);
  int16_t _value        = value;

  if ((objectNr > -1)
      && bitRead(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_ENABLED)) {
    success = true; // Always success if matched object

    if (_value != TouchObjects[objectNr].TouchStates) {
      if (bitRead(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_SLIDER)) {
        int16_t lowRange  = 0;
        int16_t highRange = 100;

        if (!TouchObjects[objectNr].captionOff.isEmpty()) { // Off caption can hold range: <from>,<to>
          parseRangeToInt16(TouchObjects[objectNr].captionOff, lowRange, highRange);

          if (lowRange > highRange) {
            if (_value < highRange) {
              _value = highRange;
            } else if (_value > lowRange) {
              _value = lowRange;
            }
          } else {
            if (_value < lowRange) {
              _value = lowRange;
            } else if (_value > highRange) {
              _value = highRange;
            }
          }
        }
      }
      TouchObjects[objectNr].TouchStates = _value;

      // Send event like it was pressed
      if (bitRead(Touch_Settings.flags, TOUCH_FLAGS_SEND_OBJECTNAME) &&
          bitRead(Touch_Settings.flags, TOUCH_FLAGS_INIT_OBJECTEVENT)) {
        generateObjectEvent(event, objectNr, _value);
      }
    }
    # ifdef TOUCH_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      addLogMove(LOG_LEVEL_DEBUG,
                 strformat(F("TOUCH setTouchObjectValue: obj: %s/%d, new value: %d"),
                           touchObject.c_str(), objectNr, _value));
    }
    # endif // ifdef TOUCH_DEBUG
  }
  return success;
}

/**
 * parseRangeToInt16: get the low and high values of a range and convert to int16_t
 */
bool ESPEasy_TouchHandler::parseRangeToInt16(const String& range,
                                             int16_t     & lowRange,
                                             int16_t     & highRange) {
  float  rangeFrom     = 0.0f;
  float  rangeTo       = 0.0f;
  String tmp           = parseString(range, 1);
  const bool validFrom = validFloatFromString(tmp, rangeFrom);

  tmp = parseString(range, 2);

  if (validFrom && validFloatFromString(tmp, rangeTo) &&
      !essentiallyZero(rangeFrom) && !essentiallyZero(rangeTo)) {
    lowRange  = static_cast<int16_t>(rangeFrom);
    highRange = static_cast<int16_t>(rangeTo);
    return true;
  }
  return false;
}

/**
 * mode: -2 = clear buttons in group, -3 = clear all buttongroups, -1 = draw buttons in group, 0 = initialize buttons
 */
# if TOUCH_FEATURE_EXTENDED_TOUCH
void ESPEasy_TouchHandler::displayButtonGroup(struct EventStruct *event,
                                              const int16_t     & buttonGroup,
                                              const int8_t      & mode) {
  for (int8_t objectNr = 0; objectNr < static_cast<int8_t>(TouchObjects.size()); ++objectNr) {
    displayButton(event, objectNr, buttonGroup, mode);

    delay(0);
  }

  if (bitRead(Touch_Settings.flags, TOUCH_FLAGS_SEND_OBJECTNAME)) {
    // Send an event <taskname>#Group,<group>,<mode> with the selected group and the mode (-3..0)
    eventQueue.add(strformat(F("%s#Group=%d,%d"),
                             getTaskDeviceName(event->TaskIndex).c_str(),
                             buttonGroup,
                             mode));
  }

  delay(0);
}

/**
 * Display a single button, using mode from displayButtonGroup
 */
bool ESPEasy_TouchHandler::displayButton(struct EventStruct *event,
                                         const int8_t      & buttonNr,
                                         const int16_t     & buttonGroup,
                                         int8_t              mode) {
  if ((buttonNr < 0) || (buttonNr >= static_cast<int8_t>(TouchObjects.size()))) { return false; } // sanity check
  int8_t state        = 99;
  const int16_t group = get8BitFromUL(TouchObjects[buttonNr].flags, TOUCH_OBJECT_FLAG_GROUP);

  #  if TOUCH_FEATURE_EXTENDED_TOUCH
  Touch_action_e action = static_cast<Touch_action_e>(get4BitFromUL(TouchObjects[buttonNr].groupFlags, TOUCH_OBJECT_GROUP_ACTION));
  #  endif // if TOUCH_FEATURE_EXTENDED_TOUCH
  bool isArrow = false;

  #  if TOUCH_FEATURE_EXTENDED_TOUCH

  if ((mode > -2) &&                                 // Not on clear (-2 and -3)
      bitRead(Touch_Settings.flags, TOUCH_FLAGS_AUTO_PAGE_ARROWS) &&
      ((action == Touch_action_e::DecrementGroup) || // Arrow buttons
       (action == Touch_action_e::IncrementGroup) ||
       (action == Touch_action_e::DecrementPage) ||
       (action == Touch_action_e::IncrementPage))) {
    isArrow = true;
  }
  #  endif // if TOUCH_FEATURE_EXTENDED_TOUCH

  if (!TouchObjects[buttonNr].objectName.isEmpty() &&
      ((bitRead(TouchObjects[buttonNr].flags, TOUCH_OBJECT_FLAG_ENABLED) && (group == 0)) || (group > 0) || isArrow) &&
      (bitRead(TouchObjects[buttonNr].flags, TOUCH_OBJECT_FLAG_BUTTON) ||
       bitRead(TouchObjects[buttonNr].flags, TOUCH_OBJECT_FLAG_SLIDER)) &&
      (((group == buttonGroup) || (buttonGroup < 0)) ||
       ((mode != -2) && (group == 0)) ||
       (mode == -3))) {
    // Act like a button, 1 = On, 0 = Off, inversion is handled in generateObjectEvent()
    state = TouchObjects[buttonNr].TouchStates;

    #  if TOUCH_FEATURE_EXTENDED_TOUCH

    if (isArrow) {                                    // Auto-Enable/Disable the arrow buttons
      const bool pgupInvert = bitRead(Touch_Settings.flags, TOUCH_FLAGS_PGUP_BELOW_MENU);
      state = 1;                                      // always get ON state!

      if (action == Touch_action_e::DecrementGroup) { // Left arrow
        bitWrite(TouchObjects[buttonNr].flags, TOUCH_OBJECT_FLAG_ENABLED, validButtonGroup(buttonGroup - (pgupInvert ? -1 : 1), true));
      } else
      if (action == Touch_action_e::IncrementGroup) { // Right arrow
        bitWrite(TouchObjects[buttonNr].flags, TOUCH_OBJECT_FLAG_ENABLED, validButtonGroup(buttonGroup + (pgupInvert ? -1 : 1), true));
      } else
      if (action == Touch_action_e::DecrementPage) {  // Down arrow or Up arrow
        bitWrite(TouchObjects[buttonNr].flags, TOUCH_OBJECT_FLAG_ENABLED, validButtonGroup(buttonGroup + (pgupInvert ? 10 : -10), true));
      } else
      if (action == Touch_action_e::IncrementPage) {  // Up arrow or Down arrow
        bitWrite(TouchObjects[buttonNr].flags, TOUCH_OBJECT_FLAG_ENABLED, validButtonGroup(buttonGroup + (pgupInvert ? -10 : 10), true));
      }
    }
    #  endif // if TOUCH_FEATURE_EXTENDED_TOUCH

    if (bitRead(TouchObjects[buttonNr].flags, TOUCH_OBJECT_FLAG_ENABLED)) {
      if (mode == 0) {
        mode = -1;
      }
    } else {
      state -= 2; // disabled
    }
    generateObjectEvent(event, buttonNr, state, mode, mode < 0, mode <= -2 ? -1 : 1);
  }
  #  ifdef TOUCH_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    addLog(LOG_LEVEL_DEBUG,
           strformat(F("TOUCH: button init, state: %d, group: %d, mode: %d, group: %d, en: %d, object: %d"),
                     state,
                     buttonGroup,
                     mode,
                     get8BitFromUL(TouchObjects[buttonNr].flags, TOUCH_OBJECT_FLAG_GROUP),
                     bitRead(TouchObjects[buttonNr].flags, TOUCH_OBJECT_FLAG_BUTTON),
                     buttonNr));
  }
  #  endif // ifdef TOUCH_DEBUG
  return true;
}

/**
 * Check if this is a valid button group, 2022-08-16: default changed to IGNORE group 0!
 * When ignoreZero = true will return false for group 0 if the number of groups > 1.
 * When ignoreZero = false will return true for group 0 also if the number of groups > 1.
 * NB: Group 0 is always available, even without button definitions!
 */
bool ESPEasy_TouchHandler::validButtonGroup(const int16_t& group,
                                            const bool   & ignoreZero /* = true*/) {
  return _buttonGroups.find(group) != _buttonGroups.end() &&
         (!ignoreZero || group > 0 || (group == 0 && _buttonGroups.size() == 1));
}

#  if TOUCH_FEATURE_EXTENDED_TOUCH && TOUCH_FEATURE_SWIPE

/**
 * set button group page via the Swipe event
 */
bool ESPEasy_TouchHandler::handleButtonSwipe(struct EventStruct *event,
                                             const int16_t     & swipeValue) {
  bool success               = false;
  const Swipe_action_e swipe = static_cast<Swipe_action_e>(swipeValue);
  const bool swapped         = bitRead(Touch_Settings.flags, TOUCH_FLAGS_SWAP_LEFT_RIGHT);

  if (swipe == Swipe_action_e::Up) {
    if (swapped) {
      prevButtonPage(event);
    } else {
      nextButtonPage(event);
    }
    success = true;
  } else if (swipe == Swipe_action_e::Right) {
    if (swapped) {
      prevButtonGroup(event);
    } else {
      nextButtonGroup(event);
    }
    success = true;
  } else if (swipe == Swipe_action_e::Down) {
    if (swapped) {
      nextButtonPage(event);
    } else {
      prevButtonPage(event);
    }
    success = true;
  } else if ((swipe == Swipe_action_e::Left)) {
    if (swapped) {
      nextButtonGroup(event);
    } else {
      prevButtonGroup(event);
    }
    success = true;
  }
  return success;
}

#  endif // if TOUCH_FEATURE_EXTENDED_TOUCH && TOUCH_FEATURE_SWIPE

/**
 * Set the desired button group, must be a known group, previous group will be erased and new group drawn
 */
bool ESPEasy_TouchHandler::setButtonGroup(struct EventStruct *event,
                                          const int16_t     & buttonGroup) {
  if (validButtonGroup(buttonGroup, false)) { // We want to be able to select group 0
    if (buttonGroup != _buttonGroup) {
      displayButtonGroup(event, _buttonGroup, -2);
      _buttonGroup = buttonGroup;
      displayButtonGroup(event, _buttonGroup, -1);
    }
    return true;
  }
  return false;
}

/**
 * Increment button group if that group exists, if max. group > 0 then min. group = 1
 */
bool ESPEasy_TouchHandler::nextButtonGroup(struct EventStruct *event) {
  const bool pgupInvert = bitRead(Touch_Settings.flags, TOUCH_FLAGS_PGUP_BELOW_MENU);

  if (validButtonGroup(_buttonGroup + (pgupInvert ? -1 : 1))) {
    return setButtonGroup(event, _buttonGroup + (pgupInvert ? -1 : 1));
  }
  return false;
}

/**
 * Decrement button group if that group exists, if max. group > 0 then min. group = 1
 */
bool ESPEasy_TouchHandler::prevButtonGroup(struct EventStruct *event) {
  const bool pgupInvert = bitRead(Touch_Settings.flags, TOUCH_FLAGS_PGUP_BELOW_MENU);

  if (validButtonGroup(_buttonGroup - (pgupInvert ? -1 : 1))) {
    return setButtonGroup(event, _buttonGroup - (pgupInvert ? -1 : 1));
  }
  return false;
}

/**
 * Increment button group by page (+10), if max. group > 0 then min. group = 1
 */
bool ESPEasy_TouchHandler::nextButtonPage(struct EventStruct *event) {
  const bool pgupInvert = bitRead(Touch_Settings.flags, TOUCH_FLAGS_PGUP_BELOW_MENU);

  if (validButtonGroup(_buttonGroup + (pgupInvert ? -10 : 10))) {
    return setButtonGroup(event, _buttonGroup + (pgupInvert ? -10 : 10));
  }
  return false;
}

/**
 * Decrement button group by page (+10), if max. group > 0 then min. group = 1
 */
bool ESPEasy_TouchHandler::prevButtonPage(struct EventStruct *event) {
  const bool pgupInvert = bitRead(Touch_Settings.flags, TOUCH_FLAGS_PGUP_BELOW_MENU);

  if (validButtonGroup(_buttonGroup + (pgupInvert ? 10 : -10))) {
    return setButtonGroup(event, _buttonGroup + (pgupInvert ? 10 : -10));
  }
  return false;
}

# endif // if TOUCH_FEATURE_EXTENDED_TOUCH

/**
 * Get the PLUGIN_GET_DEVICEVTYPE, based on the user-selected setting for including Z-axis
 */
uint8_t ESPEasy_TouchHandler::get_device_valuecount(struct EventStruct *event) {
  if (!_settingsLoaded) {
    loadTouchObjects(event);
    _settingsLoaded = true;
  }

  return getValueCountFromSensorType(
    bitRead(Touch_Settings.flags, TOUCH_FLAGS_SEND_Z)
    ? Sensor_VType::SENSOR_TYPE_TRIPLE
    : Sensor_VType::SENSOR_TYPE_DUAL);
}

/**
 * Load the settings onto the webpage
 */
bool ESPEasy_TouchHandler::plugin_webform_load(struct EventStruct *event) {
  if (!_settingsLoaded) {
    loadTouchObjects(event);
    _settingsLoaded = true;
  }

  addFormSubHeader(F("Touch configuration"));

  addFormCheckBox(F("Flip rotation 180&deg;"), F("rot_flip"), bitRead(Touch_Settings.flags, TOUCH_FLAGS_ROTATION_FLIPPED));
  # ifndef LIMIT_BUILD_SIZE
  addFormNote(F("Some touchscreens are mounted 180&deg; rotated on the display."));
  # endif // ifndef LIMIT_BUILD_SIZE

  uint8_t choice3 = 0u;

  bitWrite(choice3, TOUCH_FLAGS_SEND_XY,         bitRead(Touch_Settings.flags, TOUCH_FLAGS_SEND_XY));
  bitWrite(choice3, TOUCH_FLAGS_SEND_Z,          bitRead(Touch_Settings.flags, TOUCH_FLAGS_SEND_Z));
  bitWrite(choice3, TOUCH_FLAGS_SEND_OBJECTNAME, bitRead(Touch_Settings.flags, TOUCH_FLAGS_SEND_OBJECTNAME));
  {
    const __FlashStringHelper *options3[] =
    { F("None"),
      F("X and Y"),
      F("X, Y and Z"),
      # if TOUCH_FEATURE_EXTENDED_TOUCH
      F("Objectnames and Button groups"),
      F("Objectnames, Button groups, X and Y"),
      F("Objectnames, Button groups, X, Y and Z")
      # else // if TOUCH_FEATURE_EXTENDED_TOUCH
      F("Objectnames only"),
      F("Objectnames, X and Y"),
      F("Objectnames, X, Y and Z")
      # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
    };
    const int optionValues3[] = { 0, 1, 3, 4, 5, 7 }; // Already used as a bitmap!
    addFormSelector(F("Events"), F("events"), NR_ELEMENTS(optionValues3), options3, optionValues3, choice3);

    addFormCheckBox(F("Draw buttons when started"), F("initobj"), bitRead(Touch_Settings.flags, TOUCH_FLAGS_INIT_OBJECTEVENT));
    # ifndef LIMIT_BUILD_SIZE
    addFormNote(F("Needs Objectnames 'Events' to be enabled."));
    # endif // ifndef LIMIT_BUILD_SIZE
  }

  addFormCheckBox(F("Prevent duplicate events"), F("dedupe"),   bitRead(Touch_Settings.flags, TOUCH_FLAGS_DEDUPLICATE));

  # if TOUCH_FEATURE_EXTENDED_TOUCH
  addFormCheckBox(F("Ignore touch-screen"),      F("igntouch"), bitRead(Touch_Settings.flags, TOUCH_FLAGS_IGNORE_TOUCH));
  #  ifndef LIMIT_BUILD_SIZE
  addFormNote(F("To enable the use of touch-object display-functions only."));
  #  endif // ifndef LIMIT_BUILD_SIZE
  # endif // if TOUCH_FEATURE_EXTENDED_TOUCH

  # ifndef LIMIT_BUILD_SIZE

  if (!Settings.UseRules) {
    addFormNote(F("Tools / Advanced / Rules must be enabled for events to be fired."));
  }
  # endif // ifndef LIMIT_BUILD_SIZE

  addFormSubHeader(F("Calibration"));

  {
    const __FlashStringHelper *noYesOptions[2] = { F("No"), F("Yes") };
    const int noYesOptionValues[2]             = { 0, 1 };
    addFormSelector(F("Calibrate to screen resolution"),
                    F("usecalib"),
                    2,
                    noYesOptions,
                    noYesOptionValues,
                    Touch_Settings.calibrationEnabled ? 1 : 0,
                    true);
  }

  if (Touch_Settings.calibrationEnabled) {
    addRowLabel(F("Calibration"));
    html_table(EMPTY_STRING, false); // Sub-table
    html_table_header(EMPTY_STRING);
    html_table_header(F("x"));
    html_table_header(F("y"));
    html_table_header(EMPTY_STRING);
    html_table_header(F("x"));
    html_table_header(F("y"));

    html_TR_TD();
    addHtml(F("Top-left"));
    html_TD();
    addNumericBox(F("cal_tl_x"),
                  Touch_Settings.top_left.x,
                  0,
                  65535);
    html_TD();
    addNumericBox(F("cal_tl_y"),
                  Touch_Settings.top_left.y,
                  0,
                  65535);
    html_TD();
    addHtml(F("Bottom-right"));
    html_TD();
    addNumericBox(F("cal_br_x"),
                  Touch_Settings.bottom_right.x,
                  0,
                  65535);
    html_TD();
    addNumericBox(F("cal_br_y"),
                  Touch_Settings.bottom_right.y,
                  0,
                  65535);

    html_end_table();
  }

  addFormCheckBox(F("Enable logging for calibration"), F("logcalib"),
                  Touch_Settings.logEnabled);

  addFormSubHeader(F("Object settings"));

  # if TOUCH_FEATURE_EXTENDED_TOUCH

  AdaGFXHtmlColorDepthDataList(F("adagfx65kcolors"), _colorDepth);

  {
    String parsed;
    addRowLabel(F("Default On/Off button colors"));
    html_table(F("sub"), false); // Sub-table
    html_table_header(F("ON color"));
    html_table_header(F("OFF color"));
    html_table_header(F("Border color"));
    html_table_header(F("Caption color"));
    html_table_header(F("Disabled color"));
    html_table_header(F("Disabled caption color"));

    html_TR_TD(); // ON color
    parsed = AdaGFXcolorToString(Touch_Settings.colorOn, _colorDepth, true);
    addTextBox(getPluginCustomArgName(3000), parsed, TOUCH_MAX_COLOR_INPUTLENGTH, false, false,
               EMPTY_STRING, F("widenumber")
               #  if TOUCH_FEATURE_TOOLTIPS
               , F("ON color")
               #  endif // if TOUCH_FEATURE_TOOLTIPS
               , F("adagfx65kcolors")
               );
    html_TD(); // OFF color
    parsed = AdaGFXcolorToString(Touch_Settings.colorOff, _colorDepth, true);
    addTextBox(getPluginCustomArgName(3001), parsed, TOUCH_MAX_COLOR_INPUTLENGTH, false, false,
               EMPTY_STRING, F("widenumber")
               #  if TOUCH_FEATURE_TOOLTIPS
               , F("OFF color")
               #  endif // if TOUCH_FEATURE_TOOLTIPS
               , F("adagfx65kcolors")
               );
    html_TD(); // Border color
    parsed = AdaGFXcolorToString(Touch_Settings.colorBorder, _colorDepth, true);
    addTextBox(getPluginCustomArgName(3002), parsed, TOUCH_MAX_COLOR_INPUTLENGTH, false, false,
               EMPTY_STRING, F("widenumber")
               #  if TOUCH_FEATURE_TOOLTIPS
               , F("Border color")
               #  endif // if TOUCH_FEATURE_TOOLTIPS
               , F("adagfx65kcolors")
               );
    html_TD(); // Caption color
    parsed = AdaGFXcolorToString(Touch_Settings.colorCaption, _colorDepth, true);
    addTextBox(getPluginCustomArgName(3003), parsed, TOUCH_MAX_COLOR_INPUTLENGTH, false, false,
               EMPTY_STRING, F("widenumber")
               #  if TOUCH_FEATURE_TOOLTIPS
               , F("Caption color")
               #  endif // if TOUCH_FEATURE_TOOLTIPS
               , F("adagfx65kcolors")
               );
    html_TD(); // Disabled color
    parsed = AdaGFXcolorToString(Touch_Settings.colorDisabled, _colorDepth, true);
    addTextBox(getPluginCustomArgName(3004), parsed, TOUCH_MAX_COLOR_INPUTLENGTH, false, false,
               EMPTY_STRING, F("widenumber")
               #  if TOUCH_FEATURE_TOOLTIPS
               , F("Disabled color")
               #  endif // if TOUCH_FEATURE_TOOLTIPS
               , F("adagfx65kcolors")
               );
    html_TD(); // Disabled caption color
    parsed = AdaGFXcolorToString(Touch_Settings.colorDisabledCaption, _colorDepth, true);
    addTextBox(getPluginCustomArgName(3005), parsed, TOUCH_MAX_COLOR_INPUTLENGTH, false, false,
               EMPTY_STRING, F("widenumber")
               #  if TOUCH_FEATURE_TOOLTIPS
               , F("Disabled caption color")
               #  endif // if TOUCH_FEATURE_TOOLTIPS
               , F("adagfx65kcolors")
               );
    html_end_table();
  }
  {
    addFormNumericBox(F("Initial button group"), F("initgrp"),
                      get8BitFromUL(Touch_Settings.flags, TOUCH_FLAGS_INITIAL_GROUP), 0, TOUCH_MAX_BUTTON_GROUPS
                      #  if TOUCH_FEATURE_TOOLTIPS
                      , F("Initial group")
                      #  endif // if TOUCH_FEATURE_TOOLTIPS
                      );
    addFormCheckBox(F("Draw buttons via Rules"), F("via_rules"),
                    bitRead(Touch_Settings.flags, TOUCH_FLAGS_DRAWBTN_VIA_RULES));
    addFormCheckBox(F("Enable/Disable page buttons"), F("pagebtns"),
                    bitRead(Touch_Settings.flags, TOUCH_FLAGS_AUTO_PAGE_ARROWS));
    addFormCheckBox(F("Navigation Left/Right/Up/ Down menu reversed"), F("pageblw"),
                    bitRead(Touch_Settings.flags, TOUCH_FLAGS_PGUP_BELOW_MENU));
    addFormCheckBox(F("Swipe Left/Right/Up/Down menu reversed"), F("swipeswap"),
                    bitRead(Touch_Settings.flags, TOUCH_FLAGS_SWAP_LEFT_RIGHT));
  }
  # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
  {
    addFormNumericBox(F("Debounce delay for On/Off buttons"), F("debounce"),
                      Touch_Settings.debounceMs, 0, 255);
    addUnit(F("0..255 msec."));

    # if TOUCH_FEATURE_EXTENDED_TOUCH && TOUCH_FEATURE_SWIPE
    addFormNumericBox(F("Minimal swipe movement"), F("swipemin"),
                      Touch_Settings.swipeMinimal, 1, 25);
    addUnit(F("1..25px"));

    addFormNumericBox(F("Maximum swipe margin"), F("swipemax"),
                      Touch_Settings.swipeMargin, 5, 250);
    addUnit(F("5..250px"));
    # endif // if TOUCH_FEATURE_EXTENDED_TOUCH && TOUCH_FEATURE_SWIPE
  }
  {
    addFormSubHeader(F("Touch objects"));

    {
      addRowLabel(F("Object"));

      html_table(F("multirow tworow"), false); // Sub-table with alternating highlight per 2 rows
      html_table_header(F("&nbsp;#&nbsp;"));
      html_table_header(F("On"));
      html_table_header(F("Objectname"));
      html_table_header(F("Top-left x"));
      html_table_header(F("Top-left y"));
      # if TOUCH_FEATURE_EXTENDED_TOUCH
      html_table_header(F("Button"));
      html_table_header(F("Layout"));
      html_table_header(F("ON color"));
      html_table_header(F("ON caption"));
      html_table_header(F("Border color"));
      html_table_header(F("Disab. cap. clr"));
      html_table_header(F("Touch action"));
      # else // if TOUCH_FEATURE_EXTENDED_TOUCH
      html_table_header(F("On/Off button"));
      # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
      html_TR(); // New row
      html_table_header(EMPTY_STRING);
      html_table_header(EMPTY_STRING);
      # if TOUCH_FEATURE_EXTENDED_TOUCH
      html_table_header(F("Button-group"));
      # else // if TOUCH_FEATURE_EXTENDED_TOUCH
      html_table_header(EMPTY_STRING);
      # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
      html_table_header(F("Width"));
      html_table_header(F("Height"));
      html_table_header(F("Inverted"));
      # if TOUCH_FEATURE_EXTENDED_TOUCH
      html_table_header(F("Font scale"));
      html_table_header(F("OFF color"));
      html_table_header(F("OFF caption"));
      html_table_header(F("Caption color"));
      html_table_header(F("Disabled clr"));
      html_table_header(F("Action group"));
      # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
    }
    # if TOUCH_FEATURE_EXTENDED_TOUCH
    const __FlashStringHelper *buttonTypeOptions[] = {
      toString(Button_type_e::None),
      toString(Button_type_e::Square),
      toString(Button_type_e::Rounded),
      toString(Button_type_e::Circle),
      toString(Button_type_e::ArrowLeft),
      toString(Button_type_e::ArrowUp),
      toString(Button_type_e::ArrowRight),
      toString(Button_type_e::ArrowDown),
    };

    const int buttonTypeValues[] = {
      static_cast<int>(Button_type_e::None),
      static_cast<int>(Button_type_e::Square),
      static_cast<int>(Button_type_e::Rounded),
      static_cast<int>(Button_type_e::Circle),
      static_cast<int>(Button_type_e::ArrowLeft),
      static_cast<int>(Button_type_e::ArrowUp),
      static_cast<int>(Button_type_e::ArrowRight),
      static_cast<int>(Button_type_e::ArrowDown),
    };

    const __FlashStringHelper *buttonLayoutOptions[] = {
      toString(Button_layout_e::CenterAligned),
      toString(Button_layout_e::LeftAligned),
      toString(Button_layout_e::TopAligned),
      toString(Button_layout_e::RightAligned),
      toString(Button_layout_e::BottomAligned),
      toString(Button_layout_e::LeftTopAligned),
      toString(Button_layout_e::RightTopAligned),
      toString(Button_layout_e::LeftBottomAligned),
      toString(Button_layout_e::RightBottomAligned),
      toString(Button_layout_e::NoCaption),
      #  if ADAGFX_ENABLE_BMP_DISPLAY
      toString(Button_layout_e::Bitmap),
      #  endif // if ADAGFX_ENABLE_BMP_DISPLAY
      #  if ADAGFX_ENABLE_BUTTON_SLIDER
      toString(Button_layout_e::Slider),
      #  endif // if ADAGFX_ENABLE_BUTTON_SLIDER
    };

    const int buttonLayoutValues[] = {
      static_cast<int>(Button_layout_e::CenterAligned),
      static_cast<int>(Button_layout_e::LeftAligned),
      static_cast<int>(Button_layout_e::TopAligned),
      static_cast<int>(Button_layout_e::RightAligned),
      static_cast<int>(Button_layout_e::BottomAligned),
      static_cast<int>(Button_layout_e::LeftTopAligned),
      static_cast<int>(Button_layout_e::RightTopAligned),
      static_cast<int>(Button_layout_e::LeftBottomAligned),
      static_cast<int>(Button_layout_e::RightBottomAligned),
      static_cast<int>(Button_layout_e::NoCaption),
      #  if ADAGFX_ENABLE_BMP_DISPLAY
      static_cast<int>(Button_layout_e::Bitmap),
      #  endif // if ADAGFX_ENABLE_BMP_DISPLAY
      #  if ADAGFX_ENABLE_BUTTON_SLIDER
      static_cast<int>(Button_layout_e::Slider),
      #  endif // if ADAGFX_ENABLE_BUTTON_SLIDER
    };

    const __FlashStringHelper *touchActionOptions[] = {
      toString(Touch_action_e::Default),
      toString(Touch_action_e::ActivateGroup),
      toString(Touch_action_e::IncrementGroup),
      toString(Touch_action_e::DecrementGroup),
      toString(Touch_action_e::IncrementPage),
      toString(Touch_action_e::DecrementPage),
    };

    const int touchActionValues[] = {
      static_cast<int>(Touch_action_e::Default),
      static_cast<int>(Touch_action_e::ActivateGroup),
      static_cast<int>(Touch_action_e::IncrementGroup),
      static_cast<int>(Touch_action_e::DecrementGroup),
      static_cast<int>(Touch_action_e::IncrementPage),
      static_cast<int>(Touch_action_e::DecrementPage),
    };

    # endif // if TOUCH_FEATURE_EXTENDED_TOUCH

    const uint8_t maxIdx = std::min(static_cast<int>(TouchObjects.size() + TOUCH_EXTRA_OBJECT_COUNT), TOUCH_MAX_OBJECT_COUNT);
    String parsed;
    TouchObjects.resize(maxIdx, tTouchObjects());

    for (int8_t objectNr = 0; objectNr < maxIdx; ++objectNr) {
      html_TR_TD();
      addHtml(F("&nbsp;"));
      addHtmlInt(objectNr + 1); // Arrayindex to objectindex

      html_TD();

      // Enable new entries
      bool enabled = bitRead(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_ENABLED) || TouchObjects[objectNr].objectName.isEmpty();
      addCheckBox(getPluginCustomArgName(objectNr + 0),
                  enabled, false
                  # if TOUCH_FEATURE_TOOLTIPS
                  , F("Enabled")
                  # endif // if TOUCH_FEATURE_TOOLTIPS
                  );
      html_TD(); // Name
      addTextBox(getPluginCustomArgName(objectNr + 100),
                 TouchObjects[objectNr].objectName,
                 TOUCH_MaxObjectNameLength,
                 false, false, EMPTY_STRING, F("wide"));
      html_TD(); // top-x
      addNumericBox(getPluginCustomArgName(objectNr + 200),
                    TouchObjects[objectNr].top_left.x, 0, 65535
                    # if TOUCH_FEATURE_TOOLTIPS
                    , F("widenumber"), F("Top-left x")
                    # endif // if TOUCH_FEATURE_TOOLTIPS
                    );
      html_TD(); // top-y
      addNumericBox(getPluginCustomArgName(objectNr + 300),
                    TouchObjects[objectNr].top_left.y, 0, 65535
                    # if TOUCH_FEATURE_TOOLTIPS
                    , F("widenumber"), F("Top-left y")
                    # endif // if TOUCH_FEATURE_TOOLTIPS
                    );
      html_TD(); // (on/off) button (type)
      # if TOUCH_FEATURE_EXTENDED_TOUCH
      addSelector(getPluginCustomArgName(objectNr + 800),
                  sizeof(buttonTypeValues) / sizeof(int),
                  buttonTypeOptions,
                  buttonTypeValues,
                  nullptr,
                  get4BitFromUL(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_BUTTONTYPE), false, true, F("widenumber")
                  #  if TOUCH_FEATURE_TOOLTIPS
                  , F("Button")
                  #  endif // if TOUCH_FEATURE_TOOLTIPS
                  );
      html_TD(); // button alignment
      addSelector(getPluginCustomArgName(objectNr + 900),
                  sizeof(buttonLayoutValues) / sizeof(int),
                  buttonLayoutOptions,
                  buttonLayoutValues,
                  nullptr,
                  get4BitFromUL(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_BUTTONALIGN) << 4, false, true, F("widenumber")
                  #  if TOUCH_FEATURE_TOOLTIPS
                  , F("Layout")
                  #  endif // if TOUCH_FEATURE_TOOLTIPS
                  );
      # else // if TOUCH_FEATURE_EXTENDED_TOUCH
      addCheckBox(getPluginCustomArgName(objectNr + 600),
                  bitRead(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_BUTTON), false
                  #  if TOUCH_FEATURE_TOOLTIPS
                  , F("On/Off button")
                  #  endif // if TOUCH_FEATURE_TOOLTIPS
                  );
      # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
      # if TOUCH_FEATURE_EXTENDED_TOUCH
      html_TD(); // ON color
      parsed = AdaGFXcolorToString(TouchObjects[objectNr].colorOn, _colorDepth, true);
      addTextBox(getPluginCustomArgName(objectNr + 1000), parsed, TOUCH_MAX_COLOR_INPUTLENGTH, false, false,
                 EMPTY_STRING, F("widenumber")
                 #  if TOUCH_FEATURE_TOOLTIPS
                 , F("ON color")
                 #  endif // if TOUCH_FEATURE_TOOLTIPS
                 , F("adagfx65kcolors")
                 );
      html_TD(); // ON Caption
      parsed = TouchObjects[objectNr].captionOn;
      parsed.replace('_', ' ');
      addTextBox(getPluginCustomArgName(objectNr + 1300),
                 parsed,
                 TOUCH_MaxCaptionNameLength,
                 false,
                 false,
                 EMPTY_STRING,
                 F("xwide")
                 #  if TOUCH_FEATURE_TOOLTIPS
                 , F("ON caption")
                 #  endif // if TOUCH_FEATURE_TOOLTIPS
                 );
      html_TD(); // Border color
      parsed = AdaGFXcolorToString(TouchObjects[objectNr].colorBorder, _colorDepth, true);
      addTextBox(getPluginCustomArgName(objectNr + 1700), parsed, TOUCH_MAX_COLOR_INPUTLENGTH, false, false,
                 EMPTY_STRING, F("widenumber")
                 #  if TOUCH_FEATURE_TOOLTIPS
                 , F("Border color")
                 #  endif // if TOUCH_FEATURE_TOOLTIPS
                 , F("adagfx65kcolors")
                 );
      html_TD(); // Disabled caption color
      parsed = AdaGFXcolorToString(TouchObjects[objectNr].colorDisabledCaption, _colorDepth, true);
      addTextBox(getPluginCustomArgName(objectNr + 1900), parsed, TOUCH_MAX_COLOR_INPUTLENGTH, false, false,
                 EMPTY_STRING, F("widenumber")
                 #  if TOUCH_FEATURE_TOOLTIPS
                 , F("Disabled caption color")
                 #  endif // if TOUCH_FEATURE_TOOLTIPS
                 , F("adagfx65kcolors")
                 );
      html_TD(); // button action
      addSelector(getPluginCustomArgName(objectNr + 2000),
                  sizeof(touchActionValues) / sizeof(int),
                  touchActionOptions,
                  touchActionValues,
                  nullptr,
                  get4BitFromUL(TouchObjects[objectNr].groupFlags, TOUCH_OBJECT_GROUP_ACTION),
                  false,
                  true,
                  F("widenumber")
                  #  if TOUCH_FEATURE_TOOLTIPS
                  , F("Touch action")
                  #  endif // if TOUCH_FEATURE_TOOLTIPS
                  );
      # endif // if TOUCH_FEATURE_EXTENDED_TOUCH

      html_TR_TD(); // Start new row

      html_TD(2);   // Start with some blank columns
      # if TOUCH_FEATURE_EXTENDED_TOUCH
      {
        addNumericBox(getPluginCustomArgName(objectNr + 1600),
                      get8BitFromUL(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_GROUP), 0, TOUCH_MAX_BUTTON_GROUPS
                      #  if TOUCH_FEATURE_TOOLTIPS
                      , F("widenumber"), strformat(F("Button-group [0..%d]"), TOUCH_MAX_BUTTON_GROUPS)
                      #  endif // if TOUCH_FEATURE_TOOLTIPS
                      );
      }
      # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
      html_TD(); // Width
      addNumericBox(getPluginCustomArgName(objectNr + 400),
                    TouchObjects[objectNr].width_height.x, 0, 65535
                    # if TOUCH_FEATURE_TOOLTIPS
                    , F("widenumber"), F("Width")
                    # endif // if TOUCH_FEATURE_TOOLTIPS
                    );
      html_TD(); // Height
      addNumericBox(getPluginCustomArgName(objectNr + 500),
                    TouchObjects[objectNr].width_height.y, 0, 65535
                    # if TOUCH_FEATURE_TOOLTIPS
                    , F("widenumber"), F("Height")
                    # endif // if TOUCH_FEATURE_TOOLTIPS
                    );
      html_TD(); // inverted
      addCheckBox(getPluginCustomArgName(objectNr + 700),
                  bitRead(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_INVERTED), false
                  # if TOUCH_FEATURE_TOOLTIPS
                  , F("Inverted")
                  # endif // if TOUCH_FEATURE_TOOLTIPS
                  );
      # if TOUCH_FEATURE_EXTENDED_TOUCH
      html_TD(); // font scale
      addNumericBox(getPluginCustomArgName(objectNr + 1200),
                    get4BitFromUL(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_FONTSCALE), 0, 10
                    #  if TOUCH_FEATURE_TOOLTIPS
                    , F("widenumber"), F("Font scaling [1x..10x]")
                    #  endif // if TOUCH_FEATURE_TOOLTIPS
                    );
      html_TD(); // OFF color
      parsed = AdaGFXcolorToString(TouchObjects[objectNr].colorOff, _colorDepth, true);
      addTextBox(getPluginCustomArgName(objectNr + 1100), parsed, TOUCH_MAX_COLOR_INPUTLENGTH, false, false,
                 EMPTY_STRING, F("widenumber")
                 #  if TOUCH_FEATURE_TOOLTIPS
                 , F("OFF color")
                 #  endif // if TOUCH_FEATURE_TOOLTIPS
                 , F("adagfx65kcolors")
                 );
      html_TD(); // OFF Caption
      parsed = TouchObjects[objectNr].captionOff;
      parsed.replace('_', ' ');
      addTextBox(getPluginCustomArgName(objectNr + 1400),
                 parsed,
                 TOUCH_MaxCaptionNameLength,
                 false,
                 false,
                 EMPTY_STRING,
                 F("xwide")
                 #  if TOUCH_FEATURE_TOOLTIPS
                 , F("OFF caption")
                 #  endif // if TOUCH_FEATURE_TOOLTIPS
                 );
      html_TD(); // Caption color
      parsed = AdaGFXcolorToString(TouchObjects[objectNr].colorCaption, _colorDepth, true);
      addTextBox(getPluginCustomArgName(objectNr + 1500), parsed, TOUCH_MAX_COLOR_INPUTLENGTH, false, false,
                 EMPTY_STRING, F("widenumber")
                 #  if TOUCH_FEATURE_TOOLTIPS
                 , F("Caption color")
                 #  endif // if TOUCH_FEATURE_TOOLTIPS
                 , F("adagfx65kcolors")
                 );
      html_TD(); // Disabled color
      parsed = AdaGFXcolorToString(TouchObjects[objectNr].colorDisabled, _colorDepth, true);
      addTextBox(getPluginCustomArgName(objectNr + 1800), parsed, TOUCH_MAX_COLOR_INPUTLENGTH, false, false,
                 EMPTY_STRING, F("widenumber")
                 #  if TOUCH_FEATURE_TOOLTIPS
                 , F("Disabled color")
                 #  endif // if TOUCH_FEATURE_TOOLTIPS
                 , F("adagfx65kcolors")
                 );
      html_TD(); // Action Group
      addNumericBox(getPluginCustomArgName(objectNr + 2100),
                    get8BitFromUL(TouchObjects[objectNr].groupFlags, TOUCH_OBJECT_GROUP_ACTIONGROUP), 0, TOUCH_MAX_BUTTON_GROUPS
                    #  if TOUCH_FEATURE_TOOLTIPS
                    , F("widenumber"), F("Action group")
                    #  endif // if TOUCH_FEATURE_TOOLTIPS
                    );
      # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
    }
    html_end_table();
  }
  return false;
}

/**
 * Save the settings from the web page to flash
 */
bool ESPEasy_TouchHandler::plugin_webform_save(struct EventStruct *event) {
  # ifdef TOUCH_DEBUG
  addLog(LOG_LEVEL_INFO, F("TOUCH DEBUG webform_save start."));
  # endif // ifdef TOUCH_DEBUG
  String config;

  uint16_t saveSize = 0;

  # if TOUCH_FEATURE_EXTENDED_TOUCH
  String colorInput;
  # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
  config.reserve(80);

  uint32_t  lSettings   = 0u;
  const int eventsValue = getFormItemInt(F("events"));

  bitWrite(lSettings, TOUCH_FLAGS_SEND_XY,          bitRead(eventsValue, TOUCH_FLAGS_SEND_XY));
  bitWrite(lSettings, TOUCH_FLAGS_SEND_Z,           bitRead(eventsValue, TOUCH_FLAGS_SEND_Z));
  bitWrite(lSettings, TOUCH_FLAGS_SEND_OBJECTNAME,  bitRead(eventsValue, TOUCH_FLAGS_SEND_OBJECTNAME));
  bitWrite(lSettings, TOUCH_FLAGS_ROTATION_FLIPPED, isFormItemChecked(F("rot_flip")));
  bitWrite(lSettings, TOUCH_FLAGS_DEDUPLICATE,      isFormItemChecked(F("dedupe")));
  bitWrite(lSettings, TOUCH_FLAGS_INIT_OBJECTEVENT, isFormItemChecked(F("initobj")));
  # if TOUCH_FEATURE_EXTENDED_TOUCH
  set8BitToUL(lSettings, TOUCH_FLAGS_INITIAL_GROUP, getFormItemInt(F("initgrp"))); // Button group
  bitWrite(lSettings, TOUCH_FLAGS_DRAWBTN_VIA_RULES, isFormItemChecked(F("via_rules")));
  bitWrite(lSettings, TOUCH_FLAGS_AUTO_PAGE_ARROWS,  isFormItemChecked(F("pagebtns")));
  bitWrite(lSettings, TOUCH_FLAGS_PGUP_BELOW_MENU,   isFormItemChecked(F("pageblw")));
  bitWrite(lSettings, TOUCH_FLAGS_SWAP_LEFT_RIGHT,   isFormItemChecked(F("swipeswap")));
  bitWrite(lSettings, TOUCH_FLAGS_IGNORE_TOUCH,      isFormItemChecked(F("igntouch")));
  # endif // if TOUCH_FEATURE_EXTENDED_TOUCH

  config += getFormItemInt(F("usecalib")); // First value should NEVER be empty, or parseString() wil get confused
  config += TOUCH_SETTINGS_SEPARATOR;
  config += toStringNoZero(isFormItemChecked(F("logcalib")) ? 1 : 0);
  config += TOUCH_SETTINGS_SEPARATOR;
  config += toStringNoZero(getFormItemInt(F("cal_tl_x")));
  config += TOUCH_SETTINGS_SEPARATOR;
  config += toStringNoZero(getFormItemInt(F("cal_tl_y")));
  config += TOUCH_SETTINGS_SEPARATOR;
  config += toStringNoZero(getFormItemInt(F("cal_br_x")));
  config += TOUCH_SETTINGS_SEPARATOR;
  config += toStringNoZero(getFormItemInt(F("cal_br_y")));
  config += TOUCH_SETTINGS_SEPARATOR;
  config += toStringNoZero(getFormItemInt(F("debounce")));
  config += TOUCH_SETTINGS_SEPARATOR;
  config += ull2String(lSettings);
  # if TOUCH_FEATURE_EXTENDED_TOUCH
  config    += TOUCH_SETTINGS_SEPARATOR;
  colorInput = webArg(getPluginCustomArgName(3000)); // Default Color ON
  config    += toStringNoZero(AdaGFXparseColor(colorInput, _colorDepth));
  config    += TOUCH_SETTINGS_SEPARATOR;
  colorInput = webArg(getPluginCustomArgName(3001)); // Default Color OFF
  config    += toStringNoZero(AdaGFXparseColor(colorInput, _colorDepth, false));
  config    += TOUCH_SETTINGS_SEPARATOR;
  colorInput = webArg(getPluginCustomArgName(3002)); // Default Color Border
  config    += toStringNoZero(AdaGFXparseColor(colorInput, _colorDepth, false));
  config    += TOUCH_SETTINGS_SEPARATOR;
  colorInput = webArg(getPluginCustomArgName(3003)); // Default Color caption
  config    += toStringNoZero(AdaGFXparseColor(colorInput, _colorDepth, false));
  config    += TOUCH_SETTINGS_SEPARATOR;
  colorInput = webArg(getPluginCustomArgName(3004)); // Default Disabled Color
  config    += toStringNoZero(AdaGFXparseColor(colorInput, _colorDepth));
  config    += TOUCH_SETTINGS_SEPARATOR;
  colorInput = webArg(getPluginCustomArgName(3005)); // Default Disabled Caption Color
  config    += toStringNoZero(AdaGFXparseColor(colorInput, _colorDepth, false));
  #  if TOUCH_FEATURE_SWIPE
  config += TOUCH_SETTINGS_SEPARATOR;
  config += toStringNoZero(getFormItemInt(F("swipemin")));
  config += TOUCH_SETTINGS_SEPARATOR;
  config += toStringNoZero(getFormItemInt(F("swipemax")));
  #  endif // if TOUCH_FEATURE_SWIPE
  # endif // if TOUCH_FEATURE_EXTENDED_TOUCH

  settingsArray[TOUCH_CALIBRATION_START] = config;
  saveSize                              += config.length() + 1;

  # ifdef TOUCH_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    config.replace(TOUCH_SETTINGS_SEPARATOR, ',');
    addLogMove(LOG_LEVEL_INFO, concat(F("Save settings: "), config));
  }
  # endif // ifdef TOUCH_DEBUG

  String error;

  for (int8_t objectNr = 0; objectNr < TOUCH_MAX_OBJECT_COUNT; ++objectNr) {
    config.clear();
    config += webArg(getPluginCustomArgName(objectNr + 100)); // Name
    config.trim();                                            // Remove leading/trailing whitespace from name

    if (!config.isEmpty()) {                                  // Empty name => skip entry
      bool numStart = (config[0] >= '0' && config[0] <= '9'); // Numeric start?

      if (!ExtraTaskSettings.checkInvalidCharInNames(config.c_str()) ||
          numStart) {                                         // Check for invalid characters in objectname
        error += strformat(F("Invalid character in objectname #%d. "), objectNr + 1);
        error += numStart ? F("Should not start with a digit.\n") : F("Do not use ',-+/*=^%!#[]{}()' or space.\n");
      }
      config += TOUCH_SETTINGS_SEPARATOR;
      uint32_t flags = 0u;
      bitWrite(flags, TOUCH_OBJECT_FLAG_ENABLED,  isFormItemChecked(getPluginCustomArgName(objectNr + 0)));   // Enabled
      bitWrite(flags, TOUCH_OBJECT_FLAG_INVERTED, isFormItemChecked(getPluginCustomArgName(objectNr + 700))); // Inverted
      # if TOUCH_FEATURE_EXTENDED_TOUCH
      uint32_t groupFlags        = 0u;
      const uint8_t buttonType   = getFormItemIntCustomArgName(objectNr + 800);
      const uint8_t buttonLayout = getFormItemIntCustomArgName(objectNr + 900) >> 4;
      set4BitToUL(flags, TOUCH_OBJECT_FLAG_BUTTONTYPE,  buttonType);   // Buttontype
      set4BitToUL(flags, TOUCH_OBJECT_FLAG_BUTTONALIGN, buttonLayout); // Button layout
      #  if ADAGFX_ENABLE_BUTTON_SLIDER
      const bool isSlider = (static_cast<Button_layout_e>(buttonLayout << 4) == Button_layout_e::Slider);
      bitWrite(flags, TOUCH_OBJECT_FLAG_SLIDER, isSlider);             // Slider
      #  else // if ADAGFX_ENABLE_BUTTON_SLIDER
      const bool isSlider = false;
      #  endif // if ADAGFX_ENABLE_BUTTON_SLIDER
      const bool isButton = (static_cast<Button_type_e>(buttonType) != Button_type_e::None) && !isSlider;
      bitWrite(flags, TOUCH_OBJECT_FLAG_BUTTON, isButton);                                                   // On/Off button
      set4BitToUL(groupFlags, TOUCH_OBJECT_GROUP_ACTION, getFormItemIntCustomArgName(objectNr + 2000));      // ButtonAction
      set8BitToUL(groupFlags, TOUCH_OBJECT_GROUP_ACTIONGROUP, getFormItemIntCustomArgName(objectNr + 2100)); // ActionGroup
      set4BitToUL(flags, TOUCH_OBJECT_FLAG_FONTSCALE, getFormItemIntCustomArgName(objectNr + 1200));         // Font scaling
      set8BitToUL(flags, TOUCH_OBJECT_FLAG_GROUP, getFormItemIntCustomArgName(objectNr + 1600));             // Button group
      # else // if TOUCH_FEATURE_EXTENDED_TOUCH
      bitWrite(flags, TOUCH_OBJECT_FLAG_BUTTON, isFormItemChecked(getPluginCustomArgName(objectNr + 600)));  // On/Off button
      # endif // if TOUCH_FEATURE_EXTENDED_TOUCH

      // REMARK: Converting the code below to strformat() increases the build by 200 bytes!
      config += ull2String(flags);                                           // Flags
      config += TOUCH_SETTINGS_SEPARATOR;
      config += toStringNoZero(getFormItemIntCustomArgName(objectNr + 200)); // Top x
      config += TOUCH_SETTINGS_SEPARATOR;
      config += toStringNoZero(getFormItemIntCustomArgName(objectNr + 300)); // Top y
      config += TOUCH_SETTINGS_SEPARATOR;
      config += toStringNoZero(getFormItemIntCustomArgName(objectNr + 400)); // Bottom x
      config += TOUCH_SETTINGS_SEPARATOR;
      config += toStringNoZero(getFormItemIntCustomArgName(objectNr + 500)); // Bottom y

      # if TOUCH_FEATURE_EXTENDED_TOUCH
      config    += TOUCH_SETTINGS_SEPARATOR;
      colorInput = webArg(getPluginCustomArgName(objectNr + 1000)); // Color ON
      config    += toStringNoZero(AdaGFXparseColor(colorInput, _colorDepth, true));
      config    += TOUCH_SETTINGS_SEPARATOR;
      colorInput = webArg(getPluginCustomArgName(objectNr + 1100)); // Color OFF
      config    += toStringNoZero(AdaGFXparseColor(colorInput, _colorDepth, true));
      config    += TOUCH_SETTINGS_SEPARATOR;
      colorInput = webArg(getPluginCustomArgName(objectNr + 1500)); // Color caption
      config    += toStringNoZero(AdaGFXparseColor(colorInput, _colorDepth, true));
      config    += TOUCH_SETTINGS_SEPARATOR;                        // Caption ON
      colorInput = webArg(getPluginCustomArgName(objectNr + 1300));
      colorInput.replace(' ', '_');                                 // Replace spaces by '_', often cheaper than 2 quotes...
      config    += wrapWithQuotesIfContainsParameterSeparatorChar(colorInput);
      config    += TOUCH_SETTINGS_SEPARATOR;                        // Caption OFF
      colorInput = webArg(getPluginCustomArgName(objectNr + 1400));
      colorInput.replace(' ', '_');                                 // Replace spaces by '_', often cheaper than 2 quotes...
      config    += wrapWithQuotesIfContainsParameterSeparatorChar(colorInput);
      config    += TOUCH_SETTINGS_SEPARATOR;
      colorInput = webArg(getPluginCustomArgName(objectNr + 1700)); // Color Border
      config    += toStringNoZero(AdaGFXparseColor(colorInput, _colorDepth, true));
      config    += TOUCH_SETTINGS_SEPARATOR;
      colorInput = webArg(getPluginCustomArgName(objectNr + 1800)); // Disabled Color
      config    += toStringNoZero(AdaGFXparseColor(colorInput, _colorDepth, true));
      config    += TOUCH_SETTINGS_SEPARATOR;
      colorInput = webArg(getPluginCustomArgName(objectNr + 1900)); // Disabled Caption Color
      config    += toStringNoZero(AdaGFXparseColor(colorInput, _colorDepth, true));
      config    += TOUCH_SETTINGS_SEPARATOR;
      config    += ull2String(groupFlags);                          // Group Flags
      # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
      config.trim();
    }

    String endZero; // Trim off <sep> and <sep>0 from the end
    endZero += TOUCH_SETTINGS_SEPARATOR;
    endZero += '0';
    const uint8_t endZeroLen = endZero.length();

    while (!config.isEmpty() && (config.endsWith(endZero) || config[config.length() - 1] == TOUCH_SETTINGS_SEPARATOR)) {
      if (config[config.length() - 1] == TOUCH_SETTINGS_SEPARATOR) {
        config.remove(config.length() - 1);
      } else {
        config.remove(config.length() - endZeroLen, endZeroLen);
      }
    }

    settingsArray[objectNr + TOUCH_OBJECT_INDEX_START] = config;
    saveSize                                          += config.length() + 1;

    # ifdef TOUCH_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO) &&
        !config.isEmpty()) {
      config.replace(TOUCH_SETTINGS_SEPARATOR, ',');
      addLogMove(LOG_LEVEL_INFO, strformat(F("Save touch object #%d settings: %s"), objectNr + 1, config.c_str()));
    }
    # endif // ifdef TOUCH_DEBUG
  }

  if (!error.isEmpty()) {
    addLog(LOG_LEVEL_ERROR, error);
    addHtmlError(error);
  }

  error = SaveCustomTaskSettings(event->TaskIndex, settingsArray, TOUCH_ARRAY_SIZE, 0);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLogMove(LOG_LEVEL_INFO, concat(F("TOUCH: Save settings, size: "), saveSize));
  }

  if (!error.isEmpty()) {
    addLog(LOG_LEVEL_ERROR, error);
    addHtmlError(error);
    return false;
  }
  return true;
}

/**
 * Every 20 milliseconds we check if the screen is touched,
 * handles button switching, swiping and slider-sliding
 */
bool ESPEasy_TouchHandler::plugin_fifty_per_second(struct EventStruct *event,
                                                   const int16_t     & x,
                                                   const int16_t     & y,
                                                   const int16_t     & ox,
                                                   const int16_t     & oy,
                                                   const int16_t     & rx,
                                                   const int16_t     & ry,
                                                   const int16_t     & z) {
  bool success = false;

  // Avoid event-storms by deduplicating coordinates, ignore z value when no z-event is generated
  if (!_deduplicate ||
      (_deduplicate && ((TOUCH_GET_VALUE_X != x) || (TOUCH_GET_VALUE_Y != y) ||
                        (bitRead(Touch_Settings.flags, TOUCH_FLAGS_SEND_Z) && (TOUCH_GET_VALUE_Z != z))))) {
    success = true;
    TOUCH_SET_VALUE_X(x);
    TOUCH_SET_VALUE_Y(y);
    TOUCH_SET_VALUE_Z(z);
  }

  if (success &&
      Touch_Settings.logEnabled &&

      // This log is REQUIRED for calibration and setting up objects, so do not make this optional!
      loglevelActiveFor(LOG_LEVEL_INFO)) {
    // Space before the logged values for readability. Always log the z value even if not used.
    addLogMove(LOG_LEVEL_INFO, strformat(F("Touch calibration rx= %d, ry= %d; z= %d, x= %d, y= %d; ox= %d, oy= %d"),
                                         rx, ry, z, x, y, ox, oy));
  }

  // No events to handle if rules not enabled
  if (Settings.UseRules) {
    if (success && bitRead(Touch_Settings.flags, TOUCH_FLAGS_SEND_XY)) { // Send events for each touch
      sendData(event);                                                   // Send X/Y(/Z) event
    }

    if (bitRead(Touch_Settings.flags, TOUCH_FLAGS_SEND_OBJECTNAME)) {    // Send events for objectname if within reach, and swipes
      String selectedObjectName;
      int8_t selectedObjectIndex = -1;

      if (isValidAndTouchedTouchObject(x, y, selectedObjectName, selectedObjectIndex)) {
        # if TOUCH_FEATURE_EXTENDED_TOUCH && TOUCH_FEATURE_SWIPE
        int16_t delta_x = x - _last_point.x;
        int16_t delta_y = y - _last_point.y;

        Swipe_action_e swipe = Swipe_action_e::None;

        if ((std::abs(delta_x) >= Touch_Settings.swipeMargin) || (std::abs(delta_x) <= Touch_Settings.swipeMinimal)) {
          delta_x = 0; // Ignore
        }

        if ((std::abs(delta_y) >= Touch_Settings.swipeMargin) || (std::abs(delta_y) <= Touch_Settings.swipeMinimal)) {
          delta_y = 0; // Ignore
        }

        if ((delta_x != 0) || (delta_y != 0)) {
          _lastObjectIndex = -2;

          // Swipe, determine direction (from 12 o'clock, clock-wise)
          if ((delta_x == 0) && (delta_y < 0)) {        // Up
            swipe = Swipe_action_e::Up;
          } else if ((delta_x > 0) && (delta_y < 0)) {  // Up-Right
            swipe = Swipe_action_e::UpRight;
          } else if ((delta_x > 0) && (delta_y == 0)) { // Right
            swipe = Swipe_action_e::Right;
          } else if ((delta_x > 0) && (delta_y > 0)) {  // Right-Down
            swipe = Swipe_action_e::RightDown;
          } else if ((delta_x == 0) && (delta_y > 0)) { // Down
            swipe = Swipe_action_e::Down;
          } else if ((delta_x < 0) && (delta_y > 0)) {  // Down-Left
            swipe = Swipe_action_e::DownLeft;
          } else if ((delta_x < 0) && (delta_y == 0)) { // Left
            swipe = Swipe_action_e::Left;
          } else if ((delta_x < 0) && (delta_y < 0)) {  // Left-Up
            swipe = Swipe_action_e::LeftUp;
          }

          #  ifdef TOUCH_DEBUG

          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            addLog(LOG_LEVEL_DEBUG,
                   strformat(F("Touch Swiped, direction: %s, dx: %d, dy: %d"),
                             String(toString(swipe)).c_str(), delta_x, delta_y));
          }
          #  endif // ifdef TOUCH_DEBUG
        }
        # endif // if TOUCH_FEATURE_EXTENDED_TOUCH && TOUCH_FEATURE_SWIPE

        // Not touched yet or too long ago
        if (
          # if TOUCH_FEATURE_EXTENDED_TOUCH && TOUCH_FEATURE_SWIPE
          (swipe == Swipe_action_e::None) &&
          # endif // if TOUCH_FEATURE_EXTENDED_TOUCH && TOUCH_FEATURE_SWIPE
          ((TouchObjects[selectedObjectIndex].TouchTimers == 0) ||
           (TouchObjects[selectedObjectIndex].TouchTimers < (millis() - (1.5 * Touch_Settings.debounceMs)))
          )) {
          // From now wait the debounce time
          TouchObjects[selectedObjectIndex].TouchTimers = millis() + Touch_Settings.debounceMs;
        } else {
          // Debouncing time elapsed? Swiping/sliding passes through without debounce

          if (
            # if TOUCH_FEATURE_EXTENDED_TOUCH && TOUCH_FEATURE_SWIPE
            (swipe != Swipe_action_e::None) ||
            # endif // if TOUCH_FEATURE_EXTENDED_TOUCH && TOUCH_FEATURE_SWIPE
            (TouchObjects[selectedObjectIndex].TouchTimers <= millis())) {
            TouchObjects[selectedObjectIndex].TouchTimers = 0;

            if (
              # if TOUCH_FEATURE_EXTENDED_TOUCH && TOUCH_FEATURE_SWIPE
              (swipe == Swipe_action_e::None) &&
              # endif // if TOUCH_FEATURE_EXTENDED_TOUCH && TOUCH_FEATURE_SWIPE
              (selectedObjectIndex > -1) && bitRead(TouchObjects[selectedObjectIndex].flags, TOUCH_OBJECT_FLAG_BUTTON)) {
              // Button touched
              _lastObjectIndex = selectedObjectIndex; // Handle on release
            # if TOUCH_FEATURE_EXTENDED_TOUCH && TOUCH_FEATURE_SWIPE
            } else if ((swipe != Swipe_action_e::None) &&
                       (selectedObjectIndex > -1) && bitRead(TouchObjects[selectedObjectIndex].flags, TOUCH_OBJECT_FLAG_SLIDER)) {
              // Handle slider immediately to move/set absolute position
              _lastObjectIndex = -1; // Handled
              const bool isVertical = TouchObjects[selectedObjectIndex].width_height.x < TouchObjects[selectedObjectIndex].width_height.y;
              int16_t    position   = 0;
              int16_t    lowRange   = 0;
              int16_t    highRange  = 100;
              bool useRange         = false;

              if (!TouchObjects[selectedObjectIndex].captionOff.isEmpty()) { // Off caption can hold range: <from>,<to>
                useRange = parseRangeToInt16(TouchObjects[selectedObjectIndex].captionOff, lowRange, highRange);
              }

              if (isVertical) {
                position = (TouchObjects[selectedObjectIndex].top_left.y + TouchObjects[selectedObjectIndex].width_height.y) - y;
                position = ceil(position / (TouchObjects[selectedObjectIndex].width_height.y / 100.0));
              } else {
                position = x - TouchObjects[selectedObjectIndex].top_left.x;
                position = ceil(position / (TouchObjects[selectedObjectIndex].width_height.x / 100.0));
              }

              if (useRange) { // Calculate range-boundaries
                position                                      = map(position, 0, 100, lowRange, highRange);
                TouchObjects[selectedObjectIndex].TouchStates = position;
              } else if (position < lowRange) {
                TouchObjects[selectedObjectIndex].TouchStates = lowRange;
              } else if (position > highRange) {
                TouchObjects[selectedObjectIndex].TouchStates = highRange;
              } else {
                TouchObjects[selectedObjectIndex].TouchStates = position;
              }

              // Reduce the number of events during sliding
              if ((TouchObjects[selectedObjectIndex].TouchTimers == 0) ||
                  (TouchObjects[selectedObjectIndex].TouchTimers < millis())) {
                generateObjectEvent(event, selectedObjectIndex, TouchObjects[selectedObjectIndex].TouchStates);
                TouchObjects[selectedObjectIndex].TouchTimers = millis() + (2 * Touch_Settings.debounceMs);
              } else {
                _lastObjectIndex = selectedObjectIndex; // Update on touch-release
              }
            # endif // if TOUCH_FEATURE_EXTENDED_TOUCH && TOUCH_FEATURE_SWIPE
            } else {                                    // Generic touch event
              _lastObjectIndex = -2;                    // Update on touch-release
              _lastObjectName  = selectedObjectName;

              # if TOUCH_FEATURE_EXTENDED_TOUCH && TOUCH_FEATURE_SWIPE
              #  ifdef TOUCH_DEBUG
              addLogMove(LOG_LEVEL_INFO,
                         strformat(F("Swiped/touched, object: %s:%s"), _lastObjectName.c_str(),
                                   String(toString(swipe)).c_str()));
              #  endif // ifdef TOUCH_DEBUG

              if (swipe != Swipe_action_e::None) {
                _lastSwipe = swipe;
              }
              _last_delta_x = delta_x;
              _last_delta_y = delta_y;
              # endif // if TOUCH_FEATURE_EXTENDED_TOUCH && TOUCH_FEATURE_SWIPE
            }
          }
          _last_point.x   = x; // Save last touchpoint
          _last_point.y   = y;
          _last_point_z.x = z; // Don't want to extend the struct for 1 use
        }
      }
    }
  }

  return success;
}

/**
 * Release touch
 */
void ESPEasy_TouchHandler::releaseTouch(struct EventStruct *event) {
  if ((_lastObjectIndex > -1) && bitRead(TouchObjects[_lastObjectIndex].flags, TOUCH_OBJECT_FLAG_BUTTON)) {
    TouchObjects[_lastObjectIndex].TouchStates = (TouchObjects[_lastObjectIndex].TouchStates > 0 ? 0 : 1); // Flip state
    generateObjectEvent(event, _lastObjectIndex, TouchObjects[_lastObjectIndex].TouchStates > 0 ? 1 : 0);
    _lastObjectIndex = -1;                                                                                 // Handle only once
  } else if ((_lastObjectIndex > -1) && bitRead(TouchObjects[_lastObjectIndex].flags, TOUCH_OBJECT_FLAG_SLIDER)) {
    generateObjectEvent(event, _lastObjectIndex, TouchObjects[_lastObjectIndex].TouchStates);
    TouchObjects[_lastObjectIndex].TouchTimers = 0;
    _lastObjectIndex                           = -1; // Handle only once
  } else if (_lastObjectIndex != -1) {
    // Matching object is found, send <TaskDeviceName>#<ObjectName> event with x, y and z as %eventvalue1/2/3%
    String eventCommand;
    eventCommand.reserve(48);
    eventCommand  = getTaskDeviceName(event->TaskIndex);
    eventCommand += '#';

    # if TOUCH_FEATURE_EXTENDED_TOUCH && TOUCH_FEATURE_SWIPE

    if (_lastSwipe == Swipe_action_e::None)
    # endif // if TOUCH_FEATURE_EXTENDED_TOUCH && TOUCH_FEATURE_SWIPE
    {
      eventCommand += strformat(F("%s=%d,%d,%d"),
                                _lastObjectName.c_str(),
                                _last_point.x,
                                _last_point.y,
                                _last_point_z.x);
    }
    # if TOUCH_FEATURE_EXTENDED_TOUCH && TOUCH_FEATURE_SWIPE
    else {
      eventCommand += strformat(F("Swiped=%d,%d,%d"), // Add arguments
                                static_cast<int>(_lastSwipe),
                                _last_delta_x,
                                _last_delta_y);
      _lastSwipe = Swipe_action_e::None;
    }
    # endif // if TOUCH_FEATURE_EXTENDED_TOUCH && TOUCH_FEATURE_SWIPE
    eventQueue.addMove(std::move(eventCommand));
    _lastObjectIndex = -1; // Handle only once
  }
  _stillTouching = false;
}

/**
 * Parse and execute the plugin commands
 */
bool ESPEasy_TouchHandler::plugin_write(struct EventStruct *event,
                                        const String      & string) {
  bool    success = false;
  String  command;
  String  subcommand;
  String  arguments;
  uint8_t arg = 3;

  command = parseString(string, 1);

  if (equals(command, F("touch"))) {
    arguments.reserve(24);
    subcommand = parseString(string, 2);
    # ifdef TOUCH_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      addLog(LOG_LEVEL_DEBUG, strformat(F("TOUCH PLUGIN_WRITE arguments Par1: %d, 2: %d, 3: %d, 4: %d, string: %s"),
                                        event->Par1, event->Par2, event->Par3, event->Par4, string.c_str()));
    }
    # endif // ifdef TOUCH_DEBUG

    if (equals(subcommand, F("enable"))) { // touch,enable,<objectName>[,...] : Enable disabled objectname(s)
      arguments = parseString(string, arg);

      while (!arguments.isEmpty()) {
        success  |= setTouchObjectState(event, arguments, true);
        arguments = parseString(string, ++arg);
      }
    } else if (equals(subcommand, F("disable"))) { // touch,disable,<objectName>[,...] : Disable enabled objectname(s)
      arguments = parseString(string, arg);

      while (!arguments.isEmpty()) {
        success  |= setTouchObjectState(event, arguments, false);
        arguments = parseString(string, ++arg);
      }
    } else if (equals(subcommand, F("on"))) { // touch,on,<buttonObjectName>[,...] : Switch TouchButton(s) on
      arguments = parseString(string, arg);

      while (!arguments.isEmpty()) {
        success  |= setTouchButtonOnOff(event, arguments, true);
        arguments = parseString(string, ++arg);
      }
    } else if (equals(subcommand, F("off"))) { // touch,off,<buttonObjectName>[,...] : Switch TouchButton(s) off
      arguments = parseString(string, arg);

      while (!arguments.isEmpty()) {
        success  |= setTouchButtonOnOff(event, arguments, false);
        arguments = parseString(string, ++arg);
      }
    } else if (equals(subcommand, F("toggle"))) { // touch,toggle,<buttonObjectName>[,...] : Switch TouchButton(s) to the other state
      arguments = parseString(string, arg);

      while (!arguments.isEmpty()) {
        const int16_t state = getTouchObjectValue(event, arguments);

        if (state > -1) {
          success |= setTouchButtonOnOff(event, arguments, state == 0);
        }
        arguments = parseString(string, ++arg);
      }
    } else if (equals(subcommand, F("set"))) { // touch,set,<objectName>,<value> : Set TouchObject value
      arguments = parseString(string, arg);
      success   = setTouchObjectValue(event, arguments, event->Par3);
    # if TOUCH_FEATURE_EXTENDED_TOUCH
    #  if TOUCH_FEATURE_EXTENDED_TOUCH && TOUCH_FEATURE_SWIPE
    } else if (equals(subcommand, F("swipe"))) {        // touch,swipe,<swipeValue> : Switch button group via swipe value
      success = handleButtonSwipe(event, event->Par2);
    #  endif // if TOUCH_FEATURE_EXTENDED_TOUCH && TOUCH_FEATURE_SWIPE
    } else if (equals(subcommand, F("setgrp"))) {       // touch,setgrp,<group> : Activate button group
      success = setButtonGroup(event, event->Par2);
    } else if (equals(subcommand, F("nextgrp"))) {      // touch,nextgrp : next group and Activate
      success = nextButtonGroup(event);
    } else if (equals(subcommand, F("prevgrp"))) {      // touch,prevgrp : previous group and Activate
      success = prevButtonGroup(event);
    } else if (equals(subcommand, F("nextpage"))) {     // touch,nextpage : next page and Activate
      success = nextButtonPage(event);
    } else if (equals(subcommand, F("prevpage"))) {     // touch,prevpage : previous page and Activate
      success = prevButtonPage(event);
    } else if (equals(subcommand, F("updatebutton"))) { // touch,updatebutton,<buttonnr>[,<group>[,<mode>]] : Update a button
      arguments = parseString(string, 3);

      // Check for a valid button name or number, returns a 0-based index
      const int8_t index = getTouchObjectIndex(event, arguments, true);

      if (index > -1) {
        const bool hasPar3 = !parseString(string, 4).isEmpty();
        const bool hasPar4 = !parseString(string, 5).isEmpty();

        if (hasPar4) {
          success = displayButton(event, index, event->Par3, event->Par4);
        } else if (hasPar3) {
          success = displayButton(event, index, event->Par3);
        } else {
          success = displayButton(event, index); // Use default argument values
        }
      }
    # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
    }
  }
  return success;
}

/**
 * Handle getting config values from plugin/handler
 */
bool ESPEasy_TouchHandler::plugin_get_config_value(struct EventStruct *event,
                                                   String            & string) {
  bool success         = false;
  const String command = parseString(string, 1);

  if (equals(command, F("buttongroup"))) {
    string  = getButtonGroup();
    success = true;
  # if TOUCH_FEATURE_EXTENDED_TOUCH
  } else if (equals(command, F("hasgroup"))) {
    int32_t group; // We'll be ignoring group 0 if there are multiple button groups

    if (validIntFromString(parseString(string, 2), group)) {
      string  = validButtonGroup(group, true) ? 1 : 0;
      success = true;
    } else {
      string = '0'; // invalid number = false
    }
  # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
  } else if (equals(command, F("enabled"))) {
    const int8_t enabled = getTouchObjectState(event, parseStringKeepCase(string, 2));

    if (enabled > -1) {
      string  = enabled;
      success = true;
    }
  } else if (equals(command, F("state"))) {
    const int16_t state = getTouchObjectValue(event, parseStringKeepCase(string, 2));

    string  = state;
    success = true;
  # if TOUCH_FEATURE_EXTENDED_TOUCH
  } else if (equals(command, F("pagemode"))) {
    string  = bitRead(Touch_Settings.flags, TOUCH_FLAGS_PGUP_BELOW_MENU);
    success = true;
  #  if TOUCH_FEATURE_SWIPE
  } else if (equals(command, F("swipedir"))) {
    int32_t state;

    if (validIntFromString(parseString(string, 2), state)) {
      string  = toString(static_cast<Swipe_action_e>(state));
      success = true;
    }
  #  endif // if TOUCH_FEATURE_SWIPE
  # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
  }
  return success;
}

/**
 * generate an event for a touch object
 * When a display is configured add x,y coordinate, width,height of the object, objectIndex, and TaskIndex of display
 **************************************************************************/
void ESPEasy_TouchHandler::generateObjectEvent(struct EventStruct *event,
                                               const int8_t      & objectIndex,
                                               const int16_t     & onOffState,
                                               const int8_t      & mode,
                                               const bool        & groupSwitch,
                                               const int8_t      & factor) {
  if ((objectIndex < 0) || // Range check
      (objectIndex >= static_cast<int8_t>(TouchObjects.size()))) {
    return;
  }
  delay(0);
  String eventCommand;
  String extraCommand;

  eventCommand.reserve(120);
  extraCommand.reserve(48);

  // Task with added arguments: (%eventvalue#%)
  extraCommand += strformat(F("%s#%s="), getTaskDeviceName(event->TaskIndex).c_str(), TouchObjects[objectIndex].objectName.c_str());

  if (bitRead(Touch_Settings.flags, TOUCH_FLAGS_DRAWBTN_VIA_RULES)) {
    eventCommand = extraCommand;
  } else {                                  // Handle via direct btn commands
    if (_displayTask != event->TaskIndex) { // Add arguments for display
      // Internal command trigger
      eventCommand += strformat(F("[%d].adagfx_trigger,btn,"), _displayTask + 1);
    } else {
      addLog(LOG_LEVEL_ERROR, F("TOUCH: No valid Display task selected."));
      return;
    }
  }

  if (bitRead(TouchObjects[objectIndex].flags, TOUCH_OBJECT_FLAG_SLIDER)) {
    eventCommand += onOffState;                      // Slider control: pass value as state (1 = state)
    extraCommand += onOffState;                      // duplicate
  } else {
    if (onOffState < 0) {                            // Negative value: pass on unaltered (1 = state)
      eventCommand += onOffState;
      extraCommand += onOffState;                    // duplicate
    } else {                                         // Check for inverted output (1 = state)
      if (bitRead(TouchObjects[objectIndex].flags, TOUCH_OBJECT_FLAG_INVERTED)) {
        eventCommand += onOffState == 1 ? '0' : '1'; // Act like an inverted button, 0 = On, 1 = Off
        extraCommand += onOffState == 1 ? '0' : '1'; // Act like an inverted button, 0 = On, 1 = Off // duplicate
      } else {
        eventCommand += onOffState == 1 ? '1' : '0'; // Act like a button, 1 = On, 0 = Off
        extraCommand += onOffState == 1 ? '1' : '0'; // Act like a button, 1 = On, 0 = Off // duplicate
      }
    }
  }
  eventCommand += ',';
  eventCommand += mode;                         // (2 = mode)
  extraCommand += ',';
  extraCommand += mode;                         // (2 = mode) // duplicate

  if (_displayTask != event->TaskIndex) {       // Add arguments for display
    eventCommand += strformat(
      F(",%d,%d,%d,%d,%d,%d"),
      TouchObjects[objectIndex].top_left.x,     // (3 = x)
      TouchObjects[objectIndex].top_left.y,     // (4 = y)
      TouchObjects[objectIndex].width_height.x, // (5 = width)
      TouchObjects[objectIndex].width_height.y, // (6 = height)
      objectIndex + 1,                          // Adjust to displayed index (7 = id)
                                                // (8 = type + layout, 4+4 bit, side by side)
      get8BitFromUL(TouchObjects[objectIndex].flags, TOUCH_OBJECT_FLAG_BUTTONTYPE) * factor);
    # if TOUCH_FEATURE_EXTENDED_TOUCH
    eventCommand += strformat(
      F(",%s,%s,%s,%d,"),
      AdaGFXcolorToString(TouchObjects[objectIndex].colorOn == 0
                          ? Touch_Settings.colorOn
                          : TouchObjects[objectIndex].colorOn, _colorDepth).c_str(),      // (9 = ON color)
      AdaGFXcolorToString(TouchObjects[objectIndex].colorOff == 0
                          ? Touch_Settings.colorOff
                          : TouchObjects[objectIndex].colorOff, _colorDepth).c_str(),     // (10 = OFF color)
      AdaGFXcolorToString(TouchObjects[objectIndex].colorCaption == 0
                          ? Touch_Settings.colorCaption
                          : TouchObjects[objectIndex].colorCaption, _colorDepth).c_str(), // (11 = Caption color)
      get4BitFromUL(TouchObjects[objectIndex].flags, TOUCH_OBJECT_FLAG_FONTSCALE));       // (12 = Font scaling)

    // (13 = ON caption, default=object name)
    String _capt;

    if (TouchObjects[objectIndex].captionOn.isEmpty()) {
      if (bitRead(TouchObjects[objectIndex].flags, TOUCH_OBJECT_FLAG_SLIDER)) {
        _capt = onOffState; // Override caption if not set
      } else {
        _capt = TouchObjects[objectIndex].objectName;
      }
    } else {
      _capt = TouchObjects[objectIndex].captionOn;
    }
    _capt.replace('_', ' '); // Replace all '_' by space
    eventCommand += wrapWithQuotesIfContainsParameterSeparatorChar(_capt);
    eventCommand += ',';     // (14 = OFF caption)

    if (TouchObjects[objectIndex].captionOff.isEmpty()) {
      if (bitRead(TouchObjects[objectIndex].flags, TOUCH_OBJECT_FLAG_SLIDER)) {
        _capt = onOffState; // override caption if not set
      } else {
        _capt = TouchObjects[objectIndex].objectName;
      }
    } else {
      _capt = TouchObjects[objectIndex].captionOff;
    }
    _capt.replace('_', ' '); // Replace all '_' by space
    eventCommand += strformat(
      F("%s,%s,%s,%s"),
      wrapWithQuotesIfContainsParameterSeparatorChar(_capt).c_str(),
      AdaGFXcolorToString(TouchObjects[objectIndex].colorBorder == 0
                          ? Touch_Settings.colorBorder
                          : TouchObjects[objectIndex].colorBorder, _colorDepth).c_str(),           // (15 = Border color)
      AdaGFXcolorToString(TouchObjects[objectIndex].colorDisabled == 0
                          ? Touch_Settings.colorDisabled
                          : TouchObjects[objectIndex].colorDisabled, _colorDepth).c_str(),         // (16 = Disabled color)
      AdaGFXcolorToString(TouchObjects[objectIndex].colorDisabledCaption == 0
                          ? Touch_Settings.colorDisabledCaption
                          : TouchObjects[objectIndex].colorDisabledCaption, _colorDepth).c_str()); // (17 = Disabled caption color)
    # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
    eventCommand += strformat(
      F(",%d,%d"),
      _displayTask + 1,                                                         // What TaskIndex? (18) or (9)
      get8BitFromUL(TouchObjects[objectIndex].flags, TOUCH_OBJECT_FLAG_GROUP)); // Group (19) or (10)
    # if TOUCH_FEATURE_EXTENDED_TOUCH
    eventCommand += ',';                                                        // Group mode (20)
    const uint8_t action        = get4BitFromUL(TouchObjects[objectIndex].groupFlags, TOUCH_OBJECT_GROUP_ACTION);
    const Touch_action_e actGrp = static_cast<Touch_action_e>(action);

    if (!groupSwitch && (Touch_action_e::Default != actGrp)) {
      if (Touch_action_e::ActivateGroup == actGrp) {
        eventCommand += get8BitFromUL(TouchObjects[objectIndex].groupFlags, TOUCH_OBJECT_GROUP_ACTIONGROUP);
      } else {
        eventCommand += (action * -1); // Default is already ignored
      }
    } else {
      eventCommand += -1;              // No group to activate
    }
    # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
  }

  # if TOUCH_FEATURE_EXTENDED_TOUCH

  if (bitRead(Touch_Settings.flags, TOUCH_FLAGS_DRAWBTN_VIA_RULES)) {
    eventQueue.addMove(std::move(eventCommand));
  } else {
    eventCommand += ',';
    eventCommand += wrapWithQuotesIfContainsParameterSeparatorChar(TouchObjects[objectIndex].objectName);
    ExecuteCommand_all({ EventValueSource::Enum::VALUE_SOURCE_RULES, eventCommand }, true); // Simulate like from rules
    addLogMove(LOG_LEVEL_INFO, eventCommand);
    delay(0);

    // Handle group actions
    Touch_action_e action = static_cast<Touch_action_e>(get4BitFromUL(TouchObjects[objectIndex].groupFlags, TOUCH_OBJECT_GROUP_ACTION));

    if ((onOffState >= 0) && (mode >= 0)) {
      if (action == Touch_action_e::Default) {
        eventQueue.addMove(std::move(extraCommand)); // Issue the extra command for regular button presses
      } else {
        switch (action) {
          case Touch_action_e::ActivateGroup:
            setButtonGroup(event, get8BitFromUL(TouchObjects[objectIndex].groupFlags, TOUCH_OBJECT_GROUP_ACTIONGROUP));
            break;
          case Touch_action_e::IncrementGroup:
            nextButtonGroup(event);
            break;
          case Touch_action_e::DecrementGroup:
            prevButtonGroup(event);
            break;
          case Touch_action_e::IncrementPage:
            nextButtonPage(event);
            break;
          case Touch_action_e::DecrementPage:
            prevButtonPage(event);
            break;
          case Touch_action_e::Default:
            break;
        }
        addLogMove(LOG_LEVEL_INFO, concat(F("TOUCH event: "), toString(action)));
      }
    }
  }
  # endif // if TOUCH_FEATURE_EXTENDED_TOUCH
  delay(0);
}

#endif // ifdef PLUGIN_USES_TOUCHHANDLER
