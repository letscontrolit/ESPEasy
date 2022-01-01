#include "_Plugin_Helper.h"

#ifdef USES_P128

// #######################################################################################################
// #################################### Plugin 128: NeoPixelBusFX ########################################
// #######################################################################################################

// Changelog:
// 2022-01-01, tonhuisman On request migrated from https://github.com/djcysmic/NeopixelBusFX to ESPEasy (that is an
//                        extension from plugin _P124_NeoPixelBusFX.ini on the ESPEasyPluginPlayground)
//                        - Adjusted to work combined with FastLED library (random8() issues)
//                        - Moved firetv.h data to P128_data_struct.h
//                        - Use PCONFIG(0) macro where applicable

/*
   List of commands:

   nfx off [fadetime] [delay]
   switches the stripe off

   nfx on [fadetime] [delay]
   restores last state of the stripe

   nfx dim [dimvalue]
   dimvalue 0-255

   nfx line startpixel endpixel color
   nfx hsvline startpixel endpixel hue saturation brightness

   nfx one pixel color
   nfx hsvone pixel hue saturation brightness

   nfx all color [fadetime] [delay]
   nfx rgb color [fadetime] [delay]
   nfx fade color [fadetime] [delay]

   nfx hsv hue saturation brightness [fadetime] [delay]

   nfx colorfade startcolor endcolor [startpixel] [endpixel]

   nfx rainbow [speed] [fadetime]

   nfx kitt color [speed]

   nfx comet color [speed]

   nfx theatre color [backgroundcolor] [count] [speed]

   nfx scan color [backgroundcolor] [speed]

   nfx dualscan color [backgroundcolor] [speed]

   nfx twinkle color [backgroundcolor] [speed]

   nfx twinklefade color [count] [speed]

   nfx sparkle color [backgroundcolor] [speed]

   nfx wipe color [dotcolor] [speed]

   nfx dualwipe [dotcolor] [speed]

   nfx fire [fps] [brightness] [cooling] [sparking]

   nfx fireflicker [intensity] [speed]

   nfx faketv [startpixel] [endpixel]

   nfx simpleclock [bigtickcolor] [smalltickcolor] [hourcolor] [minutecolor] [secondcolor, set "off" to disable] [backgroundcolor]


   nfx stop
   stops the effect

   nfx statusrequest
   sends status

   nfx fadetime
   nfx fadedelay
   nfx speed
   nfx count
   nfx bgcolor
   sets default parameter

   Use:

   needed:
   color,backgroundcolor -> targetcolor in hex format e.g. ff0000 for red

   [optional]:
   fadetime ->  fadetime per pixel in ms
   delay ->  delay time to next pixel in ms, if delay < 0 fades from other end of the stripe
   speed -> 0-50, speed < 0 for reverse

   Sourced from: https://github.com/djcysmic/NeopixelBusFX

   Based on Adafruit Fake TV Light for Engineers, WS2812FX, NeoPixelBus, Lights, NeoPixel - Basic and Candle modules

   https://learn.adafruit.com/fake-tv-light-for-engineers/overview
   https://github.com/letscontrolit/ESPEasy
   https://github.com/kitesurfer1404/WS2812FX
   https://github.com/Makuna/NeoPixelBus
   https://github.com/ddtlabs/ESPEasy-Plugin-Lights

   Thank you to all developers
 */

# include "src/PluginStructs/P128_data_struct.h" // includes faketv.h, the color pattern for FakeTV
# include <NeoPixelBrightnessBus.h>

# define SPEED_MAX 50
# define ARRAYSIZE 300      // Max LED Count

// Choose your color order below:

# define GRB // should be standard - SK6812(grb), WS2811, and WS2812
// # define GRBW  //This is used for SK6812rgbw pixels that have the separate white led in them.
// # define RGB   //some older pixels
// # define RGBW  //A four element color in the order of Red, Green, Blue, and then White. A common four element format.
// # define BRG   //A three element color in the order of Blue, Red, and then Green.
// # define RBG   //A three element color in the order of Red, Blue, and then Green.

# define NEOPIXEL_LIB NeoPixelBrightnessBus // Neopixel library type
# ifdef ESP32
#  define METHOD DotStarEsp32DmaHspiMethod  // HSPI MOSI - use DotStarEsp32DmaHspiMethod (should also work on ESP32S2)
# elif ESP8266
#  ifdef METHOD
#   undef METHOD
#  endif // ifdef METHOD
#  define METHOD NeoEsp8266Uart1800KbpsMethod // GPIO2 - use NeoEsp8266Uart0800KbpsMethod for GPIO1(TX)
# endif  // ifdef ESP32

# if defined GRB
  #  define FEATURE NeoGrbFeature
# elif defined GRBW
  #  define FEATURE NeoGrbwFeature
# elif defined RGB
  #  define FEATURE NeoRgbFeature
# elif defined RGBW
  #  define FEATURE NeoRgbwFeature
# elif defined BRG
  #  define FEATURE NeoBrgFeature
# elif defined RBG
  #  define FEATURE NeoRbgFeature
# else // if defined GRB
  #  define FEATURE NeoGrbFeature
# endif // if defined GRB

# define  numPixels (sizeof(ftv_colors) / sizeof(ftv_colors[0]))

NEOPIXEL_LIB<FEATURE, METHOD> *Plugin_128_pixels = NULL;

const float pi = 3.1415926535897932384626433832795;

uint16_t pos,
         color,
         r_pixel,
         startpixel,
         endpixel,
         difference,
         fps = 50,
         colorcount;

# if defined(RGBW) || defined(GRBW)
RgbwColor rgb_target[ARRAYSIZE],
          rgb_old[ARRAYSIZE],
          rgb, rrggbb,
          rgb_tick_b = HtmlColor(0x505050),
          rgb_tick_s = HtmlColor(0x101010),
          rgb_m      = HtmlColor(0x00FF00),
          rgb_h      = HtmlColor(0x0000FF),
          rgb_s      = HtmlColor(0xFF0000);
# else // if defined(RGBW) || defined(GRBW)
RgbColor rgb_target[ARRAYSIZE],
         rgb_old[ARRAYSIZE],
         rgb, rrggbb,
         rgb_tick_b = HtmlColor(0x505050),
         rgb_tick_s = HtmlColor(0x101010),
         rgb_m      = HtmlColor(0x00FF00),
         rgb_h      = HtmlColor(0x0000FF),
         rgb_s      = HtmlColor(0xFF0000);
# endif // if defined(RGBW) || defined(GRBW)

int16_t fadedelay = 20;

uint16_t pixelCount = ARRAYSIZE;

int8_t defaultspeed  = 25,
       rainbowspeed  = 1,
       speed         = 25,
       count         = 1,
       rev_intensity = 3;

uint32_t _counter_mode_step = 0,
         fadetime           = 1000,
         ftv_holdTime,
         pixelNum;

uint16_t ftv_pr = 0, ftv_pg = 0, ftv_pb = 0; // Prev R, G, B;
uint32_t ftv_totalTime, ftv_fadeTime, ftv_startTime, ftv_elapsed;
uint16_t ftv_nr, ftv_ng, ftv_nb, ftv_r, ftv_g, ftv_b, ftv_i, ftv_frac;
uint8_t  ftv_hi, ftv_lo, ftv_r8, ftv_g8, ftv_b8;

String colorStr,
       backgroundcolorStr;

bool gReverseDirection = false;
bool rgb_s_off         = false;
bool fadeIn            = false;

byte cooling    = 50,
     sparking   = 120,
     brightness = 31;

unsigned long counter20ms = 0,
              starttime[ARRAYSIZE],
              starttimerb,
              maxtime = 0;

enum modetype {
  Off, On, Fade, ColorFade, Rainbow, Kitt, Comet,
  Theatre, Scan, Dualscan, Twinkle, TwinkleFade, Sparkle, Fire,
  FireFlicker, Wipe, Dualwipe,  FakeTV, SimpleClock
};

