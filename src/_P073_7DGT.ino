#ifdef USES_P073

// #######################################################################################################
// ###################   Plugin 073 - 7-segment display plugin TM1637/MAX7219       ######################
// #######################################################################################################
//
// Chips/displays supported:
//  0 - TM1637     -- 2 pins - 4 digits and colon in the middle (XX:XX)
//  1 - TM1637     -- 2 pins - 4 digits and dot on each digit (X.X.X.X.)
//  2 - TM1637     -- 2 pins - 6 digits and dot on each digit (X.X.X.X.X.X.)
//  3 - MAX7219/21 -- 3 pins - 8 digits and dot on each digit (X.X.X.X.X.X.X.X.)
//
// Plugin can be setup as:
//  - Manual        -- display is manually updated sending commands
//                     "7dn,<number>"        (number can be negative or positive, even with decimal)
//                     "7dt,<temperature>"   (temperature can be negative or positive and containing decimals)
//                     "7dst,<hh>,<mm>,<ss>" (show manual time -not current-, no checks done on numbers validity!)
//                     "7dsd,<dd>,<mm>,<yy>" (show manual date -not current-, no checks done on numbers validity!)
//                     "7dtext,<text>"       (show free text - supported chars 0-9,a-z,A-Z," ","-","=","_","/","^")
//  - Clock-Blink     -- display is automatically updated with current time and blinking dot/lines
//  - Clock-NoBlink   -- display is automatically updated with current time and steady dot/lines
//  - Clock12-Blink   -- display is automatically updated with current time (12h clock) and blinking dot/lines
//  - Clock12-NoBlink -- display is automatically updated with current time (12h clock) and steady dot/lines
//  - Date            -- display is automatically updated with current date
//
// Generic commands:
//  - "7don"      -- turn ON the display
//  - "7doff"     -- turn OFF the display
//  - "7db,<0-15> -- set brightness to specific value between 0 and 15
//

#define PLUGIN_073
#define PLUGIN_ID_073           73
#define PLUGIN_NAME_073         "Display - 7-segment display"
#define PLUGIN_073_DEBUG        false // activate extra log info in the debug

#define P073_TM1637_4DGTCOLON   0
#define P073_TM1637_4DGTDOTS    1
#define P073_TM1637_6DGT        2
#define P073_MAX7219_8DGT       3

#define P073_DISP_MANUAL        0
#define P073_DISP_CLOCK24BLNK   1
#define P073_DISP_CLOCK24       2
#define P073_DISP_CLOCK12BLNK   3
#define P073_DISP_CLOCK12       4
#define P073_DISP_DATE          5

#include "_Plugin_Helper.h"
struct P073_data_struct : public PluginTaskData_base {
  P073_data_struct()
    : dotpos(-1), pin1(-1), pin2(-1), pin3(-1), displayModel(0), output(0),
    brightness(0), timesep(false), shift(false) {
    ClearBuffer();
  }

  void FillBufferWithTime(boolean sevendgt_now, byte sevendgt_hours,
                          byte sevendgt_minutes, byte sevendgt_seconds,
                          boolean flag12h) {
    ClearBuffer();

    if (sevendgt_now) {
      sevendgt_hours   = node_time.hour();
      sevendgt_minutes = node_time.minute();
      sevendgt_seconds = node_time.second();
    }

    if (flag12h && (sevendgt_hours > 12)) {
      sevendgt_hours -= 12; // if flag 12h is TRUE and h>12 adjust subtracting 12
    }

    if (flag12h && (sevendgt_hours == 0)) {
      sevendgt_hours = 12; // if flag 12h is TRUE and h=0  adjust to h=12
    }
    showbuffer[0] = static_cast<uint8_t>(sevendgt_hours / 10);
    showbuffer[1] = sevendgt_hours % 10;
    showbuffer[2] = static_cast<uint8_t>(sevendgt_minutes / 10);
    showbuffer[3] = sevendgt_minutes % 10;
    showbuffer[4] = static_cast<uint8_t>(sevendgt_seconds / 10);
    showbuffer[5] = sevendgt_seconds % 10;
  }

  void FillBufferWithDate(boolean sevendgt_now, byte sevendgt_day,
                          byte sevendgt_month, int sevendgt_year) {
    ClearBuffer();
    int sevendgt_year0 = sevendgt_year;

    if (sevendgt_now) {
      sevendgt_day   = node_time.day();
      sevendgt_month = node_time.month();
      sevendgt_year0 = node_time.year();
    } else {
      if (sevendgt_year0 < 100) {
        sevendgt_year0 += 2000;
      }
    }
    byte sevendgt_year1 = static_cast<uint8_t>(sevendgt_year0 / 100);
    byte sevendgt_year2 = static_cast<uint8_t>(sevendgt_year0 % 100);

    showbuffer[0] = static_cast<uint8_t>(sevendgt_day / 10);
    showbuffer[1] = sevendgt_day % 10;
    showbuffer[2] = static_cast<uint8_t>(sevendgt_month / 10);
    showbuffer[3] = sevendgt_month % 10;
    showbuffer[4] = static_cast<uint8_t>(sevendgt_year1 / 10);
    showbuffer[5] = sevendgt_year1 % 10;
    showbuffer[6] = static_cast<uint8_t>(sevendgt_year2 / 10);
    showbuffer[7] = sevendgt_year2 % 10;
  }

