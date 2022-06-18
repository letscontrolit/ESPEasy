#include "../PluginStructs/P109_data_struct.h"

#ifdef USES_P109

/**************************************************************************
 * Constructor
 *************************************************************************/
P109_data_struct::P109_data_struct():
  _display(nullptr), _taskIndex(0), _varIndex(0) {}

/**************************************************************************
 * Destructor
 *************************************************************************/
P109_data_struct::~P109_data_struct() {}

/**************************************************************************
 * Load ExtraTaskSettings data
 *************************************************************************/
bool P109_data_struct::plugin_webform_load(struct EventStruct *event) {
  bool success = true;

  LoadCustomTaskSettings(event->TaskIndex, reinterpret_cast<uint8_t *>(&_deviceTemplate), sizeof(_deviceTemplate));

  for (uint8_t varNr = 0; varNr < P109_Nlines; varNr++) {
    String label = F("Line ");

    if (varNr == 0) {
      label = F("Temperature source ");
    }
    label += varNr + 1;
    addFormTextBox(label,
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

  if (error.length() > 0) {
    addHtmlError(error);
  }

  return success;
}

/**************************************************************************
 * Initialize plugin
 *************************************************************************/
bool P109_data_struct::plugin_init(struct EventStruct *event) {
  bool success = false;

  _lastWiFiState = P109_WIFI_STATE_UNSET;

  // Load the custom settings from flash
  LoadCustomTaskSettings(event->TaskIndex, reinterpret_cast<uint8_t *>(&_deviceTemplate), sizeof(_deviceTemplate));

  //      Init the display and turn it on
  if (_display) {
    _display->end();
    delete _display;
    _display = nullptr;
  }
  _taskIndex = event->TaskIndex;
  _varIndex  = event->BaseVarIndex;
  _relaypin  = P109_CONFIG_RELAYPIN;

  if (P109_CONFIG_DISPLAYTYPE == 1) {
    _display = new (std::nothrow) SSD1306Wire(P109_CONFIG_I2CADDRESS, Settings.Pin_i2c_sda, Settings.Pin_i2c_scl);
  } else {
    _display = new (std::nothrow) SH1106Wire(P109_CONFIG_I2CADDRESS, Settings.Pin_i2c_sda, Settings.Pin_i2c_scl);
  }

  if (nullptr == _display) {
    return success; // Premature exit
  }
  _display->init(); // call to local override of init function
  _display->displayOn();

  OLedSetContrast(_display, P109_CONFIG_CONTRAST);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String logstr = F("Thermo : Btn L:");
    logstr += CONFIG_PIN1;
    logstr += F("R:");
    logstr += CONFIG_PIN2;
    logstr += F("M:");
    logstr += CONFIG_PIN3;
    addLogMove(LOG_LEVEL_INFO, logstr);
  }

  for (uint8_t pin = 0; pin < 3; pin++) {
    if (validGpio(PIN(pin))) {
      pinMode(PIN(pin), INPUT_PULLUP);
    }
  }

  _prev_temp = P109_TEMP_STATE_UNSET;

  String fileName = F("thermo"); // Settings per task index

  fileName += _taskIndex + 1;
  fileName += F(".dat");
  fs::File f = tryOpenFile(fileName, F("r"));

  if (!f) { // Not found? Then open previous default filename
    fileName = F("thermo.dat");
    f        = tryOpenFile(fileName, F("r"));
  }

  if (f) {
    f.read(reinterpret_cast<uint8_t *>(&UserVar[event->BaseVarIndex]), 16);
    f.close();
  }
  _lastsavetime = millis();

  if (UserVar[event->BaseVarIndex] < 1) {
    UserVar[event->BaseVarIndex]     = P109_SETPOINT_STATE_INITIAL; // setpoint
    UserVar[event->BaseVarIndex + 2] = P109_MODE_STATE_INITIAL;     // mode (X=0,A=1,M=2)
  }

  // UserVar[event->BaseVarIndex + 1] = 0; // heating (0=off,1=heating in progress)
  // UserVar[event->BaseVarIndex + 3] = 0; // timeout (manual on for minutes)

  if (_relaypin != -1) {
    setHeatRelay(static_cast<uint8_t>(UserVar[event->BaseVarIndex + 1]));
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String logstr = F("Thermo : Starting status S:");
    logstr += toString(UserVar[event->BaseVarIndex]);
    logstr += F(", R:");
    logstr += toString(UserVar[event->BaseVarIndex + 1]);
    addLogMove(LOG_LEVEL_INFO, logstr);
  }

  _changed    = 1;
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

  success = true;

  return success;
}

/**************************************************************************
 * De-initialize, pre-destructor
 *************************************************************************/
bool P109_data_struct::plugin_exit(struct EventStruct *event) {
  bool success = true;

  if (nullptr != _display) {
    _display->end();
  }
  delete _display;
  _display     = nullptr;
  _initialized = false;

  return success;
}

/**************************************************************************
 * Check button state
 *************************************************************************/
bool P109_data_struct::plugin_ten_per_second(struct EventStruct *event) {
  bool success = false;

  if (_initialized) {
    uint32_t current_time;

    if (validGpio(CONFIG_PIN1) && !digitalRead(CONFIG_PIN1)) {
      current_time = millis();

      if (_buttons[0] + P109_BUTTON_DEBOUNCE_TIME_MS < current_time) {
        _buttons[0] = current_time;

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
            _changed      = 1;
            break;
          }
        }
      }
    }

    if (validGpio(CONFIG_PIN2) && !digitalRead(CONFIG_PIN2)) {
      current_time = millis();

      if (_buttons[1] + P109_BUTTON_DEBOUNCE_TIME_MS < current_time) {
        _buttons[1] = current_time;

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
            _changed      = 1;
            break;
          }
        }
      }
    }

    if (validGpio(CONFIG_PIN3) && !digitalRead(CONFIG_PIN3)) {
      current_time = millis();

      if (_buttons[2] + P109_BUTTON_DEBOUNCE_TIME_MS < current_time) {
        _buttons[2] = current_time;

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
    }
  }

  return success;
}