const char *modeName[] = {
  "off",         "on",         "fade",           "colorfade",           "rainbow",                 "kitt",                 "comet",
  "theatre",     "scan",       "dualscan",       "twinkle",             "twinklefade",             "sparkle",              "fire",
  "fireflicker", "wipe",       "dualwipe",       "faketv",              "simpleclock"
};

modetype mode, savemode, lastmode;

# define PLUGIN_128
# define PLUGIN_ID_128         128
# define PLUGIN_NAME_128       "Output - NeoPixel (BusFX) [DEVELOPMENT]"
# define PLUGIN_VALUENAME1_128 "Mode"
# define PLUGIN_VALUENAME2_128 "Lastmode"
# define PLUGIN_VALUENAME3_128 "Fadetime"
# define PLUGIN_VALUENAME4_128 "Fadedelay"

boolean Plugin_128(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_128;
      Device[deviceCount].Type               = DEVICE_TYPE_DUMMY;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].Custom             = true;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 4;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].TimerOptional      = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].DecimalsOnly       = false;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_128);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_128));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_128));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_128));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_128));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addRowLabel(F(""));
      String message = F("<span style=\"color:red\">Please connect stripe to ");
      # ifdef ESP32
      message += F("HSPI MOSI!</span>");
      # elif ESP8266
      message += F("GPIO2!</span>");
      # endif // ifdef ESP32
      addHtml(message);
      addFormNumericBox(F("Led Count"), F("plugin_128_leds"), PCONFIG(0), 1, 999);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("plugin_128_leds"));

      // Settings.TaskDevicePin1[event->TaskIndex] = getFormItemInt(F("taskdevicepin1"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      if (!Plugin_128_pixels) {
        # ifdef ESP8266
        Plugin_128_pixels = new NEOPIXEL_LIB<FEATURE, METHOD>(PCONFIG(0));
        # endif // ifdef ESP8266
        # ifdef ESP32
        Plugin_128_pixels = new NEOPIXEL_LIB<FEATURE, METHOD>(PCONFIG(0));
        # endif // ifdef ESP32
        Plugin_128_pixels->Begin(); // This initializes the NeoPixelBus library.
      }

      pixelCount = PCONFIG(0);
      success    = true;
      break;
    }

    case PLUGIN_READ: // ------------------------------------------->
    {
      // there is no need to read them, just use current values
      UserVar[event->BaseVarIndex]     = mode;
      UserVar[event->BaseVarIndex + 1] = savemode;
      UserVar[event->BaseVarIndex + 2] = fadetime;
      UserVar[event->BaseVarIndex + 3] = fadedelay;

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log;
        log  = F("Lights: mode: ");
        log += modeName[mode];
        log += F(" lastmode: ");
        log += modeName[savemode];
        log += F(" fadetime: ");
        log += (int)UserVar[event->BaseVarIndex + 2];
        log += F(" fadedelay: ");
        log += (int)UserVar[event->BaseVarIndex + 3];
        addLog(LOG_LEVEL_INFO, log);
      }

      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      String log     = "";
      String command = parseString(string, 1);

      if ((command == F("neopixelfx")) || (command == F("nfx"))) {
        success = true;
        String subCommand = parseString(string, 2);

        if (subCommand == F("fadetime")) {
          fadetime = parseString(string, 3).toInt();
        }

        else if (subCommand == F("fadedelay")) {
          fadedelay = parseString(string, 3).toInt();
        }

        else if (subCommand == F("speed")) {
          defaultspeed = parseString(string, 3).toInt();
          speed        = defaultspeed;
        }

        else if (subCommand == F("bgcolor")) {
          hex2rrggbb(parseString(string, 3));
        }

        else if (subCommand == F("count")) {
          count = parseString(string, 3).toInt();
        }

        else if ((subCommand == F("on")) || (subCommand == F("off"))) {
          fadetime  = 1000;
          fadedelay = 0;

          fadetime = (parseString(string, 3) == "")
          ? fadetime
          : parseString(string, 3).toInt();
          fadedelay = (parseString(string, 4) == "")
          ? fadedelay
          : parseString(string, 4).toInt();

          for (int pixel = 0; pixel < pixelCount; pixel++) {
            r_pixel = (fadedelay < 0)
            ? pixelCount - pixel - 1
            : pixel;

            starttime[r_pixel] = counter20ms + (pixel * abs(fadedelay) / 20);

            if ((subCommand == F("on")) && (mode == Off)) { // switch on
              rgb_target[pixel] = rgb_old[pixel];
              rgb_old[pixel]    = Plugin_128_pixels->GetPixelColor(pixel);
            } else if (subCommand == F("off")) {            // switch off
              rgb_old[pixel]    = Plugin_128_pixels->GetPixelColor(pixel);
              rgb_target[pixel] = RgbColor(0);
            }
          }

          if ((subCommand == F("on")) && (mode == Off)) { // switch on
            mode = (savemode == On) ? Fade : savemode;
          } else if (subCommand == F("off")) {            // switch off
            savemode = mode;
            mode     = Fade;
          }

          maxtime = starttime[r_pixel] + (fadetime / 20);
        }

        else if (subCommand == F("dim")) {
          Plugin_128_pixels->SetBrightness(parseString(string, 3).toInt());
        }

        else if (subCommand == F("line")) {
          mode = On;

          hex2rgb(parseString(string, 5));

          for (int i = 0; i <= (parseString(string, 4).toInt() - parseString(string, 3).toInt() + pixelCount) % pixelCount; i++) {
            Plugin_128_pixels->SetPixelColor((i + parseString(string, 3).toInt() - 1) % pixelCount, rgb);
          }
        }

        else if (subCommand == F("tick")) {
          mode = On;

          hex2rgb(parseString(string, 4));

          //          for (int i = 0; i < pixelCount ; i = i + (pixelCount / parseString(string, 3).toInt())) {
          for (int i = 0; i < parseString(string, 3).toInt(); i++) {
            Plugin_128_pixels->SetPixelColor(i * pixelCount / parseString(string, 3).toInt(), rgb);
          }
        }

        else if (subCommand == F("one")) {
          mode = On;

          uint16_t pixnum = parseString(string, 3).toInt() - 1;
          hex2rgb(parseString(string, 4));

          Plugin_128_pixels->SetPixelColor(pixnum, rgb);
        }

        else if ((subCommand == F("fade")) || (subCommand == F("all")) || (subCommand == F("rgb"))) {
          mode = Fade;

          if ((subCommand == F("all")) || (subCommand == F("rgb"))) {
            fadedelay = 0;
          }

          hex2rgb(parseString(string, 3));
          hex2rgb_pixel(parseString(string, 3));

          fadetime = (parseString(string, 4) == "")
          ? fadetime
          : parseString(string, 4).toInt();
          fadedelay = (parseString(string, 5) == "")
          ? fadedelay
          : parseString(string, 5).toInt();

          for (int pixel = 0; pixel < pixelCount; pixel++) {
            r_pixel = (fadedelay < 0)
            ? pixelCount - pixel - 1
            : pixel;

            starttime[r_pixel] = counter20ms + (pixel * abs(fadedelay) / 20);

            rgb_old[pixel] = Plugin_128_pixels->GetPixelColor(pixel);
          }
          maxtime = starttime[r_pixel] + (fadetime / 20);
        }

        else if (subCommand == F("hsv")) {
          mode      = Fade;
          fadedelay = 0;
          rgb       =
            RgbColor(HsbColor(parseString(string, 3).toFloat() / 360, parseString(string, 4).toFloat() / 100,
                              parseString(string, 5).toFloat() / 100));

          colorStr               = "";
          rgb.R < 16 ? colorStr  = "0" : "";
          colorStr              += formatToHex(rgb.R, {});
          rgb.G < 16 ? colorStr += "0" : "";
          colorStr              += formatToHex(rgb.G, {});
          rgb.B < 16 ? colorStr += "0" : "";
          colorStr              += formatToHex(rgb.B, {});

          hex2rgb_pixel(colorStr);

          fadetime = (parseString(string, 6) == "")
          ? fadetime
          : parseString(string, 6).toInt();
          fadedelay = (parseString(string, 7) == "")
          ? fadedelay
          : parseString(string, 7).toInt();

          for (int pixel = 0; pixel < pixelCount; pixel++) {
            r_pixel = (fadedelay < 0)
            ? pixelCount - pixel - 1
            : pixel;

            starttime[r_pixel] = counter20ms + (pixel * abs(fadedelay) / 20);

            rgb_old[pixel] = Plugin_128_pixels->GetPixelColor(pixel);
          }
          maxtime = starttime[r_pixel] + (fadetime / 20);
        }

        else if (subCommand == F("hsvone")) {
          mode = On;
          rgb  =
            RgbColor(HsbColor(parseString(string, 4).toFloat() / 360, parseString(string, 5).toFloat() / 100,
                              parseString(string, 6).toFloat() / 100));

          colorStr               = "";
          rgb.R < 16 ? colorStr  = "0" : "";
          colorStr              += formatToHex(rgb.R, {});
          rgb.G < 16 ? colorStr += "0" : "";
          colorStr              += formatToHex(rgb.G, {});
          rgb.B < 16 ? colorStr += "0" : "";
          colorStr              += formatToHex(rgb.B, {});

          hex2rgb(colorStr);
          uint16_t pixnum = parseString(string, 3).toInt() - 1;
          Plugin_128_pixels->SetPixelColor(pixnum, rgb);
        }

        else if (subCommand == F("hsvline")) {
          mode = On;

          rgb =
            RgbColor(HsbColor(parseString(string, 5).toFloat() / 360, parseString(string, 6).toFloat() / 100,
                              parseString(string, 7).toFloat() / 100));

          colorStr               = "";
          rgb.R < 16 ? colorStr  = "0" : "";
          colorStr              += formatToHex(rgb.R, {});
          rgb.G < 16 ? colorStr += "0" : "";
          colorStr              += formatToHex(rgb.G, {});
          rgb.B < 16 ? colorStr += "0" : "";
          colorStr              += formatToHex(rgb.B, {});

          hex2rgb(colorStr);

          for (int i = 0; i <= (parseString(string, 4).toInt() - parseString(string, 3).toInt() + pixelCount) % pixelCount; i++) {
            Plugin_128_pixels->SetPixelColor((i + parseString(string, 3).toInt() - 1) % pixelCount, rgb);
          }
        }

        else if (subCommand == F("rainbow")) {
          fadeIn      = (mode == Off) ? true : false;
          mode        = Rainbow;
          starttimerb = counter20ms;

          rainbowspeed = (parseString(string, 3) == "")
          ? speed
          : parseString(string, 3).toInt();

          fadetime = (parseString(string, 4) == "")
          ? fadetime
          : parseString(string, 4).toInt();
        }

        else if (subCommand == F("colorfade")) {
          mode = ColorFade;

          hex2rgb(parseString(string, 3));

          if (parseString(string, 4) != "") { hex2rrggbb(parseString(string, 4)); }

          startpixel = (parseString(string, 5) == "")
          ? 0
          : parseString(string, 5).toInt() - 1;
          endpixel = (parseString(string, 6) == "")
          ? pixelCount - 1
          : parseString(string, 6).toInt() - 1;
        }

        else if (subCommand == F("kitt")) {
          mode = Kitt;

          _counter_mode_step = 0;

          hex2rgb(parseString(string, 3));

          speed = (parseString(string, 4) == "")
          ? defaultspeed
          : parseString(string, 4).toInt();
        }

        else if (subCommand == F("comet")) {
          mode = Comet;

          _counter_mode_step = 0;

          hex2rgb(parseString(string, 3));

          speed = (parseString(string, 4) == "")
          ? defaultspeed
          : parseString(string, 4).toInt();
        }

        else if (subCommand == F("theatre")) {
          mode = Theatre;

          hex2rgb(parseString(string, 3));

          if (parseString(string, 4) != "") { hex2rrggbb(parseString(string, 4)); }

          count = (parseString(string, 5) == "")
          ? count
          : parseString(string, 5).toInt();

          speed = (parseString(string, 6) == "")
          ? defaultspeed
          : parseString(string, 6).toInt();

          for (int i = 0; i < pixelCount; i++) {
            if ((i / count) % 2 == 0) {
              Plugin_128_pixels->SetPixelColor(i, rgb);
            } else {
              Plugin_128_pixels->SetPixelColor(i, rrggbb);
            }
          }
        }

        else if (subCommand == F("scan")) {
          mode = Scan;

          _counter_mode_step = 0;

          hex2rgb(parseString(string, 3));

          if (parseString(string, 4) != "") { hex2rrggbb(parseString(string, 4)); }

          speed = (parseString(string, 5) == "")
          ? defaultspeed
          : parseString(string, 5).toInt();
        }

        else if (subCommand == F("dualscan")) {
          mode = Dualscan;

          _counter_mode_step = 0;

          hex2rgb(parseString(string, 3));

          if (parseString(string, 4) != "") { hex2rrggbb(parseString(string, 4)); }

          speed = (parseString(string, 5) == "")
          ? defaultspeed
          : parseString(string, 5).toInt();
        }

        else if (subCommand == F("twinkle")) {
          mode = Twinkle;

          _counter_mode_step = 0;

          hex2rgb(parseString(string, 3));

          if (parseString(string, 4) != "") { hex2rrggbb(parseString(string, 4)); }

          speed = (parseString(string, 5) == "")
          ? defaultspeed
          : parseString(string, 5).toInt();
        }

        else if (subCommand == F("twinklefade")) {
          mode = TwinkleFade;

          hex2rgb(parseString(string, 3));

          count = (parseString(string, 4) == "")
          ? count
          : parseString(string, 4).toInt();

          speed = (parseString(string, 5) == "")
          ? defaultspeed
          : parseString(string, 5).toInt();
        }

        else if (subCommand == F("sparkle")) {
          mode = Sparkle;

          _counter_mode_step = 0;

          hex2rgb(parseString(string, 3));
          hex2rrggbb(parseString(string, 4));

          speed = (parseString(string, 5) == "")
          ? defaultspeed
          : parseString(string, 5).toInt();
        }

        else if (subCommand == F("wipe")) {
          mode = Wipe;

          _counter_mode_step = 0;

          hex2rgb(parseString(string, 3));

          if (parseString(string, 4) != "") {
            hex2rrggbb(parseString(string, 4));
          } else {
            hex2rrggbb("000000");
          }

          speed = (parseString(string, 5) == "")
          ? defaultspeed
          : parseString(string, 5).toInt();
        }

        else if (subCommand == F("dualwipe")) {
          mode = Dualwipe;

          _counter_mode_step = 0;

          hex2rgb(parseString(string, 3));

          if (parseString(string, 4) != "") {
            hex2rrggbb(parseString(string, 4));
          } else {
            hex2rrggbb("000000");
          }

          speed = (parseString(string, 5) == "")
          ? defaultspeed
          : parseString(string, 5).toInt();
        }

        else if (subCommand == F("faketv")) {
          mode               = FakeTV;
          _counter_mode_step = 0;

          randomSeed(analogRead(A0));
          pixelNum = random(numPixels); // Begin at random point

          startpixel = (parseString(string, 3) == "")
          ? 0
          : parseString(string, 3).toInt() - 1;
          endpixel = (parseString(string, 4) == "")
          ? pixelCount
          : parseString(string, 4).toInt();
        }

        else if (subCommand == F("fire")) {
          mode = Fire;

          fps = (parseString(string, 3) == "")
          ? fps
          : parseString(string, 3).toInt();

          fps = (fps == 0 || fps > 50) ? 50 : fps;

          brightness = (parseString(string, 4) == "")
          ? brightness
          : parseString(string, 4).toFloat();
          cooling = (parseString(string, 5) == "")
          ? cooling
          : parseString(string, 5).toFloat();
          sparking = (parseString(string, 6) == "")
          ? sparking
          : parseString(string, 6).toFloat();
        }

        else if (subCommand == F("fireflicker")) {
          mode = FireFlicker;

          rev_intensity = (parseString(string, 3) == "")
          ? rev_intensity
          : parseString(string, 3).toInt();

          speed = (parseString(string, 4) == "")
          ? defaultspeed
          : parseString(string, 4).toInt();
        }

        else if (subCommand == F("simpleclock")) {
          mode = SimpleClock;

          # if defined(RGBW) || defined(GRBW)

          if (parseString(string, 3) != "") {
            if (parseString(string, 3).length() <= 6) {
              rgb_tick_s =
                RgbwColor(rgbStr2Num(parseString(string, 3)) >> 16, rgbStr2Num(parseString(string, 3)) >> 8,
                          rgbStr2Num(parseString(string, 3)));
            } else {
              rgb_tick_s =
                RgbwColor(rgbStr2Num(parseString(string, 3)) >> 24,
                          rgbStr2Num(parseString(string, 3)) >> 16,
                          rgbStr2Num(parseString(string, 3)) >> 8,
                          rgbStr2Num(parseString(string, 3)));
            }
          }

          if (parseString(string, 4) != "") {
            if (parseString(string, 4).length() <= 6) {
              rgb_tick_b =
                RgbwColor(rgbStr2Num(parseString(string, 4)) >> 16, rgbStr2Num(parseString(string, 4)) >> 8,
                          rgbStr2Num(parseString(string, 4)));
            } else {
              rgb_tick_b =
                RgbwColor(rgbStr2Num(parseString(string, 4)) >> 24,
                          rgbStr2Num(parseString(string, 4)) >> 16,
                          rgbStr2Num(parseString(string, 4)) >> 8,
                          rgbStr2Num(parseString(string, 4)));
            }
          }

          if (parseString(string, 5) != "") {
            if (parseString(string, 5).length() <= 6) {
              rgb_h =
                RgbwColor(rgbStr2Num(parseString(string, 5)) >> 16, rgbStr2Num(parseString(string, 5)) >> 8,
                          rgbStr2Num(parseString(string, 5)));
            } else {
              rgb_h =
                RgbwColor(rgbStr2Num(parseString(string, 5)) >> 24,
                          rgbStr2Num(parseString(string, 5)) >> 16,
                          rgbStr2Num(parseString(string, 5)) >> 8,
                          rgbStr2Num(parseString(string, 5)));
            }
          }

          if (parseString(string, 6) != "") {
            if (parseString(string, 6).length() <= 6) {
              rgb_m =
                RgbwColor(rgbStr2Num(parseString(string, 6)) >> 16, rgbStr2Num(parseString(string, 6)) >> 8,
                          rgbStr2Num(parseString(string, 6)));
            } else {
              rgb_m =
                RgbwColor(rgbStr2Num(parseString(string, 6)) >> 24,
                          rgbStr2Num(parseString(string, 6)) >> 16,
                          rgbStr2Num(parseString(string, 6)) >> 8,
                          rgbStr2Num(parseString(string, 6)));
            }
          }

          if (parseString(string, 7) != "") {
            if (parseString(string, 7) == "off") {
              rgb_s_off = true;
            } else if (parseString(string, 7).length() <= 6) {
              rgb_s_off = false;
              rgb_s     =
                RgbwColor(rgbStr2Num(parseString(string, 7)) >> 16, rgbStr2Num(parseString(string, 7)) >> 8,
                          rgbStr2Num(parseString(string, 7)));
            } else {
              rgb_s_off = false;
              rgb_s     =
                RgbwColor(rgbStr2Num(parseString(string, 7)) >> 24,
                          rgbStr2Num(parseString(string, 7)) >> 16,
                          rgbStr2Num(parseString(string, 7)) >> 8,
                          rgbStr2Num(parseString(string, 7)));
            }
          }

          if (parseString(string, 8) != "") {
            hex2rrggbb(parseString(string, 8));
          }

          # else // if defined(RGBW) || defined(GRBW)

          rgb_tick_s = (parseString(string, 3) == "")
          ? rgb_tick_s
          : RgbColor(rgbStr2Num(parseString(string, 3)) >> 16, rgbStr2Num(parseString(string, 3)) >> 8, rgbStr2Num(parseString(string, 3)));
          rgb_tick_b = (parseString(string, 4) == "")
          ? rgb_tick_b
          : RgbColor(rgbStr2Num(parseString(string, 4)) >> 16, rgbStr2Num(parseString(string, 4)) >> 8, rgbStr2Num(parseString(string, 4)));
          rgb_h = (parseString(string, 5) == "")
          ? rgb_h
          : RgbColor(rgbStr2Num(parseString(string, 5)) >> 16, rgbStr2Num(parseString(string, 5)) >> 8, rgbStr2Num(parseString(string, 5)));
          rgb_m = (parseString(string, 6) == "")
          ? rgb_m
          : RgbColor(rgbStr2Num(parseString(string, 6)) >> 16, rgbStr2Num(parseString(string, 6)) >> 8, rgbStr2Num(parseString(string, 6)));

          if (parseString(string, 7) != "") {
            if (parseString(string, 7) == "off") {
              rgb_s_off = true;
            } else {
              rgb_s_off = false;
              rgb_s     =
                RgbColor(rgbStr2Num(parseString(string, 7)) >> 16, rgbStr2Num(parseString(string, 7)) >> 8,
                         rgbStr2Num(parseString(string, 7)));
            }
          }

          if (parseString(string, 8) != "") { hex2rrggbb(parseString(string, 8)); }

          # endif // if defined(RGBW) || defined(GRBW)
        }

        else if (subCommand == F("stop")) {
          mode = On;
        }

        else if (subCommand == F("statusrequest")) {}

        else if ((subCommand != F("all")) && (subCommand != F("line"))
                 && (subCommand != F("one")) && (subCommand != F("fade"))
                 && (subCommand != F("dim")) && (subCommand != F("fadetime"))
                 && (subCommand != F("speed")) && (subCommand != F("fadedelay"))
                 && (subCommand != F("count")) && (subCommand != F("bgcolor"))
                 && (subCommand != F("on")) && (subCommand != F("off"))
                 && (subCommand != F("rgb")) && (subCommand != F("rainbow"))
                 && (subCommand != F("kitt")) && (subCommand != F("comet"))
                 && (subCommand != F("theatre")) && (subCommand != F("scan"))
                 && (subCommand != F("dualscan")) && (subCommand != F("twinkle"))
                 && (subCommand != F("sparkle")) && (subCommand != F("fire"))
                 && (subCommand != F("fireflicker")) && (subCommand != F("hsvone"))
                 && (subCommand != F("hsv")) && (subCommand != F("hsvline"))
                 && (subCommand != F("twinklefade")) && (subCommand != F("stop"))
                 && (subCommand != F("wipe")) && (subCommand != F("dualwipe"))
                 && (subCommand != F("colorfade")) && (subCommand != F("simpleclock"))
                 && (subCommand != F("faketv")) && (subCommand != F("statusrequest"))) {
          log  = F("NeoPixelBus: unknown subcommand: ");
          log += subCommand;
          addLog(LOG_LEVEL_INFO, log);

          String json;
          printToWebJSON = true;
          json          += F("{\n");
          json          += F("\"plugin\": \"128\",\n");
          json          += F("\"log\": \"");
          json          += F("NeoPixelBus: unknown command: ");
          json          += subCommand;
          json          += F("\"\n");
          json          += F("}\n");

          //          event->Source=EventValueSource::Enum::VALUE_SOURCE_HTTP;
          SendStatus(event, json); // send http response to controller (JSON format)
          printToWeb = false;
        }
        NeoPixelSendStatus(event);
      } // command neopixel

      if (speed == 0) { mode = On; // speed = 0 = stop mode
      }
      speed    = (speed > SPEED_MAX || speed < -SPEED_MAX) ? defaultspeed : speed; // avoid invalid values
      fadetime = (fadetime <= 0) ? 20 : fadetime;

      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      counter20ms++;
      lastmode = mode;

      switch (mode) {
        case Fade:
          fade();
          break;

        case ColorFade:
          colorfade();
          break;

        case Rainbow:
          rainbow();
          break;

        case Kitt:
          kitt();
          break;

        case Comet:
          comet();
          break;

        case Theatre:
          theatre();
          break;

        case Scan:
          scan();
          break;

        case Dualscan:
          dualscan();
          break;

        case Twinkle:
          twinkle();
          break;

        case TwinkleFade:
          twinklefade();
          break;

        case Sparkle:
          sparkle();
          break;

        case Fire:
          fire();
          break;

        case FireFlicker:
          fire_flicker();
          break;

        case Wipe:
          wipe();
          break;

        case Dualwipe:
          dualwipe();
          break;

        case FakeTV:
          faketv();
          break;

        case SimpleClock:
          Plugin_128_simpleclock();
          break;

        default:
          break;
      } // switch mode

      Plugin_128_pixels->Show();

      if (mode != lastmode) {
        String log = "";
        log  = F("NeoPixelBus: Mode Change: ");
        log += modeName[mode];
        addLog(LOG_LEVEL_INFO, log);
        NeoPixelSendStatus(event);
      }
      success = true;
      break;
    }
  }
  return success;
}

