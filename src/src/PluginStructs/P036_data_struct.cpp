#include "../PluginStructs/P036_data_struct.h"

#ifdef USES_P036

# include "../ESPEasyCore/ESPEasyNetwork.h"
# include "../Globals/RTC.h"
# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/Memory.h"
# include "../Helpers/Misc.h"
# include "../Helpers/Scheduler.h"
# include "../Helpers/StringConverter.h"
# include "../Helpers/StringParser.h"
# include "../Helpers/SystemVariables.h"

# ifndef P036_LIMIT_BUILD_SIZE
#  include <Dialog_Plain_12_font.h>
#  include <Dialog_Plain_18_font.h>
# endif // ifndef P036_LIMIT_BUILD_SIZE
# include <OLED_SSD1306_SH1106_images.h>

void P036_LineContent::loadDisplayLines(taskIndex_t taskIndex, uint8_t LoadVersion) {
  if (LoadVersion == 0) {
    // read data of version 0 (up to 22.11.2019)
    String DisplayLinesV0[P36_Nlines];                                           // used to load the CustomTaskSettings for V0
    LoadCustomTaskSettings(taskIndex, DisplayLinesV0, P36_Nlines, P36_NcharsV0); // max. length 1024 Byte  (DAT_TASKS_CUSTOM_SIZE)

    for (int i = 0; i < P36_Nlines; ++i) {
      DisplayLinesV1[i].Content      = std::move(DisplayLinesV0[i]);
      DisplayLinesV1[i].FontType     = 0xff;
      DisplayLinesV1[i].ModifyLayout = 0xff;
      DisplayLinesV1[i].FontSpace    = 0xff;
      DisplayLinesV1[i].reserved     = 0xff;
    }
  } else {
    // read data of version 1 (beginning from 22.11.2019)
    for (int i = 0; i < P36_Nlines; ++i) {
      tDisplayLines_storage tmp;
      LoadCustomTaskSettings(
        taskIndex,
        reinterpret_cast<uint8_t *>(&tmp),
        sizeof(tDisplayLines_storage),
        i * sizeof(tDisplayLines_storage));
      DisplayLinesV1[i] = tmp.get();
    }
  }
}

String P036_LineContent::saveDisplayLines(taskIndex_t taskIndex) {
  String error;

  if (FreeMem() > 8000) {
    // Write in one single chunk.
    tDisplayLines_storage_full *tmp = nullptr;
    # ifdef USE_SECOND_HEAP
    {
      HeapSelectIram ephemeral;
      tmp = new (std::nothrow) tDisplayLines_storage_full;
    }
    # endif // ifdef USE_SECOND_HEAP

    if (tmp == nullptr) {
      tmp = new (std::nothrow) tDisplayLines_storage_full;
    }

    if (tmp != nullptr) {
      for (int i = 0; i < P36_Nlines; ++i) {
        safe_strncpy(tmp->lines[i].Content, DisplayLinesV1[i].Content, P36_NcharsV1);
        tmp->lines[i].FontType     = DisplayLinesV1[i].FontType;
        tmp->lines[i].ModifyLayout = DisplayLinesV1[i].ModifyLayout;
        tmp->lines[i].FontSpace    = DisplayLinesV1[i].FontSpace;
        tmp->lines[i].reserved     = DisplayLinesV1[i].reserved;
      }
      error = SaveCustomTaskSettings(
        taskIndex,
        reinterpret_cast<uint8_t *>(tmp),
        sizeof(tDisplayLines_storage_full));
      delete tmp;
      return error;
    }
  }

  // Since we're making several calls to save, make sure to consider this as a single save call.
  const uint8_t flashCounter = RTC.flashDayCounter;

  for (int i = 0; i < P36_Nlines && error.length() == 0; ++i) {
    tDisplayLines_storage tmp(DisplayLinesV1[i]);
    RTC.flashDayCounter = flashCounter;
    error               = SaveCustomTaskSettings(
      taskIndex,
      reinterpret_cast<uint8_t *>(&tmp),
      sizeof(tDisplayLines_storage),
      i * sizeof(tDisplayLines_storage));
  }
  return error;
}

P036_data_struct::~P036_data_struct() {
  if (display != nullptr) {
    display->displayOff();
    display->end();
    delete display;
    display = nullptr;
  }

  if (LineContent != nullptr) {
    delete LineContent;
    LineContent = nullptr;
  }
}

void P036_data_struct::reset() {
  if (display != nullptr) {
    display->displayOff();
    display->end();
    delete display;
    display = nullptr;
  }

  if (LineContent != nullptr) {
    delete LineContent;
    LineContent = nullptr;
  }
}

# ifdef P036_FONT_CALC_LOG
const __FlashStringHelper * tFontSettings::FontName() const {
  switch (fontIdx) {
    case 0: return F("Arial_24"); break;
    #  ifndef P036_LIMIT_BUILD_SIZE
    case 1: return F("Dialog_18"); break;
    case 2: return F("Arial_16"); break;
    case 3: return F("Dialog_12"); break;
    case 4: return F("Arial_10"); break;
    #  else // ifndef P036_LIMIT_BUILD_SIZE
    case 1: return F("Arial_16"); break;
    case 2: return F("Arial_10"); break;
    #  endif // ifndef P036_LIMIT_BUILD_SIZE
    default: return F("Unknown font");
  }
}

# endif // ifdef P036_FONT_CALC_LOG

// FIXME TD-er: with using functions to get the font, this object is stored in .dram0.data
// The same as when using the DRAM_ATTR attribute used for interrupt code.
// This is very precious memory, so we must find something other way to define this.
const tFontSizes FontSizes[P36_MaxFontCount] = {
  { getArialMT_Plain_24(), 24,  28                         }, // 9643
# ifndef P036_LIMIT_BUILD_SIZE
  { getDialog_plain_18(),  19,  22                         }, // 7399
# endif // ifndef P036_LIMIT_BUILD_SIZE
  { getArialMT_Plain_16(), 16,  19                         }, // 5049
# ifndef P036_LIMIT_BUILD_SIZE
  { getDialog_plain_12(),  13,  15                         }, // 3707
# endif // ifndef P036_LIMIT_BUILD_SIZE
  { getArialMT_Plain_10(), 10,  13                         }, // 2731
};

const tSizeSettings SizeSettings[P36_MaxSizesCount] = {
  { P36_MaxDisplayWidth, P36_MaxDisplayHeight, 0,  // 128x64
    4,                                             // max. line count
    113, 15                                        // WiFi indicator
  },
  { P36_MaxDisplayWidth, 32,                   0,  // 128x32
    2,                                             // max. line count
    113, 15                                        // WiFi indicator
  },
  { 64,                  48,                   32, // 64x48
    3,                                             // max. line count
    32,  10                                        // WiFi indicator
  }
};


const tSizeSettings& P036_data_struct::getDisplaySizeSettings(p036_resolution disp_resolution) {
  int index = static_cast<int>(disp_resolution);

  if ((index < 0) || (index >= P36_MaxSizesCount)) { index = 0; }

  return SizeSettings[index];
}

bool P036_data_struct::init(taskIndex_t      taskIndex,
                            uint8_t          LoadVersion,
                            uint8_t          Type,
                            uint8_t          Address,
                            uint8_t          Sda,
                            uint8_t          Scl,
                            p036_resolution  Disp_resolution,
                            bool             Rotated,
                            uint8_t          Contrast,
                            uint16_t         DisplayTimer,
                            ePageScrollSpeed ScrollSpeed,
                            uint8_t          NrLines) {
  reset();

  lastWiFiState       = P36_WIFI_STATE_UNSET;
  disp_resolution     = Disp_resolution;
  bAlternativHeader   = false; // start with first header content
  HeaderCount         = 0;
  bPageScrollDisabled = true;  // first page after INIT without scrolling
  TopLineOffset       = 0;     // Offset for top line, used for rotated image while using displays < P36_MaxDisplayHeight lines

  HeaderContent            = eHeaderContent::eSysName;
  HeaderContentAlternative = eHeaderContent::eSysName;
  MaxFramesToDisplay       = 0xFF;
  currentFrameToDisplay    = 0;
  nextFrameToDisplay       = 0xFF; // next frame because content changed in PLUGIN_WRITE

  ButtonState     = false;         // button not touched
  ButtonLastState = 0xFF;          // Last state checked (debouncing in progress)
  DebounceCounter = 0;             // debounce counter
  RepeatCounter   = 0;             // Repeat delay counter when holding button pressed

  displayTimer          = DisplayTimer;
  frameCounter          = 0;       // need to keep track of framecounter from call to call
  disableFrameChangeCnt = 0;       // counter to disable frame change after JumpToPage in case PLUGIN_READ already scheduled

  switch (Type) {
    case 1:
      display = new (std::nothrow) SSD1306Wire(Address, Sda, Scl);
      break;
    case 2:
      display = new (std::nothrow) SH1106Wire(Address, Sda, Scl);
      break;
    default:
      return false;
  }

  {
    # ifdef USE_SECOND_HEAP
    HeapSelectIram ephemeral;
    # endif // ifdef USE_SECOND_HEAP
    LineContent = new (std::nothrow) P036_LineContent();
  }

  if (isInitialized()) {
    display->init(); // call to local override of init function

    // disp_resolution = Disp_resolution;
    bHideFooter |= !(getDisplaySizeSettings(disp_resolution).Height == P36_MaxDisplayHeight);

    if (disp_resolution == p036_resolution::pix128x32) {
      display->displayOff();
      display->SetComPins(0x02); // according to the adafruit lib, sometimes this may need to be 0x02
      bHideFooter = true;
    }

    display->displayOn();
    LineContent->loadDisplayLines(taskIndex, LoadVersion);

    // Flip screen if required
    setOrientationRotated(Rotated);

    setContrast(Contrast);

    //      Display the device name, logo, time and wifi
    display_logo();
    update_display();

    //    Initialize frame counter
    frameCounter                    = 0;
    currentFrameToDisplay           = 0;
    nextFrameToDisplay              = 0;
    bPageScrollDisabled             = true;  // first page after INIT without scrolling
    ScrollingPages.linesPerFrameDef = NrLines;
    bLineScrollEnabled              = false; // start without line scrolling

    //    Clear scrolling line data
    for (uint8_t i = 0; i < P36_MAX_LinesPerPage; i++) {
      ScrollingLines.SLine[i].Width     = 0;
      ScrollingLines.SLine[i].LastWidth = 0;
    }

    //    prepare font and positions for page and line scrolling
    prepare_pagescrolling(ScrollSpeed, NrLines);
  }

  bRunning = NetworkConnected();

  return isInitialized();
}

const char p036_subcommands[] PROGMEM = "display|frame"
# if P036_ENABLE_LINECOUNT
"|linecount"
#endif
"|restore|scroll"
# if P036_ENABLE_LEFT_ALIGN
"|leftalign|align"
#endif
# if P036_USERDEF_HEADERS
"|userdef1|userdef2"
#endif
;
enum class p036_subcommands_e {
  display,
  frame,
# if P036_ENABLE_LINECOUNT
  linecount,
#endif
  restore,
  scroll,
# if P036_ENABLE_LEFT_ALIGN
  leftalign,
  align,
#endif
# if P036_USERDEF_HEADERS
  userdef1,
  userdef2
#endif
};


