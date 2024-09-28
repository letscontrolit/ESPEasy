#include "../PluginStructs/P073_data_struct.h"

#ifdef USES_P073

# include <GPIO_Direct_Access.h>

uint8_t P073_getDefaultDigits(uint8_t displayModel,
                              uint8_t digits) {
  const uint8_t digitsSet[] = { 4, 4, 6, 8, 0 }; // Fixed except 74HC595
  uint8_t bufLen{};

  if (displayModel < NR_ELEMENTS(digitsSet)) {
    bufLen = digitsSet[displayModel];
  }
  # if P073_USE_74HC595

  if (P073_74HC595_2_8DGT == displayModel) {
    bufLen = digits;
  }
  # endif // if P073_USE_74HC595

  return bufLen;
}

/**
 * Maps an ASCII character to a generic 7-segment usable smaller characterset,
 * for easy mapping to different font-sets, see P073_getFontChar()
 */
uint8_t P073_mapCharToFontPosition(char    character,
                                   uint8_t fontset) {
  uint8_t position = 10;

  # if P073_EXTRA_FONTS
  const String specialChars = F(" -^=/_%@.,;:+*#!?'\"<>\\()|");
  const String chnorux      = F("CHNORUX");

  switch (fontset) {
    case 1: // Siekoo
    case 2: // Siekoo with uppercase 'CHNORUX'

      if ((fontset == 2) && (chnorux.indexOf(character) > -1)) {
        position = chnorux.indexOf(character) + 35;
      } else if (isDigit(character)) {
        position = character - '0';
      } else if (isAlpha(character)) {
        position = character - (isLowerCase(character) ? 'a' : 'A') + 42;
      } else {
        const int idx = specialChars.indexOf(character);

        if (idx > -1) {
          position = idx + 10;
        }
      }
      break;
    case 3:  // dSEG7 (same table size as 7Dgt)
    default: // Original fontset (7Dgt)
  # endif // if P073_EXTRA_FONTS

  if (isDigit(character)) {
    position = character - '0';
  } else if (isAlpha(character)) {
    position = character - (isLowerCase(character) ? 'a' : 'A') + 16;
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
  # if P073_EXTRA_FONTS
}

  # endif // if P073_EXTRA_FONTS
  return position;
}

/**
 * Get the 7-segment representation for an index into a specific 7-segment font
 * Fonts available depend on the build, but the 'DefaultCharTable' is always available
 */
uint8_t P073_getFontChar(uint8_t index,
                         uint8_t fontset) {
  # if P073_EXTRA_FONTS

  switch (fontset) {
    case 1:  // Siekoo
    case 2:  // Siekoo uppercase CHNORUX
      return pgm_read_byte(&(SiekooCharTable[index]));
    case 3:  // dSEG7
      return pgm_read_byte(&(Dseg7CharTable[index]));
    default: // Standard fontset
      return pgm_read_byte(&(DefaultCharTable[index]));
  }
  # else // if P073_EXTRA_FONTS
  return pgm_read_byte(&(DefaultCharTable[index]));
  # endif // if P073_EXTRA_FONTS
}

int32_t P073_parse_7dfont(struct EventStruct *event,
                          const String      & text) {
  int32_t fontNr = 0;

  # if P073_EXTRA_FONTS

  if (!text.isEmpty()) {
    const String fontArg = parseString(text, 1);
    fontNr = -1;

    if ((equals(fontArg, F("default"))) || (equals(fontArg, F("7dgt")))) {
      fontNr = 0;
    } else if (equals(fontArg, F("siekoo"))) {
      fontNr = 1;
    } else if (equals(fontArg, F("siekoo_upper"))) {
      fontNr = 2;
    } else if (equals(fontArg, F("dseg7"))) {
      fontNr = 3;
    } else if (!validIntFromString(text, fontNr)) {
      fontNr = -1; // reset if invalid
    }
  }
  # endif // if P073_EXTRA_FONTS
  return fontNr;
}

void P073_display_output_selector(const __FlashStringHelper *id, int16_t value) {
  const __FlashStringHelper *displout[] = {
    F("Manual"),
    F("Clock 24h - Blink"),
    F("Clock 24h - No Blink"),
    F("Clock 12h - Blink"),
    F("Clock 12h - No Blink"),
    F("Date"),
  };
  const int disploutOptions[] = {
    P073_DISP_MANUAL,
    P073_DISP_CLOCK24BLNK,
    P073_DISP_CLOCK24,
    P073_DISP_CLOCK12BLNK,
    P073_DISP_CLOCK12,
    P073_DISP_DATE,
  };

  addFormSelector(F("Display Output"), id, NR_ELEMENTS(disploutOptions), displout, disploutOptions, value);
}

# if P073_EXTRA_FONTS
void P073_font_selector(const __FlashStringHelper *id, int16_t value) {
  const __FlashStringHelper *fontset[] = {
    F("Default"),
    F("Siekoo"),
    F("Siekoo with uppercase 'CHNORUX'"),
    F("dSEG7"),
  };

  addFormSelector(F("Font set"), id, NR_ELEMENTS(fontset), fontset, nullptr, value);
  addFormNote(F("Check documentation for examples of the font sets."));
}

# endif // if P073_EXTRA_FONTS

/**
 * This function reverts the 7 databits/segmentbits so TM1637 and 74HC595 displays work with fonts designed for MAX7219.
 * Dot/colon bit is still bit 8
 */
uint8_t P073_revert7bits(uint8_t character) {
  uint8_t dpBit = character & 0x80; // Keep dot-bit if passed in
  uint8_t b     = character << 1;   // Pre-shift as only 7 bits to revert

  // Source: https://graphics.stanford.edu/~seander/bithacks.html#BitReverseObvious
  # ifdef ESP8266

  // 32 bit 7 operations variant, 64 bit operations is quite slow on ESP8266, but fast on most ESP32 variants
  b = ((b * 0x0802LU & 0x22110LU) | (b * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16;
  # endif // ifdef ESP8266
  # ifdef ESP32

  // 64 bit 4 operations variant
  b = ((b * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >> 32;
  # endif // ifdef ESP32

  return b | dpBit; // Restore dot-bit
}

void P073_data_struct::init(struct EventStruct *event)
{
  ClearBuffer();
  pin1         = CONFIG_PIN1;
  pin2         = CONFIG_PIN2;
  pin3         = CONFIG_PIN3;
  displayModel = P073_CFG_DISPLAYTYPE;
  output       = P073_CFG_OUTPUTTYPE;
  brightness   = P073_CFG_BRIGHTNESS;
  periods      = bitRead(P073_CFG_FLAGS, P073_OPTION_PERIOD);
  hideDegree   = bitRead(P073_CFG_FLAGS, P073_OPTION_HIDEDEGREE);
  # if P073_SCROLL_TEXT
  txtScrolling = bitRead(P073_CFG_FLAGS, P073_OPTION_SCROLLTEXT);
  scrollFull   = bitRead(P073_CFG_FLAGS, P073_OPTION_SCROLLFULL);
  setScrollSpeed(P073_CFG_SCROLLSPEED);
  # endif // if P073_SCROLL_TEXT
  rightAlignTempMAX7219 = bitRead(P073_CFG_FLAGS, P073_OPTION_RIGHTALIGN);
  suppressLeading0      = bitRead(P073_CFG_FLAGS, P073_OPTION_SUPPRESS0);
  #if P073_BLINK_DOT
  blinkdot = bitRead(P073_CFG_FLAGS, P073_OPTION_BLINK_DOT);
  #endif // if P073_BLINK_DOT
  timesep = true;
  # if P073_EXTRA_FONTS
  fontset = P073_CFG_FONTSET;
  # endif // if P073_EXTRA_FONTS
  digits = P073_CFG_DIGITS;
  # if P073_USE_74HC595

  if ((digits > 0) && ((digits < 4) || (5 == digits) || (7 == digits) || (9 == digits) || (10 == digits) || (11 == digits))) {
    isSequential = true;

    if (1 == digits) {  // 2+2
      digits = 4;
    } else
    if (9 == digits) {  // 4 sequential
      digits = 4;
    } else
    if (10 == digits) { // 4+4 sequential
      digits = 8;
    } else
    if (7 == digits) {  // 3+3
      digits = 6;
    } else
    if (11 == digits) { // 3+4/4+3 sequential
      digits = 7;
    }
  }
  # endif // if P073_USE_74HC595

  if ((digits > 0) && (digits < 4)) {
    hideDegree = true; // Hide degree symbol on small displays
  }

  switch (displayModel) {
    case P073_TM1637_4DGTCOLON:
    case P073_TM1637_4DGTDOTS:
    case P073_TM1637_6DGT:
      tm1637_InitDisplay();
      tm1637_SetPowerBrightness(brightness / 2, true);

      if (output == P073_DISP_MANUAL) {
        tm1637_ClearDisplay();
      }
      break;
    case P073_MAX7219_8DGT:
      max7219_InitDisplay();
      delay(10); // small poweroff/poweron delay
      max7219_SetPowerBrightness(brightness, true);

      if (output == P073_DISP_MANUAL) {
        max7219_ClearDisplay();
      }
      break;
    # if P073_USE_74HC595
    case P073_74HC595_2_8DGT:
      hc595_InitDisplay();

      if (output == P073_DISP_MANUAL) {
        ClearBuffer();
        hc595_ToOutputBuffer();

        if (hc595_Sequential()) { // Sequential displays don't need continuous refreshing
          hc595_ShowBuffer();
        }
      }
      break;
    # endif // if P073_USE_74HC595
  }
}

# if P073_USE_74HC595
bool P073_data_struct::plugin_fifty_per_second(struct EventStruct *event) {
  #  ifdef P073_DEBUG
  counter50++;
  #  endif // ifdef P073_DEBUG

  if (P073_74HC595_2_8DGT == displayModel) {
    if (P073_HC595_MULTIPLEX) {
      hc595_ShowBuffer();
    }
    return true;
  }
  return false;
}

bool P073_data_struct::is74HC595Matrix() {
  return P073_74HC595_2_8DGT == displayModel && P073_HC595_MULTIPLEX;
}

// ====================================
// ---- 74HC595 specific functions ----
// ====================================

void P073_data_struct::hc595_ShowBuffer() {
  #  if P073_USE_74HCMULTIPLEX
  const uint8_t hc595digit4[] = {
    0b00001000, // left segment
    0b00000100,
    0b00000010,
    0b00000001, // right segment
  };

  const uint8_t hc595digit8[] = {
    0b00010000, // left segment
    0b00100000,
    0b01000000,
    0b10000000,
    0b00000001,
    0b00000010,
    0b00000100,
    0b00001000, // right segment
  };
  #  endif // if P073_USE_74HCMULTIPLEX

  int8_t i    = digits - 1;
  int8_t stop = -1;
  int8_t incr = -1;

  #  if P073_USE_74HCMULTIPLEX

  if (P073_HC595_MULTIPLEX) {
    i    =  dspDgt;
    stop =  dspDgt + 1;
    incr =  1;
  }
  #  endif // if P073_USE_74HCMULTIPLEX

  for (; i != stop && i >= 0; i += incr) {
    shiftOut(pin1, pin2, MSBFIRST, outputbuffer[i]); // Digit data out

    // 2, 3 and some 4 digit modules use sequential digit values (in reversed order)
    // 4, 6 and 8 digit modules use multiplexing in LTR order
    #  if P073_USE_74HCMULTIPLEX
    uint8_t digit = 0xFF;

    if (P073_HC595_MULTIPLEX) {
      if (4 == digits) {
        digit = hc595digit4[i];
      } else
      if (6 == digits) {
        digit = hc595digit8[i + (i > 2 ? 1 : 0)];
      } else
      if (8 == digits) {
        digit = hc595digit8[i];
      }
    }

    if (digit != 0xFF) { // Select multiplexer digit, 0xFF is invalid
      shiftOut(pin1, pin2, MSBFIRST, digit);
    }
    #  endif // if P073_USE_74HCMULTIPLEX

    if ((P073_HC595_SEQUENTIAL && (0 == i)) || P073_HC595_MULTIPLEX) {
      digitalWrite(pin3, LOW); // Clock data
      digitalWrite(pin3, HIGH);
    }
  }

  if (i >= digits) {
    dspDgt = 0;
  } else {
    dspDgt = i;
  }

  #  ifdef P073_DEBUG

  // TODO disable log
  // if ((counter50 % 200 == 0) || P073_HC595_SEQUENTIAL) {
  //   addLog(LOG_LEVEL_INFO, strformat(F("P073: hc595_ShowBuffer (end) dgt:%d i:%d stop:%d incr:%d pin1: %d pin2: %d pin3: %d"),
  //                                    digits, i, stop, incr, pin1, pin2, pin3));
  // }
  #  endif // ifdef P073_DEBUG
}

void P073_data_struct::hc595_ToOutputBuffer() {
  for (uint8_t i = 0; i < 8; ++i) {
    uint8_t value;

    // 74HC595 uses inverted data, compared to MAX7219/TM1637
    value = ~P073_getFontChar(showbuffer[i], fontset);

    if (showperiods[i]) {
      value &= 0x7F;
    }
    outputbuffer[i] = P073_revert7bits(value); // Rotate bits 6..0
  }
}

void P073_data_struct::hc595_AdjustBuffer() {
  if (digits < 8) {
    const uint8_t delta = 8 - digits;

    for (uint8_t i = 0; i < digits; ++i) {
      showbuffer[i] = showbuffer[i + delta];
    }
  }
}

void P073_data_struct::hc595_InitDisplay() {
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);
  digitalWrite(pin3, HIGH);
}

# endif // if P073_USE_74HC595

void P073_data_struct::FillBufferWithTime(bool    sevendgt_now,
                                          uint8_t sevendgt_hours,
                                          uint8_t sevendgt_minutes,
                                          uint8_t sevendgt_seconds,
                                          bool    flag12h,
                                          bool    suppressLeading0) {
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
  Put4NumbersInBuffer(sevendgt_hours, sevendgt_minutes, sevendgt_seconds, -1
                      # if P073_SUPPRESS_ZERO
                      , suppressLeading0
                      # endif // if P073_SUPPRESS_ZERO
                      );
}

void P073_data_struct::FillBufferWithDate(bool    sevendgt_now,
                                          uint8_t sevendgt_day,
                                          uint8_t sevendgt_month,
                                          int     sevendgt_year,
                                          bool    suppressLeading0) {
  ClearBuffer();
  int sevendgt_year0 = sevendgt_year;

  if (sevendgt_now) {
    sevendgt_day   = node_time.day();
    sevendgt_month = node_time.month();
    sevendgt_year0 = node_time.year();
  } else if (sevendgt_year0 < 100) {
    sevendgt_year0 += 2000;
  }
  const uint8_t sevendgt_year1 = static_cast<uint8_t>(sevendgt_year0 / 100);
  const uint8_t sevendgt_year2 = static_cast<uint8_t>(sevendgt_year0 % 100);

  Put4NumbersInBuffer(sevendgt_day, sevendgt_month, sevendgt_year1, sevendgt_year2
                      # if P073_SUPPRESS_ZERO
                      , suppressLeading0
                      # endif // if P073_SUPPRESS_ZERO
                      );
}

void P073_data_struct::Put4NumbersInBuffer(const uint8_t nr1,
                                           const uint8_t nr2,
                                           const uint8_t nr3,
                                           const int8_t  nr4
                                           # if          P073_SUPPRESS_ZERO
                                           , const bool  suppressLeading0
                                           # endif // if P073_SUPPRESS_ZERO
                                           ) {
  showbuffer[0] = static_cast<uint8_t>(nr1 / 10);
  showbuffer[1] = nr1 % 10;
  showbuffer[2] = static_cast<uint8_t>(nr2 / 10);
  showbuffer[3] = nr2 % 10;
  showbuffer[4] = static_cast<uint8_t>(nr3 / 10);
  showbuffer[5] = nr3 % 10;

  if (nr4 > -1) {
    showbuffer[6] = static_cast<uint8_t>(nr4 / 10);
    showbuffer[7] = nr4 % 10;
  }
  # if P073_SUPPRESS_ZERO

  if (suppressLeading0 && (showbuffer[0] == 0)) { showbuffer[0] = 10; } // set to space
  # endif // if P073_SUPPRESS_ZERO
}

void P073_data_struct::FillBufferWithNumber(const String& number) {
  ClearBuffer();

  if (number.length() == 0) {
    return;
  }
  int8_t p073_index = 7;

  dotpos = -1; // -1 means no dot to display

  for (int i = number.length() - 1; i >= 0 && p073_index >= 0; --i) {
    const char p073_tmpchar = number.charAt(i);

    if (p073_tmpchar == '.') { // dot
      dotpos = p073_index;
    } else {
      showbuffer[p073_index] = P073_mapCharToFontPosition(p073_tmpchar, fontset);
      p073_index--;
    }
  }
}

void P073_data_struct::FillBufferWithTemp(int temperature) {
  ClearBuffer();
  char p073_digit[8];
  const bool between10and0      = ((temperature < 10) && (temperature >= 0)); // To have a zero prefix (0.x and -0.x) display between 0.9
  const bool between0andMinus10 = ((temperature < 0) && (temperature > -10)); // and -0.9 degrees,as all display types use 1 digit for
                                                                              // temperatures between 10.0 and -10.0
  String format;

  if (hideDegree) {
    format = (between10and0 ? F("      %02d") : (between0andMinus10 ? F("     %03d") : F("%8d")));
  } else {
    format = (between10and0 ? F("     %02d") : (between0andMinus10 ? F("    %03d") : F("%7d")));
  }
  sprintf_P(p073_digit, format.c_str(), temperature);
  const size_t p073_numlenght = strlen(p073_digit);

  for (size_t i = 0; i < p073_numlenght; ++i) {
    showbuffer[i] = P073_mapCharToFontPosition(p073_digit[i], fontset);
  }

  if (!hideDegree) {
    showbuffer[7] = 12; // degree "°"
  }
}

# if P073_7DDT_COMMAND

/**
 * FillBufferWithDualTemp()
 * leftTemperature or rightTempareature < -100.0 then shows dashes
 */
void P073_data_struct::FillBufferWithDualTemp(int  leftTemperature,
                                              bool leftWithDecimal,
                                              int  rightTemperature,
                                              bool rightWithDecimal) {
  ClearBuffer();
  char   p073_digit[8];
  String format;
  const bool leftBetween10and0 = (leftWithDecimal && (leftTemperature < 10) && (leftTemperature >= 0));

  // To have a zero prefix (0.x and -0.x) display between 0.9 and -0.9 degrees,
  // as all display types use 1 digit for temperatures between 10.0 and -10.0
  const bool leftBetween0andMinus10 = (leftWithDecimal && (leftTemperature < 0) && (leftTemperature > -10));

  if (hideDegree) {
    // Include a space for compensation of the degree symbol
    format = (leftBetween10and0 ? F("  %02d") : (leftBetween0andMinus10 ? F(" %03d") : leftTemperature < -1000 ? F("----") : F("%4d")));
  } else {
    // Include a space for compensation of the degree symbol
    format = (leftBetween10and0 ? F(" %02d ") : (leftBetween0andMinus10 ? F("%03d ") : leftTemperature < -100 ? F("----") : F("%3d ")));
  }
  bool rightBetween10and0 = (rightWithDecimal && (rightTemperature < 10) && (rightTemperature >= 0));

  // To have a zero prefix (0.x and -0.x) display between 0.9 and -0.9 degrees,
  // as all display types use 1 digit for temperatures between 10.0 and -10.0
  const bool rightBetween0andMinus10 = (rightWithDecimal && (rightTemperature < 0) && (rightTemperature > -10));

  if (hideDegree) {
    format += (rightBetween10and0 ? F("  %02d") : (rightBetween0andMinus10 ? F(" %03d") : rightTemperature < -1000 ? F("----") : F("%4d")));
  } else {
    format += (rightBetween10and0 ? F(" %02d") : (rightBetween0andMinus10 ? F("%03d") : rightTemperature < -100 ? F("----") : F("%3d")));
  }
  sprintf_P(p073_digit, format.c_str(), leftTemperature, rightTemperature);
  const size_t p073_numlenght = strlen(p073_digit);

  for (size_t i = 0; i < p073_numlenght; ++i) {
    showbuffer[i] = P073_mapCharToFontPosition(p073_digit[i], fontset);
  }

  if (!hideDegree) {
    if (leftTemperature  > -100.0) {
      showbuffer[3] = 12; // degree "°"
    }

    if (rightTemperature > -100.0) {
      showbuffer[7] = 12; // degree "°"
    }
  }

  // addLog(LOG_LEVEL_INFO, concat(F("7dgt format: "), format));
}

# endif // if P073_7DDT_COMMAND

void P073_data_struct::FillBufferWithString(const String& textToShow,
                                            bool          useBinaryData) {
  # if P073_7DBIN_COMMAND
  binaryData = useBinaryData;
  # endif // if P073_7DBIN_COMMAND
  ClearBuffer();
  const int p073_txtlength = textToShow.length();

  int p = 0;

  for (int i = 0; i < p073_txtlength && p <= 8; ++i) { // p <= 8 to allow a period after last digit
    if (periods
        && textToShow.charAt(i) == '.'
        # if P073_7DBIN_COMMAND
        && !binaryData
        # endif // if P073_7DBIN_COMMAND
        ) {         // If setting periods true
      if (p == 0) { // Text starts with a period, becomes a space with a dot
        showperiods[p] = true;
        p++;
      } else {
        // if (p > 0) {
        showperiods[p - 1] = true;                        // The period displays as a dot on the previous digit!
      }

      if ((i > 0) && (textToShow.charAt(i - 1) == '.')) { // Handle consecutive periods
        p++;

        if ((p - 1) < 8) {
          showperiods[p - 1] = true; // The period displays as a dot on the previous digit!
        }
      }
    } else if (p < 8) {
      # if P073_7DBIN_COMMAND
      showbuffer[p] = useBinaryData ? textToShow.charAt(i) : P073_mapCharToFontPosition(textToShow.charAt(i), fontset);
      # else // if fP073_7DBIN_COMMAND
      showbuffer[p] = P073_mapCharToFontPosition(textToShow.charAt(i), fontset);
      # endif // if P073_7DBIN_COMMAND
      p++;
    }
  }
  # ifdef P073_DEBUG
  LogBufferContent(F("7dtext"));
  # endif // ifdef P073_DEBUG
}

# if P073_SCROLL_TEXT
int P073_data_struct::getEffectiveTextLength(const String& text) {
  const int textLength = text.length();
  int p                = 0;

  for (int i = 0; i < textLength; ++i) {
    if (periods && (text.charAt(i) == '.')) { // If setting periods true
      if (p == 0) {                           // Text starts with a period, becomes a space with a dot
        p++;
      }

      if ((i > 0) && (text.charAt(i - 1) == '.')) { // Handle consecutive periods
        p++;
      }
    } else {
      p++;
    }
  }
  return p;
}

bool P073_data_struct::NextScroll() {
  bool result = false;

  if (isScrollEnabled() && (!_textToScroll.isEmpty())) {
    if ((scrollCount > 0) && (scrollCount < 0xFFFF)) { scrollCount--; }

    if (scrollCount == 0) {
      scrollCount = 0xFFFF; // Max value to avoid interference when scrolling long texts
      result      = true;
      const int bufToFill      = P073_getDefaultDigits(displayModel, digits);
      const int p073_txtlength = _textToScroll.length();
      ClearBuffer();

      int p = 0;

      for (int i = scrollPos; i < p073_txtlength && p <= bufToFill; ++i) { // p <= bufToFill to allow a period after last digit
        if (periods
            && _textToScroll.charAt(i) == '.'
            #  if P073_7DBIN_COMMAND
            && !binaryData
            #  endif // if P073_7DBIN_COMMAND
            ) {         // If setting periods true
          if (p == 0) { // Text starts with a period, becomes a space with a dot
            showperiods[p] = true;
            p++;
          } else {
            showperiods[p - 1] = true;                                   // The period displays as a dot on the previous digit!
          }

          if ((i > scrollPos) && (_textToScroll.charAt(i - 1) == '.')) { // Handle consecutive periods
            showperiods[p - 1] = true;                                   // The period displays as a dot on the previous digit!
            p++;
          }
        } else if (p < bufToFill) {
          #  if P073_7DBIN_COMMAND
          showbuffer[p] = binaryData ?
                          _textToScroll.charAt(i) :
                          P073_mapCharToFontPosition(_textToScroll.charAt(i), fontset);
          #  else // if P073_7DBIN_COMMAND
          showbuffer[p] = P073_mapCharToFontPosition(_textToScroll.charAt(i), fontset);
          #  endif // if P073_7DBIN_COMMAND
          p++;
        }
      }
      scrollPos++;

      if (scrollPos > _textToScroll.length() - bufToFill) {
        scrollPos = 0;            // Restart when all text displayed
      }
      scrollCount = _scrollSpeed; // Restart countdown
      #  ifdef P073_DEBUG
      LogBufferContent(F("nextScroll"));
      #  endif // ifdef P073_DEBUG
    }
  }
  return result;
}

void P073_data_struct::setTextToScroll(const String& text) {
  _textToScroll = String();

  if (!text.isEmpty()) {
    const int bufToFill = P073_getDefaultDigits(displayModel, digits);
    _textToScroll.reserve(text.length() + bufToFill + (scrollFull ? bufToFill : 0));

    for (int i = 0; scrollFull && i < bufToFill; ++i) { // Scroll text in from the right, so start with all spaces
      _textToScroll +=
        #  if P073_7DBIN_COMMAND
        binaryData ? (char)0x00 :
        #  endif // if P073_7DBIN_COMMAND
        ' ';
    }
    _textToScroll += text;

    for (int i = 0; i < bufToFill; ++i) { // Scroll text off completely before restarting
      _textToScroll +=
        #  if P073_7DBIN_COMMAND
        binaryData ? (char)0x00 :
        #  endif // if P073_7DBIN_COMMAND
        ' ';
    }
  }
  scrollCount = _scrollSpeed;
  scrollPos   = 0;
  #  if P073_7DBIN_COMMAND
  binaryData = false;
  #  endif // if P073_7DBIN_COMMAND
}

void P073_data_struct::setScrollSpeed(uint8_t speed) {
  _scrollSpeed = speed;
  scrollCount  = _scrollSpeed;
  scrollPos    = 0;
}

bool P073_data_struct::isScrollEnabled() {
  return txtScrolling && scrollAllowed;
}

void P073_data_struct::setScrollEnabled(bool scroll) {
  scrollAllowed = scroll;
}

# endif // if P073_SCROLL_TEXT

# if P073_7DBIN_COMMAND
void P073_data_struct::setBinaryData(const String& data) {
  binaryData = true;
  #  if P073_SCROLL_TEXT
  setTextToScroll(data);
  binaryData  = true; // is reset in setTextToScroll
  scrollCount = _scrollSpeed;
  scrollPos   = 0;
  #  else // if P073_SCROLL_TEXT
  _textToScroll = data;
  #  endif // if P073_SCROLL_TEXT
}

# endif // if P073_7DBIN_COMMAND

# ifdef P073_DEBUG
void P073_data_struct::LogBufferContent(String prefix) {
  String log;

  if (loglevelActiveFor(LOG_LEVEL_INFO) &&
      log.reserve(48)) {
    log = strformat(F("%s buffer: periods: %c "), prefix.c_str(), periods ? 't' : 'f');

    for (uint8_t i = 0; i < 8; ++i) {
      if (i > 0) { log += ','; }
      log += formatToHex(showbuffer[i]);
      log += ',';
      log += showperiods[i] ? F(".") : F("");
    }
    addLogMove(LOG_LEVEL_INFO, log);
  }
}

# endif // ifdef P073_DEBUG

// in case of error show all dashes
void P073_data_struct::FillBufferWithDash() {
  memset(showbuffer, 11, sizeof(showbuffer));
}

void P073_data_struct::ClearBuffer() {
  memset(showbuffer,
         # if P073_7DBIN_COMMAND
         binaryData ? 0 :
         # endif // if P073_7DBIN_COMMAND
         10, sizeof(showbuffer));

  for (uint8_t i = 0; i < 8; ++i) {
    showperiods[i] = false;
  }
}

uint8_t P073_data_struct::tm1637_getFontChar(uint8_t index,
                                             uint8_t fontset) {
  return P073_revert7bits(P073_getFontChar(index, fontset));
}

bool P073_data_struct::plugin_once_a_second(struct EventStruct *event) {
  if (output == P073_DISP_MANUAL) {
    return false;
  }

  if ((output == P073_DISP_CLOCK24BLNK) ||
      (output == P073_DISP_CLOCK12BLNK)) {
    timesep = !timesep;
  } else {
    timesep = true;
  }

  if (output == P073_DISP_DATE) {
    FillBufferWithDate(true, 0, 0, 0,
                       # if P073_SUPPRESS_ZERO
                       suppressLeading0
                       # else // if P073_SUPPRESS_ZERO
                       false
                       # endif // if P073_SUPPRESS_ZERO
                       );
  } else {
    FillBufferWithTime(true, 0, 0, 0, !((output == P073_DISP_CLOCK24BLNK) ||
                                        (output == P073_DISP_CLOCK24)),
                       # if P073_SUPPRESS_ZERO
                       suppressLeading0
                       # else // if P073_SUPPRESS_ZERO
                       false
                       # endif // if P073_SUPPRESS_ZERO
                       );
  }

  #if P073_BLINK_DOT

  if (blinkdot &&
      ((output == P073_DISP_CLOCK24BLNK) ||
       (output == P073_DISP_CLOCK12BLNK))) {
    showperiods[1] = timesep; // Blink dot on second digit
  }
  #endif // if P073_BLINK_DOT

  switch (displayModel) {
    case P073_TM1637_4DGTCOLON:
    case P073_TM1637_4DGTDOTS:
      tm1637_ShowTimeTemp4(timesep, 0);
      break;
    case P073_TM1637_6DGT:

      if (P073_CFG_OUTPUTTYPE == P073_DISP_DATE) {
        tm1637_ShowDate6();
      } else {
        tm1637_ShowTime6();
      }
      break;
    case P073_MAX7219_8DGT:

      if (P073_CFG_OUTPUTTYPE == P073_DISP_DATE) {
        max7219_ShowDate();
      } else {
        max7219_ShowTime(timesep);
      }
      break;
    # if P073_USE_74HC595
    case P073_74HC595_2_8DGT:
      hc595_AdjustBuffer();
      hc595_ToOutputBuffer();

      if (hc595_Sequential()) { // Sequential displays don't need continuous refreshing
        hc595_ShowBuffer();
      }
      break;
    # endif // if P073_USE_74HC595
  }
  return true;
}

# if P073_SCROLL_TEXT
bool P073_data_struct::plugin_ten_per_second(struct EventStruct *event) {
  if ((output != P073_DISP_MANUAL) || !isScrollEnabled()) {
    return false;
  }

  if (NextScroll()) {
    switch (displayModel) {
      case P073_TM1637_4DGTCOLON:
      case P073_TM1637_4DGTDOTS: {
        tm1637_ShowBuffer(0, 4
                          #  if P073_7DBIN_COMMAND
                          , binaryData
                          #  endif // if P073_7DBIN_COMMAND
                          );
        break;
      }
      case P073_TM1637_6DGT: {
        tm1637_SwapDigitInBuffer(0); // only needed for 6-digits displays
        tm1637_ShowBuffer(0, 6
                          #  if P073_7DBIN_COMMAND
                          , binaryData
                          #  endif // if P073_7DBIN_COMMAND
                          );
        break;
      }
      case P073_MAX7219_8DGT: {
        dotpos = -1; // avoid to display the dot
        max7219_ShowBuffer();
        break;
      }
      #  if P073_USE_74HC595
      case P073_74HC595_2_8DGT: {
        hc595_ToOutputBuffer();

        if (hc595_Sequential()) { // Sequential displays don't need continuous refreshing
          hc595_ShowBuffer();
        }
        break;
      }
      #  endif // if P073_USE_74HC595
    }
  }
  return true;
}

# endif // if P073_SCROLL_TEXT

const char p073_commands[] PROGMEM =
  "7dn|7dt|"
  # if P073_7DDT_COMMAND
  "7ddt|"
  # endif // if P073_7DDT_COMMAND
  "7dst|7dsd|7dtext|"
  # if P073_EXTRA_FONTS
  "7dfont|"
  # endif // if P073_EXTRA_FONTS
  # if P073_7DBIN_COMMAND
  "7dbin|"
  # endif // if P073_7DBIN_COMMAND
  "7don|7doff|7db|7output|"
;
enum class p073_commands_e : int8_t {
  invalid = -1,
  c7dn    = 0,
  c7dt,
  # if P073_7DDT_COMMAND
  c7ddt,
  # endif // if P073_7DDT_COMMAND
  c7dst,
  c7dsd,
  c7dtext,
  # if P073_EXTRA_FONTS
  c7dfont,
  # endif // if P073_EXTRA_FONTS
  # if P073_7DBIN_COMMAND
  c7dbin,
  # endif // if P073_7DBIN_COMMAND
  c7don,
  c7doff,
  c7db,
  c7output,
};

bool P073_data_struct::plugin_write(struct EventStruct *event,
                                    const String      & string) {
  const String cmd_s = parseString(string, 1);

  if ((cmd_s.length() < 3) || (cmd_s[0] != '7')) { return false; }

  # if P073_SCROLL_TEXT
  const bool currentScroll = isScrollEnabled(); // Save current state
  bool newScroll           = false;             // disable scroll if command changes
  setScrollEnabled(false);
  # endif // if P073_SCROLL_TEXT

  const int cmd_i = GetCommandCode(cmd_s.c_str(), p073_commands);

  if (cmd_i < 0) { return false; } // Fail fast

  const p073_commands_e cmd = static_cast<p073_commands_e>(cmd_i);

  const String text = parseStringToEndKeepCase(string, 2);
  bool success      = false;
  bool displayon    = false;

  switch (cmd) {
    case p073_commands_e::c7dn:
      return plugin_write_7dn(event, text);
    case p073_commands_e::c7dt:
      return plugin_write_7dt(text);
    # if P073_7DDT_COMMAND
    case p073_commands_e::c7ddt:
      return plugin_write_7ddt(text);
    # endif // if P073_7DDT_COMMAND
    case p073_commands_e::c7dst:
      return plugin_write_7dst(event);
    case p073_commands_e::c7dsd:
      return plugin_write_7dsd(event);
    case p073_commands_e::c7dtext:
      # if P073_SCROLL_TEXT
      setScrollEnabled(true); // Scrolling allowed for 7dtext command
      # endif // if P073_SCROLL_TEXT
      return plugin_write_7dtext(text);
    # if P073_EXTRA_FONTS
    case p073_commands_e::c7dfont:
      #  if P073_SCROLL_TEXT
      setScrollEnabled(currentScroll); // Restore state
      #  endif // if P073_SCROLL_TEXT
      return plugin_write_7dfont(event, text);
    # endif // if P073_EXTRA_FONTS
    # if P073_7DBIN_COMMAND
    case p073_commands_e::c7dbin:
      #  if P073_SCROLL_TEXT
      setScrollEnabled(true); // Scrolling allowed for 7dbin command
      #  endif // if P073_SCROLL_TEXT
      return plugin_write_7dbin(text);
    # endif // if P073_7DBIN_COMMAND
    case p073_commands_e::c7don:
      # if P073_SCROLL_TEXT
      newScroll = currentScroll; // Restore state
      # endif // if P073_SCROLL_TEXT
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_INFO, F("7DGT : Display ON"));
      # endif // ifndef BUILD_NO_DEBUG
      displayon = true;
      success   = true;
      break;
    case p073_commands_e::c7doff:
      # if P073_SCROLL_TEXT
      newScroll = currentScroll; // Restore state
      # endif // if P073_SCROLL_TEXT
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_INFO, F("7DGT : Display OFF"));
      # endif // ifndef BUILD_NO_DEBUG
      displayon = false;
      success   = true;
      break;
    case p073_commands_e::c7db:
      # if P073_SCROLL_TEXT
      newScroll = currentScroll; // Restore state
      # endif // if P073_SCROLL_TEXT

      if ((event->Par1 >= 0) && (event->Par1 < 16)) {
        # ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLog(LOG_LEVEL_INFO, concat(F("7DGT : Brightness="), event->Par1));
        }
        # endif // ifndef BUILD_NO_DEBUG
        brightness          = event->Par1;
        P073_CFG_BRIGHTNESS = event->Par1;
        displayon           = true;
        success             = true;
      }
      break;
    case p073_commands_e::c7output:

      if ((event->Par1 >= 0) && (event->Par1 < 6)) { // 0:"Manual",1:"Clock 24h - Blink",2:"Clock 24h - No Blink",
                                                     // 3:"Clock 12h - Blink",4:"Clock 12h - No Blink",5:"Date"
        # ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLog(LOG_LEVEL_INFO, concat(F("7DGT : Display output="), event->Par1));
        }
        # endif // ifndef BUILD_NO_DEBUG
        output              = event->Par1;
        P073_CFG_OUTPUTTYPE = event->Par1;
        displayon           = true;
        success             = true;
        # if P073_SCROLL_TEXT

        if (event->Par1 == 0) { newScroll = currentScroll; } // Restore state
        # endif // if P073_SCROLL_TEXT
      }
      break;
    case p073_commands_e::invalid:
      break;
  }

  if (success) {
    # if P073_SCROLL_TEXT
    setScrollEnabled(newScroll);
    # endif // if P073_SCROLL_TEXT

    switch (displayModel) {
      case P073_TM1637_4DGTCOLON:
      case P073_TM1637_4DGTDOTS:
      case P073_TM1637_6DGT:
        tm1637_SetPowerBrightness(brightness / 2, displayon);
        break;
      case P073_MAX7219_8DGT:
        max7219_SetPowerBrightness(brightness, displayon);
        break;
      # if P073_USE_74HC595
      case P073_74HC595_2_8DGT:
        // 74HC595 don't have a brightness setting
        break;
      # endif // if P073_USE_74HC595
    }
  }
  return success;
}

