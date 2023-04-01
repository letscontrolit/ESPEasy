#include "_Plugin_Helper.h"
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
//  "7dn,<number>"        (number can be negative or positive, even with decimal)
//  "7dt,<temperature>"   (temperature can be negative or positive and containing decimals)
//  "7ddt,<temperature>,<temperature>"   (Dual temperatures on Max7219 (8 digits) only, temperature can be negative or
//                                        positive and containing decimals)
//  "7dst,<hh>,<mm>,<ss>" (show manual time -not current-, no checks done on numbers validity!)
//  "7dsd,<dd>,<mm>,<yy>" (show manual date -not current-, no checks done on numbers validity!)
//  "7dtext,<text>"       (show free text - supported chars 0-9,a-z,A-Z," ","-","=","_","/","^") Depending on Font used
//  "7dfont,<font>"       (select the used font: 0/7DGT/Default = default, 1/Siekoo = Siekoo, 2/Siekoo_Upper = Siekoo
//                         with uppercase CHNORUX, 3/dSEG7 = dSEG7)
//                        Siekoo: https://www.fakoo.de/siekoo (uppercase CHNORUX is a local extension)
//                        dSEG7 : https://www.keshikan.net/fonts-e.html
//  "7dbin,[uint8_t],..."    (show data binary formatted, bits clock-wise from left to right, dot, top, right 2x, bottom,
//                            left 2x, center), scroll-enabled
//  - Clock-Blink     -- display is automatically updated with current time and blinking dot/lines
//  - Clock-NoBlink   -- display is automatically updated with current time and steady dot/lines
//  - Clock12-Blink   -- display is automatically updated with current time (12h clock) and blinking dot/lines
//  - Clock12-NoBlink -- display is automatically updated with current time (12h clock) and steady dot/lines
//  - Date            -- display is automatically updated with current date
//
// Generic commands:
//  - "7don"         -- turn ON the display
//  - "7doff"        -- turn OFF the display
//  - "7db,<0-15>    -- set brightness to specific value between 0 and 15
//  - "7output,<0-5> -- select display output mode, 0:"Manual",1:"Clock 24h - Blink",2:"Clock 24h - No Blink",3:"Clock 12h - Blink",4:"Clock
//                      12h - No Blink",5:"Date"
//
// History
// 2023-03-30, tonhuisman: Correct 7dtext on 6-digit TM1637 to also swap the dots when swapping the digits
// 2023-03-29, tonhuisman: Add option to suppress the leading zero on day and hour when < 10
//                         Disable scrolling for content/commands that don't support that
// 2023-03-28, tonhuisman: Guard scrolling feature to only be used for 7dtext and 7dbin commands, and fix scrolling on 6-digit display
//                         Fix 7dbin command to also work correctly for TM1637 displays
// 2022-02-03, tonhuisman: Move P073_data_struct to PluginStruct directory, code optimizations, de-duplication
// 2021-10-06, tonhuisman: Store via commands changed output, font and brightness setting in settings variables, but not save them yet.
// 2021-10-05, tonhuisman: Add 7output command for changing the Display Output setting. Not saved, unless the save command is also sent.
// 2021-02-13, tonhuisman: Fixed self-introduced bug of conversion from MAX7219 to TM1637 bit mapping, removed now unused TM1637 character
//                         maps, moved some logging to DEBUG level
// 2021-01-30, tonhuisman: Added font support for 7Dgt (default), Siekoo, Siekoo with uppercase CHNORUX, dSEG7 fonts. Default/7Dgt comes
//                         with these special characters: " -^=/_
//                         Siekoo comes _without_ AOU with umlauts and Eszett characters, has many extra special characters
//                         "%@.,;:+*#!?'\"<>\\()|", and optional uppercase "CHNORUX",
//                         "^" displays as degree symbol and "|" displays overscsore (top-line only).
//                         'Merged' fontdata for TM1637 by converting the data for MAX7219 (bits 0-6 are swapped around), to save a little
//                         space and maintanance burden.
//                         Added 7dfont,<font> command for changing the font dynamically runtime. NB: The numbers digits are equal for all
//                         fonts!
//                         Added Scroll Text option for scrolling texts longer then the display is wide
//                         Added 7dbin,<uint8_t>[,...] for displaying binary formatted data bits clock-wise from left to right, dot, top,
//                         right 2x, bottom, left 2x, center), scroll-enabled
// 2021-01-10, tonhuisman: Added optional . as dot display (7dtext)
//                         Added 7ddt,<temp1>,<temp2> dual temperature display
//                         Added optional removal of degree symbol on temperature display
//                         Added optional right-shift of 7dt,<temp> on MAX7219 display (is normally shifted to left by 1 digit, so last
//                         digit is blank)

# define PLUGIN_073
# define PLUGIN_ID_073           73
# define PLUGIN_NAME_073         "Display - 7-segment display"

# include "src/PluginStructs/P073_data_struct.h"

void tm1637_ShowDate6(struct EventStruct *event,
                      bool                showTime = false); // Forward declaration for default argument
void tm1637_ShowBuffer(struct EventStruct *event,
                       uint8_t             firstPos,
                       uint8_t             lastPos,
                       bool                useBinaryData = false);


