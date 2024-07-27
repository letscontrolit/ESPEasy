#include "../PluginStructs/P073_data_struct.h"

#ifdef USES_P073

uint8_t p073_getDefaultDigits(uint8_t displayModel,
                              uint8_t digits) {
  const uint8_t digitsSet[] = { 4, 4, 6, 8, 0 }; // Fixed except 74HC595
  uint8_t bufLen{};

  if (displayModel < NR_ELEMENTS(digitsSet)) {
    bufLen = digitsSet[displayModel];
  }
  # ifdef P073_USE_74HC595

  if (P073_74HC595_2_8DGT == displayModel) {
    bufLen = digits;
  }
  # endif // ifdef P073_USE_74HC595

  return bufLen;
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
  # ifdef P073_SCROLL_TEXT
  txtScrolling = bitRead(P073_CFG_FLAGS, P073_OPTION_SCROLLTEXT);
  scrollFull   = bitRead(P073_CFG_FLAGS, P073_OPTION_SCROLLFULL);
  setScrollSpeed(P073_CFG_SCROLLSPEED);
  # endif // P073_SCROLL_TEXT
  rightAlignTempMAX7219 = bitRead(P073_CFG_FLAGS, P073_OPTION_RIGHTALIGN);
  timesep               = true;
  # ifdef P073_EXTRA_FONTS
  fontset = P073_CFG_FONTSET;
  # endif // P073_EXTRA_FONTS
  digits = P073_CFG_DIGITS;
  # ifdef P073_USE_74HC595

  if ((digits > 0) && ((digits < 4) || (5 == digits) || (7 == digits))) {
    isSequential = true;

    if (1 == digits) { // 2+2
      digits = 4;
    } else
    if (7 == digits) { // 3+3
      digits = 6;
    }
  }
  # endif // ifdef P073_USE_74HC595

  if ((digits > 0) && (digits < 4)) {
    hideDegree = true; // Hide degree symbol on small displays
  }
}

# ifdef P073_USE_74HC595
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

// ====================================
// ---- 74HC595 specific functions ----
// ====================================

void P073_data_struct::hc595_ShowBuffer() {
  const uint8_t hc595digit4[] = {
    0b00001000, // left segment
    0b00000100,
    0b00000010,
    0b00000001, // right segment
  };
  const uint8_t hc595digit6[] = {
    0b00010000, // left segment
    0b00100000,
    0b01000000,
    0b00000001,
    0b00000010,
    0b00000100, // right segment
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

  int8_t i    = 0;
  int8_t stop = digits;
  int8_t incr = 1;
  int8_t trgr = digits - 1;

  if (P073_HC595_SEQUENTIAL) {
    i    = digits - 1;
    stop = -1;
    incr = -1;
    trgr = 0;
  }

  #  ifdef P073_DEBUG
  const int8_t oi = i;
  #  endif // ifdef P073_DEBUG

  for (; i != stop && i >= 0; i += incr) {
    uint8_t value;
    uint8_t digit = 0xFF;
    #  ifdef P073_EXTRA_FONTS

    // 74HC595 uses inverted data, compared to MAX7219/TM1637
    switch (fontset) {
      case 1:  // Siekoo
      case 2:  // Siekoo with uppercase CHNORUX
        value = ~pgm_read_byte(&(SiekooCharTable[showbuffer[i]]));
        break;
      case 3:  // dSEG7
        value = ~pgm_read_byte(&(Dseg7CharTable[showbuffer[i]]));
        break;
      default: // Default fontset
        value = ~pgm_read_byte(&(DefaultCharTable[showbuffer[i]]));
    }
    #  else // ifdef P073_EXTRA_FONTS
    value = ~pgm_read_byte(&(DefaultCharTable[showbuffer[i]]));
    #  endif // P073_EXTRA_FONTS

    if (showperiods[i]) {
      value &= 0x7F;
    }
    value = mapMAX7219FontToTM1673Font(value); // Revert bits 6..0

    shiftOut(pin1, pin2, MSBFIRST, value);     // Digit data out

    // 2 and 3 digit modules use sequential digit values (in reversed order)
    // 4, 6 and 8 digit modules use multiplexing in LTR order
    if (4 == digits) {
      digit = hc595digit4[i];
    } else
    if ((6 == digits) && !isSequential) {
      digit = hc595digit6[i];
    } else
    if (8 == digits) {
      digit = hc595digit8[i];
    }

    if (digit != 0xFF) { // Select multiplexer digit, 0xFF is invalid
      shiftOut(pin1, pin2, MSBFIRST, digit);
    }

    if ((P073_HC595_SEQUENTIAL && (i == trgr)) || P073_HC595_MULTIPLEX) {
      digitalWrite(pin3, LOW); // Clock data
      delay(1);
      digitalWrite(pin3, HIGH);
    }
  }

  #  ifdef P073_DEBUG

  // if ((counter50 % 100 == 0) || P073_HC595_SEQUENTIAL) {
  //   addLog(LOG_LEVEL_INFO, strformat(F("P073: hc595_ShowBuffer (end) dgt:%d oi:%d i:%d stop:%d incr:%d pin1: %d pin2: %d pin3: %d"),
  //                                    digits, oi, i, stop, incr, pin1, pin2, pin3));
  // }
  #  endif // ifdef P073_DEBUG
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

# endif // ifdef P073_USE_74HC595

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
                      # ifdef P073_SUPPRESS_ZERO
                      , suppressLeading0
                      # endif // ifdef P073_SUPPRESS_ZERO
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
                      # ifdef P073_SUPPRESS_ZERO
                      , suppressLeading0
                      # endif // ifdef P073_SUPPRESS_ZERO
                      );
}