bool P036_data_struct::plugin_write(struct EventStruct *event, const String& string)
{
  const String command = parseString(string, 1);

  if (!(equals(command, F("oledframedcmd"))) || !isInitialized()) {
    return false;
  }
  bool success = false;

  bool bUpdateDisplay = false;
  bool bDisplayON     = false;
  uint8_t eventId     = 0;

  const String subcommand = parseString(string, 2);
  int LineNo              = event->Par1;

      # if P036_SEND_EVENTS
  const bool sendEvents = bitRead(P036_FLAGS_0, P036_FLAG_SEND_EVENTS); // Bit 28 Send Events
      # endif // if P036_SEND_EVENTS

  int  command_i = GetCommandCode(subcommand.c_str(), p036_subcommands);

  if (command_i == -1) {
    if ((LineNo > 0) && (LineNo <= P36_Nlines)) {
      // content functions
      success = true;
      String *currentLine = &LineContent->DisplayLinesV1[LineNo - 1].Content;
      *currentLine = parseStringKeepCaseNoTrim(string, 3);
      *currentLine = P36_parseTemplate(*currentLine, LineNo - 1);

          # if P036_ENABLE_TICKER

      if (!bUseTicker)
          # endif // if P036_ENABLE_TICKER
      {
        // calculate Pix length of new content, not necessary for ticker
        uint16_t PixLength = CalcPixLength(LineNo - 1);

        if (PixLength > 255) {
          addHtmlError(strformat(F("Pixel length of %d too long for line! Max. 255 pix!"), PixLength));

          const unsigned int strlen = currentLine->length();

          if (strlen > 0) {
            const float fAvgPixPerChar       = static_cast<float>(PixLength) / strlen;
            const unsigned int iCharToRemove = ceilf((static_cast<float>(PixLength - 255)) / fAvgPixPerChar);

            // shorten string because OLED controller can not handle such long strings
            *currentLine = currentLine->substring(0, strlen - iCharToRemove);
          }
        }
      }
      eventId        = P036_EVENT_LINE;
      bUpdateDisplay = true;
    }
  } else {
    switch (static_cast<p036_subcommands_e>(command_i)) {
      case p036_subcommands_e::display: {
        // display functions
        const String para1 = parseString(string, 3);

        if (equals(para1, F("on"))) {
          success      = true;
          displayTimer = P036_TIMER;
          display->displayOn();

          P036_SetDisplayOn(1); //  Save the fact that the display is now ON
            # if P036_SEND_EVENTS

          if (sendEvents) {
            P036_SendEvent(event, P036_EVENT_DISPLAY, 1);
          }
            # endif // if P036_SEND_EVENTS
        }

        else if (equals(para1, F("off"))) {
          success      = true;
          displayTimer = 0;
          display->displayOff();

          P036_SetDisplayOn(0); //  Save the fact that the display is now OFF
            # if P036_SEND_EVENTS

          if (sendEvents) {
            P036_SendEvent(event, P036_EVENT_DISPLAY, 0);
          }
            # endif // if P036_SEND_EVENTS
        }

        else if (equals(para1, F("low"))) {
          success = true;
          setContrast(OLED_CONTRAST_LOW);
          LineNo     = 0; // is event parameter
          eventId    = P036_EVENT_CONTRAST;
          bDisplayON = true;
        }

        else if (equals(para1, F("med"))) {
          success = true;
          setContrast(OLED_CONTRAST_MED);
          LineNo     = 1; // is event parameter
          eventId    = P036_EVENT_CONTRAST;
          bDisplayON = true;
        }

        else if (equals(para1, F("high"))) {
          success = true;
          setContrast(OLED_CONTRAST_HIGH);
          LineNo     = 2; // is event parameter
          eventId    = P036_EVENT_CONTRAST;
          bDisplayON = true;
        }

        else if (equals(para1, F("user")) &&
                 (event->Par3 >= 1) && (event->Par3 <= 255) && // contrast
                 (event->Par4 >= 0) && (event->Par4 <= 255) && // precharge
                 (event->Par5 >= 0) && (event->Par5 <= 255))   // comdetect
        {
          success = true;
          display->setContrast(static_cast<uint8_t>(event->Par3), static_cast<uint8_t>(event->Par4),
                               static_cast<uint8_t>(event->Par5));
          LineNo     = 3; // is event parameter
          eventId    = P036_EVENT_CONTRAST;
          bDisplayON = true;
        }
        break;
      }
      case p036_subcommands_e::frame:
      {
        if ((event->Par2 >= 0) &&
            (event->Par2 <= MaxFramesToDisplay + 1)) {
          success = true;

          if (!P036_DisplayIsOn) {
            // display was OFF, turn it ON
            display->displayOn();
            P036_SetDisplayOn(1); //  Save the fact that the display is now ON
            # if P036_SEND_EVENTS

            if (sendEvents) {
              P036_SendEvent(event, P036_EVENT_DISPLAY, 1);
            }
            # endif // if P036_SEND_EVENTS
          }
          uint8_t nextFrame = (event->Par2 == 0 ? 0xFF : event->Par2 - 1);
          P036_JumpToPage(event, nextFrame);                           //  Start to display the selected page, function needs
          // 65ms!
          # if P036_SEND_EVENTS

          if (sendEvents && bitRead(P036_FLAGS_0, P036_FLAG_EVENTS_FRAME_LINE)) { // Bit 29 Send Events Frame & Line
            P036_SendEvent(event, P036_EVENT_FRAME, currentFrameToDisplay + 1);
          }
          # endif // if P036_SEND_EVENTS
        }
        break;
      }
# if P036_ENABLE_LINECOUNT
      case p036_subcommands_e::linecount:

        if ((event->Par2 >= 1) &&
            (event->Par2 <= 4)) {
          #  if P036_ENABLE_TICKER

          if (static_cast<ePageScrollSpeed>(P036_SCROLL) == ePageScrollSpeed::ePSS_Ticker) {
            // Ticker supports only 1 line, can not be changed
            success = (event->Par2 == 1);
            return success;
          }
          #  endif // if P036_ENABLE_TICKER
          success = true;

          if (P036_NLINES != event->Par2) {
            P036_NLINES = event->Par2;
            setNrLines(event, P036_NLINES);
            #  if P036_SEND_EVENTS

            if (sendEvents && bitRead(P036_FLAGS_0, P036_FLAG_EVENTS_FRAME_LINE)) { // Bit 29 Send Events Frame & Line
              P036_SendEvent(event, P036_EVENT_LINECNT, P036_NLINES);
            }
            #  endif // if P036_SEND_EVENTS
          }
        }
        break;
# endif // if P036_ENABLE_LINECOUNT
      case p036_subcommands_e::restore:

        if ((event->Par2 >= 0) &&            // 0: restore all line contents
            (event->Par2 <= P36_Nlines)) {
          // restore content functions
          success = true;
          LineNo  = event->Par2;
          RestoreLineContent(event->TaskIndex,
                             get4BitFromUL(P036_FLAGS_0, P036_FLAG_SETTINGS_VERSION), // Bit23-20 Version CustomTaskSettings
                             LineNo);

          if (LineNo == 0) {
            LineNo = 1; // after restoring all contents start with first Line
          }
          eventId        = P036_EVENT_RESTORE;
          bUpdateDisplay = true;
        }
        break;
      case p036_subcommands_e::scroll:

        if (event->Par2 >= 1) {
          // set scroll
          success = true;

          switch (event->Par2) {
            case 1: P036_SCROLL = static_cast<int16_t>(ePageScrollSpeed::ePSS_VerySlow); break;
            case 2: P036_SCROLL = static_cast<int16_t>(ePageScrollSpeed::ePSS_Slow); break;
            case 3: P036_SCROLL = static_cast<int16_t>(ePageScrollSpeed::ePSS_Fast); break;
            case 4: P036_SCROLL = static_cast<int16_t>(ePageScrollSpeed::ePSS_VeryFast); break;
            case 5: P036_SCROLL = static_cast<int16_t>(ePageScrollSpeed::ePSS_Instant); break;
# if P036_ENABLE_TICKER
            case 6: P036_SCROLL = static_cast<int16_t>(ePageScrollSpeed::ePSS_Ticker); break;
# endif // if P036_ENABLE_TICKER
            default:
              success = false;
              break;
          }

          if (success) {
            prepare_pagescrolling(static_cast<ePageScrollSpeed>(P036_SCROLL), P036_NLINES);
            eventId        = P036_EVENT_SCROLL;
            LineNo         = 1; // after change scroll start with first Line
            bUpdateDisplay = true;
          }
        }
        break;
# if P036_ENABLE_LEFT_ALIGN
      case p036_subcommands_e::leftalign:

        if ((event->Par2 == 0) ||
            (event->Par2 == 1)) {
          success = true;
          eAlignment aAlignment = (event->Par2 == 1 ? eAlignment::eLeft : eAlignment::eCenter);
          setTextAlignment(aAlignment);
          uint32_t lSettings = P036_FLAGS_1;
          set2BitToUL(lSettings, P036_FLAG_LEFT_ALIGNED, static_cast<uint8_t>(aAlignment)); // Alignment
          P036_FLAGS_1 = lSettings;
        }
        break;
      case p036_subcommands_e::align:

        if ((event->Par2 >= 0) &&
            (event->Par2 <= 2)) {
          success = true;
          const eAlignment aAlignment = static_cast<eAlignment>(event->Par2);

          setTextAlignment(aAlignment);
          uint32_t lSettings = P036_FLAGS_1;
          set2BitToUL(lSettings, P036_FLAG_LEFT_ALIGNED, static_cast<uint8_t>(aAlignment)); // Alignment
          P036_FLAGS_1 = lSettings;
        }
        break;
# endif // if P036_ENABLE_LEFT_ALIGN
# if P036_USERDEF_HEADERS

      case p036_subcommands_e::userdef1:
      {
        userDef1 = parseStringKeepCase(string, 3);
        userDef1.replace('$', '%'); // Allow system vars to be passed in by using $ instead of %
        success = true;
        break;
      }

      case p036_subcommands_e::userdef2:
      {
        userDef2 = parseStringKeepCase(string, 3);
        userDef2.replace('$', '%'); // Allow system vars to be passed in by using $ instead of %
        success = true;
        break;
      }
# endif // if P036_USERDEF_HEADERS
    }
  }


  if (success && (eventId > 0)) {
    if (bDisplayON) {
          # if P036_SEND_EVENTS

      if (sendEvents) {
        P036_SendEvent(event, eventId, LineNo);

        if (!P036_DisplayIsOn) {
          P036_SendEvent(event, P036_EVENT_DISPLAY, 1);
        }
      }
          # endif // if P036_SEND_EVENTS
      P036_SetDisplayOn(1);     //  Save the fact that the display is now ON
    }

    if (bUpdateDisplay) {
      MaxFramesToDisplay = 0xff; // update frame count

          # if P036_SEND_EVENTS
      const uint8_t currentFrame = currentFrameToDisplay;
          # endif // if P036_SEND_EVENTS

      if (!P036_DisplayIsOn &&
          (!bitRead(P036_FLAGS_0, P036_FLAG_NODISPLAY_ONRECEIVE) ||     // Bit 18 NoDisplayOnReceivedText
           (eventId == P036_EVENT_SCROLL))) {
        // display was OFF, turn it ON
        display->displayOn();
        P036_SetDisplayOn(1); //  Save the fact that the display is now ON
            # if P036_SEND_EVENTS

        if (sendEvents) {
          P036_SendEvent(event, P036_EVENT_DISPLAY, 1);

          if (bitRead(P036_FLAGS_0, P036_FLAG_EVENTS_FRAME_LINE)) { // Bit 29 Send Events Frame & Line
            P036_SendEvent(event, P036_EVENT_LINE, LineNo);
          }
        }
            # endif // if P036_SEND_EVENTS
      }

      if (P036_DisplayIsOn) {
        bLineScrollEnabled = false; // disable scrolling temporary
            # if P036_ENABLE_TICKER

        if (bUseTicker) {
          P036_JumpToPage(event, 0); // Restart the Ticker
        }
        else
            # endif // if P036_ENABLE_TICKER
        P036_JumpToPageOfLine(event, LineNo - 1); // Start to display the selected page, function needs 65ms!
            # if P036_SEND_EVENTS

        if (sendEvents && bitRead(P036_FLAGS_0, P036_FLAG_EVENTS_FRAME_LINE) && (currentFrame != currentFrameToDisplay)) {
          P036_SendEvent(event, P036_EVENT_FRAME, currentFrameToDisplay + 1);
        }
            # endif // if P036_SEND_EVENTS
      }

# ifdef PLUGIN_036_DEBUG

      if (eventId == P036_EVENT_LINE) {
        String log;

        if (loglevelActiveFor(LOG_LEVEL_INFO) &&
            log.reserve(200)) { // estimated
          log  = F("[P36] Line: ");
          log += LineNo;
          log += F(" Content:");
          log += LineContent->DisplayLinesV1[LineNo - 1].Content;
          log += F(" Length:");
          log += LineContent->DisplayLinesV1[LineNo - 1].Content.length();
          log += F(" Pix: ");
          log += display->getStringWidth(LineContent->DisplayLinesV1[LineNo - 1].Content);
          log += F(" Reserved:");
          log += LineContent->DisplayLinesV1[LineNo - 1].reserved;
          addLogMove(LOG_LEVEL_INFO, log);
          delay(5); // FIXME otherwise it is maybe too fast for the serial monitor
        }
      }
# endif // PLUGIN_036_DEBUG
    }
  }
# ifdef PLUGIN_036_DEBUG

  if (!success && loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = concat(F("[P36] Cmd: "), command);
    log += concat(F(" SubCmd:"), subcommand);
    log += F(" Success:false");
    addLogMove(LOG_LEVEL_INFO, log);
  }
# endif // PLUGIN_036_DEBUG
  return success;
}

