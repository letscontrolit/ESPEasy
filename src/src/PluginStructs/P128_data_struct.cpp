#include "../../_Plugin_Helper.h"
#ifdef USES_P128

# include "../PluginStructs/P128_data_struct.h"

// ***************************************************************/
// Constructor
// ***************************************************************/
P128_data_struct::P128_data_struct(int8_t   _gpioPin,
                                   uint16_t _pixelCount,
                                   uint8_t  _maxBright)
    : gpioPin(_gpioPin), pixelCount(_pixelCount), maxBright(_maxBright) 
{
  # ifdef ESP8266
  Plugin_128_pixels = new (std::nothrow) NEOPIXEL_LIB<FEATURE, METHOD>(min(pixelCount, static_cast<uint16_t>(ARRAYSIZE)));
  # endif // ifdef ESP8266
  # ifdef ESP32
  Plugin_128_pixels = new (std::nothrow) NEOPIXEL_LIB<FEATURE, METHOD>(min(pixelCount, static_cast<uint16_t>(ARRAYSIZE)),
                                                                        _gpioPin);
  # endif // ifdef ESP32

  if (nullptr != Plugin_128_pixels) {
    Plugin_128_pixels->Begin(); // This initializes the NeoPixelBus library.
    Plugin_128_pixels->SetBrightness(maxBright);
  }
}

// ***************************************************************/
// Destructor
// ***************************************************************/
P128_data_struct::~P128_data_struct() {
  delete Plugin_128_pixels;
  Plugin_128_pixels = nullptr;
}

bool P128_data_struct::plugin_read(struct EventStruct *event) {
  // there is no need to read them, just use current values
  UserVar[event->BaseVarIndex]     = static_cast<int>(mode);
  UserVar[event->BaseVarIndex + 1] = static_cast<int>(savemode);
  UserVar[event->BaseVarIndex + 2] = fadetime;
  UserVar[event->BaseVarIndex + 3] = fadedelay;

  # ifndef LIMIT_BUILD_SIZE

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    log.reserve(64);
    log  = F("Lights: mode: ");
    log += P128_modeType_toString(mode);
    log += F(" lastmode: ");
    log += P128_modeType_toString(savemode);
    log += F(" fadetime: ");
    log += (int)UserVar[event->BaseVarIndex + 2];
    log += F(" fadedelay: ");
    log += (int)UserVar[event->BaseVarIndex + 3];
    addLogMove(LOG_LEVEL_INFO, log);
  }
  # endif // ifndef LIMIT_BUILD_SIZE
  return true;
}