  void FillBufferWithNumber(const String& number) {
    ClearBuffer();
    byte p073_numlenght = number.length();
    byte p073_index     = 7;
    dotpos = -1; // -1 means no dot to display

    for (int i = p073_numlenght - 1; i >= 0 && p073_index >= 0; --i) {
      char p073_tmpchar = number.charAt(i);

      if (p073_tmpchar == '.') { // dot
        dotpos = p073_index;
      } else {
        showbuffer[p073_index] = P073_mapCharToFontPosition(p073_tmpchar);
        p073_index--;
      }
    }
  }

  void FillBufferWithTemp(long temperature) {
    ClearBuffer();
    char p073_digit[8];
    sprintf(p073_digit, "%7d", static_cast<int>(temperature));
    int p073_numlenght = strlen(p073_digit);

    for (int i = 0; i < p073_numlenght; i++) {
      showbuffer[i] = P073_mapCharToFontPosition(p073_digit[i]);
    }
    showbuffer[7] = 12; // degree "°"
  }

  void FillBufferWithString(const String& textToShow) {
    ClearBuffer();
    String tmpText;
    int    p073_txtlength = textToShow.length();

    if (p073_txtlength > 8) {
      p073_txtlength = 8;
    }
    tmpText = textToShow.substring(0, p073_txtlength);

    for (int i = 0; i < p073_txtlength; i++) {
      showbuffer[i] = P073_mapCharToFontPosition(tmpText.charAt(i));
    }
  }

  // in case of error show all dashes
  void FillBufferWithDash() {
    memset(showbuffer, 11, sizeof(showbuffer));
  }

  void ClearBuffer() {
    memset(showbuffer, 10, sizeof(showbuffer));
  }

  int     dotpos;
  uint8_t showbuffer[8];
  byte    spidata[2];
  uint8_t pin1, pin2, pin3;
  byte    displayModel;
  byte    output;
  byte    brightness;
  bool    timesep;
  bool    shift;
};

#define TM1637_POWER_ON B10001000
#define TM1637_POWER_OFF B10000000
#define TM1637_CLOCKDELAY 40
#define TM1637_4DIGIT 4
#define TM1637_6DIGIT 2

// each char table is specific for each display and maps all numbers/symbols
// needed:
//   - pos 0-9   - Numbers from 0 to 9
//   - pos 10    - Space " "
//   - pos 11    - minus symbol "-"
//   - pos 12    - degree symbol "°"
//   - pos 13    - equal "="
//   - pos 14    - triple lines "/"
//   - pos 15    - underscore "_"
//   - pos 16-41 - Letters from A to Z
static const byte CharTableTM1637[42]  = {
  B00111111, B00000110, B01011011, B01001111, B01100110, B01101101,
  B01111101, B00000111, B01111111, B01101111, B00000000, B01000000,
  B01100011, B01001000, B01001001, B00001000, B01110111, B01111100,
  B00111001, B01011110, B01111001, B01110001, B00111101, B01110110,
  B00110000, B00011110, B01110101, B00111000, B00010101, B00110111,
  B00111111, B01110011, B01101011, B00110011, B01101101, B01111000,
  B00111110, B00111110, B00101010, B01110110, B01101110, B01011011 };
static const byte CharTableMAX7219[42] = {
  B01111110, B00110000, B01101101, B01111001, B00110011, B01011011,
  B01011111, B01110000, B01111111, B01111011, B00000000, B00000001,
  B01100011, B00001001, B01001001, B00001000, B01110111, B00011111,
  B01001110, B00111101, B01001111, B01000111, B01011110, B00110111,
  B00000110, B00111100, B01010111, B00001110, B01010100, B01110110,
  B01111110, B01100111, B01101011, B01100110, B01011011, B00001111,
  B00111110, B00111110, B00101010, B00110111, B00111011, B01101101 };

uint8_t P073_mapCharToFontPosition(char character) {
  uint8_t position = 10;

  if ((character >= '0') && (character <= '9')) {
    position = character - '0';
  } else if ((character >= 'A') && (character <= 'Z')) {
    position = character - 'A' + 16;
  } else if ((character >= 'a') && (character <= 'z')) {
    position = character - 'a' + 16;
  } else {
    switch (character) {
      case ' ': position = 10; break;
      case '-': position = 11; break;
      case '^': position = 12; break; // degree
      case '=': position = 13; break;
      case '/': position = 14; break;
      case '_': position = 15; break;
    }
  }
  return position;
}