bool P036_data_struct::isInitialized() const {
  return (display != nullptr) && (LineContent != nullptr);
}

void P036_data_struct::setContrast(uint8_t OLED_contrast) {
  OLedSetContrast(display, OLED_contrast);
}

void P036_data_struct::setOrientationRotated(bool rotated) {
  if (rotated) {
    display->flipScreenVertically();
    TopLineOffset = P36_MaxDisplayHeight - getDisplaySizeSettings(disp_resolution).Height;
  } else {
    TopLineOffset = 0;
  }
}

void P036_data_struct::RestoreLineContent(taskIndex_t taskIndex,
                                          uint8_t     LoadVersion,
                                          uint8_t     LineNo) {
  # ifdef USE_SECOND_HEAP
  HeapSelectIram ephemeral;
  # endif // ifdef USE_SECOND_HEAP

  P036_LineContent *TempContent = new (std::nothrow) P036_LineContent();

  if (TempContent != nullptr) {
    TempContent->loadDisplayLines(taskIndex, LoadVersion);

    if (LineNo == 0) {
      for (int i = 0; i < P36_Nlines; ++i) {
        *(&LineContent->DisplayLinesV1[i].Content) = TempContent->DisplayLinesV1[i].Content;
      }
    }
    else {
      *(&LineContent->DisplayLinesV1[LineNo - 1].Content) = TempContent->DisplayLinesV1[LineNo - 1].Content;
    }
    delete TempContent;
  }
}

# if P036_ENABLE_LINECOUNT
void P036_data_struct::setNrLines(struct EventStruct *event, uint8_t NrLines) {
  if ((NrLines >= 1) && (NrLines <= P36_MAX_LinesPerPage)) {
    prepare_pagescrolling(static_cast<ePageScrollSpeed>(P036_SCROLL), NrLines); // Recalculate font
    MaxFramesToDisplay = 0xFF;                                                  // Recalculate page indicator
    CalcMaxPageCount();                                                         // Update max page count
    nextFrameToDisplay = 0;                                                     // Reset to first page
  }
}

# endif // if P036_ENABLE_LINECOUNT


void P036_data_struct::display_header() {
  if (!isInitialized()) {
    return;
  }

  if (bHideHeader) { //  hide header
    return;
  }

  eHeaderContent iHeaderContent;
  String newString, strHeader;

  if ((HeaderContentAlternative == HeaderContent) || !bAlternativHeader) {
    iHeaderContent = HeaderContent;
  } else {
    iHeaderContent = HeaderContentAlternative;
  }

  switch (iHeaderContent) {
    case eHeaderContent::eSSID:

      if (NetworkConnected()) {
        strHeader = WiFi.SSID();
      }
      else {
        newString = F("%sysname%");
      }
      break;
    case eHeaderContent::eSysName:
      newString = F("%sysname%");
      break;
    case eHeaderContent::eTime:
      newString = F("%systime%");
      break;
    case eHeaderContent::eDate:
      newString = F("%sysday_0%.%sysmonth_0%.%sysyear%");
      break;
    case eHeaderContent::eIP:
      newString = F("%ip%");
      break;
    case eHeaderContent::eMAC:
      newString = F("%mac%");
      break;
    case eHeaderContent::eRSSI:
      newString = F("%rssi%dBm");
      break;
    case eHeaderContent::eBSSID:
      newString = F("%bssid%");
      break;
    case eHeaderContent::eWiFiCh:
      newString = F("Channel: %wi_ch%");
      break;
    case eHeaderContent::eUnit:
      newString = F("Unit: %unit%");
      break;
    case eHeaderContent::eSysLoad:
      newString = F("Load: %sysload%%");
      break;
    case eHeaderContent::eSysHeap:
      newString = F("Mem: %sysheap%");
      break;
    case eHeaderContent::eSysStack:
      newString = F("Stack: %sysstack%");
      break;
    case eHeaderContent::ePageNo:
      strHeader  = F("page ");
      strHeader += (currentFrameToDisplay + 1);

      if (MaxFramesToDisplay != 0xFF) {
        strHeader += F("/");
        strHeader += (MaxFramesToDisplay + 1);
      }
      break;
    # if P036_USERDEF_HEADERS
    case eHeaderContent::eUserDef1:
      newString = userDef1;
      break;
    case eHeaderContent::eUserDef2:
      newString = userDef2;
      break;
    # endif // if P036_USERDEF_HEADERS
    case eHeaderContent::eNone:
      return;
  }

  if (newString.length() > 0) {
    // Right now only systemvariables have been used, so we don't have to call the parseTemplate.
    parseSystemVariables(newString, false);
    strHeader = newString;
  }

  strHeader.trim();
  display_title(strHeader);

  // Display time and wifibars both clear area below, so paint them after the title.
  if (getDisplaySizeSettings(disp_resolution).Width == P36_MaxDisplayWidth) {
    display_time(); // only for 128pix wide displays
  }
  display_wifibars();

# ifdef OLEDDISPLAY_DOUBLE_BUFFER

  // Update only small sections of the display, reducing the amount of data to be sent to the display
  update_display();
# endif // ifdef OLEDDISPLAY_DOUBLE_BUFFER
}

void P036_data_struct::display_time() {
  if (!isInitialized()) {
    return;
  }

  String dtime = F("%systime%");

  parseSystemVariables(dtime, false);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(getArialMT_Plain_10());
  display->setColor(BLACK);
  display->fillRect(0, TopLineOffset, 28, GetHeaderHeight() - 2);
  display->setColor(WHITE);
  display->drawString(0, TopLineOffset, dtime.substring(0, 5));
}

void P036_data_struct::display_title(const String& title) {
  if (!isInitialized()) {
    return;
  }
  display->setFont(getArialMT_Plain_10());
  display->setColor(BLACK);
  display->fillRect(0, TopLineOffset, P36_MaxDisplayWidth, GetHeaderHeight()); // don't clear line under title.
  display->setColor(WHITE);

  if (getDisplaySizeSettings(disp_resolution).Width == P36_MaxDisplayWidth) {
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->drawString(P36_DisplayCentre, TopLineOffset, title);
  } else {
    display->setTextAlignment(TEXT_ALIGN_LEFT); // Display right of WiFi bars
    display->drawString(getDisplaySizeSettings(disp_resolution).PixLeft + getDisplaySizeSettings(disp_resolution).WiFiIndicatorWidth + 3,
                        TopLineOffset,
                        title);
  }
}

void P036_data_struct::display_logo() {
  if (!isInitialized()) {
    return;
  }
  # ifdef PLUGIN_036_DEBUG
  addLog(LOG_LEVEL_INFO, F("P036_DisplayLogo"));
  # endif // PLUGIN_036_DEBUG

  int left = 24;
  int top;
  const tFontSettings iFontsettings = CalculateFontSettings(2); // get font with max. height for displaying "ESP Easy"

  bDisplayingLogo = true;                                       // next time the display must be cleared completely
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(FontSizes[iFontsettings.fontIdx].fontData);
  display->clear();                                             // resets all pixels to black
  display->setColor(WHITE);
  display->drawString(65, iFontsettings.Top + TopLineOffset,                                              F("ESP"));
  display->drawString(65, iFontsettings.Top + iFontsettings.Height + iFontsettings.Space + TopLineOffset, F("Easy"));

  if (getDisplaySizeSettings(disp_resolution).PixLeft < left) { left = getDisplaySizeSettings(disp_resolution).PixLeft; }
  top = (getDisplaySizeSettings(disp_resolution).Height - espeasy_logo_height) / 2;

  if (top < 0) { top = 0; }
  display->drawXbm(left,
                   top + TopLineOffset,
                   espeasy_logo_width,
                   espeasy_logo_height,
                   espeasy_logo_bits); // espeasy_logo_width=espeasy_logo_height=36
}

// Draw the frame position

void P036_data_struct::display_indicator() {
  if (!isInitialized()) {
    return;
  }

  if (bHideFooter) { //  hide footer
    return;
  }

  int frameCount = MaxFramesToDisplay + 1;

  //  Erase Indicator Area
  display->setColor(BLACK);
  display->fillRect(0, P036_IndicatorTop + TopLineOffset, P36_MaxDisplayWidth, P036_IndicatorHeight);

  // Only display when there is something to display.
  if ((frameCount <= 1) || (frameCount > P36_Nlines)) { return; }

  display->setColor(WHITE);

  // Display chars as required
  for (uint8_t i = 0; i < frameCount; i++) {
    const char *image;

    if (currentFrameToDisplay == i) {
      image = activeSymbole;
    } else {
      image = inactiveSymbole;
    }

    int x;
    const int y = P036_IndicatorTop + TopLineOffset; // 2022-01-31 Removed unneeded offset '+ 2'

    // I would like a margin of 20 pixels on each side of the indicator.
    // Therefore the width of the indicator should be 128-40=88 and so space between indicator dots is 88/(framecount-1)
    // The width of the display is 128 and so the first dot must be at x=20 if it is to be centred at 64
    const int number_spaces = frameCount - 1;

    if (number_spaces <= 0) {
      return;
    }
    int margin  = 20;
    int spacing = (P36_MaxDisplayWidth - 2 * margin) / number_spaces;

    // Now apply some max of 30 pixels between the indicators and center the row of indicators.
    if (spacing > 30) {
      spacing = 30;
      margin  = (P36_MaxDisplayWidth - number_spaces * spacing) / 2;
    }

    x = margin + (spacing * i);
    display->drawXbm(x, y, 8, 8, image);
  }
}

int16_t P036_data_struct::GetHeaderHeight() const {
  if (bHideHeader) {
    // no header
    return 0;
  }
  return P36_HeaderHeight;
}

int16_t P036_data_struct::GetIndicatorTop() const {
  if (bHideFooter
      # if P036_ENABLE_TICKER
      || bUseTicker
      # endif // if P036_ENABLE_TICKER
      ) {
    // no footer (indicator) -> returm max. display height
    return getDisplaySizeSettings(disp_resolution).Height;
  }
  return P036_IndicatorTop;
}