bool P128_data_struct::plugin_write(struct EventStruct *event,
                                    const String      & string) {
  bool success = false;

  const String command = parseString(string, 1);

  if ((equals(command, F("neopixelfx"))) || (equals(command, F("nfx")))) {
    const String subCommand = parseString(string, 2);

    const String  str3  = parseString(string, 3);
    const int32_t str3i = event->Par2;
    const String  str4  = parseString(string, 4);
    const int32_t str4i = event->Par3;
    const String  str5  = parseString(string, 5);
    const int32_t str5i = event->Par4;
    const String  str6  = parseString(string, 6);
    const int32_t str6i = event->Par5;
    const String  str7  = parseString(string, 7);
    const int32_t str7i = str7.toInt();

    if (equals(subCommand, F("fadetime"))) {
      success  = true;
      fadetime = str3i;
    }

    else if (equals(subCommand, F("fadedelay"))) {
      success   = true;
      fadedelay = str3i;
    }

    else if (equals(subCommand, F("speed"))) {
      success      = true;
      defaultspeed = str3i;
      speed        = defaultspeed;
    }

    else if (equals(subCommand, F("bgcolor"))) {
      success = true;
      hex2rrggbb(str3);
    }

    else if (equals(subCommand, F("count"))) {
      success = true;
      count   = str3i;
    }

    else if ((equals(subCommand, F("on"))) || (equals(subCommand, F("off")))) {
      success   = true;
      fadetime  = 1000;
      fadedelay = 0;

      fadetime = str3.isEmpty()
          ? fadetime
          : str3i;
      fadedelay = str4.isEmpty()
          ? fadedelay
          : str4i;

      for (int pixel = 0; pixel < pixelCount; pixel++) {
        r_pixel = (fadedelay < 0)
            ? pixelCount - pixel - 1
            : pixel;

        starttime[r_pixel] = counter20ms + (pixel * abs(fadedelay) / 20);

        if ((equals(subCommand, F("on"))) && (mode == P128_modetype::Off)) { // switch on
          rgb_target[pixel] = rgb_old[pixel];
          rgb_old[pixel]    = Plugin_128_pixels->GetPixelColor(pixel);
        } else if (equals(subCommand, F("off"))) {                           // switch off
          rgb_old[pixel]    = Plugin_128_pixels->GetPixelColor(pixel);
          rgb_target[pixel] = RgbColor(0);
        }
      }

      if ((equals(subCommand, F("on"))) && (mode == P128_modetype::Off)) { // switch on
        mode = (savemode == P128_modetype::On) ? P128_modetype::Fade : savemode;
      } else if (equals(subCommand, F("off"))) {                           // switch off
        savemode = mode;
        mode     = P128_modetype::Fade;
      }

      maxtime = starttime[r_pixel] + (fadetime / 20);
    }

    else if (equals(subCommand, F("dim"))) {
      if ((str3i >= 0) && (str3i <= maxBright)) { // Safety check
        success = true;
        Plugin_128_pixels->SetBrightness(str3i);
      }
    }

    else if (equals(subCommand, F("line"))) {
      success = true;
      mode    = P128_modetype::On;

      hex2rgb(str5);

      for (int i = 0; i <= (str4i - str3i + pixelCount) % pixelCount; i++) {
        Plugin_128_pixels->SetPixelColor((i + str3i - 1) % pixelCount, rgb);
      }
    }

    else if (equals(subCommand, F("tick"))) {
      success = true;
      mode    = P128_modetype::On;

      hex2rgb(str4);

      //          for (int i = 0; i < pixelCount ; i = i + (pixelCount / parseString(string, 3).toInt())) {
      for (int i = 0; i < str3i; i++) {
        Plugin_128_pixels->SetPixelColor(i * pixelCount / str3i, rgb);
      }
    }

    else if (equals(subCommand, F("one"))) {
      success = true;
      mode    = P128_modetype::On;

      const uint16_t pixnum = str3i - 1;
      hex2rgb(str4);

      Plugin_128_pixels->SetPixelColor(pixnum, rgb);
    }

    else if ((equals(subCommand, F("fade"))) || (equals(subCommand, F("all"))) || (equals(subCommand, F("rgb")))) {
      success = true;
      mode    = P128_modetype::Fade;

      if ((equals(subCommand, F("all"))) || (equals(subCommand, F("rgb")))) {
        fadedelay = 0;
      }

      hex2rgb(str3);
      hex2rgb_pixel(str3);

      fadetime = str4.isEmpty()
          ? fadetime
          : str4i;
      fadedelay = str5.isEmpty()
          ? fadedelay
          : str5i;

      for (int pixel = 0; pixel < pixelCount; pixel++) {
        r_pixel = (fadedelay < 0)
            ? pixelCount - pixel - 1
            : pixel;

        starttime[r_pixel] = counter20ms + (pixel * abs(fadedelay) / 20);

        rgb_old[pixel] = Plugin_128_pixels->GetPixelColor(pixel);
      }
      maxtime = starttime[r_pixel] + (fadetime / 20);
    }

    else if (equals(subCommand, F("hsv"))) {
      success   = true;
      mode      = P128_modetype::Fade;
      fadedelay = 0;
      rgb       =
        RgbColor(HsbColor(str3.toFloat() / 360.0f, str4.toFloat() / 100.0f,
                          str5.toFloat() / 100.0f));

      rgb2colorStr();

      hex2rgb_pixel(colorStr);

      fadetime = str6.isEmpty()
          ? fadetime
          : str6i;
      fadedelay = str7.isEmpty()
          ? fadedelay
          : str7i;

      for (int pixel = 0; pixel < pixelCount; pixel++) {
        r_pixel = (fadedelay < 0)
            ? pixelCount - pixel - 1
            : pixel;

        starttime[r_pixel] = counter20ms + (pixel * abs(fadedelay) / 20);

        rgb_old[pixel] = Plugin_128_pixels->GetPixelColor(pixel);
      }
      maxtime = starttime[r_pixel] + (fadetime / 20);
    }

    else if (equals(subCommand, F("hsvone"))) {
      success = true;
      mode    = P128_modetype::On;
      rgb     =
        RgbColor(HsbColor(str4.toFloat() / 360.0f, str5.toFloat() / 100.0f,
                          str6.toFloat() / 100.0f));

      rgb2colorStr();

      hex2rgb(colorStr);
      const uint16_t pixnum = str3i - 1;
      Plugin_128_pixels->SetPixelColor(pixnum, rgb);
    }

    else if (equals(subCommand, F("hsvline"))) {
      success = true;
      mode    = P128_modetype::On;

      rgb =
        RgbColor(HsbColor(str5.toFloat() / 360.0f, str6.toFloat() / 100.0f,
                          str7.toFloat() / 100.0f));

      rgb2colorStr();

      hex2rgb(colorStr);

      for (int i = 0; i <= (str4i - str3i + pixelCount) % pixelCount; i++) {
        Plugin_128_pixels->SetPixelColor((i + str3i - 1) % pixelCount, rgb);
      }
    }

    else if (equals(subCommand, F("rainbow"))) {
      success     = true;
      fadeIn      = (mode == P128_modetype::Off) ? true : false;
      mode        = P128_modetype::Rainbow;
      starttimerb = counter20ms;

      rainbowspeed = str3.isEmpty()
          ? speed
          : str3i;

      fadetime = str4.isEmpty()
          ? fadetime
          : str4i;
    }

    else if (equals(subCommand, F("colorfade"))) {
      success = true;
      mode    = P128_modetype::ColorFade;

      hex2rgb(str3);

      if (!str4.isEmpty()) { hex2rrggbb(str4); }

      startpixel = str5.isEmpty()
          ? 0
          : str5i - 1;
      endpixel = str6.isEmpty()
          ? pixelCount - 1
          : str6i - 1;
    }

    else if (equals(subCommand, F("kitt"))) {
      success = true;
      mode    = P128_modetype::Kitt;

      _counter_mode_step = 0;

      hex2rgb(str3);

      speed = str4.isEmpty()
          ? defaultspeed
          : str4i;
    }

    else if (equals(subCommand, F("comet"))) {
      success = true;
      mode    = P128_modetype::Comet;

      _counter_mode_step = 0;

      hex2rgb(str3);

      speed = str4.isEmpty()
          ? defaultspeed
          : str4i;
    }

    else if (equals(subCommand, F("theatre"))) {
      success = true;
      mode    = P128_modetype::Theatre;

      hex2rgb(str3);

      if (!str4.isEmpty()) { hex2rrggbb(str4); }

      count = str5.isEmpty()
          ? count
          : str5i;

      speed = str6.isEmpty()
          ? defaultspeed
          : str6i;

      for (int i = 0; i < pixelCount; i++) {
        if ((i / count) % 2 == 0) {
          Plugin_128_pixels->SetPixelColor(i, rgb);
        } else {
          Plugin_128_pixels->SetPixelColor(i, rrggbb);
        }
      }
    }

    else if (equals(subCommand, F("scan"))) {
      success = true;
      mode    = P128_modetype::Scan;

      _counter_mode_step = 0;

      hex2rgb(str3);

      if (!str4.isEmpty()) { hex2rrggbb(str4); }

      speed = str5.isEmpty()
          ? defaultspeed
          : str5i;
    }

    else if (equals(subCommand, F("dualscan"))) {
      success = true;
      mode    = P128_modetype::Dualscan;

      _counter_mode_step = 0;

      hex2rgb(str3);

      if (!str4.isEmpty()) { hex2rrggbb(str4); }

      speed = str5.isEmpty()
          ? defaultspeed
          : str5i;
    }

    else if (equals(subCommand, F("twinkle"))) {
      success = true;
      mode    = P128_modetype::Twinkle;

      _counter_mode_step = 0;

      hex2rgb(str3);

      if (!str4.isEmpty()) { hex2rrggbb(str4); }

      speed = str5.isEmpty()
          ? defaultspeed
          : str5i;
    }

    else if (equals(subCommand, F("twinklefade"))) {
      success = true;
      mode    = P128_modetype::TwinkleFade;

      hex2rgb(str3);

      count = str4.isEmpty()
          ? count
          : str4i;

      speed = str5.isEmpty()
          ? defaultspeed
          : str5i;
    }

    else if (equals(subCommand, F("sparkle"))) {
      success = true;
      mode    = P128_modetype::Sparkle;

      _counter_mode_step = 0;

      hex2rgb(str3);
      hex2rrggbb(str4);

      speed = str5.isEmpty()
          ? defaultspeed
          : str5i;
    }

    else if (equals(subCommand, F("wipe"))) {
      success = true;
      mode    = P128_modetype::Wipe;

      _counter_mode_step = 0;

      hex2rgb(str3);

      if (!str4.isEmpty()) {
        hex2rrggbb(str4);
      } else {
        hex2rrggbb("000000");
      }

      speed = str5.isEmpty()
          ? defaultspeed
          : str5i;
    }

    else if (equals(subCommand, F("dualwipe"))) {
      success = true;
      mode    = P128_modetype::Dualwipe;

      _counter_mode_step = 0;

      hex2rgb(str3);

      if (!str4.isEmpty()) {
        hex2rrggbb(str4);
      } else {
        hex2rrggbb("000000");
      }

      speed = str5.isEmpty()
          ? defaultspeed
          : str5i;
    }

    # if P128_ENABLE_FAKETV
    else if (equals(subCommand, F("faketv"))) {
      success            = true;
      mode               = P128_modetype::FakeTV;
      _counter_mode_step = 0;

      randomSeed(analogRead(A0));
      pixelNum = random(NUMPixels); // Begin at random point

      startpixel = str3.isEmpty()
          ? 0
          : str3i - 1;
      endpixel = str4.isEmpty()
          ? pixelCount
          : str4i;
    }
    # endif // if P128_ENABLE_FAKETV

    else if (equals(subCommand, F("fire"))) {
      success = true;
      mode    = P128_modetype::Fire;

      fps = str3.isEmpty()
          ? fps
          : str3i;

      fps = (fps == 0 || fps > 50) ? 50 : fps;

      brightness = str4.isEmpty()
          ? brightness
          : str4.toFloat();
      cooling = str5.isEmpty()
          ? cooling
          : str5.toFloat();
      sparking = str6.isEmpty()
          ? sparking
          : str6.toFloat();
    }

    else if (equals(subCommand, F("fireflicker"))) {
      success = true;
      mode    = P128_modetype::FireFlicker;

      rev_intensity = str3.isEmpty()
          ? rev_intensity
          : str3i;

      speed = str4.isEmpty()
          ? defaultspeed
          : str4i;
    }

    else if (equals(subCommand, F("simpleclock"))) {
      success = true;
      mode    = P128_modetype::SimpleClock;

      # if defined(RGBW) || defined(GRBW)

      if (!str3.isEmpty()) {
        const uint32_t hcolorui = rgbStr2Num(str3);

        if (str3.length() <= 6) {
          rgb_tick_s = RgbwColor(hcolorui >> 16, hcolorui >> 8,
                                 hcolorui);
        } else {
          rgb_tick_s = RgbwColor(hcolorui >> 24,
                                 hcolorui >> 16,
                                 hcolorui >> 8,
                                 hcolorui);
        }
      }

      if (!str4.isEmpty()) {
        const uint32_t hcolorui = rgbStr2Num(str4);

        if (str4.length() <= 6) {
          rgb_tick_b = RgbwColor(hcolorui >> 16, hcolorui >> 8,
                                 hcolorui);
        } else {
          rgb_tick_b = RgbwColor(hcolorui >> 24,
                                 hcolorui >> 16,
                                 hcolorui >> 8,
                                 hcolorui);
        }
      }

      if (!str5.isEmpty()) {
        const uint32_t hcolorui = rgbStr2Num(str5);

        if (str5.length() <= 6) {
          rgb_h = RgbwColor(hcolorui >> 16, hcolorui >> 8,
                            hcolorui);
        } else {
          rgb_h = RgbwColor(hcolorui >> 24,
                            hcolorui >> 16,
                            hcolorui >> 8,
                            hcolorui);
        }
      }

      if (!str6.isEmpty()) {
        const uint32_t hcolorui = rgbStr2Num(str6);

        if (str6.length() <= 6) {
          rgb_m = RgbwColor(hcolorui >> 16, hcolorui >> 8,
                            hcolorui);
        } else {
          rgb_m = RgbwColor(hcolorui >> 24,
                            hcolorui >> 16,
                            hcolorui >> 8,
                            hcolorui);
        }
      }

      if (!str7.isEmpty()) {
        if (equals(str7, F("off"))) {
          rgb_s_off = true;
        } else if (str7.length() <= 6) {
          const uint32_t hcolorui = rgbStr2Num(str7);
          rgb_s_off = false;
          rgb_s     = RgbwColor(hcolorui >> 16, hcolorui >> 8,
                                hcolorui);
        } else {
          const uint32_t hcolorui = rgbStr2Num(str7);
          rgb_s_off = false;
          rgb_s     = RgbwColor(hcolorui >> 24,
                                hcolorui >> 16,
                                hcolorui >> 8,
                                hcolorui);
        }
      }

      # else // if defined(RGBW) || defined(GRBW)

      if (!str3.isEmpty()) {
        const uint32_t hcolorui = rgbStr2Num(str3);
        rgb_tick_s = RgbColor(hcolorui >> 16, hcolorui >> 8, hcolorui);
      }

      if (!str4.isEmpty()) {
        const uint32_t hcolorui = rgbStr2Num(str4);
        rgb_tick_b = RgbColor(hcolorui >> 16, hcolorui >> 8, hcolorui);
      }

      if (!str5.isEmpty()) {
        const uint32_t hcolorui = rgbStr2Num(str5);
        rgb_h = RgbColor(hcolorui >> 16, hcolorui >> 8, hcolorui);
      }

      if (!str6.isEmpty()) {
        const uint32_t hcolorui = rgbStr2Num(str6);
        rgb_m = RgbColor(hcolorui >> 16, hcolorui >> 8, hcolorui);
      }

      if (!str7.isEmpty()) {
        if (equals(str7, F("off"))) {
          rgb_s_off = true;
        } else {
          const uint32_t hcolorui = rgbStr2Num(str7);
          rgb_s_off = false;
          rgb_s     = RgbColor(hcolorui >> 16, hcolorui >> 8,
                               hcolorui);
        }
      }

      # endif // if defined(RGBW) || defined(GRBW)

      if (!parseString(string, 8).isEmpty()) {
        hex2rrggbb(parseString(string, 8));
      }
    }

    else if (equals(subCommand, F("stop"))) {
      success = true;
      mode    = P128_modetype::On;
    }

    else if (equals(subCommand, F("statusrequest"))) {
      success = true;
    }

    if (!success) {
      success = true; // Fake the command to be successful, to get this custom error message out
      String log = F("NeoPixelBus: unknown subcommand: ");
      log += subCommand;
      addLogMove(LOG_LEVEL_INFO, log);

      String json;
      printToWebJSON = true;

      json += '{'; json += '\n';
      json += to_json_object_value(F("plugin"), F("128"));
      json += ','; json += '\n';
      String subjson = F("NeoPixelBus: unknown command: ");
      subjson.reserve(subCommand.length() + 30);
      subjson += subCommand;
      json    += to_json_object_value(F("log"), subjson);
      json    += '\n'; json += '}'; json += '\n';

      // event->Source=EventValueSource::Enum::VALUE_SOURCE_HTTP;
      SendStatus(event, json); // send http response to controller (JSON format)
      printToWeb = false;
    }
    NeoPixelSendStatus(event);

    if (speed == 0) {
      mode = P128_modetype::On; // speed = 0 = stop mode
    }

    // avoid invalid values
    if ((speed > SPEED_MAX) || (speed < -SPEED_MAX)) {
      speed = defaultspeed;
    }

    if (fadetime <= 0) {
      fadetime = 20;
    }
  } // command neopixel

  return success;
}

