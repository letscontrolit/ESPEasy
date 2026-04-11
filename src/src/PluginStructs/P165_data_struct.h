///////////////////////////////////////////////////////////////////////////////////////////////////
// Plugin data structure for P165 Display - 7-Segment Neopixel
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PLUGINSTRUCTS_P165_DATA_STRUCT_H
#define PLUGINSTRUCTS_P165_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P165

# include <map>
# include "../Helpers/AdafruitGFX_helper.h" // Use Adafruit graphics helper object
# include "../Static/WebStaticData.h"       // Javascript and support functions

# define P165_DEBUG_INFO        1           // set 1 to enable some extra debug logging
# define P165_DEBUG_DEBUG       0           // set 1 to enable some extra development debug logging

# ifdef USES_P073
#  define P165_FEATURE_P073     1           // Use P073 shared functions and fonts when available
# else // ifdef USES_P073
#  define P165_FEATURE_P073     0
# endif // ifdef USES_P073

# if P165_FEATURE_P073 // Use shared fonts and functions from P073 Display - 7-segment display when available
#  include "../PluginStructs/P073_data_struct.h"
# endif // if P165_FEATURE_P073

# if P165_FEATURE_P073
#  ifdef P073_EXTRA_FONTS // FIXME to use #if instead of #ifdef after P073 improvements from PR #5091 are merged
#   define P165_EXTRA_FONTS 1
#  else // ifdef P073_EXTRA_FONTS
#   define P165_EXTRA_FONTS 0
#  endif // ifdef P073_EXTRA_FONTS
# endif // if P165_FEATURE_P073

# ifndef P165_FEATURE_GROUPCOLOR
#  ifndef LIMIT_BUILD_SIZE
#   define P165_FEATURE_GROUPCOLOR    1
#  else // ifndef LIMIT_BUILD_SIZE
#   define P165_FEATURE_GROUPCOLOR    1 // Enabled while it still fits in the ESP8266 Neopixel build
#  endif // ifndef LIMIT_BUILD_SIZE
# endif // ifndef P165_FEATURE_GROUPCOLOR

# ifndef P165_FEATURE_DIGITCOLOR
#  ifndef LIMIT_BUILD_SIZE
#   define P165_FEATURE_DIGITCOLOR    1
#  else // ifndef LIMIT_BUILD_SIZE
#   define P165_FEATURE_DIGITCOLOR    1 // Enabled while it still fits in the ESP8266 Neopixel build
#  endif // ifndef LIMIT_BUILD_SIZE
# endif // ifndef P165_FEATURE_DIGITCOLOR

# ifndef P165_FEATURE_C_CLOCKWISE
#  ifndef LIMIT_BUILD_SIZE
#   define P165_FEATURE_C_CLOCKWISE   1
#  else // ifndef LIMIT_BUILD_SIZE
#   define P165_FEATURE_C_CLOCKWISE   1 // Enabled while it still fits in the ESP8266 Neopixel build
#  endif // ifndef LIMIT_BUILD_SIZE
# endif // ifndef P165_FEATURE_C_CLOCKWISE

# define P165_PIXEL_CHARACTER     "&#x2638;" // The character to draw for a pixel. When changing, also update cHcrnr() in p165_digit.js

# define P165_CONFIG_GROUPCOUNT   PCONFIG(0)
# define P165_CONFIG_STRIP_TYPE   PCONFIG(1)
# define P165_CONFIG_OUTPUTTYPE   PCONFIG(2)
# define P165_CONFIG_DEF_BRIGHT   PCONFIG(3)
# define P165_CONFIG_MAX_BRIGHT   PCONFIG(4)
# define P165_CONFIG_FONTSET      PCONFIG(5)
# define P165_CONFIG_SCROLLSPEED  PCONFIG(6)
# define P165_CONFIG_FG_COLOR     PCONFIG_FLOAT(0)
# define P165_CONFIG_BG_COLOR     PCONFIG_FLOAT(1)

// Settings flags
# define P165_FLAGS               PCONFIG(7) // 15 bits

# define P165_FLAG_SUPPRESS_0     0          // 1 bit
# define P165_FLAG_NUMBERPLAN     1          // 1 bit
# define P165_FLAG_SCROLL_TEXT    2          // 1 bit
# define P165_FLAG_SCROLL_FULL    3          // 1 bit
# define P165_FLAG_STD_OFFSET     4          // 4 bit
# define P165_FLAG_CLEAR_EXIT     8          // 1 bit
# define P165_FLAG_BLINK_DOT      9          // 1 bit