boolean Plugin_073(uint8_t function, struct EventStruct *event, String& string) {
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

    # ifdef P073_SCROLL_TEXT
    case PLUGIN_SET_DEFAULTS: {
      PCONFIG(3) = 10; // Default 10 * 0.1 sec scroll speed
      break;
    }
    # endif // P073_SCROLL_TEXT

    case PLUGIN_WEBFORM_LOAD: {
      addFormNote(F("TM1637:  1st=CLK-Pin, 2nd=DIO-Pin"));
      addFormNote(F("MAX7219: 1st=DIN-Pin, 2nd=CLK-Pin, 3rd=CS-Pin"));
      {
        const __FlashStringHelper *displtype[] = { F("TM1637 - 4 digit (colon)"),
                                                   F("TM1637 - 4 digit (dots)"),
                                                   F("TM1637 - 6 digit"),
                                                   F("MAX7219 - 8 digit") };
        addFormSelector(F("Display Type"), F("displtype"), 4, displtype, nullptr, PCONFIG(0));
      }
      {
        const __FlashStringHelper *displout[] = { F("Manual"),
                                                  F("Clock 24h - Blink"),
                                                  F("Clock 24h - No Blink"),
                                                  F("Clock 12h - Blink"),
                                                  F("Clock 12h - No Blink"),
                                                  F("Date") };
        addFormSelector(F("Display Output"), F("displout"), 6, displout, nullptr, PCONFIG(1));
      }

      addFormNumericBox(F("Brightness"), F("brightness"), PCONFIG(2), 0, 15);
      addUnit(F("0..15"));

      # ifdef P073_EXTRA_FONTS
      {
        const __FlashStringHelper *fontset[4] = { F("Default"),
                                                  F("Siekoo"),
                                                  F("Siekoo with uppercase 'CHNORUX'"),
                                                  F("dSEG7") };
        addFormSelector(F("Font set"), F("fontset"), 4, fontset, nullptr, PCONFIG(4));
        addFormNote(F("Check documentation for examples of the font sets."));
      }
      # endif // P073_EXTRA_FONTS

      addFormSubHeader(F("Options"));

      addFormCheckBox(F("Text show periods as dot"),    F("periods"),     bitRead(PCONFIG_LONG(0), P073_OPTION_PERIOD));

      addFormCheckBox(F("Hide &deg; for Temperatures"), F("hide_degree"), bitRead(PCONFIG_LONG(0), P073_OPTION_HIDEDEGREE));
      # ifdef P073_7DDT_COMMAND
      addFormNote(F("Commands 7dt,&lt;temp&gt; and 7ddt,&lt;temp1&gt;,&lt;temp2&gt;"));
      # else // ifdef P073_7DDT_COMMAND
      addFormNote(F("Command 7dt,&lt;temp&gt;"));
      # endif // P073_7DDT_COMMAND
      # ifdef P073_SUPPRESS_ZERO
      addFormCheckBox(F("Suppress leading 0 on day/hour"), F("supp0"), bitRead(PCONFIG_LONG(0), P073_OPTION_SUPPRESS0));
      # endif // ifdef P073_SUPPRESS_ZERO

      # ifdef P073_SCROLL_TEXT
      addFormCheckBox(F("Scroll text &gt; display width"), F("scroll_text"), bitRead(PCONFIG_LONG(0), P073_OPTION_SCROLLTEXT));
      addFormCheckBox(F("Scroll text in from right"),      F("scroll_full"), bitRead(PCONFIG_LONG(0), P073_OPTION_SCROLLFULL));

      if (PCONFIG(3) == 0) { PCONFIG(3) = 10; }
      addFormNumericBox(F("Scroll speed (0.1 sec/step)"), F("scrollspeed"), PCONFIG(3), 1, 600);
      addUnit(F("1..600 = 0.1..60 sec/step"));
      # endif // P073_SCROLL_TEXT

      addFormSubHeader(F("Options for MAX7219 - 8 digit"));

      bool bRightAlign = bitRead(PCONFIG_LONG(0), P073_OPTION_RIGHTALIGN);
      addFormCheckBox(F("Right-align Temperature (7dt)"), F("temp_rightalign"), bRightAlign);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      PCONFIG(0) = getFormItemInt(F("displtype"));
      PCONFIG(1) = getFormItemInt(F("displout"));
      PCONFIG(2) = getFormItemInt(F("brightness"));
      uint32_t lSettings = 0;
      bitWrite(lSettings, P073_OPTION_PERIOD,     isFormItemChecked(F("periods")));
      bitWrite(lSettings, P073_OPTION_HIDEDEGREE, isFormItemChecked(F("hide_degree")));
      bitWrite(lSettings, P073_OPTION_RIGHTALIGN, isFormItemChecked(F("temp_rightalign")));
      # ifdef P073_SCROLL_TEXT
      bitWrite(lSettings, P073_OPTION_SCROLLTEXT, isFormItemChecked(F("scroll_text")));
      bitWrite(lSettings, P073_OPTION_SCROLLFULL, isFormItemChecked(F("scroll_full")));
      PCONFIG(3) = getFormItemInt(F("scrollspeed"));
      # endif // P073_SCROLL_TEXT
      # ifdef P073_SUPPRESS_ZERO
      bitWrite(lSettings, P073_OPTION_SUPPRESS0, isFormItemChecked(F("supp0")));
      # endif // ifdef P073_SUPPRESS_ZERO
      # ifdef P073_EXTRA_FONTS
      PCONFIG(4) = getFormItemInt(F("fontset"));
      # endif // P073_EXTRA_FONTS
      PCONFIG_LONG(0) = lSettings;

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

      if (nullptr != P073_data) {
        P073_data->init(event);

        switch (P073_data->displayModel) {
          case P073_TM1637_4DGTCOLON:
          case P073_TM1637_4DGTDOTS:
          case P073_TM1637_6DGT: {
            tm1637_InitDisplay(CONFIG_PIN1, CONFIG_PIN2);
            tm1637_SetPowerBrightness(CONFIG_PIN1, CONFIG_PIN2, PCONFIG(2) / 2, true);

            if (PCONFIG(1) == P073_DISP_MANUAL) {
              tm1637_ClearDisplay(CONFIG_PIN1, CONFIG_PIN2);
            }
            break;
          }
          case P073_MAX7219_8DGT: {
            max7219_InitDisplay(event, CONFIG_PIN1, CONFIG_PIN2, CONFIG_PIN3);
            delay(10); // small poweroff/poweron delay
            max7219_SetPowerBrightness(event, CONFIG_PIN1, CONFIG_PIN2, CONFIG_PIN3, PCONFIG(2), true);

            if (PCONFIG(1) == P073_DISP_MANUAL) {
              max7219_ClearDisplay(event, CONFIG_PIN1, CONFIG_PIN2, CONFIG_PIN3);
            }
            break;
          }
        }
        success = true;
      }
      break;
    }

    case PLUGIN_WRITE: {
      success = p073_plugin_write(event, string);
      break;
    }

    case PLUGIN_ONCE_A_SECOND: {
      P073_data_struct *P073_data =
        static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr == P073_data) || (P073_data->output == P073_DISP_MANUAL)) {
        break;
      }

      if ((P073_data->output == P073_DISP_CLOCK24BLNK) ||
          (P073_data->output == P073_DISP_CLOCK12BLNK)) {
        P073_data->timesep = !P073_data->timesep;
      } else {
        P073_data->timesep = true;
      }

      if (P073_data->output == P073_DISP_DATE) {
        P073_data->FillBufferWithDate(true, 0, 0, 0,
                                      # ifdef P073_SUPPRESS_ZERO
                                      bitRead(PCONFIG_LONG(0), P073_OPTION_SUPPRESS0)
                                      # else // ifdef P073_SUPPRESS_ZERO
                                      false
                                      # endif // ifdef P073_SUPPRESS_ZERO
                                      );
      } else {
        P073_data->FillBufferWithTime(true, 0, 0, 0, !((P073_data->output == P073_DISP_CLOCK24BLNK) ||
                                                       (P073_data->output == P073_DISP_CLOCK24)),
                                      # ifdef P073_SUPPRESS_ZERO
                                      bitRead(PCONFIG_LONG(0), P073_OPTION_SUPPRESS0)
                                      # else // ifdef P073_SUPPRESS_ZERO
                                      false
                                      # endif // ifdef P073_SUPPRESS_ZERO
                                      );
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
          } else {
            tm1637_ShowTime6(event);
          }
          break;
        }
        case P073_MAX7219_8DGT: {
          if (PCONFIG(1) == P073_DISP_DATE) {
            max7219_ShowDate(event, P073_data->pin1, P073_data->pin2,
                             P073_data->pin3);
          } else {
            max7219_ShowTime(event, P073_data->pin1, P073_data->pin2,
                             P073_data->pin3, P073_data->timesep);
          }
          break;
        }
      }
      break;
    }

    # ifdef P073_SCROLL_TEXT
    case PLUGIN_TEN_PER_SECOND: {
      P073_data_struct *P073_data =
        static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P073_data) {
        break;
      }

      if ((P073_data->output != P073_DISP_MANUAL) || !P073_data->isScrollEnabled()) {
        break;
      }

      if (P073_data->NextScroll()) {
        switch (P073_data->displayModel) {
          case P073_TM1637_4DGTCOLON:
          case P073_TM1637_4DGTDOTS: {
            tm1637_ShowBuffer(event, 0, 4
                              #  ifdef P073_7DBIN_COMMAND
                              , P073_data->binaryData
                              #  endif // ifdef P073_7DBIN_COMMAND
                              );
            break;
          }
          case P073_TM1637_6DGT: {
            tm1637_SwapDigitInBuffer(event, 0); // only needed for 6-digits displays
            tm1637_ShowBuffer(event, 0, 6
                              #  ifdef P073_7DBIN_COMMAND
                              , P073_data->binaryData
                              #  endif // ifdef P073_7DBIN_COMMAND
                              );
            break;
          }
          case P073_MAX7219_8DGT: {
            P073_data->dotpos = -1; // avoid to display the dot
            max7219_ShowBuffer(event, P073_data->pin1, P073_data->pin2,
                               P073_data->pin3);
            break;
          }
        }
      }
      break;
    }
    # endif // P073_SCROLL_TEXT
  }
  return success;
}