void fade(void) {
  for (int pixel = 0; pixel < pixelCount; pixel++) {
    long  zaehler  = 20 * (counter20ms - starttime[pixel]);
    float progress = (float)zaehler / (float)fadetime;
    progress = (progress < 0) ? 0 : progress;
    progress = (progress > 1) ? 1 : progress;

    # if defined(RGBW) || defined(GRBW)
    RgbwColor updatedColor = RgbwColor::LinearBlend(
      rgb_old[pixel], rgb_target[pixel],
      progress);
    # else // if defined(RGBW) || defined(GRBW)
    RgbColor updatedColor = RgbColor::LinearBlend(
      rgb_old[pixel], rgb_target[pixel],
      progress);
    # endif // if defined(RGBW) || defined(GRBW)

    if ((counter20ms > maxtime) && (Plugin_128_pixels->GetPixelColor(pixel).CalculateBrightness() == 0)) {
      mode = Off;
    } else if (counter20ms > maxtime) {
      mode = On;
    }

    Plugin_128_pixels->SetPixelColor(pixel, updatedColor);
  }
}

void colorfade(void) {
  float progress = 0;

  difference = (endpixel - startpixel + pixelCount) % pixelCount;

  for (uint16_t i = 0; i <= difference; i++)
  {
    progress = (float)i / (difference - 1);
    progress = (progress >= 1) ? 1 : progress;
    progress = (progress <= 0) ? 0 : progress;

    # if defined(RGBW) || defined(GRBW)
    RgbwColor updatedColor = RgbwColor::LinearBlend(
      rgb, rrggbb,
      progress);
    # else // if defined(RGBW) || defined(GRBW)
    RgbColor updatedColor = RgbColor::LinearBlend(
      rgb, rrggbb,
      progress);
    # endif // if defined(RGBW) || defined(GRBW)

    Plugin_128_pixels->SetPixelColor((i + startpixel) % pixelCount, updatedColor);
  }
  mode = On;
}