void P073_data_struct::getDisplayLimits(int32_t& lLimit,
                                        int32_t& uLimit,
                                        int8_t   offset,
                                        uint8_t  digits) {
  uint8_t dgts = P073_getDefaultDigits(displayModel);

  # if P073_USE_74HC595

  if (P073_74HC595_2_8DGT == displayModel) {
    dgts = digits;
  }
  # endif // if P073_USE_74HC595
  dgts  -= offset;           // Subtract an offset, used for extra symbol
  lLimit = -pow10(dgts - 1); // Lowest value we can display - 1
  uLimit = pow10(dgts);      // Highest value we can display + 1
  // TODO disable log
  // addLog(LOG_LEVEL_INFO, strformat(F("P073: limits: %d digits(%d), lower: %d, upper: %d"), dgts, offset, lLimit, uLimit));
}

bool P073_data_struct::plugin_write_7dn(struct EventStruct *event,
                                        const String      & text) {
  if (output != P073_DISP_MANUAL) {
    return false;
  }

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, concat(F("7DGT : Show Number="), event->Par1));
  }
  # endif // ifndef BUILD_NO_DEBUG

  int32_t lLimit = 0;
  int32_t uLimit = 0;
  getDisplayLimits(lLimit, uLimit, 0, digits);

  if (!text.isEmpty()) {
    if ((event->Par1 > lLimit) && (event->Par1 < uLimit)) {
      FillBufferWithNumber(text.c_str());
    } else {
      FillBufferWithDash();
    }
  }

  switch (displayModel) {
    case P073_TM1637_4DGTCOLON:
    case P073_TM1637_4DGTDOTS:
      tm1637_ShowBuffer(TM1637_4DIGIT, 8);
      break;
    case P073_TM1637_6DGT:
      tm1637_SwapDigitInBuffer(2); // only needed for 6-digits displays
      tm1637_ShowBuffer(TM1637_6DIGIT, 8);
      break;
    case P073_MAX7219_8DGT:
      max7219_ShowBuffer();
      break;
    # if P073_USE_74HC595
    case P073_74HC595_2_8DGT:
      hc595_AdjustBuffer();
      hc595_ToOutputBuffer();

      if (hc595_Sequential()) { // Sequential displays don't need continuous refreshing
        hc595_ShowBuffer();
      }
      break;
    # endif // if P073_USE_74HC595
  }
  return true;
}

