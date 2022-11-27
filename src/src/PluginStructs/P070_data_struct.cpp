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
  if (!Plugin_070_pixels)
  {
    Plugin_070_pixels = new (std::nothrow) Adafruit_NeoPixel(NUMBER_LEDS, CONFIG_PIN1, NEO_GRB + NEO_KHZ800);

    if (Plugin_070_pixels == nullptr) {
      return;
    }
    Plugin_070_pixels->begin(); // This initializes the NeoPixel library.
  }
  set(event);
}

void P070_data_struct::set(struct EventStruct *event) {
  display_enabled       = PCONFIG(0);
  brightness            = PCONFIG(1);
  brightness_hour_marks = PCONFIG(2);
  offset_12h_mark       = PCONFIG(3);
  thick_12_mark         = PCONFIG(4);
}

void P070_data_struct::Clock_update()
{
  clearClock();              // turn off the LEDs

  if (display_enabled > 0) { // if the display is enabled, calculate the LEDs to turn on
    int Hours   = node_time.hour();
    int Minutes = node_time.minute();
    int Seconds = node_time.second();
    timeToStrip(Hours, Minutes, Seconds);
  }
  Plugin_070_pixels->show(); // This sends the updated pixel color to the hardware.
}

void P070_data_struct::calculateMarks()
{ // generate a list of the LEDs that have hour marks
  for (int i = 0; i < 12; i++) {
    marks[i] = 5 * i + (offset_12h_mark % 5);
  }

  if (thick_12_mark) {
    if (offset_12h_mark == 0) {
      marks[12] = 1;
      marks[13] = 59;
    }
    else if (offset_12h_mark == 59) {
      marks[12] = 0;
      marks[13] = 58;
    }
    else {
      marks[12] = offset_12h_mark + 1;
      marks[13] = offset_12h_mark - 1;
    }
  }
  else {
    marks[12] = 255;
    marks[13] = 255;
  }
}

void P070_data_struct::clearClock() {
  for (int i = 0; i < NUMBER_LEDS; i++) {
    Plugin_070_pixels->setPixelColor(i, Plugin_070_pixels->Color(0, 0, 0));
  }
}

void P070_data_struct::timeToStrip(int hours, int minutes, int seconds) {
  if (hours > 11) { hours = hours - 12; }
  hours = (hours * 5) + (minutes / 12) + offset_12h_mark; // make the hour hand move each 12 minutes and apply the offset

  if (hours > 59) { hours = hours - 60; }
  minutes = minutes + offset_12h_mark;                    // apply offset to minutes

  if (minutes > 59) { minutes = minutes - 60; }
  seconds = seconds + offset_12h_mark;                    // apply offset to seconds

  if (seconds > 59) { seconds = seconds - 60; }

  for (int i = 0; i < 14; i++) {                                                                      // set the hour marks as white;
    if ((marks[i] != hours) && (marks[i] != minutes) && (marks[i] != seconds) && (marks[i] != 255)) { // do not draw a mark there is a clock
                                                                                                      // hand in that position
      Plugin_070_pixels->setPixelColor(marks[i],
                                       Plugin_070_pixels->Color(brightness_hour_marks, brightness_hour_marks, brightness_hour_marks));
    }
  }
  uint32_t currentColor;
  uint8_t  r_val, g_val;                  // , b_val;

  for (int i = 0; i < NUMBER_LEDS; i++) { // draw the clock hands, adding the colors together
    if (i == hours) {                     // hours hand is RED
      Plugin_070_pixels->setPixelColor(i, Plugin_070_pixels->Color(brightness, 0, 0));
    }

    if (i == minutes) { // minutes hand is GREEN
      currentColor = Plugin_070_pixels->getPixelColor(i);
      r_val        = (uint8_t)(currentColor >> 16);
      Plugin_070_pixels->setPixelColor(i, Plugin_070_pixels->Color(r_val, brightness, 0));
    }

    if (i == seconds) { // seconds hand is BLUE
      currentColor = Plugin_070_pixels->getPixelColor(i);
      r_val        = (uint8_t)(currentColor >> 16);
      g_val        = (uint8_t)(currentColor >>  8);
      Plugin_070_pixels->setPixelColor(i, Plugin_070_pixels->Color(r_val, g_val, brightness));
    }
  }
}

#endif // ifdef USES_P070