void P128_data_struct::rgb2colorStr() {
  colorStr.clear();
  colorStr += formatToHex_no_prefix(rgb.R, 2);
  colorStr += formatToHex_no_prefix(rgb.G, 2);
  colorStr += formatToHex_no_prefix(rgb.B, 2);
}

bool P128_data_struct::plugin_fifty_per_second(struct EventStruct *event) {
  counter20ms++;
  lastmode = mode;

  switch (mode) {
    case P128_modetype::Fade:
      fade();
      break;

    case P128_modetype::ColorFade:
      colorfade();
      break;

    case P128_modetype::Rainbow:
      rainbow();
      break;

    case P128_modetype::Kitt:
      kitt();
      break;

    case P128_modetype::Comet:
      comet();
      break;

    case P128_modetype::Theatre:
      theatre();
      break;

    case P128_modetype::Scan:
      scan();
      break;

    case P128_modetype::Dualscan:
      dualscan();
      break;

    case P128_modetype::Twinkle:
      twinkle();
      break;

    case P128_modetype::TwinkleFade:
      twinklefade();
      break;

    case P128_modetype::Sparkle:
      sparkle();
      break;

    case P128_modetype::Fire:
      fire();
      break;

    case P128_modetype::FireFlicker:
      fire_flicker();
      break;

    case P128_modetype::Wipe:
      wipe();
      break;

    case P128_modetype::Dualwipe:
      dualwipe();
      break;

    # if P128_ENABLE_FAKETV
    case P128_modetype::FakeTV:
      faketv();
      break;
    # endif // if P128_ENABLE_FAKETV

    case P128_modetype::SimpleClock:
      Plugin_128_simpleclock();
      break;

    default:
      break;
  } // switch mode

  Plugin_128_pixels->Show();

  if (mode != lastmode) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("NeoPixelBus: Mode Change: ");
      log += P128_modeType_toString(mode);
      addLogMove(LOG_LEVEL_INFO, log);
    }
    NeoPixelSendStatus(event);
  }
  return true;
}