bool P073_data_struct::plugin_write_7dt(const String& text) {
  if  (output != P073_DISP_MANUAL) {
    return false;
  }

  float p073_temptemp    = 0;
  bool  p073_tempflagdot = false;

  if (!text.isEmpty()) {
    validFloatFromString(text, p073_temptemp);
  }

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, concat(F("7DGT : Show Temperature="), p073_temptemp));
  }
  # endif // ifndef BUILD_NO_DEBUG

  int32_t lLimit = 0;
  int32_t uLimit = 0;
  getDisplayLimits(lLimit, uLimit, hideDegree ? 0 : 1, digits);
  float lLimitErr = lLimit + 0.1f;
  float uLimitErr = uLimit - 1.0f;
  float lLimitDec = lLimit / 10.0f;
  float uLimitDec = uLimit / 10.0f;

  // TODO disable log
  // addLog(LOG_LEVEL_INFO, strformat(F("P073: 7dt: lErr: %.1f, uErr: %.1f, lDec: %.1f, uDec: %.1f"),
  //                                  lLimitErr, uLimitErr, lLimitDec, uLimitDec));

  if ((p073_temptemp > uLimitErr) || (p073_temptemp < lLimitErr)) {
    FillBufferWithDash();
  } else {
    if ((p073_temptemp < uLimitDec) && (p073_temptemp > lLimitDec)) {
      p073_temptemp    = roundf(p073_temptemp * 10.0f);
      p073_tempflagdot = true;
    }
    FillBufferWithTemp(p073_temptemp);
  }

  switch (displayModel) {
    case P073_TM1637_4DGTCOLON:
    case P073_TM1637_4DGTDOTS:
    case P073_TM1637_6DGT:

      if ((p073_temptemp == 0) && p073_tempflagdot) {
        showbuffer[5] = 0;
      }

      if (P073_TM1637_6DGT == displayModel) {
        tm1637_ShowTemp6(p073_tempflagdot);
      } else {
        tm1637_ShowTimeTemp4(p073_tempflagdot, 4);
      }
      break;
    case P073_MAX7219_8DGT:
      # ifdef P073_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLogMove(LOG_LEVEL_INFO, concat(F("7DGT : 7dt preprocessed ="), p073_temptemp));
      }
      # endif // ifdef P073_DEBUG

      max7219_ShowTemp(hideDegree ? 6 : 5, -1);
      break;
    # if P073_USE_74HC595
    case P073_74HC595_2_8DGT:
      hc595_AdjustBuffer();
      hc595_ToOutputBuffer();

      if (hc595_Sequential()) { // Sequential displays don't need continuous refreshing
        hc595_ShowBuffer();
      }
      break;
    # endif // if P073_USE_74HC595
  }
  # ifdef P073_DEBUG
  LogBufferContent(F("7dt"));
  # endif // ifdef P073_DEBUG
  return true;
}