tIndividualFontSettings P036_data_struct::CalculateIndividualFontSettings(uint8_t LineNo,
                                                                          uint8_t FontIndex,
                                                                          uint8_t LinesPerFrame,
                                                                          uint8_t FrameNo,
                                                                          int8_t  MaxHeight,
                                                                          uint8_t IdxForBiggestFont) {
  // returns the next LineNo or 0xFF if the setings do not fit
  tIndividualFontSettings result;
  uint8_t NextLineNo     = LineNo;
  uint8_t lLinesPerFrame = 0;
  int8_t  lHeight        = 0;
  int8_t  lSpace         = 0;
  int8_t  lTop           = 0;

  result.IdxForBiggestFontUsed = FontIndex;

  if (NextLineNo >= P36_Nlines) {
    // just in case
    result.NextLineNo = NextLineNo; // default, settings do fit
    return result;                  // finished
  }

  for (uint8_t i = LineNo; i < P36_Nlines; i++) {
    // calculate individual font settings
    int8_t lFontIndex             = FontIndex;
    const eModifyFont iModifyFont =
      static_cast<eModifyFont>(get3BitFromUL(LineContent->DisplayLinesV1[i].ModifyLayout, P036_FLAG_ModifyLayout_Font));

    switch (iModifyFont) {
      case eModifyFont::eEnlarge:

        if (ScrollingPages.linesPerFrameDef > 1) {
          // Font can only be enlarged if more than 1 line is displayed
          lFontIndex--;

          if (lFontIndex < IdxForBiggestFont) { lFontIndex = IdxForBiggestFont; }
          result.IdxForBiggestFontUsed = lFontIndex;
        }
        break;
      case eModifyFont::eMaximize:

        if (ScrollingPages.linesPerFrameDef > 1) {
          // Font can only be maximized if more than 1 line is displayed
          lFontIndex                   = IdxForBiggestFont;
          result.IdxForBiggestFontUsed = lFontIndex;
        }
        break;
      case eModifyFont::eReduce:
        lFontIndex++;

        if (lFontIndex > (P36_MaxFontCount - 1)) {
          lFontIndex = P36_MaxFontCount - 1;
        }
        break;
      case eModifyFont::eMinimize:
        lFontIndex = P36_MaxFontCount - 1;
        break;
      case eModifyFont::eNone:
        lFontIndex = FontIndex;
        break;
    }
    LineSettings[i].FontHeight = FontSizes[lFontIndex].Height;
    lHeight                   += FontSizes[lFontIndex].Height;
    LineSettings[i].frame      = FrameNo;
    LineSettings[i].fontIdx    = lFontIndex;
    lLinesPerFrame++;

    NextLineNo++;

    if (lLinesPerFrame == LinesPerFrame) { break; }
  }
  result.NextLineNo = NextLineNo; // default, settings do fit

  const int8_t deltaHeight = (MaxHeight - lHeight);

  if (lLinesPerFrame <= 1) {
    // just one lines per frame -> no space inbetween
    lSpace = 0;
    lTop   = (MaxHeight - lHeight) / 2;

    if (lHeight > MaxHeight) {
      result.NextLineNo = 0xFF; // settings do not fit
    }
  } else {
    if (deltaHeight >= (lLinesPerFrame - 1)) {
      // individual line setting fits
      // more than one lines per frame -> calculate space inbetween
      lSpace = deltaHeight / lLinesPerFrame;
      lTop   = (MaxHeight - (lHeight + (lSpace * (lLinesPerFrame - 1)))) / 2;
    } else {
      // individual line setting do not fit
      lTop   = 0;
      lSpace = -1; // allow overlapping by 1 pix

      if (deltaHeight < (-1 * (lLinesPerFrame - 1))) {
        if ((result.IdxForBiggestFontUsed == (P36_MaxFontCount - 1)) &&
            (LinesPerFrame == SizeSettings[static_cast<int>(disp_resolution)].MaxLines)) {
          // max lines for used display and smallest font reached -> use special space between the lines and return 'fits'
          // overlapping (lSpace<0) depends on the absolute display height
          switch (disp_resolution) {
            case p036_resolution::pix128x64: lSpace = bHideFooter ? 0 : -2;
              break;
            case p036_resolution::pix128x32: lSpace = -2;
              break;
            case p036_resolution::pix64x48: lSpace = -1;
              break;
          }
        } else {
          result.NextLineNo = 0xFF; // settings do not fit
        }
      }
    }
  }
  LineSettings[LineNo].ypos = lTop + GetHeaderHeight() + TopLineOffset;

  if (lLinesPerFrame > 1) {
    for (uint8_t k = (LineNo + 1); k < NextLineNo; k++) {
      LineSettings[k].ypos = LineSettings[k - 1].ypos + FontSizes[LineSettings[k - 1].fontIdx].Height + lSpace;
    }
  }
# ifdef P036_CHECK_INDIVIDUAL_FONT

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log1;

    if (log1.reserve(140)) { // estimated
      delay(10);             // FIXME otherwise it is maybe too fast for the serial monitor
      log1  = F("IndividualFontSettings:");
      log1 += concat(F(" result.NextLineNo:"), result.NextLineNo);
      log1 += concat(F(" result.IdxForBiggestFontUsed:"), result.IdxForBiggestFontUsed);
      log1 += concat(F(" LineNo:"), LineNo);
      log1 += concat(F(" LinesPerFrame:"), LinesPerFrame);

      if (result.NextLineNo != 0xFF) {
        log1 += strformat(F(" FrameNo:%d lTop:%d lSpace:%d"), FrameNo, lTop, lSpace);
      }
      addLogMove(LOG_LEVEL_INFO, log1);
    }
  }
# endif // # ifdef P036_CHECK_INDIVIDUAL_FONT
  return result;
}

tFontSettings P036_data_struct::CalculateFontSettings(uint8_t lDefaultLines) {
  tFontSettings result;
  int iHeight;
  int8_t  iFontIndex = -1;
  uint8_t iMaxHeightForFont;
  uint8_t iLinesPerFrame;
  uint8_t i;

  if (lDefaultLines == 0) {
    // number of lines can be reduced if no font fits the setting
    iLinesPerFrame = ScrollingPages.linesPerFrameDef;
    iHeight        = GetIndicatorTop() - GetHeaderHeight();
  } else {
    // number of lines is fixed (e.g. for splash screen)
    iLinesPerFrame = lDefaultLines;
    iHeight        = getDisplaySizeSettings(disp_resolution).Height;
  }
  uint8_t iUsedHeightForFonts = iHeight;

  # ifdef P036_FONT_CALC_LOG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO,
           strformat(F("P036 CalculateFontSettings lines: %d, height: %d, header: %s, footer: %s"),
                     iLinesPerFrame,
                     iHeight,
                     boolToString(!bHideHeader).c_str(),
                     boolToString(!bHideFooter).c_str()));
  }
  # endif // ifdef P036_FONT_CALC_LOG

  iMaxHeightForFont = lround(iHeight / (iLinesPerFrame * 1.0f)); // no extra space between lines
  // Fonts already have their own extra space, no need to add an extra pixel space

# ifdef P036_FONT_CALC_LOG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO,
           strformat(F("CalculateFontSettings LinesPerFrame: %d, iHeight: %d, maxFontHeight: %d"),
                     iLinesPerFrame, iHeight, iMaxHeightForFont));
  }
# endif // ifdef P036_FONT_CALC_LOG

  while (iFontIndex < 0) {
# ifdef P036_FONT_CALC_LOG
    String log1;
    log1.reserve(80);
# endif // ifdef P036_FONT_CALC_LOG

    for (i = 0; i < P36_MaxFontCount - 1; i++) {
      // check available fonts for the line setting
      # ifdef P036_FONT_CALC_LOG
      delay(5); // FIXME otherwise it is maybe too fast for the serial monitor
      log1 = strformat(F(" -> i: %d, h: %d"), i, FontSizes[i].Height);
      # endif // ifdef P036_FONT_CALC_LOG

      if (FontSizes[i].Height > iMaxHeightForFont) {
        // height of font is too big
        continue;
      }
      iFontIndex = i; // save the current index
      # ifdef P036_FONT_CALC_LOG
      log1 += concat(F(", fontIdx: "), iFontIndex);
      # endif // ifdef P036_FONT_CALC_LOG
      break;
    }

    if (iFontIndex < 0) {
      # ifdef P036_FONT_CALC_LOG
      log1 += concat(F(", no font fits, fontIdx: "), iFontIndex);
      addLogMove(LOG_LEVEL_INFO, log1);
      # endif // ifdef P036_FONT_CALC_LOG
      break;

      // }
    }
    # ifdef P036_FONT_CALC_LOG
    log1 += F(", font fits");
    addLogMove(LOG_LEVEL_INFO, log1);
    # endif // ifdef P036_FONT_CALC_LOG
  }

  if (iFontIndex >= 0) {
    // font found -> calculate top position and space between lines
    iUsedHeightForFonts = FontSizes[iFontIndex].Height * iLinesPerFrame;

    if (iLinesPerFrame > 1) {
      // more than one lines per frame -> calculate space inbetween
      result.Space = (iHeight - iUsedHeightForFonts) / iLinesPerFrame;
    } else {
      // just one lines per frame -> no space inbetween
      result.Space = 0;
    }
    result.Top = (iHeight - (iUsedHeightForFonts + (result.Space * (iLinesPerFrame - 1)))) / 2;
  } else {
    // no font found -> return font with shortest height
    // FIXED: tonhuisman Tweaked to match the 13 pix font to fit for 4 lines display
    result.Top = 0;

    // overlapping (lSpace<0) depends on the absolute display height
    switch (disp_resolution) {
      case p036_resolution::pix128x64:  result.Space = bHideFooter ? 0 : -2;
        break;
      case p036_resolution::pix128x32:  result.Space = -2;
        break;
      case p036_resolution::pix64x48:  result.Space = -1;
        break;
    }
    iFontIndex = P36_MaxFontCount - 1;
  }

  if (lDefaultLines == 0) {
    // calculate height for individual line font setting
    uint8_t currentLinesPerFrame = iLinesPerFrame;
    tIndividualFontSettings IndividualFontSettings;
    uint8_t currentFrame       = 0;
    uint8_t currentLine        = 0;
    uint8_t iIdxForBiggestFont = 0;

    while (currentLine < P36_Nlines) {
# if P036_ENABLE_TICKER

      if (bUseTicker && (currentLine > 0)) {
        // for ticker only the first line defines the font
        break;
      }
# endif // if P036_ENABLE_TICKER
      // calculate individual font settings
      IndividualFontSettings = CalculateIndividualFontSettings(currentLine,
                                                               iFontIndex,
                                                               currentLinesPerFrame,
                                                               currentFrame,
                                                               iHeight,
                                                               iIdxForBiggestFont);

      if (IndividualFontSettings.NextLineNo == 0xFF) {
        // individual settings do not fit
        if ((bReduceLinesPerFrame) && (currentLinesPerFrame > 1)) {
          currentLinesPerFrame--;                                                // reduce number of lines per frame
        } else {
          iIdxForBiggestFont = IndividualFontSettings.IdxForBiggestFontUsed + 1; // use smaller font size as maximum
        }
      } else {
        // individual line setting do fits
        currentLinesPerFrame = iLinesPerFrame; // default for next call of CalculateIndividualFontSettings()
        iIdxForBiggestFont   = 0;              // default for next call of CalculateIndividualFontSettings()
        currentLine          = IndividualFontSettings.NextLineNo;
        currentFrame++;                        // next frame
      }
    }

# ifdef P036_CHECK_INDIVIDUAL_FONT

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log1;

      if (log1.reserve(140)) { // estimated
        for (uint8_t i = 0; i < P36_Nlines; i++) {
          delay(5);            // FIXME otherwise it is maybe too fast for the serial monitor
          log1 = strformat(F("Line[%d]: Frame:%d FontIdx:%d ypos:%d FontHeight:%d"), i, LineSettings[i].frame,
                           LineSettings[i].fontIdx, LineSettings[i].ypos - TopLineOffset, LineSettings[i].FontHeight);
          addLogMove(LOG_LEVEL_INFO, log1);
        }
      }
    }
# endif // ifdef P036_CHECK_INDIVIDUAL_FONT
  }
  result.fontIdx = iFontIndex;
  result.Height  = FontSizes[iFontIndex].Height;

# ifdef P036_FONT_CALC_LOG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log1;

    if (log1.reserve(140)) { // estimated
      delay(5);              // FIXME otherwise it is maybe too fast for the serial monitor
      log1  = concat(F("CalculateFontSettings: Font:"), result.FontName());
      log1 += strformat(F(" Idx:%d Top:%d FontHeight:%d Space:%d"), iFontIndex, result.Top, result.Height, result.Space);
      log1 += concat(F(" HeightForLines:"), iHeight);
      log1 += concat(F(" LinesPerFrame:"), iLinesPerFrame);

      if (lDefaultLines == 0) {
        log1 += concat(F(" DefaultLinesPerFrame:"), ScrollingPages.linesPerFrameDef);
      }
      else {
        log1 += concat(F(" DefaultLines:"), lDefaultLines);
      }
      addLogMove(LOG_LEVEL_INFO, log1);
    }
  }
# endif // P036_FONT_CALC_LOG

  return result;
}

void P036_data_struct::prepare_pagescrolling(ePageScrollSpeed lscrollspeed,
                                             uint8_t          NrLines) {
  if (!isInitialized()) {
    return;
  }
# if P036_ENABLE_TICKER
  bUseTicker = (lscrollspeed == ePageScrollSpeed::ePSS_Ticker);

  if (bUseTicker) {
    ScrollingPages.linesPerFrameDef = 1;
  }
  else
# endif // if P036_ENABLE_TICKER
  {
    ScrollingPages.linesPerFrameDef = NrLines;
  }
  CalculateFontSettings(0);
}

uint8_t P036_data_struct::display_scroll(ePageScrollSpeed lscrollspeed, int lTaskTimer)
{
  if (!isInitialized()) {
    return 0;
  }

  int iPageScrollTime;
  int iCharToRemove = 0;

# ifdef PLUGIN_036_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, concat(F("Start Scrolling: Speed: "), static_cast<int>(lscrollspeed)));
  }
# endif // PLUGIN_036_DEBUG

  ScrollingLines.wait = 0;

  // calculate total page scrolling time
  if ((lscrollspeed == ePageScrollSpeed::ePSS_Instant) // no scrolling, just the handling time to build the new page
      # if P036_ENABLE_TICKER                          // for ticker, no scrolling, just the handling time to build the new page
      || (lscrollspeed == ePageScrollSpeed::ePSS_Ticker)
      # endif // if P036_ENABLE_TICKER
      ) {
    iPageScrollTime = P36_PageScrollTick - P36_PageScrollTimer;
  } else {
    iPageScrollTime = (P36_MaxDisplayWidth / (P36_PageScrollPix * static_cast<int>(lscrollspeed))) * P36_PageScrollTick;
  }
  int iScrollTime = static_cast<float>(lTaskTimer * 1000 - iPageScrollTime - 2 * P36_WaitScrollLines * 100) / 100; // scrollTime in ms