void wipe(void) {
  if (counter20ms % (unsigned long)(SPEED_MAX / abs(speed)) == 0) {
    if (speed > 0) {
      Plugin_128_pixels->SetPixelColor(_counter_mode_step, rrggbb);

      if (_counter_mode_step > 0) { Plugin_128_pixels->SetPixelColor(_counter_mode_step - 1, rgb); }
    } else {
      Plugin_128_pixels->SetPixelColor(pixelCount - _counter_mode_step - 1, rrggbb);

      if (_counter_mode_step > 0) { Plugin_128_pixels->SetPixelColor(pixelCount - _counter_mode_step, rgb); }
    }

    if (_counter_mode_step == pixelCount) { mode = On; }
    _counter_mode_step++;
  }
}

void dualwipe(void) {
  if (counter20ms % (unsigned long)(SPEED_MAX / abs(speed)) == 0) {
    if (speed > 0) {
      int i = _counter_mode_step - pixelCount;
      i = abs(i);
      Plugin_128_pixels->SetPixelColor(_counter_mode_step, rrggbb);
      Plugin_128_pixels->SetPixelColor(i,                  rgb);

      if (_counter_mode_step > 0) {
        Plugin_128_pixels->SetPixelColor(_counter_mode_step - 1, rgb);
        Plugin_128_pixels->SetPixelColor(i - 1,                  rrggbb);
      }
    } else {
      int i = (pixelCount / 2) - _counter_mode_step;
      i = abs(i);
      Plugin_128_pixels->SetPixelColor(_counter_mode_step + (pixelCount / 2), rrggbb);
      Plugin_128_pixels->SetPixelColor(i,                                     rgb);

      if (_counter_mode_step > 0) {
        Plugin_128_pixels->SetPixelColor(_counter_mode_step + (pixelCount / 2) - 1, rgb);
        Plugin_128_pixels->SetPixelColor(i - 1,                                     rrggbb);
      }
    }

    if (_counter_mode_step >= pixelCount / 2) {
      mode = On;
      Plugin_128_pixels->SetPixelColor(_counter_mode_step - 1, rgb);
    }
    _counter_mode_step++;
  }
}

