#include "../PluginStructs/P157_data_struct.h"

#ifdef USES_P157

uint8_t P157_getDefaultDigits(uint8_t displayModel,
                              uint8_t displays) {
  const uint8_t digitsSet[] = { 4, 8, 4, 8 }; // Fixed
  uint8_t bufLen{};

  if (displayModel < NR_ELEMENTS(digitsSet)) {
    bufLen = digitsSet[displayModel];
  }

  return bufLen * displays;
}

const __FlashStringHelper* P157_DisplayModel(uint8_t model) {
  switch (model)
  {
    case P157_MODEL_4DGT: return F("HT16K33 - 4 digit");
    case P157_MODEL_8DGT: return F("HT16K33 - 8 digit");
    case P157_MODEL_4DGT_7SEG: return F("HT16K33 - 4 digit (7-segment)");
    case P157_MODEL_8DGT_7SEG: return F("HT16K33 - 8 digit (7-segment)");
  }
  return F("");
}

void P157_display_output_selector(const __FlashStringHelper *id, int16_t value) {
  const __FlashStringHelper *displout[] = {
    F("Manual"),
    F("Clock 24h - Blink"),
    F("Clock 24h - No Blink"),
    F("Clock 12h - Blink"),
    F("Clock 12h - No Blink"),
    F("Date"),
  };
  const int disploutOptions[] = {
    P157_DISP_MANUAL,
    P157_DISP_CLOCK24BLNK,
    P157_DISP_CLOCK24,
    P157_DISP_CLOCK12BLNK,
    P157_DISP_CLOCK12,
    P157_DISP_DATE,
  };
  const FormSelectorOptions selector(NR_ELEMENTS(disploutOptions), displout, disploutOptions);

  selector.addFormSelector(F("Display Output"), id, value);
}

bool P157_is7SegmentDisplay(uint8_t model) { return (P157_MODEL_4DGT_7SEG == model) || (P157_MODEL_8DGT_7SEG == model); }

bool P157_is4DigitDisplay(uint8_t model)   { return (P157_MODEL_4DGT == model) || (P157_MODEL_4DGT_7SEG == model); }

P157_data_struct::~P157_data_struct() {
  if (nullptr != ht16k33) {
    ht16k33->off();
    delete ht16k33;
    ht16k33 = nullptr;
  }
}

bool P157_data_struct::init(struct EventStruct *event)
{
  clearBuffer();
  i2cAddress   = P157_CFG_I2C_ADDRESS;
  displayModel = P157_CFG_DISPLAYTYPE;
  output       = P157_CFG_OUTPUTTYPE;
  brightness   = P157_CFG_BRIGHTNESS;
  periods      = true; // bitRead(P157_CFG_FLAGS, P157_OPTION_PERIOD);
  hideDegree   = bitRead(P157_CFG_FLAGS, P157_OPTION_HIDEDEGREE);
  # if P157_SCROLL_TEXT
  txtScrolling = bitRead(P157_CFG_FLAGS, P157_OPTION_SCROLLTEXT);
  scrollFull   = bitRead(P157_CFG_FLAGS, P157_OPTION_SCROLLFULL);
  setScrollSpeed(P157_CFG_SCROLLSPEED);
  # endif // if P157_SCROLL_TEXT
  rightAlignTempMAX7219 = bitRead(P157_CFG_FLAGS, P157_OPTION_RIGHTALIGN);
  suppressLeading0      = bitRead(P157_CFG_FLAGS, P157_OPTION_SUPPRESS0);
  timesep               = true;

  // # if P157_EXTRA_FONTS
  // fontset = P157_CFG_FONTSET;
  // # endif // if P157_EXTRA_FONTS
  displays = P157_CFG_DISPLAYS;

  if (0 == displays) {
    displays = 1;
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, strformat(F("P157 : Model: %s, displays: %d"), FsP(P157_DisplayModel(displayModel)), displays));
  }

  if (!P157_is7SegmentDisplay(displayModel)) {
    ht16k33 = new (std::nothrow) Noiasca_ht16k33_hw_14(); // 14 segment
  } else {
    ht16k33 = new (std::nothrow) Noiasca_ht16k33_hw_7();  // 7 segment
  }

  if (nullptr == ht16k33) {
    return false;
  }

  if (0 == ht16k33->begin(i2cAddress, displays)) {
    if (ht16k33->isConnected()) // Happy flow
    {
      ht16k33->setDigits(P157_is4DigitDisplay(displayModel) ? 4 : 8);
      ht16k33->setBrightness(brightness);
      ht16k33->on();
      return true;
    }
  }
  return false;
}

void P157_data_struct::fillBufferWithTime(bool    sevendgt_now,
                                          uint8_t sevendgt_hours,
                                          uint8_t sevendgt_minutes,
                                          uint8_t sevendgt_seconds,
                                          bool    flag12h,
                                          bool    suppressLeading0) {
  clearBuffer();
  const int bufToFill = P157_getDefaultDigits(displayModel, displays);

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
  put4NumbersInBuffer(sevendgt_hours, sevendgt_minutes, bufToFill > 4 ? sevendgt_seconds : -1, -1
                      # if P157_SUPPRESS_ZERO
                      , suppressLeading0
                      # endif // if P157_SUPPRESS_ZERO
                      , timesep
                      );
}