void P073_data_struct::Put4NumbersInBuffer(const uint8_t nr1,
                                           const uint8_t nr2,
                                           const uint8_t nr3,
                                           const int8_t  nr4
                                           # ifdef       P073_SUPPRESS_ZERO
                                           , const bool  suppressLeading0
                                           # endif // ifdef P073_SUPPRESS_ZERO
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
  # ifdef P073_SUPPRESS_ZERO

  if (suppressLeading0 && (showbuffer[0] == 0)) { showbuffer[0] = 10; } // set to space
  # endif // ifdef P073_SUPPRESS_ZERO
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
      showbuffer[p073_index] = mapCharToFontPosition(p073_tmpchar, fontset);
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
    showbuffer[i] = mapCharToFontPosition(p073_digit[i], fontset);
  }

  if (!hideDegree) {
    showbuffer[7] = 12; // degree "°"
  }
}

# ifdef P073_7DDT_COMMAND

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
    showbuffer[i] = mapCharToFontPosition(p073_digit[i], fontset);
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

# endif // ifdef P073_7DDT_COMMAND

void P073_data_struct::FillBufferWithString(const String& textToShow,
                                            bool          useBinaryData) {
  # ifdef P073_7DBIN_COMMAND
  binaryData = useBinaryData;
  # endif // P073_7DBIN_COMMAND
  ClearBuffer();
  const int p073_txtlength = textToShow.length();

  int p = 0;

  for (int i = 0; i < p073_txtlength && p <= 8; ++i) { // p <= 8 to allow a period after last digit
    if (periods
        && textToShow.charAt(i) == '.'
        # ifdef P073_7DBIN_COMMAND
        && !binaryData
        # endif // P073_7DBIN_COMMAND
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
      # ifdef P073_7DBIN_COMMAND
      showbuffer[p] = useBinaryData ? textToShow.charAt(i) : mapCharToFontPosition(textToShow.charAt(i), fontset);
      # else // P073_7DBIN_COMMAND
      showbuffer[p] = mapCharToFontPosition(textToShow.charAt(i), fontset);
      # endif // P073_7DBIN_COMMAND
      p++;
    }
  }
  # ifdef P073_DEBUG
  LogBufferContent(F("7dtext"));
  # endif // ifdef P073_DEBUG
}

# ifdef P073_SCROLL_TEXT
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
      const int bufToFill      = p073_getDefaultDigits(displayModel, digits);
      const int p073_txtlength = _textToScroll.length();
      ClearBuffer();

      int p = 0;

      for (int i = scrollPos; i < p073_txtlength && p <= bufToFill; ++i) { // p <= bufToFill to allow a period after last digit
        if (periods
            && _textToScroll.charAt(i) == '.'
            #  ifdef P073_7DBIN_COMMAND
            && !binaryData
            #  endif // P073_7DBIN_COMMAND
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
          #  ifdef P073_7DBIN_COMMAND
          showbuffer[p] = binaryData ?
                          _textToScroll.charAt(i) :
                          mapCharToFontPosition(_textToScroll.charAt(i), fontset);
          #  else // P073_7DBIN_COMMAND
          showbuffer[p] = mapCharToFontPosition(_textToScroll.charAt(i), fontset);
          #  endif // P073_7DBIN_COMMAND
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
      #  endif // P073_DEBUG
    }
  }
  return result;
}

