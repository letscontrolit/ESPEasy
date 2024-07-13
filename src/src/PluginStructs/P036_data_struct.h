#ifndef PLUGINSTRUCTS_P036_DATA_STRUCT_H
#define PLUGINSTRUCTS_P036_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P036
# include "../Helpers/OLed_helper.h"

# include <SSD1306.h>
# include <SH1106Wire.h>

# include <vector>

# if defined(LIMIT_BUILD_SIZE) || defined(PLUGIN_BUILD_IR)
#  define P036_LIMIT_BUILD_SIZE
# endif // ifdef LIMIT_BUILD_SIZE

// Macros
# define P036_DisplayIsOn (UserVar[event->BaseVarIndex] > 0)
# define P036_SetDisplayOn(_state) (UserVar.setFloat(event->TaskIndex, 0, _state))

// # define PLUGIN_036_DEBUG    // additional debug messages in the log
// # define P036_FONT_CALC_LOG  // Enable to add extra logging during font calculation (selection)
// # define P036_SCROLL_CALC_LOG   // Enable to add extra logging during scrolling calculation (selection)
// # define P036_CHECK_HEAP        // Enable to add extra logging during Plugin_036()
// # define P036_CHECK_INDIVIDUAL_FONT // Enable to add extra logging for individual font calculation
# ifndef P036_FEATURE_DISPLAY_PREVIEW
#  define P036_FEATURE_DISPLAY_PREVIEW   1
# endif // ifndef P036_FEATURE_DISPLAY_PREVIEW
# ifndef P036_FEATURE_ALIGN_PREVIEW
#  define P036_FEATURE_ALIGN_PREVIEW     1
# endif // ifdef P036_FEATURE_ALIGN_PREVIEW

# if defined(ESP8266_1M) && defined(P036_FEATURE_ALIGN_PREVIEW) && P036_FEATURE_ALIGN_PREVIEW
#  undef P036_FEATURE_ALIGN_PREVIEW
#  define P036_FEATURE_ALIGN_PREVIEW   0 // Disable for 1M builds
# endif // if defined(ESP8266_1M) && defined(P036_FEATURE_ALIGN_PREVIEW) && P036_FEATURE_ALIGN_PREVIEW

# ifndef P036_LIMIT_BUILD_SIZE
#  ifndef P036_SEND_EVENTS
#   define P036_SEND_EVENTS       1 // Enable sending events on Display On/Off, Contrast Low/Med/High, Frame and Line
#  endif // ifndef P036_SEND_EVENTS
#  ifndef P036_ENABLE_LINECOUNT
#   define P036_ENABLE_LINECOUNT  1 // Enable the linecount subcommand
#  endif // ifndef P036_ENABLE_LINECOUNT
#  ifndef P036_USERDEF_HEADERS
#   define P036_USERDEF_HEADERS   1 // Enable User defined headers
#  endif // ifndef P036_USERDEF_HEADERS
#  ifndef P036_ENABLE_TICKER
#   define P036_ENABLE_TICKER   1   // Enable ticker function
#  endif // ifndef
# else // ifndef P036_LIMIT_BUILD_SIZE
#  if defined(P036_SEND_EVENTS) && P036_SEND_EVENTS
#   undef P036_SEND_EVENTS
#  endif // if defined(P036_SEND_EVENTS) && P036_SEND_EVENTS
#  ifndef P036_SEND_EVENTS
#   define P036_SEND_EVENTS       0 // Disable sending events
#  endif // ifndef P036_SEND_EVENTS
#  if defined(P036_ENABLE_LINECOUNT) && P036_ENABLE_LINECOUNT
#   undef P036_ENABLE_LINECOUNT
#  endif // if defined(P036_ENABLE_LINECOUNT) && P036_ENABLE_LINECOUNT
#  ifndef P036_ENABLE_LINECOUNT
#   define P036_ENABLE_LINECOUNT  0 // Disable the linecount subcommand
#  endif // ifndef P036_ENABLE_LINECOUNT
// We can always disable this feature later, if needed
// #  if defined(P036_USERDEF_HEADERS) && P036_USERDEF_HEADERS
// #   undef P036_USERDEF_HEADERS
// #  endif // if defined(P036_USERDEF_HEADERS) && P036_USERDEF_HEADERS
// #  ifndef P036_USERDEF_HEADERS
// #   define P036_USERDEF_HEADERS   0 // Disable User defined headers
// #  endif // ifndef P036_USERDEF_HEADERS
#  ifndef P036_ENABLE_TICKER
#   define P036_ENABLE_TICKER   0 // Disable ticker function
#  endif // ifndef
# endif // ifndef P036_LIMIT_BUILD_SIZE
# ifndef P036_USERDEF_HEADERS
#  define P036_USERDEF_HEADERS   1  // Enable User defined headers if not handled yet
# endif // ifndef P036_USERDEF_HEADERS
# ifndef P036_ENABLE_HIDE_FOOTER
#  define P036_ENABLE_HIDE_FOOTER 1 // Enable the Hide indicator (footer) option
# endif // ifndef P036_ENABLE_HIDE_FOOTER
# ifndef P036_ENABLE_LEFT_ALIGN
#  define P036_ENABLE_LEFT_ALIGN  1 // Enable the Left-align content option and leftalign subcommand
# endif // ifndef P036_ENABLE_LEFT_ALIGN