/**************************************************************************
 * Update display
 *************************************************************************/
bool P109_data_struct::plugin_once_a_second(struct EventStruct *event) {
  bool success = false;

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
      }
    }

    if (_changed == 1) {
      sendData(event);
      _saveneeded = 1;
      _changed    = 0;
    }

    if ((_saveneeded == 1) && ((_lastsavetime + 30000) < millis())) {
      _saveneeded   = 0;
      _lastsavetime = millis();
      String fileName = F("thermo");
      fileName += _taskIndex + 1;
      fileName += F(".dat");
      fs::File f = tryOpenFile(fileName, F("w"));

      if (f) {
        f.write(reinterpret_cast<const uint8_t *>(&UserVar[event->BaseVarIndex]), 16);
        f.close();
        flashCount();
      }
      addLog(LOG_LEVEL_INFO, F("Thermo : Save UserVars to SPIFFS"));
    }
    success = true;
  }
  return success;
}

/**************************************************************************
 * Handle data
 *************************************************************************/
bool P109_data_struct::plugin_read(struct EventStruct *event) {
  bool success = false;

  if (_initialized) {
    //      Update display
    display_header();

    if (UserVar[event->BaseVarIndex + 2] == 1) {
      String atempstr2 = _deviceTemplate[0];
      String atempstr  = parseTemplate(atempstr2);
      atempstr.trim();

      if ((atempstr.length() > 0) &&
          (_prev_temp != P109_TEMP_STATE_UNSET)) { // do not switch until the first temperature data arrives
        float atemp = atempstr.toFloat();

        if (atemp != 0.0f) {
          if ((UserVar[event->BaseVarIndex] > atemp) &&
              (UserVar[event->BaseVarIndex + 1] < 1)) {
            setHeater(F("1"));
            _changed = 1;
          } else if (((atemp - P109_CONFIG_HYSTERESIS) >= UserVar[event->BaseVarIndex]) &&
                     (UserVar[event->BaseVarIndex + 1] > 0)) {
            setHeater(F("0"));
            _changed = 1;
          } else {
            display_heat();
          }
        }
      } else {
        display_heat();
      }
    }

    display_current_temp();
    display_timeout();

    _display->display();

    success = true;
  }
  return success;
}