void faketv(void) {
  if (counter20ms >= ftv_holdTime) {
    difference = abs(endpixel - startpixel);

    if (ftv_elapsed >= ftv_fadeTime) {
      // Read next 16-bit (5/6/5) color
      ftv_hi = pgm_read_byte(&ftv_colors[pixelNum * 2]);
      ftv_lo = pgm_read_byte(&ftv_colors[pixelNum * 2 + 1]);

      if (++pixelNum >= numPixels) { pixelNum = 0; }

      // Expand to 24-bit (8/8/8)
      ftv_r8 = (ftv_hi & 0xF8) | (ftv_hi >> 5);
      ftv_g8 = (ftv_hi << 5) | ((ftv_lo & 0xE0) >> 3) | ((ftv_hi & 0x06) >> 1);
      ftv_b8 = (ftv_lo << 3) | ((ftv_lo & 0x1F) >> 2);

      // Apply gamma correction, further expand to 16/16/16
      ftv_nr = (uint8_t)pgm_read_byte(&ftv_gamma8[ftv_r8]) * 257; // New R/G/B
      ftv_ng = (uint8_t)pgm_read_byte(&ftv_gamma8[ftv_g8]) * 257;
      ftv_nb = (uint8_t)pgm_read_byte(&ftv_gamma8[ftv_b8]) * 257;

      ftv_totalTime = random(12, 125);                            // Semi-random pixel-to-pixel time
      ftv_fadeTime  = random(0, ftv_totalTime);                   // Pixel-to-pixel transition time

      if (random(10) < 3) { ftv_fadeTime = 0;                     // Force scene cut 30% of time
      }
      ftv_holdTime  = counter20ms + ftv_totalTime - ftv_fadeTime; // Non-transition time
      ftv_startTime = counter20ms;
    }

    ftv_elapsed = counter20ms - ftv_startTime;

    if (ftv_fadeTime) {
      ftv_r = map(ftv_elapsed, 0, ftv_fadeTime, ftv_pr, ftv_nr); // 16-bit interp
      ftv_g = map(ftv_elapsed, 0, ftv_fadeTime, ftv_pg, ftv_ng);
      ftv_b = map(ftv_elapsed, 0, ftv_fadeTime, ftv_pb, ftv_nb);
    } else {                                                     // Avoid divide-by-ftv_fraczero in map()
      ftv_r = ftv_nr;
      ftv_g = ftv_ng;
      ftv_b = ftv_nb;
    }

    for (ftv_i = 0; ftv_i < difference; ftv_i++) {
      ftv_r8   = ftv_r >> 8;                                          // Quantize to 8-bit
      ftv_g8   = ftv_g >> 8;
      ftv_b8   = ftv_b >> 8;
      ftv_frac = (ftv_i << 16) / difference;                          // LED index scaled to 0-65535 (16Bit)

      if ((ftv_r8 < 255) && ((ftv_r & 0xFF) >= ftv_frac)) { ftv_r8++; // Boost some fraction
      }

      if ((ftv_g8 < 255) && ((ftv_g & 0xFF) >= ftv_frac)) { ftv_g8++; // of LEDs to handle
      }

      if ((ftv_b8 < 255) && ((ftv_b & 0xFF) >= ftv_frac)) { ftv_b8++; // interp > 8bit
      }
      Plugin_128_pixels->SetPixelColor(ftv_i + startpixel, RgbColor(ftv_r8, ftv_g8, ftv_b8));
    }

    ftv_pr = ftv_nr; // Prev RGB = new RGB
    ftv_pg = ftv_ng;
    ftv_pb = ftv_nb;
  }
}