# define P36_Nlines 12              // The number of different lines which can be displayed - each line is 64 chars max
# define P36_NcharsV0 32            // max chars per line up to 22.11.2019 (V0)
# define P36_NcharsV1 64            // max chars per line from 22.11.2019 (V1)

# define P36_MaxDisplayWidth  128
# define P36_MaxDisplayHeight 64
# define P36_DisplayCentre    64
# define P36_HeaderHeight     12
# define P036_IndicatorTop    56
# define P036_IndicatorHeight 8

# define P36_WIFI_STATE_UNSET          -2
# define P36_WIFI_STATE_NOT_CONNECTED  -1
# define P36_MAX_LinesPerPage          4
# define P36_WaitScrollLines           5                          // wait 0.5s before and after scrolling line
# define P36_PageScrollTimer           25                         // timer in msec for page Scrolling
# define P36_PageScrollTick            (P36_PageScrollTimer + 20) // total time for one PageScrollTick (including the handling time of 20ms
                                                                  // in PLUGIN_TIMER_IN)
# define P36_PageScrollPix             4                          // min pixel change while page scrolling
# define P36_DebounceTreshold          5                          // number of 20 msec (fifty per second) ticks before the button has
                                                                  // settled
# define P36_RepeatDelay               50                         // number of 20 msec ticks before repeating the button action when holding

# define P036_ADR         PCONFIG(0)
# define P036_ROTATE      PCONFIG(1)
# define P036_NLINES      PCONFIG(2)
# define P036_SCROLL      PCONFIG(3)
# define P036_TIMER       PCONFIG(4)
# define P036_CONTROLLER  PCONFIG(5)
# define P036_CONTRAST    PCONFIG(6)
# define P036_RESOLUTION  PCONFIG(7)

# define P036_FLAGS_0     PCONFIG_ULONG(0)
# define P036_FLAGS_1     PCONFIG_ULONG(1)

// P036_FLAGS_0
# define P036_FLAG_HEADER_ALTERNATIVE   0 // Bit 7-0 HeaderContentAlternative
# define P036_FLAG_HEADER               8 // Bit15-8 HeaderContent
# define P036_FLAG_PIN3_INVERSE        16 // Bit 16 Pin3Invers
# define P036_FLAG_SCROLL_LINES        17 // Bit 17 ScrollLines
# define P036_FLAG_NODISPLAY_ONRECEIVE 18 // Bit 18 NoDisplayOnReceivingText
# define P036_FLAG_STEP_PAGES_BUTTON   19 // Bit 19 StepThroughPagesWithButton
# define P036_FLAG_SETTINGS_VERSION    20 // Bit23-20 Version CustomTaskSettings -> version V1
# define P036_FLAG_SCROLL_WITHOUTWIFI  24 // Bit 24 ScrollWithoutWifi
# define P036_FLAG_HIDE_HEADER         25 // Bit 25 Hide header
# define P036_FLAG_INPUT_PULLUP        26 // Bit 26 Input PullUp
// # define P036_FLAG_INPUT_PULLDOWN      27 // Bit 27 Input PullDown, 2022-09-04 not longer used
# define P036_FLAG_SEND_EVENTS         28 // Bit 28 SendEvents
# define P036_FLAG_EVENTS_FRAME_LINE   29 // Bit 29 SendEvents also on Frame & Line
# define P036_FLAG_HIDE_FOOTER         30 // Bit 30 Hide footer