void P128_data_struct::fade(void) {
  for (int pixel = 0; pixel < pixelCount; pixel++) {
    long  counter  = 20 * (counter20ms - starttime[pixel]);
    float progress = (float)counter / (float)fadetime;
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
      mode = P128_modetype::Off;
    } else if (counter20ms > maxtime) {
      mode = P128_modetype::On;
    }

    Plugin_128_pixels->SetPixelColor(pixel, updatedColor);
  }
}

void P128_data_struct::colorfade(void) {
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
  mode = P128_modetype::On;
}

void P128_data_struct::wipe(void) {
  if (counter20ms % (unsigned long)(SPEED_MAX / abs(speed)) == 0) {
    if (speed > 0) {
      Plugin_128_pixels->SetPixelColor(_counter_mode_step, rrggbb);

      if (_counter_mode_step > 0) { Plugin_128_pixels->SetPixelColor(_counter_mode_step - 1, rgb); }
    } else {
      Plugin_128_pixels->SetPixelColor(pixelCount - _counter_mode_step - 1, rrggbb);

      if (_counter_mode_step > 0) { Plugin_128_pixels->SetPixelColor(pixelCount - _counter_mode_step, rgb); }
    }

    if (_counter_mode_step == pixelCount) { mode = P128_modetype::On; }
    _counter_mode_step++;
  }
}