bool p073_plugin_write(struct EventStruct *event,
                       const String      & string) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return false;
  }

  const String cmd = parseString(string, 1);

  if ((cmd.length() < 3) || (cmd[0] != '7')) { return false; }

  # ifdef P073_SCROLL_TEXT
  const bool currentScroll = P073_data->isScrollEnabled(); // Save current state
  bool newScroll           = false;                        // disable scroll if command changes
  P073_data->setScrollEnabled(false);
  # endif // ifdef P073_SCROLL_TEXT

  const String text = parseStringToEndKeepCase(string, 2);

  if (equals(cmd, F("7dn"))) {
    return p073_plugin_write_7dn(event, text);
  } else if (equals(cmd, F("7dt"))) {
    return p073_plugin_write_7dt(event, text);
  # ifdef P073_7DDT_COMMAND
  } else if (equals(cmd, F("7ddt"))) {
    return p073_plugin_write_7ddt(event, text);
  # endif // ifdef P073_7DDT_COMMAND
  } else if (equals(cmd, F("7dst"))) {
    return p073_plugin_write_7dst(event);
  } else if (equals(cmd, F("7dsd"))) {
    return p073_plugin_write_7dsd(event);
  } else if (equals(cmd, F("7dtext"))) {
    # ifdef P073_SCROLL_TEXT
    P073_data->setScrollEnabled(true); // Scrolling allowed for 7dtext command
    # endif // ifdef P073_SCROLL_TEXT
    return p073_plugin_write_7dtext(event, text);
  # ifdef P073_EXTRA_FONTS
  } else if (equals(cmd, F("7dfont"))) {
    #  ifdef P073_SCROLL_TEXT
    P073_data->setScrollEnabled(currentScroll); // Restore state
    #  endif // ifdef P073_SCROLL_TEXT
    return p073_plugin_write_7dfont(event, text);
  # endif // P073_EXTRA_FONTS
  # ifdef P073_7DBIN_COMMAND
  } else if (equals(cmd, F("7dbin"))) {
    #  ifdef P073_SCROLL_TEXT
    P073_data->setScrollEnabled(true); // Scrolling allowed for 7dbin command
    #  endif // ifdef P073_SCROLL_TEXT
    return p073_plugin_write_7dbin(event, text);
  # endif // P073_7DBIN_COMMAND
  } else {
    bool p073_validcmd  = false;
    bool p073_displayon = false;

    if (equals(cmd, F("7don"))) {
      # ifdef P073_SCROLL_TEXT
      newScroll = currentScroll; // Restore state
      # endif // ifdef P073_SCROLL_TEXT
      addLog(LOG_LEVEL_INFO, F("7DGT : Display ON"));
      p073_displayon = true;
      p073_validcmd  = true;
    } else if (equals(cmd, F("7doff"))) {
      # ifdef P073_SCROLL_TEXT
      newScroll = currentScroll; // Restore state
      # endif // ifdef P073_SCROLL_TEXT
      addLog(LOG_LEVEL_INFO, F("7DGT : Display OFF"));
      p073_displayon = false;
      p073_validcmd  = true;
    } else if (equals(cmd, F("7db"))) {
      # ifdef P073_SCROLL_TEXT
      newScroll = currentScroll; // Restore state
      # endif // ifdef P073_SCROLL_TEXT

      if ((event->Par1 >= 0) && (event->Par1 < 16)) {
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("7DGT : Brightness=");
          log += event->Par1;
          addLogMove(LOG_LEVEL_INFO, log);
        }
        P073_data->brightness = event->Par1;
        PCONFIG(2)            = event->Par1;
        p073_displayon        = true;
        p073_validcmd         = true;
      }
    } else if (equals(cmd, F("7output"))) {
      if ((event->Par1 >= 0) && (event->Par1 < 6)) { // 0:"Manual",1:"Clock 24h - Blink",2:"Clock 24h - No Blink",
                                                     // 3:"Clock 12h - Blink",4:"Clock 12h - No Blink",5:"Date"
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("7DGT : Display output=");
          log += event->Par1;
          addLogMove(LOG_LEVEL_INFO, log);
        }
        P073_data->output = event->Par1;
        PCONFIG(1)        = event->Par1;
        p073_displayon    = true;
        p073_validcmd     = true;
        # ifdef P073_SCROLL_TEXT

        if (event->Par1 == 0) { newScroll = currentScroll; } // Restore state
        # endif // ifdef P073_SCROLL_TEXT
      }
    }

    if (p073_validcmd) {
      # ifdef P073_SCROLL_TEXT
      P073_data->setScrollEnabled(newScroll);
      # endif // ifdef P073_SCROLL_TEXT

      switch (P073_data->displayModel) {
        case P073_TM1637_4DGTCOLON:
        case P073_TM1637_4DGTDOTS:
        case P073_TM1637_6DGT:
        {
          tm1637_SetPowerBrightness(P073_data->pin1, P073_data->pin2,
                                    P073_data->brightness / 2, p073_displayon);
          break;
        }
        case P073_MAX7219_8DGT:
        {
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

bool p073_plugin_write_7dn(struct EventStruct *event,
                           const String      & text) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr == P073_data) || (P073_data->output != P073_DISP_MANUAL)) {
    return false;
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("7DGT : Show Number=");
    log += event->Par1;
    addLogMove(LOG_LEVEL_INFO, log);
  }

  switch (P073_data->displayModel) {
    case P073_TM1637_4DGTCOLON:
    case P073_TM1637_4DGTDOTS:
    {
      if ((event->Par1 > -1000) && (event->Par1 < 10000)) {
        P073_data->FillBufferWithNumber(text.c_str());
      } else {
        P073_data->FillBufferWithDash();
      }
      tm1637_ShowBuffer(event, TM1637_4DIGIT, 8);
      break;
    }
    case P073_TM1637_6DGT:
    {
      if ((event->Par1 > -100000) && (event->Par1 < 1000000)) {
        P073_data->FillBufferWithNumber(text.c_str());
      } else {
        P073_data->FillBufferWithDash();
      }
      tm1637_SwapDigitInBuffer(event, 2); // only needed for 6-digits displays
      tm1637_ShowBuffer(event, TM1637_6DIGIT, 8);
      break;
    }
    case P073_MAX7219_8DGT:
    {
      if (!text.isEmpty()) {
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

bool p073_plugin_write_7dt(struct EventStruct *event,
                           const String      & text) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr == P073_data) || (P073_data->output != P073_DISP_MANUAL)) {
    return false;
  }

  float p073_temptemp    = 0;
  bool  p073_tempflagdot = false;

  if (!text.isEmpty()) {
    validFloatFromString(text, p073_temptemp);
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("7DGT : Show Temperature=");
    log += p073_temptemp;
    addLogMove(LOG_LEVEL_INFO, log);
  }

  switch (P073_data->displayModel) {
    case P073_TM1637_4DGTCOLON:
    case P073_TM1637_4DGTDOTS:
    case P073_TM1637_6DGT:
    {
      if ((p073_temptemp > 999.0f) || (p073_temptemp < -99.9f)) {
        P073_data->FillBufferWithDash();
      } else {
        if ((p073_temptemp < 100.0f) && (p073_temptemp > -10.0f)) {
          p073_temptemp    = roundf(p073_temptemp * 10.0f);
          p073_tempflagdot = true;
        }
        P073_data->FillBufferWithTemp(p073_temptemp);

        if ((p073_temptemp == 0) && p073_tempflagdot) {
          P073_data->showbuffer[5] = 0;
        }
      }

      if (P073_TM1637_6DGT == P073_data->displayModel) {
        tm1637_ShowTemp6(event, p073_tempflagdot);
      } else {
        tm1637_ShowTimeTemp4(event, p073_tempflagdot, 4);
      }
      break;
    }
    case P073_MAX7219_8DGT:
    {
      p073_temptemp = roundf(p073_temptemp * 10.0f);
      P073_data->FillBufferWithTemp(p073_temptemp);

      # ifdef P073_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("7DGT : 7dt preprocessed =");
        log += p073_temptemp;
        addLogMove(LOG_LEVEL_INFO, log);
      }
      # endif // ifdef P073_DEBUG

      max7219_ShowTemp(event, P073_data->pin1, P073_data->pin2, P073_data->pin3, P073_data->hideDegree ? 6 : 5, -1);
      break;
    }
  }
  # ifdef P073_DEBUG
  P073_data->LogBufferContent(F("7dt"));
  # endif // ifdef P073_DEBUG
  return true;
}