void P157_data_struct::fillBufferWithDate(bool    sevendgt_now,
                                          uint8_t sevendgt_day,
                                          uint8_t sevendgt_month,
                                          int     sevendgt_year,
                                          bool    suppressLeading0) {
  clearBuffer();
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

  put4NumbersInBuffer(sevendgt_day, sevendgt_month, sevendgt_year1, sevendgt_year2
                      # if P157_SUPPRESS_ZERO
                      , suppressLeading0
                      # endif // if P157_SUPPRESS_ZERO
                      , false
                      );
}

void P157_data_struct::put4NumbersInBuffer(const uint8_t nr1,
                                           const uint8_t nr2,
                                           const int8_t  nr3,
                                           const int8_t  nr4
                                           # if          P157_SUPPRESS_ZERO
                                           , const bool  suppressLeading0
                                           # endif // if P157_SUPPRESS_ZERO
                                           , const bool  sep
                                           ) {
  uint8_t idx = 0;

  showbuffer[idx++] = 48 + static_cast<uint8_t>(nr1 / 10);

  # if P157_SUPPRESS_ZERO

  if (suppressLeading0 && (showbuffer[idx - 1] == '0')) { showbuffer[idx - 1] = ' '; } // set to space
  # endif // if P157_SUPPRESS_ZERO

  showbuffer[idx++] = 48 + (nr1 % 10);

  if (sep) { showbuffer[idx++] = ':'; }

  showbuffer[idx++] = 48 + static_cast<uint8_t>(nr2 / 10);
  showbuffer[idx++] = 48 + (nr2 % 10);

  if (nr3 > -1) {
    if (sep) { showbuffer[idx++] = ':'; }

    showbuffer[idx++] = 48 + static_cast<uint8_t>(nr3 / 10);
    showbuffer[idx++] = 48 + (nr3 % 10);

    if (nr4 > -1) {
      showbuffer[idx++] = 48 + static_cast<uint8_t>(nr4 / 10);
      showbuffer[idx++] = 48 + (nr4 % 10);
    }
  }
}

void P157_data_struct::fillBufferWithNumber(const String& number) {
  clearBuffer();

  if (number.length() == 0) {
    return;
  }
  int8_t P157_index = 7;

  dotpos = -1; // -1 means no dot to display

  for (int i = number.length() - 1; i >= 0 && P157_index >= 0; --i) {
    const char P157_tmpchar = number.charAt(i);

    if (P157_tmpchar == '.') { // dot
      dotpos = P157_index;
    } else {
      showbuffer[P157_index] = P157_tmpchar;
      P157_index--;
    }
  }

  while (P157_index >= 0) { // Prefix with spaces
    showbuffer[P157_index] = ' ';
    P157_index--;
  }
}

void P157_data_struct::fillBufferWithTemp(int temperature) {
  clearBuffer();
  char P157_digit[8];
  const bool between10and0      = ((temperature < 10) && (temperature >= 0)); // To have a zero prefix (0.x and -0.x) display between 0.9
  const bool between0andMinus10 = ((temperature < 0) && (temperature > -10)); // and -0.9 degrees,as all display types use 1 digit for
                                                                              // temperatures between 10.0 and -10.0
  String format;

  if (hideDegree) {
    format = (between10and0 ? F("      %02d") : (between0andMinus10 ? F("     %03d") : F("%8d")));
  } else {
    format = (between10and0 ? F("     %02d") : (between0andMinus10 ? F("    %03d") : F("%7d")));
  }
  sprintf_P(P157_digit, format.c_str(), temperature);
  const size_t P157_numlenght = strlen(P157_digit);

  for (size_t i = 0; i < P157_numlenght; ++i) {
    showbuffer[i] = P157_digit[i];
  }

  if (P157_numlenght > 2) {
    showperiods[P157_numlenght - 2] = true;
  }

  if (!hideDegree) {
    showbuffer[7] = P157_CHAR_DEGREE; // degree "°"
  }
}

# if P157_7DDT_COMMAND

/**
 * fillBufferWithDualTemp()
 * leftTemperature or rightTempareature < -100.0 then shows dashes
 */
