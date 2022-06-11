#include "../Helpers/ESPEasy_TouchHandler.h"

#ifdef PLUGIN_USES_TOUCHHANDLER

/****************************************************************************
 * toString: Display-value for the touch action
 ***************************************************************************/
# ifdef TOUCH_USE_EXTENDED_TOUCH
const __FlashStringHelper* toString(Touch_action_e action) {
  switch (action) {
    case Touch_action_e::Default: return F("Default");
    case Touch_action_e::ActivateGroup: return F("Activate Group");
    case Touch_action_e::IncrementGroup: return F("Next Group");
    case Touch_action_e::DecrementGroup: return F("Previous Group");
    case Touch_action_e::IncrementPage: return F("Next Page (+10)");
    case Touch_action_e::DecrementPage: return F("Previous Page (-10)");
    case Touch_action_e::TouchAction_MAX: break;
  }
  return F("Unsupported!");
}

# endif // ifdef TOUCH_USE_EXTENDED_TOUCH

/**
 * Constructors
 */
ESPEasy_TouchHandler::ESPEasy_TouchHandler() {}

ESPEasy_TouchHandler::ESPEasy_TouchHandler(const uint16_t        & displayTask,
                                           const AdaGFXColorDepth& colorDepth)
  : _displayTask(displayTask), _colorDepth(colorDepth) {}

/**
 * Load the touch objects from the settings, and initialize then properly where needed.
 */
void ESPEasy_TouchHandler::loadTouchObjects(struct EventStruct *event) {
  # ifdef TOUCH_DEBUG
  addLogMove(LOG_LEVEL_INFO, F("TOUCH DEBUG loadTouchObjects"));
  # endif // TOUCH_DEBUG
  LoadCustomTaskSettings(event->TaskIndex, settingsArray, TOUCH_ARRAY_SIZE, 0);

  lastObjectIndex = TOUCH_OBJECT_INDEX_START - 1; // START must be > 0!!!

  objectCount = 0;
  _buttonGroups.clear();                          // Clear groups
  _buttonGroups.insert(0u);                       // Always have group 0

  for (uint8_t i = TOUCH_OBJECT_INDEX_END; i >= TOUCH_OBJECT_INDEX_START; i--) {
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

  bitWrite(lSettings, TOUCH_FLAGS_SEND_XY,         TOUCH_TS_SEND_XY);
  bitWrite(lSettings, TOUCH_FLAGS_SEND_Z,          TOUCH_TS_SEND_Z);
  bitWrite(lSettings, TOUCH_FLAGS_SEND_OBJECTNAME, TOUCH_TS_SEND_OBJECTNAME);
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
  # ifdef TOUCH_USE_EXTENDED_TOUCH
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

  if ((Touch_Settings.colorOn              == 0u) &&
      (Touch_Settings.colorOff             == 0u) &&
      (Touch_Settings.colorCaption         == 0u) &&
      (Touch_Settings.colorBorder          == 0u) &&
      (Touch_Settings.colorDisabled        == 0u) &&
      (Touch_Settings.colorDisabledCaption == 0u)) {
    Touch_Settings.colorOn              = ADAGFX_BLUE;
    Touch_Settings.colorOff             = ADAGFX_RED;
    Touch_Settings.colorCaption         = ADAGFX_WHITE;
    Touch_Settings.colorBorder          = ADAGFX_WHITE;
    Touch_Settings.colorDisabled        = TOUCH_DEFAULT_COLOR_DISABLED;
    Touch_Settings.colorDisabledCaption = TOUCH_DEFAULT_COLOR_DISABLED_CAPTION;
  }
  # endif // ifdef TOUCH_USE_EXTENDED_TOUCH

  settingsArray[TOUCH_CALIBRATION_START].clear(); // Free a little memory

  // Buffer some settings, mostly for readability, but also to be able to set from write command
  _flipped     = bitRead(Touch_Settings.flags, TOUCH_FLAGS_ROTATION_FLIPPED);
  _deduplicate = bitRead(Touch_Settings.flags, TOUCH_FLAGS_DEDUPLICATE);

  TouchObjects.clear();

  if (objectCount > 0) {
    TouchObjects.reserve(objectCount);
    uint8_t t = 0u;

    for (uint8_t i = TOUCH_OBJECT_INDEX_START; i <= lastObjectIndex; i++) {
      if (!settingsArray[i].isEmpty()) {
        TouchObjects.push_back(tTouchObjects());
        TouchObjects[t].flags          = parseStringToInt(settingsArray[i], TOUCH_OBJECT_FLAGS, TOUCH_SETTINGS_SEPARATOR);
        TouchObjects[t].objectName     = parseStringKeepCase(settingsArray[i], TOUCH_OBJECT_NAME, TOUCH_SETTINGS_SEPARATOR);
        TouchObjects[t].top_left.x     = parseStringToInt(settingsArray[i], TOUCH_OBJECT_COORD_TOP_X, TOUCH_SETTINGS_SEPARATOR);
        TouchObjects[t].top_left.y     = parseStringToInt(settingsArray[i], TOUCH_OBJECT_COORD_TOP_Y, TOUCH_SETTINGS_SEPARATOR);
        TouchObjects[t].width_height.x = parseStringToInt(settingsArray[i], TOUCH_OBJECT_COORD_WIDTH, TOUCH_SETTINGS_SEPARATOR);
        TouchObjects[t].width_height.y = parseStringToInt(settingsArray[i], TOUCH_OBJECT_COORD_HEIGHT, TOUCH_SETTINGS_SEPARATOR);
        # ifdef TOUCH_USE_EXTENDED_TOUCH
        TouchObjects[t].colorOn              = parseStringToInt(settingsArray[i], TOUCH_OBJECT_COLOR_ON, TOUCH_SETTINGS_SEPARATOR);
        TouchObjects[t].colorOff             = parseStringToInt(settingsArray[i], TOUCH_OBJECT_COLOR_OFF, TOUCH_SETTINGS_SEPARATOR);
        TouchObjects[t].colorCaption         = parseStringToInt(settingsArray[i], TOUCH_OBJECT_COLOR_CAPTION, TOUCH_SETTINGS_SEPARATOR);
        TouchObjects[t].captionOn            = parseStringKeepCase(settingsArray[i], TOUCH_OBJECT_CAPTION_ON, TOUCH_SETTINGS_SEPARATOR);
        TouchObjects[t].captionOff           = parseStringKeepCase(settingsArray[i], TOUCH_OBJECT_CAPTION_OFF, TOUCH_SETTINGS_SEPARATOR);
        TouchObjects[t].colorBorder          = parseStringToInt(settingsArray[i], TOUCH_OBJECT_COLOR_BORDER, TOUCH_SETTINGS_SEPARATOR);
        TouchObjects[t].colorDisabled        = parseStringToInt(settingsArray[i], TOUCH_OBJECT_COLOR_DISABLED, TOUCH_SETTINGS_SEPARATOR);
        TouchObjects[t].colorDisabledCaption = parseStringToInt(settingsArray[i], TOUCH_OBJECT_COLOR_DISABCAPT, TOUCH_SETTINGS_SEPARATOR);
        TouchObjects[t].groupFlags           = parseStringToInt(settingsArray[i], TOUCH_OBJECT_GROUPFLAGS, TOUCH_SETTINGS_SEPARATOR);

        if (!validButtonGroup(get8BitFromUL(TouchObjects[t].flags, TOUCH_OBJECT_FLAG_GROUP))) {
          _buttonGroups.insert(get8BitFromUL(TouchObjects[t].flags, TOUCH_OBJECT_FLAG_GROUP));
        }
        # endif // ifdef TOUCH_USE_EXTENDED_TOUCH

        TouchObjects[t].SurfaceAreas = 0; // Reset runtime stuff
        TouchObjects[t].TouchTimers  = 0;
        TouchObjects[t].TouchStates  = false;

        t++;

        settingsArray[i].clear(); // Free a little memory
      }
    }
  }

  // if (_maxButtonGroup > 0) {
  //   _minButtonGroup = 1;
  // }
}

/**
 * init
 */
void ESPEasy_TouchHandler::init(struct EventStruct *event) {
  if (!_settingsLoaded) {
    loadTouchObjects(event);
    _settingsLoaded = true;
  }

  if (bitRead(Touch_Settings.flags, TOUCH_FLAGS_SEND_OBJECTNAME) &&
      bitRead(Touch_Settings.flags, TOUCH_FLAGS_INIT_OBJECTEVENT)) {
    if (_buttonGroups.size() > 1) {                // Multiple groups?
      displayButtonGroup(event, _buttonGroup, -3); // Clear all displayed groups
    }
    _buttonGroup = get8BitFromUL(Touch_Settings.flags, TOUCH_FLAGS_INITIAL_GROUP);
    # ifdef TOUCH_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("TOUCH DEBUG group: ");
      log += _buttonGroup;
      log += F(", max group: ");
      log += *_buttonGroups.crbegin();
      addLogMove(LOG_LEVEL_INFO, log);
    }
    # endif // ifdef TOUCH_DEBUG

    displayButtonGroup(event, _buttonGroup); // Initialize selected group and group 0

    # ifdef TOUCH_DEBUG
    addLogMove(LOG_LEVEL_INFO, F("TOUCH DEBUG group done."));
    # endif // ifdef TOUCH_DEBUG
  }
}