boolean Plugin_073(byte function, struct EventStruct *event, String& string) {
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number           = PLUGIN_ID_073;
      Device[deviceCount].Type               = DEVICE_TYPE_TRIPLE;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_NONE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 0;
      Device[deviceCount].SendDataOption     = false;
      Device[deviceCount].TimerOption        = false;
      Device[deviceCount].TimerOptional      = false;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_073);
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      addFormNote(F("TM1637:  1st=CLK-Pin, 2nd=DIO-Pin"));
      addFormNote(F("MAX7219: 1st=DIN-Pin, 2nd=CLK-Pin, 3rd=CS-Pin"));
      String displtype[5] = { F("TM1637 - 4 digit (colon)"),
                              F("TM1637 - 4 digit (dots)"),
                              F("TM1637 - 6 digit"),
                              F("MAX7219 - 8 digit") };
      addFormSelector(F("Display Type"), F("plugin_073_displtype"), 4, displtype,
                      NULL, PCONFIG(0));
      String displout[6] = { F("Manual"),
                             F("Clock 24h - Blink"),
                             F("Clock 24h - No Blink"),
                             F("Clock 12h - Blink"),
                             F("Clock 12h - No Blink"),
                             F("Date") };
      addFormSelector(F("Display Output"), F("plugin_073_displout"), 6, displout,
                      NULL, PCONFIG(1));
      addFormNumericBox(F("Brightness"), F("plugin_073_brightness"), PCONFIG(2),
                        0, 15);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      PCONFIG(0) = getFormItemInt(F("plugin_073_displtype"));
      PCONFIG(1) = getFormItemInt(F("plugin_073_displout"));
      PCONFIG(2) = getFormItemInt(F("plugin_073_brightness"));
      P073_data_struct *P073_data =
        static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P073_data) {
        P073_data->pin1         = CONFIG_PIN1;
        P073_data->pin2         = CONFIG_PIN2;
        P073_data->pin3         = CONFIG_PIN3;
        P073_data->displayModel = PCONFIG(0);
        P073_data->output       = PCONFIG(1);
        P073_data->brightness   = PCONFIG(2);
        P073_data->timesep      = true;

        switch (PCONFIG(0)) {
          case P073_TM1637_4DGTCOLON: // set brightness of TM1637
          case P073_TM1637_4DGTDOTS:
          case P073_TM1637_6DGT: {
            int tm1637_bright = PCONFIG(2) / 2;
            tm1637_SetPowerBrightness(CONFIG_PIN1, CONFIG_PIN2, tm1637_bright,
                                      true);

            if (PCONFIG(1) == P073_DISP_MANUAL) {
              tm1637_ClearDisplay(CONFIG_PIN1, CONFIG_PIN2);
            }
            break;
          }
          case P073_MAX7219_8DGT: // set brightness of MAX7219
          {
            max7219_SetPowerBrightness(event, CONFIG_PIN1, CONFIG_PIN2, CONFIG_PIN3,
                                       PCONFIG(2), true);

            if (PCONFIG(1) == P073_DISP_MANUAL) {
              max7219_ClearDisplay(event, CONFIG_PIN1, CONFIG_PIN2, CONFIG_PIN3);
            }
            break;
          }
        }
      }
      success = true;
      break;
    }

    case PLUGIN_EXIT: {
      success = true;
      break;
    }

    case PLUGIN_INIT: {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P073_data_struct());
      P073_data_struct *P073_data =
        static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P073_data) {
        return success;
      }
      P073_data->pin1         = CONFIG_PIN1;
      P073_data->pin2         = CONFIG_PIN2;
      P073_data->pin3         = CONFIG_PIN3;
      P073_data->displayModel = PCONFIG(0);
      P073_data->output       = PCONFIG(1);
      P073_data->brightness   = PCONFIG(2);

      switch (PCONFIG(0)) {
        case P073_TM1637_4DGTCOLON:
        case P073_TM1637_4DGTDOTS:
        case P073_TM1637_6DGT: {
          tm1637_InitDisplay(CONFIG_PIN1, CONFIG_PIN2);
          int tm1637_bright = PCONFIG(2) / 2;
          tm1637_SetPowerBrightness(CONFIG_PIN1, CONFIG_PIN2, tm1637_bright, true);
          break;
        }
        case P073_MAX7219_8DGT: {
          max7219_InitDisplay(event, CONFIG_PIN1, CONFIG_PIN2, CONFIG_PIN3);
          max7219_SetPowerBrightness(event, CONFIG_PIN1, CONFIG_PIN2, CONFIG_PIN3,
                                     PCONFIG(2), true);
          break;
        }
      }
      success = true;
      break;
    }

    case PLUGIN_WRITE: {
      success = p073_plugin_write(event, string);
      break;
    }

    case PLUGIN_ONCE_A_SECOND: {
      P073_data_struct *P073_data =
        static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P073_data) {
        break;
      }

      if (P073_data->output == P073_DISP_MANUAL) {
        break;
      }

      if ((P073_data->output == P073_DISP_CLOCK24BLNK) ||
          (P073_data->output == P073_DISP_CLOCK12BLNK)) {
        P073_data->timesep = !P073_data->timesep;
      } else {
        P073_data->timesep = true;
      }

      if (P073_data->output == P073_DISP_DATE) {
        P073_data->FillBufferWithDate(true, 0, 0, 0);
      }
      else if ((P073_data->output == P073_DISP_CLOCK24BLNK) ||
               (P073_data->output == P073_DISP_CLOCK24)) {
        P073_data->FillBufferWithTime(true, 0, 0, 0, false);
      }
      else {
        P073_data->FillBufferWithTime(true, 0, 0, 0, true);
      }

      switch (P073_data->displayModel) {
        case P073_TM1637_4DGTCOLON:
        case P073_TM1637_4DGTDOTS: {
          tm1637_ShowTimeTemp4(event, P073_data->timesep, 0);
          break;
        }
        case P073_TM1637_6DGT: {
          if (PCONFIG(1) == P073_DISP_DATE) {
            tm1637_ShowDate6(event);
          }
          else {
            tm1637_ShowTime6(event);
          }
          break;
        }
        case P073_MAX7219_8DGT: {
          if (PCONFIG(1) == P073_DISP_DATE) {
            max7219_ShowDate(event, P073_data->pin1, P073_data->pin2,
                             P073_data->pin3);
          }
          else {
            max7219_ShowTime(event, P073_data->pin1, P073_data->pin2,
                             P073_data->pin3, P073_data->timesep);
          }
          break;
        }
      }
    }
  }
  return success;
}