# define P165_GET_FLAG_SUPP0 (bitRead(P165_FLAGS, P165_FLAG_SUPPRESS_0))
# define P165_GET_FLAG_NUMBERPLAN (bitRead(P165_FLAGS, P165_FLAG_NUMBERPLAN))
# define P165_GET_FLAG_SCROLL_TEXT (bitRead(P165_FLAGS, P165_FLAG_SCROLL_TEXT))
# define P165_GET_FLAG_SCROLL_FULL (bitRead(P165_FLAGS, P165_FLAG_SCROLL_FULL))
# define P165_GET_FLAG_STD_OFFSET (get4BitFromUL(P165_FLAGS, P165_FLAG_STD_OFFSET))
# define P165_GET_FLAG_CLEAR_EXIT (bitRead(P165_FLAGS, P165_FLAG_CLEAR_EXIT))
# define P165_GET_FLAG_BLINK_DOT (bitRead(P165_FLAGS, P165_FLAG_BLINK_DOT))

# define P165_SET_FLAG_SUPP0(V) (bitWrite(P165_FLAGS, P165_FLAG_SUPPRESS_0, V))
# define P165_SET_FLAG_NUMBERPLAN(V) (bitWrite(P165_FLAGS, P165_FLAG_NUMBERPLAN, V))
# define P165_SET_FLAG_SCROLL_TEXT(V) (bitWrite(P165_FLAGS, P165_FLAG_SCROLL_TEXT, V))
# define P165_SET_FLAG_SCROLL_FULL(V) (bitWrite(P165_FLAGS, P165_FLAG_SCROLL_FULL, V))
# define P165_SET_FLAG_STD_OFFSET(V) (set4BitToUL(P165_FLAGS, P165_FLAG_STD_OFFSET, V))
# define P165_SET_FLAG_CLEAR_EXIT(V) (bitWrite(P165_FLAGS, P165_FLAG_CLEAR_EXIT, V))
# define P165_SET_FLAG_BLINK_DOT(V) (bitWrite(P165_FLAGS, P165_FLAG_BLINK_DOT, V))

// Config per display group, all 4 PCONFIG_(U)LONG variables used
# define P165_GROUP_CFG(N) PCONFIG_ULONG(N)

# define P165_CONFIG_IDX_WPIXELS  0u  // 4 bits
# define P165_CONFIG_IDX_HPIXELS  4u  // 4 bits
# define P165_CONFIG_IDX_CORNER   8u  // 1 bit
# define P165_CONFIG_IDX_DOT      9u  // 4 bits
# define P165_CONFIG_IDX_EXTRA    13u // 4 bits
# define P165_CONFIG_IDX_OFFSET   17u // 4 bits
# define P165_CONFIG_IDX_DIGITS   21u // 3 bits
# define P165_CONFIG_IDX_START    24u // 1 bit
# define P165_CONFIG_IDX_DEND     25u // 1 bit
# define P165_CONFIG_IDX_RTLD     26u // 1 bit
# define P165_CONFIG_IDX_SPLTG    27u // 1 bit
# define P165_CONFIG_IDX_CCLKW    28u // 1 bit
# define P165_CONFIG_IDX_GSTRT    29u // 1 bit