// P036_FLAGS_1
# define P036_FLAG_LEFT_ALIGNED        0  // Bit1-0 Layout left aligned
# define P036_FLAG_REDUCE_LINE_NO      2  // Bit 2 Reduce line number to fit individual line font settings


# define P036_EVENT_DISPLAY  0 // event: <taskname>#display=0/1
# define P036_EVENT_CONTRAST 1 // event: <taskname>#contrast=0/1/2
# define P036_EVENT_FRAME    2 // event: <taskname>#frame=1..n
# define P036_EVENT_LINE     3 // event: <taskname>#line=1..n
# define P036_EVENT_LINECNT  4 // event: <taskname>#linecount=1..4
# define P036_EVENT_RESTORE  5 // event: <taskname>#restore=1..n
# define P036_EVENT_SCROLL   6 // event: <taskname>#scroll=ePSS_VerySlow..ePSS_Ticker


enum class eHeaderContent : uint8_t {
  eNone     = 0u,
  eSSID     = 1u,
  eSysName  = 2u,
  eIP       = 3u,
  eMAC      = 4u,
  eRSSI     = 5u,
  eBSSID    = 6u,
  eWiFiCh   = 7u,
  eUnit     = 8u,
  eSysLoad  = 9u,
  eSysHeap  = 10u,
  eSysStack = 11u,
  eTime     = 12u,
  eDate     = 13u,
  ePageNo   = 14u,
  # if P036_USERDEF_HEADERS
  eUserDef1 = 15u,
  eUserDef2 = 16u,
  # endif // if P036_USERDEF_HEADERS
};

enum class p036_resolution : uint8_t {
  pix128x64 = 0u,
  pix128x32 = 1u,
  pix64x48  = 2u
};

enum class ePageScrollSpeed : uint8_t {
  ePSS_VerySlow = 1u,  // 800ms
  ePSS_Slow     = 2u,  // 400ms
  ePSS_Fast     = 4u,  // 200ms
  ePSS_VeryFast = 8u,  // 100ms
  ePSS_Instant  = 32u, // 20ms
  ePSS_Ticker   = 255u // tickerspeed depends on line length
};

enum class eP036pinmode : uint8_t {
  ePPM_Input       = 0u,
  ePPM_InputPullUp = 1u
};

typedef struct {
  String   SLcontent;          // content
  int      CurrentLeft = 0;    // current left pix position
  float    dPix        = 0.0f; // pix change per scroll time (100ms)
  float    fPixSum     = 0.0f; // pix sum while scrolling (100ms)
  uint16_t LastWidth   = 0;    // width of last line in pix
  uint16_t Width       = 0;    // width in pix
  uint8_t  SLidx       = 0;    // index to DisplayLinesV1
  uint8_t  reserved22{};         // Fillers added to achieve better instance/memory alignment (multiple of 8)
  uint8_t  reserved23{};
  uint8_t  reserved24{};
} tScrollLine;

typedef struct {
  String   Tcontent;                // content (all parsed lines)
  uint16_t len                 = 0; // length of content
  uint16_t IdxStart            = 0; // Start index of TickerContent for displaying (left side)
  uint16_t IdxEnd              = 0; // End index of TickerContent for displaying (right side)
  uint16_t TickerAvgPixPerChar = 0; // max of average pixel per character or pix change per scroll time (100ms)
  int16_t  MaxPixLen           = 0; // Max pix length to display (display width + 2*TickerAvgPixPerChar)
  # ifdef ESP8266                   // Helpful on ESP8266 only, it seems
  uint8_t reserved15{};             // Fillers added to achieve better instance/memory alignment (multiple of 8)
  uint8_t reserved16{};
  # endif // ifdef ESP8266
} tTicker;

typedef struct {
  tScrollLine SLine[P36_MAX_LinesPerPage]{};
# if P036_ENABLE_TICKER
  tTicker Ticker;
# endif // if P036_ENABLE_TICKER
  uint16_t wait = 0; // waiting time before scrolling
} tScrollingLines;

typedef struct {
  String                     SPLcontent;    // content
  OLEDDISPLAY_TEXT_ALIGNMENT Alignment = TEXT_ALIGN_LEFT;
  uint8_t                    SPLidx    = 0; // index to DisplayLinesV1
} tScrollingPageLines;

