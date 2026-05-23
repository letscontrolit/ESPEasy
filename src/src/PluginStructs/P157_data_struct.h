#ifndef PLUGINSTRUCTS_P157_DATA_STRUCT_H
#define PLUGINSTRUCTS_P157_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P157

# include "NoiascaHt16k33.h"
# include <vector>

# define P157_CFG_DISPLAYTYPE    PCONFIG(0)
# define P157_CFG_OUTPUTTYPE     PCONFIG(1)
# define P157_CFG_BRIGHTNESS     PCONFIG(2)
# define P157_CFG_SCROLLSPEED    PCONFIG(3)
# define P157_CFG_I2C_ADDRESS    PCONFIG(4)
# define P157_CFG_DISPLAYS       PCONFIG(5)
# define P157_CFG_FONTSET        PCONFIG(6)
# define P157_CFG_FLAGS          PCONFIG_ULONG(0)

# define P157_DISP_MANUAL        0
# define P157_DISP_CLOCK24BLNK   1
# define P157_DISP_CLOCK24       2
# define P157_DISP_CLOCK12BLNK   3
# define P157_DISP_CLOCK12       4
# define P157_DISP_DATE          5

# define P157_MODEL_4DGT         0
# define P157_MODEL_8DGT         1
# define P157_MODEL_4DGT_7SEG    2
# define P157_MODEL_8DGT_7SEG    3

# define P157_CHAR_EURO          128
# define P157_CHAR_DEGREE        129

// # define P157_OPTION_PERIOD      0 // Period as dot
# define P157_OPTION_HIDEDEGREE  1 // Hide degree symbol for temperatures
# define P157_OPTION_RIGHTALIGN  2 // Align 7dt output to the right
# define P157_OPTION_SCROLLTEXT  3 // Scroll text > display width
# define P157_OPTION_SCROLLFULL  4 // Scroll text from the right in, starting with a blank display
# define P157_OPTION_SUPPRESS0   5 // Suppress leading zero on day/hour of Date/Time display
# define P157_OPTION_BLINK_DOT   6 // Use dot on second digit for flashing instead of colon

# ifdef USES_P073
#  define P157_FEATURE_P073     1           // Use P073 shared functions and fonts when available
# else // ifdef USES_P073
#  define P157_FEATURE_P073     0
# endif // ifdef USES_P073

# if P157_FEATURE_P073 // Use shared fonts and functions from P073 Display - 7-segment display when available
#  include "../PluginStructs/P073_data_struct.h"
# endif // if P157_FEATURE_P073

# if P157_FEATURE_P073
#  if P073_EXTRA_FONTS
#   define P157_EXTRA_FONTS 1
#  else // if P073_EXTRA_FONTS
#   define P157_EXTRA_FONTS 0
#  endif // if P073_EXTRA_FONTS
# endif // if P157_FEATURE_P073

# ifndef P157_7DDT_COMMAND
#  define P157_7DDT_COMMAND     1  // Enable 7ddt by default
# endif // ifndef P157_7DDT_COMMAND
# ifndef P157_EXTRA_FONTS
#  define P157_EXTRA_FONTS      1  // Enable extra fonts
# endif // ifndef P157_EXTRA_FONTS
# ifndef P157_SCROLL_TEXT
#  define P157_SCROLL_TEXT      1  // Enable scrolling of 7dtext by default
# endif // ifndef P157_SCROLL_TEXT
# ifndef P157_7DBIN_COMMAND
#  define P157_7DBIN_COMMAND    1  // Enable input of binary data via 7dbin,uint8_t,... command
# endif // ifndef P157_7DBIN_COMMAND
# ifndef P157_SUPPRESS_ZERO
#  define P157_SUPPRESS_ZERO    1  // Enable Suppress leading zero on day/hour
# endif // ifndef P157_SUPPRESS_ZERO

# if defined(PLUGIN_SET_COLLECTION) && defined(ESP8266)
#  if P157_7DDT_COMMAND
#   undef P157_7DDT_COMMAND // Optionally activate if .bin file space is problematic, remove the 7ddt command
#   define P157_7DDT_COMMAND    0
#  endif // if P157_7DDT_COMMAND
#  if P157_EXTRA_FONTS
#   undef P157_EXTRA_FONTS // Optionally activate if .bin file space is problematic, remove the font selection and 7dfont command
#   define P157_EXTRA_FONTS     0
#  endif // if  P157_EXTRA_FONTS
#  if P157_SCROLL_TEXT
#   undef P157_SCROLL_TEXT // Optionally activate if .bin file space is problematic, remove the scrolling text feature
#   define P157_SCROLL_TEXT     0
#  endif // if P157_SCROLL_TEXT
#  if P157_7DBIN_COMMAND
#   undef P157_7DBIN_COMMAND // Optionally activate if .bin file space is problematic, remove the 7dbin command
#   define P157_7DBIN_COMMAND   0
#  endif // if P157_7DBIN_COMMAND
#  if P157_SUPPRESS_ZERO
#   undef P157_SUPPRESS_ZERO // Optionally activate if .bin file space is problematic, remove the Suppress leading zero feature
#   define P157_SUPPRESS_ZERO   0
#  endif // if P157_SUPPRESS_ZERO
# endif // if defined(PLUGIN_SET_COLLECTION) && defined(ESP8266)
# define P157_DEBUG // Leave out some debugging on demand, activates extra log info in the debug

const __FlashStringHelper* P157_DisplayModel(uint8_t model);
uint8_t                    P157_getDefaultDigits(uint8_t displayModel,
                                                 uint8_t digits = 0);