bool p073_plugin_write(struct EventStruct *event, const String& string) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return false;
  }

  String cmd = parseString(string, 1);
  cmd.toLowerCase();
  String text = parseStringToEndKeepCase(string, 2);
  if (cmd.equals("7dn")) {
    return p073_plugin_write_7dn(event, text);
  } else if (cmd.equals("7dt")) {
    return p073_plugin_write_7dt(event, text);
  } else if (cmd.equals("7dst")) {
    return p073_plugin_write_7dst(event);
  } else if (cmd.equals("7dsd")) {
    return p073_plugin_write_7dsd(event);
  } else if (cmd.equals("7dtext")) {
    return p073_plugin_write_7dtext(event, text);
  } else {
    bool p073_validcmd = false;
    bool p073_displayon;

    if (cmd.equals("7don")) {
      addLog(LOG_LEVEL_INFO, F("7DGT : Display ON"));
      p073_displayon = true;
      p073_validcmd  = true;
    } else if (cmd.equals("7doff")) {
      addLog(LOG_LEVEL_INFO, F("7DGT : Display OFF"));
      p073_displayon = false;
      p073_validcmd  = true;
    } else if (cmd.equals("7db")) {
      if ((event->Par1 >= 0) && (event->Par1 < 16)) {
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("7DGT : Brightness=");
          log += event->Par1;
          addLog(LOG_LEVEL_INFO, log);
        }
        P073_data->brightness = event->Par1;
        p073_displayon        = true;
        p073_validcmd         = true;
      }
    }

    if (p073_validcmd) {
      switch (P073_data->displayModel) {
        case P073_TM1637_4DGTCOLON:
        case P073_TM1637_4DGTDOTS:
        case P073_TM1637_6DGT: {
          int tm1637_bright = P073_data->brightness / 2;
          tm1637_SetPowerBrightness(P073_data->pin1, P073_data->pin2,
                                    tm1637_bright, p073_displayon);
          break;
        }
        case P073_MAX7219_8DGT: {
          max7219_SetPowerBrightness(event, P073_data->pin1, P073_data->pin2,
                                     P073_data->pin3, P073_data->brightness,
                                     p073_displayon);
          break;
        }
      }
    }
    return p073_validcmd;
  }
  return false;
}

bool p073_plugin_write_7dn(struct EventStruct *event, const String& text) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return false;
  }

  if (P073_data->output != P073_DISP_MANUAL) {
    return false;
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("7DGT : Show Number=");
    log += event->Par1;
    addLog(LOG_LEVEL_INFO, log);
  }

  switch (P073_data->displayModel) {
    case P073_TM1637_4DGTCOLON: {
      if ((event->Par1 > -1000) && (event->Par1 < 10000)) {
        P073_data->FillBufferWithNumber(String(int(event->Par1)));
      }
      else {
        P073_data->FillBufferWithDash();
      }
      tm1637_ShowBuffer(event, TM1637_4DIGIT, 8);
      break;
    }
    case P073_TM1637_4DGTDOTS: {
      if ((event->Par1 > -1000) && (event->Par1 < 10000)) {
        P073_data->FillBufferWithNumber(text.c_str());
      }
      else {
        P073_data->FillBufferWithDash();
      }
      tm1637_ShowBuffer(event, TM1637_4DIGIT, 8);
      break;
    }
    case P073_TM1637_6DGT: {
      if ((event->Par1 > -100000) && (event->Par1 < 1000000)) {
        P073_data->FillBufferWithNumber(text.c_str());
      }
      else {
        P073_data->FillBufferWithDash();
      }
      tm1637_SwapDigitInBuffer(event, 2); // only needed for 6-digits displays
      tm1637_ShowBuffer(event, TM1637_6DIGIT, 8);
      break;
    }
    case P073_MAX7219_8DGT: {
      if (text.length() > 0) {
        if ((event->Par1 > -10000000) && (event->Par1 < 100000000)) {
          P073_data->FillBufferWithNumber(text.c_str());
        } else {
          P073_data->FillBufferWithDash();
        }
        max7219_ShowBuffer(event, P073_data->pin1, P073_data->pin2,
                           P073_data->pin3);
      }
      break;
    }
  }
  return true;
}