/*
 * Cycles a rainbow over the entire string of LEDs.
 */
void rainbow(void) {
  long  zaehler  = 20 * (counter20ms - starttimerb);
  float progress = (float)zaehler / (float)fadetime;

  if (fadeIn == true) {
    Plugin_128_pixels->SetBrightness(progress * 255);
    fadeIn = (progress == 1) ? false : true;
  }

  for (int i = 0; i < pixelCount; i++)
  {
    uint8_t r1 = (Wheel(((i * 256 / pixelCount) + counter20ms * rainbowspeed / 10) & 255) >> 16);
    uint8_t g1 = (Wheel(((i * 256 / pixelCount) + counter20ms * rainbowspeed / 10) & 255) >> 8);
    uint8_t b1 = (Wheel(((i * 256 / pixelCount) + counter20ms * rainbowspeed / 10) & 255));
    Plugin_128_pixels->SetPixelColor(i, RgbColor(r1, g1, b1));
  }
  mode = (rainbowspeed == 0) ? On : Rainbow;
}

/*
 * Put a value 0 to 255 in to get a color value.
 * The colours are a transition r -> g -> b -> back to r
 * Inspired by the Adafruit examples.
 */
uint32_t Wheel(uint8_t pos) {
  pos = 255 - pos;

  if (pos < 85) {
    return ((uint32_t)(255 - pos * 3) << 16) | ((uint32_t)(0) << 8) | (pos * 3);
  } else if (pos < 170) {
    pos -= 85;
    return ((uint32_t)(0) << 16) | ((uint32_t)(pos * 3) << 8) | (255 - pos * 3);
  } else {
    pos -= 170;
    return ((uint32_t)(pos * 3) << 16) | ((uint32_t)(255 - pos * 3) << 8) | (0);
  }
}

// Larson Scanner K.I.T.T.
void kitt(void) {
  if (counter20ms % (unsigned long)(SPEED_MAX / abs(speed)) == 0)
  {
    for (uint16_t i = 0; i < pixelCount; i++) {
      # if defined(RGBW) || defined(GRBW)
      RgbwColor px_rgb = Plugin_128_pixels->GetPixelColor(i);

      // fade out (divide by 2)
      px_rgb.R = px_rgb.R >> 1;
      px_rgb.G = px_rgb.G >> 1;
      px_rgb.B = px_rgb.B >> 1;
      px_rgb.W = px_rgb.W >> 1;

      # else // if defined(RGBW) || defined(GRBW)

      RgbColor px_rgb = Plugin_128_pixels->GetPixelColor(i);

      // fade out (divide by 2)
      px_rgb.R = px_rgb.R >> 1;
      px_rgb.G = px_rgb.G >> 1;
      px_rgb.B = px_rgb.B >> 1;
      # endif // if defined(RGBW) || defined(GRBW)

      Plugin_128_pixels->SetPixelColor(i, px_rgb);
    }

    uint16_t pos = 0;

    if (_counter_mode_step < pixelCount) {
      pos = _counter_mode_step;
    } else {
      pos = (pixelCount * 2) - _counter_mode_step - 2;
    }

    Plugin_128_pixels->SetPixelColor(pos, rgb);

    _counter_mode_step = (_counter_mode_step + 1) % ((pixelCount * 2) - 2);
  }
}

// Firing comets from one end.
void comet(void) {
  if (counter20ms % (unsigned long)(SPEED_MAX / abs(speed)) == 0)
  {
    for (uint16_t i = 0; i < pixelCount; i++) {
      if (speed > 0) {
        # if defined(RGBW) || defined(GRBW)
        RgbwColor px_rgb = Plugin_128_pixels->GetPixelColor(i);

        // fade out (divide by 2)
        px_rgb.R = px_rgb.R >> 1;
        px_rgb.G = px_rgb.G >> 1;
        px_rgb.B = px_rgb.B >> 1;
        px_rgb.W = px_rgb.W >> 1;

        # else // if defined(RGBW) || defined(GRBW)

        RgbColor px_rgb = Plugin_128_pixels->GetPixelColor(i);

        // fade out (divide by 2)
        px_rgb.R = px_rgb.R >> 1;
        px_rgb.G = px_rgb.G >> 1;
        px_rgb.B = px_rgb.B >> 1;
        # endif // if defined(RGBW) || defined(GRBW)

        Plugin_128_pixels->SetPixelColor(i, px_rgb);
      } else {
        # if defined(RGBW) || defined(GRBW)
        RgbwColor px_rgb = Plugin_128_pixels->GetPixelColor(pixelCount - i - 1);

        // fade out (divide by 2)
        px_rgb.R = px_rgb.R >> 1;
        px_rgb.G = px_rgb.G >> 1;
        px_rgb.B = px_rgb.B >> 1;
        px_rgb.W = px_rgb.W >> 1;

        # else // if defined(RGBW) || defined(GRBW)

        RgbColor px_rgb = Plugin_128_pixels->GetPixelColor(pixelCount - i - 1);

        // fade out (divide by 2)
        px_rgb.R = px_rgb.R >> 1;
        px_rgb.G = px_rgb.G >> 1;
        px_rgb.B = px_rgb.B >> 1;
        # endif // if defined(RGBW) || defined(GRBW)

        Plugin_128_pixels->SetPixelColor(pixelCount - i - 1, px_rgb);
      }
    }

    if (speed > 0) {
      Plugin_128_pixels->SetPixelColor(_counter_mode_step, rgb);
    } else {
      Plugin_128_pixels->SetPixelColor(pixelCount - _counter_mode_step - 1, rgb);
    }

    _counter_mode_step = (_counter_mode_step + 1) % pixelCount;
  }
}

// Theatre lights
void theatre(void) {
  if ((counter20ms % (unsigned long)(SPEED_MAX / abs(speed)) == 0) && (speed != 0))
  {
    if (speed > 0) {
      Plugin_128_pixels->RotateLeft(1, 0, (pixelCount / count) * count - 1);
    } else {
      Plugin_128_pixels->RotateRight(1, 0, (pixelCount / count) * count - 1);
    }
  }
}

/*
 * Runs a single pixel back and forth.
 */
void scan(void) {
  if ((counter20ms % (unsigned long)(SPEED_MAX / abs(speed)) == 0) && (speed != 0))
  {
    if (_counter_mode_step > uint16_t((pixelCount * 2) - 2)) {
      _counter_mode_step = 0;
    }
    _counter_mode_step++;

    int i = _counter_mode_step - (pixelCount - 1);
    i = abs(i);

    // Plugin_128_pixels->ClearTo(rrggbb);
    for (int i = 0; i < pixelCount; i++) { Plugin_128_pixels->SetPixelColor(i, rrggbb); }
    Plugin_128_pixels->SetPixelColor(abs(i), rgb);
  }
}

/*
 * Runs two pixel back and forth in opposite directions.
 */