typedef struct {
  tScrollingPageLines In[P36_MAX_LinesPerPage]{};
  tScrollingPageLines Out[P36_MAX_LinesPerPage]{};
  int                 dPixSum          = 0; // act pix change
  uint8_t             Scrolling        = 0; // 0=Ready, 1=Scrolling
  uint8_t             dPix             = 0; // pix change per scroll time (25ms per page, 100ms per line)
  uint8_t             linesPerFrameDef = 0; // the default number of lines in frame in/out
  uint8_t             linesPerFrameIn  = 0; // the number of lines in frame in
  uint8_t             linesPerFrameOut = 0; // the number of lines in frame out
} tScrollingPages;

enum class eModifyFont : uint8_t {
  eMinimize = 4u,
  eReduce   = 3u,
  eNone     = 7u, // because of compatibility to previously saved DisplayLinesV1[].ModifyLayout with 0xff
  eEnlarge  = 1u,
  eMaximize = 2u
};

enum class eAlignment : uint8_t {
  eGlobal = 7u, // because of compatibility to previously saved DisplayLinesV1[].ModifyLayout with 0xff
  eLeft   = 1u,
  eCenter = 0u,
  eRight  = 2u
};

# define P036_FLAG_ModifyLayout_Font        0 // Bit 2-0 eModifyFont
# define P036_FLAG_ModifyLayout_Alignment   3 // Bit 5-3 eAlignment

typedef struct {
  String  Content;
  uint8_t FontType     = 0;
  uint8_t ModifyLayout = 0; // Bit 2-0 eModifyFont, Bit 5-3 eAlignment
  uint8_t FontSpace    = 0;
  uint8_t reserved     = 0;
} tDisplayLines;

struct tDisplayLines_storage {
  tDisplayLines_storage() = default;

  tDisplayLines_storage(const tDisplayLines& memory) :
    FontType(memory.FontType),
    ModifyLayout(memory.ModifyLayout),
    FontSpace(memory.FontSpace),
    reserved(memory.reserved)
  {
    safe_strncpy(Content, memory.Content, P36_NcharsV1);
    ZERO_TERMINATE(Content);
  }

  tDisplayLines get() const {
    tDisplayLines res;

    res.Content      = String(Content);
    res.FontType     = FontType;
    res.ModifyLayout = ModifyLayout;
    res.FontSpace    = FontSpace;
    res.reserved     = reserved;
    return res;
  }

  char    Content[P36_NcharsV1] = { 0 };
  uint8_t FontType              = 0;
  uint8_t ModifyLayout          = 0; // Bit 2-0 eModifyFont, Bit 5-3 eAlignment
  uint8_t FontSpace             = 0;
  uint8_t reserved              = 0;
};

struct tDisplayLines_storage_full {
  tDisplayLines_storage lines[P36_Nlines]{};
};

typedef struct {
  const char *fontData; // font
  uint8_t     Width;    // font width in pix
  uint8_t     Height;   // font height in pix
} tFontSizes;

typedef struct {
  uint8_t fontIdx = 0; // font index for this line setting
  uint8_t Top     = 0; // top in pix for this line setting
  uint8_t Height  = 0; // font height in pix
  int8_t  Space   = 0; // space in pix between lines for this line setting, allow negative values to squeeze the lines closer!
# ifdef P036_FONT_CALC_LOG
  const __FlashStringHelper* FontName() const;
# endif // ifdef P036_FONT_CALC_LOG
} tFontSettings;

typedef struct {
  uint8_t Width;              // width in pix
  uint8_t Height;             // height in pix
  uint8_t PixLeft;            // first left pix position
  uint8_t MaxLines;           // max. line count
  uint8_t WiFiIndicatorLeft;  // left of WiFi indicator
  uint8_t WiFiIndicatorWidth; // width of WiFi indicator
  uint8_t reserved7;          // Fillers added to achieve better instance/memory alignment (multiple of 8)
  uint8_t reserved8;
} tSizeSettings;