/**
 * helper function: use parseString() to read an argument, and convert that to an int value
 */
int ESPEasy_TouchHandler::parseStringToInt(const String & string,
                                           const uint8_t& indexFind,
                                           const char   & separator,
                                           const int    & defaultValue) {
  String parsed = parseStringKeepCase(string, indexFind, separator);

  int result = defaultValue;

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

  for (int objectNr = 0; objectNr < static_cast<int>(TouchObjects.size()); objectNr++) {
    uint8_t group = get8BitFromUL(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_GROUP);

    if (!TouchObjects[objectNr].objectName.isEmpty()
        && bitRead(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_ENABLED)
        && (TouchObjects[objectNr].width_height.x != 0)
        && (TouchObjects[objectNr].width_height.y != 0) // Not initial could be valid
        && ((group == 0) || (group == _buttonGroup))) { // Group 0 is always active
      if (TouchObjects[objectNr].SurfaceAreas == 0) {   // Need to calculate the surface area
        TouchObjects[objectNr].SurfaceAreas = TouchObjects[objectNr].width_height.x * TouchObjects[objectNr].width_height.y;
      }

      if ((TouchObjects[objectNr].top_left.x <= x)
          && (TouchObjects[objectNr].top_left.y <= y)
          && ((TouchObjects[objectNr].width_height.x + TouchObjects[objectNr].top_left.x) >= x)
          && ((TouchObjects[objectNr].width_height.y + TouchObjects[objectNr].top_left.y) >= y)
          && ((lastObjectArea == 0) ||
              (TouchObjects[objectNr].SurfaceAreas < lastObjectArea))) { // Select smallest area that fits the coordinates
        selectedObjectName  = TouchObjects[objectNr].objectName;
        selectedObjectIndex = objectNr;
        lastObjectArea      = TouchObjects[objectNr].SurfaceAreas;
        selected            = true;
      }
      # if defined(TOUCH_DEBUG) && !defined(BUILD_NO_DEBUG)

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        String log = F("TOUCH DEBUG Touched: obj: ");
        log += TouchObjects[objectNr].objectName;
        log += ',';
        log += TouchObjects[objectNr].top_left.x;
        log += ',';
        log += TouchObjects[objectNr].top_left.y;
        log += ',';
        log += TouchObjects[objectNr].width_height.x;
        log += ',';
        log += TouchObjects[objectNr].width_height.y;
        log += F(" surface:");
        log += TouchObjects[objectNr].SurfaceAreas;
        log += F(" x,y:");
        log += x;
        log += ',';
        log += y;
        log += F(" sel:");
        log += selectedObjectName;
        log += '/';
        log += selectedObjectIndex;
        log += '/';
        log += selected ? 'T' : 'f';
        addLogMove(LOG_LEVEL_DEBUG, log);
      }
      # endif // if defined(TOUCH_DEBUG) && !defined(BUILD_NO_DEBUG)
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

  int index = -1;

  // ATTENTION: Any externally provided objectNumber is 1-based, result is 0-based
  if (validIntFromString(touchObject, index) &&
      (index > 0) &&
      (index <= static_cast<int>(TouchObjects.size()))) {
    return static_cast<int8_t>(index - 1);
  }

  for (size_t objectNr = 0; objectNr < TouchObjects.size(); objectNr++) {
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

  int8_t objectNr = getTouchObjectIndex(event, touchObject);

  if (objectNr > -1) {
    success = true;                                                             // Succes if matched object

    if (state != bitRead(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_ENABLED)) {
      bitWrite(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_ENABLED, state); // Store in settings, no save

      // Event when enabling/disabling
      if (bitRead(Touch_Settings.flags, TOUCH_FLAGS_SEND_OBJECTNAME) &&
          bitRead(Touch_Settings.flags, TOUCH_FLAGS_INIT_OBJECTEVENT)) {
        generateObjectEvent(event, objectNr, TouchObjects[objectNr].TouchStates ? 1 : 0, state ? -1 : -2); // Redraw only, no activation
      }
    }
    # ifdef TOUCH_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("TOUCH setTouchObjectState: obj: ");
      log += touchObject;
      log += '/';
      log += objectNr;

      if (success) {
        log += F(", new state: ");
        log += (state ? F("en") : F("dis"));
        log += F("abled.");
      } else {
        log += F(" failed!");
      }
      addLogMove(LOG_LEVEL_INFO, log);
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
  int8_t result = -1;

  int8_t objectNr = getTouchObjectIndex(event, touchObject);

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
  bool success = false;

  int8_t objectNr = getTouchObjectIndex(event, touchObject, true);

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

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("TOUCH setTouchButtonOnOff: obj: ");
      log += touchObject;
      log += '/';
      log += objectNr;
      log += F(", new state: ");
      log += (state ? F("on") : F("off"));
      addLogMove(LOG_LEVEL_INFO, log);
    }
    # endif // ifdef TOUCH_DEBUG
  }
  return success;
}

/**
 * Get the on/off state of an enabled touch-button object.
 */
int8_t ESPEasy_TouchHandler::getTouchButtonOnOff(struct EventStruct *event,
                                                 const String      & touchObject) {
  if (touchObject.isEmpty()) { return false; }
  int8_t result = -1; // invalid button

  int8_t objectNr = getTouchObjectIndex(event, touchObject, true);

  if ((objectNr > -1)
      && bitRead(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_ENABLED)
      && bitRead(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_BUTTON)) {
    result = TouchObjects[objectNr].TouchStates && !bitRead(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_INVERTED) ? 1 : 0;
  }
  return result;
}

/**
 * mode: -2 = clear buttons in group, -3 = clear all buttongroups, -1 = draw buttons in group, 0 = initialize buttons
 */
void ESPEasy_TouchHandler::displayButtonGroup(struct EventStruct *event,
                                              const int16_t     & buttonGroup,
                                              const int8_t      & mode) {
  for (int objectNr = 0; objectNr < static_cast<int>(TouchObjects.size()); objectNr++) {
    displayButton(event, objectNr, buttonGroup, mode);

    delay(0);
  }

  if (bitRead(Touch_Settings.flags, TOUCH_FLAGS_SEND_OBJECTNAME)) {
    // Send an event <taskname>#Group,<group>,<mode> with the selected group and the mode (-3..0)
    String eventCommand;
    eventCommand.reserve(24);
    eventCommand += getTaskDeviceName(event->TaskIndex);
    eventCommand += '#';
    eventCommand += F("Group");
    eventCommand += '='; // Add arguments
    eventCommand += buttonGroup;
    eventCommand += ',';
    eventCommand += mode;
    eventQueue.addMove(std::move(eventCommand));
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
  int8_t  state = 99;
  int16_t group = get8BitFromUL(TouchObjects[buttonNr].flags, TOUCH_OBJECT_FLAG_GROUP);

  # ifdef TOUCH_USE_EXTENDED_TOUCH
  Touch_action_e action = static_cast<Touch_action_e>(get4BitFromUL(TouchObjects[buttonNr].groupFlags, TOUCH_OBJECT_GROUP_ACTION));
  # endif // ifdef TOUCH_USE_EXTENDED_TOUCH
  bool isArrow = false;

  # ifdef TOUCH_USE_EXTENDED_TOUCH

  if ((mode > -2) &&                                 // Not on clear (-2 and -3)
      bitRead(Touch_Settings.flags, TOUCH_FLAGS_AUTO_PAGE_ARROWS) &&
      ((action == Touch_action_e::DecrementGroup) || // Arrow buttons
       (action == Touch_action_e::IncrementGroup) ||
       (action == Touch_action_e::DecrementPage) ||
       (action == Touch_action_e::IncrementPage))) {
    isArrow = true;
  }
  # endif // ifdef TOUCH_USE_EXTENDED_TOUCH

  if (!TouchObjects[buttonNr].objectName.isEmpty() &&
      ((bitRead(TouchObjects[buttonNr].flags, TOUCH_OBJECT_FLAG_ENABLED) && (group == 0)) || (group > 0) || isArrow) &&
      bitRead(TouchObjects[buttonNr].flags, TOUCH_OBJECT_FLAG_BUTTON) &&
      (((group == buttonGroup) || (buttonGroup < 0)) ||
       ((mode != -2) && (group == 0)) ||
       (mode == -3))) {
    // Act like a button, 1 = On, 0 = Off, inversion is handled in generateObjectEvent()
    state = TouchObjects[buttonNr].TouchStates ? 1 : 0;

    # ifdef TOUCH_USE_EXTENDED_TOUCH

    if (isArrow) {                                    // Auto-Enable/Disable the arrow buttons
      bool pgupInvert = bitRead(Touch_Settings.flags, TOUCH_FLAGS_PGUP_BELOW_MENU);
      state = 1;                                      // always get ON state!

      if (action == Touch_action_e::DecrementGroup) { // Left arrow
        bitWrite(TouchObjects[buttonNr].flags, TOUCH_OBJECT_FLAG_ENABLED, validButtonGroup(buttonGroup - 1, true));
      } else
      if (action == Touch_action_e::IncrementGroup) { // Right arrow
        bitWrite(TouchObjects[buttonNr].flags, TOUCH_OBJECT_FLAG_ENABLED, validButtonGroup(buttonGroup + 1, true));
      } else
      if (action == Touch_action_e::DecrementPage) {  // Down arrow or Up arrow
        bitWrite(TouchObjects[buttonNr].flags, TOUCH_OBJECT_FLAG_ENABLED, validButtonGroup(buttonGroup + (pgupInvert ? 10 : -10), true));
      } else
      if (action == Touch_action_e::IncrementPage) {  // Up arrow or Down arrow
        bitWrite(TouchObjects[buttonNr].flags, TOUCH_OBJECT_FLAG_ENABLED, validButtonGroup(buttonGroup + (pgupInvert ? -10 : 10), true));
      }
    }
    # endif // ifdef TOUCH_USE_EXTENDED_TOUCH

    if (bitRead(TouchObjects[buttonNr].flags, TOUCH_OBJECT_FLAG_ENABLED)) {
      if (mode == 0) {
        mode = -1;
      }
    } else {
      state -= 2; // disabled
    }
    generateObjectEvent(event, buttonNr, state, mode, mode < 0, mode <= -2 ? -1 : 1);
  }
  # ifdef TOUCH_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("TOUCH: button init, state: ");
    log += state;
    log += F(", group: ");
    log += buttonGroup;
    log += F(", mode: ");
    log += mode;
    log += F(", group: ");
    log += get8BitFromUL(TouchObjects[buttonNr].flags, TOUCH_OBJECT_FLAG_GROUP);
    log += F(", en: ");
    log += bitRead(TouchObjects[buttonNr].flags, TOUCH_OBJECT_FLAG_BUTTON);
    log += F(", object: ");
    log += buttonNr;
    addLog(LOG_LEVEL_DEBUG, log);
  }
  # endif // ifdef TOUCH_DEBUG
  return true;
}

/**
 * Check if this is a valid button group
 * When ignoreZero = true will return false for group 0 if the number of groups > 1.
 * NB: Group 0 is always available, even without button definitions!
 */
bool ESPEasy_TouchHandler::validButtonGroup(const int16_t& group,
                                            const bool   & ignoreZero) {
  return _buttonGroups.find(group) != _buttonGroups.end() &&
         (!ignoreZero || group > 0 || (group == 0 && _buttonGroups.size() == 1));
}

/**
 * Set the desired button group, must be a known group, previous group will be erased and new group drawn
 */
bool ESPEasy_TouchHandler::setButtonGroup(struct EventStruct *event,
                                          const int16_t     & buttonGroup) {
  if (validButtonGroup(buttonGroup)) {
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
bool ESPEasy_TouchHandler::incrementButtonGroup(struct EventStruct *event) {
  if (validButtonGroup(_buttonGroup + 1)) {
    return setButtonGroup(event, _buttonGroup + 1);
  }
  return false;
}

/**
 * Decrement button group if that group exists, if max. group > 0 then min. group = 1
 */
bool ESPEasy_TouchHandler::decrementButtonGroup(struct EventStruct *event) {
  if (validButtonGroup(_buttonGroup - 1)) {
    return setButtonGroup(event, _buttonGroup - 1);
  }
  return false;
}

/**
 * Increment button group by page (+10), if max. group > 0 then min. group = 1
 */
bool ESPEasy_TouchHandler::incrementButtonPage(struct EventStruct *event) {
  bool pgupInvert = bitRead(Touch_Settings.flags, TOUCH_FLAGS_PGUP_BELOW_MENU);

  if (validButtonGroup(_buttonGroup + (pgupInvert ? -10 : 10))) {
    return setButtonGroup(event, _buttonGroup + (pgupInvert ? -10 : 10));
  }
  return false;
}

/**
 * Decrement button group by page (+10), if max. group > 0 then min. group = 1
 */
bool ESPEasy_TouchHandler::decrementButtonPage(struct EventStruct *event) {
  bool pgupInvert = bitRead(Touch_Settings.flags, TOUCH_FLAGS_PGUP_BELOW_MENU);

  if (validButtonGroup(_buttonGroup + (pgupInvert ? 10 : -10))) {
    return setButtonGroup(event, _buttonGroup + (pgupInvert ? 10 : -10));
  }
  return false;
}

/**
 * Load the settings onto the webpage
 */
bool ESPEasy_TouchHandler::plugin_webform_load(struct EventStruct *event) {
  addFormSubHeader(F("Touch configuration"));

  addFormCheckBox(F("Flip rotation 180&deg;"), F("rotation_flipped"), bitRead(Touch_Settings.flags, TOUCH_FLAGS_ROTATION_FLIPPED));
  # ifndef LIMIT_BUILD_SIZE
  addFormNote(F("Some touchscreens are mounted 180&deg; rotated on the display."));
  # endif // ifndef LIMIT_BUILD_SIZE

  uint8_t choice3 = 0u;

  bitWrite(choice3, TOUCH_FLAGS_SEND_XY,         bitRead(Touch_Settings.flags, TOUCH_FLAGS_SEND_XY));
  bitWrite(choice3, TOUCH_FLAGS_SEND_Z,          bitRead(Touch_Settings.flags, TOUCH_FLAGS_SEND_Z));
  bitWrite(choice3, TOUCH_FLAGS_SEND_OBJECTNAME, bitRead(Touch_Settings.flags, TOUCH_FLAGS_SEND_OBJECTNAME));
  {
    # define TOUCH_EVENTS_OPTIONS 6
    const __FlashStringHelper *options3[TOUCH_EVENTS_OPTIONS] =
    { F("None"),
      F("X and Y"),
      F("X, Y and Z"),
      # ifdef TOUCH_USE_EXTENDED_TOUCH
      F("Objectnames and Button groups"),
      F("Objectnames, Button groups, X and Y"),
      F("Objectnames, Button groups, X, Y and Z")
      # else // ifdef TOUCH_USE_EXTENDED_TOUCH
      F("Objectnames only"),
      F("Objectnames, X and Y"),
      F("Objectnames, X, Y and Z")
      # endif // ifdef TOUCH_USE_EXTENDED_TOUCH
    };
    const int optionValues3[TOUCH_EVENTS_OPTIONS] = { 0, 1, 3, 4, 5, 7 }; // Already used as a bitmap!
    addFormSelector(F("Events"), F("events"), TOUCH_EVENTS_OPTIONS, options3, optionValues3, choice3);

    addFormCheckBox(F("Draw buttons when started"), F("init_objectevent"), bitRead(Touch_Settings.flags, TOUCH_FLAGS_INIT_OBJECTEVENT));
    # ifndef LIMIT_BUILD_SIZE
    addFormNote(F("Needs Objectnames 'Events' to be enabled."));
    # endif // ifndef LIMIT_BUILD_SIZE
  }

  addFormCheckBox(F("Prevent duplicate events"), F("deduplicate"), bitRead(Touch_Settings.flags, TOUCH_FLAGS_DEDUPLICATE));

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
                    F("use_calibration"),
                    2,
                    noYesOptions,
                    noYesOptionValues,
                    Touch_Settings.calibrationEnabled ? 1 : 0,
                    true);
  }

  if (Touch_Settings.calibrationEnabled) {
    addRowLabel(F("Calibration"));
    html_table(EMPTY_STRING, false); // Sub-table
    html_table_header(F(""));
    html_table_header(F("x"));
    html_table_header(F("y"));
    html_table_header(F(""));
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

  addFormCheckBox(F("Enable logging for calibration"), F("log_calibration"),
                  Touch_Settings.logEnabled);

  addFormSubHeader(F("Touch objects"));

  # ifdef TOUCH_USE_EXTENDED_TOUCH

  AdaGFXHtmlColorDepthDataList(F("adagfx65kcolors"), _colorDepth);

  {
    String parsed;
    addRowLabel(F("Default On/Off button colors"));
    html_table(EMPTY_STRING, false); // Sub-table
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
               #  ifdef TOUCH_USE_TOOLTIPS
               , F("ON color")
               #  endif // ifdef TOUCH_USE_TOOLTIPS
               , F("adagfx65kcolors")
               );
    html_TD(); // OFF color
    parsed = AdaGFXcolorToString(Touch_Settings.colorOff, _colorDepth, true);
    addTextBox(getPluginCustomArgName(3001), parsed, TOUCH_MAX_COLOR_INPUTLENGTH, false, false,
               EMPTY_STRING, F("widenumber")
               #  ifdef TOUCH_USE_TOOLTIPS
               , F("OFF color")
               #  endif // ifdef TOUCH_USE_TOOLTIPS
               , F("adagfx65kcolors")
               );
    html_TD(); // Border color
    parsed = AdaGFXcolorToString(Touch_Settings.colorBorder, _colorDepth, true);
    addTextBox(getPluginCustomArgName(3002), parsed, TOUCH_MAX_COLOR_INPUTLENGTH, false, false,
               EMPTY_STRING, F("widenumber")
               #  ifdef TOUCH_USE_TOOLTIPS
               , F("Border color")
               #  endif // ifdef TOUCH_USE_TOOLTIPS
               , F("adagfx65kcolors")
               );
    html_TD(); // Caption color
    parsed = AdaGFXcolorToString(Touch_Settings.colorCaption, _colorDepth, true);
    addTextBox(getPluginCustomArgName(3003), parsed, TOUCH_MAX_COLOR_INPUTLENGTH, false, false,
               EMPTY_STRING, F("widenumber")
               #  ifdef TOUCH_USE_TOOLTIPS
               , F("Caption color")
               #  endif // ifdef TOUCH_USE_TOOLTIPS
               , F("adagfx65kcolors")
               );
    html_TD(); // Disabled color
    parsed = AdaGFXcolorToString(Touch_Settings.colorDisabled, _colorDepth, true);
    addTextBox(getPluginCustomArgName(3004), parsed, TOUCH_MAX_COLOR_INPUTLENGTH, false, false,
               EMPTY_STRING, F("widenumber")
               #  ifdef TOUCH_USE_TOOLTIPS
               , F("Disabled color")
               #  endif // ifdef TOUCH_USE_TOOLTIPS
               , F("adagfx65kcolors")
               );
    html_TD(); // Disabled caption color
    parsed = AdaGFXcolorToString(Touch_Settings.colorDisabledCaption, _colorDepth, true);
    addTextBox(getPluginCustomArgName(3005), parsed, TOUCH_MAX_COLOR_INPUTLENGTH, false, false,
               EMPTY_STRING, F("widenumber")
               #  ifdef TOUCH_USE_TOOLTIPS
               , F("Disabled caption color")
               #  endif // ifdef TOUCH_USE_TOOLTIPS
               , F("adagfx65kcolors")
               );
    html_end_table();
  }
  {
    addFormNumericBox(F("Initial button group"), F("initial_group"),
                      get8BitFromUL(Touch_Settings.flags, TOUCH_FLAGS_INITIAL_GROUP), 0, TOUCH_MAX_BUTTON_GROUPS
                      #  ifdef TOUCH_USE_TOOLTIPS
                      , F("Initial group")
                      #  endif // ifdef TOUCH_USE_TOOLTIPS
                      );
    addFormCheckBox(F("Draw buttons via Rules"),      F("via_rules"),    bitRead(Touch_Settings.flags, TOUCH_FLAGS_DRAWBTN_VIA_RULES));
    addFormCheckBox(F("Enable/Disable page buttons"), F("page_buttons"), bitRead(Touch_Settings.flags, TOUCH_FLAGS_AUTO_PAGE_ARROWS));
    addFormCheckBox(F("PageUp/PageDown reversed"),    F("page_below"),   bitRead(Touch_Settings.flags, TOUCH_FLAGS_PGUP_BELOW_MENU));
  }
  # endif // ifdef TOUCH_USE_EXTENDED_TOUCH
  {
    addRowLabel(F("Object"));

    {
      html_table(EMPTY_STRING, false); // Sub-table
      html_table_header(F("&nbsp;#&nbsp;"));
      html_table_header(F("On"));
      html_table_header(F("Objectname"));
      html_table_header(F("Top-left x"));
      html_table_header(F("Top-left y"));
      # ifdef TOUCH_USE_EXTENDED_TOUCH
      html_table_header(F("Button"));
      html_table_header(F("Layout"));
      html_table_header(F("ON color"));
      html_table_header(F("ON caption"));
      html_table_header(F("Border color"));
      html_table_header(F("Disab. cap. clr"));
      html_table_header(F("Touch action"));
      # else // ifdef TOUCH_USE_EXTENDED_TOUCH
      html_table_header(F("On/Off button"));
      # endif // ifdef TOUCH_USE_EXTENDED_TOUCH
      html_TR(); // New row
      html_table_header(EMPTY_STRING);
      html_table_header(EMPTY_STRING);
      # ifdef TOUCH_USE_EXTENDED_TOUCH
      html_table_header(F("Button-group"));
      # else // ifdef TOUCH_USE_EXTENDED_TOUCH
      html_table_header(EMPTY_STRING);
      # endif // ifdef TOUCH_USE_EXTENDED_TOUCH
      html_table_header(F("Width"));
      html_table_header(F("Height"));
      html_table_header(F("Inverted"));
      # ifdef TOUCH_USE_EXTENDED_TOUCH
      html_table_header(F("Font scale"));
      html_table_header(F("OFF color"));
      html_table_header(F("OFF caption"));
      html_table_header(F("Caption color"));
      html_table_header(F("Disabled clr"));
      html_table_header(F("Action group"));
      # endif // ifdef TOUCH_USE_EXTENDED_TOUCH
    }
    # ifdef TOUCH_USE_EXTENDED_TOUCH
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
      toString(Button_layout_e::Bitmap),
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
      static_cast<int>(Button_layout_e::Bitmap),
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

    # endif // ifdef TOUCH_USE_EXTENDED_TOUCH

    uint8_t maxIdx = std::min(static_cast<int>(TouchObjects.size() + TOUCH_EXTRA_OBJECT_COUNT), TOUCH_MAX_OBJECT_COUNT);
    String  parsed;
    TouchObjects.resize(maxIdx, tTouchObjects());

    for (int objectNr = 0; objectNr < maxIdx; objectNr++) {
      html_TR_TD();
      addHtml(F("&nbsp;"));
      addHtmlInt(objectNr + 1); // Arrayindex to objectindex

      html_TD();

      // Enable new entries
      bool enabled = bitRead(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_ENABLED) || TouchObjects[objectNr].objectName.isEmpty();
      addCheckBox(getPluginCustomArgName(objectNr + 0),
                  enabled, false
                  # ifdef TOUCH_USE_TOOLTIPS
                  , F("Enabled")
                  # endif // ifdef TOUCH_USE_TOOLTIPS
                  );
      html_TD(); // Name
      addTextBox(getPluginCustomArgName(objectNr + 100),
                 TouchObjects[objectNr].objectName,
                 TOUCH_MaxObjectNameLength,
                 false, false, EMPTY_STRING, EMPTY_STRING);
      html_TD(); // top-x
      addNumericBox(getPluginCustomArgName(objectNr + 200),
                    TouchObjects[objectNr].top_left.x, 0, 65535
                    # ifdef TOUCH_USE_TOOLTIPS
                    , F("widenumber"), F("Top-left x")
                    # endif // ifdef TOUCH_USE_TOOLTIPS
                    );
      html_TD(); // top-y
      addNumericBox(getPluginCustomArgName(objectNr + 300),
                    TouchObjects[objectNr].top_left.y, 0, 65535
                    # ifdef TOUCH_USE_TOOLTIPS
                    , F("widenumber"), F("Top-left y")
                    # endif // ifdef TOUCH_USE_TOOLTIPS
                    );
      html_TD(); // (on/off) button (type)
      # ifdef TOUCH_USE_EXTENDED_TOUCH
      addSelector(getPluginCustomArgName(objectNr + 800),
                  static_cast<int>(Button_type_e::Button_MAX),
                  buttonTypeOptions,
                  buttonTypeValues,
                  nullptr,
                  get4BitFromUL(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_BUTTONTYPE), false, true, F("widenumber")
                  #  ifdef TOUCH_USE_TOOLTIPS
                  , F("Buttontype")
                  #  endif // ifdef TOUCH_USE_TOOLTIPS
                  );
      html_TD(); // button alignment
      addSelector(getPluginCustomArgName(objectNr + 900),
                  static_cast<int>(Button_layout_e::Alignment_MAX),
                  buttonLayoutOptions,
                  buttonLayoutValues,
                  nullptr,
                  get4BitFromUL(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_BUTTONALIGN) << 4, false, true, F("widenumber")
                  #  ifdef TOUCH_USE_TOOLTIPS
                  , F("Button alignment")
                  #  endif // ifdef TOUCH_USE_TOOLTIPS
                  );
      # else // ifdef TOUCH_USE_EXTENDED_TOUCH
      addCheckBox(getPluginCustomArgName(objectNr + 600),
                  bitRead(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_BUTTON), false
                  #  ifdef TOUCH_USE_TOOLTIPS
                  , F("On/Off button")
                  #  endif // ifdef TOUCH_USE_TOOLTIPS
                  );
      # endif // ifdef TOUCH_USE_EXTENDED_TOUCH
      # ifdef TOUCH_USE_EXTENDED_TOUCH
      html_TD(); // ON color
      parsed = AdaGFXcolorToString(TouchObjects[objectNr].colorOn, _colorDepth, true);
      addTextBox(getPluginCustomArgName(objectNr + 1000), parsed, TOUCH_MAX_COLOR_INPUTLENGTH, false, false,
                 EMPTY_STRING, F("widenumber")
                 #  ifdef TOUCH_USE_TOOLTIPS
                 , F("ON color")
                 #  endif // ifdef TOUCH_USE_TOOLTIPS
                 , F("adagfx65kcolors")
                 );
      html_TD(); // ON Caption
      addTextBox(getPluginCustomArgName(objectNr + 1300),
                 TouchObjects[objectNr].captionOn,
                 TOUCH_MaxCaptionNameLength,
                 false,
                 false,
                 EMPTY_STRING,
                 F("wide")
                 #  ifdef TOUCH_USE_TOOLTIPS
                 , F("ON caption")
                 #  endif // ifdef TOUCH_USE_TOOLTIPS
                 );
      html_TD(); // Border color
      parsed = AdaGFXcolorToString(TouchObjects[objectNr].colorBorder, _colorDepth, true);
      addTextBox(getPluginCustomArgName(objectNr + 1700), parsed, TOUCH_MAX_COLOR_INPUTLENGTH, false, false,
                 EMPTY_STRING, F("widenumber")
                 #  ifdef TOUCH_USE_TOOLTIPS
                 , F("Border color")
                 #  endif // ifdef TOUCH_USE_TOOLTIPS
                 , F("adagfx65kcolors")
                 );
      html_TD(); // Disabled caption color
      parsed = AdaGFXcolorToString(TouchObjects[objectNr].colorDisabledCaption, _colorDepth, true);
      addTextBox(getPluginCustomArgName(objectNr + 1900), parsed, TOUCH_MAX_COLOR_INPUTLENGTH, false, false,
                 EMPTY_STRING, F("widenumber")
                 #  ifdef TOUCH_USE_TOOLTIPS
                 , F("Disabled caption color")
                 #  endif // ifdef TOUCH_USE_TOOLTIPS
                 , F("adagfx65kcolors")
                 );
      html_TD(); // button action
      addSelector(getPluginCustomArgName(objectNr + 2000),
                  static_cast<int>(Touch_action_e::TouchAction_MAX),
                  touchActionOptions,
                  touchActionValues,
                  nullptr,
                  get4BitFromUL(TouchObjects[objectNr].groupFlags, TOUCH_OBJECT_GROUP_ACTION),
                  false,
                  true,
                  F("widenumber")
                  #  ifdef TOUCH_USE_TOOLTIPS
                  , F("Touch action")
                  #  endif // ifdef TOUCH_USE_TOOLTIPS
                  );
      # endif // ifdef TOUCH_USE_EXTENDED_TOUCH

      html_TR_TD(); // Start new row

      html_TD(2);   // Start with some blank columns
      # ifdef TOUCH_USE_EXTENDED_TOUCH
      {
        #  ifdef TOUCH_USE_TOOLTIPS
        String buttonGroupToolTip = F("Button-group [0..");
        buttonGroupToolTip += TOUCH_MAX_BUTTON_GROUPS;
        buttonGroupToolTip += ']';
        #  endif // ifdef TOUCH_USE_TOOLTIPS
        addNumericBox(getPluginCustomArgName(objectNr + 1600),
                      get8BitFromUL(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_GROUP), 0, TOUCH_MAX_BUTTON_GROUPS
                      #  ifdef TOUCH_USE_TOOLTIPS
                      , F("widenumber"), buttonGroupToolTip
                      #  endif // ifdef TOUCH_USE_TOOLTIPS
                      );
      }
      # endif // ifdef TOUCH_USE_EXTENDED_TOUCH
      html_TD(); // Width
      addNumericBox(getPluginCustomArgName(objectNr + 400),
                    TouchObjects[objectNr].width_height.x, 0, 65535
                    # ifdef TOUCH_USE_TOOLTIPS
                    , F("widenumber"), F("Width")
                    # endif // ifdef TOUCH_USE_TOOLTIPS
                    );
      html_TD(); // Height
      addNumericBox(getPluginCustomArgName(objectNr + 500),
                    TouchObjects[objectNr].width_height.y, 0, 65535
                    # ifdef TOUCH_USE_TOOLTIPS
                    , F("widenumber"), F("Height")
                    # endif // ifdef TOUCH_USE_TOOLTIPS
                    );
      html_TD(); // inverted
      addCheckBox(getPluginCustomArgName(objectNr + 700),
                  bitRead(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_INVERTED), false
                  # ifdef TOUCH_USE_TOOLTIPS
                  , F("Inverted")
                  # endif // ifdef TOUCH_USE_TOOLTIPS
                  );
      # ifdef TOUCH_USE_EXTENDED_TOUCH
      html_TD(); // font scale
      addNumericBox(getPluginCustomArgName(objectNr + 1200),
                    get4BitFromUL(TouchObjects[objectNr].flags, TOUCH_OBJECT_FLAG_FONTSCALE), 1, 10
                    #  ifdef TOUCH_USE_TOOLTIPS
                    , F("widenumber"), F("Font scaling [1x..10x]")
                    #  endif // ifdef TOUCH_USE_TOOLTIPS
                    );
      html_TD(); // OFF color
      parsed = AdaGFXcolorToString(TouchObjects[objectNr].colorOff, _colorDepth, true);
      addTextBox(getPluginCustomArgName(objectNr + 1100), parsed, TOUCH_MAX_COLOR_INPUTLENGTH, false, false,
                 EMPTY_STRING, F("widenumber")
                 #  ifdef TOUCH_USE_TOOLTIPS
                 , F("OFF color")
                 #  endif // ifdef TOUCH_USE_TOOLTIPS
                 , F("adagfx65kcolors")
                 );
      html_TD(); // OFF Caption
      addTextBox(getPluginCustomArgName(objectNr + 1400),
                 TouchObjects[objectNr].captionOff,
                 TOUCH_MaxCaptionNameLength,
                 false,
                 false,
                 EMPTY_STRING,
                 F("wide")
                 #  ifdef TOUCH_USE_TOOLTIPS
                 , F("OFF caption")
                 #  endif // ifdef TOUCH_USE_TOOLTIPS
                 );
      html_TD(); // Caption color
      parsed = AdaGFXcolorToString(TouchObjects[objectNr].colorCaption, _colorDepth, true);
      addTextBox(getPluginCustomArgName(objectNr + 1500), parsed, TOUCH_MAX_COLOR_INPUTLENGTH, false, false,
                 EMPTY_STRING, F("widenumber")
                 #  ifdef TOUCH_USE_TOOLTIPS
                 , F("Caption color")
                 #  endif // ifdef TOUCH_USE_TOOLTIPS
                 , F("adagfx65kcolors")
                 );
      html_TD(); // Disabled color
      parsed = AdaGFXcolorToString(TouchObjects[objectNr].colorDisabled, _colorDepth, true);
      addTextBox(getPluginCustomArgName(objectNr + 1800), parsed, TOUCH_MAX_COLOR_INPUTLENGTH, false, false,
                 EMPTY_STRING, F("widenumber")
                 #  ifdef TOUCH_USE_TOOLTIPS
                 , F("Disabled color")
                 #  endif // ifdef TOUCH_USE_TOOLTIPS
                 , F("adagfx65kcolors")
                 );
      html_TD(); // Action Group
      addNumericBox(getPluginCustomArgName(objectNr + 2100),
                    get8BitFromUL(TouchObjects[objectNr].groupFlags, TOUCH_OBJECT_GROUP_ACTIONGROUP), 0, TOUCH_MAX_BUTTON_GROUPS
                    #  ifdef TOUCH_USE_TOOLTIPS
                    , F("widenumber")
                    , F("Action group")
                    #  endif // ifdef TOUCH_USE_TOOLTIPS
                    );
      # endif // ifdef TOUCH_USE_EXTENDED_TOUCH
    }
    html_end_table();

    addFormNumericBox(F("Debounce delay for On/Off buttons"), F("debounce"),
                      Touch_Settings.debounceMs, 0, 255);
    addUnit(F("0-255 msec."));
  }
  return false;
}

/**
 * Helper: Convert an integer to string, but return an empty string for 0, to save a little space in settings
 */
String toStringNoZero(int64_t value) {
  if (value != 0) {
    return toString(value, 0);
  } else {
    return EMPTY_STRING;
  }
}

/**
 * Save the settings from the web page to flash
 */
bool ESPEasy_TouchHandler::plugin_webform_save(struct EventStruct *event) {
  String config;

  # ifdef TOUCH_DEBUG
  uint16_t saveSize = 0;
  # endif // ifdef TOUCH_DEBUG

  # ifdef TOUCH_USE_EXTENDED_TOUCH
  String colorInput;
  # endif // ifdef TOUCH_USE_EXTENDED_TOUCH
  config.reserve(80);

  uint32_t  lSettings   = 0u;
  const int eventsValue = getFormItemInt(F("events"));

  bitWrite(lSettings, TOUCH_FLAGS_SEND_XY,          bitRead(eventsValue, TOUCH_FLAGS_SEND_XY));
  bitWrite(lSettings, TOUCH_FLAGS_SEND_Z,           bitRead(eventsValue, TOUCH_FLAGS_SEND_Z));
  bitWrite(lSettings, TOUCH_FLAGS_SEND_OBJECTNAME,  bitRead(eventsValue, TOUCH_FLAGS_SEND_OBJECTNAME));
  bitWrite(lSettings, TOUCH_FLAGS_ROTATION_FLIPPED, isFormItemChecked(F("rotation_flipped")));
  bitWrite(lSettings, TOUCH_FLAGS_DEDUPLICATE,      isFormItemChecked(F("deduplicate")));
  bitWrite(lSettings, TOUCH_FLAGS_INIT_OBJECTEVENT, isFormItemChecked(F("init_objectevent")));
  # ifdef TOUCH_USE_EXTENDED_TOUCH
  set8BitToUL(lSettings, TOUCH_FLAGS_INITIAL_GROUP, getFormItemInt(F("initial_group"))); // Button group
  bitWrite(lSettings, TOUCH_FLAGS_DRAWBTN_VIA_RULES, isFormItemChecked(F("via_rules")));
  bitWrite(lSettings, TOUCH_FLAGS_AUTO_PAGE_ARROWS,  isFormItemChecked(F("page_buttons")));
  bitWrite(lSettings, TOUCH_FLAGS_PGUP_BELOW_MENU,   isFormItemChecked(F("page_below")));
  # endif // ifdef TOUCH_USE_EXTENDED_TOUCH

  config += getFormItemInt(F("use_calibration")); // First value should NEVER be empty, or parseString() wil get confused
  config += TOUCH_SETTINGS_SEPARATOR;
  config += toStringNoZero(isFormItemChecked(F("log_calibration")) ? 1 : 0);
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
  # ifdef TOUCH_USE_EXTENDED_TOUCH
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
  # endif // ifdef TOUCH_USE_EXTENDED_TOUCH

  settingsArray[TOUCH_CALIBRATION_START] = config;
  # ifdef TOUCH_DEBUG
  saveSize += config.length() + 1;
  # endif // ifdef TOUCH_DEBUG

  # ifdef TOUCH_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("Save settings: ");
    config.replace(TOUCH_SETTINGS_SEPARATOR, ',');
    log += config;
    addLogMove(LOG_LEVEL_INFO, log);
  }
  # endif // ifdef TOUCH_DEBUG

  String error;

  for (int objectNr = 0; objectNr < TOUCH_MAX_OBJECT_COUNT; objectNr++) {
    config.clear();
    config += webArg(getPluginCustomArgName(objectNr + 100)); // Name

    if (!config.isEmpty()) {                                  // Empty name => skip entry
      bool numStart = (config[0] >= '0' && config[0] <= '9'); // Numeric start?

      if (!ExtraTaskSettings.checkInvalidCharInNames(config.c_str()) ||
          numStart) {                                         // Check for invalid characters in objectname
        error += F("Invalid character in objectname #");
        error += objectNr + 1;
        error += numStart ? F(". Should not start with a digit.\n") : F(". Do not use ',-+/*=^%!#[]{}()' or space.\n");
      }
      config += TOUCH_SETTINGS_SEPARATOR;
      uint32_t flags = 0u;
      bitWrite(flags, TOUCH_OBJECT_FLAG_ENABLED,  isFormItemChecked(getPluginCustomArgName(objectNr + 0)));   // Enabled
      bitWrite(flags, TOUCH_OBJECT_FLAG_INVERTED, isFormItemChecked(getPluginCustomArgName(objectNr + 700))); // Inverted
      # ifdef TOUCH_USE_EXTENDED_TOUCH
      uint32_t groupFlags      = 0u;
      const uint8_t buttonType = getFormItemIntCustomArgName(objectNr + 800);
      set4BitToUL(flags, TOUCH_OBJECT_FLAG_BUTTONTYPE,  buttonType);                                              // Buttontype
      set4BitToUL(flags, TOUCH_OBJECT_FLAG_BUTTONALIGN, getFormItemIntCustomArgName(objectNr + 900) >> 4);        // Button layout
      bitWrite(flags, TOUCH_OBJECT_FLAG_BUTTON, (static_cast<Button_type_e>(buttonType) != Button_type_e::None)); // On/Off button
      set4BitToUL(groupFlags, TOUCH_OBJECT_GROUP_ACTION, getFormItemIntCustomArgName(objectNr + 2000));           // ButtonAction
      set8BitToUL(groupFlags, TOUCH_OBJECT_GROUP_ACTIONGROUP, getFormItemIntCustomArgName(objectNr + 2100));      // ActionGroup
      set4BitToUL(flags, TOUCH_OBJECT_FLAG_FONTSCALE, getFormItemIntCustomArgName(objectNr + 1200));              // Font scaling
      set8BitToUL(flags, TOUCH_OBJECT_FLAG_GROUP, getFormItemIntCustomArgName(objectNr + 1600));                  // Button group
      # else // ifdef TOUCH_USE_EXTENDED_TOUCH
      bitWrite(flags, TOUCH_OBJECT_FLAG_BUTTON, isFormItemChecked(getPluginCustomArgName(objectNr + 600)));       // On/Off button
      # endif // ifdef TOUCH_USE_EXTENDED_TOUCH

      config += ull2String(flags);                                                                                // Flags
      config += TOUCH_SETTINGS_SEPARATOR;
      config += toStringNoZero(getFormItemIntCustomArgName(objectNr + 200));                                      // Top x
      config += TOUCH_SETTINGS_SEPARATOR;
      config += toStringNoZero(getFormItemIntCustomArgName(objectNr + 300));                                      // Top y
      config += TOUCH_SETTINGS_SEPARATOR;
      config += toStringNoZero(getFormItemIntCustomArgName(objectNr + 400));                                      // Bottom x
      config += TOUCH_SETTINGS_SEPARATOR;
      config += toStringNoZero(getFormItemIntCustomArgName(objectNr + 500));                                      // Bottom y

      # ifdef TOUCH_USE_EXTENDED_TOUCH
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
      config    += wrapWithQuotesIfContainsParameterSeparatorChar(webArg(getPluginCustomArgName(objectNr + 1300)));
      config    += TOUCH_SETTINGS_SEPARATOR;                        // Caption OFF
      config    += wrapWithQuotesIfContainsParameterSeparatorChar(webArg(getPluginCustomArgName(objectNr + 1400)));
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
      # endif // ifdef TOUCH_USE_EXTENDED_TOUCH
    }
    config.trim();

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
    # ifdef TOUCH_DEBUG
    saveSize += config.length() + 1;
    # endif // ifdef TOUCH_DEBUG

    # ifdef TOUCH_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO) &&
        !config.isEmpty()) {
      String log = F("Save object #");
      log += objectNr;
      log += F(" settings: ");
      config.replace(TOUCH_SETTINGS_SEPARATOR, ',');
      log += config;
      addLogMove(LOG_LEVEL_INFO, log);
    }
    # endif // ifdef TOUCH_DEBUG
  }

  if (!error.isEmpty()) {
    addLog(LOG_LEVEL_ERROR, error);
    addHtmlError(error);
  }

  error = SaveCustomTaskSettings(event->TaskIndex, settingsArray, TOUCH_ARRAY_SIZE, 0);

  # ifdef TOUCH_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("TOUCH Save settings size: ");
    log += saveSize;
    addLogMove(LOG_LEVEL_INFO, log);
  }
  # endif // ifdef TOUCH_DEBUG

  if (!error.isEmpty()) {
    addLog(LOG_LEVEL_ERROR, error);
    addHtmlError(error);
    return false;
  }
  return true;
}

