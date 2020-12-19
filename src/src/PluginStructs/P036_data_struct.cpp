#include "../PluginStructs/P036_data_struct.h"

#ifdef USES_P036

# include "../ESPEasyCore/ESPEasyNetwork.h"
# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/Misc.h"
# include "../Helpers/Scheduler.h"
# include "../Helpers/StringConverter.h"
# include "../Helpers/StringParser.h"
# include "../Helpers/SystemVariables.h"

# include <Dialog_Plain_12_font.h>
# include <OLED_SSD1306_SH1106_images.h>

P036_data_struct::P036_data_struct() : display(nullptr) {}

P036_data_struct::~P036_data_struct() {
  reset();
}

void P036_data_struct::reset() {
  if (display != nullptr) {
    display->displayOff();
    display->end();
    delete display;
    display = nullptr;
  }
}

const tFontSizes FontSizes[P36_MaxFontCount] = {
  { ArialMT_Plain_24, 24, 28},
  { ArialMT_Plain_16, 16, 19},
  { Dialog_plain_12,  13, 15},
  { ArialMT_Plain_10, 10, 13}
};

const tSizeSettings SizeSettings[P36_MaxSizesCount] = {
  { P36_MaxDisplayWidth, P36_MaxDisplayHeight, 0, // 128x64
       4,               // max. line count
       113, 15          // WiFi indicator
  },
  { P36_MaxDisplayWidth, 32,                   0, // 128x32
       2,               // max. line count
       113, 15          // WiFi indicator
  },
  { 64,                  48,                   32, // 64x48
       3,               // max. line count
       32,  10          // WiFi indicator
  }
};


const tSizeSettings& P036_data_struct::getDisplaySizeSettings(p036_resolution disp_resolution) {
  int index = static_cast<int>(disp_resolution);

  if ((index < 0) || (index >= P36_MaxSizesCount)) { index = 0; }

  return SizeSettings[index];
}

bool P036_data_struct::init(taskIndex_t     taskIndex,
                            uint8_t         LoadVersion,
                            uint8_t         Type,
                            uint8_t         Address,
                            uint8_t         Sda,
                            uint8_t         Scl,
                            p036_resolution Disp_resolution,
                            bool            Rotated,
                            uint8_t         Contrast,
                            uint8_t         DisplayTimer,
                            uint8_t         NrLines) {
  reset();

  lastWiFiState       = P36_WIFI_STATE_UNSET;
  disp_resolution     = p036_resolution::pix128x64;
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

  if (display != nullptr) {
    display->init(); // call to local override of init function

    disp_resolution = Disp_resolution;
    bHideFooter = !(getDisplaySizeSettings(disp_resolution).Height == P36_MaxDisplayHeight);

    if (disp_resolution == p036_resolution::pix128x32) {
      display->displayOff();
      display->SetComPins(0x02); // according to the adafruit lib, sometimes this may need to be 0x02
      bHideFooter = true;
    }

    display->displayOn();
    loadDisplayLines(taskIndex, LoadVersion);

    // Flip screen if required
    setOrientationRotated(Rotated);

    setContrast(Contrast);

    //      Display the device name, logo, time and wifi
    display_logo();
    update_display();

    //    Initialize frame counter
    frameCounter                 = 0;
    currentFrameToDisplay        = 0;
    nextFrameToDisplay           = 0;
    bPageScrollDisabled          = true;  // first page after INIT without scrolling
    ScrollingPages.linesPerFrame = NrLines;
    bLineScrollEnabled           = false; // start without line scrolling

    //    Clear scrolling line data
    for (uint8_t i = 0; i < P36_MAX_LinesPerPage; i++) {
      ScrollingLines.Line[i].Width     = 0;
      ScrollingLines.Line[i].LastWidth = 0;
    }

    //    prepare font and positions for page and line scrolling
    prepare_pagescrolling();
  }

  return isInitialized();
}

bool P036_data_struct::isInitialized() const {
  return display != nullptr;
}

void P036_data_struct::loadDisplayLines(taskIndex_t taskIndex, uint8_t LoadVersion) {
  if (LoadVersion == 0) {
    // read data of version 0 (up to 22.11.2019)
    String DisplayLinesV0[P36_Nlines];                                           // used to load the CustomTaskSettings for V0
    LoadCustomTaskSettings(taskIndex, DisplayLinesV0, P36_Nlines, P36_NcharsV0); // max. length 1024 Byte  (DAT_TASKS_CUSTOM_SIZE)

    for (int i = 0; i < P36_Nlines; ++i) {
      safe_strncpy(DisplayLinesV1[i].Content, DisplayLinesV0[i], P36_NcharsV1);
      DisplayLinesV1[i].Content[P36_NcharsV1 - 1] = 0; // Terminate in case of uninitalized data
      DisplayLinesV1[i].FontType                  = 0xff;
      DisplayLinesV1[i].FontHeight                = 0xff;
      DisplayLinesV1[i].FontSpace                 = 0xff;
      DisplayLinesV1[i].reserved                  = 0xff;
    }
  }
  else {
    // read data of version 1 (beginning from 22.11.2019)
    LoadCustomTaskSettings(taskIndex, (uint8_t *)&(DisplayLinesV1), sizeof(DisplayLinesV1));

    for (int i = 0; i < P36_Nlines; ++i) {
      DisplayLinesV1[i].Content[P36_NcharsV1 - 1] = 0; // Terminate in case of uninitalized data
    }
  }
}

