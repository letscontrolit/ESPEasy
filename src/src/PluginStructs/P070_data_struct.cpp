#include "../PluginStructs/P070_data_struct.h"

#ifdef USES_P070


P070_data_struct::~P070_data_struct() {
  if (Plugin_070_pixels != nullptr) {
    delete Plugin_070_pixels;
    Plugin_070_pixels = nullptr;
  }
}

void P070_data_struct::reset() {
  if (Plugin_070_pixels != nullptr) {
    delete Plugin_070_pixels;
    Plugin_070_pixels = nullptr;
  }
}

void P070_data_struct::init(struct EventStruct *event) {
  if (nullptr == Plugin_070_pixels)
  {
    set(event);

    const uint8_t neoFlags = (led_type == P070_LED_TYPE_RGBW)
                             ? (NEO_GRBW + NEO_KHZ800)
                             : (NEO_GRB  + NEO_KHZ800);

    Plugin_070_pixels = new (std::nothrow) NeoPixelBus_wrapper(number_leds, CONFIG_PIN1, neoFlags);

    if (Plugin_070_pixels == nullptr) {
      return;
    }
    Plugin_070_pixels->begin();
  }
  else
  {
    set(event);
  }
}

void P070_data_struct::set(struct EventStruct *event) {
  display_enabled       = PCONFIG(0);
  brightness            = PCONFIG(1);
  brightness_hour_marks = PCONFIG(2);
  offset_12h_mark       = PCONFIG(3);
  thick_12_mark         = PCONFIG(4);
  led_mode              = PCONFIG(5);
  auto_brightness       = PCONFIG(6);
  lightshow_mode        = PCONFIG(7);
  lightshow_duration    = PCONFIG(8);
  led_type              = PCONFIG(9);

  // Color settings stored in PCONFIG_LONG
  color_hours   = PCONFIG_LONG(0);
  color_minutes = PCONFIG_LONG(1);
  color_seconds = PCONFIG_LONG(2);
  color_marks   = PCONFIG_LONG(3);

  // Clamp colors to valid range
  if (color_hours   >= P070_COLOR_COUNT) { color_hours   = P070_COLOR_RED;   }
  if (color_minutes >= P070_COLOR_COUNT) { color_minutes = P070_COLOR_GREEN; }
  if (color_seconds >= P070_COLOR_COUNT) { color_seconds = P070_COLOR_BLUE;  }
  if (color_marks   >= P070_COLOR_COUNT) { color_marks   = P070_COLOR_WHITE; }

  // Clamp lightshow duration
  if (lightshow_duration < 1)  { lightshow_duration = 1;  }
  if (lightshow_duration > 30) { lightshow_duration = 30; }

  // Set actual LED count based on selected mode
  switch (led_mode) {
    case P070_LED_MODE_24: number_leds = NUMBER_LEDS_24; break;
    case P070_LED_MODE_16: number_leds = NUMBER_LEDS_16; break;
    case P070_LED_MODE_12: number_leds = NUMBER_LEDS_12; break;
    default:               number_leds = NUMBER_LEDS_60; break; // P070_LED_MODE_60
  }
}

// ---------------------------------------------------------------------------
// Color helpers
// ---------------------------------------------------------------------------

void P070_data_struct::colorFromIndex(uint8_t colorIndex, uint8_t bright, uint8_t &r, uint8_t &g, uint8_t &b) {
  r = 0; g = 0; b = 0;
  switch (colorIndex) {
    case P070_COLOR_RED:     r = bright;                         break;
    case P070_COLOR_GREEN:   g = bright;                         break;
    case P070_COLOR_BLUE:    b = bright;                         break;
    case P070_COLOR_YELLOW:  r = bright; g = bright;             break;
    case P070_COLOR_CYAN:    g = bright; b = bright;             break;
    case P070_COLOR_MAGENTA: r = bright; b = bright;             break;
    case P070_COLOR_WHITE:   r = bright; g = bright; b = bright; break;
    default:                 r = bright;                         break;
  }
}

// Sets a pixel respecting RGB vs RGBW mode.
// For RGBW: WHITE color uses the dedicated W channel for a pure white.
// All other colors use RGB channels with W=0.
void P070_data_struct::setPixel(uint16_t pos, uint8_t r, uint8_t g, uint8_t b) {
  if (led_type == P070_LED_TYPE_RGBW) {
    // If all channels equal → pure white via W channel
    if (r == g && g == b) {
      Plugin_070_pixels->setPixelColor(pos, Plugin_070_pixels->Color(0, 0, 0, r));
    } else {
      Plugin_070_pixels->setPixelColor(pos, Plugin_070_pixels->Color(r, g, b, 0));
    }
  } else {
    Plugin_070_pixels->setPixelColor(pos, Plugin_070_pixels->Color(r, g, b));
  }
}