# ifdef PLUGIN_036_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, concat(F("PageScrollTime: "), iPageScrollTime));
  }
# endif // PLUGIN_036_DEBUG

  uint16_t MaxPixWidthForPageScrolling = P36_MaxDisplayWidth;

  if (bLineScrollEnabled) {
    // Reduced scrolling width because line is displayed left or right aligned
    MaxPixWidthForPageScrolling -= getDisplaySizeSettings(disp_resolution).PixLeft;
  }

# if P036_ENABLE_TICKER

  if (bUseTicker) {
    ScrollingLines.Ticker.Tcontent = EMPTY_STRING;
    ScrollingLines.Ticker.IdxEnd   = 0;
    ScrollingLines.Ticker.IdxStart = 0;

    for (uint8_t i = 0; i < P36_Nlines; i++) {
      String tmpString(LineContent->DisplayLinesV1[i].Content);
      tmpString.replace(F("<|>"), "   "); // replace the split token with three space char
      ScrollingLines.Ticker.Tcontent += P36_parseTemplate(tmpString, i);
    }
    ScrollingLines.Ticker.len = ScrollingLines.Ticker.Tcontent.length();
  }
# endif // if P036_ENABLE_TICKER

  for (uint8_t j = 0; j < ScrollingPages.linesPerFrameDef; j++) {
    // default no line scrolling and strings are centered
    uint16_t PixLengthLineOut = 0; // pix length of line out
    uint16_t PixLengthLineIn  = 0; // pix length of line in
    ScrollingLines.SLine[j].LastWidth = 0;
    ScrollingLines.SLine[j].Width     = 0;

    // get last and new line width
    if (j < ScrollingPages.linesPerFrameIn) {
      PixLengthLineIn = TrimStringTo255Chars(&ScrollingPages.In[j]);
    }

    if (j < ScrollingPages.linesPerFrameIn) {
      PixLengthLineOut = TrimStringTo255Chars(&ScrollingPages.Out[j]);
    }

    if (bLineScrollEnabled) {
      // settings for following line scrolling
      if (PixLengthLineOut > getDisplaySizeSettings(disp_resolution).Width) {
        ScrollingLines.SLine[j].LastWidth = PixLengthLineOut; // while page scrolling this line is right aligned
      }

      if ((
            # if P036_ENABLE_TICKER
            bUseTicker ||
            # endif // if P036_ENABLE_TICKER
            (PixLengthLineIn > getDisplaySizeSettings(disp_resolution).Width)) &&
          (iScrollTime > 0)) {
        // width of the line > display width -> scroll line
        # if P036_ENABLE_TICKER

        if (bUseTicker) {
          ScrollingLines.SLine[j].Width = 0;
          uint16_t AddPixTicker = 0;

          switch (textAlignment) {
            case TEXT_ALIGN_CENTER: ScrollingLines.SLine[j].CurrentLeft = getDisplaySizeSettings(disp_resolution).PixLeft +
                                                                          getDisplaySizeSettings(disp_resolution).Width / 2;
              AddPixTicker = getDisplaySizeSettings(disp_resolution).Width / 2; // half width at begin
              break;
            case TEXT_ALIGN_RIGHT:  ScrollingLines.SLine[j].CurrentLeft = getDisplaySizeSettings(disp_resolution).PixLeft +
                                                                          getDisplaySizeSettings(disp_resolution).Width;
              AddPixTicker = getDisplaySizeSettings(disp_resolution).Width; // full width at begin
              break;
            default: ScrollingLines.SLine[j].CurrentLeft = getDisplaySizeSettings(disp_resolution).PixLeft;
              break;
          }
          ScrollingLines.SLine[j].fPixSum = ScrollingLines.SLine[j].CurrentLeft;

          display->setFont(FontSizes[LineSettings[j].fontIdx].fontData);
          ScrollingLines.SLine[j].dPix = (static_cast<float>(display->getStringWidth(ScrollingLines.Ticker.Tcontent) + AddPixTicker)) /
                                         static_cast<float>(iScrollTime);
          ScrollingLines.SLine[j].SLcontent = EMPTY_STRING;

          ScrollingLines.Ticker.TickerAvgPixPerChar = lround(static_cast<float>(display->getStringWidth(
                                                                                  ScrollingLines.Ticker.Tcontent)) /
                                                             static_cast<float>(ScrollingLines.Ticker.len));

          if (ScrollingLines.Ticker.TickerAvgPixPerChar < ScrollingLines.SLine[j].dPix) {
            ScrollingLines.Ticker.TickerAvgPixPerChar = round(2 * ScrollingLines.SLine[j].dPix);
          }
          ScrollingLines.Ticker.MaxPixLen = getDisplaySizeSettings(disp_resolution).Width + 2 * ScrollingLines.Ticker.TickerAvgPixPerChar;

          // add more characters to display
          while (true) {
            char c             = ScrollingLines.Ticker.Tcontent.charAt(ScrollingLines.Ticker.IdxEnd);
            uint8_t PixForChar = display->getCharWidth(c);

            if ((ScrollingLines.SLine[0].Width + PixForChar) >= ScrollingLines.Ticker.MaxPixLen) {
              break; // no more characters necessary to add
            }
            ScrollingLines.Ticker.IdxEnd++;
            ScrollingLines.SLine[j].Width += PixForChar;
          }
        } else
        # endif // if P036_ENABLE_TICKER
        {
          ScrollingLines.SLine[j].SLcontent   = ScrollingPages.In[j].SPLcontent;
          ScrollingLines.SLine[j].SLidx       = ScrollingPages.In[j].SPLidx; // index to LineSettings[]
          ScrollingLines.SLine[j].Width       = PixLengthLineIn; // while page scrolling this line is left aligned
          ScrollingLines.SLine[j].CurrentLeft = getDisplaySizeSettings(disp_resolution).PixLeft;
          ScrollingLines.SLine[j].fPixSum     = getDisplaySizeSettings(disp_resolution).PixLeft;

          // pix change per scrolling line tick
          ScrollingLines.SLine[j].dPix =
            (static_cast<float>(PixLengthLineIn - getDisplaySizeSettings(disp_resolution).Width)) / iScrollTime;
        }

        # ifdef P036_SCROLL_CALC_LOG

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          delay(5); // FIXME otherwise it is maybe too fast for the serial monitor
          addLog(LOG_LEVEL_INFO, strformat(F("Line: %d width: %d dPix: %d"),
                                           j + 1, ScrollingLines.SLine[j].Width, ScrollingLines.SLine[j].dPix));
          #  if P036_ENABLE_TICKER

          if (bUseTicker) {
            delay(5); // FIXME otherwise it is maybe too fast for the serial monitor
            String log1;
            log1.reserve(200);
            log1  = concat(F("+++ iScrollTime: "), iScrollTime);
            log1 += concat(F(" StrLength: "), ScrollingLines.Ticker.len);
            log1 += concat(F(" StrInPix: "), display->getStringWidth(ScrollingLines.Ticker.Tcontent));
            log1 += concat(F(" PixPerChar: "), ScrollingLines.Ticker.TickerAvgPixPerChar);
            addLogMove(LOG_LEVEL_INFO, log1);
          }
          #  endif // if P036_ENABLE_TICKER
        }
        # endif    // P036_SCROLL_CALC_LOG
      }
    }

    // reduce line content for page scrolling to max width
    if (PixLengthLineIn > MaxPixWidthForPageScrolling) {
      const int strlen = ScrollingPages.In[j].SPLcontent.length();
# ifdef P036_SCROLL_CALC_LOG
      const String LineInStr = ScrollingPages.In[j].SPLcontent;
# endif // P036_SCROLL_CALC_LOG
      float fAvgPixPerChar = static_cast<float>(PixLengthLineIn) / strlen;

      if (bLineScrollEnabled) {
        // shorten string on right side because line is displayed left aligned while scrolling
        // using floor() because otherwise empty space on right side
        iCharToRemove = floor(
          (static_cast<float>(PixLengthLineIn - MaxPixWidthForPageScrolling)) / fAvgPixPerChar);
        ScrollingPages.In[j].SPLcontent = ScrollingPages.In[j].SPLcontent.substring(0, strlen - iCharToRemove);
      } else {
        switch (ScrollingPages.In[j].Alignment) {
          case TEXT_ALIGN_LEFT:
            // shorten string on right side because line is left aligned
            // using floor() because otherwise empty space on right side
            iCharToRemove = floor(
              (static_cast<float>(PixLengthLineIn - MaxPixWidthForPageScrolling)) / fAvgPixPerChar);
            ScrollingPages.In[j].SPLcontent = ScrollingPages.In[j].SPLcontent.substring(0, strlen - iCharToRemove);
            break;
          case TEXT_ALIGN_RIGHT:
            // shorten string on left side because line is right aligned
            // using floor() because otherwise empty space on left side
            iCharToRemove = floor(
              (static_cast<float>(PixLengthLineIn - MaxPixWidthForPageScrolling)) / fAvgPixPerChar);
            ScrollingPages.In[j].SPLcontent = ScrollingPages.In[j].SPLcontent.substring(iCharToRemove);
            break;
          default:
            // shorten string on both sides because line is displayed centered
            // using floor() because otherwise empty space on both sides
            iCharToRemove =
              floor((static_cast<float>(PixLengthLineIn - MaxPixWidthForPageScrolling)) / (2 * fAvgPixPerChar));
            ScrollingPages.In[j].SPLcontent = ScrollingPages.In[j].SPLcontent.substring(0, strlen - iCharToRemove);
            ScrollingPages.In[j].SPLcontent = ScrollingPages.In[j].SPLcontent.substring(iCharToRemove);
            break;
        }
      }
      # ifdef P036_SCROLL_CALC_LOG
      String log;

      if (loglevelActiveFor(LOG_LEVEL_INFO) &&
          log.reserve(128)) {
        delay(5); // FIXME otherwise it is maybe too fast for the serial monitor
        log  = concat(F("Line: "), j + 1);
        log += concat(F(" LineIn: "), LineInStr);
        log += concat(F(" Length: "), strlen);
        log += concat(F(" PixLength: "), PixLengthLineIn);
        log += concat(F(" AvgPixPerChar: "), fAvgPixPerChar);
        log += concat(F(" CharsRemoved: "), iCharToRemove);
        addLog(LOG_LEVEL_INFO, log);
        log.clear();
        log += concat(F(" -> Changed to: "), ScrollingPages.In[j].SPLcontent);
        log += concat(F(" Length: "), ScrollingPages.In[j].SPLcontent.length());
        display->setFont(FontSizes[LineSettings[ScrollingPages.In[j].SPLidx].fontIdx].fontData);
        log += concat(F(" PixLength: "), display->getStringWidth(ScrollingPages.In[j].SPLcontent));
        addLogMove(LOG_LEVEL_INFO, log);
      }
      # endif // P036_SCROLL_CALC_LOG
    }

    // reduce line content for page scrolling to max width

    if (PixLengthLineOut > MaxPixWidthForPageScrolling) {
      const int strlen = ScrollingPages.Out[j].SPLcontent.length();
      # ifdef P036_SCROLL_CALC_LOG
      const String LineOutStr = ScrollingPages.Out[j].SPLcontent;
      # endif // P036_SCROLL_CALC_LOG
      float fAvgPixPerChar = static_cast<float>(PixLengthLineOut) / strlen;

      boolean bCheckLengthLeft  = false;
      boolean bCheckLengthRight = false;

      if (bLineScrollEnabled) {
        // shorten string on left side because line is displayed right aligned while scrolling
        // using ceilf() because otherwise overlapping the new text
        iCharToRemove = ceilf(
          (static_cast<float>(PixLengthLineOut - MaxPixWidthForPageScrolling)) / fAvgPixPerChar);
        ScrollingPages.Out[j].SPLcontent = ScrollingPages.Out[j].SPLcontent.substring(iCharToRemove);
        bCheckLengthRight                = true;
      } else {
        switch (ScrollingPages.Out[j].Alignment) {
          case TEXT_ALIGN_LEFT:
            // shorten string on right side because line is left aligned
            // using ceilf() because otherwise overlapping the max of 255 pixels
            iCharToRemove = ceilf(
              (static_cast<float>(PixLengthLineOut - MaxPixWidthForPageScrolling)) / fAvgPixPerChar);
            ScrollingPages.Out[j].SPLcontent = ScrollingPages.Out[j].SPLcontent.substring(0, strlen - iCharToRemove);
            bCheckLengthLeft                 = true;
            break;
          case TEXT_ALIGN_RIGHT:
            // shorten string on left side because line is right aligned
            // using ceilf() because otherwise overlapping the new text
            iCharToRemove = ceilf(
              (static_cast<float>(PixLengthLineOut - MaxPixWidthForPageScrolling)) / fAvgPixPerChar);
            ScrollingPages.Out[j].SPLcontent = ScrollingPages.Out[j].SPLcontent.substring(iCharToRemove);
            bCheckLengthRight                = true;
            break;
          default: // TEXT_ALIGN_CENTER/TEST_ALIGN_CENTER_BOTH
            // shorten string on both sides because line is displayed centered
            // using ceilf() because otherwise overlapping the new text on left side
            // using ceilf() because otherwise overlapping the max of 255 pixels on right side
            iCharToRemove = ceilf(
              (static_cast<float>(PixLengthLineOut - MaxPixWidthForPageScrolling)) / (2 * fAvgPixPerChar));
            ScrollingPages.Out[j].SPLcontent = ScrollingPages.Out[j].SPLcontent.substring(0, strlen - iCharToRemove);
            ScrollingPages.Out[j].SPLcontent = ScrollingPages.Out[j].SPLcontent.substring(iCharToRemove);
            bCheckLengthLeft                 = true;
            bCheckLengthRight                = true;
            break;
        }
      }

      if (bCheckLengthLeft || bCheckLengthRight) {
        // set font only once if necessary
        display->setFont(FontSizes[LineSettings[ScrollingPages.Out[j].SPLidx].fontIdx].fontData);
      }

      while (bCheckLengthLeft || bCheckLengthRight) {
        uint16_t StringWidthPix = display->getStringWidth(ScrollingPages.Out[j].SPLcontent);

        if (bCheckLengthLeft) {
          if (StringWidthPix > MaxPixWidthForPageScrolling) {
            // remove one more character because still overlapping the new text
            ScrollingPages.Out[j].SPLcontent = ScrollingPages.Out[j].SPLcontent.substring(1, iCharToRemove - 1);

            if (bCheckLengthRight) {
              // get new value for CheckLengthRight
              StringWidthPix = display->getStringWidth(ScrollingPages.Out[j].SPLcontent);
            }
          } else {
            bCheckLengthLeft = false;
          }
        }

        if (bCheckLengthRight) {
          if (StringWidthPix > MaxPixWidthForPageScrolling) {
            // remove one more character because otherwise overlapping the max of 255 pixels on right side
            ScrollingPages.Out[j].SPLcontent = ScrollingPages.Out[j].SPLcontent.substring(1 + 1);
          } else {
            bCheckLengthRight = false;
          }
        }
      }

# ifdef P036_SCROLL_CALC_LOG
      String log;

      if (loglevelActiveFor(LOG_LEVEL_INFO) &&
          log.reserve(128)) {
        delay(5); // FIXME otherwise it is maybe too fast for the serial monitor
        log  = concat(F("Line: "), j + 1);
        log += concat(F(" LineOut: "), LineOutStr);
        log += concat(F(" Length: "), strlen);
        log += concat(F(" PixLength: "), PixLengthLineOut);
        log += concat(F(" AvgPixPerChar: "), fAvgPixPerChar);
        log += concat(F(" CharsRemoved: "), iCharToRemove);
        addLog(LOG_LEVEL_INFO, log);
        delay(5); // FIXME otherwise it is maybe too fast for the serial monitor
        log.clear();
        log += concat(F(" -> Changed to: "), ScrollingPages.Out[j].SPLcontent);
        log += concat(F(" Length: "), ScrollingPages.Out[j].SPLcontent.length());
        display->setFont(FontSizes[LineSettings[ScrollingPages.Out[j].SPLidx].fontIdx].fontData);
        log += concat(F(" PixLength: "), display->getStringWidth(ScrollingPages.Out[j].SPLcontent));
        addLogMove(LOG_LEVEL_INFO, log);
      }
# endif // P036_SCROLL_CALC_LOG
    }
  }

  ScrollingPages.dPix    = P36_PageScrollPix * static_cast<int>(lscrollspeed); // pix change per scrolling page tick
  ScrollingPages.dPixSum = ScrollingPages.dPix;

  display_scroll_timer(true, lscrollspeed);                                    // Initial display of the page

