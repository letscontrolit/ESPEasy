#include "_Plugin_Helper.h"
#ifdef USES_P042

// #######################################################################################################
// ######################################## Plugin 042: NeoPixel Candle ##################################
// #######################################################################################################

// PROJECT INFO
// Wifi Candle for ESPEasy by Dominik Schmidt (10.2016)

/** Changelog:
 * 2023-01-21 tonhuisman: Further refactor and improve code, including GH feedback
 *                        Add setting for Led Count, defaults to 20 (was fixed size)
 *                        Move RGBtoHSV to misc.cpp and rename to RGB2HSV after changing to use float instead of double
 * 2023-01-20 tonhuisman: Minify jacascript code, reduce string usage, minor code improvements
 *                        Update to support current jscolor version/features
 *                        TODO: Code improvements and deduplication, like HSV2RGB() instead of local HSVtoRGB()
 *                        TODO: Allow configuration of number of pixels
 */

// INCLUDE jscolor (http://jscolor.com/)
//   * Download the lib from here: http://jscolor.com/release/latest.zip
//   * Extract jscolor.min.js
//   * Now open the Web UI of your ESPEasy with this URL:
//     http://<IP-ESPEasy>/upload
//   * Select Browse ... and choose the extracted jscolor.min.js File (ensure the ...min... version !!)
//   * Press Upload und you are done.

// Add the Adafruit Neopixel Library to your library path. You will find it here:
// https://github.com/adafruit/Adafruit_NeoPixel
// That´s all :-) Now ESPEasy has a new 25ms "Timer loop" and Neopixel Support.

// NOTES
// Please keep in mind that you can add tasks which produce a very large delay while reading the sensor.
// For example the DS18B20 is very slow in reading the values. This can slow down the simulation and you
// will notice that the candle did not run smooth. So keep an eye on your tasks and don't add to much other tasks.

// HARDWARE
// The Wifi Candle uses 20 WS2812 RGB pixels. They are all connected in one row.
// I build a wooden wick with 5 pixels on each side. (A picture is here : http://www.esp8266.nu/forum/viewtopic.php?f=2&t=2147)
// The pixels are connected to 5V and the data pin I use is GPIO13 (but you can choose another one).
// Please ensure that you use a strong power supply because the pixels consume a lot of power when they
// shine in white with high brightness!
// I also placed a 100µF capacitor at the end of the WS2812 chain on +5/GND just to ensure a good power stability.
// btw ... My Testboard was a NodeMCU V3.

// QUESTIONS
// Send me an email at dominik@logview.info
// or place a comment in the Forum:
// http://www.esp8266.nu/forum/viewtopic.php?f=2&t=2147

// Candle Infos
// http://www.howtodotip.com/how+to+do+arduino+candle++3
// https://codebender.cc/sketch:129316#Neopixel%20Candle.ino
// http://www.instructables.com/id/Garden-Arduino-Lights/?ALLSTEPS                    Garten Beleuchtung
// http://www.instructables.com/id/Arduino-Controlled-Electric-Candle/?ALLSTEPS       InstaMorph Kerze
// https://github.com/danesparza/Halloweenfire/blob/master/halloweenfire.ino          Halloweenfire

// RGB / HSV Converter
// https://github.com/ratkins/RGBConverter          Lib
// https://www.ruinelli.ch/rgb-to-hsv               Code
// http://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both    Code Sammlung

# include <Adafruit_NeoPixel.h>


# define P042_NUM_PIXEL     20  // Defines the default amount of LED Pixels
# define P042_MAX_PIXELS    300 // Max. allowed pixels
# define P042_RANDOM_PIXEL  70  // Defines the Flicker Level for Simple Candle
# define P042_BRIGHT_START  128 // Defines the Start Brightness
# define P042_FLAME_OPTIONS 8   // Number of flame types

enum SimType {
  TypeOff            = 0,
  TypeSimpleCandle   = 1,
  TypeAdvancedCandle = 2,
  TypeStaticLight    = 3,
  TypePolice         = 4,
  TypeBlink          = 5,
  TypeStrobe         = 6,
  TypeColorFader     = 7,
};

enum ColorType {
  ColorDefault  = 0,
  ColorSelected = 1,
};

uint8_t   Candle_red    = 0;
uint8_t   Candle_green  = 0;
uint8_t   Candle_blue   = 0;
uint8_t   Candle_bright = 128;
int       Candle_pxlcnt = P042_NUM_PIXEL;
SimType   Candle_type   = TypeSimpleCandle;
ColorType Candle_color  = ColorDefault;