# if P073_7DDT_COMMAND
bool P073_data_struct::plugin_write_7ddt(const String& text) {
  if (output != P073_DISP_MANUAL) {
    return false;
  }

  float p073_lefttemp    = 0.0f;
  float p073_righttemp   = 0.0f;
  bool  p073_tempflagdot = false;

  if (!text.isEmpty()) {
    validFloatFromString(parseString(text, 1), p073_lefttemp);

    if (text.indexOf(',') > -1) {
      validFloatFromString(parseString(text, 2), p073_righttemp);
    }
  }

  #  ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, strformat(F("7DGT : Dual Temperature 1st=%.2f 2nd=%.2f"), p073_lefttemp, p073_righttemp));
  }
  #  endif // ifndef BUILD_NO_DEBUG

  switch (displayModel) {
    case P073_TM1637_4DGTCOLON:
    case P073_TM1637_4DGTDOTS:
    case P073_TM1637_6DGT:
    {
      FillBufferWithDash();

      if (displayModel == P073_TM1637_6DGT) {
        tm1637_ShowTemp6(p073_tempflagdot);
      } else {
        tm1637_ShowTimeTemp4(p073_tempflagdot, 4);
      }
      break;
    }
    case P073_MAX7219_8DGT:
    case P073_74HC595_2_8DGT:
    {
      uint8_t firstDot   = -1; // No decimals is no dots
      uint8_t secondDot  = -1;
      float   hideFactor = hideDegree ? 10.0f : 1.0f;

      if ((p073_lefttemp > 999.99f * hideFactor) || (p073_lefttemp < -99.99f * hideFactor)) {
        p073_lefttemp = -101.0f * hideFactor; // Triggers on -100
      } else {
        if ((p073_lefttemp < 100.0f * hideFactor) && (p073_lefttemp > -10.0f * hideFactor)) {
          p073_lefttemp = roundf(p073_lefttemp * 10.0f);
          firstDot      = hideDegree ? 2 : 1;
        }
      }

      if ((p073_righttemp > 999.99f * hideFactor) || (p073_righttemp < -99.99f * hideFactor)) {
        p073_righttemp = -101.0f * hideFactor;
      } else {
        if ((p073_righttemp < 100.0f * hideFactor) && (p073_righttemp > -10.0f * hideFactor)) {
          p073_righttemp = roundf(p073_righttemp * 10.0f);
          secondDot      = hideDegree ? 6 : 5;
        }
      }

      #  ifdef P073_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLog(LOG_LEVEL_INFO, strformat(F("7DGT : 7ddt preprocessed 1st=%.2f 2nd=%.2f"), p073_lefttemp, p073_righttemp));
      }
      #  endif // ifdef P073_DEBUG

      FillBufferWithDualTemp(p073_lefttemp, firstDot > -1, p073_righttemp, secondDot > -1);

      if (P073_MAX7219_8DGT == displayModel) {
        bool alignSave = rightAlignTempMAX7219; // Save setting
        rightAlignTempMAX7219 = true;

        max7219_ShowTemp(firstDot, secondDot);

        rightAlignTempMAX7219 = alignSave; // Restore
      #  if P073_USE_74HC595
      } else

      // if (P073_74HC595_2_8DGT == P073_data->displayModel)
      {
        if (digits < 8) {
          FillBufferWithDash();
        }
        hc595_ToOutputBuffer();

        if (hc595_Sequential()) { // Sequential displays don't need continuous refreshing
          hc595_ShowBuffer();
        }
      #  endif // if P073_USE_74HC595
      }

      break;
    }
  }
  #  ifdef P073_DEBUG
  LogBufferContent(F("7ddt"));
  #  endif // ifdef P073_DEBUG
  return true;
}