# ifdef PLUGIN_036_DEBUG
  addLog(LOG_LEVEL_INFO, F("Scrolling finished"));
# endif // PLUGIN_036_DEBUG
  return ScrollingPages.Scrolling;
}

uint8_t P036_data_struct::display_scroll_timer(bool             initialScroll,
                                               ePageScrollSpeed lscrollspeed) {
  if (!isInitialized()) {
    return 0;
  }

  // page scrolling (using PLUGIN_TASKTIMER_IN)
  display->setColor(BLACK);

  // We allow 12 pixels (including underline) at the top because otherwise the wifi indicator gets too squashed!!
  // scrolling window is 44 pixels high - ie 64 less margin of 12 at top and 8 at bottom
  display->fillRect(0, GetHeaderHeight() + (initialScroll ? 0 : 1) + TopLineOffset, P36_MaxDisplayWidth,
                    GetIndicatorTop() - GetHeaderHeight());
  display->setColor(WHITE);

  if (initialScroll) {
    if (!bHideHeader) {
      display->drawLine(0,
                        GetHeaderHeight() + TopLineOffset,
                        P36_MaxDisplayWidth,
                        GetHeaderHeight() + TopLineOffset); // line below title
    }
  }

  # if P036_ENABLE_TICKER

  if (bUseTicker) {
    // for Ticker start with the set alignment
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->setFont(FontSizes[LineSettings[ScrollingLines.SLine[0].SLidx].fontIdx].fontData);
    display->drawString(ScrollingLines.SLine[0].CurrentLeft,
                        LineSettings[ScrollingLines.SLine[0].SLidx].ypos,
                        ScrollingLines.Ticker.Tcontent.substring(ScrollingLines.Ticker.IdxStart, ScrollingLines.Ticker.IdxEnd));
  } else
  # endif // if P036_ENABLE_TICKER
  {
    for (uint8_t j = 0; j < ScrollingPages.linesPerFrameOut; j++) {
      if ((initialScroll && (lscrollspeed < ePageScrollSpeed::ePSS_Instant)) ||
          !initialScroll) {
        // scrolling, prepare scrolling page out to right
        DrawScrollingPageLine(&ScrollingPages.Out[j], ScrollingLines.SLine[j].LastWidth, TEXT_ALIGN_RIGHT);
      }
    }

    for (uint8_t j = 0; j < ScrollingPages.linesPerFrameIn; j++) {
      // non-scrolling or scrolling prepare scrolling page in from left
      DrawScrollingPageLine(&ScrollingPages.In[j], ScrollingLines.SLine[j].Width, TEXT_ALIGN_LEFT);
    }
  }

  update_display();

  if ((initialScroll && (lscrollspeed < ePageScrollSpeed::ePSS_Instant)) ||
      (!initialScroll && (ScrollingPages.dPixSum < P36_MaxDisplayWidth))) { // scrolling
    // page still scrolling
    ScrollingPages.dPixSum += ScrollingPages.dPix;
  } else {
    // page scrolling finished
    ScrollingPages.Scrolling = 0; // allow following line scrolling
  }
  return ScrollingPages.Scrolling;
}

// Draw scrolling line (1pix/s)
void P036_data_struct::display_scrolling_lines() {
  if (!isInitialized()) {
    return;
  }

  // line scrolling (using PLUGIN_TEN_PER_SECOND)

  uint8_t i;
  bool    bscroll       = false;
  bool    updateDisplay = false;
  int     iCurrentLeft;

  for (i = 0; i < ScrollingPages.linesPerFrameIn; i++) {
    if (ScrollingLines.SLine[i].Width != 0) {
      bscroll = true;
      break;
    }
  }

  if (bscroll) {
    if (ScrollingLines.wait < P36_WaitScrollLines) {
      ScrollingLines.wait++;
      return; // wait before scrolling line not finished
    }

    for (i = 0; i < ScrollingPages.linesPerFrameIn; i++) {
      if (ScrollingLines.SLine[i].Width != 0) {
        // scroll this line
        ScrollingLines.SLine[i].fPixSum -= ScrollingLines.SLine[i].dPix;
        iCurrentLeft                     = lround(ScrollingLines.SLine[i].fPixSum);

        if (iCurrentLeft != ScrollingLines.SLine[i].CurrentLeft) {
          // still scrolling
          ScrollingLines.SLine[i].CurrentLeft = iCurrentLeft;
          updateDisplay                       = true;
          display->setColor(BLACK);
          display->fillRect(0, LineSettings[ScrollingLines.SLine[i].SLidx].ypos + 1, P36_MaxDisplayWidth,
                            FontSizes[LineSettings[ScrollingLines.SLine[i].SLidx].fontIdx].Height);
          display->setColor(WHITE);

          display->setFont(FontSizes[LineSettings[ScrollingLines.SLine[i].SLidx].fontIdx].fontData);

          if (
            # if P036_ENABLE_TICKER
            bUseTicker ||
            # endif // if P036_ENABLE_TICKER
            (((iCurrentLeft - getDisplaySizeSettings(disp_resolution).PixLeft) +
              ScrollingLines.SLine[i].Width) >= getDisplaySizeSettings(disp_resolution).Width)) {
            display->setTextAlignment(TEXT_ALIGN_LEFT);

            # if P036_ENABLE_TICKER

            if (bUseTicker) {
              display->drawString(iCurrentLeft,
                                  LineSettings[ScrollingLines.SLine[0].SLidx].ypos,
                                  ScrollingLines.Ticker.Tcontent.substring(ScrollingLines.Ticker.IdxStart, ScrollingLines.Ticker.IdxEnd));

              // add more characters to display
              iCurrentLeft -= getDisplaySizeSettings(disp_resolution).PixLeft;

              while (true) {
                if (ScrollingLines.Ticker.IdxEnd >= ScrollingLines.Ticker.len) { // end of string
                  break;
                }
                const uint8_t c          = ScrollingLines.Ticker.Tcontent.charAt(ScrollingLines.Ticker.IdxEnd);
                const uint8_t PixForChar = display->getCharWidth(c); // PixForChar can be 0 if c is non ascii

                if ((static_cast<int>(ScrollingLines.SLine[0].Width + PixForChar) + iCurrentLeft) >= ScrollingLines.Ticker.MaxPixLen) {
                  break;                                             // no more characters necessary to add
                }
                ScrollingLines.Ticker.IdxEnd++;
                ScrollingLines.SLine[0].Width += PixForChar;
              }

              // remove already displayed characters
              float fCurrentPixLeft = static_cast<float>(getDisplaySizeSettings(disp_resolution).PixLeft) - 2.0f *
                                      ScrollingLines.Ticker.TickerAvgPixPerChar;

              while (ScrollingLines.SLine[0].fPixSum < fCurrentPixLeft) {
                const uint8_t c          = ScrollingLines.Ticker.Tcontent.charAt(ScrollingLines.Ticker.IdxStart);
                const uint8_t PixForChar = display->getCharWidth(c); // PixForChar can be 0 if c is non ascii
                ScrollingLines.SLine[0].fPixSum += static_cast<float>(PixForChar);
                ScrollingLines.Ticker.IdxStart++;

                if (ScrollingLines.Ticker.IdxStart >= ScrollingLines.Ticker.IdxEnd) {
                  ScrollingLines.SLine[0].Width = 0; // Stop scrolling

                  #  ifdef PLUGIN_036_DEBUG

                  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                    addLog(LOG_LEVEL_INFO, F("Ticker finished"));
                  }
                  #  endif // PLUGIN_036_DEBUG
                  break;
                }

                if (ScrollingLines.SLine[0].Width > PixForChar) {
                  ScrollingLines.SLine[0].Width -= PixForChar;
                }
              }
              break;
            } else
            # endif // if P036_ENABLE_TICKER
            {
              display->drawString(iCurrentLeft,
                                  LineSettings[ScrollingLines.SLine[i].SLidx].ypos,
                                  ScrollingLines.SLine[i].SLcontent);
            }
          } else {
            # if P036_ENABLE_TICKER

            if (!bUseTicker)
            # endif // if P036_ENABLE_TICKER
            {
              // line scrolling finished -> line is shown as aligned right
              display->setTextAlignment(TEXT_ALIGN_RIGHT);
              display->drawString(P36_MaxDisplayWidth - getDisplaySizeSettings(disp_resolution).PixLeft,
                                  LineSettings[ScrollingLines.SLine[i].SLidx].ypos,
                                  ScrollingLines.SLine[i].SLcontent);
            }
            ScrollingLines.SLine[i].Width = 0; // Stop scrolling
          }
        }
      }
    }

    if (updateDisplay && (ScrollingPages.Scrolling == 0)) {
      update_display();
    }
  }
}

