#ifndef PLUGINSTRUCTS_P036_DATA_STRUCT_H
#define PLUGINSTRUCTS_P036_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P036

#include <SSD1306.h>
#include <SH1106Wire.h>


// #define PLUGIN_036_DEBUG    // additional debug messages in the log


#define P36_Nlines 12       // The number of different lines which can be displayed - each line is 64 chars max
#define P36_NcharsV0 32     // max chars per line up to 22.11.2019 (V0)
#define P36_NcharsV1 64     // max chars per line from 22.11.2019 (V1)
#define P36_MaxSizesCount 3 // number of different OLED sizes
#define P36_MaxFontCount 4  // number of different fonts

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

enum class eHeaderContent {
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

enum class p036_resolution {
  pix128x64 = 0,
  pix128x32 = 1,
  pix64x48  = 2
};

enum class ePageScrollSpeed {
  ePSS_VerySlow = 1, // 800ms
  ePSS_Slow     = 2, // 400ms
  ePSS_Fast     = 4, // 200ms
  ePSS_VeryFast = 8, // 100ms
  ePSS_Instant  = 32 // 20ms
};

enum class eP036pinmode {
  ePPM_Input          = 0,
  ePPM_InputPullUp    = 1,
  ePPM_InputPullDown  = 2
};

typedef struct {
  String   LineContent;       // content
  int      CurrentLeft = 0;   // current left pix position
  float    dPix        = 0.0f; // pix change per scroll time (100ms)
  float    fPixSum     = 0.0f; // pix sum while scrolling (100ms)
  uint16_t LastWidth   = 0;   // width of last line in pix
  uint16_t Width       = 0;   // width in pix
  uint8_t  Height      = 0;   // Height in Pix
  uint8_t  ypos        = 0;   // y position in pix
} tScrollLine;

typedef struct {
  tScrollLine Line[P36_MAX_LinesPerPage];
  const char *Font  = nullptr; // font for this line setting
  uint16_t    wait  = 0;       // waiting time before scrolling
  uint8_t     Space = 0;       // space in pix between lines for this line setting
} tScrollingLines;

typedef struct {
  String LineIn[P36_MAX_LinesPerPage];
  String LineOut[P36_MAX_LinesPerPage];
  int         ypos[P36_MAX_LinesPerPage] = { 0 };   // ypos contains the heights of the various lines - this depends on the font and the
                                                    // number of lines
  int         dPixSum                    = 0;       // act pix change
  const char *Font                       = nullptr; // font for this line setting
  uint8_t     Scrolling                  = 0;       // 0=Ready, 1=Scrolling
  uint8_t     dPix                       = 0;       // pix change per scroll time (25ms)
  uint8_t     linesPerFrame              = 0;       // the number of lines in each frame
} tScrollingPages;

typedef struct {
  char    Content[P36_NcharsV1] = { 0 };
  uint8_t FontType              = 0;
  uint8_t FontHeight            = 0;
  uint8_t FontSpace             = 0;
  uint8_t reserved              = 0;
} tDisplayLines;

typedef struct {
  const char  *fontData;  // font
  uint8_t     Width;      // font width in pix
  uint8_t     Height;     // font height in pix
} tFontSizes;

typedef struct {
  const char *fontData; // font for this line setting
  uint8_t     Top;      // top in pix for this line setting
  uint8_t     Height;   // font height in pix
  uint8_t     Space;    // space in pix between lines for this line setting
} tFontSettings;

typedef struct {
  uint8_t       Width;              // width in pix
  uint8_t       Height;             // height in pix
  uint8_t       PixLeft;            // first left pix position
  uint8_t       MaxLines;           // max. line count
  uint8_t       WiFiIndicatorLeft;  // left of WiFi indicator
  uint8_t       WiFiIndicatorWidth; // width of WiFi indicator
} tSizeSettings;

struct P036_data_struct : public PluginTaskData_base {
  P036_data_struct();

  virtual ~P036_data_struct();

  void                        reset();

  static const tSizeSettings& getDisplaySizeSettings(p036_resolution disp_resolution);

  bool init(taskIndex_t      taskIndex,
            uint8_t          LoadVersion,
            uint8_t          Type,
            uint8_t          Address,
            uint8_t          Sda,
            uint8_t          Scl,
            p036_resolution  Disp_resolution,
            bool             Rotated,
            uint8_t          Contrast,
            uint8_t          DisplayTimer,
            uint8_t          NrLines);

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
  uint8_t display_scroll(ePageScrollSpeed lscrollspeed,
                         int lTaskTimer);
  uint8_t display_scroll_timer();

  // Draw scrolling line (1pix/s)
  void    display_scrolling_lines();

  // Draw Signal Strength Bars, return true when there was an update.
  bool    display_wifibars();

  // Perform the actual write to the display.
  void    update_display();

  // get pixel positions
  int16_t GetHeaderHeight();
  int16_t GetIndicatorTop();
  tFontSettings CalculateFontSettings(uint8_t _defaultLines);

  void    P036_JumpToPage(struct EventStruct *event,
                          uint8_t             nextFrame);

  void    P036_DisplayPage(struct EventStruct *event);

  // Perform some specific changes for OLED display
  String  P36_parseTemplate(String& tmpString,
                            uint8_t lineSize);

  void    registerButtonState(uint8_t newButtonState, bool bPin3Invers);

  void    markButtonStateProcessed();

  // Instantiate display here - does not work to do this within the INIT call
  OLEDDisplay *display = nullptr;

  tScrollingLines ScrollingLines;
  tScrollingPages ScrollingPages;

  // CustomTaskSettings
  tDisplayLines DisplayLinesV1[P36_Nlines]; // holds the CustomTaskSettings for V1

  int8_t lastWiFiState = 0;
  bool bDisplayingLogo = false;

  // display
  p036_resolution  disp_resolution   = p036_resolution::pix128x64;
  uint8_t          TopLineOffset      = 0; // Offset for top line, used for rotated image while using displays < P36_MaxDisplayHeight lines
  bool             bLineScrollEnabled = false;
  // Display button
  bool    ButtonState     = false;         // button not touched
  uint8_t ButtonLastState = 0;             // Last state checked (debouncing in progress)
  uint8_t DebounceCounter = 0;             // debounce counter
  uint8_t RepeatCounter   = 0;             // Repeat delay counter when holding button pressed
  uint8_t displayTimer    = 0;             // counter for display OFF
  // frame header
  uint16_t       HeaderCount       = 0;
  eHeaderContent HeaderContent = eHeaderContent::eSSID;
  eHeaderContent HeaderContentAlternative = eHeaderContent::eSSID;
  bool           bHideHeader = false;
  bool           bHideFooter = false;
  bool           bAlternativHeader = false;

  // frames
  uint8_t MaxFramesToDisplay    = 0;    // total number of frames to display
  uint8_t currentFrameToDisplay = 0;
  uint8_t nextFrameToDisplay    = 0;    // next frame because content changed in PLUGIN_WRITE
  uint8_t frameCounter          = 0;    // need to keep track of framecounter from call to call
  uint8_t disableFrameChangeCnt = 0;    // counter to disable frame change after JumpToPage in case PLUGIN_READ already scheduled
  bool    bPageScrollDisabled   = true; // first page after INIT or after JumpToPage without scrolling
};

#endif
#endif // ifndef PLUGINSTRUCTS_P036_DATA_STRUCT_H