# ifdef P073_7DDT_COMMAND
bool p073_plugin_write_7ddt(struct EventStruct *event,
                            const String      & text) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr == P073_data) || (P073_data->output != P073_DISP_MANUAL)) {
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

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("7DGT : Show Temperature 1st=");
    log += p073_lefttemp;
    log += F(" 2nd=");
    log += p073_righttemp;
    addLogMove(LOG_LEVEL_INFO, log);
  }

  switch (P073_data->displayModel) {
    case P073_TM1637_4DGTCOLON:
    case P073_TM1637_4DGTDOTS:
    case P073_TM1637_6DGT:
    {
      P073_data->FillBufferWithDash();

      if (P073_data->displayModel == P073_TM1637_6DGT) {
        tm1637_ShowTemp6(event, p073_tempflagdot);
      } else {
        tm1637_ShowTimeTemp4(event, p073_tempflagdot, 4);
      }
      break;
    }
    case P073_MAX7219_8DGT:
    {
      uint8_t firstDot       = -1; // No decimals is no dots
      uint8_t secondDot      = -1;
      float   hideFactor     = P073_data->hideDegree ? 10.0f : 1.0f;
      bool    firstDecimals  = false;
      bool    secondDecimals = false;

      if ((p073_lefttemp > 999.99f * hideFactor) || (p073_lefttemp < -99.99f * hideFactor)) {
        p073_lefttemp = -101.0f * hideFactor; // Triggers on -100
      } else {
        if ((p073_lefttemp < 100.0f * hideFactor) && (p073_lefttemp > -10.0f * hideFactor)) {
          p073_lefttemp = roundf(p073_lefttemp * 10.0f);
          firstDot      = P073_data->hideDegree ? 2 : 1;
          firstDecimals = true;
        }
      }

      if ((p073_righttemp > 999.99f * hideFactor) || (p073_righttemp < -99.99f * hideFactor)) {
        p073_righttemp = -101.0f * hideFactor;
      } else {
        if ((p073_righttemp < 100.0f * hideFactor) && (p073_righttemp > -10.0f * hideFactor)) {
          p073_righttemp = roundf(p073_righttemp * 10.0f);
          secondDot      = P073_data->hideDegree ? 6 : 5;
          secondDecimals = true;
        }
      }

      #  ifdef P073_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("7DGT : 7ddt preprocessed 1st=");
        log += p073_lefttemp;
        log += F(" 2nd=");
        log += p073_righttemp;
        addLogMove(LOG_LEVEL_INFO, log);
      }
      #  endif // ifdef P073_DEBUG

      P073_data->FillBufferWithDualTemp(p073_lefttemp, firstDecimals, p073_righttemp, secondDecimals);

      bool alignSave = P073_data->rightAlignTempMAX7219; // Save setting
      P073_data->rightAlignTempMAX7219 = true;

      max7219_ShowTemp(event, P073_data->pin1, P073_data->pin2, P073_data->pin3, firstDot, secondDot);

      P073_data->rightAlignTempMAX7219 = alignSave; // Restore

      break;
    }
  }
  #  ifdef P073_DEBUG
  P073_data->LogBufferContent(F("7ddt"));
  #  endif // ifdef P073_DEBUG
  return true;
}

