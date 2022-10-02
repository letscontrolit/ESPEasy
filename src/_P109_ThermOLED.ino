#include "_Plugin_Helper.h"
#ifdef USES_P109

# include "src/PluginStructs/P109_data_struct.h"

/*##########################################################################################
 ##################### Plugin 109: OLED SSD1306 display for Thermostat ####################
 ##########################################################################################

   This is a modification to Plugin_036 with graphics library provided from squix78 github
   https://github.com/squix78/esp8266-oled-ssd1306

   Features :
    - Displays and use current temperature from specified Device/Value (can be a Dummy for example)
    - Displays and maintains setpoint value
    - on power down/up this plugin maintains and reloads RELAY and SETPOINT values from SPIFFS
    - Supports 3 buttons, LEFT, RIGHT and MODE selection (MODE button cycles modes below,
      LEFT/RIGHT increases-decreases setpoint OR timeout (Mode sensitive)
    - one output relay need to be specified, currently only HIGH level active supported
    - 3 mode is available:
        - 0 or X: set relay permanently off no matter what
        - 1 or A: set relay ON if current temperature below setpoint, and set OFF when
                  temperature+hysteresis reached - comparison made at setted Plugin interval (AUTO MODE)
        - 2 or M: set relay ON for specified time in minutes (MANUAL ON MODE), after timeout, mode switch to "A"

   List of commands :
   - oledframedcmd,[OLED_STATUS]              Inherited command from P036 status can be:
                                              [off/on/low/med/high]
   - thermo,setpoint,[target_temperature]     Target setpoint, only used in Mode "A"
   - thermo,heating,[RELAY_STATUS]            Manually forcing relay status [off/on]
   - thermo,mode,[MODE],[TIMEOUT]             Set to either mode X/A/M, if M selected,
                                              then TIMEOUT can be specified in minutes
   - thermo,left                              Emulate the Left button action
   - thermo,right                             Emulate the Right button action
   - thermo,modebtn                           Emulate the Mode button action

   Command Examples :
   -  /control?cmd=thermo,setpoint,23          Set target setpoint to 23 Celsius
   -  /control?cmd=thermo,mode,1               Set mode to AUTOMATIC so it starts to maintain setpoint temperature
   -  /control?cmd=thermo,mode,2,5             Starts pre-heat for 5 minute, does not care about TEMP, then go to AUTO mode after timeout
   -  /control?cmd=thermo,mode,0               Switch heating off, absolutely do nothing until further notice

   ------------------------------------------------------------------------------------------
   Copyleft Nagy Sándor 2018 - https://bitekmindenhol.blog.hu/
   ------------------------------------------------------------------------------------------
   2022-09-21 tonhuisman: Adjust subcommands to leftbtn and rightbtn, to be consistent with modebtn
   2022-09-20 tonhuisman: Add commands for emulating the Left, Right and Mode buttons
   2022-08-28 tonhuisman: Changelog reversed order to newest on top
                          Deduplicate code for displaying text on display
   2022-06-18 tonhuisman: Enable multi-instance use, implement OLed_helper functions,
                          remove P109/Plugin_109 prefixes on variables and methods where appropriate
   2022-06-18 tonhuisman: More optimizations, use #defines where appropriate
   2022-06-17 tonhuisman: Optimizations
   No older changelog recorded.
 */

# define PLUGIN_109
# define PLUGIN_ID_109          109
# define PLUGIN_NAME_109        "Display - OLED SSD1306/SH1106 Thermo"
# define PLUGIN_VALUENAME1_109  "setpoint"
# define PLUGIN_VALUENAME2_109  "heating"
# define PLUGIN_VALUENAME3_109  "mode"
# define PLUGIN_VALUENAME4_109  "timeout"