/**************************************************************************
 * Handle commands
 *************************************************************************/
bool P109_data_struct::plugin_write(struct EventStruct *event,
                                    String            & string) {
  bool   success    = false;
  String command    = parseString(string, 1);
  String subcommand = parseString(string, 2);

  if (_initialized) {
    if (command == F("oledframedcmd")) {
      success = true;

      if (subcommand == F("off")) {
        OLedSetContrast(_display, OLED_CONTRAST_OFF);
      }
      else if (subcommand == F("on")) {
        _display->displayOn();
      }
      else if (subcommand == F("low")) {
        OLedSetContrast(_display, OLED_CONTRAST_LOW);
      }
      else if (subcommand == F("med")) {
        OLedSetContrast(_display, OLED_CONTRAST_MED);
      }
      else if (subcommand == F("high")) {
        OLedSetContrast(_display, OLED_CONTRAST_HIGH);
      }
      SendStatus(event, F("\nOk")); // Will cause duplicate Ok message
    }

    if (command == F("thermo")) {
      success = true;
      String par1 = parseString(string, 3);

      if (subcommand == F("setpoint")) {
        setSetpoint(par1);
      }
      else if (subcommand == F("heating")) {
        setHeater(par1);
        _changed = 1;
      }
      else if (subcommand == F("mode")) {
        setMode(par1, parseString(string, 4));
      }
      SendStatus(event, F("\nOk")); // Will cause duplicate Ok message
    }
  }
  return success;
}

/*******************
 * Private methods
 ******************/
/**
 * Display header, alternating between WiFi AP SSID and Sysname
 */
void P109_data_struct::display_header() {
  if (_showWiFiName && WiFiEventData.WiFiServicesInitialized()) {
    String newString = WiFi.SSID();
    newString.trim();
    display_title(newString);
  } else {
    String dtime(F("%sysname%"));
    String newString = parseTemplate(dtime);
    newString.trim();
    display_title(newString);
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
  String dtime(F("%systime%"));
  String newString = parseTemplate(dtime);

  _display->setTextAlignment(TEXT_ALIGN_LEFT);
  _display->setFont(getDialog_plain_12());
  _display->setColor(BLACK);
  _display->fillRect(0, 0, 28, 13);
  _display->setColor(WHITE);
  _display->drawString(0, 0, newString.substring(0, 5));
}

/**
 * Display the title centered
 */
void P109_data_struct::display_title(const String& title) {
  _display->setTextAlignment(TEXT_ALIGN_CENTER);
  _display->setFont(getDialog_plain_12());
  _display->setColor(BLACK);
  _display->fillRect(0, 0, 128, 15); // Underscores use an extra line, clear also.
  _display->setColor(WHITE);
  _display->drawString(64, 0, title);
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
    atemp = (round(atemp * 10.0f)) / 10.0f;

    if (_prev_temp != atemp) {
      _display->setColor(BLACK);
      _display->fillRect(3, 19, 47, 25);
      _display->setColor(WHITE);
      tmpString = toString(atemp, 1);
      _display->setFont(getArialMT_Plain_24());
      _display->drawString(3, 19, tmpString.substring(0, 5));
      _prev_temp = atemp;
    }
  }
}

/**
 * Display the Setpoint temperature
 */
void P109_data_struct::display_setpoint_temp(const uint8_t& force) {
  if (UserVar[_varIndex + 2] == 1) {
    float stemp = (round(UserVar[_varIndex] * 10.0f)) / 10.0f;

    if ((_prev_setpoint != stemp) || (force == 1)) {
      _display->setColor(BLACK);
      _display->fillRect(86, 35, 41, 21);
      _display->setColor(WHITE);
      String tmpString = toString(stemp, 1);
      _display->setFont(getDialog_plain_18());
      _display->drawString(86, 35, tmpString.substring(0, 5));
      _prev_setpoint = stemp;
      _changed       = 1;
    }
  }
}

