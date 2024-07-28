#ifndef PLUGINSTRUCTS_P073_DATA_STRUCT_H
#define PLUGINSTRUCTS_P073_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P073

# define P073_CFG_DISPLAYTYPE    PCONFIG(0)
# define P073_CFG_OUTPUTTYPE     PCONFIG(1)
# define P073_CFG_BRIGHTNESS     PCONFIG(2)
# define P073_CFG_SCROLLSPEED    PCONFIG(3)
# define P073_CFG_FONTSET        PCONFIG(4)
# define P073_CFG_DIGITS         PCONFIG(5)
# define P073_CFG_FLAGS          PCONFIG_ULONG(0)

# define P073_TM1637_4DGTCOLON   0
# define P073_TM1637_4DGTDOTS    1
# define P073_TM1637_6DGT        2
# define P073_MAX7219_8DGT       3
# define P073_74HC595_2_8DGT     4

# define P073_DISP_MANUAL        0
# define P073_DISP_CLOCK24BLNK   1
# define P073_DISP_CLOCK24       2
# define P073_DISP_CLOCK12BLNK   3
# define P073_DISP_CLOCK12       4
# define P073_DISP_DATE          5

# define P073_OPTION_PERIOD      0 // Period as dot
# define P073_OPTION_HIDEDEGREE  1 // Hide degree symbol for temperatures
# define P073_OPTION_RIGHTALIGN  2 // Align 7dt output right on MAX7219 display
# define P073_OPTION_SCROLLTEXT  3 // Scroll text > 8 characters
# define P073_OPTION_SCROLLFULL  4 // Scroll text from the right in, starting with a blank display
# define P073_OPTION_SUPPRESS0   5 // Suppress leading zero on day/hour of Date/Time display

# ifndef P073_7DDT_COMMAND
#  define P073_7DDT_COMMAND     1  // Enable 7ddt by default
# endif // ifndef P073_7DDT_COMMAND
# ifndef P073_EXTRA_FONTS
#  define P073_EXTRA_FONTS      1  // Enable extra fonts
# endif // ifndef P073_EXTRA_FONTS
# ifndef P073_SCROLL_TEXT
#  define P073_SCROLL_TEXT      1  // Enable scrolling of 7dtext by default
# endif // ifndef P073_SCROLL_TEXT
# ifndef P073_7DBIN_COMMAND
#  define P073_7DBIN_COMMAND    1  // Enable input of binary data via 7dbin,uint8_t,... command
# endif // ifndef P073_7DBIN_COMMAND
# ifndef P073_SUPPRESS_ZERO
#  define P073_SUPPRESS_ZERO    1  // Enable Suppress leading zero on day/hour
# endif // ifndef P073_SUPPRESS_ZERO
# ifndef P073_USE_74HC595
#  define P073_USE_74HC595      1  // Enable support for 74HC595 based (sequential and multiplexing) displays
# endif // ifndef P073_USE_74HC595

# if defined(PLUGIN_SET_COLLECTION) && defined(ESP8266)
#  if P073_7DDT_COMMAND
#   undef P073_7DDT_COMMAND // Optionally activate if .bin file space is problematic, remove the 7ddt command
#   define P073_7DDT_COMMAND    0
#  endif // if P073_7DDT_COMMAND
#  if P073_EXTRA_FONTS
#   undef P073_EXTRA_FONTS // Optionally activate if .bin file space is problematic, remove the font selection and 7dfont command
#   define P073_EXTRA_FONTS     0
#  endif // if  P073_EXTRA_FONTS
#  if P073_SCROLL_TEXT
#   undef P073_SCROLL_TEXT // Optionally activate if .bin file space is problematic, remove the scrolling text feature
#   define P073_SCROLL_TEXT     0
#  endif // if P073_SCROLL_TEXT
#  if P073_7DBIN_COMMAND
#   undef P073_7DBIN_COMMAND // Optionally activate if .bin file space is problematic, remove the 7dbin command
#   define P073_7DBIN_COMMAND   0
#  endif // if P073_7DBIN_COMMAND
#  if P073_SUPPRESS_ZERO
#   undef P073_SUPPRESS_ZERO // Optionally activate if .bin file space is problematic, remove the Suppress leading zero feature
#   define P073_SUPPRESS_ZERO   0
#  endif // if P073_SUPPRESS_ZERO
# else // if defined(PLUGIN_SET_COLLECTION) && defined(ESP8266)