bool p073_plugin_write_7dt(struct EventStruct *event, const String& text) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return false;
  }

  if (P073_data->output != P073_DISP_MANUAL) {
    return false;
  }
  double p073_temptemp    = 0;
  bool   p073_tempflagdot = false;
  if (text.length() > 0) {
    validDoubleFromString(text, p073_temptemp);
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("7DGT : Show Temperature=");
    log += p073_temptemp;
    addLog(LOG_LEVEL_INFO, log);
  }

  switch (P073_data->displayModel) {
    case P073_TM1637_4DGTCOLON:
    case P073_TM1637_4DGTDOTS: {
      if ((p073_temptemp > 999) || (p073_temptemp < -99.9)) {
        P073_data->FillBufferWithDash();
      }
      else {
        if ((p073_temptemp < 100) && (p073_temptemp > -10)) {
          p073_temptemp    = int(p073_temptemp * 10);
          p073_tempflagdot = true;
        }
        P073_data->FillBufferWithTemp(p073_temptemp);

        if ((p073_temptemp == 0) && p073_tempflagdot) {
          P073_data->showbuffer[5] = 0;
        }
      }
      tm1637_ShowTimeTemp4(event, p073_tempflagdot, 4);
      break;
    }
    case P073_TM1637_6DGT: {
      if ((p073_temptemp > 999) || (p073_temptemp < -99.9)) {
        P073_data->FillBufferWithDash();
      }
      else {
        if ((p073_temptemp < 100) && (p073_temptemp > -10)) {
          p073_temptemp    = int(p073_temptemp * 10);
          p073_tempflagdot = true;
        }
        P073_data->FillBufferWithTemp(p073_temptemp);

        if ((p073_temptemp == 0) && p073_tempflagdot) {
          P073_data->showbuffer[5] = 0;
        }
      }
      tm1637_ShowTemp6(event, p073_tempflagdot);
      break;
    }
    case P073_MAX7219_8DGT: {
      p073_temptemp = int(p073_temptemp * 10);
      P073_data->FillBufferWithTemp(p073_temptemp);

      if (p073_temptemp == 0) {
        P073_data->showbuffer[5] = 0;
      }
      max7219_ShowTemp(event, P073_data->pin1, P073_data->pin2, P073_data->pin3);
      break;
    }
  }
  return true;
}

bool p073_plugin_write_7dst(struct EventStruct *event) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return false;
  }

  if (P073_data->output != P073_DISP_MANUAL) {
    return false;
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("7DGT : Show Time=");
    log += event->Par1;
    log += ":";
    log += event->Par2;
    log += ":";
    log += event->Par3;
    addLog(LOG_LEVEL_INFO, log);
  }
  P073_data->timesep = true;
  P073_data->FillBufferWithTime(false, event->Par1, event->Par2, event->Par3, false);

  switch (P073_data->displayModel) {
    case P073_TM1637_4DGTCOLON:
    case P073_TM1637_4DGTDOTS: {
      tm1637_ShowTimeTemp4(event, P073_data->timesep, 0);
      break;
    }
    case P073_TM1637_6DGT: {
      tm1637_ShowTime6(event);
      break;
    }
    case P073_MAX7219_8DGT: {
      max7219_ShowTime(event, P073_data->pin1, P073_data->pin2, P073_data->pin3,
                       P073_data->timesep);
      break;
    }
  }
  return true;
}

bool p073_plugin_write_7dsd(struct EventStruct *event) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return false;
  }

  if (P073_data->output != P073_DISP_MANUAL) {
    return false;
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("7DGT : Show Date=");
    log += event->Par1;
    log += "-";
    log += event->Par2;
    log += "-";
    log += event->Par3;
    addLog(LOG_LEVEL_INFO, log);
  }
  P073_data->FillBufferWithDate(false, event->Par1, event->Par2, event->Par3);

  switch (P073_data->displayModel) {
    case P073_TM1637_4DGTCOLON:
    case P073_TM1637_4DGTDOTS: {
      tm1637_ShowTimeTemp4(event, P073_data->timesep, 0);
      break;
    }
    case P073_TM1637_6DGT: {
      tm1637_ShowDate6(event);
      break;
    }
    case P073_MAX7219_8DGT: {
      max7219_ShowDate(event, P073_data->pin1, P073_data->pin2, P073_data->pin3);
      break;
    }
  }
  return true;
}

bool p073_plugin_write_7dtext(struct EventStruct *event, const String& text) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return false;
  }

  if (P073_data->output != P073_DISP_MANUAL) {
    return false;
  }
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("7DGT : Show Text=");
    log += text;
    addLog(LOG_LEVEL_INFO, log);
  }
  P073_data->FillBufferWithString(text);

  switch (P073_data->displayModel) {
    case P073_TM1637_4DGTCOLON:
    case P073_TM1637_4DGTDOTS: {
      tm1637_ShowBuffer(event, 0, 4);
      break;
    }
    case P073_TM1637_6DGT: {
      tm1637_SwapDigitInBuffer(event, 0); // only needed for 6-digits displays
      tm1637_ShowBuffer(event, 0, 6);
      break;
    }
    case P073_MAX7219_8DGT: {
      P073_data->dotpos = -1; // avoid to display the dot
      max7219_ShowBuffer(event, P073_data->pin1, P073_data->pin2,
                         P073_data->pin3);
      break;
    }
  }
  return true;
}

// ===================================
// ---- TM1637 specific functions ----
// ===================================

#define CLK_HIGH() digitalWrite(clk_pin, HIGH)
#define CLK_LOW() digitalWrite(clk_pin, LOW)
#define DIO_HIGH() pinMode(dio_pin, INPUT)
#define DIO_LOW() pinMode(dio_pin, OUTPUT)

void tm1637_i2cStart(uint8_t clk_pin, uint8_t dio_pin) {
  if (PLUGIN_073_DEBUG) {
    String log = F("7DGT : Comm Start");
    addLog(LOG_LEVEL_INFO, log);
  }
  DIO_HIGH();
  CLK_HIGH();
  delayMicroseconds(TM1637_CLOCKDELAY);
  DIO_LOW();
}