// global variables
unsigned long Candle_Update = 0;
word Candle_Temp[3]         = { 0 }; // Temp variables
boolean GPIO_Set = false;

Adafruit_NeoPixel *Candle_pixels;

# define PLUGIN_042
# define PLUGIN_ID_042         42
# define PLUGIN_NAME_042       "Output - NeoPixel (Candle)"
# define PLUGIN_VALUENAME1_042 "Color"
# define PLUGIN_VALUENAME2_042 "Brightness"
# define PLUGIN_VALUENAME3_042 "Type"

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

boolean Plugin_042(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  // Webvar defines
  // Used in PLUGIN_WEBFORM_LOAD and PLUGIN_WEBFORM_SAVE
  const __FlashStringHelper *_webVars[P042_WEBVAR_COUNT] =
  { F(P042_WEBVAR_RED_S),
    F(P042_WEBVAR_GREEN_S),
    F(P042_WEBVAR_BLUE_S),
    F(P042_WEBVAR_BRIGHTNESS_S),
    F(P042_WEBVAR_CANDLETYPE_S),
    F(P042_WEBVAR_COLORTYPE_S),
    F(P042_WEBVAR_PIXELCOUNT_S),
  };

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_042;
      Device[deviceCount].Type               = DEVICE_TYPE_SINGLE;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TRIPLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 3;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = false;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_042);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_042));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_042));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_042));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_output(F("Data"));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P042_CONFIG_PIXELCOUNT = P042_NUM_PIXEL;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      html_add_script_arg(F("src=\"jscolor.min.js\""), true);
      html_add_script_end();

      if (P042_CONFIG_PIXELCOUNT == 0) { P042_CONFIG_PIXELCOUNT = P042_NUM_PIXEL; }

      addFormNumericBox(F("Led Count"), P042_WEBVAR_PIXELCOUNT, P042_CONFIG_PIXELCOUNT, 1, P042_MAX_PIXELS);

      {
        const __FlashStringHelper *options[P042_FLAME_OPTIONS] = {
          F("Off"),
          F("Static Light"),
          F("Simple Candle"),
          F("Advanced Candle"),
          F("Police"),
          F("Blink"),
          F("Strobe"),
          F("Color Fader")
        };

        if (P042_CONFIG_CANDLETYPE > P042_FLAME_OPTIONS - 1) {
          P042_CONFIG_CANDLETYPE = 2;
        }

        // Candle Type Selection
        addFormSelector(F("Flame Type"), P042_WEBVAR_CANDLETYPE, P042_FLAME_OPTIONS, options, nullptr, P042_CONFIG_CANDLETYPE);
      }

      // Advanced Color options
      Candle_color = static_cast<ColorType>(P042_CONFIG_COLORTYPE);
      addRowLabel(F("Color Handling")); // checked
      addHtml(F("<input type='radio' id='clrDef' name='" P042_WEBVAR_COLORTYPE_S "' value='0'"));

      if (Candle_color == ColorDefault) {
        addHtml(F(" checked>"));
      } else {
        addHtml('>');
      }
      addHtml(F("<label for='clrDef'> Use default color</label><br>"));
      addHtml(F("<input type='radio' id='clrSel' name='" P042_WEBVAR_COLORTYPE_S "' value='1'"));

      if (Candle_color == ColorSelected) {
        addHtml(F(" checked>"));
      } else {
        addHtml('>');
      }
      addHtml(F("<label for='clrSel'> Use selected color</label><br>"));

      // http://jscolor.com/examples/
      addRowLabel(F("Color"));
      addHtml(F("<input data-jscolor=\"{onInput:'update(this)',position:'top',value:'#"));
      addHtml(formatToHex_no_prefix(P042_CONFIG_RED, 2));
      addHtml(formatToHex_no_prefix(P042_CONFIG_GREEN, 2));
      addHtml(formatToHex_no_prefix(P042_CONFIG_BLUE, 2));
      addHtml(F("'}\">"));
      addFormNumericBox(F("RGB Color"), P042_WEBVAR_RED, P042_CONFIG_RED, 0, 255);
      addNumericBox(P042_WEBVAR_GREEN, P042_CONFIG_GREEN, 0, 255);
      addNumericBox(P042_WEBVAR_BLUE,  P042_CONFIG_BLUE,  0, 255);

      // Brightness Selection
      addRowLabel(F("Brightness"));
      addHtml(F("min<input type='range' id='" P042_OTHVAR_BRIGHTNESSSLIDE_S "' min='0' max='255' value='"));
      addHtmlInt(P042_CONFIG_BRIGHTNESS);
      addHtml(F("'> max"));

      {
        addFormNumericBox(F("Brightness Value"), P042_WEBVAR_BRIGHTNESS, P042_CONFIG_BRIGHTNESS, 0, 255);
      }

      // Some Javascript we need to update the items
      // function update(picker) {
      //     document.getElementById('wRed').value = Math.round(picker.channel('R'));
      //     document.getElementById('wGreen').value = Math.round(picker.channel('R'));
      //     document.getElementById('wBlue').value = Math.round(picker.channel('R'));
      // }
      // Minified:
      html_add_script(false);
      addHtml(F("function update(e){document.getElementById('" P042_WEBVAR_RED_S
                "').value=Math.round(e.channel('R')),"
                "document.getElementById('" P042_WEBVAR_GREEN_S
                "').value=Math.round(e.channel('G')),"
                "document.getElementById('" P042_WEBVAR_BLUE_S
                "').value=Math.round(e.channel('B'))}"));

      // Respond to slider moving:
      // window.addEventListener('load', function(){
      // var slider = document.getElementById('brSlide');
      // slider.addEventListener('change', function(){
      // document.getElementById('brText').value = this.value;
      // });});
      // Minified:
      addHtml(F("window.addEventListener('load',function(){document.getElementById('" P042_OTHVAR_BRIGHTNESSSLIDE_S
                "').addEventListener('input',function(){document.getElementById('" P042_WEBVAR_BRIGHTNESS_S
                "').value=this.value})})"));
      html_add_script_end();

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      for (int p = 0; p < P042_WEBVAR_COUNT; p++) {
        PCONFIG(p) = getFormItemInt(_webVars[p]);
      }

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      if (P042_CONFIG_PIXELCOUNT == 0) { P042_CONFIG_PIXELCOUNT = P042_NUM_PIXEL; }
      Candle_red    = P042_CONFIG_RED;
      Candle_green  = P042_CONFIG_GREEN;
      Candle_blue   = P042_CONFIG_BLUE;
      Candle_bright = P042_CONFIG_BRIGHTNESS;
      Candle_pxlcnt = P042_CONFIG_PIXELCOUNT;

      if ((Candle_red == 0) && (Candle_green == 0) && (Candle_blue == 0)) {
        Candle_bright = P042_BRIGHT_START;
      }
      Candle_type  = static_cast<SimType>(P042_CONFIG_CANDLETYPE);
      Candle_color = static_cast<ColorType>(P042_CONFIG_COLORTYPE);

      if (!Candle_pixels || (GPIO_Set == false)) {
        GPIO_Set = validGpio(CONFIG_PIN1);

        if (GPIO_Set) {
          if (Candle_pixels) {
            delete Candle_pixels;
          }
          Candle_pixels = new (std::nothrow) Adafruit_NeoPixel(P042_CONFIG_PIXELCOUNT, CONFIG_PIN1, NEO_GRB + NEO_KHZ800);

          if (Candle_pixels != nullptr) {
            SetPixelsBlack();
            Candle_pixels->setBrightness(Candle_bright);
            Candle_pixels->begin();
          }

          # ifndef BUILD_NO_DEBUG

          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            addLogMove(LOG_LEVEL_DEBUG, concat(F("CAND : Init WS2812 Pin : "), static_cast<int>(CONFIG_PIN1)));
          }
          # endif // ifndef BUILD_NO_DEBUG
        }
      }

      success = Candle_pixels != nullptr;
      break;
    }

    case PLUGIN_EXIT:
    {
      if (Candle_pixels != nullptr) {
        delete Candle_pixels;
        Candle_pixels = nullptr;
      }
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      Candle_pixels->setBrightness(Candle_bright);
      Candle_pixels->show(); // This sends the updated pixel color to the hardware.
      success = true;
      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      switch (Candle_type)
      {
        case TypeOff: // "Update" for OFF
        {
          SetPixelsBlack();
          break;
        }

        case TypeSimpleCandle: // Update for LIGHT
        {
          type_Static_Light();
          break;
        }

        case TypeAdvancedCandle: // Random Updates for Simple Candle, Advanced Candle, Fire Simulation
        case TypeStaticLight:
        {
          if (timeOutReached(Candle_Update)) {
            if (Candle_type == TypeAdvancedCandle) {
              type_Simple_Candle();
            }

            if (Candle_type == TypeStaticLight) {
              type_Advanced_Candle();
            }
            Candle_Update = millis() + random(25, 150);
          }
          break;
        }

        case TypePolice: // Update for Police
        {
          if (timeOutReached(Candle_Update)) {
            type_Police();
            Candle_Update = millis() + 150;
          }
          break;
        }

        case TypeBlink: // Update for Blink
        {
          if (timeOutReached(Candle_Update)) {
            type_BlinkStrobe();
            Candle_Update = millis() + 100;
          }
          break;
        }

        case TypeStrobe: // Update for Strobe
        {
          type_BlinkStrobe();
          break;
        }
        case TypeColorFader: // Update for ColorFader
        {
          if (timeOutReached(Candle_Update)) {
            type_ColorFader();
            Candle_Update = millis() + 2000;
          }
          break;
        }
      }

      Candle_pixels->show();

      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      UserVar[event->BaseVarIndex]     = Candle_red << 16 | Candle_green << 8 | Candle_blue;
      UserVar[event->BaseVarIndex + 1] = Candle_bright;
      UserVar[event->BaseVarIndex + 2] = Candle_type;

      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      char   cmdSep = ','; // Try comma first
      String cmd    = parseString(string, 1, cmdSep);

      if (!cmd.equals(F("candle"))) {
        cmdSep = ':'; // Fallback to colon
        cmd    = parseString(string, 1, cmdSep);
      }

      // Test
      // MQTT   : mosquitto_pub -d -t sensors/espeasy/ESP_Candle/cmd  -m "CANDLE_OFF"
      // HTTP   : http://192.168.30.183/tools?cmd=CANDLE%3A5%3AFF0000%3A200
      //          http://192.168.30.183/tools?cmd=CANDLE:4:FF0000:200
      // SERIAL : CANDLE:4:FF0000:200<CR><LF>

      // Commands
      // CANDLE:<FlameType>:<Color>:<Brightness>
      //    <FlameType>  : 1 Static Light, 2 Simple Candle, 3 Advanced Candle, 4 Police, 5 Blink, 6 Strobe, 7 Color Fader
      //    <Color>      : n.def.  Use the default color
      //                   RRGGBB  Use color in RRGGBB style (red, green blue) as HEX
      //    <Brightness> : 0-255
      // Samples:   CANDLE:2::100           Simple Candle with Default color and Brigthness at 100
      //            CANDLE:5:FF0000:200     Blink with RED Color and Brigthness at 200
      //            CANDLE:0::              Candle OFF
      //            CANDLE:1::255           Candle ON - White and full brigthness
      // 2023-01-21: command can also be in lowercase, and separator can either be comma or colon, but all have to be the same

      if (cmd.equals(F("candle"))) {
        const String val_Type   = parseString(string, 2, cmdSep);
        const String val_Color  = parseString(string, 3, cmdSep);
        const String val_Bright = parseString(string, 4, cmdSep);

        if (!val_Type.isEmpty()) {
          if ((val_Type.toInt() > -1) && (val_Type.toInt() < P042_FLAME_OPTIONS)) {
            P042_CONFIG_CANDLETYPE = val_Type.toInt(); // Type
            Candle_type            = static_cast<SimType>(P042_CONFIG_CANDLETYPE);
            # ifndef BUILD_NO_DEBUG

            if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
              addLogMove(LOG_LEVEL_DEBUG, concat(F("CAND : CMD - Type : "), val_Type));
            }
            # endif // ifndef BUILD_NO_DEBUG
          }
        }

        if (!val_Bright.isEmpty()) {
          if ((val_Bright.toInt() > -1) && (val_Bright.toInt() < 256)) {
            P042_CONFIG_BRIGHTNESS = val_Bright.toInt(); // Brightness
            Candle_bright          = P042_CONFIG_BRIGHTNESS;
            # ifndef BUILD_NO_DEBUG

            if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
              addLogMove(LOG_LEVEL_DEBUG, concat(F("CAND : CMD - Bright : "), val_Bright));
            }
            # endif // ifndef BUILD_NO_DEBUG
          }
        }

        uint32_t number = 0;

        if (!val_Color.isEmpty() && validUIntFromString(concat(F("0x"), val_Color), number)) {

          // Split RGB to r, g, b values
          P042_CONFIG_RED       = (number >> 16) & 0xFF;
          P042_CONFIG_GREEN     = (number >> 8) & 0xFF;
          P042_CONFIG_BLUE      = number & 0xFF;
          Candle_red            = P042_CONFIG_RED;
          Candle_green          = P042_CONFIG_GREEN;
          Candle_blue           = P042_CONFIG_BLUE;
          P042_CONFIG_COLORTYPE = 1;
          Candle_color          = static_cast<ColorType>(P042_CONFIG_COLORTYPE); // ColorType (ColorSelected)

          # ifndef BUILD_NO_DEBUG

          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            String log = F("CAND : CMD - R ");
            log += Candle_red;
            log += F(" G ");
            log += Candle_green;
            log += F(" B ");
            log += Candle_blue;
            addLogMove(LOG_LEVEL_DEBUG, log);
          }
          # endif // ifndef BUILD_NO_DEBUG
        } else {
          P042_CONFIG_COLORTYPE = 0;
          Candle_color          = static_cast<ColorType>(P042_CONFIG_COLORTYPE); // ColorType (ColorDefault)
          # ifndef BUILD_NO_DEBUG
          addLog(LOG_LEVEL_DEBUG, F("CAND : CMD - Color : DEFAULT"));
          # endif // ifndef BUILD_NO_DEBUG
        }

        success = true;
      }

      break;
    }
  }
  return success;
}