// #  define P073_DEBUG // Leave out some debugging on demand, activates extra log info in the debug
# endif // if defined(PLUGIN_SET_COLLECTION) && defined(ESP8266)

# if defined(ESP8266)
#  if P073_USE_74HC595
#   undef P073_USE_74HC595 // Removes the support for 74HC595 displays
#   define P073_USE_74HC595 0
#  endif // if P073_USE_74HC595
# endif // if defined(ESP8266)

# define TM1637_POWER_ON    0b10001000
# define TM1637_POWER_OFF   0b10000000
# define TM1637_CLOCKDELAY  40
# define TM1637_4DIGIT      4
# define TM1637_6DIGIT      2

# define P073_HC595_SEQUENTIAL  (isSequential)  // Sequential digits
# define P073_HC595_MULTIPLEX   (!isSequential) // Multiplexed digits, have to be constantly refreshed

// each char table is specific for each display and maps all numbers/symbols
// needed:
//   - pos 0-9   - Numbers from 0 to 9
//   - pos 10    - Space " "
//   - pos 11    - minus symbol "-"
//   - pos 12    - degree symbol "°"
//   - pos 13    - equal "="
//   - pos 14    - triple lines "/"
//   - pos 15    - underscore "_"
//   - pos 16-41 - Letters from A to Z
static const uint8_t DefaultCharTable[42] PROGMEM = {
  0b01111110, 0b00110000, 0b01101101, 0b01111001, 0b00110011, 0b01011011,
  0b01011111, 0b01110000, 0b01111111, 0b01111011, 0b00000000, 0b00000001,
  0b01100011, 0b00001001, 0b01001001, 0b00001000, 0b01110111, 0b00011111,
  0b01001110, 0b00111101, 0b01001111, 0b01000111, 0b01011110, 0b00110111,
  0b00000110, 0b00111100, 0b01010111, 0b00001110, 0b01010100, 0b01110110,
  0b01111110, 0b01100111, 0b01101011, 0b01100110, 0b01011011, 0b00001111,
  0b00111110, 0b00111110, 0b00101010, 0b00110111, 0b00111011, 0b01101101 };

# if P073_EXTRA_FONTS