void dualscan(void) {
  if ((counter20ms % (unsigned long)(SPEED_MAX / abs(speed)) == 0) && (speed != 0)) {
    if (_counter_mode_step > uint16_t((pixelCount * 2) - 2)) {
      _counter_mode_step = 0;
    }

    _counter_mode_step++;

    int i = _counter_mode_step - (pixelCount - 1);
    i = abs(i);

    // Plugin_128_pixels->ClearTo(rrggbb);
    for (int i = 0; i < pixelCount; i++) { Plugin_128_pixels->SetPixelColor(i, rrggbb); }
    Plugin_128_pixels->SetPixelColor(abs(i),               rgb);
    Plugin_128_pixels->SetPixelColor(pixelCount - (i + 1), rgb);
  }
}

/*
 * Blink several LEDs on, reset, repeat.
 * Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
 */
void twinkle(void) {
  if ((counter20ms % (unsigned long)(SPEED_MAX / abs(speed)) == 0) && (speed != 0))
  {
    if (_counter_mode_step == 0) {
      // Plugin_128_pixels->ClearTo(rrggbb);
      for (int i = 0; i < pixelCount; i++) { Plugin_128_pixels->SetPixelColor(i, rrggbb); }
      uint16_t min_leds = _max(1, pixelCount / 5); // make sure, at least one LED is on
      uint16_t max_leds = _max(1, pixelCount / 2); // make sure, at least one LED is on
      _counter_mode_step = random(min_leds, max_leds);
    }

    Plugin_128_pixels->SetPixelColor(random(pixelCount), rgb);

    _counter_mode_step--;
  }
}

/*
 * Blink several LEDs on, fading out.
 */
void twinklefade(void) {
  if ((counter20ms % (unsigned long)(SPEED_MAX / abs(speed)) == 0) && (speed != 0))
  {
    for (uint16_t i = 0; i < pixelCount; i++) {
      # if defined(RGBW) || defined(GRBW)
      RgbwColor px_rgb = Plugin_128_pixels->GetPixelColor(pixelCount - i - 1);

      // fade out (divide by 2)
      px_rgb.R = px_rgb.R >> 1;
      px_rgb.G = px_rgb.G >> 1;
      px_rgb.B = px_rgb.B >> 1;
      px_rgb.W = px_rgb.W >> 1;

      # else // if defined(RGBW) || defined(GRBW)

      RgbColor px_rgb = Plugin_128_pixels->GetPixelColor(pixelCount - i - 1);

      // fade out (divide by 2)
      px_rgb.R = px_rgb.R >> 1;
      px_rgb.G = px_rgb.G >> 1;
      px_rgb.B = px_rgb.B >> 1;
      # endif // if defined(RGBW) || defined(GRBW)

      Plugin_128_pixels->SetPixelColor(i, px_rgb);
    }

    if (random(count) < 50) {
      Plugin_128_pixels->SetPixelColor(random(pixelCount), rgb);
    }
  }
}

/*
 * Blinks one LED at a time.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
void sparkle(void) {
  if ((counter20ms % (unsigned long)(SPEED_MAX / abs(speed)) == 0) && (speed != 0))
  {
    // Plugin_128_pixels->ClearTo(rrggbb);
    for (int i = 0; i < pixelCount; i++) { Plugin_128_pixels->SetPixelColor(i, rrggbb); }
    Plugin_128_pixels->SetPixelColor(random(pixelCount), rgb);
  }
}

// Fire
unsigned long fireTimer;
RgbColor leds[ARRAYSIZE];

void fire(void) {
  if (counter20ms > fireTimer + 50 / fps) {
    fireTimer = counter20ms;
    Fire2012();
    RgbColor pixel;

    for (int i = 0; i < pixelCount; i++) {
      pixel = leds[i];
      pixel = RgbColor::LinearBlend(pixel, RgbColor(0, 0, 0), (255 - brightness) / 255.0);
      Plugin_128_pixels->SetPixelColor(i, pixel);
    }
  }
}

/// random number seed
uint16_t rand16seed; // = RAND16_SEED;

/// Generate an 8-bit random number
uint8_t random8() {
  rand16seed = (rand16seed * ((uint16_t)(2053))) + ((uint16_t)(13849));

  // return the sum of the high and low bytes, for better
  //  mixing and non-sequential correlation
  return (uint8_t)(((uint8_t)(rand16seed & 0xFF)) +
                   ((uint8_t)(rand16seed >> 8)));
}

/// Generate an 8-bit random number between 0 and lim
/// @param lim the upper bound for the result
uint8_t random8(uint8_t lim) {
  uint8_t r = random8();

  r = (r * lim) >> 8;
  return r;
}

/// Generate an 8-bit random number in the given range
/// @param min the lower bound for the random number
/// @param lim the upper bound for the random number
uint8_t random8(uint8_t min, uint8_t lim) {
  uint8_t delta = lim - min;
  uint8_t r     = random8(delta) + min;

  return r;
}

/// subtract one byte from another, saturating at 0x00
/// @returns i - j with a floor of 0
uint8_t qsub8(uint8_t i, uint8_t j) {
  int t = i - j;

  if (t < 0) { t = 0; }
  return t;
}

/// add one byte to another, saturating at 0xFF
/// @param i - first byte to add
/// @param j - second byte to add
/// @returns the sum of i & j, capped at 0xFF
uint8_t qadd8(uint8_t i, uint8_t j) {
  unsigned int t = i + j;

  if (t > 255) { t = 255; }
  return t;
}

///  The "video" version of scale8 guarantees that the output will
///  be only be zero if one or both of the inputs are zero.  If both
///  inputs are non-zero, the output is guaranteed to be non-zero.
///  This makes for better 'video'/LED dimming, at the cost of
///  several additional cycles.
uint8_t scale8_video(uint8_t i, uint8_t scale) {
  uint8_t j = (((int)i * (int)scale) >> 8) + ((i && scale) ? 1 : 0);

  // uint8_t nonzeroscale = (scale != 0) ? 1 : 0;
  // uint8_t j = (i == 0) ? 0 : (((int)i * (int)(scale) ) >> 8) + nonzeroscale;
  return j;
}

void Fire2012(void) {
  // Array of temperature readings at each simulation cell
  static byte heat[ARRAYSIZE];

  // Step 1.  Cool down every cell a little
  for (int i = 0; i < pixelCount; i++) {
    heat[i] = qsub8(heat[i],  random8(0, ((cooling * 10) / pixelCount) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for (int k = pixelCount - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if (random8() < sparking) {
    int y = random8(7);
    heat[y] = qadd8(heat[y], random8(160, 255));
  }

  // Step 4.  Map from heat cells to LED colors
  for (int j = 0; j < pixelCount; j++) {
    RgbColor heatcolor;

    // Scale 'heat' down from 0-255 to 0-191,
    // which can then be easily divided into three
    // equal 'thirds' of 64 units each.
    uint8_t t192 = scale8_video(heat[j], 191);

    // calculate a value that ramps up from
    // zero to 255 in each 'third' of the scale.
    uint8_t heatramp = t192 & 0x3F; // 0..63
    heatramp <<= 2;                 // scale up to 0..252

    // now figure out which third of the spectrum we're in:
    if (t192 & 0x80) {
      // we're in the hottest third
      heatcolor.R = 255;      // full red
      heatcolor.G = 255;      // full green
      heatcolor.B = heatramp; // ramp up blue
    } else if (t192 & 0x40) {
      // we're in the middle third
      heatcolor.R = 255;      // full red
      heatcolor.G = heatramp; // ramp up green
      heatcolor.B = 0;        // no blue
    } else {
      // we're in the coolest third
      heatcolor.R = heatramp; // ramp up red
      heatcolor.G = 0;        // no green
      heatcolor.B = 0;        // no blue
    }

    int pixelnumber;

    if (gReverseDirection) {
      pixelnumber = (pixelCount - 1) - j;
    } else {
      pixelnumber = j;
    }
    leds[pixelnumber] = heatcolor;
  }
}

/*
 * Fire flicker function
 */