void P073_data_struct::setTextToScroll(const String& text) {
  _textToScroll = String();

  if (!text.isEmpty()) {
    const int bufToFill = p073_getDefaultDigits(displayModel, digits);
    _textToScroll.reserve(text.length() + bufToFill + (scrollFull ? bufToFill : 0));

    for (int i = 0; scrollFull && i < bufToFill; ++i) { // Scroll text in from the right, so start with all spaces
      _textToScroll +=
        #  ifdef P073_7DBIN_COMMAND
        binaryData ? (char)0x00 :
        #  endif // P073_7DBIN_COMMAND
        ' ';
    }
    _textToScroll += text;

    for (int i = 0; i < bufToFill; ++i) { // Scroll text off completely before restarting
      _textToScroll +=
        #  ifdef P073_7DBIN_COMMAND
        binaryData ? (char)0x00 :
        #  endif // P073_7DBIN_COMMAND
        ' ';
    }
  }
  scrollCount = _scrollSpeed;
  scrollPos   = 0;
  #  ifdef P073_7DBIN_COMMAND
  binaryData = false;
  #  endif // P073_7DBIN_COMMAND
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

# endif // P073_SCROLL_TEXT

# ifdef P073_7DBIN_COMMAND
void P073_data_struct::setBinaryData(const String& data) {
  binaryData = true;
  #  ifdef P073_SCROLL_TEXT
  setTextToScroll(data);
  binaryData  = true; // is reset in setTextToScroll
  scrollCount = _scrollSpeed;
  scrollPos   = 0;
  #  else // P073_SCROLL_TEXT
  _textToScroll = data;
  #  endif // P073_SCROLL_TEXT
}

# endif      // P073_7DBIN_COMMAND

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

# endif // P073_DEBUG

// in case of error show all dashes
void P073_data_struct::FillBufferWithDash() {
  memset(showbuffer, 11, sizeof(showbuffer));
}

void P073_data_struct::ClearBuffer() {
  memset(showbuffer,
         # ifdef P073_7DBIN_COMMAND
         binaryData ? 0 :
         # endif // P073_7DBIN_COMMAND
         10, sizeof(showbuffer));

  for (uint8_t i = 0; i < 8; ++i) {
    showperiods[i] = false;
  }
}

uint8_t P073_data_struct::mapCharToFontPosition(char    character,
                                                uint8_t fontset) {
  uint8_t position = 10;

  # ifdef P073_EXTRA_FONTS
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
  # endif // P073_EXTRA_FONTS

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
  # ifdef P073_EXTRA_FONTS
}

  # endif // P073_EXTRA_FONTS
  return position;
}

/**
 * This function reverts the 7 databits/segmentbits so TM1637 and 74HC595 displays work with fonts designed for MAX7219.
 * Dot/colon bit is still bit 8
 */
uint8_t P073_data_struct::mapMAX7219FontToTM1673Font(uint8_t character) {
  uint8_t newCharacter = character & 0x80; // Keep dot-bit if passed in

  for (int b = 0; b < 7; ++b) {
    if (character & (0x01 << b)) {
      newCharacter |= (0x40 >> b);
    }
  }
  return newCharacter;
}

uint8_t P073_data_struct::tm1637_getFontChar(uint8_t index,
                                             uint8_t fontset) {
  # ifdef P073_EXTRA_FONTS

  switch (fontset) {
    case 1:  // Siekoo
    case 2:  // Siekoo uppercase CHNORUX
      return mapMAX7219FontToTM1673Font(pgm_read_byte(&(SiekooCharTable[index])));
    case 3:  // dSEG7
      return mapMAX7219FontToTM1673Font(pgm_read_byte(&(Dseg7CharTable[index])));
    default: // Standard fontset
      return mapMAX7219FontToTM1673Font(pgm_read_byte(&(DefaultCharTable[index])));
  }
  # else // ifdef P073_EXTRA_FONTS
  return mapMAX7219FontToTM1673Font(pgm_read_byte(&(DefaultCharTable[index])));
  # endif // ifdef P073_EXTRA_FONTS
}

#endif    // ifdef USES_P073