void tm1637_i2cStop(uint8_t clk_pin, uint8_t dio_pin) {
  if (PLUGIN_073_DEBUG) {
    String log = F("7DGT : Comm Stop");
    addLog(LOG_LEVEL_INFO, log);
  }
  CLK_LOW();
  delayMicroseconds(TM1637_CLOCKDELAY);
  DIO_LOW();
  delayMicroseconds(TM1637_CLOCKDELAY);
  CLK_HIGH();
  delayMicroseconds(TM1637_CLOCKDELAY);
  DIO_HIGH();
}

void tm1637_i2cAck(uint8_t clk_pin, uint8_t dio_pin) {
  bool dummyAck = false;

  CLK_LOW();
  pinMode(dio_pin, INPUT_PULLUP);

  // DIO_HIGH();
  delayMicroseconds(TM1637_CLOCKDELAY);

  // while(digitalRead(dio_pin));
  dummyAck = digitalRead(dio_pin);

  if (PLUGIN_073_DEBUG) {
    String log = F("7DGT : Comm ACK=");

    if (dummyAck == 0) {
      log += "TRUE";
    } else {
      log += "FALSE";
    }
    addLog(LOG_LEVEL_INFO, log);
  }
  CLK_HIGH();
  delayMicroseconds(TM1637_CLOCKDELAY);
  CLK_LOW();
  pinMode(dio_pin, OUTPUT);
}

void tm1637_i2cWrite_ack(uint8_t clk_pin, uint8_t dio_pin,
                         uint8_t bytesToPrint[], byte length) {
  tm1637_i2cStart(clk_pin, dio_pin);

  for (byte i = 0; i < length; ++i) {
    tm1637_i2cWrite_ack(clk_pin, dio_pin, bytesToPrint[i]);
  }
  tm1637_i2cStop(clk_pin, dio_pin);
}

void tm1637_i2cWrite_ack(uint8_t clk_pin, uint8_t dio_pin,
                         uint8_t bytetoprint) {
  tm1637_i2cWrite(clk_pin, dio_pin, bytetoprint);
  tm1637_i2cAck(clk_pin, dio_pin);
}

void tm1637_i2cWrite(uint8_t clk_pin, uint8_t dio_pin, uint8_t bytetoprint) {
  if (PLUGIN_073_DEBUG) {
    String log = F("7DGT : WriteByte");
    addLog(LOG_LEVEL_INFO, log);
  }
  uint8_t i;

  for (i = 0; i < 8; i++) {
    CLK_LOW();

    if (bytetoprint & B00000001) {
      DIO_HIGH();
    } else {
      DIO_LOW();
    }
    delayMicroseconds(TM1637_CLOCKDELAY);
    bytetoprint = bytetoprint >> 1;
    CLK_HIGH();
    delayMicroseconds(TM1637_CLOCKDELAY);
  }
}

void tm1637_ClearDisplay(uint8_t clk_pin, uint8_t dio_pin) {
  uint8_t bytesToPrint[7] = { 0 };

  bytesToPrint[0] = 0xC0;
  tm1637_i2cWrite_ack(clk_pin, dio_pin, bytesToPrint, 7);
}

void tm1637_SetPowerBrightness(uint8_t clk_pin, uint8_t dio_pin,
                               uint8_t brightlvl, bool poweron) {
  if (PLUGIN_073_DEBUG) {
    String log = F("7DGT : Set BRIGHT");
    addLog(LOG_LEVEL_INFO, log);
  }
  uint8_t brightvalue = (brightlvl & 0b111);

  if (poweron) {
    brightvalue = TM1637_POWER_ON | brightvalue;
  } else {
    brightvalue = TM1637_POWER_OFF | brightvalue;
  }

  uint8_t bytesToPrint[1] = { 0 };
  bytesToPrint[0] = brightvalue;
  tm1637_i2cWrite_ack(clk_pin, dio_pin, bytesToPrint, 1);
}

void tm1637_InitDisplay(uint8_t clk_pin, uint8_t dio_pin) {
  pinMode(clk_pin, OUTPUT);
  pinMode(dio_pin, OUTPUT);
  CLK_HIGH();
  DIO_HIGH();

  //  pinMode(dio_pin, INPUT_PULLUP);
  //  pinMode(clk_pin, OUTPUT);
  uint8_t bytesToPrint[1] = { 0 };
  bytesToPrint[0] = 0x40;
  tm1637_i2cWrite_ack(clk_pin, dio_pin, bytesToPrint, 1);
  tm1637_ClearDisplay(clk_pin, dio_pin);
}

uint8_t tm1637_separator(uint8_t value, bool sep) {
  if (sep) {
    value |= 0b10000000;
  }
  return value;
}

void tm1637_ShowTime6(struct EventStruct *event) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }

  uint8_t clk_pin         = P073_data->pin1;
  uint8_t dio_pin         = P073_data->pin2;
  bool    sep             = P073_data->timesep;
  uint8_t bytesToPrint[7] = { 0 };
  bytesToPrint[0] = 0xC0;
  bytesToPrint[1] = CharTableTM1637[P073_data->showbuffer[2]];
  bytesToPrint[2] =
    tm1637_separator(CharTableTM1637[P073_data->showbuffer[1]], sep);
  bytesToPrint[3] = CharTableTM1637[P073_data->showbuffer[0]];
  bytesToPrint[4] = CharTableTM1637[P073_data->showbuffer[5]];
  bytesToPrint[5] = CharTableTM1637[P073_data->showbuffer[4]];
  bytesToPrint[6] =
    tm1637_separator(CharTableTM1637[P073_data->showbuffer[3]], sep);
  tm1637_i2cWrite_ack(clk_pin, dio_pin, bytesToPrint, 7);
}

