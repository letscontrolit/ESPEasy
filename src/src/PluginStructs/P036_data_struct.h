#ifndef PLUGINSTRUCTS_P036_DATA_STRUCT_H
#define PLUGINSTRUCTS_P036_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#include "../../ESPEasy_common.h"

//#define PLUGIN_036_DEBUG    // additional debug messages in the log


#define P36_Nlines 12       // The number of different lines which can be displayed - each line is 64 chars max
#define P36_NcharsV0 32     // max chars per line up to 22.11.2019 (V0)
#define P36_NcharsV1 64     // max chars per line from 22.11.2019 (V1)
#define P36_MaxSizesCount 3 // number of different OLED sizes

#define P36_MaxDisplayWidth 128
#define P36_MaxDisplayHeight 64
#define P36_DisplayCentre 64
#define P36_HeaderHeight 12
#define P036_IndicatorTop 54
#define P036_IndicatorHeight 10

#define P36_CONTRAST_OFF    1
#define P36_CONTRAST_LOW    64
#define P36_CONTRAST_MED  0xCF
#define P36_CONTRAST_HIGH 0xFF


#include "SSD1306.h"
#include "SH1106Wire.h"
#include "OLED_SSD1306_SH1106_images.h"
#include "Dialog_Plain_12_font.h"

#define P36_WIFI_STATE_UNSET          -2
#define P36_WIFI_STATE_NOT_CONNECTED  -1
#define P36_MAX_LinesPerPage          4
#define P36_WaitScrollLines           5                          // wait 0.5s before and after scrolling line
#define P36_PageScrollTimer           25                         // timer in msec for page Scrolling
#define P36_PageScrollTick            (P36_PageScrollTimer + 20) // total time for one PageScrollTick (including the handling time of 20ms
                                                                 // in PLUGIN_TIMER_IN)
#define P36_PageScrollPix             4                          // min pixel change while page scrolling
#define P36_DebounceTreshold          5                          // number of 20 msec (fifty per second) ticks before the button has settled
#define P36_RepeatDelay               50                         // number of 20 msec ticks before repeating the button action when holding
                                                                 // it pressed

enum eHeaderContent {
  eSSID     = 1,
  eSysName  = 2,
  eIP       = 3,
  eMAC      = 4,
  eRSSI     = 5,
  eBSSID    = 6,
  eWiFiCh   = 7,
  eUnit     = 8,
  eSysLoad  = 9,
  eSysHeap  = 10,
  eSysStack = 11,
  eTime     = 12,
  eDate     = 13,
  ePageNo   = 14,
};

typedef struct {
  uint8_t     Top;      // top in pix for this line setting
  const char *fontData; // font for this line setting
  uint8_t     Space;    // space in pix between lines for this line setting
} tFontSettings;

typedef struct {
  uint8_t       Width;              // width in pix
  uint8_t       Height;             // height in pix
  uint8_t       PixLeft;            // first left pix position
  uint8_t       MaxLines;           // max. line count
  tFontSettings L1;                 // settings for 1 line
  tFontSettings L2;                 // settings for 2 lines
  tFontSettings L3;                 // settings for 3 lines
  tFontSettings L4;                 // settings for 4 lines
  uint8_t       WiFiIndicatorLeft;  // left of WiFi indicator
  uint8_t       WiFiIndicatorWidth; // width of WiFi indicator
} tSizeSettings;

const tSizeSettings SizeSettings[P36_MaxSizesCount] = {
   { P36_MaxDisplayWidth, P36_MaxDisplayHeight, 0,   // 128x64
     4,
     // page scrolling height = 42
     { 19, ArialMT_Plain_24, 28},  //  Width: 24 Height: 28
     { 15, ArialMT_Plain_16, 19},  //  Width: 16 Height: 19
     { 12, Dialog_plain_12,  14},  //  Width: 13 Height: 15
     { 12, ArialMT_Plain_10, 10},  //  Width: 10 Height: 13
     113,
     15
   },
   { P36_MaxDisplayWidth, 32, 0,               // 128x32
     2,
     // page scrolling height = 20
     { 14, Dialog_plain_12,  15},  //  Width: 13 Height: 15
     { 12, ArialMT_Plain_10, 10},  //  Width: 10 Height: 13
     {  0, ArialMT_Plain_10,  0},  //  Width: 10 Height: 13 not used!
     {  0, ArialMT_Plain_10,  0},  //  Width: 10 Height: 13 not used!
     113,
     15
   },
   { 64, 48, 32,               // 64x48
     3,
     // page scrolling height = 36
     { 20, ArialMT_Plain_24, 28},  //  Width: 24 Height: 28
     { 14, Dialog_plain_12,  17},  //  Width: 13 Height: 15
     { 13, ArialMT_Plain_10, 11},  //  Width: 10 Height: 13
     {  0, ArialMT_Plain_10,  0},  //  Width: 10 Height: 13 not used!
     32,
     10
   }
};

enum ePageScrollSpeed {
  ePSS_VerySlow = 1, // 800ms
  ePSS_Slow     = 2, // 400ms
  ePSS_Fast     = 4, // 200ms
  ePSS_VeryFast = 8, // 100ms
  ePSS_Instant  = 32 // 20ms
};