# define P165_GET_CONFIG_WPIXELS(D) (get4BitFromUL(P165_GROUP_CFG(D), P165_CONFIG_IDX_WPIXELS))
# define P165_GET_CONFIG_HPIXELS(D) (get4BitFromUL(P165_GROUP_CFG(D), P165_CONFIG_IDX_HPIXELS))
# define P165_GET_CONFIG_CORNER(D) (bitRead(P165_GROUP_CFG(D), P165_CONFIG_IDX_CORNER))
# define P165_GET_CONFIG_DOT(D) (get4BitFromUL(P165_GROUP_CFG(D), P165_CONFIG_IDX_DOT))
# define P165_GET_CONFIG_EXTRA(D) (get4BitFromUL(P165_GROUP_CFG(D), P165_CONFIG_IDX_EXTRA))
# define P165_GET_CONFIG_OFFSET(D) (get4BitFromUL(P165_GROUP_CFG(D), P165_CONFIG_IDX_OFFSET))
# define P165_GET_CONFIG_DIGITS(D) (get3BitFromUL(P165_GROUP_CFG(D), P165_CONFIG_IDX_DIGITS))
# define P165_GET_CONFIG_START(D) (bitRead(P165_GROUP_CFG(D), P165_CONFIG_IDX_START))
# define P165_GET_CONFIG_DEND(D) (bitRead(P165_GROUP_CFG(D), P165_CONFIG_IDX_DEND))
# define P165_GET_CONFIG_RTLD(D) (bitRead(P165_GROUP_CFG(D), P165_CONFIG_IDX_RTLD))
# define P165_GET_CONFIG_SPLTG(D) (bitRead(P165_GROUP_CFG(D), P165_CONFIG_IDX_SPLTG))
# define P165_GET_CONFIG_CCLKW(D) (bitRead(P165_GROUP_CFG(D), P165_CONFIG_IDX_CCLKW))
# define P165_GET_CONFIG_GSTRT(D) (bitRead(P165_GROUP_CFG(D), P165_CONFIG_IDX_GSTRT))

# define P165_SET_CONFIG_WPIXELS(D, V) (set4BitToUL(P165_GROUP_CFG(D), P165_CONFIG_IDX_WPIXELS, V))
# define P165_SET_CONFIG_HPIXELS(D, V) (set4BitToUL(P165_GROUP_CFG(D), P165_CONFIG_IDX_HPIXELS, V))
# define P165_SET_CONFIG_CORNER(D, V) (bitWrite(P165_GROUP_CFG(D), P165_CONFIG_IDX_CORNER, V))
# define P165_SET_CONFIG_DOT(D, V) (set4BitToUL(P165_GROUP_CFG(D), P165_CONFIG_IDX_DOT, V))
# define P165_SET_CONFIG_EXTRA(D, V) (set4BitToUL(P165_GROUP_CFG(D), P165_CONFIG_IDX_EXTRA, V))
# define P165_SET_CONFIG_OFFSET(D, V) (set4BitToUL(P165_GROUP_CFG(D), P165_CONFIG_IDX_OFFSET, V))
# define P165_SET_CONFIG_DIGITS(D, V) (set3BitToUL(P165_GROUP_CFG(D), P165_CONFIG_IDX_DIGITS, V))
# define P165_SET_CONFIG_START(D, V) (bitWrite(P165_GROUP_CFG(D), P165_CONFIG_IDX_START, V))
# define P165_SET_CONFIG_DEND(D, V) (bitWrite(P165_GROUP_CFG(D), P165_CONFIG_IDX_DEND, V))
# define P165_SET_CONFIG_RTLD(D, V) (bitWrite(P165_GROUP_CFG(D), P165_CONFIG_IDX_RTLD, V))
# define P165_SET_CONFIG_SPLTG(D, V) (bitWrite(P165_GROUP_CFG(D), P165_CONFIG_IDX_SPLTG, V))
# define P165_SET_CONFIG_CCLKW(D, V) (bitWrite(P165_GROUP_CFG(D), P165_CONFIG_IDX_CCLKW, V))
# define P165_SET_CONFIG_GSTRT(D, V) (bitWrite(P165_GROUP_CFG(D), P165_CONFIG_IDX_GSTRT, V))

# define P165_SHOW_BUFFER_SIZE  16 // Max number of characters in the buffer to show (1..4 groups of 1..4 digits)

# define P165_DISP_MANUAL       0
# define P165_DISP_CLOCK24BLNK  1
# define P165_DISP_CLOCK24      2
# define P165_DISP_CLOCK12BLNK  3
# define P165_DISP_CLOCK12      4
# define P165_DISP_DATE         5

# define P165_STRIP_TYPE_RGB    0
# define P165_STRIP_TYPE_RGBW   1

# define P165_SEGMENT_WIDTH_PIXELS  7  // Max: 15 (4 bits), also needs digit table and javascript changes!
# define P165_SEGMENT_HEIGHT_PIXELS 7  // Max: 15 (4 bits)
# define P165_SEGMENT_DOT_PIXELS    7  // Max: 15 (4 bits)
# define P165_SEGMENT_ADDON_PIXELS  12 // Max: 15 (4 bits)
# define P165_SEGMENT_EXTRA_PIXELS  15 // Max: 15 (4 bits)

# define P165_DIGIT_TABLE_H_INT   17
# define P165_DIGIT_TABLE_HEIGHT  "17" // Used in F() string