void P128_data_struct::dualwipe(void) {
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
      mode = P128_modetype::On;
      Plugin_128_pixels->SetPixelColor(_counter_mode_step - 1, rgb);
    }
    _counter_mode_step++;
  }
}

# if P128_ENABLE_FAKETV
void P128_data_struct::faketv(void) {
  if (counter20ms >= ftv_holdTime) {
    difference = abs(endpixel - startpixel);

    if (ftv_elapsed >= ftv_fadeTime) {
      // Read next 16-bit (5/6/5) color
      ftv_hi = pgm_read_byte(&ftv_colors[pixelNum * 2]);
      ftv_lo = pgm_read_byte(&ftv_colors[pixelNum * 2 + 1]);

      if (++pixelNum >= NUMPixels) { pixelNum = 0; }

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

      if (random(10) < 3) { ftv_fadeTime = 0; }                   // Force scene cut 30% of time
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
      ftv_r8   = ftv_r >> 8;                 // Quantize to 8-bit
      ftv_g8   = ftv_g >> 8;
      ftv_b8   = ftv_b >> 8;
      ftv_frac = (ftv_i << 16) / difference; // LED index scaled to 0-65535 (16Bit)

      if ((ftv_r8 < 255) && ((ftv_r & 0xFF) >= ftv_frac)) { ftv_r8++; } // Boost some fraction

      if ((ftv_g8 < 255) && ((ftv_g & 0xFF) >= ftv_frac)) { ftv_g8++; } // of LEDs to handle

      if ((ftv_b8 < 255) && ((ftv_b & 0xFF) >= ftv_frac)) { ftv_b8++; } // interp > 8bit

      Plugin_128_pixels->SetPixelColor(ftv_i + startpixel, RgbColor(ftv_r8, ftv_g8, ftv_b8));
    }

    ftv_pr = ftv_nr; // Prev RGB = new RGB
    ftv_pg = ftv_ng;
    ftv_pb = ftv_nb;
  }
}

# endif // if P128_ENABLE_FAKETV

/*
 * Cycles a rainbow over the entire string of LEDs.
 */