typedef struct {
  String   LineContent;       // content
  uint16_t LastWidth   = 0;   // width of last line in pix
  uint16_t Width       = 0;   // width in pix
  uint8_t  Height      = 0;   // Height in Pix
  uint8_t  ypos        = 0;   // y position in pix
  int      CurrentLeft = 0;   // current left pix position
  float    dPix        = 0.0; // pix change per scroll time (100ms)
  float    fPixSum     = 0.0; // pix sum while scrolling (100ms)
} tScrollLine;

typedef struct {
  const char *Font  = nullptr; // font for this line setting
  uint8_t     Space = 0;       // space in pix between lines for this line setting
  uint16_t    wait  = 0;       // waiting time before scrolling
  tScrollLine Line[P36_MAX_LinesPerPage];
} tScrollingLines;

typedef struct {
  uint8_t     Scrolling                  = 0;       // 0=Ready, 1=Scrolling
  const char *Font                       = nullptr; // font for this line setting
  uint8_t     dPix                       = 0;       // pix change per scroll time (25ms)
  int         dPixSum                    = 0;       // act pix change
  uint8_t     linesPerFrame              = 0;       // the number of lines in each frame
  int         ypos[P36_MAX_LinesPerPage] = { 0 };   // ypos contains the heights of the various lines - this depends on the font and the
                                                    // number of lines
  String LineIn[P36_MAX_LinesPerPage];
  String LineOut[P36_MAX_LinesPerPage];
} tScrollingPages;

typedef struct {
  char    Content[P36_NcharsV1] = { 0 };
  uint8_t FontType              = 0;
  uint8_t FontHeight            = 0;
  uint8_t FontSpace             = 0;
  uint8_t reserved              = 0;
} tDisplayLines;


struct P036_data_struct : public PluginTaskData_base {
  P036_data_struct();

  ~P036_data_struct();

  void reset();

  bool init(uint8_t _type,
            uint8_t _address,
            uint8_t _sda,
            uint8_t _scl);

  bool isInitialized() const;

  void loadDisplayLines(taskIndex_t taskIndex,
                        uint8_t     LoadVersion);

  // Set the display contrast
  // really low brightness & contrast: contrast = 10, precharge = 5, comdetect = 0
  // normal brightness & contrast:  contrast = 100
  void setContrast(uint8_t OLED_contrast);

  void setOrientationRotated(bool rotated);


  // The screen is set up as:
  // - 10 rows at the top for the header
  // - 44 rows in the middle for the scroll region
  // - 10 rows at the bottom for the footer
  void    display_header();
  void    display_time();
  void    display_title(const String& title);
  void    display_logo();
  void    display_indicator();
  void    prepare_pagescrolling();
  uint8_t display_scroll(int lscrollspeed,
                         int lTaskTimer);
  uint8_t display_scroll_timer();

  // Draw scrolling line (1pix/s)
  void    display_scrolling_lines();

  // Draw Signal Strength Bars, return true when there was an update.
  bool    display_wifibars();

  // Perform the actual write to the display.
  void    update_display();

  void    P036_JumpToPage(struct EventStruct *event,
                          uint8_t             nextFrame);

  void    P036_DisplayPage(struct EventStruct *event);

  // Perform some specific changes for OLED display
  String  P36_parseTemplate(String& tmpString,
                            uint8_t lineSize);


  // Instantiate display here - does not work to do this within the INIT call
  OLEDDisplay *display = nullptr;

  tScrollingLines ScrollingLines;
  tScrollingPages ScrollingPages;

  // CustomTaskSettings
  tDisplayLines DisplayLinesV1[P36_Nlines]; // holds the CustomTaskSettings for V1

  int8_t lastWiFiState = 0;

  // display
  uint8_t OLEDIndex          = 0;
  boolean bLineScrollEnabled = false;
  uint8_t TopLineOffset      = 0;  // Offset for top line, used for rotated image while using displays < P36_MaxDisplayHeight lines
  // Display button
  boolean ButtonState     = false; // button not touched
  uint8_t ButtonLastState = 0;     // Last state checked (debouncing in progress)
  uint8_t DebounceCounter = 0;     // debounce counter
  uint8_t RepeatCounter   = 0;     // Repeat delay counter when holding button pressed
  uint8_t displayTimer    = 0;     // counter for display OFF
  // frame header
  boolean        bAlternativHeader = false;
  uint16_t       HeaderCount       = 0;
  eHeaderContent HeaderContent;
  eHeaderContent HeaderContentAlternative;

  // frames
  uint8_t MaxFramesToDisplay    = 0;    // total number of frames to display
  uint8_t currentFrameToDisplay = 0;
  uint8_t nextFrameToDisplay    = 0;    // next frame because content changed in PLUGIN_WRITE
  uint8_t frameCounter          = 0;    // need to keep track of framecounter from call to call
  uint8_t disableFrameChangeCnt = 0;    // counter to disable frame change after JumpToPage in case PLUGIN_READ already scheduled
  boolean bPageScrollDisabled   = true; // first page after INIT or after JumpToPage without scrolling
};


#endif // ifndef PLUGINSTRUCTS_P036_DATA_STRUCT_H