void fire_flicker() {
  if ((counter20ms % (unsigned long)(SPEED_MAX / abs(speed)) == 0) && (speed != 0))
  {
    byte w   = 0;   // (SEGMENT.colors[0] >> 24) & 0xFF;
    byte r   = 255; // (SEGMENT.colors[0] >> 16) & 0xFF;
    byte g   = 96;  // (SEGMENT.colors[0] >>  8) & 0xFF;
    byte b   = 12;  // (SEGMENT.colors[0]        & 0xFF);
    byte lum = max(w, max(r, max(g, b))) / rev_intensity;

    for (uint16_t i = 0; i <= numPixels - 1; i++) {
      int flicker = random8(lum);

      # if defined(RGBW) || defined(GRBW)
      Plugin_128_pixels->SetPixelColor(i, RgbwColor(max(r - flicker, 0), max(g - flicker, 0), max(b - flicker, 0), max(w - flicker, 0)));
      # else // if defined(RGBW) || defined(GRBW)
      Plugin_128_pixels->SetPixelColor(i, RgbColor(max(r - flicker, 0), max(g - flicker, 0), max(b - flicker, 0)));
      # endif  // if defined(RGBW) || defined(GRBW)
    }
  }
}

void Plugin_128_simpleclock() {
  byte Hours      = node_time.hour() % 12;
  byte Minutes    = node_time.minute();
  byte Seconds    = node_time.second();
  byte big_tick   = 15;
  byte small_tick = 5;

  // hack for sub-second calculations.... reset when first time new second begins..
  if (cooling != Seconds) { maxtime = counter20ms; }
  cooling = Seconds;
  Plugin_128_pixels->ClearTo(rrggbb);

  for (int i = 0; i < (60 / small_tick); i++) {
    if (i % (big_tick / small_tick) == 0) { Plugin_128_pixels->SetPixelColor((i * pixelCount * small_tick / 60) % pixelCount, rgb_tick_b); }
    else { Plugin_128_pixels->SetPixelColor((i * pixelCount * small_tick / 60) % pixelCount, rgb_tick_s); }
  }


  for (int i = 0; i < pixelCount; i++) {
    if (round((((float)Seconds + ((float)counter20ms - (float)maxtime) / 50.0) * (float)pixelCount) / 60.0) == i) {
      if (rgb_s_off  == false) {
        Plugin_128_pixels->SetPixelColor(i, rgb_s);
      }
    }
    else if (round((((float)Minutes * 60.0) + (float)Seconds) / 60.0 * (float)pixelCount / 60.0) == i) {
      Plugin_128_pixels->SetPixelColor(i, rgb_m);
    }
    else if (round(((float)Hours + (float)Minutes / 60) * (float)pixelCount / 12.0)  == i) {
      Plugin_128_pixels->SetPixelColor(i,                                 rgb_h);
      Plugin_128_pixels->SetPixelColor((i + 1) % pixelCount,              rgb_h);
      Plugin_128_pixels->SetPixelColor((i - 1 + pixelCount) % pixelCount, rgb_h);
    }
  }
}

uint32_t rgbStr2Num(String rgbStr) {
  uint32_t rgbDec = (int)strtoul(&rgbStr[0], NULL, 16);

  return rgbDec;
}

void hex2rgb(String hexcolor) {
  colorStr = hexcolor;
  # if defined(RGBW) || defined(GRBW)
  hexcolor.length() <= 6
    ? rgb = RgbColor(rgbStr2Num(hexcolor) >> 16, rgbStr2Num(hexcolor) >> 8, rgbStr2Num(hexcolor))
    : rgb = RgbwColor(rgbStr2Num(hexcolor) >> 24, rgbStr2Num(hexcolor) >> 16, rgbStr2Num(hexcolor) >> 8, rgbStr2Num(hexcolor));
  # else // if defined(RGBW) || defined(GRBW)
  rgb = RgbColor(rgbStr2Num(hexcolor) >> 16, rgbStr2Num(hexcolor) >> 8, rgbStr2Num(hexcolor));
  # endif // if defined(RGBW) || defined(GRBW)
}

void hex2rrggbb(String hexcolor) {
  backgroundcolorStr = hexcolor;
  # if defined(RGBW) || defined(GRBW)
  hexcolor.length() <= 6
    ? rrggbb = RgbColor(rgbStr2Num(hexcolor) >> 16, rgbStr2Num(hexcolor) >> 8, rgbStr2Num(hexcolor))
    : rrggbb = RgbwColor(rgbStr2Num(hexcolor) >> 24, rgbStr2Num(hexcolor) >> 16, rgbStr2Num(hexcolor) >> 8, rgbStr2Num(hexcolor));
  # else // if defined(RGBW) || defined(GRBW)
  rrggbb = RgbColor(rgbStr2Num(hexcolor) >> 16, rgbStr2Num(hexcolor) >> 8, rgbStr2Num(hexcolor));
  # endif // if defined(RGBW) || defined(GRBW)
}

void hex2rgb_pixel(String hexcolor) {
  colorStr = hexcolor;

  for (int i = 0; i < pixelCount; i++) {
    # if defined(RGBW) || defined(GRBW)
    hexcolor.length() <= 6
      ? rgb_target[i] = RgbColor(rgbStr2Num(hexcolor) >> 16, rgbStr2Num(hexcolor) >> 8, rgbStr2Num(hexcolor))
      : rgb_target[i] = RgbwColor(rgbStr2Num(hexcolor) >> 24, rgbStr2Num(hexcolor) >> 16, rgbStr2Num(hexcolor) >> 8, rgbStr2Num(hexcolor));
    # else // if defined(RGBW) || defined(GRBW)
    rgb_target[i] = RgbColor(rgbStr2Num(hexcolor) >> 16, rgbStr2Num(hexcolor) >> 8, rgbStr2Num(hexcolor));
    # endif // if defined(RGBW) || defined(GRBW)
  }
}

// ---------------------------------------------------------------------------------
// ------------------------------ JsonResponse -------------------------------------
// ---------------------------------------------------------------------------------
void NeoPixelSendStatus(struct EventStruct *eventSource) {
  String log = String(F("NeoPixelBusFX: Set ")) + rgb.R
               + String(F("/")) + rgb.G + String(F("/")) + rgb.B;

  addLog(LOG_LEVEL_INFO, log);

  String json;

  printToWebJSON = true;
  json          += F("{\n");
  json          += F("\"plugin\": \"128");
  json          += F("\",\n\"mode\": \"");
  json          += modeName[mode];
  json          += F("\",\n\"lastmode\": \"");
  json          += modeName[savemode];
  json          += F("\",\n\"fadetime\": \"");
  json          += fadetime;
  json          += F("\",\n\"fadedelay\": \"");
  json          += fadedelay;
  json          += F("\",\n\"dim\": \"");
  json          += Plugin_128_pixels->GetBrightness();
  json          += F("\",\n\"rgb\": \"");
  json          += colorStr;
  json          += F("\",\n\"hue\": \"");
  json          += toString((HsbColor(RgbColor(rgb.R, rgb.G, rgb.B)).H * 360), 0);
  json          += F("\",\n\"saturation\": \"");
  json          += toString((HsbColor(RgbColor(rgb.R, rgb.G, rgb.B)).S * 100), 0);
  json          += F("\",\n\"brightness\": \"");
  json          += toString((HsbColor(RgbColor(rgb.R, rgb.G, rgb.B)).B * 100), 0);
  json          += F("\",\n\"bgcolor\": \"");
  json          += backgroundcolorStr;
  json          += F("\",\n\"count\": \"");
  json          += count;
  json          += F("\",\n\"speed\": \"");
  json          += speed;
  json          += F("\",\n\"pixelcount\": \"");
  json          += pixelCount;
  json          += F("\"\n}\n");
  SendStatus(eventSource, json); // send http response to controller (JSON format)
  printToWeb = false;
}

#endif // USES_P128