void P157_data_struct::fillBufferWithDualTemp(int  leftTemperature,
                                              bool leftWithDecimal,
                                              int  rightTemperature,
                                              bool rightWithDecimal) {
  clearBuffer();
  char   P157_digit[8];
  String format;
  const bool leftBetween10and0 = (leftWithDecimal && (leftTemperature < 10) && (leftTemperature >= 0));

  // To have a zero prefix (0.x and -0.x) display between 0.9 and -0.9 degrees,
  // as all display types use 1 digit for temperatures between 10.0 and -10.0
  const bool leftBetween0andMinus10 = (leftWithDecimal && (leftTemperature < 0) && (leftTemperature > -10));

  if (hideDegree) {
    // Include a space for compensation of the degree symbol
    format =
      (leftBetween10and0 ? F("  %02d") : (leftBetween0andMinus10 ? F(" %03d") : leftTemperature < -1000 ? F("----") : F("%4d")));
    showperiods[2] = leftTemperature >= -1000;
  } else {
    // Include a space for compensation of the degree symbol
    format =
      (leftBetween10and0 ? F(" %02d ") : (leftBetween0andMinus10 ? F("%03d ") : leftTemperature < -100 ? F("----") : F("%3d ")));
    showperiods[2] = leftTemperature >= -100;
  }
  bool rightBetween10and0 = (rightWithDecimal && (rightTemperature < 10) && (rightTemperature >= 0));

  // To have a zero prefix (0.x and -0.x) display between 0.9 and -0.9 degrees,
  // as all display types use 1 digit for temperatures between 10.0 and -10.0
  const bool rightBetween0andMinus10 = (rightWithDecimal && (rightTemperature < 0) && (rightTemperature > -10));

  if (hideDegree) {
    format +=
      (rightBetween10and0 ? F("  %02d") : (rightBetween0andMinus10 ? F(" %03d") : rightTemperature < -1000 ? F("----") : F("%4d")));
    showperiods[6] = rightTemperature >= -1000;
  } else {
    format +=
      (rightBetween10and0 ? F(" %02d") : (rightBetween0andMinus10 ? F("%03d") : rightTemperature < -100 ? F("----") : F("%3d")));
    showperiods[6] = rightTemperature >= -100;
  }
  sprintf_P(P157_digit, format.c_str(), leftTemperature, rightTemperature);
  const size_t P157_numlenght = strlen(P157_digit);

  for (size_t i = 0; i < P157_numlenght; ++i) {
    showbuffer[i] = P157_digit[i];
  }

  if (!hideDegree) {
    if (leftTemperature  > -100.0) {
      showbuffer[3] = P157_CHAR_DEGREE; // degree "°"
    }

    if (rightTemperature > -100.0) {
      showbuffer[7] = P157_CHAR_DEGREE; // degree "°"
    }
  }

  // addLog(LOG_LEVEL_INFO, concat(F("7dgt format: "), format));
}

# endif // if P157_7DDT_COMMAND

void P157_data_struct::fillBufferWithString(const String& textToShow,
                                            bool          useBinaryData) {
  clearBuffer();
  const size_t bufToFill = P157_getDefaultDigits(displayModel, displays);

  String buf(textToShow);

  while (getEffectiveTextLength(buf) > bufToFill && !buf.isEmpty()) {
    buf = buf.substring(0, buf.length() - 1);
  }

  while (buf.length() < bufToFill) {
    buf += ' ';
  }
  memcpy(showbuffer, buf.c_str(), bufToFill);

  # ifdef P157_DEBUG
  logBufferContent(F("7dtext"));
  # endif // ifdef P157_DEBUG
}

# if P157_SCROLL_TEXT || P157_7DBIN_COMMAND

bool P157_data_struct::isPeriodChar(const char thisChar) {
  return std::find(std::begin(periodchars), std::end(periodchars), thisChar) != std::end(periodchars);
}

