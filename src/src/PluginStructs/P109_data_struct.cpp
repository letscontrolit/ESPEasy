#include "../PluginStructs/P109_data_struct.h"

#ifdef USES_P109

# include "../src/src/Globals/ESPEasy_time.h"

/**************************************************************************
 * Constructor
 *************************************************************************/
P109_data_struct::P109_data_struct():
  _display(nullptr), _taskIndex(0), _varIndex(0)
{
  for (int i = 0; i < P109_Nlines; ++i) {
    ZERO_FILL(_deviceTemplate[i]);
  }
}

/**************************************************************************
 * Destructor
 *************************************************************************/
P109_data_struct::~P109_data_struct() {
  if (nullptr != _display) {
    _display->end();
  }
  delete _display;
  _display = nullptr;
}

/**************************************************************************
 * Load ExtraTaskSettings data
 *************************************************************************/
bool P109_data_struct::plugin_webform_load(struct EventStruct *event) {
  bool success = true;

  LoadCustomTaskSettings(event->TaskIndex, reinterpret_cast<uint8_t *>(&_deviceTemplate), sizeof(_deviceTemplate));

  for (int varNr = 0; varNr < P109_Nlines; varNr++) {
    addFormTextBox(concat(varNr == 0 ? F("Temperature source ") : F("Line "), varNr + 1),
                   getPluginCustomArgName(varNr + 1),
                   _deviceTemplate[varNr],
                   P109_Nchars);
  }

  return success;
}

/**************************************************************************
 * Save ExtraTaskSetting data
 *************************************************************************/
bool P109_data_struct::plugin_webform_save(struct EventStruct *event) {
  bool success = false;

  for (uint8_t varNr = 0; varNr < P109_Nlines; varNr++) {
    strncpy(_deviceTemplate[varNr],
            web_server.arg(getPluginCustomArgName(varNr + 1)).c_str(),
            sizeof(_deviceTemplate[varNr]) - 1);
    _deviceTemplate[varNr][sizeof(_deviceTemplate[varNr]) - 1] = 0;
  }

  String error = SaveCustomTaskSettings(event->TaskIndex, reinterpret_cast<uint8_t *>(&_deviceTemplate), sizeof(_deviceTemplate));

  if (!error.isEmpty()) {
    addHtmlError(error);
  }

  return success;
}

/**************************************************************************
 * Initialize plugin
 *************************************************************************/