void SetPixelsBlack() {
  for (int i = 0; i < Candle_pxlcnt; i++) {
    Candle_pixels->setPixelColor(i, Candle_pixels->Color(0, 0, 0));
  }
}

void SetPixelToColor(int PixelIdx) {
  Candle_pixels->setPixelColor(PixelIdx, Candle_pixels->Color(Candle_red, Candle_green, Candle_blue));
}

void type_Static_Light() {
  for (int i = 0; i < Candle_pxlcnt; i++) {
    if (Candle_color == ColorDefault) {
      Candle_pixels->setPixelColor(i, 255, 255, 255); // Default is white
    } else {
      Candle_pixels->setPixelColor(i, Candle_red, Candle_green, Candle_blue);
    }
  }
}

void type_Simple_Candle() {
  int r, g, b;

  if (Candle_color == ColorDefault) {
    r = 226, g = 42, b =  35; // Regular (orange) flame
    // r = 158, g =   8, b = 148;   // Purple flame
    // r =  74, g = 150, b =  12;   // Green flame
  } else {
    r = Candle_red, g = Candle_green, b = Candle_blue;
  }

  //  Flicker, based on our initial RGB values
  for (int i = 0; i < Candle_pxlcnt; i++) {
    int flicker = random(0, P042_RANDOM_PIXEL);
    int r1      = r - flicker;
    int g1      = g - flicker;
    int b1      = b - flicker;

    if (g1 < 0) { g1 = 0; }

    if (r1 < 0) { r1 = 0; }

    if (b1 < 0) { b1 = 0; }
    Candle_pixels->setPixelColor(i, r1, g1, b1);
  }
}