void tm1637_ShowDate6(struct EventStruct *event) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }
  uint8_t clk_pin = P073_data->pin1;
  uint8_t dio_pin = P073_data->pin2;
  bool    sep     = P073_data->timesep;

  uint8_t bytesToPrint[7] = { 0 };
  bytesToPrint[0] = 0xC0;
  bytesToPrint[1] = CharTableTM1637[P073_data->showbuffer[2]];
  bytesToPrint[2] =
    tm1637_separator(CharTableTM1637[P073_data->showbuffer[1]], sep);
  bytesToPrint[3] = CharTableTM1637[P073_data->showbuffer[0]];
  bytesToPrint[4] = CharTableTM1637[P073_data->showbuffer[7]];
  bytesToPrint[5] = CharTableTM1637[P073_data->showbuffer[6]];
  bytesToPrint[6] =
    tm1637_separator(CharTableTM1637[P073_data->showbuffer[3]], sep);
  tm1637_i2cWrite_ack(clk_pin, dio_pin, bytesToPrint, 7);
}

void tm1637_ShowTemp6(struct EventStruct *event, bool sep) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }
  uint8_t clk_pin = P073_data->pin1;
  uint8_t dio_pin = P073_data->pin2;

  uint8_t bytesToPrint[7] = { 0 };
  bytesToPrint[0] = 0xC0;
  bytesToPrint[1] =
    tm1637_separator(CharTableTM1637[P073_data->showbuffer[5]], sep);
  bytesToPrint[2] = CharTableTM1637[P073_data->showbuffer[4]];
  bytesToPrint[3] = CharTableTM1637[10];
  bytesToPrint[4] = CharTableTM1637[10];
  bytesToPrint[5] = CharTableTM1637[P073_data->showbuffer[7]];
  bytesToPrint[6] = CharTableTM1637[P073_data->showbuffer[6]];
  tm1637_i2cWrite_ack(clk_pin, dio_pin, bytesToPrint, 7);
}

void tm1637_ShowTimeTemp4(struct EventStruct *event, bool sep, byte bufoffset) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }
  uint8_t clk_pin = P073_data->pin1;
  uint8_t dio_pin = P073_data->pin2;

  uint8_t bytesToPrint[7] = { 0 };
  bytesToPrint[0] = 0xC0;
  bytesToPrint[1] = CharTableTM1637[P073_data->showbuffer[0 + bufoffset]];
  bytesToPrint[2] = tm1637_separator(
    CharTableTM1637[P073_data->showbuffer[1 + bufoffset]], sep);
  bytesToPrint[3] = CharTableTM1637[P073_data->showbuffer[2 + bufoffset]];
  bytesToPrint[4] = CharTableTM1637[P073_data->showbuffer[3 + bufoffset]];
  tm1637_i2cWrite_ack(clk_pin, dio_pin, bytesToPrint, 5);
}

void tm1637_SwapDigitInBuffer(struct EventStruct *event, byte startPos) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }
  uint8_t p073_temp;
  p073_temp                           = P073_data->showbuffer[2 + startPos];
  P073_data->showbuffer[2 + startPos] = P073_data->showbuffer[0 + startPos];
  P073_data->showbuffer[0 + startPos] = p073_temp;
  p073_temp                           = P073_data->showbuffer[3 + startPos];
  P073_data->showbuffer[3 + startPos] = P073_data->showbuffer[5 + startPos];
  P073_data->showbuffer[5 + startPos] = p073_temp;

  switch (P073_data->dotpos) {
    case 2: {
      P073_data->dotpos = 4;
      break;
    }
    case 4: {
      P073_data->dotpos = 2;
      break;
    }
    case 5: {
      P073_data->dotpos = 7;
      break;
    }
    case 7: {
      P073_data->dotpos = 5;
      break;
    }
  }
}

void tm1637_ShowBuffer(struct EventStruct *event, byte firstPos, byte lastPos) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }
  uint8_t clk_pin = P073_data->pin1;
  uint8_t dio_pin = P073_data->pin2;

  uint8_t bytesToPrint[8] = { 0 };
  bytesToPrint[0] = 0xC0;
  byte length = 1;

  for (int i = firstPos; i < lastPos; i++) {
    byte p073_datashowpos1 = tm1637_separator(
      CharTableTM1637[P073_data->showbuffer[i]], P073_data->dotpos == i);
    bytesToPrint[length] = p073_datashowpos1;
    ++length;
  }
  tm1637_i2cWrite_ack(clk_pin, dio_pin, bytesToPrint, length);
}

// ====================================
// ---- MAX7219 specific functions ----
// ====================================

#define OP_DECODEMODE   9
#define OP_INTENSITY   10
#define OP_SCANLIMIT   11
#define OP_SHUTDOWN    12
#define OP_DISPLAYTEST 15