bool P109_data_struct::plugin_init(struct EventStruct *event) {
  _lastWiFiState = P109_WIFI_STATE_UNSET;

  // Load the custom settings from flash
  LoadCustomTaskSettings(event->TaskIndex, reinterpret_cast<uint8_t *>(&_deviceTemplate), sizeof(_deviceTemplate));

  //      Init the display and turn it on
  if (_display) {
    _display->end();
    delete _display;
    _display = nullptr;
  }
  _taskIndex       = event->TaskIndex;
  _varIndex        = event->BaseVarIndex;
  _relaypin        = P109_CONFIG_RELAYPIN;
  _relayInverted   = P109_GET_RELAY_INVERT;
  _setpointTimeout = P109_CONFIG_SETPOINT_DELAY - P109_SETPOINT_OFFSET;

  if (P109_CONFIG_DISPLAYTYPE == 1) {
    _display = new (std::nothrow) SSD1306Wire(P109_CONFIG_I2CADDRESS, Settings.Pin_i2c_sda, Settings.Pin_i2c_scl);
  } else {
    _display = new (std::nothrow) SH1106Wire(P109_CONFIG_I2CADDRESS, Settings.Pin_i2c_sda, Settings.Pin_i2c_scl);
  }

  if (nullptr == _display) {
    return false; // Premature exit
  }

  if (P109_GET_TASKNAME_IN_TITLE == 1) {
    _title = getTaskDeviceName(event->TaskIndex);
  } else {
    _title = Settings.getHostname();
  }
  _alternateTitle = !P109_GET_ALTERNATE_HEADER;

  _display->init(); // call to local override of init function
  _display->displayOn();

  OLedSetContrast(_display, P109_CONFIG_CONTRAST);

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    log += concat(F("Thermo : Btn L:"), static_cast<int>(CONFIG_PIN1));
    log += concat(F(", R:"), static_cast<int>(CONFIG_PIN2));
    log += concat(F(", M:"), static_cast<int>(CONFIG_PIN3));
    addLogMove(LOG_LEVEL_INFO, log);
  }
  # endif // ifndef BUILD_NO_DEBUG

  for (uint8_t pin = 0; pin < 3; pin++) {
    if (validGpio(PIN(pin))) {
      pinMode(PIN(pin), INPUT_PULLUP);
    }
  }

  _prev_temp = P109_TEMP_STATE_UNSET;

  String fileName;
  fileName += concat(F("thermo"), static_cast<int>(_taskIndex + 1)); // Settings per task index
  fileName += F(".dat");
  fs::File f = tryOpenFile(fileName, String('r'));

  if (!f) { // Not found? Then open previous default filename
    fileName = F("thermo.dat");
    f        = tryOpenFile(fileName, String('r'));
  }

  if (f) {
    f.read(reinterpret_cast<uint8_t *>(&UserVar[event->BaseVarIndex]), 16);
    f.close();
  }
  _save_setpoint = UserVar[event->BaseVarIndex];
  _prev_setpoint = UserVar[event->BaseVarIndex];

  if (UserVar[event->BaseVarIndex] < 1) {
    UserVar[event->BaseVarIndex] = P109_SETPOINT_STATE_INITIAL; // setpoint
  }
  UserVar[event->BaseVarIndex + 1] = 0.5f;                      // Unitialize relay state
  UserVar[event->BaseVarIndex + 2] = P109_MODE_STATE_INITIAL;   // mode (X=0,A=1,M=2)
  UserVar[event->BaseVarIndex + 3] = 0;                         // Reset

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    log.reserve(48);
    log += F("Thermo : Starting status S:");
    log += formatUserVarNoCheck(event, 0);
    log += concat(F(", R:"), static_cast<int>(UserVar[event->BaseVarIndex + 1]));
    addLogMove(LOG_LEVEL_INFO, log);
  }
  # endif // ifndef BUILD_NO_DEBUG

  _changed    = 0; // No need to save, settings have just been restored
  _buttons[0] = 0;
  _buttons[1] = 0;
  _buttons[2] = 0;

  // flip screen if required
  if (P109_CONFIG_ROTATION == 2) { _display->flipScreenVertically(); }

  // Display the device name, logo, time and wifi
  display_header();
  display_page();
  _display->display();
  _initialized = true;

  check_auto_mode(event);

  return _initialized;
}

/**************************************************************************
 * De-initialize, pre-destructor
 *************************************************************************/
bool P109_data_struct::plugin_exit(struct EventStruct *event) {
  if (_saveneeded == 1) { // Can't wait for timeout any longer
    saveThermoSettings(event);
  }
  _initialized = false;

  return true;
}

/**************************************************************************
 * Check button state
 *************************************************************************/
bool P109_data_struct::plugin_ten_per_second(struct EventStruct *event) {
  if (_initialized) {
    uint32_t current_time;

    if (validGpio(CONFIG_PIN1) && !digitalRead(CONFIG_PIN1)) {
      current_time = millis();

      if (_buttons[0] + P109_BUTTON_DEBOUNCE_TIME_MS < current_time) {
        _buttons[0] = current_time;
        actionLeft(event);
      }
    }

    if (validGpio(CONFIG_PIN2) && !digitalRead(CONFIG_PIN2)) {
      current_time = millis();

      if (_buttons[1] + P109_BUTTON_DEBOUNCE_TIME_MS < current_time) {
        _buttons[1] = current_time;
        actionRight(event);
      }
    }

    if (validGpio(CONFIG_PIN3) && !digitalRead(CONFIG_PIN3)) {
      current_time = millis();

      if (_buttons[2] + P109_BUTTON_DEBOUNCE_TIME_MS < current_time) {
        _buttons[2] = current_time;
        actionMode(event);
      }
    }
  }

  return _initialized;
}

/**************************************************************************
 * Update display
 *************************************************************************/