void type_Advanced_Candle() {
  Candle_Temp[0] = random(1, 4);                  // 1..4  LEDs in RED
  Candle_Temp[1] = random(1, 4) + Candle_Temp[0]; // 1..3  LEDs in Yellow / Orange
  Candle_Temp[2] = random(0, 2);                  // 0..1  Choose Yellow = 0 / Orange = 1

  int colorbase[3];
  int color1[3];
  int color2[3];
  int color3[3];

  if (Candle_color == ColorDefault) {
    colorbase[0] = 255; colorbase[1] = 120; colorbase[2] = 0; // Light Orange #FF7800
    color1[0]    = 115; color1[1] = 50; color1[2] = 0;        // Brown      #733200
    color2[0]    = 180; color2[1] = 80; color2[2] = 0;        // Orange     #B45000
    color3[0]    =  70; color3[1] = 30; color3[2] = 0;        // Dark brown #4A2000
  } else {
    colorbase[0] = Candle_red; colorbase[1] = Candle_green; colorbase[2] = Candle_blue;
    float hsv[3];

    // Calc HSV
    RGB2HSV(Candle_red, Candle_green, Candle_blue, hsv);
    float newH = hsv[0] - 5.0f;

    if (newH < 0) { newH += 359.0f; }
    float newV  = hsv[2] / 2.0f;
    float newV2 = hsv[2] / 4.0f;

    // Calc new RGBs
    HSV2RGB(newH,   hsv[1], hsv[2], color1);
    HSV2RGB(hsv[0], hsv[1], newV,   color2);
    HSV2RGB(newH,   hsv[1], newV2,  color3);
  }

  for (int j = 0; j < 4; j++) {
    for (unsigned int i = 1; i < 6; i++) {
      if (i <= Candle_Temp[0]) {
        Candle_pixels->setPixelColor(j * 5 + i - 1, colorbase[0], colorbase[1], colorbase[2]);
      }

      if ((i > Candle_Temp[0]) && (i <= Candle_Temp[1])) {
        if (Candle_Temp[2] == 0) {
          Candle_pixels->setPixelColor(j * 5 + i - 1, color1[0], color1[1], color1[2]);
        } else {
          Candle_pixels->setPixelColor(j * 5 + i - 1, color2[0], color2[1], color2[2]);
        }
      }

      if (i > Candle_Temp[1]) {
        Candle_pixels->setPixelColor(j * 5 + i - 1, color3[0], color3[1], color3[2]);
      }
    }
  }
}