# endif // if P073_7DDT_COMMAND

bool P073_data_struct::plugin_write_7dst(struct EventStruct *event) {
  if (output != P073_DISP_MANUAL) {
    return false;
  }

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, strformat(F("7DGT : Show Time=%02d:%02d:%02d"), event->Par1, event->Par2, event->Par3));
  }
  # endif // ifndef BUILD_NO_DEBUG
  timesep = true;
  FillBufferWithTime(false, event->Par1, event->Par2, event->Par3, false,
                     # if P073_SUPPRESS_ZERO
                     suppressLeading0
                     # else // if P073_SUPPRESS_ZERO
                     false
                     # endif // if P073_SUPPRESS_ZERO
                     );

  switch (displayModel) {
    case P073_TM1637_4DGTCOLON:
    case P073_TM1637_4DGTDOTS:
      tm1637_ShowTimeTemp4(timesep, 0);
      break;
    case P073_TM1637_6DGT:
      tm1637_ShowTime6();
      break;
    case P073_MAX7219_8DGT:
      max7219_ShowTime(timesep);
      break;
    # if P073_USE_74HC595
    case P073_74HC595_2_8DGT:
      hc595_AdjustBuffer();
      hc595_ToOutputBuffer();

      if (hc595_Sequential()) { // Sequential displays don't need continuous refreshing
        hc595_ShowBuffer();
      }
      break;
    # endif // if P073_USE_74HC595
  }
  return true;
}