// Siekoo alphabet https://www.fakoo.de/siekoo
// as the 'over score' character isn't normally available, the pipe "|" is used for that, and for degree the "^"" is used
// specials:
//   - pos 0-9   - Numbers from 0 to 9
//   - pos 10    - Space " "
//   - pos 11    - minus symbol "-"
//   - pos 12    - degree symbol "°" (specially handled "^" into a degree)
//   - pos 13    - equal "="
//   - pos 14    - slash "/"
//   - pos 15    - underscore "_"
//   - pos 16-40 - Special characters not handled yet -- MAX7219 -- -- TM1637 --
//   - pos 16    - percent "%"                          0b00010010
//   - pos 17    - at "@"                               0b01110100
//   - pos 18    - period "."                           0b00000100
//   - pos 10    - comma ","                            0b00011000
//   - pos 20    - semicolon ";"                        0b00101000
//   - pos 21    - colon ":"                            0b01001000
//   - pos 22    - plus "+"                             0b00110001
//   - pos 23    - asterisk "*"                         0b01001001
//   - pos 24    - hash "#"                             0b00110110
//   - pos 25    - exclamation mark "!"                 0b01101011
//   - pos 26    - question mark "?"                    0b01101001
//   - pos 27    - single quote "'"                     0b00000010
//   - pos 28    - double quote '"'                     0b00100010
//   - pos 29    - left sharp bracket "<"               0b01000010
//   - pos 30    - right sharp bracket ">"              0b01100000
//   - pos 31    - backslash "\"                        0b00010011
//   - pos 32    - left round bracket "("               0b01001110
//   - pos 33    - right round bracket ")"              0b01111000
//   - pos 34    - overscore "|" (the top-most line)    0b01000000
//   - pos 35    - uppercase C "C" (optionally enabled) 0b01001110
//   - pos 36    - uppercase H "H"                      0b00110111
//   - pos 37    - uppercase N "N"                      0b01110110
//   - pos 38    - uppercase O "O"                      0b01111110
//   - pos 39    - uppercase R "R"                      0b01100110
//   - pos 40    - uppercase U "U"                      0b00111110
//   - pos 41    - uppercase X "X"                      0b00110111
//   - pos 42-67 - Letters from A to Z Siekoo style
static const uint8_t SiekooCharTable[68] PROGMEM = {
  0b01111110, 0b00110000, 0b01101101, 0b01111001, 0b00110011, 0b01011011,
  0b01011111, 0b01110000, 0b01111111, 0b01111011, 0b00000000, 0b00000001,
  0b01100011, 0b00001001, 0b00100101, 0b00001000, 0b00010010, 0b01110100,
  0b00000100, 0b00011000, 0b00101000, 0b01001000, 0b00110001, 0b01001001,
  0b00110110, 0b01101011, 0b01101001, 0b00000010, 0b00100010, 0b01000010,
  0b01100000, 0b00010011, 0b01001110, 0b01111000, 0b01000000, 0b01001110,
  0b00110111, 0b01110110, 0b01111110, 0b01100110, 0b00111110, 0b00110111,
  0b01111101, 0b00011111, 0b00001101, 0b00111101, 0b01001111, 0b01000111, /* ABCDEF */
  0b01011110, 0b00010111, 0b01000100, 0b01011000, 0b01010111, 0b00001110,
  0b01010101, 0b00010101, 0b00011101, 0b01100111, 0b01110011, 0b00000101,
  0b01011010, 0b00001111, 0b00011100, 0b00101010, 0b00101011, 0b00010100,
  0b00111011, 0b01101100 };

// dSEG7 https://www.keshikan.net/fonts-e.html
// specials:
//   - pos 0-9   - Numbers from 0 to 9
//   - pos 10    - Space " "
//   - pos 11    - minus symbol "-"
//   - pos 12    - degree symbol "°" (specially handled "^" into a degree)
//   - pos 13    - equal "="
//   - pos 14    - slash "/"
//   - pos 15    - underscore "_"
//   - pos 16-41 - Letters from A to Z dSEG7 style
static const uint8_t Dseg7CharTable[42] PROGMEM = {
  0b01111110, 0b00110000, 0b01101101, 0b01111001, 0b00110011, 0b01011011,
  0b01011111, 0b01110000, 0b01111111, 0b01111011, 0b00000000, 0b00000001,
  0b01100011, 0b00001001, 0b01001001, 0b00001000, 0b01110111, 0b00011111, /* AB */
  0b00001101, 0b00111101, 0b01001111, 0b01000111, 0b01011110, 0b00010111,
  0b00010000, 0b00111100, 0b01010111, 0b00001110, 0b01110110, 0b00010101,
  0b00011101, 0b01100111, 0b01110011, 0b00000101, 0b00011011, 0b00001111,
  0b00011100, 0b00111110, 0b00111111, 0b00110111, 0b00111011, 0b01101100 };

# endif // P073_EXTRA_FONTS

uint8_t p073_getDefaultDigits(uint8_t displayModel,
                              uint8_t digits = 0);

struct P073_data_struct : public PluginTaskData_base {
public:

  P073_data_struct()          = default;
  virtual ~P073_data_struct() = default;

  void init(struct EventStruct *event);
  bool p073_plugin_write(struct EventStruct *event,
                         const String      & string);
  bool plugin_once_a_second(struct EventStruct *event);
  # if P073_SCROLL_TEXT
  bool plugin_ten_per_second(struct EventStruct *event);
  # endif // if P073_SCROLL_TEXT

