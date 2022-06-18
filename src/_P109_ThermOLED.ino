#include "_Plugin_Helper.h"
#ifdef USES_P109

# include "src/Globals/ESPEasyWiFiEvent.h"

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
   - oledframedcmd,[OLED_STATUS]               Inherited command from P036 status can be:
                                              [off/on/low/med/high]
   - thermo,setpoint,[target_temperature]      Target setpoint, only used in Mode "A"
   - thermo,heating,[RELAY_STATUS]             Manually forcing relay status [off/on]
   - thermo,mode,[MODE],[TIMEOUT]              Set to either mode X/A/M, if M selected,
                                              then TIMEOUT can be specified in minutes

   Command Examples :
   -  /control?cmd=thermo,setpoint,23          Set target setpoint to 23 Celsius
   -  /control?cmd=thermo,mode,1               Set mode to AUTOMATIC so it starts to maintain setpoint temperature
   -  /control?cmd=thermo,mode,2,5             Starts pre-heat for 5 minute, does not care about TEMP, then go to AUTO mode after timeout
   -  /control?cmd=thermo,mode,0               Switch heating off, absolutely do nothing until further notice

   ------------------------------------------------------------------------------------------
   Copyleft Nagy Sándor 2018 - https://bitekmindenhol.blog.hu/
   ------------------------------------------------------------------------------------------
   2022-06-17 tonhuisman: Optimizations
   2022-06-18 tonhuisman: More optimizations, use #defines where appropriate
 */

# define PLUGIN_109
# define PLUGIN_ID_109          109
# define PLUGIN_NAME_109        "Display - OLED SSD1306/SH1106 Thermo"
# define PLUGIN_VALUENAME1_109  "setpoint"
# define PLUGIN_VALUENAME2_109  "heating"
# define PLUGIN_VALUENAME3_109  "mode"
# define PLUGIN_VALUENAME4_109  "timeout"

# define P109_Nlines            1
# define P109_Nchars            32

# define P109_CONTRAST_OFF      1
# define P109_CONTRAST_LOW      64
# define P109_CONTRAST_MED      0xCF
# define P109_CONTRAST_HIGH     0xFF

# include "SSD1306.h"
# include "SH1106Wire.h"
# include "Dialog_Plain_12_font.h"
# include "Dialog_Plain_18_font.h"

const char flameimg[] PROGMEM = {
  0x00, 0x20, 0x00,
  0x00, 0x70, 0x00,
  0x00, 0x78, 0x00,
  0x00, 0x7c, 0x00,
  0x00, 0x7c, 0x00,
  0x80, 0x7f, 0x00,
  0xc0, 0xff, 0x00,
  0xc0, 0xff, 0x00,
  0xe0, 0xff, 0x04,
  0xe0, 0xff, 0x05,
  0xf0, 0xff, 0x0f,
  0xf0, 0xff, 0x0f,
  0xf8, 0xff, 0x0f,
  0xf8, 0xff, 0x1f,
  0xf8, 0xff, 0x1f,
  0xf8, 0xff, 0x3f,
  0xf8, 0xff, 0x3f,
  0xf8, 0xf3, 0x3f,
  0xf8, 0xf1, 0x3f,
  0xf8, 0xf1, 0x3f,
  0xf8, 0xe1, 0x3f,
  0xf0, 0x21, 0x3f,
  0xf0, 0x01, 0x1f,
  0xe0, 0x01, 0x0f,
  0xc0, 0x01, 0x0f,
  0x80, 0x01, 0x07,
  0x00, 0x03, 0x01
};

# define P109_WIFI_STATE_UNSET          -2
# define P109_WIFI_STATE_NOT_CONNECTED  -1
# define P109_TEMP_STATE_UNSET          99.0f
# define P109_SETPOINT_STATE_UNSET      0.0f
# define P109_SETPOINT_STATE_INITIAL    19.0f
# define P109_TIMEOUT_STATE_UNSET       32768.0f
# define P109_HEATING_STATE_UNSET       255
# define P109_MODE_STATE_UNSET          255
# define P109_MODE_STATE_INITIAL        1
# define P109_BUTTON_DEBOUNCE_TIME_MS   300