// Draw Signal Strength Bars, return true when there was an update.
bool P036_data_struct::display_wifibars() {
  if (!isInitialized()) {
    return false;
  }

  if (bHideHeader) { //  hide header
    return false;
  }

  const bool connected    = NetworkConnected();
  const int  nbars_filled = (WiFi.RSSI() + 100) / 12; // all bars filled if RSSI better than -46dB
  const int  newState     = connected ? nbars_filled : P36_WIFI_STATE_NOT_CONNECTED;

  if (newState == lastWiFiState) {
    return false; // nothing to do.
  }
  const int x         = getDisplaySizeSettings(disp_resolution).WiFiIndicatorLeft;
  const int y         = TopLineOffset;
  int size_x          = getDisplaySizeSettings(disp_resolution).WiFiIndicatorWidth;
  const int size_y    = GetHeaderHeight() - 2;
  const int nbars     = 5;
  const int16_t width = (size_x / nbars);

  size_x = width * nbars - 1; // Correct for round errors.

  //  x,y are the x,y locations
  //  sizex,sizey are the sizes (should be a multiple of the number of bars)
  //  nbars is the number of bars and nbars_filled is the number of filled bars.

  //  We leave a 1 pixel gap between bars
  display->setColor(BLACK);
  display->fillRect(x, y, size_x, size_y);
  display->setColor(WHITE);

  if (NetworkConnected()) {
    for (uint8_t ibar = 0; ibar < nbars; ibar++) {
      const int16_t height = size_y * (ibar + 1) / nbars;
      const int16_t xpos   = x + ibar * width;
      const int16_t ypos   = y + size_y - height;

      if (ibar <= nbars_filled) {
        // Fill complete bar
        display->fillRect(xpos, ypos, width - 1, height);
      } else {
        // Only draw top and bottom.
        display->fillRect(xpos, ypos,           width - 1, 1);
        display->fillRect(xpos, y + size_y - 1, width - 1, 1);
      }
    }
  } else {
    // Draw a not connected sign (empty rectangle)
    display->drawRect(x, y, size_x, size_y - 1);
  }
  return true;
}

void P036_data_struct::update_display()
{
  if (isInitialized()) {
    display->display();
  }
}

void P036_data_struct::P036_JumpToPage(struct EventStruct *event, uint8_t nextFrame)
{
  if (!isInitialized()) {
    return;
  }

  // reschedule page change
  Scheduler.schedule_task_device_timer(event->TaskIndex,
                                       millis() + (Settings.TaskDeviceTimer[event->TaskIndex] * 1000));
  nextFrameToDisplay    = nextFrame;
  bPageScrollDisabled   = true; //  show next page without scrolling
  disableFrameChangeCnt = 2;    //  disable next page change in PLUGIN_READ if PLUGIN_READ was already scheduled
  P036_DisplayPage(event);      //  Display the selected page, function needs 65ms!
  displayTimer = P036_TIMER;    //  Restart timer
}

void P036_data_struct::P036_JumpToPageOfLine(struct EventStruct *event, uint8_t LineNo)
{
  CalcMaxPageCount(); // Update max page count and DisplayedPageNo
  P036_JumpToPage(event, LineSettings[LineNo].DisplayedPageNo);
}

// Defines the Scroll area layout
// Displays the selected page, function needs 65ms!
// Called by PLUGIN_READ and P036_JumpToPage()
void P036_data_struct::P036_DisplayPage(struct EventStruct *event)
{
  # ifdef PLUGIN_036_DEBUG
  addLog(LOG_LEVEL_INFO, F("P036_DisplayPage"));
  # endif // PLUGIN_036_DEBUG

  if (!isInitialized()) {
    return;
  }

  uint8_t NFrames; // the number of frames
  uint8_t lineCounter = 0;

  if (P036_DisplayIsOn) {
    // Display is on.
    ScrollingPages.Scrolling = 1;                                  // page scrolling running -> no line scrolling allowed
    NFrames                  = LineSettings[P36_Nlines - 1].frame; // last prepared page in prepare_pagescrolling()
    HeaderContent            = static_cast<eHeaderContent>(get8BitFromUL(PCONFIG_LONG(0), P036_FLAG_HEADER));
    HeaderContentAlternative = static_cast<eHeaderContent>(get8BitFromUL(PCONFIG_LONG(0), P036_FLAG_HEADER_ALTERNATIVE));

    // Construct the outgoing string
    for (uint8_t i = 0; i < P36_Nlines; i++) {
      if (LineSettings[i].frame == frameCounter) {
        lineCounter = i;
        break;
      }
    }

    for (uint8_t i = 0; i < ScrollingPages.linesPerFrameDef; i++)
    {
      if (LineSettings[lineCounter + i].frame != frameCounter) {
        continue;
      }
      ScrollingPages.linesPerFrameOut = i + 1;
      CreateScrollingPageLine(&ScrollingPages.Out[i], lineCounter + i);
    }

    // now loop round looking for the next frame with some content
    //   skip this frame if all lines in frame are blank
    // - we exit the while loop if any line is not empty
    bool foundText = false;
    int  ntries    = 0;

    while (!foundText) {
      //        Stop after framecount loops if no data found
      ntries++;

      if (ntries > (NFrames + 1)) {
        // do not leave the while loop to early
        // it needs to loop back to frameCounter=0 if just one frame is having text
        break;
      }

      if (nextFrameToDisplay == 0xff) {
        // Increment the frame counter
        frameCounter++;

        if (frameCounter > NFrames) {
          frameCounter          = 0;
          currentFrameToDisplay = 0;
        }
      } else {
        // next frame because content changed in PLUGIN_WRITE
        frameCounter = nextFrameToDisplay;
      }

      //        Contruct incoming strings
      for (uint8_t i = 0; i < P36_Nlines; i++) {
        if (nextFrameToDisplay == 0xff) {
          // showing next page
          if (LineSettings[i].frame == frameCounter) {
            lineCounter = i;
            break;
          }
        } else {
          // jump to a displayed page from PlugIn
          if (LineSettings[i].DisplayedPageNo == nextFrameToDisplay) {
            frameCounter = LineSettings[i].frame;
            lineCounter  = i;
            break;
          }
        }
      }

      for (uint8_t i = 0; i < ScrollingPages.linesPerFrameDef; i++)
      {
        if (LineSettings[lineCounter + i].frame != frameCounter) {
          continue;
        }
        ScrollingPages.linesPerFrameIn = i + 1;
        CreateScrollingPageLine(&ScrollingPages.In[i], lineCounter + i);
        # if P036_FEATURE_DISPLAY_PREVIEW
        currentLines[i] = ScrollingPages.In[i].SPLcontent;
        # endif // if P036_FEATURE_DISPLAY_PREVIEW

        if (ScrollingPages.In[i].SPLcontent.length() > 0) {
          foundText = true;

          # if P036_FEATURE_DISPLAY_PREVIEW && P036_FEATURE_ALIGN_PREVIEW

          #  if P036_ENABLE_TICKER

          if (!bUseTicker)
          #  endif // if P036_ENABLE_TICKER
          {
            // Preview: Center or Right-Align add spaces on the left
            const bool isAlignCenter = ScrollingPages.In[i].Alignment == OLEDDISPLAY_TEXT_ALIGNMENT::TEXT_ALIGN_CENTER;
            const bool isAlignRight  = ScrollingPages.In[i].Alignment == OLEDDISPLAY_TEXT_ALIGNMENT::TEXT_ALIGN_RIGHT;

            if (isAlignRight || isAlignCenter) {
              const uint16_t maxlength  = getDisplaySizeSettings(disp_resolution).Width;
              const uint16_t pixlength  = display->getStringWidth(currentLines[i]); // pix length for entire string
              const uint16_t charlength = display->getStringWidth(F(" "));          // pix length for a space char
              int16_t addSpaces         = (maxlength - pixlength) / charlength;

              if (isAlignCenter) {
                addSpaces /= 2;
              }

              if (addSpaces > 0) {
                currentLines[i].reserve(currentLines[i].length() + addSpaces);

                while (addSpaces > 0) {
                  currentLines[i] = ' ' + currentLines[i];
                  addSpaces--;
                }
              }
            }
          }
          # endif // if P036_FEATURE_DISPLAY_PREVIEW && P036_FEATURE_ALIGN_PREVIEW
        }
      }

      if (foundText) {
        if (nextFrameToDisplay == 0xff) {
          if (frameCounter != 0) {
            ++currentFrameToDisplay;
          }
        } else {
          currentFrameToDisplay = nextFrameToDisplay;
        }
      } else {
        nextFrameToDisplay = 0xff; // no text on the page -> jump to this page is not possible
      }
    }
    nextFrameToDisplay = 0xFF;

    CalcMaxPageCount(); // Update max page count

    // Update display
    if (bDisplayingLogo) {
      bDisplayingLogo = false;
      display->clear();        // resets all pixels to black
    }

    bAlternativHeader = false; // start with first header content
    HeaderCount       = 0;     // reset header count
    display_header();

    display_indicator();

    update_display();

    const bool bScrollWithoutWifi = bitRead(PCONFIG_LONG(0), 24); // Bit 24
    const bool bScrollLines       = bitRead(PCONFIG_LONG(0), 17); // Bit 17
    bRunning           = NetworkConnected() || bScrollWithoutWifi;
    bLineScrollEnabled = ((bScrollLines
                           # if P036_ENABLE_TICKER
                           || bUseTicker
                           # endif // if P036_ENABLE_TICKER
                           ) && bRunning); // scroll lines only if WifiIsConnected,
    // WifiIsConnected,
    // otherwise too slow

    ePageScrollSpeed lscrollspeed = static_cast<ePageScrollSpeed>(P036_SCROLL);

    if (bPageScrollDisabled) { lscrollspeed = ePageScrollSpeed::ePSS_Instant; } // first page after INIT without scrolling

    int lTaskTimer = Settings.TaskDeviceTimer[event->TaskIndex];

    if (display_scroll(lscrollspeed, lTaskTimer)) {
      Scheduler.setPluginTaskTimer(P36_PageScrollTimer, event->TaskIndex, event->Par1); // calls next page scrollng tick
    }

    if (bRunning) {
      // scroll lines only if WifiIsConnected, otherwise too slow
      bPageScrollDisabled = false; // next PLUGIN_READ will do page scrolling
    }
  # ifdef PLUGIN_036_DEBUG
  } else {
    addLog(LOG_LEVEL_INFO, F("P036_DisplayPage Display off"));
  # endif // PLUGIN_036_DEBUG
  }
}

// Perform some specific changes for OLED display
String P036_data_struct::P36_parseTemplate(String& tmpString, uint8_t lineIdx) {
  if (tmpString.length() == 0) {
    return EMPTY_STRING;
  }
  String result = parseTemplate_padded(tmpString, 20);

  result.trim();

  // OLED lib uses this routine to convert UTF8 to extended ASCII
  // http://playground.arduino.cc/Main/Utf8ascii
  // Attempt to display euro sign (FIXME)

  /*
     const char euro[4] = {0xe2, 0x82, 0xac, 0}; // Unicode euro symbol
     const char euro_oled[3] = {0xc2, 0x80, 0}; // Euro symbol OLED display font
     result.replace(euro, euro_oled);
   */
  const eAlignment iAlignment =
    static_cast<eAlignment>(get3BitFromUL(LineContent->DisplayLinesV1[lineIdx].ModifyLayout, P036_FLAG_ModifyLayout_Alignment));

  OLEDDISPLAY_TEXT_ALIGNMENT iTextAlignment = getTextAlignment(static_cast<eAlignment>(iAlignment));

  # if P036_ENABLE_TICKER

  if (bUseTicker) {
    iTextAlignment = TEXT_ALIGN_RIGHT; // ticker is always right aligned
  }
  # endif // if P036_ENABLE_TICKER

  switch (iTextAlignment) {
    case TEXT_ALIGN_LEFT:

      // add leading spaces from tmpString to the result
      for (uint16_t l = 0; l < tmpString.length(); l++) {
        if (tmpString[l] != ' ') {
          break;
        }
        result = ' ' + result;
      }
      break;
    case TEXT_ALIGN_RIGHT:

      // add trailing spaces from tmpString to the result
      for (int16_t l = tmpString.length() - 1; l >= 0; l--) {
        if (tmpString[l] != ' ') {
          break;
        }
        result += ' ';
      }
      break;
    default:
      break;
  }
  return result;
}