# define P165_TD_SIZE "1.6rem"         // For use in the Digit display, size (width & height) of the table TD's

// Set typedef before include <Noiasca_NeopixelDisplay.h>
typedef uint64_t segsize_t;            // largest storage size available, allows for up to 64 pixels per digit

# include <NeoPixelBus_wrapper.h>
# if P165_FEATURE_P073
#  define NEOPIXEL_DISPLAY_USE_WRITE 0 // Leave out character table and write() method
# endif // if P165_FEATURE_P073
# include <Noiasca_NeopixelDisplay.h>

struct P165_data_struct : public PluginTaskData_base {
private:

  struct PixelGroupCfg {  // Bit-mapping must match with P165_GET/SET_CONFIG_*() / P165_GROUP_CFG() macros
    uint32_t wpix   : 4;  // widht (1..15) (7)
    uint32_t hpix   : 4;  // height (1..15) (7)
    uint32_t crnr   : 1;  // corners off/on
    uint32_t dotp   : 4;  // dot pixels (0..15) (7)
    uint32_t addn   : 4;  // extra pixels (0..15) (12)
    uint32_t offs   : 4;  // offset before (0..15) (15)
    uint32_t dgts   : 3;  // digits in group (1..7) (4)
    uint32_t strt   : 1;  // start segment 0 = left-top/a, 1 = right-top/b
    uint32_t dend   : 1;  // dot at: 1: end of digit 0: between c/d segments
    uint32_t rtld   : 1;  // right to left display
    uint32_t splt   : 1;  // split g-segment (best enabled > 3 horizontal pixels)
    uint32_t cclkw  : 1;  // number-plan counter-clockwise (tbd)
    uint32_t gstrt  : 1;  // start numbering at g-segment
    uint32_t unused : 2;
    uint32_t aoffs  : 16; // Add-on pixels offset (use uin32_t for better memory alignment)
    uint32_t boffs  : 16; // Before pixels offset
  };

public:

  P165_data_struct(struct EventStruct *event);
  virtual ~P165_data_struct();

  static bool plugin_webform_load(struct EventStruct *event);
  static bool plugin_webform_save(struct EventStruct *event);
  static void initDigitGroup(struct EventStruct *event,
                             uint8_t             grp);

  bool        isInitialized() {
    return _initialized;
  }

  bool plugin_once_a_second(struct EventStruct *event);
  bool plugin_ten_per_second(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    const String      & string);

private:

  static uint16_t calculateGroupPixels(const uint8_t count,
                                       const uint8_t wpixels,
                                       const uint8_t hpixels,
                                       const bool    overlap,
                                       const uint8_t decPt,
                                       const uint8_t addN);
  static void drawSevenSegment(const uint8_t  digit,
                               const uint8_t  grp10,
                               const uint8_t  wpixels,
                               const uint8_t  hpixels,
                               const bool     overlap,
                               const uint8_t  decPt,
                               const uint8_t  addN,
                               const uint8_t  max,
                               const uint16_t offset,
                               const bool     strt,
                               const bool     dend,
                               const String & fgColor,
                               const bool     dspPlan,
                               const int16_t  aOffs,
                               const bool     splitG,
                               const bool     rtld,
                               const bool     cclkw,
                               const bool     gstrt);
  static String calculatePixelIndex(const uint8_t  hor,
                                    const int8_t   ver,
                                    const uint8_t  seg,
                                    const uint16_t offset,
                                    const uint8_t  wpixels,
                                    const uint8_t  hpixels,
                                    const bool     overlap,
                                    const bool     strt,
                                    const bool     dend,
                                    const uint8_t  decPt,
                                    const uint8_t  addN,
                                    const bool     splitG,
                                    const bool     cclkw,
                                    const bool     gstrt);
  static void  addJavascript();
  uint16_t     calculateDisplayPixels();
  uint16_t     calculateDisplayDigits();
  void         fillSegmentBitmap(const uint8_t       grp,
                                 const PixelGroupCfg pixCfg);

  static   int offsetLogic_callback(uint16_t position);