boolean Plugin_109(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_109;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 4;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_109);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_109));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_109));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_109));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_109));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x3c, 0x3d };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("pi2caddr"), 2, i2cAddressValues, P109_CONFIG_I2CADDRESS);
      } else {
        success = intArrayContains(2, i2cAddressValues, event->Par1);
      }
      break;
    }

    # ifndef LIMIT_BUILD_SIZE
    case PLUGIN_WEBFORM_SHOW_GPIO_DESCR:
    {
      string  = F("Btn L: ");
      string += formatGpioLabel(CONFIG_PIN1, false);
      string += event->String1; // newline
      string += F("Btn R: ");
      string += formatGpioLabel(CONFIG_PIN2, false);
      string += event->String1; // newline
      string += F("Btn M: ");
      string += formatGpioLabel(CONFIG_PIN3, false);
      string += event->String1; // newline
      string += F("Relay: ");
      string += formatGpioLabel(P109_CONFIG_RELAYPIN, false);
      success = true;
      break;
    }
    # endif // ifndef LIMIT_BUILD_SIZE

    case PLUGIN_WEBFORM_LOAD:
    {
      OLedFormController(F("controller"), nullptr, P109_CONFIG_DISPLAYTYPE);

      OLedFormRotation(F("rotate"), P109_CONFIG_ROTATION);

      {
        P109_data_struct *P109_data = new (std::nothrow) P109_data_struct();

        if (nullptr != P109_data) {
          success = P109_data->plugin_webform_load(event); // Load CustomTaskSettings
          delete P109_data;
        }
      }

      addFormPinSelect(PinSelectPurpose::Generic_input,  F("Button left"),  F("taskdevicepin1"), CONFIG_PIN1);
      addFormPinSelect(PinSelectPurpose::Generic_input,  F("Button right"), F("taskdevicepin2"), CONFIG_PIN2);
      addFormPinSelect(PinSelectPurpose::Generic_input,  F("Button mode"),  F("taskdevicepin3"), CONFIG_PIN3);

      addFormPinSelect(PinSelectPurpose::Generic_output, F("Relay"),        F("heatrelay"),      P109_CONFIG_RELAYPIN);

      OLedFormContrast(F("contrast"), P109_CONFIG_CONTRAST);

      {
        const __FlashStringHelper *options4[] = { F("0.2"), F("0.5"), F("1") };
        const int optionValues4[]             = { 2, 5, 10 };
        addFormSelector(F("Hysteresis"), F("hyst"), 3, options4, optionValues4, static_cast<int>(P109_CONFIG_HYSTERESIS * 10.0f));
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P109_CONFIG_I2CADDRESS  = getFormItemInt(F("pi2caddr"));
      P109_CONFIG_ROTATION    = getFormItemInt(F("rotate"));
      P109_CONFIG_DISPLAYTYPE = getFormItemInt(F("controller"));
      P109_CONFIG_CONTRAST    = getFormItemInt(F("contrast"));
      P109_CONFIG_RELAYPIN    = getFormItemInt(F("heatrelay"));
      P109_CONFIG_HYSTERESIS  = (getFormItemInt(F("hyst")) / 10.0f);

      {
        P109_data_struct *P109_data = new (std::nothrow) P109_data_struct();

        if (nullptr != P109_data) {
          success = P109_data->plugin_webform_save(event); // Save CustomTaskSettings
          delete P109_data;
        }
      }

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P109_data_struct());
      P109_data_struct *P109_data = static_cast<P109_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P109_data) {
        success = P109_data->plugin_init(event); // Start plugin
      }

      break;
    }

    case PLUGIN_EXIT:
    {
      P109_data_struct *P109_data = static_cast<P109_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P109_data) {
        success = P109_data->plugin_exit(event); // Stop plugin
      }

      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P109_data_struct *P109_data = static_cast<P109_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P109_data) {
        success = P109_data->plugin_ten_per_second(event); // Check buttons
      }

      break;
    }

    // Switch off display after displayTimer seconds
    case PLUGIN_ONCE_A_SECOND:
    {
      P109_data_struct *P109_data = static_cast<P109_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P109_data) {
        success = P109_data->plugin_once_a_second(event); // Update display
      }

      break;
    }

    case PLUGIN_READ:
    {
      P109_data_struct *P109_data = static_cast<P109_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P109_data) {
        success = P109_data->plugin_read(event); // Read operation, get data
      }

      break;
    }

    case PLUGIN_WRITE:
    {
      P109_data_struct *P109_data = static_cast<P109_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P109_data) {
        success = P109_data->plugin_write(event, string); // Write operation, handle commands
      }

      break;
    }
  }

  return success;
}