bool P073_data_struct::plugin_write_7dsd(struct EventStruct *event) {
  if (output != P073_DISP_MANUAL) {
    return false;
  }

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, strformat(F("7DGT : Show Date=%02d:%02d:%02d"), event->Par1, event->Par2, event->Par3));
  }
  # endif // ifndef BUILD_NO_DEBUG
  FillBufferWithDate(false, event->Par1, event->Par2, event->Par3,
                     # if P073_SUPPRESS_ZERO
                     suppressLeading0
                     # else // if P073_SUPPRESS_ZERO
                     false
                     # endif // if P073_SUPPRESS_ZERO
                     );

  switch (displayModel) {
    case P073_TM1637_4DGTCOLON:
    case P073_TM1637_4DGTDOTS:
      tm1637_ShowTimeTemp4(timesep, 0);
      break;
    case P073_TM1637_6DGT:
      tm1637_ShowDate6();
      break;
    case P073_MAX7219_8DGT:
      max7219_ShowDate();
      break;
    # if P073_USE_74HC595
    case P073_74HC595_2_8DGT:
      hc595_AdjustBuffer();
      hc595_ToOutputBuffer();

      if (hc595_Sequential()) { // Sequential displays don't need continuous refreshing
        hc595_ShowBuffer();
      }
      break;
    # endif // if P073_USE_74HC595
  }
  return true;
}