typedef struct {
  uint8_t frame           = 0; // frame for this line
  uint8_t DisplayedPageNo = 0; // number of shown pages for this line, set in CalcMaxPageCount()
  uint8_t ypos            = 0; // ypos for this line
  uint8_t fontIdx         = 0; // font index for this line
  uint8_t FontHeight      = 0; // font height for this line
  # ifdef ESP8266              // Helpful on ESP8266 only, it seems
  uint8_t reserved6;           // Fillers added to achieve better instance/memory alignment (multiple of 8)
  uint8_t reserved7;
  uint8_t reserved8;
  # endif // ifdef ESP8266
} tLineSettings;

typedef struct {
  uint8_t NextLineNo            = 0; // number of next line or 0xFF if settings do not fit
  uint8_t IdxForBiggestFontUsed = 0; // ypos for this line
} tIndividualFontSettings;

class P036_LineContent {
public:

  P036_LineContent() {
    # ifdef USE_SECOND_HEAP
    HeapSelectIram ephemeral;
    # endif // ifdef USE_SECOND_HEAP

    DisplayLinesV1.resize(P36_Nlines);
  }

  void   loadDisplayLines(taskIndex_t taskIndex,
                          uint8_t     LoadVersion);

  String saveDisplayLines(taskIndex_t taskIndex);

  // CustomTaskSettings
  std::vector<tDisplayLines>DisplayLinesV1; // holds the CustomTaskSettings for V1
};

struct P036_data_struct : public PluginTaskData_base {
  P036_data_struct() = default;

  virtual ~P036_data_struct();

  void                        reset();

  static const tSizeSettings& getDisplaySizeSettings(p036_resolution disp_resolution);

  bool                        init(taskIndex_t      taskIndex,
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
                                   uint8_t          NrLines);

  bool plugin_write(struct EventStruct *event, const String& string);

  bool isInitialized() const;

  // Set the display contrast
  // really low brightness & contrast: contrast = 10, precharge = 5, comdetect = 0
  // normal brightness & contrast:  contrast = 100
  void setContrast(uint8_t OLED_contrast);

  void setOrientationRotated(bool rotated);
  # if P036_ENABLE_LINECOUNT
  void setNrLines(struct EventStruct *event,
                  uint8_t             NrLines);
  # endif // if P036_ENABLE_LINECOUNT

  // Restores line content from flash memory
  // LineNo == 0: all line contents
  // otherwise just the line content of the given LineNo
  void RestoreLineContent(taskIndex_t taskIndex,
                          uint8_t     LoadVersion,
                          uint8_t     LineNo);

private:
  String create_display_header_text(eHeaderContent iHeaderContent) const;

public:

  // The screen is set up as:
  // - 10 rows at the top for the header
  // - 46 rows in the middle for the scroll region
  // -  8 rows at the bottom for the footer
  void    display_header();
  void    display_time();
  void    display_title(const String& title);
  void    display_logo();
  void    display_indicator();
  void    prepare_pagescrolling(ePageScrollSpeed lscrollspeed,
                                uint8_t          NrLines);
  uint8_t display_scroll(ePageScrollSpeed lscrollspeed,
                         int              lTaskTimer);
  uint8_t display_scroll_timer(bool             initialScroll = false,
                               ePageScrollSpeed lscrollspeed  = ePageScrollSpeed::ePSS_Instant);

  // Draw scrolling line (1pix/s)
  void                       display_scrolling_lines();

  // Draw Signal Strength Bars, return true when there was an update.
  bool                       display_wifibars();

  // Perform the actual write to the display.
  void                       update_display();

  // get pixel positions
  int16_t                    GetHeaderHeight() const;
  int16_t                    GetIndicatorTop() const;
  tFontSettings              CalculateFontSettings(uint8_t _defaultLines);

  void                       P036_JumpToPage(struct EventStruct *event,
                                             uint8_t             nextFrame);

  void                       P036_JumpToPageOfLine(struct EventStruct *event,
                                                   uint8_t             LineNo);
  void                       P036_DisplayPage(struct EventStruct *event);

  // Perform some specific changes for OLED display
  String                     P36_parseTemplate(String& tmpString,
                                               uint8_t lineIdx);

  void                       registerButtonState(uint8_t newButtonState,
                                                 bool    bPin3Invers);

  void                       markButtonStateProcessed();

  # if P036_ENABLE_LEFT_ALIGN
  void                       setTextAlignment(eAlignment aAlignment);
  OLEDDISPLAY_TEXT_ALIGNMENT getTextAlignment(eAlignment aAlignment) const;
  uint8_t                    GetTextLeftMargin(OLEDDISPLAY_TEXT_ALIGNMENT _textAlignment) const;
  # endif // ifdef P036_ENABLE_LEFT_ALIGN

