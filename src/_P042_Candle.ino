#include "_Plugin_Helper.h"
#ifdef USES_P042

// #######################################################################################################
// ######################################## Plugin 042: NeoPixel Candle ##################################
// #######################################################################################################

// PROJECT INFO
// Wifi Candle for ESPEasy by Dominik Schmidt (10.2016)

/** Changelog:
 * 2023-01-21 tonhuisman: Move to PluginStruct_base to enable multi-instance use of this plugin
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

# include "src/PluginStructs/P042_data_struct.h"

# define PLUGIN_042
# define PLUGIN_ID_042         42
# define PLUGIN_NAME_042       "Output - NeoPixel (Candle)"
# define PLUGIN_VALUENAME1_042 "Color"
# define PLUGIN_VALUENAME2_042 "Brightness"
# define PLUGIN_VALUENAME3_042 "Type"

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
      P042_ColorType Candle_color = static_cast<P042_ColorType>(P042_CONFIG_COLORTYPE);
      addRowLabel(F("Color Handling")); // checked
      addHtml(F("<input type='radio' id='clrDef' name='" P042_WEBVAR_COLORTYPE_S "' value='0'"));

      if (Candle_color == P042_ColorType::ColorDefault) {
        addHtml(F(" checked>"));
      } else {
        addHtml('>');
      }
      addHtml(F("<label for='clrDef'> Use default color</label><br>"));
      addHtml(F("<input type='radio' id='clrSel' name='" P042_WEBVAR_COLORTYPE_S "' value='1'"));

      if (Candle_color == P042_ColorType::ColorSelected) {
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
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P042_data_struct());
      P042_data_struct *P042_data = static_cast<P042_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P042_data) {
        success = P042_data->plugin_init(event);
      }

      break;
    }

    case PLUGIN_EXIT:
    {
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      P042_data_struct *P042_data = static_cast<P042_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P042_data) {
        success = P042_data->plugin_once_a_second(event);
      }

      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      P042_data_struct *P042_data = static_cast<P042_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P042_data) {
        success = P042_data->plugin_fifty_per_second(event);
      }

      break;
    }

    case PLUGIN_READ:
    {
      P042_data_struct *P042_data = static_cast<P042_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P042_data) {
        success = P042_data->plugin_read(event);
      }

      break;
    }

    case PLUGIN_WRITE:
    {
      P042_data_struct *P042_data = static_cast<P042_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P042_data) {
        success = P042_data->plugin_write(event, string);
      }

      break;
    }
  }
  return success;
}

#endif // USES_P042