bool P109_data_struct::plugin_once_a_second(struct EventStruct *event) {
  if (_initialized) {
    if (_display && display_wifibars()) {
      // WiFi symbol was updated.
      _display->display();
    }

    if (UserVar[event->BaseVarIndex + 2] == 2) { // manual timeout
      if (UserVar[event->BaseVarIndex + 3] > 0) {
        UserVar[event->BaseVarIndex + 3] = UserVar[event->BaseVarIndex + 3] - 1;
        display_timeout();
      } else {
        UserVar[event->BaseVarIndex + 3] = 0;
        setMode(F("a"), F("0")); // heater to auto
        display_setpoint_temp(1);
        check_auto_mode(event);  // Check now to avoid a double save
      }
    }

    if (_setpointDelay > 0) {
      _setpointDelay--;

      if ((_setpointDelay == 0) && !_last_heater.isEmpty()) {
        setHeater(_last_heater); // Last requested status, applied after delay
      }
    } else {
      check_auto_mode(event);
    }

    if (_changed == 1) {
      sendData(event);
      bool isSpDif = !essentiallyEqual(UserVar[event->BaseVarIndex], _save_setpoint);

      // Don't save when in manual mode
      _saveneeded     = (isSpDif && UserVar[event->BaseVarIndex + 2] != 2) ? 1 : 0;
      _changed        = 0;
      _lastchangetime = millis(); // Let's wait before actually saving
    }

    if ((_saveneeded == 1) && ((_lastchangetime + P109_DELAY_BETWEEN_SAVE) < millis())) {
      _saveneeded    = 0;
      _save_setpoint = UserVar[event->BaseVarIndex]; // Save this for next save-check

      saveThermoSettings(event);
    }
  }
  return _initialized;
}

/**************************************************************************
 * Save thermo settings unconditionally
 *************************************************************************/
void P109_data_struct::saveThermoSettings(struct EventStruct *event) {
  String fileName;

  fileName += concat(F("thermo"), static_cast<int>(event->TaskIndex + 1));
  fileName += F(".dat");
  fs::File f = tryOpenFile(fileName, F("w"));

  if (f) {
    f.write(reinterpret_cast<const uint8_t *>(&UserVar[event->BaseVarIndex]), 16);
    f.close();
    flashCount();
  }
  # ifndef BUILD_NO_DEBUG
  String log;
  log.reserve(fileName.length() + 36);
  log += F("Thermo : (delayed) Save UserVars to ");
  log += fileName;
  addLog(LOG_LEVEL_INFO, log);
  # endif // ifndef BUILD_NO_DEBUG
}

/**************************************************************************
 * Handle data
 *************************************************************************/
bool P109_data_struct::plugin_read(struct EventStruct *event) {
  if (_initialized) {
    //      Update display
    display_header();

    check_auto_mode(event);

    display_current_temp();
    display_timeout();

    _display->display();
  }
  return _initialized;
}

void P109_data_struct::check_auto_mode(struct EventStruct *event) {
  if (UserVar[event->BaseVarIndex + 2] == 1) {
    String atempstr2 = _deviceTemplate[0];
    String atempstr  = parseTemplate(atempstr2);

    if (!atempstr.isEmpty() &&
        (_prev_temp != P109_TEMP_STATE_UNSET)) { // do not switch until the first temperature data arrives
      float atemp = atempstr.toFloat();

      if (atemp != 0.0f) {
        if ((UserVar[event->BaseVarIndex] > atemp) &&
            (UserVar[event->BaseVarIndex + 1] < 1.0f)) {
          if (_prev_heating != 1) {
            _changed = 1;
          }
          setHeater(F("1"));
        } else if (((atemp - P109_CONFIG_HYSTERESIS) >= UserVar[event->BaseVarIndex]) &&
                   (UserVar[event->BaseVarIndex + 1] > 0.0f)) {
          if (_prev_heating != 0) {
            _changed = 1;
          }
          setHeater(F("0"));
        } else {
          display_heat();
        }
      }
    } else {
      display_heat();
    }
  }
}

/**************************************************************************
 * Handle commands
 *************************************************************************/