bool P073_data_struct::plugin_write_7dtext(const String& text) {
  if (output != P073_DISP_MANUAL) {
    return false;
  }

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLogMove(LOG_LEVEL_INFO, concat(F("7DGT : Show Text="), text));
  }
  # endif // ifndef BUILD_NO_DEBUG
  # if P073_SCROLL_TEXT
  setTextToScroll(EMPTY_STRING);
  const uint8_t bufLen = P073_getDefaultDigits(displayModel, digits);

  if (isScrollEnabled() && (getEffectiveTextLength(text) > bufLen)) {
    setTextToScroll(text);
  } else
  # endif // if P073_SCROLL_TEXT
  {
    FillBufferWithString(text);

    switch (displayModel) {
      case P073_TM1637_4DGTCOLON:
      case P073_TM1637_4DGTDOTS:
        tm1637_ShowBuffer(0, 4);
        break;
      case P073_TM1637_6DGT:
        tm1637_SwapDigitInBuffer(0); // only needed for 6-digits displays
        tm1637_ShowBuffer(0, 6);
        break;
      case P073_MAX7219_8DGT:
        dotpos = -1; // avoid to display the dot
        max7219_ShowBuffer();
        break;
      # if P073_USE_74HC595
      case P073_74HC595_2_8DGT:
        hc595_ToOutputBuffer();

        if (hc595_Sequential()) { // Sequential displays don't need continuous refreshing
          hc595_ShowBuffer();
        }
        break;
      # endif // if P073_USE_74HC595
    }
  }
  return true;
}

# if P073_EXTRA_FONTS
bool P073_data_struct::plugin_write_7dfont(struct EventStruct *event,
                                           const String      & text) {
  if (!text.isEmpty()) {
    int32_t fontNr = P073_parse_7dfont(event, text);
    #  ifdef P073_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO, strformat(F("P073 7dfont,%s -> %d"), parseString(text, 1).c_str(), fontNr));
    }
    #  endif // ifdef P073_DEBUG

    if ((fontNr >= 0) && (fontNr <= 3)) {
      fontset          = fontNr;
      P073_CFG_FONTSET = fontNr;
      return true;
    }
  }
  return false;
}

# endif // if P073_EXTRA_FONTS

# if P073_7DBIN_COMMAND
bool P073_data_struct::plugin_write_7dbin(const String& text) {
  if (!text.isEmpty()) {
    String  data;
    int32_t byteValue{};
    int     arg      = 1;
    String  argValue = parseString(text, arg);

    while (!argValue.isEmpty()) {
      if (validIntFromString(argValue, byteValue) && (byteValue < 256) && (byteValue > -1)) {
        data += static_cast<char>(displayModel == P073_MAX7219_8DGT ?
                                  byteValue :
                                  P073_revert7bits(byteValue));
      }
      arg++;
      argValue = parseString(text, arg);
    }
    #  if P073_SCROLL_TEXT
    const uint8_t bufLen = P073_getDefaultDigits(displayModel, digits);
    #  endif // if P073_SCROLL_TEXT

    if (!data.isEmpty()) {
      #  if P073_SCROLL_TEXT
      setTextToScroll(EMPTY_STRING); // Clear any scrolling text

      if (isScrollEnabled() && (data.length() > bufLen)) {
        setBinaryData(data);
      } else
      #  endif // if P073_SCROLL_TEXT
      {
        FillBufferWithString(data, true);

        switch (displayModel) {
          case P073_TM1637_4DGTCOLON:
          case P073_TM1637_4DGTDOTS:
            tm1637_ShowBuffer(0, 4);
            break;
          case P073_TM1637_6DGT:
            tm1637_SwapDigitInBuffer(0); // only needed for 6-digits displays
            tm1637_ShowBuffer(0, 6, true);
            break;
          case P073_MAX7219_8DGT:
            dotpos = -1; // avoid to display the dot
            max7219_ShowBuffer();
            break;
          #  if P073_USE_74HC595
          case P073_74HC595_2_8DGT:
            hc595_ToOutputBuffer();

            if (hc595_Sequential()) { // Sequential displays don't need continuous refreshing
              hc595_ShowBuffer();
            }
            break;
          #  endif // if P073_USE_74HC595
        }
      }
      return true;
    }
  }
  return false;
}

# endif // if P073_7DBIN_COMMAND

// ===================================
// ---- TM1637 specific functions ----
// ===================================

# define CLK_HIGH() DIRECT_pinWrite(this->pin1, HIGH)
# define CLK_LOW() DIRECT_pinWrite(this->pin1, LOW)
# define DIO_HIGH() DIRECT_PINMODE_INPUT(this->pin2)
# define DIO_LOW() DIRECT_PINMODE_OUTPUT(this->pin2) //; DIRECT_pinWrite(this->pin2, LOW)

void P073_data_struct::tm1637_i2cStart() {
  # ifdef P073_DEBUG
  addLog(LOG_LEVEL_DEBUG, F("7DGT : Comm Start"));
  # endif // ifdef P073_DEBUG
  DIO_HIGH();
  CLK_HIGH();
  delayMicroseconds(TM1637_CLOCKDELAY);
  DIO_LOW();
}

void P073_data_struct::tm1637_i2cStop() {
  # ifdef P073_DEBUG
  addLog(LOG_LEVEL_DEBUG, F("7DGT : Comm Stop"));
  # endif // ifdef P073_DEBUG
  CLK_LOW();
  delayMicroseconds(TM1637_CLOCKDELAY);
  DIO_LOW();
  delayMicroseconds(TM1637_CLOCKDELAY);
  CLK_HIGH();
  delayMicroseconds(TM1637_CLOCKDELAY);
  DIO_HIGH();
}

void P073_data_struct::tm1637_i2cAck() {
  CLK_LOW();
  pinMode(pin2, INPUT_PULLUP);

  // DIO_HIGH();
  delayMicroseconds(TM1637_CLOCKDELAY);

  // while(digitalRead(dio_pin));
  # ifdef P073_DEBUG
  const bool dummyAck =
  # endif // ifdef P073_DEBUG
  DIRECT_pinRead(pin2);

  # ifdef P073_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("7DGT : Comm ACK=");

    if (dummyAck == 0) {
      log += F("TRUE");
    } else {
      log += F("FALSE");
    }
    addLogMove(LOG_LEVEL_DEBUG, log);
  }
  # endif // ifdef P073_DEBUG
  CLK_HIGH();
  delayMicroseconds(TM1637_CLOCKDELAY);
  CLK_LOW();
  DIO_LOW();
}

void P073_data_struct::tm1637_i2cWrite_ack(uint8_t bytesToPrint[],
                                           uint8_t length) {
  #ifdef P073_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, concat(F("7DGT : TM1637 databuffer: 0x"), formatToHex_array(bytesToPrint, length)));
  }
  #endif // ifdef P073_DEBUG
  tm1637_i2cStart();

  for (uint8_t i = 0; i < length; ++i) {
    tm1637_i2cWrite_ack(bytesToPrint[i]);
  }
  tm1637_i2cStop();
}

void P073_data_struct::tm1637_i2cWrite_ack(uint8_t bytetoprint) {
  tm1637_i2cWrite(bytetoprint);
  tm1637_i2cAck();
}