<<<<<<< HEAD
=======
// Set the display contrast
// really low brightness & contrast: contrast = 10, precharge = 5, comdetect = 0
// normal brightness & contrast:  contrast = 100
void P109_setContrast(uint8_t OLED_contrast) {
  char contrast  = 100;
  char precharge = 241;
  char comdetect = 64;

  switch (OLED_contrast) {
    case P109_CONTRAST_OFF:

      if (P109_display) {
        P109_display->displayOff();
      }
      return;
    case P109_CONTRAST_LOW:
      contrast = 10; precharge = 5; comdetect = 0;
      break;
    case P109_CONTRAST_MED:
      contrast = P109_CONTRAST_MED; precharge = 0x1F; comdetect = 64;
      break;
    case P109_CONTRAST_HIGH:
    default:
      contrast = P109_CONTRAST_HIGH; precharge = 241; comdetect = 64;
      break;
  }

  if (P109_display) {
    P109_display->displayOn();
    P109_display->setContrast(contrast, precharge, comdetect);
  }
}

void P109_display_header() {
  static boolean showWiFiName = true;

  if (showWiFiName && (WiFiEventData.WiFiServicesInitialized())) {
    String newString = WiFi.SSID();
    newString.trim();
    P109_display_title(newString);
  } else {
    String dtime     = "%sysname%";
    String newString = parseTemplate(dtime, 10);
    newString.trim();
    P109_display_title(newString);
  }
  showWiFiName = !showWiFiName;

  // Display time and wifibars both clear area below, so paint them after the title.
  P109_display_time();
  P109_display_wifibars();
}

void P109_display_time() {
  String dtime     = "%systime%";
  String newString = parseTemplate(dtime, 10);

  P109_display->setTextAlignment(TEXT_ALIGN_LEFT);
  P109_display->setFont(getDialog_plain_12());
  P109_display->setColor(BLACK);
  P109_display->fillRect(0, 0, 28, 13);
  P109_display->setColor(WHITE);
  P109_display->drawString(0, 0, newString.substring(0, 5));
}

void P109_display_title(String& title) {
  P109_display->setTextAlignment(TEXT_ALIGN_CENTER);
  P109_display->setFont(getDialog_plain_12());
  P109_display->setColor(BLACK);
  P109_display->fillRect(0, 0, 128, 15); // Underscores use a extra lines, clear also.
  P109_display->setColor(WHITE);
  P109_display->drawString(64, 0, title);
}

// Draw Signal Strength Bars, return true when there was an update.
bool P109_display_wifibars() {
  const bool connected    = WiFiEventData.WiFiServicesInitialized();
  const int  nbars_filled = (WiFi.RSSI() + 100) / 8;
  const int  newState     = connected ? nbars_filled : P109_WIFI_STATE_UNSET;

  if (newState == P109_lastWiFiState) {
    return false; // nothing to do.
  }
  int x         = 105;
  int y         = 0;
  int size_x    = 15;
  int size_y    = 10;
  int nbars     = 5;
  int16_t width = (size_x / nbars);

  size_x = width * nbars - 1; // Correct for round errors.

  //  x,y are the x,y locations
  //  sizex,sizey are the sizes (should be a multiple of the number of bars)
  //  nbars is the number of bars and nbars_filled is the number of filled bars.

  //  We leave a 1 pixel gap between bars
  P109_display->setColor(BLACK);
  P109_display->fillRect(x, y, size_x, size_y);
  P109_display->setColor(WHITE);

  if (WiFiEventData.WiFiServicesInitialized()) {
    for (byte ibar = 0; ibar < nbars; ibar++) {
      int16_t height = size_y * (ibar + 1) / nbars;
      int16_t xpos   = x + ibar * width;
      int16_t ypos   = y + size_y - height;

      if (ibar <= nbars_filled) {
        // Fill complete bar
        P109_display->fillRect(xpos, ypos, width - 1, height);
      } else {
        // Only draw top and bottom.
        P109_display->fillRect(xpos, ypos,           width - 1, 1);
        P109_display->fillRect(xpos, y + size_y - 1, width - 1, 1);
      }
    }
  } else {
    // Draw a not connected sign.
  }
  return true;
}