bool P109_data_struct::plugin_write(struct EventStruct *event,
                                    String            & string) {
  bool   success = false;
  String command = parseString(string, 1);

  if (_initialized) {
    String subcommand = parseString(string, 2);

    if (equals(command, F("oledframedcmd"))) {
      success = true;

      if (equals(subcommand, F("off"))) {
        OLedSetContrast(_display, OLED_CONTRAST_OFF);
      }
      else if (equals(subcommand, F("on"))) {
        _display->displayOn();
      }
      else if (equals(subcommand, F("low"))) {
        OLedSetContrast(_display, OLED_CONTRAST_LOW);
      }
      else if (equals(subcommand, F("med"))) {
        OLedSetContrast(_display, OLED_CONTRAST_MED);
      }
      else if (equals(subcommand, F("high"))) {
        OLedSetContrast(_display, OLED_CONTRAST_HIGH);
      } else {
        success = false;
      }
    }

    if (!success && equals(command, F("thermo"))) {
      success = true;
      String par3 = parseString(string, 3);

      if (equals(subcommand, F("setpoint"))) {
        setSetpoint(par3);
        check_auto_mode(event);
      }
      else if (equals(subcommand, F("down"))) {    // Emulate Left button action
        actionLeft(event);
      }
      else if (equals(subcommand, F("up"))) {      // Emulate Right button action
        actionRight(event);
      }
      else if (equals(subcommand, F("modebtn"))) { // Emulate Mode button action
        actionMode(event);
      }
      else if (equals(subcommand, F("heating"))) {
        int prev = UserVar[event->BaseVarIndex + 1];
        setHeater(par3);

        if (prev != UserVar[event->BaseVarIndex + 1]) {
          _changed = 1; // Only if actually changed
        }
      }
      else if (equals(subcommand, F("mode"))) {
        setMode(par3, parseString(string, 4));
      } else {
        success = false;
      }
    }

    if (success) {
      SendStatus(event, F("\nOk")); // FIXME: Will cause duplicate Ok message, why?
    }
  }
  return success;
}

/*******************
 * Private methods
 ******************/
/**************************************************************************
 * Left button action
 *************************************************************************/
void P109_data_struct::actionLeft(struct EventStruct *event) {
  switch (int(UserVar[event->BaseVarIndex + 2])) {
    case 0: { // off mode, no func
      break;
    }
    case 1: { // auto mode, setpoint dec
      setSetpoint(F("-0.5"));
      break;
    }
    case 2: { // manual on mode, timer dec
      UserVar[event->BaseVarIndex + 3] = UserVar[event->BaseVarIndex + 3] - P109_BUTTON_DEBOUNCE_TIME_MS;

      if (UserVar[event->BaseVarIndex + 3] < 0) {
        UserVar[event->BaseVarIndex + 3] = 5400;
      }
      _prev_timeout = P109_TIMEOUT_STATE_UNSET;
      break;
    }
  }
}

/**************************************************************************
 * Right button action
 *************************************************************************/
void P109_data_struct::actionRight(struct EventStruct *event) {
  switch (int(UserVar[event->BaseVarIndex + 2])) {
    case 0: { // off mode, no func
      break;
    }
    case 1: { // auto mode, setpoint inc
      setSetpoint(F("+0.5"));
      break;
    }
    case 2: { // manual on mode, timer dec
      UserVar[event->BaseVarIndex + 3] = UserVar[event->BaseVarIndex + 3] + P109_BUTTON_DEBOUNCE_TIME_MS;

      if (UserVar[event->BaseVarIndex + 3] > 5400) {
        UserVar[event->BaseVarIndex + 3] = 60;
      }
      _prev_timeout = P109_TIMEOUT_STATE_UNSET;
      break;
    }
  }
}

/**************************************************************************
 * Rotate mode
 *************************************************************************/
void P109_data_struct::actionMode(struct EventStruct *event) {
  switch (int(UserVar[event->BaseVarIndex + 2])) {
    case 0: { // off mode, next
      setMode(F("a"), F("0"));
      break;
    }
    case 1: { // auto mode, next
      setMode(F("m"), F("5"));
      break;
    }
    case 2: { // manual on mode, next
      setMode(F("x"), F("0"));
      break;
    }
  }
}

/**
 * Display header, alternating between WiFi AP SSID and Sysname
 */
void P109_data_struct::display_header() {
  if (_alternateTitle && _showWiFiName && WiFiEventData.WiFiServicesInitialized()) {
    // String newString = ;
    display_title(WiFi.SSID());
  } else {
    display_title(_title);
  }
  _showWiFiName = !_showWiFiName;

  // Display time and wifibars both clear area below, so paint them after the title.
  display_time();
  display_wifibars();
}

/**
 * Display the current time
 */
void P109_data_struct::display_time() {
  String newString = node_time.getTimeString(':', false); // Avoid the overhead of calling parseTemplate()

  _display->setTextAlignment(TEXT_ALIGN_LEFT);
  displayBigText(0, 0, 28, 13, getDialog_plain_12(), 0, 0, newString.substring(0, 5));
}

/**
 * Display the title centered
 */
void P109_data_struct::display_title(const String& title) {
  _display->setTextAlignment(TEXT_ALIGN_CENTER);
  displayBigText(0, 0, 128, 15, getDialog_plain_12(), 64, 0, title);
}

/**
 * Draw Signal Strength Bars, return true when there was an update.
 */