uint8_t P070_data_struct::luxToBrightness(float lux) {
  if (lux >= 1000.0f) { return 255; } // Sehr hell
  if (lux >= 500.0f)  { return 128; } // Hell
  if (lux >= 100.0f)  { return 100; } // Flurbeleuchtung
  if (lux >= 50.0f)   { return  64; } // Dunkel
  return 32;                           // Sehr dunkel (0..49 lux)
}

void P070_data_struct::applyAutoBrightness() {
  if (!auto_brightness) { return; }

  for (taskIndex_t i = 0; i < TASKS_MAX; i++) {
    if (Settings.TaskDeviceEnabled[i]) {
      LoadTaskSettings(i);
      if (String(ExtraTaskSettings.TaskDeviceName).equalsIgnoreCase(F(P070_LUX_TASK_NAME))) {
        const float   lux            = UserVar.getFloat(i, 0);
        const uint8_t new_brightness = luxToBrightness(lux);
        brightness            = new_brightness;
        brightness_hour_marks = new_brightness;
        return;
      }
    }
  }
}

// ---------------------------------------------------------------------------
// Hourly lightshow
// ---------------------------------------------------------------------------

void P070_data_struct::runLightshow() {
  if (lightshow_mode == P070_LIGHTSHOW_OFF) { return; }

  const int sec = node_time.second();
  const int min = node_time.minute();

  // --- Start condition: exactly at hh:00:00 ---
  if (min == 0 && sec == 0) {
    lightshow_running     = true;
    lightshow_step        = 0;
    lightshow_flash_phase = 0;
    // Pick a random color (avoid repeating the same one)
    uint8_t newColor;
    do {
      newColor = (uint8_t)(random(P070_COLOR_COUNT));
    } while (newColor == lightshow_color && P070_COLOR_COUNT > 1);
    lightshow_color = newColor;
  }

  // --- Stop condition ---
  if (lightshow_running && sec >= lightshow_duration) {
    lightshow_running = false;
    lightshow_step    = 0;
    lightshow_flash_phase = 0;
  }

  if (!lightshow_running) { return; }

  const uint8_t bright = brightness > 0 ? brightness : 128;

  // -----------------------------------------------------------------------
  // Mode 1: Colour-chase (Lauflicht)
  // -----------------------------------------------------------------------
  if (lightshow_mode == P070_LIGHTSHOW_CHASE) {
    clearClock();

    const uint8_t trail = 3;
    for (uint8_t t = 0; t <= trail; t++) {
      int pos = (int)lightshow_step - (int)t;
      if (pos < 0) { pos += number_leds; }
      pos = pos % number_leds;

      uint8_t fade = bright >> t; // each trail LED half as bright
      uint8_t r, g, b;
      colorFromIndex(lightshow_color, fade, r, g, b);
      setPixel(pos, r, g, b);
    }

    Plugin_070_pixels->show();
    lightshow_step = (lightshow_step + (number_leds / lightshow_duration) + 1) % number_leds;
    return;
  }

  // -----------------------------------------------------------------------
  // Mode 2: Flash — all LEDs white, then hand blink, repeating
  // Each "flash cycle" = 2 seconds: sec 0 flash-on, sec 1 flash-off+hands
  // -----------------------------------------------------------------------
  if (lightshow_mode == P070_LIGHTSHOW_FLASH) {
    const uint8_t cycle = sec % 2; // 0 = flash all, 1 = show hands briefly

    if (cycle == 0) {
      // All LEDs on in the chosen random color
      uint8_t r, g, b;
      colorFromIndex(lightshow_color, bright, r, g, b);
      for (int i = 0; i < number_leds; i++) {
        setPixel(i, r, g, b);
      }
      Plugin_070_pixels->show();
    }
    else {
      // Show clock hands briefly then clear (hand blink)
      clearClock();

      int hours   = node_time.hour();
      int minutes = node_time.minute();
      int seconds = node_time.second();
      if (hours > 11) { hours -= 12; }

      const float scale = (float)number_leds / 60.0f;
      int hpos = (int)((hours * 5 + minutes / 12) * scale + 0.5f) + offset_12h_mark;
      hpos = ((hpos % number_leds) + number_leds) % number_leds;
      int mpos = (int)(minutes * scale + 0.5f) + offset_12h_mark;
      mpos = ((mpos % number_leds) + number_leds) % number_leds;
      int spos = (int)(seconds * scale + 0.5f) + offset_12h_mark;
      spos = ((spos % number_leds) + number_leds) % number_leds;

      // Hour hand: 3 LEDs wide (cyan)
      uint8_t r, g, b;
      colorFromIndex(P070_COLOR_CYAN, bright, r, g, b);
      setPixel(hpos % number_leds,                     r, g, b);
      setPixel((hpos + 1) % number_leds,               r, g, b);
      setPixel((hpos - 1 + number_leds) % number_leds, r, g, b);

      // Minute hand: magenta
      colorFromIndex(P070_COLOR_MAGENTA, bright, r, g, b);
      setPixel(mpos % number_leds, r, g, b);

      // Second hand: yellow
      colorFromIndex(P070_COLOR_YELLOW, bright, r, g, b);
      setPixel(spos % number_leds, r, g, b);

      Plugin_070_pixels->show();
    }
    return;
  }
}