void P109_display_current_temp() {
  String tmpString = P109_deviceTemplate[0];
  String atempstr  = parseTemplate(tmpString, 20);

  atempstr.trim();

  if (atempstr.length() > 0) {
    float atemp = atempstr.toFloat();
    atemp = (roundf(atemp * 10)) / 10.0f;

    if (Plugin_109_prev_temp != atemp) {
      P109_display->setColor(BLACK);
      P109_display->fillRect(3, 19, 47, 25);
      P109_display->setColor(WHITE);
      tmpString = toString(atemp, 1);
      P109_display->setFont(getArialMT_Plain_24());
      P109_display->drawString(3, 19, tmpString.substring(0, 5));
      Plugin_109_prev_temp = atemp;
    }
  }
}

void P109_display_setpoint_temp(byte force) {
  if (UserVar[Plugin_109_varindex + 2] == 1) {
    float stemp = (roundf(UserVar[Plugin_109_varindex] * 10)) / 10.0f;

    if ((Plugin_109_prev_setpoint != stemp) || (force == 1)) {
      P109_display->setColor(BLACK);
      P109_display->fillRect(86, 35, 41, 21);
      P109_display->setColor(WHITE);
      String tmpString = toString(stemp, 1);
      P109_display->setFont(getDialog_plain_18());
      P109_display->drawString(86, 35, tmpString.substring(0, 5));
      Plugin_109_prev_setpoint = stemp;
      Plugin_109_changed       = 1;
    }
  }
}

void P109_display_timeout() {
  if (UserVar[Plugin_109_varindex + 2] == 2) {
    if (Plugin_109_prev_timeout >= (UserVar[Plugin_109_varindex + 3] + 60)) {
      float  timeinmin = UserVar[Plugin_109_varindex + 3] / 60;
      String thour     = toString((static_cast<int>(timeinmin / 60)), 0);
      thour += ':';
      String thour2 = toString((static_cast<int>(timeinmin) % 60), 0);

      if (thour2.length() < 2) {
        thour += '0';
      }
      thour += thour2;
      P109_display->setColor(BLACK);
      P109_display->fillRect(86, 35, 41, 21);
      P109_display->setColor(WHITE);
      P109_display->setFont(getDialog_plain_18());
      P109_display->drawString(86, 35, thour.substring(0, 5));
      Plugin_109_prev_timeout = UserVar[Plugin_109_varindex + 3];
    }
  }
}

void P109_display_mode() {
  if (Plugin_109_prev_mode != UserVar[Plugin_109_varindex + 2]) {
    String tmpString;

    switch (int(UserVar[Plugin_109_varindex + 2]))
    {
      case 0:
      {
        tmpString = F("X");
        break;
      }
      case 1:
      {
        tmpString = F("A");
        break;
      }
      case 2:
      {
        tmpString = F("M");
        break;
      }
    }
    P109_display->setColor(BLACK);
    P109_display->fillRect(61, 49, 12, 17);
    P109_display->setColor(WHITE);
    P109_display->setFont(getArialMT_Plain_16());
    P109_display->drawString(61, 49, tmpString.substring(0, 5));
    Plugin_109_prev_mode = UserVar[Plugin_109_varindex + 2];
  }
}

void P109_display_heat() {
  if (Plugin_109_prev_heating != UserVar[Plugin_109_varindex + 1]) {
    P109_display->setColor(BLACK);
    P109_display->fillRect(54, 19, 24, 27);
    P109_display->setColor(WHITE);

    if (UserVar[Plugin_109_varindex + 1] == 1) {
      P109_display->drawXbm(54, 19, 24, 27, flameimg);
    }
    Plugin_109_prev_heating = UserVar[Plugin_109_varindex + 1];
  }
}