# endif // ifdef P073_7DDT_COMMAND

bool p073_plugin_write_7dst(struct EventStruct *event) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr == P073_data) || (P073_data->output != P073_DISP_MANUAL)) {
    return false;
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("7DGT : Show Time=");
    log += event->Par1;
    log += ':';
    log += event->Par2;
    log += ':';
    log += event->Par3;
    addLogMove(LOG_LEVEL_INFO, log);
  }
  P073_data->timesep = true;
  P073_data->FillBufferWithTime(false, event->Par1, event->Par2, event->Par3, false,
                                # ifdef P073_SUPPRESS_ZERO
                                bitRead(PCONFIG_LONG(0), P073_OPTION_SUPPRESS0)
                                # else // ifdef P073_SUPPRESS_ZERO
                                false
                                # endif // ifdef P073_SUPPRESS_ZERO
                                );

  switch (P073_data->displayModel) {
    case P073_TM1637_4DGTCOLON:
    case P073_TM1637_4DGTDOTS:
    {
      tm1637_ShowTimeTemp4(event, P073_data->timesep, 0);
      break;
    }
    case P073_TM1637_6DGT:
    {
      tm1637_ShowTime6(event);
      break;
    }
    case P073_MAX7219_8DGT:
    {
      max7219_ShowTime(event, P073_data->pin1, P073_data->pin2, P073_data->pin3, P073_data->timesep);
      break;
    }
  }
  return true;
}

bool p073_plugin_write_7dsd(struct EventStruct *event) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr == P073_data) || (P073_data->output != P073_DISP_MANUAL)) {
    return false;
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("7DGT : Show Date=");
    log += event->Par1;
    log += '-';
    log += event->Par2;
    log += '-';
    log += event->Par3;
    addLogMove(LOG_LEVEL_INFO, log);
  }
  P073_data->FillBufferWithDate(false, event->Par1, event->Par2, event->Par3,
                                # ifdef P073_SUPPRESS_ZERO
                                bitRead(PCONFIG_LONG(0), P073_OPTION_SUPPRESS0)
                                # else // ifdef P073_SUPPRESS_ZERO
                                false
                                # endif // ifdef P073_SUPPRESS_ZERO
                                );

  switch (P073_data->displayModel) {
    case P073_TM1637_4DGTCOLON:
    case P073_TM1637_4DGTDOTS:
    {
      tm1637_ShowTimeTemp4(event, P073_data->timesep, 0);
      break;
    }
    case P073_TM1637_6DGT:
    {
      tm1637_ShowDate6(event);
      break;
    }
    case P073_MAX7219_8DGT:
    {
      max7219_ShowDate(event, P073_data->pin1, P073_data->pin2, P073_data->pin3);
      break;
    }
  }
  return true;
}

bool p073_plugin_write_7dtext(struct EventStruct *event,
                              const String      & text) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr == P073_data) || (P073_data->output != P073_DISP_MANUAL)) {
    return false;
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("7DGT : Show Text=");
    log += text;
    addLogMove(LOG_LEVEL_INFO, log);
  }
  # ifdef P073_SCROLL_TEXT
  P073_data->setTextToScroll("");
  uint8_t bufLen = P073_data->getBufferLength(P073_data->displayModel);

  if (P073_data->isScrollEnabled() && (P073_data->getEffectiveTextLength(text) > bufLen)) {
    P073_data->setTextToScroll(text);
  } else
  # endif // P073_SCROLL_TEXT
  {
    P073_data->FillBufferWithString(text);

    switch (P073_data->displayModel) {
      case P073_TM1637_4DGTCOLON:
      case P073_TM1637_4DGTDOTS:
      {
        tm1637_ShowBuffer(event, 0, 4);
        break;
      }
      case P073_TM1637_6DGT:
      {
        tm1637_SwapDigitInBuffer(event, 0); // only needed for 6-digits displays
        tm1637_ShowBuffer(event, 0, 6);
        break;
      }
      case P073_MAX7219_8DGT:
      {
        P073_data->dotpos = -1; // avoid to display the dot
        max7219_ShowBuffer(event, P073_data->pin1, P073_data->pin2, P073_data->pin3);
        break;
      }
    }
  }
  return true;
}