/**
 * Display the remaining timeout, if any is set
 */
void P109_data_struct::display_timeout() {
  if (UserVar[_varIndex + 2] == 2) {
    if (_prev_timeout >= (UserVar[_varIndex + 3] + 60.0f)) {
      float  timeinmin = UserVar[_varIndex + 3] / 60.0f;
      String thour     = toString((static_cast<int>(timeinmin / 60.0f)), 0);
      thour += ':';
      String thour2 = toString((static_cast<int>(timeinmin) % 60), 0);

      if (thour2.length() < 2) {
        thour += '0';
      }
      thour += thour2;
      _display->setColor(BLACK);
      _display->fillRect(86, 35, 41, 21);
      _display->setColor(WHITE);
      _display->setFont(getDialog_plain_18());
      _display->drawString(86, 35, thour.substring(0, 5));
      _prev_timeout = UserVar[_varIndex + 3];
    }
  }
}

/**
 * Display the current mode
 */
void P109_data_struct::display_mode() {
  if (_prev_mode != UserVar[_varIndex + 2]) {
    String tmpString;

    switch (static_cast<int>(UserVar[_varIndex + 2])) {
      case 0:
        tmpString = 'X';
        break;
      case 1:
        tmpString = 'A';
        break;
      case 2:
        tmpString = 'M';
        break;
    }
    _display->setColor(BLACK);
    _display->fillRect(61, 49, 12, 17);
    _display->setColor(WHITE);
    _display->setFont(getArialMT_Plain_16());
    _display->drawString(61, 49, tmpString.substring(0, 5));
    _prev_mode = UserVar[_varIndex + 2];
  }
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
  _display->setColor(BLACK);
  _display->fillRect(0, 15, 128, 49);
  _display->setColor(WHITE);

  _prev_temp     = P109_TEMP_STATE_UNSET;
  _prev_setpoint = P109_SETPOINT_STATE_UNSET;
  _prev_heating  = P109_HEATING_STATE_UNSET;
  _prev_mode     = P109_MODE_STATE_UNSET;
  _prev_timeout  = P109_TIMEOUT_STATE_UNSET;

  _display->setFont(getDialog_plain_12());
  _display->setTextAlignment(TEXT_ALIGN_LEFT);
  String tstr      = F("{D}C");
  String newString = parseTemplate(tstr);

  _display->drawString(18, 46, newString.substring(0, 5));

  display_heat();

  _display->drawHorizontalLine(0, 15, 128);
  _display->drawVerticalLine(52, 14, 49);

  _display->drawCircle(109, 47, 26);
  _display->drawHorizontalLine(78, 47, 8);
  _display->drawVerticalLine(109, 19, 10);

  display_mode();
  display_setpoint_temp();
  display_current_temp();
}

/**
 * Change the setpoint value, and update the display
 */
void P109_data_struct::setSetpoint(const String& sptemp) {
  float stemp = (round(UserVar[_varIndex] * 10.0f)) / 10.0f;

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
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String logstr = F("Thermo : Set Relay");

    logstr += _relaypin;
    logstr += '=';
    logstr += state;
    addLogMove(LOG_LEVEL_INFO, logstr);
  }

  if (_relaypin != -1) {
    pinMode(_relaypin, OUTPUT);
    digitalWrite(_relaypin, state);
  }
}

/**
 * Change the heater state and update the display
 */
void P109_data_struct::setHeater(const String& heater) {
  if ((heater.charAt(0) == '1') || (heater == F("on")) ||
      ((heater.length() == 0) && (UserVar[_varIndex + 1] == 0))) {
    UserVar[_varIndex + 1] = 1;
    setHeatRelay(HIGH);
  } else {
    UserVar[_varIndex + 1] = 0;
    setHeatRelay(LOW);
  }
  display_heat();
}

/**
 * Set the mode and update the display
 */
void P109_data_struct::setMode(const String& amode,
                               const String& atimeout) {
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
  _changed = 1;
  display_mode();
}

#endif // ifdef USES_P109