int P157_data_struct::getEffectiveTextLength(const String& text) {
  const int textLength = text.length();
  int p                = 0;

  for (int i = 0; i < textLength; ++i) {

    if (periods && isPeriodChar(text.charAt(i))) { // If setting periods true
      if (p == 0) {                                // Text starts with a period, becomes a space with a dot
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

# endif // if P157_SCROLL_TEXT || P157_7DBIN_COMMAND

# if P157_SCROLL_TEXT

bool P157_data_struct::nextScroll() {
  bool result = false;

  if (isScrollEnabled() && (!_textToScroll.isEmpty()
                            #  if P157_7DBIN_COMMAND
                            || binData.size() > 0
                            #  endif // if P157_7DBIN_COMMAND
                            )) {

    if ((scrollCount > 0) && (scrollCount < 0xFFFF)) { scrollCount--; }

    if (scrollCount == 0) {
      scrollCount = 0xFFFF; // Max value to avoid interference when scrolling long texts
      result      = true;
      const int bufToFill = P157_getDefaultDigits(displayModel, displays);
      #  if P157_7DBIN_COMMAND

      if (binData.size() > 0) {
        scrollPos++;

        if (scrollPos > (binData.size() - bufToFill)) {
          scrollPos = 0;            // Redisplay
        }
        scrollCount = _scrollSpeed; // Restart countdown
      } else
      #  endif // if P157_7DBIN_COMMAND
      { const int P157_txtlength = _textToScroll.length();
        clearBuffer();

        if (isPeriodChar(_textToScroll.charAt(scrollPos))) {
          scrollPos++;
        }

        String part = _textToScroll.substring(scrollPos, scrollPos + 1.5 * bufToFill);

        while (getEffectiveTextLength(part) > bufToFill && !part.isEmpty()) {
          part = part.substring(0, part.length() - 1);
        }

        for (uint16_t i = 0; i < bufToFill && i < part.length(); ++i) {
          showbuffer[i] = part.charAt(i);
        }

        // for (uint16_t i = scrollPos; i < P157_txtlength && p <= bufToFill; ++i) { // p <= bufToFill to allow a period after last digit
        //   const char *isPeriod = std::find(std::begin(periodchars), std::end(periodchars), _textToScroll.charAt(i));

        //   if (periods && (isPeriod != std::end(periodchars))) {                   // If setting periods true
        //     if (p == 0) {                                                         // Text starts with a period, becomes a space with a
        // dot
        //       showperiods[p] = true;
        //       p++;
        //     } else {
        //       showperiods[p - 1] = true;                                  // The period displays as a dot on the previous digit!
        //     }

        //     if ((i > scrollPos) && (isPeriod != std::end(periodchars))) { // Handle consecutive periods
        //       showperiods[p - 1] = true;                                  // The period displays as a dot on the previous digit!
        //       p++;
        //     }
        //   } else if (p < bufToFill) {
        //     showbuffer[p] = _textToScroll.charAt(i);
        //     p++;
        //   }
        // }
        scrollPos++;

        if (scrollPos > _textToScroll.length() - bufToFill) {
          scrollPos = 0;            // Restart when all text displayed
        }
        scrollCount = _scrollSpeed; // Restart countdown
        #  ifdef P157_DEBUG
        logBufferContent(F("nextScroll"));
        #  endif // ifdef P157_DEBUG
      }
    }
  }
  return result;
}

void P157_data_struct::setTextToScroll(const String& text) {
  free_string(_textToScroll);

  if (!text.isEmpty()) {
    const int bufToFill = P157_getDefaultDigits(displayModel, displays);
    _textToScroll.reserve(text.length() + bufToFill + (scrollFull ? bufToFill : 0));

    for (int i = 0; scrollFull && i < bufToFill; ++i) { // Scroll text in from the right, so start with all spaces
      _textToScroll += ' ';
    }
    _textToScroll += text;

    for (int i = 0; i < bufToFill; ++i) { // Scroll text off completely before restarting
      _textToScroll += ' ';
    }
  }
  scrollCount = _scrollSpeed;
  scrollPos   = 0;
  #  if P157_7DBIN_COMMAND
  binaryData = false;
  #  endif // if P157_7DBIN_COMMAND
}

void P157_data_struct::setScrollSpeed(uint8_t speed) {
  _scrollSpeed = speed;
  scrollCount  = _scrollSpeed;
  scrollPos    = 0;
}

# endif // if P157_SCROLL_TEXT

// # if P157_7DBIN_COMMAND

// void P157_data_struct::setBinaryData(const String& data) {
//   binaryData = true;
//   #  if P157_SCROLL_TEXT
//   setTextToScroll(data);
//   binaryData  = true; // is reset in setTextToScroll
//   scrollCount = _scrollSpeed;
//   scrollPos   = 0;
//   #  else // if P157_SCROLL_TEXT
//   _textToScroll = data;
//   #  endif // if P157_SCROLL_TEXT
// }

// # endif // if P157_7DBIN_COMMAND

# ifdef P157_DEBUG

void P157_data_struct::logBufferContent(String prefix) {
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

# endif // ifdef P157_DEBUG

// in case of error show all dashes
void P157_data_struct::fillBufferWithDash() { memset(showbuffer, '-', sizeof(showbuffer)); }

void P157_data_struct::clearBuffer() {
  memset(showbuffer,  0, sizeof(showbuffer));
  memset(showperiods, 0, sizeof(showperiods));
}

void P157_data_struct::printBuffer() {
  if (isInitialized()) {
    const size_t maxLen = P157_getDefaultDigits(displayModel, displays);
    # if P157_7DBIN_COMMAND

    if (binData.size() > 0) {
      #  ifdef P157_DEBUG
      String log;
      #  endif // ifdef P157_DEBUG

      for (uint16_t j = 0; j < maxLen && (scrollPos + j) < binData.size(); ++j) {
        ht16k33->writeLowLevel(j, binData[scrollPos + j]);
        #  ifdef P157_DEBUG
        log += formatToHex(binData[scrollPos + j]);
        log += ',';
        #  endif // ifdef P157_DEBUG
      }
      #  ifdef P157_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLog(LOG_LEVEL_INFO, strformat(F("P157 : printBuffer len: %u maxLen: %u data: %s"),
                                         binData.size(), maxLen, log.c_str()));
      }
      #  endif // ifdef P157_DEBUG
    } else
    # endif // if P157_7DBIN_COMMAND
    {
      String  buf = String(showbuffer);
      uint8_t dotOff{};

      if ((maxLen < 8) && (buf.length() > maxLen) && buf.startsWith(F("    "))) {
        dotOff = 4;
        buf    = buf.substring(4);
      }

      while (getEffectiveTextLength(buf) > maxLen && !buf.isEmpty()) {
        buf = buf.substring(0, buf.length() - 1);
      }

      while (buf.length() < maxLen) { // TODO Account for offset?
        buf += ' ';
      }

      # ifdef P157_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLog(LOG_LEVEL_INFO, strformat(F("P157 : printBuffer len: %u buffer: %s, maxLen: %u raw: '%s'"),
                                         buf.length(), buf.c_str(), maxLen, showbuffer));
      }
      # endif // ifdef P157_DEBUG

      uint8_t i = 0;
      # if P157_EXTRA_FONTS
      uint8_t pos = ht16k33->getCursor();
      # endif

      while (i < buf.length()) {
        # if P157_EXTRA_FONTS

        if (P157_is7SegmentDisplay(displayModel)) {
          // Re-use the fonts from P073, but they use the MAX7219 layout, that has bits 0..6 reverted
          char toPrint = P073_revert7bits(P073_getFontChar(P073_mapCharToFontPosition(buf[i], fontSet), fontSet));

          if (showperiods[i + dotOff] || (dotpos == (i + dotOff))) {
            toPrint |= SEG_DP;
          }

          if (((i + 1) < buf.length()) && isPeriodChar(buf[i + 1])) {
            toPrint |= SEG_DP;
            ++i;
          }
          ht16k33->writeLowLevel(pos, toPrint);
          ++pos;
        } else
        # endif // if P157_EXTRA_FONTS
        {
          ht16k33->write(buf[i]);

          if (showperiods[i + dotOff] || (dotpos == (i + dotOff))) {
            ht16k33->write('.');

          }
        }
        ++i;
      }
    }
    ht16k33->setCursor(0);
  }

}

