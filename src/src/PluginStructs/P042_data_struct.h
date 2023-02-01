#ifndef PLUGINSTRUCTS_P042_DATA_STRUCT_H
#define PLUGINSTRUCTS_P042_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"

#ifdef USES_P042

# include <Adafruit_NeoPixel.h>


# define P042_NUM_PIXEL     20  // Defines the default amount of LED Pixels
# define P042_MAX_PIXELS    300 // Max. allowed pixels
# define P042_RANDOM_PIXEL  70  // Defines the Flicker Level for Simple Candle
# define P042_BRIGHT_START  128 // Defines the Start Brightness
# define P042_FLAME_OPTIONS 8   // Number of flame types

enum class P042_SimType : uint8_t {
  TypeOff            = 0,
  TypeSimpleCandle   = 1,
  TypeAdvancedCandle = 2,
  TypeStaticLight    = 3,
  TypePolice         = 4,
  TypeBlink          = 5,
  TypeStrobe         = 6,
  TypeColorFader     = 7,
};

enum class P042_ColorType : uint8_t {
  ColorDefault  = 0,
  ColorSelected = 1,
};

# define P042_WEBVAR_COUNT              7
# define P042_WEBVAR_RED                _webVars[0]
# define P042_WEBVAR_GREEN              _webVars[1]
# define P042_WEBVAR_BLUE               _webVars[2]
# define P042_WEBVAR_BRIGHTNESS         _webVars[3]
# define P042_WEBVAR_CANDLETYPE         _webVars[4]
# define P042_WEBVAR_COLORTYPE          _webVars[5]
# define P042_WEBVAR_PIXELCOUNT         _webVars[6]
# define P042_WEBVAR_RED_S              "wRed"
# define P042_WEBVAR_GREEN_S            "wGreen"
# define P042_WEBVAR_BLUE_S             "wBlue"
# define P042_WEBVAR_BRIGHTNESS_S       "brText"
# define P042_WEBVAR_CANDLETYPE_S       "cType"
# define P042_WEBVAR_COLORTYPE_S        "clrType"
# define P042_WEBVAR_PIXELCOUNT_S       "pxCnt"

// Other vars
# define P042_OTHVAR_BRIGHTNESSSLIDE_S  "brSlide"

// Config defines
# define P042_CONFIG_RED                PCONFIG(0)
# define P042_CONFIG_GREEN              PCONFIG(1)
# define P042_CONFIG_BLUE               PCONFIG(2)
# define P042_CONFIG_BRIGHTNESS         PCONFIG(3)
# define P042_CONFIG_CANDLETYPE         PCONFIG(4)
# define P042_CONFIG_COLORTYPE          PCONFIG(5)
# define P042_CONFIG_PIXELCOUNT         PCONFIG(6)

struct P042_data_struct : public PluginTaskData_base {
public:

  P042_data_struct();
  virtual ~P042_data_struct();

  bool plugin_init(struct EventStruct *event);

  // Perform read and return true when when succesful
  bool plugin_read(struct EventStruct *event);
  bool plugin_once_a_second(struct EventStruct *event);
  bool plugin_fifty_per_second(struct EventStruct *event);

  // Perform write and return true when successful
  bool plugin_write(struct EventStruct *event,
                    const String      & string);

private:

  void SetPixelsBlack();
  void SetPixelToColor(int PixelIdx);
  void type_Static_Light();
  void type_Simple_Candle();
  void type_Advanced_Candle();
  void type_Police();
  void type_BlinkStrobe();
  void type_ColorFader();

  uint8_t        Candle_red    = 0;
  uint8_t        Candle_green  = 0;
  uint8_t        Candle_blue   = 0;
  uint8_t        Candle_bright = 128;
  int            Candle_pxlcnt = P042_NUM_PIXEL;
  int            segment       = Candle_pxlcnt / 4;
  P042_SimType   Candle_type   = P042_SimType::TypeSimpleCandle;
  P042_ColorType Candle_color  = P042_ColorType::ColorDefault;

  // global variables
  unsigned long Candle_Update  = 0;
  word          Candle_Temp[3] = { 0 }; // Temp variables
  boolean       GPIO_Set       = false;

  Adafruit_NeoPixel *Candle_pixels;
};


#endif // ifdef USES_P042
#endif // ifndef PLUGINSTRUCTS_P042_DATA_STRUCT_H