// ---------------------------------------------------------------------------
// Main clock update
// ---------------------------------------------------------------------------

void P070_data_struct::Clock_update()
{
  applyAutoBrightness();

  // If lightshow is active, runLightshow() handles drawing and show()
  if (lightshow_mode != P070_LIGHTSHOW_OFF) {
    runLightshow();
    if (lightshow_running) { return; }
  }

  clearClock();

  if (display_enabled > 0) {
    int Hours   = node_time.hour();
    int Minutes = node_time.minute();
    int Seconds = node_time.second();
    timeToStrip(Hours, Minutes, Seconds);
  }
  Plugin_070_pixels->show();
}

// ---------------------------------------------------------------------------
// Hour marks
// ---------------------------------------------------------------------------

void P070_data_struct::calculateMarks()
{
  // For each mode: 12 hour marks evenly distributed, plus optional thick 12h mark
  // number of LEDs per hour-mark interval: total / 12
  const uint8_t step = number_leds / 12; // 60→5, 24→2, 16→1 (rounded), 12→1

  for (int i = 0; i < 12; i++) {
    marks[i] = (step * i + offset_12h_mark) % number_leds;
  }

  if (thick_12_mark && step >= 2) {
    // Only add neighbours when there is room (step >= 2)
    marks[12] = (offset_12h_mark + 1) % number_leds;
    marks[13] = (offset_12h_mark + number_leds - 1) % number_leds;
  } else {
    marks[12] = 255;
    marks[13] = 255;
  }
}

// ---------------------------------------------------------------------------
// Clear
// ---------------------------------------------------------------------------

void P070_data_struct::clearClock() {
  for (int i = 0; i < number_leds; i++) {
    if (led_type == P070_LED_TYPE_RGBW) {
      Plugin_070_pixels->setPixelColor(i, Plugin_070_pixels->Color(0, 0, 0, 0));
    } else {
      Plugin_070_pixels->setPixelColor(i, Plugin_070_pixels->Color(0, 0, 0));
    }
  }
}

// ---------------------------------------------------------------------------
// Draw clock hands
// ---------------------------------------------------------------------------

void P070_data_struct::timeToStrip(int hours, int minutes, int seconds) {
  if (hours > 11) { hours = hours - 12; }

  // Scale factor: how many LEDs per full rotation
  const float scale = (float)number_leds / 60.0f; // 60→1.0, 24→0.4, 16→0.267, 12→0.2

  // Hours: 12h * 5 positions = 60 base units, moves 1 unit per 12 min
  int hpos = (int)((hours * 5 + minutes / 12) * scale + 0.5f) + offset_12h_mark;
  hpos = ((hpos % number_leds) + number_leds) % number_leds;

  // Minutes: 60 base units
  int mpos = (int)(minutes * scale + 0.5f) + offset_12h_mark;
  mpos = ((mpos % number_leds) + number_leds) % number_leds;

  // Seconds: 60 base units
  int spos = (int)(seconds * scale + 0.5f) + offset_12h_mark;
  spos = ((spos % number_leds) + number_leds) % number_leds;

  // Draw hour marks
  uint8_t mr, mg, mb;
  colorFromIndex(color_marks, brightness_hour_marks, mr, mg, mb);

  for (int i = 0; i < 14; i++) {
    if ((marks[i] != (uint8_t)hpos) && (marks[i] != (uint8_t)mpos) &&
        (marks[i] != (uint8_t)spos) && (marks[i] != 255)) {
      setPixel(marks[i], mr, mg, mb);
    }
  }

  // Draw clock hands — blend colors when hands overlap
  uint8_t r, g, b;
  uint32_t currentColor;

  for (int i = 0; i < number_leds; i++) {
    if (i == hpos) {
      colorFromIndex(color_hours, brightness, r, g, b);
      setPixel(i, r, g, b);
    }
    if (i == mpos) {
      currentColor = Plugin_070_pixels->getPixelColor(i);
      uint8_t er = (uint8_t)(currentColor >> 16);
      uint8_t eg = (uint8_t)(currentColor >>  8);
      uint8_t eb = (uint8_t)(currentColor);
      colorFromIndex(color_minutes, brightness, r, g, b);
      setPixel(i, er | r, eg | g, eb | b);
    }
    if (i == spos) {
      currentColor = Plugin_070_pixels->getPixelColor(i);
      uint8_t er = (uint8_t)(currentColor >> 16);
      uint8_t eg = (uint8_t)(currentColor >>  8);
      uint8_t eb = (uint8_t)(currentColor);
      colorFromIndex(color_seconds, brightness, r, g, b);
      setPixel(i, er | r, eg | g, eb | b);
    }
  }
}

#endif // ifdef USES_P070