bool P157_data_struct::plugin_once_a_second(struct EventStruct *event) {
  if (output == P157_DISP_MANUAL) {
    return false;
  }

  if ((output == P157_DISP_CLOCK24BLNK) ||
      (output == P157_DISP_CLOCK12BLNK)) {
    timesep = !timesep;
  } else {
    timesep = true;
  }

  if (output == P157_DISP_DATE) {
    fillBufferWithDate(true, 0, 0, 0,
                       # if P157_SUPPRESS_ZERO
                       suppressLeading0
                       # else // if P157_SUPPRESS_ZERO
                       false
                       # endif // if P157_SUPPRESS_ZERO
                       );
  } else {
    fillBufferWithTime(true, 0, 0, 0, !((output == P157_DISP_CLOCK24BLNK) ||
                                        (output == P157_DISP_CLOCK24)),
                       # if P157_SUPPRESS_ZERO
                       suppressLeading0
                       # else // if P157_SUPPRESS_ZERO
                       false
                       # endif // if P157_SUPPRESS_ZERO
                       );
  }

  printBuffer();

  return true;
}

# if P157_SCROLL_TEXT

bool P157_data_struct::plugin_ten_per_second(struct EventStruct *event) {
  if ((output != P157_DISP_MANUAL) || !isScrollEnabled()) {
    return false;
  }

  if (nextScroll()) {
    printBuffer();
  }
  return true;
}

# endif // if P157_SCROLL_TEXT

const char P157_commands[] PROGMEM =
  "7dn|7dt|"
  # if P157_7DDT_COMMAND
  "7ddt|"
  # endif // if P157_7DDT_COMMAND
  "7dst|7dsd|7dtext|"
  # if P157_EXTRA_FONTS
  "7dfont|"
  # endif // if P157_EXTRA_FONTS
  # if P157_7DBIN_COMMAND
  "7dbin|"
  # endif // if P157_7DBIN_COMMAND
  "7don|7doff|7db|7output|"
;
enum class P157_commands_e : int8_t {
  invalid = -1,
  c7dn    = 0,
  c7dt,
  # if P157_7DDT_COMMAND
  c7ddt,
  # endif // if P157_7DDT_COMMAND
  c7dst,
  c7dsd,
  c7dtext,
  # if P157_EXTRA_FONTS
  c7dfont,
  # endif // if P157_EXTRA_FONTS
  # if P157_7DBIN_COMMAND
  c7dbin,
  # endif // if P157_7DBIN_COMMAND
  c7don,
  c7doff,
  c7db,
  c7output,

};