# define P109_CONFIG_I2CADDRESS         PCONFIG(0)
# define P109_CONFIG_ROTATION           PCONFIG(1)
# define P109_CONFIG_DISPLAYTYPE        PCONFIG(2)
# define P109_CONFIG_CONTRAST           PCONFIG(3)
# define P109_CONFIG_RELAYPIN           PCONFIG(4)

float   Plugin_109_prev_temp = P109_TEMP_STATE_UNSET;
float   Plugin_109_prev_setpoint;
float   Plugin_109_prev_timeout;
uint8_t Plugin_109_prev_heating;
uint8_t Plugin_109_prev_mode;

uint8_t  Plugin_109_taskindex;
uint8_t  Plugin_109_varindex;
uint8_t  Plugin_109_changed;
boolean  Plugin_109_init         = false;
uint32_t Plugin_109_lastsavetime = 0;
uint8_t  Plugin_109_saveneeded   = 0;

static uint32_t Plugin_109_buttons[3];

static int8_t P109_lastWiFiState = P109_WIFI_STATE_UNSET;

// Instantiate display here - does not work to do this within the INIT call

OLEDDisplay *P109_display = nullptr;

char P109_deviceTemplate[P109_Nlines][P109_Nchars];

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
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 4;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].TimerOptional      = false;
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

    case PLUGIN_WEBFORM_LOAD:
    {
      const __FlashStringHelper *options5[] = { F("SSD1306"), F("SH1106") };
      const int optionValues5[]             = { 1, 2 };
      addFormSelector(F("Controller"), F("controler"), 2, options5, optionValues5, P109_CONFIG_DISPLAYTYPE);

      const __FlashStringHelper *options1[] = { F("Normal"), F("Rotated") };
      const int optionValues1[]             = { 1, 2 };
      addFormSelector(F("Rotation"), F("rotate"), 2, options1, optionValues1, P109_CONFIG_ROTATION);

      LoadCustomTaskSettings(event->TaskIndex, reinterpret_cast<uint8_t *>(&P109_deviceTemplate), sizeof(P109_deviceTemplate));

      for (uint8_t varNr = 0; varNr < P109_Nlines; varNr++) {
        String label = F("Line ");

        if (varNr == 0) {
          label = F("Temperature source ");
        }
        label += varNr + 1;
        addFormTextBox(label,
                       getPluginCustomArgName(varNr + 1),
                       P109_deviceTemplate[varNr],
                       P109_Nchars);
      }

      addFormPinSelect(F("Button left"),  F("taskdevicepin1"), CONFIG_PIN1);
      addFormPinSelect(F("Button right"), F("taskdevicepin2"), CONFIG_PIN2);
      addFormPinSelect(F("Button mode"),  F("taskdevicepin3"), CONFIG_PIN3);

      addFormPinSelect(F("Relay"),        F("heatrelay"),      P109_CONFIG_RELAYPIN);

      uint8_t choice6 = P109_CONFIG_CONTRAST;

      if (choice6 == 0) { choice6 = P109_CONTRAST_HIGH; }
      const __FlashStringHelper *options6[] = { F("Low"), F("Medium"), F("High") };
      const int optionValues6[]             = { P109_CONTRAST_LOW, P109_CONTRAST_MED, P109_CONTRAST_HIGH };
      addFormSelector(F("Contrast"), F("contrast"), 3, options6, optionValues6, choice6);

      const __FlashStringHelper *options4[] = { F("0.2"), F("0.5"), F("1") };
      const int optionValues4[]             = { 2, 5, 10 };
      addFormSelector(F("Hysteresis"), F("hyst"), 3, options4, optionValues4, static_cast<int>(PCONFIG_FLOAT(0) * 10.0f));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P109_CONFIG_I2CADDRESS  = getFormItemInt(F("pi2caddr"));
      P109_CONFIG_ROTATION    = getFormItemInt(F("rotate"));
      P109_CONFIG_DISPLAYTYPE = getFormItemInt(F("controler"));
      P109_CONFIG_CONTRAST    = getFormItemInt(F("contrast"));
      P109_CONFIG_RELAYPIN    = getFormItemInt(F("heatrelay"));
      PCONFIG_FLOAT(0)        = (getFormItemInt(F("hyst")) / 10.0f);

      for (uint8_t varNr = 0; varNr < P109_Nlines; varNr++) {
        strncpy(P109_deviceTemplate[varNr],
                web_server.arg(getPluginCustomArgName(varNr + 1)).c_str(),
                sizeof(P109_deviceTemplate[varNr]) - 1);
        P109_deviceTemplate[varNr][sizeof(P109_deviceTemplate[varNr]) - 1] = 0;
      }

      SaveCustomTaskSettings(event->TaskIndex, reinterpret_cast<uint8_t *>(&P109_deviceTemplate), sizeof(P109_deviceTemplate));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      P109_lastWiFiState = P109_WIFI_STATE_UNSET;

      // Load the custom settings from flash
      LoadCustomTaskSettings(event->TaskIndex, reinterpret_cast<uint8_t *>(&P109_deviceTemplate), sizeof(P109_deviceTemplate));

      //      Init the display and turn it on
      if (P109_display) {
        P109_display->end();
        delete P109_display;
        P109_display = nullptr;
      }
      Plugin_109_taskindex = event->TaskIndex;
      Plugin_109_varindex  = event->BaseVarIndex;

      if (P109_CONFIG_DISPLAYTYPE == 1) {
        P109_display = new (std::nothrow) SSD1306Wire(P109_CONFIG_I2CADDRESS, Settings.Pin_i2c_sda, Settings.Pin_i2c_scl);
      } else {
        P109_display = new (std::nothrow) SH1106Wire(P109_CONFIG_I2CADDRESS, Settings.Pin_i2c_sda, Settings.Pin_i2c_scl);
      }

      if (P109_display == nullptr) {
        break;
      }
      P109_display->init(); // call to local override of init function
      P109_display->displayOn();

      uint8_t OLED_contrast = Settings.TaskDevicePluginConfig[event->TaskIndex][3];
      P109_setContrast(OLED_contrast);

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

      Plugin_109_prev_temp = P109_TEMP_STATE_UNSET;

      fs::File f = tryOpenFile(F("thermo.dat"), F("r"));

      if (f) {
        f.read(reinterpret_cast<uint8_t *>(&UserVar[event->BaseVarIndex]), 16);
        f.close();
      }
      Plugin_109_lastsavetime = millis();

      if (UserVar[event->BaseVarIndex] < 1) {
        UserVar[event->BaseVarIndex]     = P109_SETPOINT_STATE_INITIAL; // setpoint
        UserVar[event->BaseVarIndex + 2] = P109_MODE_STATE_INITIAL;     // mode (X=0,A=1,M=2)
      }

      // UserVar[event->BaseVarIndex + 1] = 0; // heating (0=off,1=heating in progress)
      // UserVar[event->BaseVarIndex + 3] = 0; // timeout (manual on for minutes)
      if (P109_CONFIG_RELAYPIN != -1) {
        // pinMode(Settings.TaskDevicePluginConfig[event->TaskIndex][4], OUTPUT);
        P109_setHeatRelay(static_cast<uint8_t>(UserVar[event->BaseVarIndex + 1]));
      }

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String logstr = F("Thermo : Starting status S:");
        logstr += toString(UserVar[event->BaseVarIndex]);
        logstr += F(", R:");
        logstr += toString(UserVar[event->BaseVarIndex + 1]);
        addLogMove(LOG_LEVEL_INFO, logstr);
      }

      Plugin_109_changed    = 1;
      Plugin_109_buttons[0] = 0;
      Plugin_109_buttons[1] = 0;
      Plugin_109_buttons[2] = 0;

      //      flip screen if required
      if (P109_CONFIG_ROTATION == 2) { P109_display->flipScreenVertically(); }

      //      Display the device name, logo, time and wifi
      P109_display_header();
      P109_display_page();
      P109_display->display();
      Plugin_109_init = true;

      success = true;
      break;
    }

    case PLUGIN_EXIT:
    {
      if (P109_display) {
        P109_display->end();
        delete P109_display;
        P109_display    = nullptr;
        Plugin_109_init = false;
      }
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      uint32_t current_time;

      if (Plugin_109_init) {
        if (validGpio(CONFIG_PIN1) && !digitalRead(CONFIG_PIN1)) {
          current_time = millis();

          if (Plugin_109_buttons[0] + P109_BUTTON_DEBOUNCE_TIME_MS < current_time) {
            Plugin_109_buttons[0] = current_time;

            switch (int(UserVar[event->BaseVarIndex + 2])) {
              case 0: { // off mode, no func
                break;
              }
              case 1: { // auto mode, setpoint dec
                P109_setSetpoint(F("-0.5"));
                break;
              }
              case 2: { // manual on mode, timer dec
                UserVar[event->BaseVarIndex + 3] = UserVar[event->BaseVarIndex + 3] - P109_BUTTON_DEBOUNCE_TIME_MS;

                if (UserVar[event->BaseVarIndex + 3] < 0) {
                  UserVar[event->BaseVarIndex + 3] = 5400;
                }
                Plugin_109_prev_timeout = P109_TIMEOUT_STATE_UNSET;
                Plugin_109_changed      = 1;
                break;
              }
            }
          }
        }

        if (validGpio(CONFIG_PIN2) && !digitalRead(CONFIG_PIN2)) {
          current_time = millis();

          if (Plugin_109_buttons[1] + P109_BUTTON_DEBOUNCE_TIME_MS < current_time) {
            Plugin_109_buttons[1] = current_time;

            switch (int(UserVar[event->BaseVarIndex + 2])) {
              case 0: { // off mode, no func
                break;
              }
              case 1: { // auto mode, setpoint inc
                P109_setSetpoint(F("+0.5"));
                break;
              }
              case 2: { // manual on mode, timer dec
                UserVar[event->BaseVarIndex + 3] = UserVar[event->BaseVarIndex + 3] + P109_BUTTON_DEBOUNCE_TIME_MS;

                if (UserVar[event->BaseVarIndex + 3] > 5400) {
                  UserVar[event->BaseVarIndex + 3] = 60;
                }
                Plugin_109_prev_timeout = P109_TIMEOUT_STATE_UNSET;
                Plugin_109_changed      = 1;
                break;
              }
            }
          }
        }

        if (validGpio(CONFIG_PIN3) && !digitalRead(CONFIG_PIN3)) {
          current_time = millis();

          if (Plugin_109_buttons[2] + P109_BUTTON_DEBOUNCE_TIME_MS < current_time) {
            Plugin_109_buttons[2] = current_time;

            switch (int(UserVar[event->BaseVarIndex + 2])) {
              case 0: { // off mode, next
                P109_setMode(F("a"), F("0"));
                break;
              }
              case 1: { // auto mode, next
                P109_setMode(F("m"), F("5"));
                break;
              }
              case 2: { // manual on mode, next
                P109_setMode(F("x"), F("0"));
                break;
              }
            }
          }
        }
      }
      break;
    }

    // Switch off display after displayTimer seconds
    case PLUGIN_ONCE_A_SECOND:
    {
      if (Plugin_109_init) {
        if (P109_display && P109_display_wifibars()) {
          // WiFi symbol was updated.
          P109_display->display();
        }

        if (UserVar[event->BaseVarIndex + 2] == 2) { // manual timeout
          if (UserVar[event->BaseVarIndex + 3] > 0) {
            UserVar[event->BaseVarIndex + 3] = UserVar[event->BaseVarIndex + 3] - 1;
            P109_display_timeout();
          } else {
            UserVar[event->BaseVarIndex + 3] = 0;
            P109_setMode(F("a"), F("0")); // heater to auto
            P109_display_setpoint_temp(1);
          }
        }

        if (Plugin_109_changed == 1) {
          sendData(event);
          Plugin_109_saveneeded = 1;
          Plugin_109_changed    = 0;
        }

        if ((Plugin_109_saveneeded == 1) && ((Plugin_109_lastsavetime + 30000) < millis())) {
          Plugin_109_saveneeded   = 0;
          Plugin_109_lastsavetime = millis();
          fs::File f = tryOpenFile(F("thermo.dat"), F("w"));

          if (f) {
            f.write(reinterpret_cast<const uint8_t *>(&UserVar[event->BaseVarIndex]), 16);
            f.close();
            flashCount();
          }
          addLog(LOG_LEVEL_INFO, F("Thermo : Save UserVars to SPIFFS"));
        }
        success = true;

        break;
      }
    }

    case PLUGIN_READ:
    {
      if (Plugin_109_init) {
        //      Update display
        P109_display_header();

        if (UserVar[event->BaseVarIndex + 2] == 1) {
          String atempstr2 = P109_deviceTemplate[0];
          String atempstr  = parseTemplate(atempstr2, 20);
          atempstr.trim();

          if ((atempstr.length() > 0) &&
              (Plugin_109_prev_temp != P109_TEMP_STATE_UNSET)) { // do not switch until the first temperature data arrives
            float atemp = atempstr.toFloat();

            if (!essentiallyEqual(atemp, 0.0f)) {
              if (definitelyGreaterThan(UserVar[event->BaseVarIndex], atemp) &&
                  (UserVar[event->BaseVarIndex + 1] < 1)) {
                P109_setHeater(F("1"));
                Plugin_109_changed = 1;
              } else if (!definitelyLessThan(UserVar[event->BaseVarIndex], atemp - PCONFIG_FLOAT(0)) &&
                         (UserVar[event->BaseVarIndex + 1] > 0)) {
                P109_setHeater(F("0"));
                Plugin_109_changed = 1;
              } else {
                P109_display_heat();
              }
            }
          } else {
            P109_display_heat();
          }
        }

        P109_display_current_temp();
        P109_display_timeout();

        P109_display->display();

        success = true;
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      String command    = parseString(string, 1);
      String subcommand = parseString(string, 2);
      String logstr;

      if (Plugin_109_init) {
        if (command == F("oledframedcmd")) {
          success = true;

          if (subcommand == F("off")) {
            P109_setContrast(P109_CONTRAST_OFF);
          }
          else if (subcommand == F("on")) {
            P109_display->displayOn();
          }
          else if (subcommand == F("low")) {
            P109_setContrast(P109_CONTRAST_LOW);
          }
          else if (subcommand == F("med")) {
            P109_setContrast(P109_CONTRAST_MED);
          }
          else if (subcommand == F("high")) {
            P109_setContrast(P109_CONTRAST_HIGH);
          }
          logstr = F("\nOk");
          SendStatus(event, logstr);
        }

        if (command == F("thermo")) {
          success = true;
          String par1 = parseString(string, 3);

          if (subcommand == F("setpoint")) {
            P109_setSetpoint(par1);
          }
          else if (subcommand == F("heating")) {
            P109_setHeater(par1); Plugin_109_changed = 1;
          }
          else if (subcommand == F("mode")) {
            P109_setMode(par1, parseString(string, 4));
          }
          logstr = F("\nOk");
          SendStatus(event, logstr);
        }
      }
      break;
    }
  }

  return success;
}