# ifdef P073_EXTRA_FONTS
bool p073_plugin_write_7dfont(struct EventStruct *event,
                              const String      & text) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return false;
  }

  if (!text.isEmpty()) {
    String fontArg = parseString(text, 1);
    int    fontNr  = -1;

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

    #  ifdef P073_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String info = F("P037 7dfont,");
      info += fontArg;
      info += F(" -> ");
      info += fontNr;
      addLogMove(LOG_LEVEL_INFO, info);
    }
    #  endif // P073_DEBUG

    if ((fontNr >= 0) && (fontNr <= 3)) {
      P073_data->fontset = fontNr;
      PCONFIG(4)         = fontNr;
      return true;
    }
  }
  return false;
}

# endif // P073_EXTRA_FONTS

# ifdef P073_7DBIN_COMMAND
bool p073_plugin_write_7dbin(struct EventStruct *event,
                             const String      & text) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return false;
  }

  if (!text.isEmpty()) {
    String data;
    int    byteValue;
    int    arg      = 1;
    String argValue = parseString(text, arg);

    while (!argValue.isEmpty()) {
      if (validIntFromString(argValue, byteValue) && (byteValue < 256) && (byteValue > -1)) {
        data += static_cast<char>(P073_data->displayModel == P073_MAX7219_8DGT ?
                                  byteValue :
                                  P073_data->mapMAX7219FontToTM1673Font(byteValue));
      }
      arg++;
      argValue = parseString(text, arg);
    }
    #  ifdef P073_SCROLL_TEXT
    const uint8_t bufLen = P073_data->getBufferLength(P073_data->displayModel);
    #  endif // P073_SCROLL_TEXT

    if (!data.isEmpty()) {
      #  ifdef P073_SCROLL_TEXT
      P073_data->setTextToScroll(EMPTY_STRING); // Clear any scrolling text

      if (P073_data->isScrollEnabled() && (data.length() > bufLen)) {
        P073_data->setBinaryData(data);
      } else
      #  endif // P073_SCROLL_TEXT
      {
        P073_data->FillBufferWithString(data, true);

        switch (P073_data->displayModel) {
          case P073_TM1637_4DGTCOLON:
          case P073_TM1637_4DGTDOTS:
          {
            tm1637_ShowBuffer(event, 0, 4);
            break;
          }
          case P073_TM1637_6DGT:
          {
            tm1637_SwapDigitInBuffer(event, 0); // only needed for 6-digits displays
            tm1637_ShowBuffer(event, 0, 6, true);
            break;
          }
          case P073_MAX7219_8DGT:
          {
            P073_data->dotpos = -1; // avoid to display the dot
            max7219_ShowBuffer(event, P073_data->pin1, P073_data->pin2, P073_data->pin3);
            break;
          }
        }
      }
      return true;
    }
  }
  return false;
}

# endif // P073_7DBIN_COMMAND

// ===================================
// ---- TM1637 specific functions ----
// ===================================

# define CLK_HIGH() digitalWrite(clk_pin, HIGH)
# define CLK_LOW() digitalWrite(clk_pin, LOW)
# define DIO_HIGH() pinMode(dio_pin, INPUT)
# define DIO_LOW() pinMode(dio_pin, OUTPUT)

void tm1637_i2cStart(uint8_t clk_pin,
                     uint8_t dio_pin) {
  # ifdef P073_DEBUG
  addLog(LOG_LEVEL_DEBUG, F("7DGT : Comm Start"));
  # endif // ifdef P073_DEBUG
  DIO_HIGH();
  CLK_HIGH();
  delayMicroseconds(TM1637_CLOCKDELAY);
  DIO_LOW();
}