void type_Police() {
  Candle_Temp[0]++;

  if (Candle_Temp[0] > 3) {
    Candle_Temp[0] = 0;
  }

  for (unsigned int i = 0; i < 4; i++) {
    if (i == Candle_Temp[0])
    {
      for (int j = 0; j < 5; j++) {
        if (Candle_color == ColorDefault) {
          Candle_pixels->setPixelColor(i * 5 + j, 0, 0, 255);
        } else {
          Candle_pixels->setPixelColor(i * 5 + j, Candle_red, Candle_green, Candle_blue);
        }
      }
    } else {
      for (int j = 0; j < 5; j++) {
        Candle_pixels->setPixelColor(i * 5 + j, 0, 0, 0);
      }
    }
  }
}

void type_BlinkStrobe() {
  Candle_Temp[0]++;

  if (Candle_Temp[0] > 1) {
    Candle_Temp[0] = 0;
  }

  for (int i = 0; i < Candle_pxlcnt; i++) {
    if (Candle_Temp[0] == 0) {
      Candle_pixels->setPixelColor(i, 0, 0, 0);
    } else {
      if (Candle_color == ColorDefault) {
        Candle_pixels->setPixelColor(i, 255, 255, 255); // Default is white
      } else {
        Candle_pixels->setPixelColor(i, Candle_red, Candle_green, Candle_blue);
      }
    }
  }
}