// Set the display contrast
// really low brightness & contrast: contrast = 10, precharge = 5, comdetect = 0
// normal brightness & contrast:  contrast = 100
void P109_setContrast(const uint8_t& OLED_contrast) {
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

void P109_display_title(const String& title) {
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
  P109_display->setColor(BLACK);
  P109_display->fillRect(x, y, size_x, size_y);
  P109_display->setColor(WHITE);

  if (WiFiEventData.WiFiServicesInitialized()) {
    for (uint8_t ibar = 0; ibar < nbars; ibar++) {
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
    atemp = (round(atemp * 10.0f)) / 10.0f;

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

void P109_display_setpoint_temp(const uint8_t& force) {
  if (UserVar[Plugin_109_varindex + 2] == 1) {
    float stemp = (round(UserVar[Plugin_109_varindex] * 10.0f)) / 10.0f;

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
      float  timeinmin = UserVar[Plugin_109_varindex + 3] / 60.0f;
      String thour     = toString((static_cast<int>(timeinmin / 60.0f)), 0);
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

    switch (static_cast<int>(UserVar[Plugin_109_varindex + 2])) {
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

  Plugin_109_prev_temp     = P109_TEMP_STATE_UNSET;
  Plugin_109_prev_setpoint = P109_SETPOINT_STATE_UNSET;
  Plugin_109_prev_heating  = P109_HEATING_STATE_UNSET;
  Plugin_109_prev_mode     = P109_MODE_STATE_UNSET;
  Plugin_109_prev_timeout  = P109_TIMEOUT_STATE_UNSET;

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

void P109_setSetpoint(const String& sptemp) {
  float stemp = (round(UserVar[Plugin_109_varindex] * 10.0f)) / 10.0f;

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

void P109_setHeatRelay(const uint8_t& state) {
  uint8_t relaypin = Settings.TaskDevicePluginConfig[Plugin_109_taskindex][4];

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String logstr = F("Thermo : Set Relay");

    logstr += relaypin;
    logstr += '=';
    logstr += state;
    addLogMove(LOG_LEVEL_INFO, logstr);
  }

  if (relaypin != -1) {
    pinMode(relaypin, OUTPUT);
    digitalWrite(relaypin, state);
  }
}

void P109_setHeater(const String& heater) {
  if ((heater[0] == '1') || (heater == F("on")) ||
      ((heater.length() == 0) && (UserVar[Plugin_109_varindex + 1] == 0))) {
    UserVar[Plugin_109_varindex + 1] = 1;
    P109_setHeatRelay(HIGH);
  } else {
    UserVar[Plugin_109_varindex + 1] = 0;
    P109_setHeatRelay(LOW);
  }
  P109_display_heat();
}

void P109_setMode(const String& amode, const String& atimeout) {
  if ((amode[0] == '0') || (amode[0] == 'x')) {
    UserVar[Plugin_109_varindex + 2] = 0;
    P109_setHeater(F("0"));
    P109_display->setColor(BLACK);
    P109_display->fillRect(86, 35, 41, 21);
    Plugin_109_prev_setpoint = P109_SETPOINT_STATE_UNSET;
  } else if ((amode[0] == '1') || (amode[0] == 'a')) {
    UserVar[Plugin_109_varindex + 2] = 1;
    P109_display_setpoint_temp(1);
  } else if ((amode[0] == '2') || (amode[0] == 'm')) {
    UserVar[Plugin_109_varindex + 2] = 2;
    UserVar[Plugin_109_varindex + 3] = (atimeout.toFloat() * 60);
    Plugin_109_prev_timeout          = P109_TIMEOUT_STATE_UNSET;
    P109_display_timeout();
    P109_setHeater(F("1"));
  } else {
    UserVar[Plugin_109_varindex + 2] = 0;
  }
  Plugin_109_changed = 1;
  P109_display_mode();
}

#endif // ifdef USES_P109