bool P157_data_struct::plugin_write(struct EventStruct *event,
                                    const String      & string) {
  const String cmd_s = parseString(string, 1);

  if ((cmd_s.length() < 3) || (cmd_s[0] != '7')) { return false; }

  # if P157_SCROLL_TEXT
  const bool currentScroll = isScrollEnabled(); // Save current state
  bool newScroll           = false;             // disable scroll if command changes
  setScrollEnabled(false);
  # endif // if P157_SCROLL_TEXT

  const int cmd_i = GetCommandCode(cmd_s.c_str(), P157_commands);

  if (cmd_i < 0) { return false; } // Fail fast

  const P157_commands_e cmd = static_cast<P157_commands_e>(cmd_i);

  const String text = parseStringToEndKeepCase(string, 2);
  bool success      = false;
  bool displayon    = false;

  # if P157_7DBIN_COMMAND

  if (
    (cmd != P157_commands_e::c7dbin) &&
    (cmd != P157_commands_e::c7db) &&
    (cmd != P157_commands_e::c7don) &&
    (cmd != P157_commands_e::c7doff)
    ) {
    binData.clear();
  }
  # endif // if P157_7DBIN_COMMAND

  switch (cmd)
  {
    case P157_commands_e::c7dn:
      return plugin_write_7dn(event, text);
    case P157_commands_e::c7dt:
      return plugin_write_7dt(text);
    # if P157_7DDT_COMMAND
    case P157_commands_e::c7ddt:
      return plugin_write_7ddt(text);
    # endif // if P157_7DDT_COMMAND
    case P157_commands_e::c7dst:
      return plugin_write_7dst(event);
    case P157_commands_e::c7dsd:
      return plugin_write_7dsd(event);
    case P157_commands_e::c7dtext:
      # if P157_SCROLL_TEXT
      setScrollEnabled(true); // Scrolling allowed for 7dtext command
      # endif // if P157_SCROLL_TEXT
      return plugin_write_7dtext(text);
    # if P157_EXTRA_FONTS
    case P157_commands_e::c7dfont:
      #  if P157_SCROLL_TEXT
      setScrollEnabled(currentScroll); // Restore state
      #  endif // if P157_SCROLL_TEXT
      return plugin_write_7dfont(event, text);
    # endif // if P157_EXTRA_FONTS
    # if P157_7DBIN_COMMAND
    case P157_commands_e::c7dbin:
      #  if P157_SCROLL_TEXT
      setScrollEnabled(true); // Scrolling allowed for 7dbin command
      #  endif // if P157_SCROLL_TEXT
      return plugin_write_7dbin(string, 1);
    # endif // if P157_7DBIN_COMMAND
    case P157_commands_e::c7don:
      # if P157_SCROLL_TEXT
      newScroll = currentScroll; // Restore state
      # endif // if P157_SCROLL_TEXT
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_INFO, F("7DGT : Display ON"));
      # endif // ifndef BUILD_NO_DEBUG
      displayon = true;
      success   = true;
      break;
    case P157_commands_e::c7doff:
      # if P157_SCROLL_TEXT
      newScroll = currentScroll; // Restore state
      # endif // if P157_SCROLL_TEXT
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_INFO, F("7DGT : Display OFF"));
      # endif // ifndef BUILD_NO_DEBUG
      displayon = false;
      success   = true;
      break;
    case P157_commands_e::c7db:
      # if P157_SCROLL_TEXT
      newScroll = currentScroll; // Restore state
      # endif // if P157_SCROLL_TEXT

      if ((event->Par1 >= 0) && (event->Par1 < 16)) {
        # ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLog(LOG_LEVEL_INFO, concat(F("7DGT : Brightness="), event->Par1));
        }
        # endif // ifndef BUILD_NO_DEBUG
        brightness          = event->Par1;
        P157_CFG_BRIGHTNESS = event->Par1;
        displayon           = true;
        success             = true;
      }
      break;
    case P157_commands_e::c7output:

      if ((event->Par1 >= 0) && (event->Par1 < 6)) { // 0:"Manual",1:"Clock 24h - Blink",2:"Clock 24h - No Blink",
                                                     // 3:"Clock 12h - Blink",4:"Clock 12h - No Blink",5:"Date"
        # ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLog(LOG_LEVEL_INFO, concat(F("7DGT : Display output="), event->Par1));
        }
        # endif // ifndef BUILD_NO_DEBUG
        output              = event->Par1;
        P157_CFG_OUTPUTTYPE = event->Par1;
        displayon           = true;
        success             = true;
        # if P157_SCROLL_TEXT

        if (event->Par1 == 0) { newScroll = currentScroll; } // Restore state
        # endif // if P157_SCROLL_TEXT
      }
      break;
    case P157_commands_e::invalid:
      break;
  }

  if (success) {
    # if P157_SCROLL_TEXT
    setScrollEnabled(newScroll);
    # endif // if P157_SCROLL_TEXT

    if (isInitialized()) {
      ht16k33->setBrightness(brightness);

      if (displayon) {
        ht16k33->on();
      }
      else {
        ht16k33->off();
      }
    }
  }
  return success;
}

void P157_data_struct::getDisplayLimits(int32_t& lLimit,
                                        int32_t& uLimit,
                                        int8_t   offset,
                                        uint8_t  displays) {
  uint8_t dgts = P157_getDefaultDigits(displayModel, displays);

  dgts  -= offset;           // Subtract an offset, used for extra symbol
  lLimit = -pow10(dgts - 1); // Lowest value we can display - 1
  uLimit = pow10(dgts);      // Highest value we can display + 1
  // TODO disable log
  // addLog(LOG_LEVEL_INFO, strformat(F("P157: limits: %d digits(%d), lower: %d, upper: %d"), dgts, offset, lLimit, uLimit));
}

bool P157_data_struct::plugin_write_7dn(struct EventStruct *event,
                                        const String      & text) {
  if (output != P157_DISP_MANUAL) {
    return false;
  }

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, concat(F("7DGT : Show Number="), event->Par1));
  }
  # endif // ifndef BUILD_NO_DEBUG

  int32_t lLimit = 0;
  int32_t uLimit = 0;
  getDisplayLimits(lLimit, uLimit, 0, displays);

  if (!text.isEmpty()) {
    if ((event->Par1 > lLimit) && (event->Par1 < uLimit)) {
      fillBufferWithNumber(text.c_str());
    } else {
      fillBufferWithDash();
    }
  }

  printBuffer();
  return true;
}

