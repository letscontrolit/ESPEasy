#include "../PluginStructs/P073_data_struct.h"

#ifdef USES_P073

void P073_data_struct::init(struct EventStruct *event)
{
  ClearBuffer();
  pin1         = CONFIG_PIN1;
  pin2         = CONFIG_PIN2;
  pin3         = CONFIG_PIN3;
  displayModel = PCONFIG(0);
  output       = PCONFIG(1);
  brightness   = PCONFIG(2);
  periods      = bitRead(PCONFIG_LONG(0), P073_OPTION_PERIOD);
  hideDegree   = bitRead(PCONFIG_LONG(0), P073_OPTION_HIDEDEGREE);
  # ifdef P073_SCROLL_TEXT
  txtScrolling = bitRead(PCONFIG_LONG(0), P073_OPTION_SCROLLTEXT);
  scrollFull   = bitRead(PCONFIG_LONG(0), P073_OPTION_SCROLLFULL);
  setScrollSpeed(PCONFIG(3));
  # endif // P073_SCROLL_TEXT
  rightAlignTempMAX7219 = bitRead(PCONFIG_LONG(0), P073_OPTION_RIGHTALIGN);
  timesep               = true;
  # ifdef P073_EXTRA_FONTS
  fontset = PCONFIG(4);
  # endif // P073_EXTRA_FONTS
}

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
  showbuffer[0] = static_cast<uint8_t>(sevendgt_hours / 10);
  showbuffer[1] = sevendgt_hours % 10;
  showbuffer[2] = static_cast<uint8_t>(sevendgt_minutes / 10);
  showbuffer[3] = sevendgt_minutes % 10;
  showbuffer[4] = static_cast<uint8_t>(sevendgt_seconds / 10);
  showbuffer[5] = sevendgt_seconds % 10;
  # ifdef P073_SUPPRESS_ZERO

  if (suppressLeading0 && (showbuffer[0] == 0)) { showbuffer[0] = 10; } // set to space
  # endif // ifdef P073_SUPPRESS_ZERO
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

  showbuffer[0] = static_cast<uint8_t>(sevendgt_day / 10);
  showbuffer[1] = sevendgt_day % 10;
  showbuffer[2] = static_cast<uint8_t>(sevendgt_month / 10);
  showbuffer[3] = sevendgt_month % 10;
  showbuffer[4] = static_cast<uint8_t>(sevendgt_year1 / 10);
  showbuffer[5] = sevendgt_year1 % 10;
  showbuffer[6] = static_cast<uint8_t>(sevendgt_year2 / 10);
  showbuffer[7] = sevendgt_year2 % 10;
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