void P036_data_struct::setContrast(uint8_t OLED_contrast) {
  if (!isInitialized()) {
    return;
  }
  char contrast  = 100;
  char precharge = 241;
  char comdetect = 64;

  switch (OLED_contrast) {
    case P36_CONTRAST_OFF:
      display->displayOff();
      return;
    case P36_CONTRAST_LOW:
      contrast = 10; precharge = 5; comdetect = 0;
      break;
    case P36_CONTRAST_MED:
      contrast = P36_CONTRAST_MED; precharge = 0x1F; comdetect = 64;
      break;
    case P36_CONTRAST_HIGH:
    default:
      contrast = P36_CONTRAST_HIGH; precharge = 241; comdetect = 64;
      break;
  }
  display->displayOn();
  display->setContrast(contrast, precharge, comdetect);
}

void P036_data_struct::setOrientationRotated(bool rotated) {
  if (rotated) {
    display->flipScreenVertically();
    TopLineOffset = P36_MaxDisplayHeight - getDisplaySizeSettings(disp_resolution).Height;
  } else {
    TopLineOffset = 0;
  }
}

void P036_data_struct::display_header() {
  if (!isInitialized()) {
    return;
  }
  if (bHideHeader) {  //  hide header
    return;
  }

  eHeaderContent iHeaderContent;
  String newString, strHeader;

  if ((HeaderContentAlternative == HeaderContent) || !bAlternativHeader) {
    iHeaderContent = HeaderContent;
  }
  else
  {
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
      newString = F("%rssi%dB");
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
    default:
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
}

void P036_data_struct::display_time() {
  if (!isInitialized()) {
    return;
  }

  String dtime = F("%systime%");

  parseSystemVariables(dtime, false);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->setColor(BLACK);
  display->fillRect(0, TopLineOffset, 28, GetHeaderHeight() - 2);
  display->setColor(WHITE);
  display->drawString(0, TopLineOffset, dtime.substring(0, 5));
}

void P036_data_struct::display_title(const String& title) {
  if (!isInitialized()) {
    return;
  }
  display->setFont(ArialMT_Plain_10);
  display->setColor(BLACK);
  display->fillRect(0, TopLineOffset, P36_MaxDisplayWidth, GetHeaderHeight()); // don't clear line under title.
  display->setColor(WHITE);

  if (getDisplaySizeSettings(disp_resolution).Width == P36_MaxDisplayWidth) {
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->drawString(P36_DisplayCentre, TopLineOffset, title);
  }
  else {
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
  tFontSettings iFontsettings = CalculateFontSettings(2); // get font with max. height for displaying "ESP Easy"

  bDisplayingLogo = true; // next time the display must be cleared completely
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(iFontsettings.fontData);
  display->clear(); // resets all pixels to black
  display->setColor(WHITE);
  display->drawString(65, iFontsettings.Top + TopLineOffset, F("ESP"));
  display->drawString(65, iFontsettings.Top + iFontsettings.Height + iFontsettings.Space + TopLineOffset, F("Easy"));

  if (getDisplaySizeSettings(disp_resolution).PixLeft < left) { left = getDisplaySizeSettings(disp_resolution).PixLeft; }
  top = (getDisplaySizeSettings(disp_resolution).Height-espeasy_logo_height)/2;
  if (top < 0) { top = 0; }
  display->drawXbm(left,
                   top+TopLineOffset,
                   espeasy_logo_width,
                   espeasy_logo_height,
                   espeasy_logo_bits); // espeasy_logo_width=espeasy_logo_height=36
}

// Draw the frame position

void P036_data_struct::display_indicator() {
  if (!isInitialized()) {
    return;
  }
  if (bHideFooter) {  //  hide footer
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

    int x, y;

    y = P036_IndicatorTop + 2 + TopLineOffset;

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

int16_t P036_data_struct::GetHeaderHeight()
{
  if (bHideHeader) {
    // no header
    return 0;
  }
  return P36_HeaderHeight;

}
int16_t P036_data_struct::GetIndicatorTop()
{
  if (bHideFooter) {
    // no footer (indicator) -> returm max. display height
    return getDisplaySizeSettings(disp_resolution).Height;
  }
  return P036_IndicatorTop;
}

tFontSettings P036_data_struct::CalculateFontSettings(uint8_t lDefaultLines)
{
  tFontSettings result;
  int iHeight;
  int8_t iFontIndex = -1;
  uint8_t iMaxHeightForFont;
  uint8_t iLinesPerFrame;

  if (lDefaultLines == 0) 
  {
    // number of lines can be reduced if no font fits the setting
    iLinesPerFrame = ScrollingPages.linesPerFrame;
    iHeight = GetIndicatorTop() - GetHeaderHeight();
  }
  else
  {
    // number of lines is fixed (e.g. for splash screen)
    iLinesPerFrame = lDefaultLines;
    iHeight = getDisplaySizeSettings(disp_resolution).Height;
  }
  
  while (iFontIndex < 0) {
    iMaxHeightForFont = (iHeight - (iLinesPerFrame - 1)) / iLinesPerFrame;  // at least 1 pixel space between lines

    for (uint8_t i = P36_MaxFontCount; i > 0; i--) {
      // check available fonts for the line setting  
      if (FontSizes[i-1].Height > iMaxHeightForFont) {
        // height of font is to big
        break;
      }
      iFontIndex = i-1; // save the current index
      if (FontSizes[iFontIndex].Height == iMaxHeightForFont) {
        // height of font just fits the line setting
        break;
      }
    }
    if (iFontIndex < 0) {
      // no font fits -> reduce number of lines per page
      iLinesPerFrame--;
      if (iLinesPerFrame == 0) {
        // lines per frame is at minimum
        break;
      }
    }
  }
  if (iFontIndex >= 0) {
    // font found -> calculate top position and space between lines
    iMaxHeightForFont = FontSizes[iFontIndex].Height * iLinesPerFrame;
    if (iLinesPerFrame > 1) {
      // more than one lines per frame -> calculate space inbetween
      result.Space = (iHeight-iMaxHeightForFont) / iLinesPerFrame;
    }
    else {
      // just one lines per frame -> no space inbetween
      result.Space = 0;
    }
    result.Top = (iHeight - (iMaxHeightForFont + (result.Space * (iLinesPerFrame-1)))) / 2;
  }
  else {
    // no font found -> return font with shortest height
    result.Top = 0;
    result.Space = 1;
    iLinesPerFrame = 1;
    iFontIndex = P36_MaxFontCount-1;
 }
  result.fontData = FontSizes[iFontIndex].fontData;
  result.Height = FontSizes[iFontIndex].Height;

# ifdef PLUGIN_036_DEBUG
  String log;
  log.reserve(128); // estimated
  log = F("CalculateFontSettings: FontIndex:");
  log += iFontIndex;
  log += F(" Top:");
  log += result.Top;
  log += F(" FontHeight:");
  log += result.Height;
  log += F(" Space:");
  log += result.Space;
  log += F(" Height:");
  log += iHeight;
  log += F(" LinesPerFrame:");
  log += iLinesPerFrame;
  log += F(" DefaultLines:");
  log += lDefaultLines;
  addLog(LOG_LEVEL_INFO, log);
# endif // PLUGIN_036_DEBUG
  
  if (lDefaultLines == 0) 
    ScrollingPages.linesPerFrame = iLinesPerFrame;
  return result;
}

void P036_data_struct::prepare_pagescrolling()
{
  if (!isInitialized()) {
    return;
  }

  tFontSettings iFontsettings = CalculateFontSettings(0);

  ScrollingPages.Font    = iFontsettings.fontData;
  ScrollingPages.ypos[0] = iFontsettings.Top + GetHeaderHeight() + TopLineOffset;
  ScrollingPages.ypos[1] = ScrollingPages.ypos[0] + iFontsettings.Height + iFontsettings.Space;
  ScrollingPages.ypos[2] = ScrollingPages.ypos[1] + iFontsettings.Height + iFontsettings.Space;
  ScrollingPages.ypos[3] = ScrollingPages.ypos[2] + iFontsettings.Height + iFontsettings.Space;

  ScrollingLines.Font  = ScrollingPages.Font;
  ScrollingLines.Space = iFontsettings.Height + iFontsettings.Space + 1;

  for (uint8_t i = 0; i < P36_MAX_LinesPerPage; i++) {
    ScrollingLines.Line[i].ypos = ScrollingPages.ypos[i];
  }
}

uint8_t P036_data_struct::display_scroll(ePageScrollSpeed lscrollspeed, int lTaskTimer)
{
  if (!isInitialized()) {
    return 0;
  }

  // LineOut[] contain the outgoing strings in this frame
  // LineIn[] contain the incoming strings in this frame

  int iPageScrollTime;
  int iCharToRemove;

# ifdef PLUGIN_036_DEBUG
  String log;
  log.reserve(128); // estimated
  log = F("Start Scrolling: Speed: ");
  log += ((int) lscrollspeed);
  addLog(LOG_LEVEL_INFO, log);
# endif // PLUGIN_036_DEBUG

  display->setFont(ScrollingPages.Font);

  ScrollingLines.wait = 0;

  // calculate total page scrolling time
  if (lscrollspeed == ePageScrollSpeed::ePSS_Instant) {
    // no scrolling, just the handling time to build the new page
    iPageScrollTime = P36_PageScrollTick - P36_PageScrollTimer;
  } else {
    iPageScrollTime = (P36_MaxDisplayWidth / (P36_PageScrollPix * static_cast<int>(lscrollspeed))) * P36_PageScrollTick;
  }
  int iScrollTime = (float)(lTaskTimer * 1000 - iPageScrollTime - 2 * P36_WaitScrollLines * 100) / 100; // scrollTime in ms

# ifdef PLUGIN_036_DEBUG
  log  = F("PageScrollTime: ");
  log += iPageScrollTime;
  addLog(LOG_LEVEL_INFO, log);
# endif // PLUGIN_036_DEBUG

  uint16_t MaxPixWidthForPageScrolling = P36_MaxDisplayWidth;

  if (bLineScrollEnabled) {
    // Reduced scrolling width because line is displayed left or right aligned
    MaxPixWidthForPageScrolling -= getDisplaySizeSettings(disp_resolution).PixLeft;
  }

  for (uint8_t j = 0; j < ScrollingPages.linesPerFrame; j++)
  {
    // default no line scrolling and strings are centered
    ScrollingLines.Line[j].LastWidth = 0;
    ScrollingLines.Line[j].Width     = 0;

    // get last and new line width
    uint16_t PixLengthLineOut = display->getStringWidth(ScrollingPages.LineOut[j]);
    uint16_t PixLengthLineIn  = display->getStringWidth(ScrollingPages.LineIn[j]);

    if (PixLengthLineIn > 255) {
      // shorten string because OLED controller can not handle such long strings
      int   strlen         = ScrollingPages.LineIn[j].length();
      float fAvgPixPerChar = ((float)PixLengthLineIn) / strlen;
      iCharToRemove            = ceil(((float)(PixLengthLineIn - 255)) / fAvgPixPerChar);
      ScrollingPages.LineIn[j] = ScrollingPages.LineIn[j].substring(0, strlen - iCharToRemove);
      PixLengthLineIn          = display->getStringWidth(ScrollingPages.LineIn[j]);
    }

    if (PixLengthLineOut > 255) {
      // shorten string because OLED controller can not handle such long strings
      int   strlen         = ScrollingPages.LineOut[j].length();
      float fAvgPixPerChar = ((float)PixLengthLineOut) / strlen;
      iCharToRemove             = ceil(((float)(PixLengthLineOut - 255)) / fAvgPixPerChar);
      ScrollingPages.LineOut[j] = ScrollingPages.LineOut[j].substring(0, strlen - iCharToRemove);
      PixLengthLineOut          = display->getStringWidth(ScrollingPages.LineOut[j]);
    }

    if (bLineScrollEnabled) {
      // settings for following line scrolling
      if (PixLengthLineOut > getDisplaySizeSettings(disp_resolution).Width) {
        ScrollingLines.Line[j].LastWidth = PixLengthLineOut; // while page scrolling this line is right aligned
      }

      if ((PixLengthLineIn > getDisplaySizeSettings(disp_resolution).Width) && (iScrollTime > 0))
      {
        // width of the line > display width -> scroll line
        ScrollingLines.Line[j].LineContent = ScrollingPages.LineIn[j];
        ScrollingLines.Line[j].Width       = PixLengthLineIn; // while page scrolling this line is left aligned
        ScrollingLines.Line[j].CurrentLeft = getDisplaySizeSettings(disp_resolution).PixLeft;
        ScrollingLines.Line[j].fPixSum     = (float)getDisplaySizeSettings(disp_resolution).PixLeft;

        // pix change per scrolling line tick
        ScrollingLines.Line[j].dPix = ((float)(PixLengthLineIn - getDisplaySizeSettings(disp_resolution).Width)) / iScrollTime;

# ifdef PLUGIN_036_DEBUG
        log  = String(F("Line: ")) + String(j + 1);
        log += F(" width: ");
        log += ScrollingLines.Line[j].Width;
        log += F(" dPix: ");
        log += ScrollingLines.Line[j].dPix;
        addLog(LOG_LEVEL_INFO, log);
# endif // PLUGIN_036_DEBUG
      }
    }

    // reduce line content for page scrolling to max width
    if (PixLengthLineIn > MaxPixWidthForPageScrolling) {
      int strlen = ScrollingPages.LineIn[j].length();
# ifdef PLUGIN_036_DEBUG
      String LineInStr = ScrollingPages.LineIn[j];
# endif // PLUGIN_036_DEBUG
      float fAvgPixPerChar = ((float)PixLengthLineIn) / strlen;

      if (bLineScrollEnabled) {
        // shorten string on right side because line is displayed left aligned while scrolling
        // using floor() because otherwise empty space on right side
        iCharToRemove            = floor(((float)(PixLengthLineIn - MaxPixWidthForPageScrolling)) / fAvgPixPerChar);
        ScrollingPages.LineIn[j] = ScrollingPages.LineIn[j].substring(0, strlen - iCharToRemove);
      }
      else {
        // shorten string on both sides because line is displayed centered
        // using floor() because otherwise empty space on both sides
        iCharToRemove            = floor(((float)(PixLengthLineIn - MaxPixWidthForPageScrolling)) / (2 * fAvgPixPerChar));
        ScrollingPages.LineIn[j] = ScrollingPages.LineIn[j].substring(0, strlen - iCharToRemove);
        ScrollingPages.LineIn[j] = ScrollingPages.LineIn[j].substring(iCharToRemove);
      }
# ifdef PLUGIN_036_DEBUG
      log  = String(F("Line: ")) + String(j + 1);
      log += String(F(" LineIn: ")) + String(LineInStr);
      log += String(F(" Length: ")) + String(strlen);
      log += String(F(" PixLength: ")) + String(PixLengthLineIn);
      log += String(F(" AvgPixPerChar: ")) + String(fAvgPixPerChar);
      log += String(F(" CharsRemoved: ")) + String(iCharToRemove);
      addLog(LOG_LEVEL_INFO, log);
      log  = String(F(" -> Changed to: ")) + String(ScrollingPages.LineIn[j]);
      log += String(F(" Length: ")) + String(ScrollingPages.LineIn[j].length());
      log += String(F(" PixLength: ")) + String(display->getStringWidth(ScrollingPages.LineIn[j]));
      addLog(LOG_LEVEL_INFO, log);
# endif // PLUGIN_036_DEBUG
    }

    // reduce line content for page scrolling to max width
    if (PixLengthLineOut > MaxPixWidthForPageScrolling) {
      int strlen = ScrollingPages.LineOut[j].length();
# ifdef PLUGIN_036_DEBUG
      String LineOutStr = ScrollingPages.LineOut[j];
# endif // PLUGIN_036_DEBUG
      float fAvgPixPerChar = ((float)PixLengthLineOut) / strlen;

      if (bLineScrollEnabled) {
        // shorten string on left side because line is displayed right aligned while scrolling
        // using ceil() because otherwise overlapping the new text
        iCharToRemove             = ceil(((float)(PixLengthLineOut - MaxPixWidthForPageScrolling)) / fAvgPixPerChar);
        ScrollingPages.LineOut[j] = ScrollingPages.LineOut[j].substring(iCharToRemove);

        if (display->getStringWidth(ScrollingPages.LineOut[j]) > MaxPixWidthForPageScrolling) {
          // remove one more character because still overlapping the new text
          ScrollingPages.LineOut[j] = ScrollingPages.LineOut[j].substring(1, iCharToRemove - 1);
        }
      }
      else {
        // shorten string on both sides because line is displayed centered
        // using ceil() because otherwise overlapping the new text
        iCharToRemove             = ceil(((float)(PixLengthLineOut - MaxPixWidthForPageScrolling)) / (2 * fAvgPixPerChar));
        ScrollingPages.LineOut[j] = ScrollingPages.LineOut[j].substring(0, strlen - iCharToRemove);
        ScrollingPages.LineOut[j] = ScrollingPages.LineOut[j].substring(iCharToRemove);
      }
# ifdef PLUGIN_036_DEBUG
      log  = String(F("Line: ")) + String(j + 1);
      log += String(F(" LineOut: ")) + String(LineOutStr);
      log += String(F(" Length: ")) + String(strlen);
      log += String(F(" PixLength: ")) + String(PixLengthLineOut);
      log += String(F(" AvgPixPerChar: ")) + String(fAvgPixPerChar);
      log += String(F(" CharsRemoved: ")) + String(iCharToRemove);
      addLog(LOG_LEVEL_INFO, log);
      log  = String(F(" -> Changed to: ")) + String(ScrollingPages.LineOut[j]);
      log += String(F(" Length: ")) + String(ScrollingPages.LineOut[j].length());
      log += String(F(" PixLength: ")) + String(display->getStringWidth(ScrollingPages.LineOut[j]));
      addLog(LOG_LEVEL_INFO, log);
# endif // PLUGIN_036_DEBUG
    }
  }

  ScrollingPages.dPix    = P36_PageScrollPix * static_cast<int>(lscrollspeed); // pix change per scrolling page tick
  ScrollingPages.dPixSum = ScrollingPages.dPix;

  display->setColor(BLACK);
  // We allow 12 pixels at the top because otherwise the wifi indicator gets too squashed!!
  // scrolling window is 42 pixels high - ie 64 less margin of 12 at top and 10 at bottom
  display->fillRect(0, GetHeaderHeight() + TopLineOffset, P36_MaxDisplayWidth, GetIndicatorTop() - GetHeaderHeight());
  display->setColor(WHITE);

  if (!bHideHeader) {
    display->drawLine(0,
                      GetHeaderHeight() + TopLineOffset,
                      P36_MaxDisplayWidth,
                      GetHeaderHeight() + TopLineOffset); // line below title
  }

  for (uint8_t j = 0; j < ScrollingPages.linesPerFrame; j++)
  {
    if (lscrollspeed < ePageScrollSpeed::ePSS_Instant) { // scrolling
      if (ScrollingLines.Line[j].LastWidth > 0) {
        // width of LineOut[j] > display width -> line at beginning of scrolling page is right aligned
        display->setTextAlignment(TEXT_ALIGN_RIGHT);
        display->drawString(P36_MaxDisplayWidth - getDisplaySizeSettings(disp_resolution).PixLeft + ScrollingPages.dPixSum,
                            ScrollingPages.ypos[j],
                            ScrollingPages.LineOut[j]);
      }
      else {
        // line at beginning of scrolling page is centered
        display->setTextAlignment(TEXT_ALIGN_CENTER);
        display->drawString(P36_DisplayCentre + ScrollingPages.dPixSum,
                            ScrollingPages.ypos[j],
                            ScrollingPages.LineOut[j]);
      }
    }

    if (ScrollingLines.Line[j].Width > 0) {
      // width of LineIn[j] > display width -> line at end of scrolling page should be left aligned
      display->setTextAlignment(TEXT_ALIGN_LEFT);
      display->drawString(-P36_MaxDisplayWidth + getDisplaySizeSettings(disp_resolution).PixLeft + ScrollingPages.dPixSum,
                          ScrollingPages.ypos[j],
                          ScrollingPages.LineIn[j]);
    }
    else {
      // line at end of scrolling page should be centered
      display->setTextAlignment(TEXT_ALIGN_CENTER);
      display->drawString(-P36_DisplayCentre + ScrollingPages.dPixSum,
                          ScrollingPages.ypos[j],
                          ScrollingPages.LineIn[j]);
    }
  }

  update_display();

  if (lscrollspeed < ePageScrollSpeed::ePSS_Instant) {
    // page scrolling (using PLUGIN_TIMER_IN)
    ScrollingPages.dPixSum += ScrollingPages.dPix;
  }
  else {
    // no page scrolling
    ScrollingPages.Scrolling = 0; // allow following line scrolling
  }
# ifdef PLUGIN_036_DEBUG
  log = F("Scrolling finished");
  addLog(LOG_LEVEL_INFO, log);
# endif // PLUGIN_036_DEBUG
  return ScrollingPages.Scrolling;
}

uint8_t P036_data_struct::display_scroll_timer() {
  if (!isInitialized()) {
    return 0;
  }

  // page scrolling (using PLUGIN_TIMER_IN)
  display->setColor(BLACK);

  // We allow 13 pixels (including underline) at the top because otherwise the wifi indicator gets too squashed!!
  // scrolling window is 42 pixels high - ie 64 less margin of 12 at top and 10 at bottom
  display->fillRect(0, GetHeaderHeight() + 1 + TopLineOffset, P36_MaxDisplayWidth, GetIndicatorTop() - GetHeaderHeight());
  display->setColor(WHITE);
  display->setFont(ScrollingPages.Font);

  for (uint8_t j = 0; j < ScrollingPages.linesPerFrame; j++)
  {
    if (ScrollingLines.Line[j].LastWidth > 0) {
      // width of LineOut[j] > display width -> line is right aligned while scrolling page
      display->setTextAlignment(TEXT_ALIGN_RIGHT);
      display->drawString(P36_MaxDisplayWidth - getDisplaySizeSettings(disp_resolution).PixLeft + ScrollingPages.dPixSum,
                          ScrollingPages.ypos[j],
                          ScrollingPages.LineOut[j]);
    }
    else {
      // line is centered while scrolling page
      display->setTextAlignment(TEXT_ALIGN_CENTER);
      display->drawString(P36_DisplayCentre + ScrollingPages.dPixSum,
                          ScrollingPages.ypos[j],
                          ScrollingPages.LineOut[j]);
    }

    if (ScrollingLines.Line[j].Width > 0) {
      // width of LineIn[j] > display width -> line is left aligned while scrolling page
      display->setTextAlignment(TEXT_ALIGN_LEFT);
      display->drawString(-P36_MaxDisplayWidth + getDisplaySizeSettings(disp_resolution).PixLeft + ScrollingPages.dPixSum,
                          ScrollingPages.ypos[j],
                          ScrollingPages.LineIn[j]);
    }
    else {
      // line is centered while scrolling page
      display->setTextAlignment(TEXT_ALIGN_CENTER);
      display->drawString(-P36_DisplayCentre + ScrollingPages.dPixSum,
                          ScrollingPages.ypos[j],
                          ScrollingPages.LineIn[j]);
    }
  }

  update_display();

  if (ScrollingPages.dPixSum < P36_MaxDisplayWidth) { // scrolling
    // page still scrolling
    ScrollingPages.dPixSum += ScrollingPages.dPix;
  }
  else {
    // page scrolling finished
    ScrollingPages.Scrolling = 0; // allow following line scrolling
    // String log = F("Scrolling finished");
    // addLog(LOG_LEVEL_INFO, log);
  }
  return ScrollingPages.Scrolling;
}

// Draw scrolling line (1pix/s)
void P036_data_struct::display_scrolling_lines() {
  if (!isInitialized()) {
    return;
  }

  // line scrolling (using PLUGIN_TEN_PER_SECOND)

  int  i;
  bool bscroll       = false;
  bool updateDisplay = false;
  int  iCurrentLeft;

  for (i = 0; i < ScrollingPages.linesPerFrame; i++) {
    if (ScrollingLines.Line[i].Width != 0) {
      display->setFont(ScrollingLines.Font);
      bscroll = true;
      break;
    }
  }

  if (bscroll) {
    ScrollingLines.wait++;

    if (ScrollingLines.wait < P36_WaitScrollLines) {
      return; // wait before scrolling line not finished
    }

    for (i = 0; i < ScrollingPages.linesPerFrame; i++) {
      if (ScrollingLines.Line[i].Width != 0) {
        // scroll this line
        ScrollingLines.Line[i].fPixSum -= ScrollingLines.Line[i].dPix;
        iCurrentLeft                    = round(ScrollingLines.Line[i].fPixSum);

        if (iCurrentLeft != ScrollingLines.Line[i].CurrentLeft) {
          // still scrolling
          ScrollingLines.Line[i].CurrentLeft = iCurrentLeft;
          updateDisplay                      = true;
          display->setColor(BLACK);
          display->fillRect(0, ScrollingLines.Line[i].ypos + 1, P36_MaxDisplayWidth,
                            ScrollingLines.Space + 1); // clearing window was too high
          display->setColor(WHITE);

          if (((ScrollingLines.Line[i].CurrentLeft - getDisplaySizeSettings(disp_resolution).PixLeft) +
               ScrollingLines.Line[i].Width) >= getDisplaySizeSettings(disp_resolution).Width) {
            display->setTextAlignment(TEXT_ALIGN_LEFT);
            display->drawString(ScrollingLines.Line[i].CurrentLeft,
                                ScrollingLines.Line[i].ypos,
                                ScrollingLines.Line[i].LineContent);
          }
          else {
            // line scrolling finished -> line is shown as aligned right
            display->setTextAlignment(TEXT_ALIGN_RIGHT);
            display->drawString(P36_MaxDisplayWidth - getDisplaySizeSettings(disp_resolution).PixLeft,
                                ScrollingPages.ypos[i],
                                ScrollingLines.Line[i].LineContent);
            ScrollingLines.Line[i].Width = 0; // Stop scrolling
          }
        }
      }
    }

    if (updateDisplay && (ScrollingPages.Scrolling == 0)) { update_display(); }
  }
}

// Draw Signal Strength Bars, return true when there was an update.
bool P036_data_struct::display_wifibars() {
  if (!isInitialized()) {
    return false;
  }
  if (bHideHeader) {  //  hide header
    return false;
  }

  const bool connected    = NetworkConnected();
  const int  nbars_filled = (WiFi.RSSI() + 100) / 12; // all bars filled if RSSI better than -46dB
  const int  newState     = connected ? nbars_filled : P36_WIFI_STATE_NOT_CONNECTED;

  if (newState == lastWiFiState) {
    return false; // nothing to do.
  }
  int x         = getDisplaySizeSettings(disp_resolution).WiFiIndicatorLeft;
  int y         = TopLineOffset;
  int size_x    = getDisplaySizeSettings(disp_resolution).WiFiIndicatorWidth;
  int size_y    = GetHeaderHeight() - 2;
  int nbars     = 5;
  int16_t width = (size_x / nbars);

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
      int16_t height = size_y * (ibar + 1) / nbars;
      int16_t xpos   = x + ibar * width;
      int16_t ypos   = y + size_y - height;

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
  Scheduler.schedule_task_device_timer(event->TaskIndex,
                                       millis() + (Settings.TaskDeviceTimer[event->TaskIndex] * 1000)); // reschedule page change
  nextFrameToDisplay    = nextFrame;
  bPageScrollDisabled   = true;                                                                         //  show next page without scrolling
  disableFrameChangeCnt = 2;                                                                            //  disable next page change in
                                                                                                        // PLUGIN_READ if
  // PLUGIN_READ was already scheduled
  P036_DisplayPage(event);                                                                              //  Display the selected page,
                                                                                                        // function needs
                                                                                                        // 65ms!
  displayTimer = PCONFIG(4);                                                                            //  Restart timer
}

void P036_data_struct::P036_DisplayPage(struct EventStruct *event)
{
  # ifdef PLUGIN_036_DEBUG
  addLog(LOG_LEVEL_INFO, F("P036_DisplayPage"));
  # endif // PLUGIN_036_DEBUG

  if (!isInitialized()) {
    return;
  }

  int NFrames; // the number of frames

  if (UserVar[event->BaseVarIndex] == 1) {
    // Display is on.
    ScrollingPages.Scrolling = 1;                                                              // page scrolling running -> no
    // line scrolling allowed
    NFrames                  = P36_Nlines / ScrollingPages.linesPerFrame;
    HeaderContent            = static_cast<eHeaderContent>(get8BitFromUL(PCONFIG_LONG(0), 8)); // Bit15-8 HeaderContent
    HeaderContentAlternative = static_cast<eHeaderContent>(get8BitFromUL(PCONFIG_LONG(0), 0)); // Bit 7-0
    // HeaderContentAlternative

    //      Now create the string for the outgoing and incoming frames
    String tmpString;
    tmpString.reserve(P36_NcharsV1);

    //      Construct the outgoing string
    for (uint8_t i = 0; i < ScrollingPages.linesPerFrame; i++)
    {
      tmpString =
        String(DisplayLinesV1[(ScrollingPages.linesPerFrame * frameCounter) + i].Content);
      ScrollingPages.LineOut[i] = P36_parseTemplate(tmpString, 20);
    }

    // now loop round looking for the next frame with some content
    //   skip this frame if all lines in frame are blank
    // - we exit the while loop if any line is not empty
    bool foundText = false;
    int  ntries    = 0;

    while (!foundText) {
      //        Stop after framecount loops if no data found
      ntries += 1;

      if (ntries > NFrames) { break; }

      if (nextFrameToDisplay == 0xff) {
        // Increment the frame counter
        frameCounter++;

        if (frameCounter > NFrames - 1) {
          frameCounter          = 0;
          currentFrameToDisplay = 0;
        }
      }
      else {
        // next frame because content changed in PLUGIN_WRITE
        frameCounter = nextFrameToDisplay;
      }

      //        Contruct incoming strings
      for (uint8_t i = 0; i < ScrollingPages.linesPerFrame; i++)
      {
        tmpString =
          String(DisplayLinesV1[(ScrollingPages.linesPerFrame * frameCounter) + i].Content);
        ScrollingPages.LineIn[i] = P36_parseTemplate(tmpString, 20);

        if (ScrollingPages.LineIn[i].length() > 0) { foundText = true; }
      }

      if (foundText) {
        if (nextFrameToDisplay == 0xff) {
          if (frameCounter != 0) {
            ++currentFrameToDisplay;
          }
        }
        else { currentFrameToDisplay = nextFrameToDisplay; }
      }
    }
    nextFrameToDisplay = 0xFF;

    // Update max page count
    if (MaxFramesToDisplay == 0xFF) {
      // not updated yet
      for (uint8_t i = 0; i < NFrames; i++) {
        for (uint8_t k = 0; k < ScrollingPages.linesPerFrame; k++)
        {
          tmpString = String(DisplayLinesV1[(ScrollingPages.linesPerFrame * i) + k].Content);
          tmpString = P36_parseTemplate(tmpString, 20);

          if (tmpString.length() > 0) {
            // page not empty
            if (MaxFramesToDisplay == 0xFF) {
              MaxFramesToDisplay = 0;
            } else {
              MaxFramesToDisplay++;
            }
            break;
          }
        }
      }

      if (MaxFramesToDisplay == 0xFF) {
        // nothing to display
        MaxFramesToDisplay = 0;
      }
    }

    //      Update display
    if (bDisplayingLogo) {
      bDisplayingLogo = false;
      display->clear(); // resets all pixels to black
    }

    bAlternativHeader = false; // start with first header content
    HeaderCount       = 0;     // reset header count
    display_header();

    display_indicator();

    update_display();

    bool bScrollWithoutWifi = bitRead(PCONFIG_LONG(0), 24);                            // Bit 24
    bool bScrollLines       = bitRead(PCONFIG_LONG(0), 17);                            // Bit 17
    bLineScrollEnabled = (bScrollLines && (NetworkConnected() || bScrollWithoutWifi)); // scroll lines only if WifiIsConnected,
    // otherwise too slow

    ePageScrollSpeed lscrollspeed = static_cast<ePageScrollSpeed>(PCONFIG(3));

    if (bPageScrollDisabled) { lscrollspeed = ePageScrollSpeed::ePSS_Instant; // first page after INIT without scrolling
    }
    int lTaskTimer = Settings.TaskDeviceTimer[event->TaskIndex];

    if (display_scroll(lscrollspeed, lTaskTimer)) {
      Scheduler.setPluginTaskTimer(P36_PageScrollTimer, event->TaskIndex, event->Par1); // calls next page scrollng tick
    }

    if (NetworkConnected() || bScrollWithoutWifi) {
      // scroll lines only if WifiIsConnected, otherwise too slow
      bPageScrollDisabled = false; // next PLUGIN_READ will do page scrolling
    }
  } else {
    # ifdef PLUGIN_036_DEBUG
    addLog(LOG_LEVEL_INFO, F("P036_DisplayPage Display off"));
    # endif // PLUGIN_036_DEBUG
  }
}

// Perform some specific changes for OLED display
String P036_data_struct::P36_parseTemplate(String& tmpString, uint8_t lineSize) {
  String result = parseTemplate_padded(tmpString, lineSize);

  // OLED lib uses this routine to convert UTF8 to extended ASCII
  // http://playground.arduino.cc/Main/Utf8ascii
  // Attempt to display euro sign (FIXME)

  /*
     const char euro[4] = {0xe2, 0x82, 0xac, 0}; // Unicode euro symbol
     const char euro_oled[3] = {0xc2, 0x80, 0}; // Euro symbol OLED display font
     result.replace(euro, euro_oled);
   */
  result.trim();
  return result;
}

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

#endif // ifdef USES_P036