bool P157_data_struct::plugin_write_7dt(const String& text) {
  if (output != P157_DISP_MANUAL) {
    return false;
  }

  float P157_temptemp    = 0.0f;
  bool  P157_tempflagdot = false;

  if (!text.isEmpty()) {
    validFloatFromString(text, P157_temptemp);
  }

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, concat(F("7DGT : Show Temperature="), P157_temptemp));
  }
  # endif // ifndef BUILD_NO_DEBUG

  int32_t lLimit = 0;
  int32_t uLimit = 0;
  getDisplayLimits(lLimit, uLimit, hideDegree ? 0 : 1, displays);
  float lLimitErr = lLimit + 0.1f;
  float uLimitErr = uLimit - 1.0f;
  float lLimitDec = lLimit / 10.0f;
  float uLimitDec = uLimit / 10.0f;

  // TODO disable log
  // addLog(LOG_LEVEL_INFO, strformat(F("P157: 7dt: lErr: %.1f, uErr: %.1f, lDec: %.1f, uDec: %.1f"),
  //                                  lLimitErr, uLimitErr, lLimitDec, uLimitDec));

  if ((P157_temptemp > uLimitErr) || (P157_temptemp < lLimitErr)) {
    fillBufferWithDash();
  } else {
    if ((P157_temptemp < uLimitDec) && (P157_temptemp > lLimitDec)) {
      P157_temptemp    = roundf(P157_temptemp * 10.0f);
      P157_tempflagdot = true;
    }
    fillBufferWithTemp(P157_temptemp);
  }

  printBuffer();

  # ifdef P157_DEBUG
  logBufferContent(F("7dt"));
  # endif // ifdef P157_DEBUG
  return true;
}

# if P157_7DDT_COMMAND

bool P157_data_struct::plugin_write_7ddt(const String& text) {
  if (output != P157_DISP_MANUAL) {
    return false;
  }

  float P157_lefttemp    = 0.0f;
  float P157_righttemp   = 0.0f;
  bool  P157_tempflagdot = false;

  if (!text.isEmpty()) {
    validFloatFromString(parseString(text, 1), P157_lefttemp);

    if (text.indexOf(',') > -1) {
      validFloatFromString(parseString(text, 2), P157_righttemp);
    }
  }

  #  ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, strformat(F("7DGT : Dual Temperature 1st=%.2f 2nd=%.2f"), P157_lefttemp, P157_righttemp));
  }
  #  endif // ifndef BUILD_NO_DEBUG

  {
    uint8_t firstDot   = -1; // No decimals is no dots
    uint8_t secondDot  = -1;
    float   hideFactor = hideDegree ? 10.0f : 1.0f;

    if ((P157_lefttemp > 999.99f * hideFactor) || (P157_lefttemp < -99.99f * hideFactor)) {
      P157_lefttemp = -101.0f * hideFactor; // Triggers on -100
    } else {
      if ((P157_lefttemp < 100.0f * hideFactor) && (P157_lefttemp > -10.0f * hideFactor)) {
        P157_lefttemp = roundf(P157_lefttemp * 10.0f);
        firstDot      = hideDegree ? 2 : 1;
      }
    }

    if ((P157_righttemp > 999.99f * hideFactor) || (P157_righttemp < -99.99f * hideFactor)) {
      P157_righttemp = -101.0f * hideFactor;
    } else {
      if ((P157_righttemp < 100.0f * hideFactor) && (P157_righttemp > -10.0f * hideFactor)) {
        P157_righttemp = roundf(P157_righttemp * 10.0f);
        secondDot      = hideDegree ? 6 : 5;
      }
    }

    #  ifdef P157_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO, strformat(F("7DGT : 7ddt preprocessed 1st=%.2f 2nd=%.2f"), P157_lefttemp, P157_righttemp));
    }
    #  endif // ifdef P157_DEBUG

    fillBufferWithDualTemp(P157_lefttemp, firstDot > -1, P157_righttemp, secondDot > -1);

    printBuffer();

  }
  #  ifdef P157_DEBUG
  logBufferContent(F("7ddt"));
  #  endif // ifdef P157_DEBUG
  return true;
}

# endif // if P157_7DDT_COMMAND

bool P157_data_struct::plugin_write_7dst(struct EventStruct *event) {
  if (output != P157_DISP_MANUAL) {
    return false;
  }

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, strformat(F("7DGT : Show Time=%02d:%02d:%02d"), event->Par1, event->Par2, event->Par3));
  }
  # endif // ifndef BUILD_NO_DEBUG
  timesep = true;
  fillBufferWithTime(false, event->Par1, event->Par2, event->Par3, false,
                     # if P157_SUPPRESS_ZERO
                     suppressLeading0
                     # else // if P157_SUPPRESS_ZERO
                     false
                     # endif // if P157_SUPPRESS_ZERO
                     );

  printBuffer();
  return true;
}