void                       P157_display_output_selector(const __FlashStringHelper *id,
                                                        int16_t                    value);
bool                       P157_is7SegmentDisplay(uint8_t model);
bool                       P157_is4DigitDisplay(uint8_t model);

struct P157_data_struct : public PluginTaskData_base {
public:

  P157_data_struct() = default;
  virtual ~P157_data_struct();

  bool init(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    const String      & string);
  bool plugin_once_a_second(struct EventStruct *event);
  # if P157_SCROLL_TEXT
  bool plugin_ten_per_second(struct EventStruct *event);
  # endif // if P157_SCROLL_TEXT

  void printBuffer();
  void fillBufferWithTime(bool    sevendgt_now,
                          uint8_t sevendgt_hours,
                          uint8_t sevendgt_minutes,
                          uint8_t sevendgt_seconds,
                          bool    flag12h,
                          bool    suppressLeading0);
  void fillBufferWithDate(bool    sevendgt_now,
                          uint8_t sevendgt_day,
                          uint8_t sevendgt_month,
                          int     sevendgt_year,
                          bool    suppressLeading0);
  void put4NumbersInBuffer(const uint8_t nr1,
                           const uint8_t nr2,
                           const int8_t  nr3,
                           const int8_t  nr4
                           # if          P157_SUPPRESS_ZERO
                           ,
                           const bool    suppressLeading0
                           # endif // if P157_SUPPRESS_ZERO
                           ,
                           const bool sep
                           );
  void fillBufferWithNumber(const String& number);
  void fillBufferWithTemp(int temperature);
  # if P157_7DDT_COMMAND
  void fillBufferWithDualTemp(int  leftTemperature,
                              bool leftWithDecimal,
                              int  rightTemperature,
                              bool rightWithDecimal);
  # endif // if P157_7DDT_COMMAND
  void fillBufferWithString(const String& textToShow,
                            bool          useBinaryData = false);
  # if P157_SCROLL_TEXT || P157_7DBIN_COMMAND
  bool isPeriodChar(const char thisChar);
  int  getEffectiveTextLength(const String& text);
  # endif // if P157_SCROLL_TEXT || P157_7DBIN_COMMAND
  # if P157_SCROLL_TEXT
  bool nextScroll();
  void setTextToScroll(const String& text);
  void setScrollSpeed(uint8_t speed);

  bool isScrollEnabled() const       { return txtScrolling && scrollAllowed; }

  void setScrollEnabled(bool scroll) { scrollAllowed = scroll; }

  # endif // if P157_SCROLL_TEXT
  // # if P157_7DBIN_COMMAND
  // void setBinaryData(const String& data);
  // # endif // if P157_7DBIN_COMMAND
  # ifdef P157_DEBUG
  void logBufferContent(String prefix);
  # endif // ifdef P157_DEBUG
  void fillBufferWithDash();
  void clearBuffer();

  int     dotpos                = -1;
  char    showbuffer[64]        = { 0 };
  bool    showperiods[64]       = { 0 };
  uint8_t i2cAddress            = 0;
  uint8_t displayModel          = 0;
  uint8_t output                = 0;
  uint8_t brightness            = 0;
  uint8_t displays              = 4;
  uint8_t fontSet               = 0;
  bool    timesep               = false;
  bool    shift                 = false;
  bool    periods               = false;
  bool    hideDegree            = false;
  bool    rightAlignTempMAX7219 = false;
  bool    suppressLeading0      = false;
  uint8_t fontset               = 0;
  # if P157_7DBIN_COMMAND
  bool                 binaryData = false;
  std::vector<uint16_t>binData;
  # endif // P157_7DBIN_COMMAND
  # if P157_SCROLL_TEXT
  bool     txtScrolling  = false;
  bool     scrollAllowed = false;
  uint16_t scrollCount   = 0;
  uint16_t scrollPos     = 0;
  bool     scrollFull    = false;

private:

  uint16_t _scrollSpeed = 0;
  # endif // P157_SCROLL_TEXT
  # if defined(P157_SCROLL_TEXT) || defined(P157_7DBIN_COMMAND)
  String _textToScroll;
  # endif // if defined(P157_SCROLL_TEXT) || defined(P157_7DBIN_COMMAND)
  # ifdef P157_DEBUG
  uint32_t counter50 = 0;
  # endif // ifdef P157_DEBUG

private:

  Noiasca_ht16k33*ht16k33 = nullptr;

  bool isInitialized() const { return nullptr != ht16k33; }

  void getDisplayLimits(int32_t& lLimit,
                        int32_t& uLimit,
                        int8_t   offset = 0,
                        uint8_t  digits = 0);
  bool plugin_write_7dn(struct EventStruct *event,
                        const String      & text);
  bool plugin_write_7dt(const String& text);
  # if P157_7DDT_COMMAND
  bool plugin_write_7ddt(const String& text);
  # endif // if P157_7DDT_COMMAND
  bool plugin_write_7dst(struct EventStruct *event);
  bool plugin_write_7dsd(struct EventStruct *event);
  bool plugin_write_7dtext(const String& text);
  # if P157_EXTRA_FONTS
  bool plugin_write_7dfont(struct EventStruct *event,
                           const String      & text);
  # endif // if P157_EXTRA_FONTS
  # if P157_7DBIN_COMMAND
  bool plugin_write_7dbin(const String& text,
                          const uint8_t offset);
  # endif // if P157_7DBIN_COMMAND
  const char periodchars[4] = { '.', ',', ':', ';' };

};

#endif    // ifdef USES_P157
#endif // ifndef PLUGINSTRUCTS_P157_DATA_STRUCT_H