bool P109_data_struct::display_wifibars() {
  const bool connected    = WiFiEventData.WiFiServicesInitialized();
  const int  nbars_filled = (WiFi.RSSI() + 100) / 8;
  const int  newState     = connected ? nbars_filled : P109_WIFI_STATE_UNSET;

  if (newState == _lastWiFiState) {
    return false; // nothing to do.
  }
  const int x         = 105;
  const int y         = 0;
  int size_x          = 15;
  const int size_y    = 10;
  const int nbars     = 5;
  const int16_t width = (size_x / nbars);

  size_x = width * nbars - 1; // Correct for round errors.

  //  x,y are the x,y locations
  //  sizex,sizey are the sizes (should be a multiple of the number of bars)
  //  nbars is the number of bars and nbars_filled is the number of filled bars.

  //  We leave a 1 pixel gap between bars
  _display->setColor(BLACK);
  _display->fillRect(x, y, size_x, size_y);
  _display->setColor(WHITE);

  if (WiFiEventData.WiFiServicesInitialized()) {
    for (uint8_t ibar = 0; ibar < nbars; ibar++) {
      int16_t height = size_y * (ibar + 1) / nbars;
      int16_t xpos   = x + ibar * width;
      int16_t ypos   = y + size_y - height;

      if (ibar <= nbars_filled) {
        // Fill complete bar
        _display->fillRect(xpos, ypos, width - 1, height);
      } else {
        // Only draw top and bottom.
        _display->fillRect(xpos, ypos,           width - 1, 1);
        _display->fillRect(xpos, y + size_y - 1, width - 1, 1);
      }
    }
  } else {
    // Draw a not connected sign.
  }
  return true;
}

/**
 * Display current temperature
 */
void P109_data_struct::display_current_temp() {
  String tmpString = _deviceTemplate[0];
  String atempstr  = parseTemplate(tmpString);

  atempstr.trim();

  if (atempstr.length() > 0) {
    float atemp = atempstr.toFloat();
    atemp = (roundf(atemp * 10.0f)) / 10.0f;

    if (!essentiallyEqual(_prev_temp, atemp)) {
      tmpString = toString(atemp, 1);
      displayBigText(3, 19, 47, 25, getArialMT_Plain_24(), 3, 19, tmpString.substring(0, 5));

      _prev_temp = atemp;
    }
  }
}

/**
 * Display the Setpoint temperature
 */
void P109_data_struct::display_setpoint_temp(const uint8_t& force) {
  if (UserVar[_varIndex + 2] == 1) {
    float stemp = (roundf(UserVar[_varIndex] * 10.0f)) / 10.0f;
    bool  isDif = !essentiallyEqual(_prev_setpoint, stemp);

    if (isDif || (force == 1)) {
      String tmpString = toString(stemp, 1);
      displayBigText(86, 35, 41, 21, getDialog_plain_18(), 86, 35, tmpString.substring(0, 5));

      _prev_setpoint = stemp;

      if (isDif) {
        _changed       = 1;
        _setpointDelay = _setpointTimeout; // Start delay
      }
    }
  }
}

/**
 * Display the remaining timeout, if any is set
 */
void P109_data_struct::display_timeout() {
  if (UserVar[_varIndex + 2] == 2) {
    if (_prev_timeout >= (UserVar[_varIndex + 3] + 60.0f)) {
      String thour = minutesToHourColonMinute(static_cast<int>(UserVar[_varIndex + 3] / 60.0f));
      displayBigText(86, 35, 41, 21, getDialog_plain_18(), 89, 35, thour.substring(1, 5));

      _prev_timeout = UserVar[_varIndex + 3];
    }
  }
}

/**
 * Display the current mode
 */
void P109_data_struct::display_mode() {
  if (_prev_mode != UserVar[_varIndex + 2]) {
    String   tmpString = F("XAM");
    uint16_t xamIdx    = min(static_cast<int>(UserVar[_varIndex + 2]), 2);

    displayBigText(61, 49, 12, 17, getArialMT_Plain_16(), 61, 49, tmpString.substring(xamIdx, xamIdx + 1));

    _prev_mode = UserVar[_varIndex + 2];
  }
}

void P109_data_struct::displayBigText(int16_t       x1,
                                      int16_t       y1,
                                      int16_t       w1,
                                      int16_t       h1,
                                      const char   *font,
                                      int16_t       x2,
                                      int16_t       y2,
                                      const String& text) {
  _display->setColor(BLACK);
  _display->fillRect(x1, y1, w1, h1);
  _display->setColor(WHITE);
  _display->setFont(font);
  _display->drawString(x2, y2, text);
}