void P109_display_page() {
  // init with full clear
  P109_display->setColor(BLACK);
  P109_display->fillRect(0, 15, 128, 49);
  P109_display->setColor(WHITE);

  Plugin_109_prev_temp     = 99;
  Plugin_109_prev_setpoint = 0;
  Plugin_109_prev_heating  = 255;
  Plugin_109_prev_mode     = 255;
  Plugin_109_prev_timeout  = 32768;

  P109_display->setFont(getDialog_plain_12());
  P109_display->setTextAlignment(TEXT_ALIGN_LEFT);
  String tstr      = F("{D}C");
  String newString = parseTemplate(tstr, 10);

  P109_display->drawString(18, 46, newString.substring(0, 5));

  P109_display_heat();

  P109_display->drawHorizontalLine(0, 15, 128);
  P109_display->drawVerticalLine(52, 14, 49);

  P109_display->drawCircle(109, 47, 26);
  P109_display->drawHorizontalLine(78, 47, 8);
  P109_display->drawVerticalLine(109, 19, 10);

  P109_display_mode();
  P109_display_setpoint_temp(0);
  P109_display_current_temp();
}

void P109_setSetpoint(String sptemp) {
  float stemp = (roundf(UserVar[Plugin_109_varindex] * 10)) / 10.0f;

  if ((sptemp.charAt(0) == '+') || (sptemp.charAt(0) == 'p'))  {
    stemp = stemp + sptemp.substring(1).toFloat();
  } else if ((sptemp.charAt(0) == '-') || (sptemp.charAt(0) == 'm')) {
    stemp = stemp - sptemp.substring(1).toFloat();
  } else {
    stemp = sptemp.toFloat();
  }
  UserVar[Plugin_109_varindex] = stemp;
  P109_display_setpoint_temp(0);
}

void P109_setHeatRelay(byte state) {
  uint8_t relaypin = Settings.TaskDevicePluginConfig[Plugin_109_taskindex][4];

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String  logstr   = F("Thermo : Set Relay");

    logstr += relaypin;
    logstr += F("=");
    logstr += state;
    addLogMove(LOG_LEVEL_INFO, logstr);
  }

  if (relaypin != -1) {
    pinMode(relaypin, OUTPUT);
    digitalWrite(relaypin, state);
  }
}

void P109_setHeater(String heater) {
  if ((heater.equals(F("1"))) || (heater.equals(F("on")))) {
    UserVar[Plugin_109_varindex + 1] = 1;
    P109_setHeatRelay(HIGH);
  } else if ((heater.equals(F("0"))) || (heater.equals(F("off")))) {
    UserVar[Plugin_109_varindex + 1] = 0;
    P109_setHeatRelay(LOW);
  } else if (UserVar[Plugin_109_varindex + 1] == 0) {
    UserVar[Plugin_109_varindex + 1] = 1;
    P109_setHeatRelay(HIGH);
  } else {
    UserVar[Plugin_109_varindex + 1] = 0;
    P109_setHeatRelay(LOW);
  }
  P109_display_heat();
}

void P109_setMode(String amode, String atimeout) {
  if ((amode.equals(F("0"))) || (amode.equals(F("x")))) {
    UserVar[Plugin_109_varindex + 2] = 0;
    P109_setHeater(F("0"));
    P109_display->setColor(BLACK);
    P109_display->fillRect(86, 35, 41, 21);
    Plugin_109_prev_setpoint = 0;
  } else if ((amode.equals(F("1"))) || (amode.equals(F("a")))) {
    UserVar[Plugin_109_varindex + 2] = 1;
    P109_display_setpoint_temp(1);
  } else if ((amode.equals(F("2"))) || (amode.equals(F("m")))) {
    UserVar[Plugin_109_varindex + 2] = 2;
    UserVar[Plugin_109_varindex + 3] = (atimeout.toFloat() * 60);
    Plugin_109_prev_timeout          = 32768;
    P109_display_timeout();
    P109_setHeater(F("1"));
  } else {
    UserVar[Plugin_109_varindex + 2] = 0;
  }
  Plugin_109_changed = 1;
  P109_display_mode();
}

>>>>>>> 884573f6976e2201b72494a1c4dde79723bf2ec0
#endif // ifdef USES_P109