void P073_data_struct::FillBufferWithTemp(long temperature) {
  ClearBuffer();
  char p073_digit[8];
  bool between10and0 = ((temperature < 10) && (temperature >= 0));      // To have a zero prefix (0.x and -0.x) display between 0.9 and
                                                                        // -0.9
  bool between0andMinus10 = ((temperature < 0) && (temperature > -10)); // degrees,as all display types use 1 digit for temperatures
                                                                        // between 10.0 and -10.0
  String format;

  if (hideDegree) {
    format = (between10and0 ? F("      %02d") : (between0andMinus10 ? F("     %03d") : F("%8d")));
  } else {
    format = (between10and0 ? F("     %02d") : (between0andMinus10 ? F("    %03d") : F("%7d")));
  }
  sprintf_P(p073_digit, format.c_str(), static_cast<int>(temperature));
  int p073_numlenght = strlen(p073_digit);

  for (int i = 0; i < p073_numlenght; i++) {
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
void P073_data_struct::FillBufferWithDualTemp(long leftTemperature,
                                              bool leftWithDecimal,
                                              long rightTemperature,
                                              bool rightWithDecimal) {
  ClearBuffer();
  char   p073_digit[8];
  String format;
  bool   leftBetween10and0 = (leftWithDecimal && (leftTemperature < 10) && (leftTemperature >= 0));

  // To have a zero prefix (0.x and -0.x) display between 0.9 and -0.9 degrees,
  // as all display types use 1 digit for temperatures between 10.0 and -10.0
  bool leftBetween0andMinus10 = (leftWithDecimal && (leftTemperature < 0) && (leftTemperature > -10));

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
  bool rightBetween0andMinus10 = (rightWithDecimal && (rightTemperature < 0) && (rightTemperature > -10));

  if (hideDegree) {
    format += (rightBetween10and0 ? F("  %02d") : (rightBetween0andMinus10 ? F(" %03d") : rightTemperature < -1000 ? F("----") : F("%4d")));
  } else {
    format += (rightBetween10and0 ? F(" %02d") : (rightBetween0andMinus10 ? F("%03d") : rightTemperature < -100 ? F("----") : F("%3d")));
  }
  sprintf_P(p073_digit, format.c_str(), static_cast<int>(leftTemperature), static_cast<int>(rightTemperature));
  const int p073_numlenght = strlen(p073_digit);

  for (int i = 0; i < p073_numlenght; i++) {
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

  // addLog(LOG_LEVEL_INFO, String(F("7dgt format")) + format);
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

  for (int i = 0; i < p073_txtlength && p <= 8; i++) { // p <= 8 to allow a period after last digit
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
uint8_t P073_data_struct::getBufferLength(uint8_t displayModel) {
  uint8_t bufLen = 0;

  switch (displayModel) {
    case P073_TM1637_4DGTCOLON:
    case P073_TM1637_4DGTDOTS:
      bufLen = 4;
      break;
    case P073_TM1637_6DGT:
      bufLen = 6;
      break;
    case P073_MAX7219_8DGT:
      bufLen = 8;
      break;
  }
  return bufLen;
}

int P073_data_struct::getEffectiveTextLength(const String& text) {
  const int textLength = text.length();
  int p                = 0;

  for (int i = 0; i < textLength; i++) {
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
      const int bufToFill      = getBufferLength(displayModel);
      const int p073_txtlength = _textToScroll.length();
      ClearBuffer();

      int p = 0;

      for (int i = scrollPos; i < p073_txtlength && p <= bufToFill; i++) { // p <= bufToFill to allow a period after last digit
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
    const int bufToFill = getBufferLength(displayModel);
    _textToScroll.reserve(text.length() + bufToFill + (scrollFull ? bufToFill : 0));

    for (int i = 0; scrollFull && i < bufToFill; i++) { // Scroll text in from the right, so start with all spaces
      _textToScroll +=
        #  ifdef P073_7DBIN_COMMAND
        binaryData ? (char)0x00 :
        #  endif // P073_7DBIN_COMMAND
        ' ';
    }
    _textToScroll += text;

    for (int i = 0; i < bufToFill; i++) { // Scroll text off completely before restarting
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
    log  = prefix;
    log += F(" buffer: periods: ");
    log += periods ? 't' : 'f';
    log += ' ';

    for (uint8_t i = 0; i < 8; i++) {
      if (i > 0) { log += ','; }
      log += F("0x");
      log += String(showbuffer[i], HEX);
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

  for (uint8_t i = 0; i < 8; i++) {
    showperiods[i] = false;
  }
}

uint8_t P073_data_struct::mapCharToFontPosition(char    character,
                                                uint8_t fontset) {
  uint8_t position = 10;

  # ifdef P073_EXTRA_FONTS
  String specialChars = F(" -^=/_%@.,;:+*#!?'\"<>\\()|");
  String chnorux      = F("CHNORUX");

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
        int idx = specialChars.indexOf(character);

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

uint8_t P073_data_struct::mapMAX7219FontToTM1673Font(uint8_t character) {
  uint8_t newCharacter = character & 0x80; // Keep dot-bit if passed in

  for (int b = 0; b < 7; b++) {
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
    case 1:                                                                        // Siekoo
    case 2:                                                                        // Siekoo uppercase CHNORUX
      return mapMAX7219FontToTM1673Font(pgm_read_byte(&(SiekooCharTable[index]))); // SiekooTableTM1637[index];
    case 3:                                                                        // dSEG7
      return mapMAX7219FontToTM1673Font(pgm_read_byte(&(Dseg7CharTable[index])));  // Dseg7TableTM1637[index];
    default:                                                                       // Standard fontset
  # endif // P073_EXTRA_FONTS
  return mapMAX7219FontToTM1673Font(pgm_read_byte(&(DefaultCharTable[index])));    // CharTableTM1637[index];
  # ifdef P073_EXTRA_FONTS
} // Out of wack because of the conditional compilation ifdef's

  # endif // P073_EXTRA_FONTS
}

#endif    // ifdef USES_P073