void P128_data_struct::rainbow(void) {
  long  counter  = 20 * (counter20ms - starttimerb);
  float progress = (float)counter / (float)fadetime;

  if (fadeIn == true) {
    Plugin_128_pixels->SetBrightness(progress * maxBright); // Safety check
    fadeIn = (progress == 1) ? false : true;
  }

  for (int i = 0; i < pixelCount; i++) {
    uint8_t r1 = (Wheel(((i * 256 / pixelCount) + counter20ms * rainbowspeed / 10) & 255) >> 16);
    uint8_t g1 = (Wheel(((i * 256 / pixelCount) + counter20ms * rainbowspeed / 10) & 255) >> 8);
    uint8_t b1 = (Wheel(((i * 256 / pixelCount) + counter20ms * rainbowspeed / 10) & 255));
    Plugin_128_pixels->SetPixelColor(i, RgbColor(r1, g1, b1));
  }
  mode = (rainbowspeed == 0) ? P128_modetype::On : P128_modetype::Rainbow;
}

/*
 * Put a value 0 to 255 in to get a color value.
 * The colours are a transition r -> g -> b -> back to r
 * Inspired by the Adafruit examples.
 */
uint32_t P128_data_struct::Wheel(uint8_t pos) {
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
void P128_data_struct::kitt(void) {
  if (counter20ms % (unsigned long)(SPEED_MAX / abs(speed)) == 0) {
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
void P128_data_struct::comet(void) {
  if (counter20ms % (unsigned long)(SPEED_MAX / abs(speed)) == 0) {
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
void P128_data_struct::theatre(void) {
  if ((counter20ms % (unsigned long)(SPEED_MAX / abs(speed)) == 0) && (speed != 0)) {
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
void P128_data_struct::scan(void) {
  if ((counter20ms % (unsigned long)(SPEED_MAX / abs(speed)) == 0) && (speed != 0)) {
    if (_counter_mode_step > uint16_t((pixelCount * 2) - 2)) {
      _counter_mode_step = 0;
    }
    _counter_mode_step++;

    int i = _counter_mode_step - (pixelCount - 1);
    i = abs(i);

    // Plugin_128_pixels->ClearTo(rrggbb);
    for (int i = 0; i < pixelCount; i++) {
      Plugin_128_pixels->SetPixelColor(i, rrggbb);
    }
    Plugin_128_pixels->SetPixelColor(abs(i), rgb);
  }
}

/*
 * Runs two pixel back and forth in opposite directions.
 */
void P128_data_struct::dualscan(void) {
  if ((counter20ms % (unsigned long)(SPEED_MAX / abs(speed)) == 0) && (speed != 0)) {
    if (_counter_mode_step > uint16_t((pixelCount * 2) - 2)) {
      _counter_mode_step = 0;
    }

    _counter_mode_step++;

    int i = _counter_mode_step - (pixelCount - 1);
    i = abs(i);

    // Plugin_128_pixels->ClearTo(rrggbb);
    for (int i = 0; i < pixelCount; i++) {
      Plugin_128_pixels->SetPixelColor(i, rrggbb);
    }
    Plugin_128_pixels->SetPixelColor(abs(i),               rgb);
    Plugin_128_pixels->SetPixelColor(pixelCount - (i + 1), rgb);
  }
}

/*
 * Blink several LEDs on, reset, repeat.
 * Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
 */
void P128_data_struct::twinkle(void) {
  if ((counter20ms % (unsigned long)(SPEED_MAX / abs(speed)) == 0) && (speed != 0)) {
    if (_counter_mode_step == 0) {
      // Plugin_128_pixels->ClearTo(rrggbb);
      for (int i = 0; i < pixelCount; i++) {
        Plugin_128_pixels->SetPixelColor(i, rrggbb);
      }
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
void P128_data_struct::twinklefade(void) {
  if ((counter20ms % (unsigned long)(SPEED_MAX / abs(speed)) == 0) && (speed != 0)) {
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
void P128_data_struct::sparkle(void) {
  if ((counter20ms % (unsigned long)(SPEED_MAX / abs(speed)) == 0) && (speed != 0)) {
    // Plugin_128_pixels->ClearTo(rrggbb);
    for (int i = 0; i < pixelCount; i++) {
      Plugin_128_pixels->SetPixelColor(i, rrggbb);
    }
    Plugin_128_pixels->SetPixelColor(random(pixelCount), rgb);
  }
}

void P128_data_struct::fire(void) {
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

/// Generate an 8-bit random number
uint8_t P128_data_struct::random8() {
  rand16seed = (rand16seed * ((uint16_t)(2053))) + ((uint16_t)(13849));

  // return the sum of the high and low bytes, for better
  //  mixing and non-sequential correlation
  return (uint8_t)(((uint8_t)(rand16seed & 0xFF)) +
                   ((uint8_t)(rand16seed >> 8)));
}

/// Generate an 8-bit random number between 0 and lim
/// @param lim the upper bound for the result
uint8_t P128_data_struct::random8(uint8_t lim) {
  uint8_t r = random8();

  r = (r * lim) >> 8;
  return r;
}

/// Generate an 8-bit random number in the given range
/// @param min the lower bound for the random number
/// @param lim the upper bound for the random number
uint8_t P128_data_struct::random8(uint8_t min, uint8_t lim) {
  uint8_t delta = lim - min;
  uint8_t r     = random8(delta) + min;

  return r;
}

/// subtract one byte from another, saturating at 0x00
/// @returns i - j with a floor of 0
uint8_t P128_data_struct::qsub8(uint8_t i, uint8_t j) {
  int t = i - j;

  if (t < 0) { t = 0; }
  return t;
}

/// add one byte to another, saturating at 0xFF
/// @param i - first byte to add
/// @param j - second byte to add
/// @returns the sum of i & j, capped at 0xFF
uint8_t P128_data_struct::qadd8(uint8_t i, uint8_t j) {
  unsigned int t = i + j;

  if (t > 255) { t = 255; }
  return t;
}

///  The "video" version of scale8 guarantees that the output will
///  be only be zero if one or both of the inputs are zero.  If both
///  inputs are non-zero, the output is guaranteed to be non-zero.
///  This makes for better 'video'/LED dimming, at the cost of
///  several additional cycles.
uint8_t P128_data_struct::scale8_video(uint8_t i, uint8_t scale) {
  uint8_t j = (((int)i * (int)scale) >> 8) + ((i && scale) ? 1 : 0);

  // uint8_t nonzeroscale = (scale != 0) ? 1 : 0;
  // uint8_t j = (i == 0) ? 0 : (((int)i * (int)(scale) ) >> 8) + nonzeroscale;
  return j;
}

void P128_data_struct::Fire2012(void) {
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
void P128_data_struct::fire_flicker() {
  if ((counter20ms % (unsigned long)(SPEED_MAX / abs(speed)) == 0) && (speed != 0)) {
    byte w   = 0;   // (SEGMENT.colors[0] >> 24) & 0xFF;
    byte r   = 255; // (SEGMENT.colors[0] >> 16) & 0xFF;
    byte g   = 96;  // (SEGMENT.colors[0] >>  8) & 0xFF;
    byte b   = 12;  // (SEGMENT.colors[0]        & 0xFF);
    byte lum = max(w, max(r, max(g, b))) / rev_intensity;

    for (uint16_t i = 0; i <= NUMPixels - 1; i++) {
      int flicker = random8(lum);

      # if defined(RGBW) || defined(GRBW)
      Plugin_128_pixels->SetPixelColor(i, RgbwColor(max(r - flicker, 0), max(g - flicker, 0), max(b - flicker, 0), max(w - flicker, 0)));
      # else // if defined(RGBW) || defined(GRBW)
      Plugin_128_pixels->SetPixelColor(i, RgbColor(max(r - flicker, 0), max(g - flicker, 0), max(b - flicker, 0)));
      # endif  // if defined(RGBW) || defined(GRBW)
    }
  }
}

void P128_data_struct::Plugin_128_simpleclock() {
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
    if (i % (big_tick / small_tick) == 0) {
      Plugin_128_pixels->SetPixelColor((i * pixelCount * small_tick / 60) % pixelCount, rgb_tick_b);
    } else {
      Plugin_128_pixels->SetPixelColor((i * pixelCount * small_tick / 60) % pixelCount, rgb_tick_s);
    }
  }


  for (int i = 0; i < pixelCount; i++) {
    if (lround((((float)Seconds + ((float)counter20ms - (float)maxtime) / 50.0) * (float)pixelCount) / 60.0) == i) {
      if (rgb_s_off  == false) {
        Plugin_128_pixels->SetPixelColor(i, rgb_s);
      }
    }
    else if (lround((((float)Minutes * 60.0) + (float)Seconds) / 60.0 * (float)pixelCount / 60.0) == i) {
      Plugin_128_pixels->SetPixelColor(i, rgb_m);
    }
    else if (lround(((float)Hours + (float)Minutes / 60) * (float)pixelCount / 12.0)  == i) {
      Plugin_128_pixels->SetPixelColor(i,                                 rgb_h);
      Plugin_128_pixels->SetPixelColor((i + 1) % pixelCount,              rgb_h);
      Plugin_128_pixels->SetPixelColor((i - 1 + pixelCount) % pixelCount, rgb_h);
    }
  }
}

uint32_t P128_data_struct::rgbStr2Num(String rgbStr) {
  uint32_t rgbDec = static_cast<uint32_t>(strtoul(&rgbStr[0], NULL, 16));

  return rgbDec;
}

void P128_data_struct::hex2rgb(const String& hexcolor) {
  colorStr = hexcolor;
  const uint32_t hcolorui = rgbStr2Num(hexcolor);

  # if defined(RGBW) || defined(GRBW)
  hexcolor.length() <= 6
    ? rgb = RgbColor(hcolorui >> 16, hcolorui >> 8, hcolorui)
    : rgb = RgbwColor(hcolorui >> 24, hcolorui >> 16, hcolorui >> 8, hcolorui);
  # else // if defined(RGBW) || defined(GRBW)
  rgb = RgbColor(hcolorui >> 16, hcolorui >> 8, hcolorui);
  # endif // if defined(RGBW) || defined(GRBW)
}

void P128_data_struct::hex2rrggbb(const String& hexcolor) {
  backgroundcolorStr = hexcolor;
  const uint32_t hcolorui = rgbStr2Num(hexcolor);

  # if defined(RGBW) || defined(GRBW)
  hexcolor.length() <= 6
    ? rrggbb = RgbColor(hcolorui >> 16, hcolorui >> 8, hcolorui)
    : rrggbb = RgbwColor(hcolorui >> 24, hcolorui >> 16, hcolorui >> 8, hcolorui);
  # else // if defined(RGBW) || defined(GRBW)
  rrggbb = RgbColor(hcolorui >> 16, hcolorui >> 8, hcolorui);
  # endif // if defined(RGBW) || defined(GRBW)
}

void P128_data_struct::hex2rgb_pixel(const String& hexcolor) {
  colorStr = hexcolor;
  const uint32_t hcolorui = rgbStr2Num(hexcolor);

  for (int i = 0; i < pixelCount; i++) {
    # if defined(RGBW) || defined(GRBW)
    hexcolor.length() <= 6
      ? rgb_target[i] = RgbColor(hcolorui >> 16, hcolorui >> 8, hcolorui)
      : rgb_target[i] = RgbwColor(hcolorui >> 24, hcolorui >> 16, hcolorui >> 8, hcolorui);
    # else // if defined(RGBW) || defined(GRBW)
    rgb_target[i] = RgbColor(hcolorui >> 16, hcolorui >> 8, hcolorui);
    # endif // if defined(RGBW) || defined(GRBW)
  }
}

// ---------------------------------------------------------------------------------
// ------------------------------ JsonResponse -------------------------------------
// ---------------------------------------------------------------------------------
void P128_data_struct::NeoPixelSendStatus(struct EventStruct *eventSource) {
  String log = F("NeoPixelBusFX: Set ");

  log += rgb.R;
  log += '/';
  log += rgb.G;
  log += '/';
  log += rgb.B;

  addLogMove(LOG_LEVEL_INFO, log);

  String json;

  json.reserve(285); // Awfully long string :-|

  printToWebJSON = true;

  json += '{'; json += '\n';                                                               // 2
  json += to_json_object_value(F("plugin"), F("128"));                                     // 12
  json += ','; json += '\n';
  json += to_json_object_value(F("mode"), P128_modeType_toString(mode));                   // 14..23
  json += ','; json += '\n';
  json += to_json_object_value(F("lastmode"), P128_modeType_toString(savemode));           // 18..27
  json += ','; json += '\n';
  json += to_json_object_value(F("fadetime"), toString(fadetime, 0));                      // 15..19
  json += ','; json += '\n';
  json += to_json_object_value(F("fadedelay"), toString(fadedelay, 0));                    // 15..19
  json += ','; json += '\n';
  json += to_json_object_value(F("dim"), toString(Plugin_128_pixels->GetBrightness(), 0)); // 8..10
  json += ','; json += '\n';
  json += to_json_object_value(F("rgb"), colorStr, true);                                  // 15..17
  json += ','; json += '\n';

  HsbColor hsbColor = HsbColor(RgbColor(rgb.R, rgb.G, rgb.B));                             // Calculate only once

  json += to_json_object_value(F("hue"), toString(hsbColor.H * 360.0f, 0));                // 17
  json += ','; json += '\n';
  json += to_json_object_value(F("saturation"), toString(hsbColor.S * 100.0f, 0));         // 26?
  json += ','; json += '\n';
  json += to_json_object_value(F("brightness"), toString(hsbColor.B * 100.0f, 0));         // 26?
  json += ','; json += '\n';
  json += to_json_object_value(F("bgcolor"), backgroundcolorStr, true);                    // 21..23
  json += ','; json += '\n';
  json += to_json_object_value(F("count"), toString(count, 0));                            // 12..15
  json += ','; json += '\n';
  json += to_json_object_value(F("speed"), toString(speed, 0));                            // 12..14
  json += ','; json += '\n';
  json += to_json_object_value(F("pixelcount"), toString(pixelCount, 0));                  // 17..19
  json += '\n'; json += '}'; json += '\n';                                                 // 4

  SendStatus(eventSource, json);                                                           // send http response to controller (JSON format)
  printToWeb = false;
}

const __FlashStringHelper * P128_data_struct::P128_modeType_toString(P128_modetype modeType) {
  switch (modeType) {
    case P128_modetype::Off: return F("off");
    case P128_modetype::On: return F("on");
    case P128_modetype::Fade: return F("fade");
    case P128_modetype::ColorFade: return F("colorfade");
    case P128_modetype::Rainbow: return F("rainbow");
    case P128_modetype::Kitt: return F("kitt");
    case P128_modetype::Comet: return F("comet");
    case P128_modetype::Theatre: return F("theatre");
    case P128_modetype::Scan: return F("scan");
    case P128_modetype::Dualscan: return F("dualscan");
    case P128_modetype::Twinkle: return F("twinkle");
    case P128_modetype::TwinkleFade: return F("twinklefade");
    case P128_modetype::Sparkle: return F("sparkle");
    case P128_modetype::Fire: return F("fire");
    case P128_modetype::FireFlicker: return F("fireflicker");
    case P128_modetype::Wipe: return F("wipe");
    case P128_modetype::Dualwipe: return F("dualwipe");
    # if P128_ENABLE_FAKETV
    case P128_modetype::FakeTV: return F("faketv");
    # endif // if P128_ENABLE_FAKETV
    case P128_modetype::SimpleClock: return F("simpleclock");
  }
  return F("*unknown*");
}

#endif // ifdef USES_P128