/**
 * Every 10th second we check if the screen is touched
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

  // Avoid event-storms by deduplicating coordinates
  if (!_deduplicate ||
      (_deduplicate && ((TOUCH_VALUE_X != x) || (TOUCH_VALUE_Y != y) || (TOUCH_VALUE_Z != z)))) {
    success       = true;
    TOUCH_VALUE_X = x;
    TOUCH_VALUE_Y = y;
    TOUCH_VALUE_Z = z;
  }

  if (success &&
      Touch_Settings.logEnabled &&
      loglevelActiveFor(LOG_LEVEL_INFO)) { // REQUIRED for calibration and setting up objects, so do not make this optional!
    String log;
    log.reserve(72);
    log  = F("Touch calibration rx= ");    // Space before the logged values added for readability
    log += rx;
    log += F(", ry= ");
    log += ry;
    log += F("; z= "); // Always log the z value even if not used.
    log += z;
    log += F(", x= ");
    log += x;
    log += F(", y= ");
    log += y;
    log += F("; ox= ");
    log += ox;
    log += F(", oy= ");
    log += oy;
    addLogMove(LOG_LEVEL_INFO, log);
  }

  // No events to handle if rules not enabled
  if (Settings.UseRules) {
    if (success && bitRead(Touch_Settings.flags, TOUCH_FLAGS_SEND_XY)) { // Send events for each touch
      const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(event->TaskIndex);

      // Do NOT send a Z event for each touch?
      if (!bitRead(Touch_Settings.flags, TOUCH_FLAGS_SEND_Z) && validDeviceIndex(DeviceIndex)) {
        Device[DeviceIndex].VType      = Sensor_VType::SENSOR_TYPE_DUAL;
        Device[DeviceIndex].ValueCount = 2;
      }
      sendData(event);                                                                           // Send X/Y(/Z) event

      if (!bitRead(Touch_Settings.flags, TOUCH_FLAGS_SEND_Z) && validDeviceIndex(DeviceIndex)) { // Reset device configuration
        Device[DeviceIndex].VType      = Sensor_VType::SENSOR_TYPE_TRIPLE;
        Device[DeviceIndex].ValueCount = 3;
      }
    }

    if (bitRead(Touch_Settings.flags, TOUCH_FLAGS_SEND_OBJECTNAME)) { // Send events for objectname if within reach
      String selectedObjectName;
      int8_t selectedObjectIndex = -1;

      if (isValidAndTouchedTouchObject(x, y, selectedObjectName, selectedObjectIndex)) {
        // Not touched yet or too long ago
        if ((TouchObjects[selectedObjectIndex].TouchTimers == 0) ||
            (TouchObjects[selectedObjectIndex].TouchTimers < (millis() - (1.5 * Touch_Settings.debounceMs)))) {
          // From now wait the debounce time
          TouchObjects[selectedObjectIndex].TouchTimers = millis() + Touch_Settings.debounceMs;
        } else {
          // Debouncing time elapsed?
          if (TouchObjects[selectedObjectIndex].TouchTimers <= millis()) {
            TouchObjects[selectedObjectIndex].TouchTimers = 0;

            if ((selectedObjectIndex > -1) && bitRead(TouchObjects[selectedObjectIndex].flags, TOUCH_OBJECT_FLAG_BUTTON)) {
              TouchObjects[selectedObjectIndex].TouchStates = !TouchObjects[selectedObjectIndex].TouchStates;
              generateObjectEvent(event, selectedObjectIndex, TouchObjects[selectedObjectIndex].TouchStates ? 1 : 0);
            } else {
              // Matching object is found, send <TaskDeviceName>#<ObjectName> event with x, y and z as %eventvalue1/2/3%
              String eventCommand;
              eventCommand.reserve(48);
              eventCommand  = getTaskDeviceName(event->TaskIndex);
              eventCommand += '#';
              eventCommand += selectedObjectName;
              eventCommand += '='; // Add arguments
              eventCommand += x;
              eventCommand += ',';
              eventCommand += y;
              eventCommand += ',';
              eventCommand += z;
              eventQueue.addMove(std::move(eventCommand));
            }
          }
        }
      }
    }
  }
  return success;
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

  arguments.reserve(24);
  command    = parseString(string, 1);
  subcommand = parseString(string, 2);

  if (command.equals(F("touch"))) {
    # ifdef TOUCH_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("TOUCH PLUGIN_WRITE arguments Par1:");
      log += event->Par1;
      log += F(", 2: ");
      log += event->Par2;
      log += F(", 3: ");
      log += event->Par3;
      log += F(", 4: ");
      log += event->Par4;
      log += F(", string: ");
      log += string;
      addLog(LOG_LEVEL_INFO, log);
    }
    # endif // ifdef TOUCH_DEBUG

    if (subcommand.equals(F("enable"))) { // touch,enable,<objectName>[,...] : Enable disabled objectname(s)
      arguments = parseString(string, arg);

      while (!arguments.isEmpty()) {
        success |= setTouchObjectState(event, arguments, true);
        arg++;
        arguments = parseString(string, arg);
      }
    } else if (subcommand.equals(F("disable"))) { // touch,disable,<objectName>[,...] : Disable enabled objectname(s)
      arguments = parseString(string, arg);

      while (!arguments.isEmpty()) {
        success |= setTouchObjectState(event, arguments, false);
        arg++;
        arguments = parseString(string, arg);
      }
    } else if (subcommand.equals(F("on"))) { // touch,on,<buttonObjectName>[,...] : Switch TouchButton(s) on
      arguments = parseString(string, arg);

      while (!arguments.isEmpty()) {
        success |= setTouchButtonOnOff(event, arguments, true);
        arg++;
        arguments = parseString(string, arg);
      }
    } else if (subcommand.equals(F("off"))) { // touch,off,<buttonObjectName>[,...] : Switch TouchButton(s) off
      arguments = parseString(string, arg);

      while (!arguments.isEmpty()) {
        success |= setTouchButtonOnOff(event, arguments, false);
        arg++;
        arguments = parseString(string, arg);
      }
    } else if (subcommand.equals(F("toggle"))) { // touch,toggle,<buttonObjectName>[,...] : Switch TouchButton(s) to the other state
      arguments = parseString(string, arg);

      while (!arguments.isEmpty()) {
        int8_t state = getTouchButtonOnOff(event, arguments);

        if (state > -1) {
          success |= setTouchButtonOnOff(event, arguments, state == 0);
        }
        arg++;
        arguments = parseString(string, arg);
      }
    } else if (subcommand.equals(F("setgrp"))) {       // touch,setgrp,<group> : Activate button group
      success = setButtonGroup(event, event->Par2);
    } else if (subcommand.equals(F("incgrp"))) {       // touch,incgrp : increment group and Activate
      success = incrementButtonGroup(event);
    } else if (subcommand.equals(F("decgrp"))) {       // touch,decgrp : Decrement group and Activate
      success = decrementButtonGroup(event);
    } else if (subcommand.equals(F("incpage"))) {      // touch,incpage : increment page and Activate
      success = incrementButtonPage(event);
    } else if (subcommand.equals(F("decpage"))) {      // touch,decpage : Decrement page and Activate
      success = decrementButtonPage(event);
    } else if (subcommand.equals(F("updatebutton"))) { // touch,updatebutton,<buttonnr>[,<group>[,<mode>]] : Update a button
      arguments = parseString(string, 3);

      // Check for a valid button name or number, returns a 0-based index
      int index = getTouchObjectIndex(event, arguments, true);

      if (index > -1) {
        bool hasPar3 = !parseString(string, 4).isEmpty();
        bool hasPar4 = !parseString(string, 5).isEmpty();

        if (hasPar4) {
          success = displayButton(event, index, event->Par3, event->Par4);
        } else if (hasPar3) {
          success = displayButton(event, index, event->Par3);
        } else {
          success = displayButton(event, index); // Use default argument values
        }
      }
    }
  }
  return success;
}

/**
 * Handle getting config values from plugin/handler
 */