  # if P036_FEATURE_DISPLAY_PREVIEW
  bool web_show_values();
  # endif // if P036_FEATURE_DISPLAY_PREVIEW

  // Instantiate display here - does not work to do this within the INIT call
  OLEDDisplay *display = nullptr;

  tScrollingLines ScrollingLines{}; // scrolling lines in from right, out to left
  tScrollingPages ScrollingPages{}; // scrolling pages in from left, out to right

  // CustomTaskSettings
  P036_LineContent *LineContent = nullptr;

  int8_t lastWiFiState   = 0;
  bool   bDisplayingLogo = false;

  // display
  p036_resolution disp_resolution    = p036_resolution::pix128x64;
  uint8_t         TopLineOffset      = 0; // Offset for top line, used for rotated image while using displays < P36_MaxDisplayHeight lines
  bool            bLineScrollEnabled = false;

  // Display button
  bool     ButtonState     = false; // button not touched
  uint8_t  ButtonLastState = 0;     // Last state checked (debouncing in progress)
  uint8_t  DebounceCounter = 0;     // debounce counter
  uint8_t  RepeatCounter   = 0;     // Repeat delay counter when holding button pressed
  uint16_t displayTimer    = 0;     // counter for display OFF
  // frame header
  uint16_t       HeaderCount              = 0;
  eHeaderContent HeaderContent            = eHeaderContent::eSSID;
  eHeaderContent HeaderContentAlternative = eHeaderContent::eSSID;
  bool           bHideHeader              = false;
  bool           bHideFooter              = false;
  bool           bAlternativHeader        = false;
  bool           bReduceLinesPerFrame     = false;

  // frames
  uint8_t MaxFramesToDisplay    = 0;     // total number of frames to display
  uint8_t currentFrameToDisplay = 0;
  uint8_t nextFrameToDisplay    = 0;     // next frame because content changed in PLUGIN_WRITE
  uint8_t frameCounter          = 0;     // need to keep track of framecounter from call to call
  uint8_t disableFrameChangeCnt = 0;     // counter to disable frame change after JumpToPage in case PLUGIN_READ already scheduled
  bool    bPageScrollDisabled   = true;  // first page after INIT or after JumpToPage without scrolling
  bool    bRunning              = false; // page updates are rumming = (NetworkConnected() || bScrollWithoutWifi)
  # if P036_ENABLE_TICKER
  bool bUseTicker = false;               // scroll line like a ticker
  # endif // if P036_ENABLE_TICKER

  OLEDDISPLAY_TEXT_ALIGNMENT textAlignment = TEXT_ALIGN_CENTER;

  tLineSettings LineSettings[P36_Nlines]{};
  uint16_t CalcPixLength(uint8_t LineNo);

  # if P036_USERDEF_HEADERS
  String userDef1;
  String userDef2;
  # endif // if P036_USERDEF_HEADERS

private:

  tIndividualFontSettings CalculateIndividualFontSettings(uint8_t LineNo,
                                                          uint8_t FontIndex,
                                                          uint8_t LinesPerFrame,
                                                          uint8_t FrameNo,
                                                          int8_t  MaxHeight,
                                                          uint8_t IdxForBiggestFont);
  void     CalcMaxPageCount(void);
  uint16_t TrimStringTo255Chars(tScrollingPageLines *ScrollingPageLine);
  void     DrawScrollingPageLine(tScrollingPageLines       *ScrollingPageLine,
                                 uint16_t                   Width,
                                 OLEDDISPLAY_TEXT_ALIGNMENT textAlignment);
  void     CreateScrollingPageLine(tScrollingPageLines *ScrollingPageLine,
                                   uint8_t              Counter);

  # if P036_FEATURE_DISPLAY_PREVIEW
  String currentLines[P36_MAX_LinesPerPage]{};
  # endif // if P036_FEATURE_DISPLAY_PREVIEW

# if P036_SEND_EVENTS
 
public:
  static void P036_SendEvent(struct EventStruct *event, uint8_t eventId, int16_t eventValue);
#endif

};

#endif // ifdef USES_P036
#endif // ifndef PLUGINSTRUCTS_P036_DATA_STRUCT_H