  # if P073_USE_74HC595
  bool plugin_fifty_per_second(struct EventStruct *event);
  # endif // if P073_USE_74HC595
  void FillBufferWithTime(bool    sevendgt_now,
                          uint8_t sevendgt_hours,
                          uint8_t sevendgt_minutes,
                          uint8_t sevendgt_seconds,
                          bool    flag12h,
                          bool    suppressLeading0);
  void FillBufferWithDate(bool    sevendgt_now,
                          uint8_t sevendgt_day,
                          uint8_t sevendgt_month,
                          int     sevendgt_year,
                          bool    suppressLeading0);
  void Put4NumbersInBuffer(const uint8_t nr1,
                           const uint8_t nr2,
                           const uint8_t nr3,
                           const int8_t  nr4
                           # if          P073_SUPPRESS_ZERO
                           ,
                           const bool    suppressLeading0
                           # endif // if P073_SUPPRESS_ZERO
                           );
  void FillBufferWithNumber(const String& number);
  void FillBufferWithTemp(int temperature);
  # if P073_7DDT_COMMAND
  void FillBufferWithDualTemp(int  leftTemperature,
                              bool leftWithDecimal,
                              int  rightTemperature,
                              bool rightWithDecimal);
  # endif // if P073_7DDT_COMMAND
  void    FillBufferWithString(const String& textToShow,
                               bool          useBinaryData = false);
  # if P073_SCROLL_TEXT
  int     getEffectiveTextLength(const String& text);
  bool    NextScroll();
  void    setTextToScroll(const String& text);
  void    setScrollSpeed(uint8_t speed);
  bool    isScrollEnabled();
  void    setScrollEnabled(bool scroll);
  # endif // if P073_SCROLL_TEXT
  # if P073_7DBIN_COMMAND
  void    setBinaryData(const String& data);
  # endif // if P073_7DBIN_COMMAND
  # ifdef P073_DEBUG
  void    LogBufferContent(String prefix);
  # endif // ifdef P073_DEBUG
  void    FillBufferWithDash();
  void    ClearBuffer();

  uint8_t mapCharToFontPosition(char    character,
                                uint8_t fontset);
  uint8_t mapMAX7219FontToTM1673Font(uint8_t character);
  uint8_t tm1637_getFontChar(uint8_t index,
                             uint8_t fontset);

  int     dotpos                = -1;
  uint8_t showbuffer[8]         = { 0 };
  bool    showperiods[8]        = { 0 };
  uint8_t spidata[2]            = { 0 };
  int8_t  pin1                  = -1;
  int8_t  pin2                  = -1;
  int8_t  pin3                  = -1;
  uint8_t displayModel          = 0;
  uint8_t output                = 0;
  uint8_t brightness            = 0;
  uint8_t digits                = 4;
  bool    timesep               = false;
  bool    shift                 = false;
  bool    periods               = false;
  bool    hideDegree            = false;
  bool    rightAlignTempMAX7219 = false;
  bool    suppressLeading0      = false;
  uint8_t fontset               = 0;
  # if P073_7DBIN_COMMAND
  bool binaryData = false;
  # endif // P073_7DBIN_COMMAND
  # if P073_SCROLL_TEXT
  bool     txtScrolling  = false;
  bool     scrollAllowed = false;
  uint16_t scrollCount   = 0;
  uint16_t scrollPos     = 0;
  bool     scrollFull    = false;

private:

  uint16_t _scrollSpeed = 0;
  # endif // P073_SCROLL_TEXT
  # if defined(P073_SCROLL_TEXT) || defined(P073_7DBIN_COMMAND)
  String _textToScroll;
  # endif // if defined(P073_SCROLL_TEXT) || defined(P073_7DBIN_COMMAND)
  # ifdef P073_DEBUG
  uint32_t counter50 = 0;
  # endif // ifdef P073_DEBUG
  # if P073_USE_74HC595
  bool isSequential = false;
  # endif // if P073_USE_74HC595

private:

  void getDisplayLimits(int32_t& lLimit,
                        int32_t& uLimit,
                        int8_t   offset = 0,
                        uint8_t  digits = 0);
  bool plugin_write_7dn(struct EventStruct *event,
                        const String      & text);
  bool plugin_write_7dt(const String& text);
  # if P073_7DDT_COMMAND
  bool plugin_write_7ddt(const String& text);
  # endif // if P073_7DDT_COMMAND
  bool plugin_write_7dst(struct EventStruct *event);
  bool plugin_write_7dsd(struct EventStruct *event);
  bool plugin_write_7dtext(const String& text);
  # if P073_EXTRA_FONTS
  bool plugin_write_7dfont(struct EventStruct *event,
                           const String      & text);
  # endif // if P073_EXTRA_FONTS
  # if P073_7DBIN_COMMAND
  bool plugin_write_7dbin(const String& text);
  # endif // if P073_7DBIN_COMMAND