void type_ColorFader() {
  int   colors[3];
  float hsv[3];

  if (Candle_color != ColorDefault) {
    if ((Candle_Temp[0] > 254) && (Candle_Temp[1] == 1)) {
      Candle_Temp[1] = 0;
    }

    if ((Candle_Temp[0] < 55) && (Candle_Temp[1] == 0)) {
      Candle_Temp[1] = 1;
    }

    if (Candle_Temp[1] > 0) {
      Candle_Temp[0]++;
    } else {
      Candle_Temp[0]--;
    }

    // Calc HSV
    // void RGB2HSV(uint8_t r, uint8_t g, uint8_t b, float hsv[3])
    RGB2HSV(Candle_red, Candle_green, Candle_blue, hsv);

    // Calc RGB with new V
    // HSV2RGB(float hue, float sat, float val, int colors[3])
    // hue: 0-359, sat: 0-255, val (lightness): 0-255
    HSV2RGB(hsv[0], hsv[1], Candle_Temp[0], colors);

    for (int i = 0; i < Candle_pxlcnt; i++) {
      Candle_pixels->setPixelColor(i, colors[0], colors[1], colors[2]);
    }
  } else {
    Candle_Temp[0]++;

    if (Candle_Temp[0] > 359) {
      Candle_Temp[0] = 0;
    }

    // hue: 0-359, sat: 0-255, val (lightness): 0-255
    HSV2RGB(Candle_Temp[0], 255, 255, colors);

    for (int i = 0; i < Candle_pxlcnt; i++) {
      Candle_pixels->setPixelColor(i, colors[0], colors[1], colors[2]);
    }
  }
}

#endif // USES_P042