void tm1637_i2cStop(uint8_t clk_pin,
                    uint8_t dio_pin) {
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

void tm1637_i2cAck(uint8_t clk_pin,
                   uint8_t dio_pin) {
  # ifdef P073_DEBUG
  bool dummyAck = false;
  # endif // ifdef P073_DEBUG

  CLK_LOW();
  pinMode(dio_pin, INPUT_PULLUP);

  // DIO_HIGH();
  delayMicroseconds(TM1637_CLOCKDELAY);

  // while(digitalRead(dio_pin));
  # ifdef P073_DEBUG
  dummyAck =
  # endif // ifdef P073_DEBUG
  digitalRead(dio_pin);

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
  pinMode(dio_pin, OUTPUT);
}

void tm1637_i2cWrite_ack(uint8_t clk_pin,
                         uint8_t dio_pin,
                         uint8_t bytesToPrint[],
                         uint8_t length) {
  tm1637_i2cStart(clk_pin, dio_pin);

  for (uint8_t i = 0; i < length; ++i) {
    tm1637_i2cWrite_ack(clk_pin, dio_pin, bytesToPrint[i]);
  }
  tm1637_i2cStop(clk_pin, dio_pin);
}

void tm1637_i2cWrite_ack(uint8_t clk_pin,
                         uint8_t dio_pin,
                         uint8_t bytetoprint) {
  tm1637_i2cWrite(clk_pin, dio_pin, bytetoprint);
  tm1637_i2cAck(clk_pin, dio_pin);
}

void tm1637_i2cWrite(uint8_t clk_pin,
                     uint8_t dio_pin,
                     uint8_t bytetoprint) {
  # ifdef P073_DEBUG
  addLog(LOG_LEVEL_DEBUG, F("7DGT : WriteByte"));
  # endif // ifdef P073_DEBUG
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

void tm1637_ClearDisplay(uint8_t clk_pin,
                         uint8_t dio_pin) {
  uint8_t bytesToPrint[7] = { 0 };

  bytesToPrint[0] = 0xC0;
  tm1637_i2cWrite_ack(clk_pin, dio_pin, bytesToPrint, 7);
}

void tm1637_SetPowerBrightness(uint8_t clk_pin,
                               uint8_t dio_pin,
                               uint8_t brightlvl,
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

  uint8_t bytesToPrint[1] = { 0 };
  bytesToPrint[0] = brightvalue;
  tm1637_i2cWrite_ack(clk_pin, dio_pin, bytesToPrint, 1);
}

void tm1637_InitDisplay(uint8_t clk_pin,
                        uint8_t dio_pin) {
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

uint8_t tm1637_separator(uint8_t value,
                         bool    sep) {
  if (sep) {
    value |= 0b10000000;
  }
  return value;
}

void tm1637_ShowTime6(struct EventStruct *event) {
  tm1637_ShowDate6(event, true); // deduplicated
}

void tm1637_ShowDate6(struct EventStruct *event, bool showTime) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }
  uint8_t bytesToPrint[7] = { 0 };

  bytesToPrint[0] = 0xC0;
  bytesToPrint[1] = P073_data->tm1637_getFontChar(P073_data->showbuffer[2], P073_data->fontset);
  bytesToPrint[2] = tm1637_separator(P073_data->tm1637_getFontChar(P073_data->showbuffer[1], P073_data->fontset), P073_data->timesep);
  bytesToPrint[3] = P073_data->tm1637_getFontChar(P073_data->showbuffer[0], P073_data->fontset);

  if (showTime) {
    bytesToPrint[4] = P073_data->tm1637_getFontChar(P073_data->showbuffer[5], P073_data->fontset);
    bytesToPrint[5] = P073_data->tm1637_getFontChar(P073_data->showbuffer[4], P073_data->fontset);
  } else {
    bytesToPrint[4] = P073_data->tm1637_getFontChar(P073_data->showbuffer[7], P073_data->fontset);
    bytesToPrint[5] = P073_data->tm1637_getFontChar(P073_data->showbuffer[6], P073_data->fontset);
  }
  bytesToPrint[6] = tm1637_separator(P073_data->tm1637_getFontChar(P073_data->showbuffer[3], P073_data->fontset), P073_data->timesep);

  tm1637_i2cWrite_ack(P073_data->pin1, P073_data->pin2, bytesToPrint, 7);
}

void tm1637_ShowTemp6(struct EventStruct *event,
                      bool                sep) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }
  uint8_t bytesToPrint[7] = { 0 };

  bytesToPrint[0] = 0xC0;
  bytesToPrint[1] = tm1637_separator(P073_data->tm1637_getFontChar(P073_data->showbuffer[5], P073_data->fontset), sep);
  bytesToPrint[2] = P073_data->tm1637_getFontChar(P073_data->showbuffer[4], P073_data->fontset);
  bytesToPrint[3] = P073_data->tm1637_getFontChar(10, P073_data->fontset);
  bytesToPrint[4] = P073_data->tm1637_getFontChar(10, P073_data->fontset);
  bytesToPrint[5] = P073_data->tm1637_getFontChar(P073_data->showbuffer[7], P073_data->fontset);
  bytesToPrint[6] = P073_data->tm1637_getFontChar(P073_data->showbuffer[6], P073_data->fontset);

  tm1637_i2cWrite_ack(P073_data->pin1, P073_data->pin2, bytesToPrint, 7);
}

void tm1637_ShowTimeTemp4(struct EventStruct *event,
                          bool                sep,
                          uint8_t             bufoffset) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }
  uint8_t bytesToPrint[7] = { 0 };

  bytesToPrint[0] = 0xC0;
  bytesToPrint[1] = P073_data->tm1637_getFontChar(P073_data->showbuffer[0 + bufoffset], P073_data->fontset);
  bytesToPrint[2] = tm1637_separator(P073_data->tm1637_getFontChar(P073_data->showbuffer[1 + bufoffset], P073_data->fontset), sep);
  bytesToPrint[3] = P073_data->tm1637_getFontChar(P073_data->showbuffer[2 + bufoffset], P073_data->fontset);
  bytesToPrint[4] = P073_data->tm1637_getFontChar(P073_data->showbuffer[3 + bufoffset], P073_data->fontset);

  tm1637_i2cWrite_ack(P073_data->pin1, P073_data->pin2, bytesToPrint, 5);
}

void tm1637_SwapDigitInBuffer(struct EventStruct *event,
                              uint8_t             startPos) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }
  uint8_t p073_tmp; // Swap digits

  p073_tmp                            = P073_data->showbuffer[2 + startPos];
  P073_data->showbuffer[2 + startPos] = P073_data->showbuffer[0 + startPos];
  P073_data->showbuffer[0 + startPos] = p073_tmp;
  p073_tmp                            = P073_data->showbuffer[3 + startPos];
  P073_data->showbuffer[3 + startPos] = P073_data->showbuffer[5 + startPos];
  P073_data->showbuffer[5 + startPos] = p073_tmp;

  bool p073_per; // Swap periods

  p073_per                             = P073_data->showperiods[2 + startPos];
  P073_data->showperiods[2 + startPos] = P073_data->showperiods[0 + startPos];
  P073_data->showperiods[0 + startPos] = p073_per;
  p073_per                             = P073_data->showperiods[3 + startPos];
  P073_data->showperiods[3 + startPos] = P073_data->showperiods[5 + startPos];
  P073_data->showperiods[5 + startPos] = p073_per;

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

void tm1637_ShowBuffer(struct EventStruct *event,
                       uint8_t             firstPos,
                       uint8_t             lastPos,
                       bool                useBinaryData) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }
  uint8_t bytesToPrint[8] = { 0 };

  bytesToPrint[0] = 0xC0;
  uint8_t length = 1;

  if (P073_data->dotpos > -1) {
    P073_data->showperiods[P073_data->dotpos] = true;
  }

  uint8_t p073_datashowpos1;

  for (int i = firstPos; i < lastPos; i++) {
    if (useBinaryData) {
      bytesToPrint[length] = P073_data->showbuffer[i];
    } else {
      p073_datashowpos1 = tm1637_separator(
        P073_data->tm1637_getFontChar(P073_data->showbuffer[i], P073_data->fontset),
        P073_data->showperiods[i]);
      bytesToPrint[length] = p073_datashowpos1;
    }
    ++length;
  }
  tm1637_i2cWrite_ack(P073_data->pin1, P073_data->pin2, bytesToPrint, length);
}