# if P036_ENABLE_LEFT_ALIGN
void P036_data_struct::setTextAlignment(eAlignment aAlignment) {
  switch (aAlignment) {
    case eAlignment::eLeft:   textAlignment = TEXT_ALIGN_LEFT; break;
    case eAlignment::eRight:  textAlignment = TEXT_ALIGN_RIGHT; break;
    case eAlignment::eCenter: // Fall through
    case eAlignment::eGlobal: textAlignment = TEXT_ALIGN_CENTER; break;
  }

  MaxFramesToDisplay = 0xFF; // Recalculate page indicator
  nextFrameToDisplay = 0;    // Reset to first page
}

OLEDDISPLAY_TEXT_ALIGNMENT P036_data_struct::getTextAlignment(eAlignment aAlignment) const {
  switch (aAlignment) {
    case eAlignment::eLeft:   return TEXT_ALIGN_LEFT;
    case eAlignment::eRight:  return TEXT_ALIGN_RIGHT;
    case eAlignment::eCenter: return TEXT_ALIGN_CENTER;
    case eAlignment::eGlobal: break;
  }
  return textAlignment;
}

uint8_t P036_data_struct::GetTextLeftMargin(OLEDDISPLAY_TEXT_ALIGNMENT _textAlignment) const {
  // left margin must be offset with PixLeft (the first shown left pixel on 64x48 displays is 32!)
  if (_textAlignment == TEXT_ALIGN_LEFT) {
    return getDisplaySizeSettings(disp_resolution).PixLeft;
  }

  if (_textAlignment == TEXT_ALIGN_CENTER) {
    return (getDisplaySizeSettings(disp_resolution).Width / 2) + getDisplaySizeSettings(disp_resolution).PixLeft;
  }

  if (_textAlignment == TEXT_ALIGN_RIGHT) {
    return getDisplaySizeSettings(disp_resolution).Width + getDisplaySizeSettings(disp_resolution).PixLeft;
  }
  return getDisplaySizeSettings(disp_resolution).PixLeft;
}

# endif // if P036_ENABLE_LEFT_ALIGN

void P036_data_struct::registerButtonState(uint8_t newButtonState, bool bPin3Invers) {
  if ((ButtonLastState == 0xFF) || (bPin3Invers != (!!newButtonState))) {
    ButtonLastState = newButtonState;
    DebounceCounter++;

    if (RepeatCounter > 0) {
      RepeatCounter--;      // decrease the repeat count
    }
  } else {
    ButtonLastState = 0xFF; // Reset
    DebounceCounter = 0;
    RepeatCounter   = 0;
    ButtonState     = false;
  }

  if ((ButtonLastState == newButtonState) &&
      (DebounceCounter >= P36_DebounceTreshold) &&
      (RepeatCounter == 0)) {
    ButtonState = true;
  }
}

void P036_data_struct::markButtonStateProcessed() {
  ButtonState     = false;
  DebounceCounter = 0;
  RepeatCounter   = P36_RepeatDelay; //  Wait a bit before repeating the button action
}

uint16_t P036_data_struct::CalcPixLength(uint8_t LineNo) {
  if (LineContent->DisplayLinesV1[LineNo].Content[0] == 0) {
    // empty string
    return 0;
  }
  display->setFont(FontSizes[LineSettings[LineNo].fontIdx].fontData);
  return display->getStringWidth(LineContent->DisplayLinesV1[LineNo].Content);
}

void P036_data_struct::CalcMaxPageCount(void) {
  // Update max page count
  if (MaxFramesToDisplay == 0xFF) {
    // not updated yet
    uint8_t iFrame = 0;

    for (uint8_t i = 0; i < P36_Nlines; i++) {
      if (LineContent->DisplayLinesV1[i].Content[0] != 0) {   // line is not empty
        LineSettings[i].DisplayedPageNo = MaxFramesToDisplay; // current MaxFramesToDisplay is the number of the shown page
      } else {
        LineSettings[i].DisplayedPageNo = 0xff;               // line is not shown
      }

      if (LineSettings[i].frame != iFrame) { continue; } // line is not yet on the next page

      for (uint8_t k = 0; k < ScrollingPages.linesPerFrameDef; k++) {
        if ((i + k) >= P36_Nlines) { break; }

        if ((LineSettings[i + k].frame) != iFrame) { // line is already on the next page
          iFrame++;                                  // next frame to check
          break;
        }

        if (LineContent->DisplayLinesV1[i + k].Content[0] != 0) { // line is not empty
          iFrame++;                                               // next frame to check

          if (MaxFramesToDisplay == 0xFF) {
            MaxFramesToDisplay = 0;
          } else {
            MaxFramesToDisplay++;
          }
          LineSettings[i + k].DisplayedPageNo = MaxFramesToDisplay; // current MaxFramesToDisplay is the number of the shown page
          break;
        }
      }
    }
# ifdef P036_CHECK_INDIVIDUAL_FONT

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log1;

      if (log1.reserve(140)) { // estimated
        log1 = concat(F("CalcMaxPageCount: MaxFramesToDisplay:"), MaxFramesToDisplay);
        addLog(LOG_LEVEL_INFO, log1);

        for (uint8_t i = 0; i < P36_Nlines; i++) {
          log1.clear();
          delay(5); // FIXME otherwise it is maybe too fast for the serial monitor
          log1 = strformat(F("Line[%d]: Frame:%d DisplayedPageNo:%d FontIdx:%d ypos:%d FontHeight:%d"),
                           i,
                           LineSettings[i].frame,
                           LineSettings[i].DisplayedPageNo,
                           LineSettings[i].fontIdx,
                           LineSettings[i].ypos - TopLineOffset,
                           LineSettings[i].FontHeight);
          addLogMove(LOG_LEVEL_INFO, log1);
        }
      }
    }
# endif // ifdef P036_CHECK_INDIVIDUAL_FONT
  }
}

uint16_t P036_data_struct::TrimStringTo255Chars(tScrollingPageLines *ScrollingPageLine) {
  uint16_t PixLengthLine = 0;

  if (ScrollingPageLine->SPLcontent.length() > 0) {
    display->setFont(FontSizes[LineSettings[ScrollingPageLine->SPLidx].fontIdx].fontData);
    PixLengthLine = display->getStringWidth(ScrollingPageLine->SPLcontent);

    if (PixLengthLine > 255) {
      // shorten string because OLED controller can not handle such long strings
      const int   strlen         = ScrollingPageLine->SPLcontent.length();
      const float fAvgPixPerChar = static_cast<float>(PixLengthLine) / strlen;
      const int   iCharToRemove  = ceilf((static_cast<float>(PixLengthLine - 255)) / fAvgPixPerChar);
      ScrollingPageLine->SPLcontent = ScrollingPageLine->SPLcontent.substring(0, strlen - iCharToRemove);
      PixLengthLine                 = display->getStringWidth(ScrollingPageLine->SPLcontent);
    }
  }
  return PixLengthLine;
}

void P036_data_struct::DrawScrollingPageLine(tScrollingPageLines       *ScrollingPageLine,
                                             uint16_t                   Width,
                                             OLEDDISPLAY_TEXT_ALIGNMENT textAlignment) {
  int16_t LeftOffset = 0;

  switch (textAlignment) {
    case TEXT_ALIGN_LEFT: LeftOffset  = -P36_MaxDisplayWidth; break;
    case TEXT_ALIGN_RIGHT: LeftOffset = 0; break;
    default: LeftOffset               = 0; break;
  }
  display->setFont(FontSizes[LineSettings[ScrollingPageLine->SPLidx].fontIdx].fontData);

  if (Width > 0) {
    // textAlignment=TEXT_ALIGN_LEFT:  width of LineIn[j] > display width -> line is left aligned while scrolling page
    // textAlignment=TEXT_ALIGN_RIGHT: width of LineOut[j] > display width -> line is right aligned while scrolling page
    display->setTextAlignment(textAlignment);
    display->drawString(-P36_MaxDisplayWidth + getDisplaySizeSettings(disp_resolution).PixLeft + ScrollingPages.dPixSum,
                        LineSettings[ScrollingPageLine->SPLidx].ypos,
                        ScrollingPageLine->SPLcontent);
  } else {
    // line is kept aligned while scrolling page
    display->setTextAlignment(ScrollingPageLine->Alignment);

    // textAlignment=TEXT_ALIGN_LEFT: for non-scrolling pages ScrollingPages.dPixSum=P36_MaxDisplayWidth -> therefore the calculation must
    // use P36_MaxDisplayWidth, too
    display->drawString(LeftOffset + GetTextLeftMargin(ScrollingPageLine->Alignment) + ScrollingPages.dPixSum,
                        LineSettings[ScrollingPageLine->SPLidx].ypos,
                        ScrollingPageLine->SPLcontent);
  }
}

void P036_data_struct::CreateScrollingPageLine(tScrollingPageLines *ScrollingPageLine, uint8_t Counter) {
  # if P036_ENABLE_TICKER

  if (bUseTicker) {
    ScrollingPageLine->SPLcontent = EMPTY_STRING;
  } else
  # endif // if P036_ENABLE_TICKER
  {
    String tmpString(LineContent->DisplayLinesV1[Counter].Content);

    ScrollingPageLine->SPLcontent = P36_parseTemplate(tmpString, Counter);

    if (ScrollingPageLine->SPLcontent.length() > 0) {
      const int splitIdx = ScrollingPageLine->SPLcontent.indexOf(F("<|>")); // check for split token

      if (splitIdx >= 0) {
        // split line into left and right part
        tmpString = ScrollingPageLine->SPLcontent;
        tmpString.replace(F("<|>"), F(" "));                            // replace in tmpString the split token with one space char
        display->setFont(FontSizes[LineSettings[Counter].fontIdx].fontData);
        uint16_t pixlength = display->getStringWidth(tmpString);        // pixlength without split token but with one space char
        tmpString = ' ';
        const uint16_t charlength = display->getStringWidth(tmpString); // pix length for a space char
        pixlength += charlength;

        while (pixlength <= getDisplaySizeSettings(disp_resolution).Width) {
          // add more space chars until pixlength of the final line is almost the display width
          tmpString += ' ';                                         // add another space char
          pixlength += charlength;
        }
        ScrollingPageLine->SPLcontent.replace(F("<|>"), tmpString); // replace in final line the split token with space chars
      }
    }
    const eAlignment iAlignment =
      static_cast<eAlignment>(get3BitFromUL(LineContent->DisplayLinesV1[Counter].ModifyLayout, P036_FLAG_ModifyLayout_Alignment));

    ScrollingPageLine->Alignment = getTextAlignment(iAlignment);
    ScrollingPageLine->SPLidx    = Counter; // index to LineSettings[]
  }
}

# if P036_FEATURE_DISPLAY_PREVIEW
bool P036_data_struct::web_show_values() {
  addHtml(F("<pre>")); // To keep spaces etc. in the shown output

  for (uint8_t i = 0; i < ScrollingPages.linesPerFrameDef; i++) {
    addHtmlDiv(F("div_l"), currentLines[i], EMPTY_STRING, F("style='font-size:75%;'"));

    if (i != ScrollingPages.linesPerFrameDef - 1) {
      addHtmlDiv(F("div_br"));
    }
  }
  addHtml(F("</pre>"));
  return true;
}

# endif // if P036_FEATURE_DISPLAY_PREVIEW


# if P036_SEND_EVENTS
void P036_data_struct::P036_SendEvent(struct EventStruct *event, uint8_t eventId, int16_t eventValue) {
  const __FlashStringHelper* eventid_str = F("");
  
  switch (eventId) {
    case P036_EVENT_DISPLAY:   eventid_str =  F("display");  break;
    case P036_EVENT_CONTRAST:  eventid_str =  F("contrast"); break;
    case P036_EVENT_FRAME:     eventid_str =  F("frame");    break;
    case P036_EVENT_LINE:      eventid_str =  F("line");     break;
    #  if P036_ENABLE_LINECOUNT
    case P036_EVENT_LINECNT:   eventid_str =  F("linecount"); break;
    #  endif // if P036_ENABLE_LINECOUNT
    case P036_EVENT_RESTORE:   eventid_str =  F("restore");   break;
    case P036_EVENT_SCROLL:    eventid_str =  F("scroll");    break;
    default:
    return;
  }


  eventQueue.add(event->TaskIndex, eventid_str, eventValue);
}

# endif // if P036_SEND_EVENTS


#endif  // ifdef USES_P036