  // ---- TM1637 specific functions ----
  void tm1637_i2cStart(uint8_t clk_pin,
                       uint8_t dio_pin);
  void tm1637_i2cStop(uint8_t clk_pin,
                      uint8_t dio_pin);
  void tm1637_i2cAck(uint8_t clk_pin,
                     uint8_t dio_pin);
  void tm1637_i2cWrite_ack(uint8_t clk_pin,
                           uint8_t dio_pin,
                           uint8_t bytesToPrint[],
                           uint8_t length);
  void tm1637_i2cWrite_ack(uint8_t clk_pin,
                           uint8_t dio_pin,
                           uint8_t bytetoprint);
  void tm1637_i2cWrite(uint8_t clk_pin,
                       uint8_t dio_pin,
                       uint8_t bytetoprint);
  void tm1637_ClearDisplay(uint8_t clk_pin,
                           uint8_t dio_pin);
  void tm1637_SetPowerBrightness(uint8_t clk_pin,
                                 uint8_t dio_pin,
                                 uint8_t brightlvl,
                                 bool    poweron);
  void    tm1637_InitDisplay(uint8_t clk_pin,
                             uint8_t dio_pin);
  uint8_t tm1637_separator(uint8_t value,
                           bool    sep);
  void    tm1637_ShowTime6();
  void    tm1637_ShowDate6(bool showTime = false);
  void    tm1637_ShowTemp6(bool sep);
  void    tm1637_ShowTimeTemp4(bool    sep,
                               uint8_t bufoffset);
  void    tm1637_SwapDigitInBuffer(uint8_t startPos);
  void    tm1637_ShowBuffer(uint8_t firstPos,
                            uint8_t lastPos,
                            bool    useBinaryData = false);

  // ---- MAX7219 specific functions ----
  void max7219_spiTransfer(uint8_t                   din_pin,
                           uint8_t                   clk_pin,
                           uint8_t                   cs_pin,
                           ESPEASY_VOLATILE(uint8_t) opcode,
                           ESPEASY_VOLATILE(uint8_t) data);
  void max7219_ClearDisplay(uint8_t din_pin,
                            uint8_t clk_pin,
                            uint8_t cs_pin);
  void max7219_SetPowerBrightness(uint8_t din_pin,
                                  uint8_t clk_pin,
                                  uint8_t cs_pin,
                                  uint8_t brightlvl,
                                  bool    poweron);
  void max7219_SetDigit(uint8_t din_pin,
                        uint8_t clk_pin,
                        uint8_t cs_pin,
                        int     dgtpos,
                        uint8_t dgtvalue,
                        bool    showdot,
                        bool    binaryData = false);
  void max7219_InitDisplay(uint8_t din_pin,
                           uint8_t clk_pin,
                           uint8_t cs_pin);
  void max7219_ShowTime(uint8_t din_pin,
                        uint8_t clk_pin,
                        uint8_t cs_pin,
                        bool    sep);
  void max7219_ShowTemp(uint8_t din_pin,
                        uint8_t clk_pin,
                        uint8_t cs_pin,
                        int8_t  firstDot,
                        int8_t  secondDot);
  void max7219_ShowDate(uint8_t din_pin,
                        uint8_t clk_pin,
                        uint8_t cs_pin);
  void max7219_ShowBuffer(uint8_t din_pin,
                          uint8_t clk_pin,
                          uint8_t cs_pin);
  # if P073_USE_74HC595
  void hc595_InitDisplay();
  void hc595_ShowBuffer();
  void hc595_AdjustBuffer();
  bool hc595_Sequential() {
    return P073_HC595_SEQUENTIAL;
  }

  # endif // if P073_USE_74HC595
};

#endif    // ifdef USES_P073
#endif // ifndef PLUGINSTRUCTS_P073_DATA_STRUCT_H