  void         fillBufferWithTime(const bool    sevendgt_now,
                                  uint8_t       sevendgt_hours,
                                  uint8_t       sevendgt_minutes,
                                  uint8_t       sevendgt_seconds,
                                  const bool    flag12h,
                                  const bool    suppressLeading0,
                                  const uint8_t offset);
  void fillBufferWithDate(const bool    sevendgt_now,
                          uint8_t       sevendgt_day,
                          uint8_t       sevendgt_month,
                          const int     sevendgt_year,
                          const bool    suppressLeading0,
                          const uint8_t offset);
  void put4NumbersInBuffer(const uint8_t nr1,
                           const uint8_t nr2,
                           const uint8_t nr3,
                           const int8_t  nr4,
                           const bool    suppressLeading0,
                           const uint8_t offset);
  void clearBuffer();

  void writeCharacterToDisplay(uint8_t group,
                               uint8_t digit,
                               uint8_t character,
                               bool    period);
  void      writeBufferToDisplay(uint8_t group = 0);
  segsize_t digit2SegmentMap(uint8_t grp,
                             uint8_t segments);


  int  getEffectiveTextLength(const String& text);
  bool nextScroll();
  void setTextToScroll(const String& text);
  void setScrollSpeed(uint8_t speed);
  bool isScrollEnabled();
  void setScrollEnabled(bool scroll);
  bool plugin_write_7dtext(const String& text);
  # if P165_FEATURE_P073
  bool plugin_write_7dfont(struct EventStruct *event,
                           const String      & text);
  # endif // if P165_FEATURE_P073
  bool plugin_write_7dbin(const String& text);
  bool plugin_write_7digit(const String& text);
  void fillBufferWithString(const String& textToShow,
                            bool          useBinaryData = false);
  void setBinaryData(const String& data);
  bool extraPixelsState(uint8_t  grp,
                        uint8_t  state,
                        uint32_t color,
                        bool     pxlExtra = true);
  # if P165_DEBUG_INFO || P165_DEBUG_DEBUG
  void logBufferContent(String prefix);
  # endif // if P165_DEBUG_INFO || P165_DEBUG_DEBUG
  bool parseRGBWColors(const String& string,
                       bool          rgbW,
                       uint32_t    & fgColor,
                       uint32_t    & bgColor,
                       bool        & fgSet,
                       bool        & bgSet);

  NeoPixelBus_wrapper     *strip = nullptr;
  Noiasca_NeopixelDisplay *display[PLUGIN_CONFIGLONGVAR_MAX]{ nullptr };

  segsize_t     _segments[PLUGIN_CONFIGLONGVAR_MAX][8]{}; // 4*8*uint64_t = 256 bytes...
  PixelGroupCfg _pixelGroupCfg[PLUGIN_CONFIGLONGVAR_MAX]{};

  uint16_t _fgColor          = ADAGFX_RED;
  uint16_t _bgColor          = ADAGFX_BLACK;
  uint8_t  _stripType        = 0;
  uint8_t  _pixelGroups      = 0;
  uint8_t  _defBrightness    = 0;
  uint8_t  _maxBrightness    = 0;
  uint8_t  _output           = 0;
  uint8_t  _fontset          = 0;
  uint8_t  _stdOffset        = 0;
  uint8_t  _totalDigits      = 0;
  bool     _initialized      = false;
  bool     _timesep          = false;
  bool     _suppressLeading0 = false;
  bool     _clearOnExit      = false;
  bool     _blinkDot         = false;

  String   _textToScroll;
  bool     _txtScrolling  = false;
  bool     _scrollAllowed = false;
  uint16_t _scrollSpeed   = 0;
  uint16_t _scrollCount   = 0;
  uint16_t _scrollPos     = 0;
  bool     _scrollFull    = false;
  bool     _binaryData    = false;
  bool     _periods       = true;
  uint8_t  _currentGroup  = 0;

  uint8_t showbuffer[P165_SHOW_BUFFER_SIZE]  = { 0 }; // The characters to put on the display
  bool    showperiods[P165_SHOW_BUFFER_SIZE] = { 0 }; // show the dot
  uint8_t showmap[P165_SHOW_BUFFER_SIZE]     = { 0 }; // map buffer index to digit index

  # if P165_FEATURE_DIGITCOLOR || P165_FEATURE_GROUPCOLOR
  std::map<uint16_t, uint32_t>digitColors;
  # endif // if P165_FEATURE_DIGITCOLOR || P165_FEATURE_GROUPCOLOR
};
#endif // ifdef USES_P165
#endif // ifndef PLUGINSTRUCTS_P165_DATA_STRUCT_H