bool ESPEasy_TouchHandler::plugin_get_config_value(struct EventStruct *event,
                                                   String            & string) {
  bool   success = false;
  String command = parseString(string, 1);

  if (command == F("buttongroup")) {
    string  = getButtonGroup();
    success = true;
  } else if (command == F("hasgroup")) {
    int group; // We'll be ignoring group 0 if there are multiple button groups

    if (validIntFromString(parseString(string, 2), group)) {
      string  = validButtonGroup(group, true) ? 1 : 0;
      success = true;
    } else {
      string = '0'; // invalid number = false
    }
  } else if (command == F("enabled")) {
    String arguments = parseStringKeepCase(string, 2);
    int8_t enabled   = getTouchObjectState(event, arguments);

    if (enabled > -1) {
      string  = enabled;
      success = true;
    }
  } else if (command == F("state")) {
    String arguments = parseStringKeepCase(string, 2);
    int8_t state     = getTouchButtonOnOff(event, arguments);

    if (state > -1) {
      string  = state;
      success = true;
    }
  }
  return success;
}

/**
 * generate an event for a touch object
 * When a display is configured add x,y coordinate, width,height of the object, objectIndex, and TaskIndex of display
 **************************************************************************/
void ESPEasy_TouchHandler::generateObjectEvent(struct EventStruct *event,
                                               const int8_t      & objectIndex,
                                               const int8_t      & onOffState,
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

  extraCommand += getTaskDeviceName(event->TaskIndex);
  extraCommand += '#';
  extraCommand += TouchObjects[objectIndex].objectName;
  extraCommand += '='; // Add arguments: (%eventvalue#%)

  if (bitRead(Touch_Settings.flags, TOUCH_FLAGS_DRAWBTN_VIA_RULES)) {
    eventCommand = extraCommand;
  } else {                                        // Handle via direct btn commands
    if (_displayTask != event->TaskIndex) {       // Add arguments for display
      eventCommand += '[';
      eventCommand += _displayTask + 1;
      eventCommand += F("].adagfx_trigger,btn,"); // Internal command trigger
    } else {
      addLog(LOG_LEVEL_ERROR, F("TOUCH: No valid Display task selected."));
      return;
    }
  }

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
  eventCommand += ',';
  eventCommand += mode;                                       // (2 = mode)
  extraCommand += ',';
  extraCommand += mode;                                       // (2 = mode) // duplicate

  if (_displayTask != event->TaskIndex) {                     // Add arguments for display
    eventCommand += ',';
    eventCommand += TouchObjects[objectIndex].top_left.x;     // (3 = x)
    eventCommand += ',';
    eventCommand += TouchObjects[objectIndex].top_left.y;     // (4 = y)
    eventCommand += ',';
    eventCommand += TouchObjects[objectIndex].width_height.x; // (5 = width)
    eventCommand += ',';
    eventCommand += TouchObjects[objectIndex].width_height.y; // (6 = height)
    eventCommand += ',';
    eventCommand += objectIndex + 1;                          // Adjust to displayed index (7 = id)
    eventCommand += ',';                                      // (8 = type + layout, 4+4 bit, side by side)
    eventCommand += get8BitFromUL(TouchObjects[objectIndex].flags, TOUCH_OBJECT_FLAG_BUTTONTYPE) * factor;
    # ifdef TOUCH_USE_EXTENDED_TOUCH
    eventCommand += ',';                                      // (9 = ON color)
    eventCommand += AdaGFXcolorToString(TouchObjects[objectIndex].colorOn == 0
                                        ? Touch_Settings.colorOn
                                        : TouchObjects[objectIndex].colorOn,
                                        _colorDepth);
    eventCommand += ','; // (10 = OFF color)
    eventCommand += AdaGFXcolorToString(TouchObjects[objectIndex].colorOff == 0
                                        ? Touch_Settings.colorOff
                                        : TouchObjects[objectIndex].colorOff,
                                        _colorDepth);
    eventCommand += ','; // (11 = Caption color)
    eventCommand += AdaGFXcolorToString(TouchObjects[objectIndex].colorCaption == 0
                                        ? Touch_Settings.colorCaption
                                        : TouchObjects[objectIndex].colorCaption,
                                        _colorDepth);
    eventCommand += ','; // (12 = Font scaling)
    eventCommand += get4BitFromUL(TouchObjects[objectIndex].flags, TOUCH_OBJECT_FLAG_FONTSCALE);
    eventCommand += ','; // (13 = ON caption, default=object name)
    eventCommand += wrapWithQuotesIfContainsParameterSeparatorChar(TouchObjects[objectIndex].captionOn.isEmpty() ?
                                                                   TouchObjects[objectIndex].objectName :
                                                                   TouchObjects[objectIndex].captionOn);
    eventCommand += ','; // (14 = OFF caption)
    eventCommand += wrapWithQuotesIfContainsParameterSeparatorChar(TouchObjects[objectIndex].captionOff.isEmpty() ?
                                                                   TouchObjects[objectIndex].objectName :
                                                                   TouchObjects[objectIndex].captionOff);
    eventCommand += ','; // (15 = Border color)
    eventCommand += AdaGFXcolorToString(TouchObjects[objectIndex].colorBorder == 0
                                        ? Touch_Settings.colorBorder
                                        : TouchObjects[objectIndex].colorBorder,
                                        _colorDepth);
    eventCommand += ','; // (16 = Disabled color)
    eventCommand += AdaGFXcolorToString(TouchObjects[objectIndex].colorDisabled == 0
                                        ? Touch_Settings.colorDisabled
                                        : TouchObjects[objectIndex].colorDisabled,
                                        _colorDepth);
    eventCommand += ','; // (17 = Disabled caption color)
    eventCommand += AdaGFXcolorToString(TouchObjects[objectIndex].colorDisabledCaption == 0
                                        ? Touch_Settings.colorDisabledCaption
                                        : TouchObjects[objectIndex].colorDisabledCaption,
                                        _colorDepth);
    # endif // ifdef TOUCH_USE_EXTENDED_TOUCH
    eventCommand += ',';
    eventCommand += _displayTask + 1; // What TaskIndex? (18) or (9)
    eventCommand += ',';              // Group (19) or (10)
    eventCommand += get8BitFromUL(TouchObjects[objectIndex].flags, TOUCH_OBJECT_FLAG_GROUP);
    # ifdef TOUCH_USE_EXTENDED_TOUCH
    eventCommand += ',';              // Group mode (20)
    uint8_t action = get4BitFromUL(TouchObjects[objectIndex].groupFlags, TOUCH_OBJECT_GROUP_ACTION);

    if (!groupSwitch && (static_cast<Touch_action_e>(action) != Touch_action_e::Default)) {
      switch (static_cast<Touch_action_e>(action)) {
        case Touch_action_e::ActivateGroup:
          eventCommand += get8BitFromUL(TouchObjects[objectIndex].groupFlags, TOUCH_OBJECT_GROUP_ACTIONGROUP);
          break;
        case Touch_action_e::IncrementGroup:
          eventCommand += -2;
          break;
        case Touch_action_e::DecrementGroup:
          eventCommand += -3;
          break;
        case Touch_action_e::IncrementPage:
          eventCommand += -4;
          break;
        case Touch_action_e::DecrementPage:
          eventCommand += -5;
          break;
        case Touch_action_e::Default:
        case Touch_action_e::TouchAction_MAX:
          eventCommand += -1; // Ignore
          break;
      }
    } else {
      eventCommand += -1; // No group to activate
    }
    # endif // ifdef TOUCH_USE_EXTENDED_TOUCH
  }

  # ifdef TOUCH_USE_EXTENDED_TOUCH

  if (bitRead(Touch_Settings.flags, TOUCH_FLAGS_DRAWBTN_VIA_RULES)) {
    eventQueue.addMove(std::move(eventCommand));
  } else {
    eventCommand += ',';
    eventCommand += wrapWithQuotesIfContainsParameterSeparatorChar(TouchObjects[objectIndex].objectName);
    ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_RULES, eventCommand.c_str()); // Simulate like from rules
    addLogMove(LOG_LEVEL_INFO, eventCommand);
    delay(0);

    // Handle group actions
    Touch_action_e action = static_cast<Touch_action_e>(get4BitFromUL(TouchObjects[objectIndex].groupFlags, TOUCH_OBJECT_GROUP_ACTION));

    if ((onOffState >= 0) && (mode >= 0)) {
      if ((action == Touch_action_e::Default)) {
        eventQueue.addMove(std::move(extraCommand)); // Issue the extra command for regular button presses
      } else {
        switch (action) {
          case Touch_action_e::ActivateGroup:
            setButtonGroup(event, get8BitFromUL(TouchObjects[objectIndex].groupFlags, TOUCH_OBJECT_GROUP_ACTIONGROUP));
            break;
          case Touch_action_e::IncrementGroup:
            incrementButtonGroup(event);
            break;
          case Touch_action_e::DecrementGroup:
            decrementButtonGroup(event);
            break;
          case Touch_action_e::IncrementPage:
            incrementButtonPage(event);
            break;
          case Touch_action_e::DecrementPage:
            decrementButtonPage(event);
            break;
          case Touch_action_e::Default:
          case Touch_action_e::TouchAction_MAX: // no action
            break;
        }
        String log = F("TOUCH event: ");
        log += toString(action);
        addLogMove(LOG_LEVEL_INFO, log);
      }
    }
  }
  # endif // ifdef TOUCH_USE_EXTENDED_TOUCH
  delay(0);
}

#endif // ifdef PLUGIN_USES_TOUCHHANDLER