void max7219_spiTransfer(struct EventStruct *event, uint8_t din_pin,
                         uint8_t clk_pin, uint8_t cs_pin, volatile byte opcode,
                         volatile byte data) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }

  P073_data->spidata[1] = opcode;
  P073_data->spidata[0] = data;
  digitalWrite(cs_pin, LOW);
  shiftOut(din_pin, clk_pin, MSBFIRST, P073_data->spidata[1]);
  shiftOut(din_pin, clk_pin, MSBFIRST, P073_data->spidata[0]);
  digitalWrite(cs_pin, HIGH);
}

void max7219_ClearDisplay(struct EventStruct *event, uint8_t din_pin,
                          uint8_t clk_pin, uint8_t cs_pin) {
  for (int i = 0; i < 8; i++) {
    max7219_spiTransfer(event, din_pin, clk_pin, cs_pin, i + 1, 0);
  }
}

void max7219_SetPowerBrightness(struct EventStruct *event, uint8_t din_pin,
                                uint8_t clk_pin, uint8_t cs_pin,
                                uint8_t brightlvl, bool poweron) {
  max7219_spiTransfer(event, din_pin, clk_pin, cs_pin, OP_INTENSITY, brightlvl);
  max7219_spiTransfer(event, din_pin, clk_pin, cs_pin, OP_SHUTDOWN,
                      poweron ? 1 : 0);
}

void max7219_SetDigit(struct EventStruct *event, uint8_t din_pin,
                      uint8_t clk_pin, uint8_t cs_pin, int dgtpos,
                      byte dgtvalue, boolean showdot) {
  byte p073_tempvalue;

  p073_tempvalue = CharTableMAX7219[dgtvalue];

  if (showdot) {
    p073_tempvalue |= 0b10000000;
  }
  max7219_spiTransfer(event, din_pin, clk_pin, cs_pin, dgtpos + 1,
                      p073_tempvalue);
}

void max7219_InitDisplay(struct EventStruct *event, uint8_t din_pin,
                         uint8_t clk_pin, uint8_t cs_pin) {
  pinMode(din_pin, OUTPUT);
  pinMode(clk_pin, OUTPUT);
  pinMode(cs_pin,  OUTPUT);
  digitalWrite(cs_pin, HIGH);
  max7219_spiTransfer(event, din_pin, clk_pin, cs_pin, OP_DISPLAYTEST, 0);
  max7219_spiTransfer(event, din_pin, clk_pin, cs_pin, OP_SCANLIMIT,   7); // scanlimit setup to max at Init
  max7219_spiTransfer(event, din_pin, clk_pin, cs_pin, OP_DECODEMODE,  0);
  max7219_ClearDisplay(event, din_pin, clk_pin, cs_pin);
  max7219_SetPowerBrightness(event, din_pin, clk_pin, cs_pin, 0, false);
}

void max7219_ShowTime(struct EventStruct *event, uint8_t din_pin,
                      uint8_t clk_pin, uint8_t cs_pin, bool sep) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }

  max7219_SetDigit(event, din_pin, clk_pin, cs_pin, 0, P073_data->showbuffer[5], false);
  max7219_SetDigit(event, din_pin, clk_pin, cs_pin, 1, P073_data->showbuffer[4], false);
  max7219_SetDigit(event, din_pin, clk_pin, cs_pin, 3, P073_data->showbuffer[3], false);
  max7219_SetDigit(event, din_pin, clk_pin, cs_pin, 4, P073_data->showbuffer[2], false);
  max7219_SetDigit(event, din_pin, clk_pin, cs_pin, 6, P073_data->showbuffer[1], false);
  max7219_SetDigit(event, din_pin, clk_pin, cs_pin, 7, P073_data->showbuffer[0], false);
  uint8_t sepChar = P073_mapCharToFontPosition(sep ? '-' : ' ');
  max7219_SetDigit(event, din_pin, clk_pin, cs_pin, 2,   sepChar,                false);
  max7219_SetDigit(event, din_pin, clk_pin, cs_pin, 5,   sepChar,                false);
}

void max7219_ShowTemp(struct EventStruct *event, uint8_t din_pin,
                      uint8_t clk_pin, uint8_t cs_pin) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }

  max7219_SetDigit(event, din_pin, clk_pin, cs_pin, 0, 10, false);
  byte dotflags[8] = { false, false, false, false, false, true, false, false };

  for (int i = 1; i < 8; i++) {
    max7219_SetDigit(event, din_pin, clk_pin, cs_pin, i,
                     P073_data->showbuffer[8 - i], dotflags[8 - i]);
  }
}

void max7219_ShowDate(struct EventStruct *event, uint8_t din_pin,
                      uint8_t clk_pin, uint8_t cs_pin) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }

  byte dotflags[8] = { false, true, false, true, false, false, false, false };

  for (int i = 0; i < 8; i++) {
    max7219_SetDigit(event, din_pin, clk_pin, cs_pin, i,
                     P073_data->showbuffer[7 - i], dotflags[7 - i]);
  }
}

void max7219_ShowBuffer(struct EventStruct *event, uint8_t din_pin,
                        uint8_t clk_pin, uint8_t cs_pin) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }

  byte dotflags[8] = { false, false, false, false, false, false, false, false };

  if (P073_data->dotpos >= 0) {
    dotflags[P073_data->dotpos] = true;
  }

  for (int i = 0; i < 8; i++) {
    max7219_SetDigit(event, din_pin, clk_pin, cs_pin, i,
                     P073_data->showbuffer[7 - i], dotflags[7 - i]);
  }
}

#endif // USES_P073