// ====================================
// ---- MAX7219 specific functions ----
// ====================================

# define OP_DECODEMODE   9
# define OP_INTENSITY   10
# define OP_SCANLIMIT   11
# define OP_SHUTDOWN    12
# define OP_DISPLAYTEST 15

void max7219_spiTransfer(struct EventStruct *event,
                         uint8_t             din_pin,
                         uint8_t             clk_pin,
                         uint8_t             cs_pin,
                         volatile uint8_t    opcode,
                         volatile uint8_t    data) {
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

void max7219_ClearDisplay(struct EventStruct *event,
                          uint8_t             din_pin,
                          uint8_t             clk_pin,
                          uint8_t             cs_pin) {
  for (int i = 0; i < 8; i++) {
    max7219_spiTransfer(event, din_pin, clk_pin, cs_pin, i + 1, 0);
  }
}

void max7219_SetPowerBrightness(struct EventStruct *event,
                                uint8_t             din_pin,
                                uint8_t             clk_pin,
                                uint8_t             cs_pin,
                                uint8_t             brightlvl,
                                bool                poweron) {
  max7219_spiTransfer(event, din_pin, clk_pin, cs_pin, OP_INTENSITY, brightlvl);
  max7219_spiTransfer(event, din_pin, clk_pin, cs_pin, OP_SHUTDOWN,  poweron ? 1 : 0);
}

void max7219_SetDigit(struct EventStruct *event,
                      uint8_t             din_pin,
                      uint8_t             clk_pin,
                      uint8_t             cs_pin,
                      int                 dgtpos,
                      uint8_t             dgtvalue,
                      bool                showdot,
                      bool                binaryData = false) {
  uint8_t p073_tempvalue;

  # ifdef P073_EXTRA_FONTS

  switch (PCONFIG(4)) {
    case 1:  // Siekoo
    case 2:  // Siekoo with uppercase CHNORUX
      p073_tempvalue = pgm_read_byte(&(SiekooCharTable[dgtvalue]));
      break;
    case 3:  // dSEG7
      p073_tempvalue = pgm_read_byte(&(Dseg7CharTable[dgtvalue]));
      break;
    default: // Default fontset
  # endif // P073_EXTRA_FONTS
  p073_tempvalue = pgm_read_byte(&(DefaultCharTable[dgtvalue]));
  # ifdef P073_EXTRA_FONTS
}

  # endif // P073_EXTRA_FONTS

  if (showdot) {
    p073_tempvalue |= 0b10000000;
  }

  if (binaryData) {
    p073_tempvalue = dgtvalue; // Overwrite if binary data
  }
  max7219_spiTransfer(event, din_pin, clk_pin, cs_pin, dgtpos + 1, p073_tempvalue);
}

void max7219_InitDisplay(struct EventStruct *event,
                         uint8_t             din_pin,
                         uint8_t             clk_pin,
                         uint8_t             cs_pin) {
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

void max7219_ShowTime(struct EventStruct *event,
                      uint8_t             din_pin,
                      uint8_t             clk_pin,
                      uint8_t             cs_pin,
                      bool                sep) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }

  const uint8_t idx_list[] = { 7, 6, 4, 3, 1, 0 }; // Digits in reversed order, as the loop is backward

  for (int8_t i = 5; i >= 0; i--) {
    max7219_SetDigit(event, din_pin, clk_pin, cs_pin, idx_list[i], P073_data->showbuffer[i], false);
  }

  const uint8_t sepChar = P073_data->mapCharToFontPosition(sep ? '-' : ' ', P073_data->fontset);

  max7219_SetDigit(event, din_pin, clk_pin, cs_pin, 2, sepChar, false);
  max7219_SetDigit(event, din_pin, clk_pin, cs_pin, 5, sepChar, false);
}

void max7219_ShowTemp(struct EventStruct *event,
                      uint8_t             din_pin,
                      uint8_t             clk_pin,
                      uint8_t             cs_pin,
                      int8_t              firstDot,
                      int8_t              secondDot) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }

  max7219_SetDigit(event, din_pin, clk_pin, cs_pin, 0, 10, false);

  if (firstDot  > -1) { P073_data->showperiods[firstDot] = true; }

  if (secondDot > -1) { P073_data->showperiods[secondDot] = true; }

  const int alignRight = P073_data->rightAlignTempMAX7219 ? 0 : 1;

  for (int i = alignRight; i < 8; i++) {
    const int bufIndex = (7 + alignRight) - i;

    if (bufIndex < 8) {
      max7219_SetDigit(event, din_pin, clk_pin, cs_pin, i,
                       P073_data->showbuffer[bufIndex],
                       P073_data->showperiods[bufIndex]);
    }
  }
}

void max7219_ShowDate(struct EventStruct *event,
                      uint8_t             din_pin,
                      uint8_t             clk_pin,
                      uint8_t             cs_pin) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }

  const uint8_t dotflags[8] = { false, true, false, true, false, false, false, false };

  for (int i = 0; i < 8; i++) {
    max7219_SetDigit(event, din_pin, clk_pin, cs_pin, i,
                     P073_data->showbuffer[7 - i],
                     dotflags[7 - i]);
  }
}

void max7219_ShowBuffer(struct EventStruct *event,
                        uint8_t             din_pin,
                        uint8_t             clk_pin,
                        uint8_t             cs_pin) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }

  if (P073_data->dotpos > -1) {
    P073_data->showperiods[P073_data->dotpos] = true;
  }

  for (int i = 0; i < 8; i++) {
    max7219_SetDigit(event, din_pin, clk_pin, cs_pin, i,
                     P073_data->showbuffer[7 - i],
                     P073_data->showperiods[7 - i]
                     # ifdef P073_7DBIN_COMMAND
                     , P073_data->binaryData
                     # endif // P073_7DBIN_COMMAND
                     );
  }
}

#endif // USES_P073