void P073_data_struct::tm1637_i2cWrite(uint8_t bytetoprint) {
  # ifdef P073_DEBUG
  addLog(LOG_LEVEL_DEBUG, F("7DGT : WriteByte"));
  # endif // ifdef P073_DEBUG
  uint8_t i;

  for (i = 0; i < 8; ++i) {
    CLK_LOW();

    if (bytetoprint & 0b00000001) {
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

void P073_data_struct::tm1637_ClearDisplay() {
  uint8_t bytesToPrint[7]{};

  bytesToPrint[0] = 0xC0;
  tm1637_i2cWrite_ack(bytesToPrint, 7);
}

void P073_data_struct::tm1637_SetPowerBrightness(uint8_t brightlvl,
                                                 bool    poweron) {
  # ifdef P073_DEBUG
  addLog(LOG_LEVEL_INFO, F("7DGT : Set BRIGHT"));
  # endif // ifdef P073_DEBUG
  uint8_t brightvalue = (brightlvl & 0b111);

  if (poweron) {
    brightvalue = TM1637_POWER_ON | brightvalue;
  } else {
    brightvalue = TM1637_POWER_OFF | brightvalue;
  }

  const uint8_t byteToPrint = brightvalue;
  tm1637_i2cWrite_ack(byteToPrint);
}

void P073_data_struct::tm1637_InitDisplay() {
  // pinMode(pin1, OUTPUT);
  // pinMode(pin2, OUTPUT);
  directModeOutput(pin1);
  directModeOutput(pin2);
  CLK_HIGH();
  DIO_HIGH();

  const uint8_t byteToPrint = 0x40;

  tm1637_i2cWrite_ack(byteToPrint);
  tm1637_ClearDisplay();
}

uint8_t P073_data_struct::tm1637_separator(uint8_t value,
                                           bool    sep) {
  if (sep) {
    value |= 0b10000000;
  }
  return value;
}

void P073_data_struct::tm1637_ShowTime6() {
  tm1637_ShowDate6(true); // deduplicated
}

void P073_data_struct::tm1637_ShowDate6(bool showTime) {
  uint8_t bytesToPrint[7]{};

  bytesToPrint[0] = 0xC0;
  bytesToPrint[1] = tm1637_getFontChar(showbuffer[2], fontset);
  bytesToPrint[2] = tm1637_separator(tm1637_getFontChar(showbuffer[1], fontset), timesep);
  bytesToPrint[3] = tm1637_getFontChar(showbuffer[0], fontset);

  if (showTime) {
    bytesToPrint[4] = tm1637_getFontChar(showbuffer[5], fontset);
    bytesToPrint[5] = tm1637_getFontChar(showbuffer[4], fontset);
  } else {
    bytesToPrint[4] = tm1637_getFontChar(showbuffer[7], fontset);
    bytesToPrint[5] = tm1637_getFontChar(showbuffer[6], fontset);
  }
  bytesToPrint[6] = tm1637_separator(tm1637_getFontChar(showbuffer[3], fontset), timesep);

  tm1637_i2cWrite_ack(bytesToPrint, 7);
}

void P073_data_struct::tm1637_ShowTemp6(bool sep) {
  uint8_t bytesToPrint[7]{};

  bytesToPrint[0] = 0xC0;
  bytesToPrint[1] = tm1637_separator(tm1637_getFontChar(showbuffer[5], fontset), sep);
  bytesToPrint[2] = tm1637_getFontChar(showbuffer[4], fontset);
  bytesToPrint[3] = tm1637_getFontChar(10, fontset);
  bytesToPrint[4] = tm1637_getFontChar(10, fontset);
  bytesToPrint[5] = tm1637_getFontChar(showbuffer[7], fontset);
  bytesToPrint[6] = tm1637_getFontChar(showbuffer[6], fontset);

  tm1637_i2cWrite_ack(bytesToPrint, 7);
}

void P073_data_struct::tm1637_ShowTimeTemp4(bool    sep,
                                            uint8_t bufoffset) {
  uint8_t bytesToPrint[5]{};

  bytesToPrint[0] = 0xC0;
  bytesToPrint[1] = tm1637_getFontChar(showbuffer[0 + bufoffset], fontset);
  bytesToPrint[2] = tm1637_separator(tm1637_getFontChar(showbuffer[1 + bufoffset], fontset), sep);
  bytesToPrint[3] = tm1637_getFontChar(showbuffer[2 + bufoffset], fontset);
  bytesToPrint[4] = tm1637_getFontChar(showbuffer[3 + bufoffset], fontset);

  tm1637_i2cWrite_ack(bytesToPrint, 5);
}

void P073_data_struct::tm1637_SwapDigitInBuffer(uint8_t startPos) {
  std::swap(showbuffer[2 + startPos],  showbuffer[0 + startPos]);
  std::swap(showbuffer[3 + startPos],  showbuffer[5 + startPos]);

  std::swap(showperiods[2 + startPos], showperiods[0 + startPos]);
  std::swap(showperiods[3 + startPos], showperiods[5 + startPos]);

  if (dotpos > -1) {
    const uint8_t dotPositionSwap[] = { 0, 1, 4, 3, 2, 7, 6, 5, 8 };

    dotpos = dotPositionSwap[dotpos];
  }
}

void P073_data_struct::tm1637_ShowBuffer(uint8_t firstPos,
                                         uint8_t lastPos,
                                         bool    useBinaryData) {
  uint8_t bytesToPrint[8]{};

  bytesToPrint[0] = 0xC0;
  uint8_t length = 1;

  if (dotpos > -1) {
    showperiods[dotpos] = true;
  }

  uint8_t p073_datashowpos1;

  for (int i = firstPos; i < lastPos; ++i) {
    if (useBinaryData) {
      bytesToPrint[length] = showbuffer[i];
    } else {
      p073_datashowpos1 = tm1637_separator(
        tm1637_getFontChar(showbuffer[i], fontset),
        showperiods[i]);
      bytesToPrint[length] = p073_datashowpos1;
    }
    length++;
  }
  # ifdef P073_DEBUG
  addLog(LOG_LEVEL_INFO, strformat(F("TM1673: Write bytes: %d buffer %d to %d"), length, firstPos, lastPos));
  # endif // ifdef P073_DEBUG
  tm1637_i2cWrite_ack(bytesToPrint, length);
}

// ====================================
// ---- MAX7219 specific functions ----
// ====================================

# define OP_DECODEMODE   9
# define OP_INTENSITY   10
# define OP_SCANLIMIT   11
# define OP_SHUTDOWN    12
# define OP_DISPLAYTEST 15

void P073_data_struct::max7219_spiTransfer(ESPEASY_VOLATILE(uint8_t) opcode,
                                           ESPEASY_VOLATILE(uint8_t) data) {
  spidata[1] = opcode;
  spidata[0] = data;
  digitalWrite(pin3, LOW);
  shiftOut(pin1, pin2, MSBFIRST, spidata[1]);
  shiftOut(pin1, pin2, MSBFIRST, spidata[0]);
  digitalWrite(pin3, HIGH);
}

void P073_data_struct::max7219_ClearDisplay() {
  for (int i = 0; i < 8; i++) {
    max7219_spiTransfer(i + 1, 0);
  }
}

void P073_data_struct::max7219_SetPowerBrightness(uint8_t brightlvl,
                                                  bool    poweron) {
  max7219_spiTransfer(OP_INTENSITY, brightlvl);
  max7219_spiTransfer(OP_SHUTDOWN,  poweron ? 1 : 0);
}

void P073_data_struct::max7219_SetDigit(int     dgtpos,
                                        uint8_t dgtvalue,
                                        bool    showdot,
                                        bool    binaryData) {
  uint8_t p073_tempvalue;

  if (binaryData) {
    p073_tempvalue = dgtvalue; // Overwrite if binary data
  } else
  {
    p073_tempvalue = P073_getFontChar(dgtvalue, fontset);

    if (showdot) {
      p073_tempvalue |= 0b10000000;
    }
  }
  max7219_spiTransfer(dgtpos + 1, p073_tempvalue);
}

void P073_data_struct::max7219_InitDisplay() {
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);
  digitalWrite(pin3, HIGH);
  max7219_spiTransfer(OP_DISPLAYTEST, 0);
  max7219_spiTransfer(OP_SCANLIMIT,   7); // scanlimit setup to max at Init
  max7219_spiTransfer(OP_DECODEMODE,  0);
  max7219_ClearDisplay();
  max7219_SetPowerBrightness(0, false);
}

void P073_data_struct::max7219_ShowTime(bool sep) {
  const uint8_t idx_list[] = { 7, 6, 4, 3, 1, 0 }; // Digits in reversed order, as the loop is backward

  for (int8_t i = 5; i >= 0; --i) {
    max7219_SetDigit(idx_list[i], showbuffer[i], false);
  }

  const uint8_t sepChar = P073_mapCharToFontPosition(sep ? '-' : ' ', fontset);

  max7219_SetDigit(2, sepChar, false);
  max7219_SetDigit(5, sepChar, false);
}

void P073_data_struct::max7219_ShowTemp(int8_t firstDot,
                                        int8_t secondDot) {
  max7219_SetDigit(0, 10, false);

  if (firstDot  > -1) { showperiods[firstDot] = true; }

  if (secondDot > -1) { showperiods[secondDot] = true; }

  const int alignRight = rightAlignTempMAX7219 ? 0 : 1;

  for (int i = alignRight; i < 8; ++i) {
    const int bufIndex = (7 + alignRight) - i;

    if (bufIndex < 8) {
      max7219_SetDigit(i,
                       showbuffer[bufIndex],
                       showperiods[bufIndex]);
    }
  }
}

void P073_data_struct::max7219_ShowDate() {
  const uint8_t dotflags[8] = { false, true, false, true, false, false, false, false };

  for (int i = 0; i < 8; ++i) {
    max7219_SetDigit(i,
                     showbuffer[7 - i],
                     dotflags[7 - i]);
  }
}

void P073_data_struct::max7219_ShowBuffer() {
  if (dotpos > -1) {
    showperiods[dotpos] = true;
  }

  for (int i = 0; i < 8; i++) {
    max7219_SetDigit(i,
                     showbuffer[7 - i],
                     showperiods[7 - i]
                     # if P073_7DBIN_COMMAND
                     , binaryData
                     # endif // if P073_7DBIN_COMMAND
                     );
  }
}

#endif    // ifdef USES_P073