/**
 * Display the Heater status (flame)
 */
void P109_data_struct::display_heat() {
  if (_prev_heating != UserVar[_varIndex + 1]) {
    _display->setColor(BLACK);
    _display->fillRect(54, 19, 24, 27);
    _display->setColor(WHITE);

    if (UserVar[_varIndex + 1] == 1) {
      _display->drawXbm(54, 19, 24, 27, flameimg);
    }
    _prev_heating = UserVar[_varIndex + 1];
  }
}

/**
 * Setup the initial page, draw lines etc.
 */
void P109_data_struct::display_page() {
  // init with full clear

  String tstr      = F("{D}C");
  String newString = parseTemplate(tstr);

  _display->setTextAlignment(TEXT_ALIGN_LEFT);
  displayBigText(0, 15, 128, 49, getDialog_plain_12(), 18, 46, newString.substring(0, 5));

  display_heat();

  _display->drawHorizontalLine(0, 15, 128);
  _display->drawVerticalLine(52, 14, 49);

  _display->drawCircle(109, 47, 26);
  _display->drawHorizontalLine(78, 47, 8);
  _display->drawVerticalLine(109, 19, 10);

  display_mode();
  display_setpoint_temp(1);
  display_current_temp();
}

/**
 * Change the setpoint value, and update the display
 */
void P109_data_struct::setSetpoint(const String& sptemp) {
  float stemp = (roundf(UserVar[_varIndex] * 10.0f)) / 10.0f;

  if ((sptemp.charAt(0) == '+') || (sptemp.charAt(0) == 'p'))  {
    stemp = stemp + sptemp.substring(1).toFloat();
  } else if ((sptemp.charAt(0) == '-') || (sptemp.charAt(0) == 'm')) {
    stemp = stemp - sptemp.substring(1).toFloat();
  } else {
    stemp = sptemp.toFloat();
  }
  UserVar[_varIndex] = stemp;
  display_setpoint_temp();
}

/**
 * Set the heater relay state
 */
void P109_data_struct::setHeatRelay(const uint8_t& state) {
  if (validGpio(_relaypin)) {
    # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log;

      log += concat(F("Thermo : Set Relay"), static_cast<int>(_relaypin));
      log += '=';
      log += _relayInverted ? !state : state;
      addLogMove(LOG_LEVEL_INFO, log);
    }
    # endif // ifndef BUILD_NO_DEBUG

    pinMode(_relaypin, OUTPUT);
    digitalWrite(_relaypin, _relayInverted ? !state : state);
  }
}

/**
 * Change the heater state and update the display
 */
void P109_data_struct::setHeater(const String& heater) {
  if (_setpointDelay == 0) {
    if ((heater.charAt(0) == '1') || (equals(heater, F("on"))) ||
        ((heater.length() == 0) && (UserVar[_varIndex + 1] == 0))) {
      UserVar[_varIndex + 1] = 1;
      setHeatRelay(HIGH);
    } else {
      UserVar[_varIndex + 1] = 0;
      setHeatRelay(LOW);
    }
    display_heat();
  }
  _last_heater = heater;
}

/**
 * Set the mode and update the display
 */
void P109_data_struct::setMode(const String& amode,
                               const String& atimeout) {
  UserVar[_varIndex + 3] = 0.0f; // Reset timeout

  if ((amode[0] == '0') || (amode[0] == 'x')) {
    UserVar[_varIndex + 2] = 0;
    setHeater(F("0"));
    _display->setColor(BLACK);
    _display->fillRect(86, 35, 41, 21);
    _prev_setpoint = P109_SETPOINT_STATE_UNSET;
  } else if ((amode[0] == '1') || (amode[0] == 'a')) {
    UserVar[_varIndex + 2] = 1;
    display_setpoint_temp(1);
  } else if ((amode[0] == '2') || (amode[0] == 'm')) {
    UserVar[_varIndex + 2] = 2;
    UserVar[_varIndex + 3] = (atimeout.toFloat() * 60.0f);
    _prev_timeout          = P109_TIMEOUT_STATE_UNSET;
    display_timeout();
    setHeater(F("1"));
  } else {
    UserVar[_varIndex + 2] = 0;
  }

  // _changed = 1;
  display_mode();
}

#endif // ifdef USES_P109