bool P157_data_struct::plugin_write_7dsd(struct EventStruct *event) {
  if (output != P157_DISP_MANUAL) {
    return false;
  }

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, strformat(F("7DGT : Show Date=%02d:%02d:%02d"), event->Par1, event->Par2, event->Par3));
  }
  # endif // ifndef BUILD_NO_DEBUG
  fillBufferWithDate(false, event->Par1, event->Par2, event->Par3,
                     # if P157_SUPPRESS_ZERO
                     suppressLeading0
                     # else // if P157_SUPPRESS_ZERO
                     false
                     # endif // if P157_SUPPRESS_ZERO
                     );

  printBuffer();
  return true;
}

bool P157_data_struct::plugin_write_7dtext(const String& text) {
  if (output != P157_DISP_MANUAL) {
    return false;
  }
  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLogMove(LOG_LEVEL_INFO, concat(F("7DGT : Show Text="), text));
  }
  # endif // ifndef BUILD_NO_DEBUG
  # if P157_SCROLL_TEXT
  setTextToScroll(EMPTY_STRING);
  const uint8_t bufLen = P157_getDefaultDigits(displayModel, displays);

  if (isScrollEnabled() && (getEffectiveTextLength(text) > bufLen)) {
    setTextToScroll(text);
  } else
  # endif // if P157_SCROLL_TEXT
  {
    fillBufferWithString(text);

    printBuffer();
  }
  return true;
}

# if P157_EXTRA_FONTS

bool P157_data_struct::plugin_write_7dfont(struct EventStruct *event,
                                           const String      & text) {
  if (!P157_is7SegmentDisplay(displayModel)) { return false; }

  if (!text.isEmpty()) {
    int32_t fontNr = P073_parse_7dfont(event, text);
    #  ifdef P157_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO, strformat(F("P157 7dfont,%s -> %d"), parseString(text, 1).c_str(), fontNr));
    }
    #  endif // ifdef P157_DEBUG

    if ((fontNr >= 0) && (fontNr <= 3)) {
      fontSet          = fontNr;
      P157_CFG_FONTSET = fontNr;
      return true;
    }
  }
  return false;
}

# endif // if P157_EXTRA_FONTS

# if P157_7DBIN_COMMAND

bool P157_data_struct::plugin_write_7dbin(const String& text,
                                          const uint8_t offset) {
  if (!text.isEmpty()) {
    binData.clear();
    scrollPos = 0;

    uint32_t wordValue{};
    uint8_t  arg      = 1;
    String   argValue = parseStringKeepCaseNoTrim(text, offset + arg);

    while (!argValue.isEmpty()) {
      NumericalType numType;

      if (isNumerical(argValue, numType) && (numType != NumericalType::FloatingPoint) &&
          validUIntFromString(argValue, wordValue) && (wordValue <= 0xFFFF)) {
        #  ifdef P157_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLog(LOG_LEVEL_INFO, strformat(F("7dbin : argValue: %s value: 0x%04x"), argValue.c_str(), wordValue));
        }
        #  endif // ifdef P157_DEBUG
        binData.push_back(wordValue & 0xFFFF);
      } else {
        #  ifdef P157_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLog(LOG_LEVEL_INFO, strformat(F("7dbin : argValue: %s"), argValue.c_str()));
        }
        #  endif // ifdef P157_DEBUG

        uint16_t i = 0;

        while (i < argValue.length()) {
          uint16_t bitmap;

          #  if P157_EXTRA_FONTS

          if (P157_is7SegmentDisplay(displayModel)) {
            // Re-use the fonts from P073, but they use the MAX7219 layout, that has bits 0..6 reverted
            bitmap = P073_revert7bits(P073_getFontChar(P073_mapCharToFontPosition(argValue.charAt(i), fontSet), fontSet));

          } else
          #  endif // if P157_EXTRA_FONTS
          {
            bitmap = ht16k33->getCharacterBitmap(argValue.charAt(i));
          }

          if ((i < argValue.length()) && isPeriodChar(argValue.charAt(i + 1))) {
            bitmap |= P157_is7SegmentDisplay(displayModel) ? SEG_DP : SEG14_DP;
            ++i;
          }
          binData.push_back(bitmap);
          ++i;
        }
      }
      arg++;
      argValue = parseStringKeepCaseNoTrim(text, offset + arg);
    }

    if (binData.size() > 0) {
      #  if P157_SCROLL_TEXT

      if (isScrollEnabled()) {
        const uint8_t bufLen = P157_getDefaultDigits(displayModel, displays);

        for (uint8_t i = 0; scrollFull && i < bufLen; ++i) { // prepend to start display empty
          binData.insert(binData.begin(), 0);
        }

        for (uint8_t i; i < bufLen; ++i) { // append empty to scroll until empty
          binData.push_back(0);
        }
      }
      #  endif // if P157_SCROLL_TEXT

      if (!isScrollEnabled()) {
        printBuffer();
      }
      return true;
    }
  }
  return false;
}

# endif // if P157_7DBIN_COMMAND

#endif    // ifdef USES_P157
