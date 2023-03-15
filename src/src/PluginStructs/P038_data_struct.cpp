#include "../PluginStructs/P038_data_struct.h"

#ifdef USES_P038

// **************************************************************************/
// Constructor
// **************************************************************************/
P038_data_struct::P038_data_struct(int8_t   gpioPin,
                                   uint16_t ledCount,
                                   uint8_t  stripType,
                                   uint8_t  brightness,
                                   uint8_t  maxbright)
  : _gpioPin(gpioPin), _maxPixels(ledCount), _stripType(stripType), _brightness(brightness), _maxbright(maxbright) {}

// **************************************************************************/
// Destructor
// **************************************************************************/
P038_data_struct::~P038_data_struct() {
  if (Plugin_038_pixels != nullptr) {
    delete Plugin_038_pixels;
    Plugin_038_pixels = nullptr;
  }
}

bool P038_data_struct::plugin_init(struct EventStruct *event) {
  bool success = false;

  if (!isInitialized()) {
    Plugin_038_pixels = new (std::nothrow) Adafruit_NeoPixel(_maxPixels,
                                                             _gpioPin,
                                                             (_stripType == P038_STRIP_TYPE_RGBW ? NEO_GRBW : NEO_GRB) + NEO_KHZ800);

    if (Plugin_038_pixels != nullptr) {
      Plugin_038_pixels->begin();                                          // This initializes the NeoPixel library.
      Plugin_038_pixels->setBrightness(std::min(_maxbright, _brightness)); // Set brightness, so we don't get blinded by the light
      success = true;
    }
  }

  return success;
}

bool P038_data_struct::plugin_exit(struct EventStruct *event) {
  if (isInitialized()) {
    delete Plugin_038_pixels;
    Plugin_038_pixels = nullptr;
  }
  return true;
}

bool P038_data_struct::plugin_write(struct EventStruct *event, const String& string) {
  bool success = false;

  if (isInitialized()) {
    const String cmd = parseString(string, 1);

    if (!cmd.startsWith(F("neo"))) {
      return success;
    }

    {
      String log;

      if (loglevelActiveFor(LOG_LEVEL_INFO) &&
          log.reserve(64)) {
        log += F("P038 : write - ");
        log += string;
        addLogMove(LOG_LEVEL_INFO, log);
      }
    }

    success = true;

    if (equals(cmd, F("neopixel"))) { // NeoPixel
      Plugin_038_pixels->setPixelColor(event->Par1 - 1, Plugin_038_pixels->Color(event->Par2, event->Par3, event->Par4, event->Par5));
    } else

    if (equals(cmd, F("neopixelbright")) && (event->Par1 >= 0) && (event->Par1 <= 255)) {
      if (parseString(string, 2).isEmpty() || (event->Par1 == 0)) {          // No argument or 0, then
        Plugin_038_pixels->setBrightness(std::min(_maxbright, _brightness)); // use initial brightness
      } else {
        Plugin_038_pixels->setBrightness(std::min(_maxbright, static_cast<uint8_t>(event->Par1)));
      }
    } else

    // extra function to receive HSV values (i.e. homie controler)
    if (equals(cmd, F("neopixelhsv"))) { // NeoPixelHSV
      int rgbw[4];
      rgbw[3] = 0;

      HSV2RGBWorRGBandLog(event->Par2, event->Par3, event->Par4, rgbw);

      Plugin_038_pixels->setPixelColor(event->Par1 - 1, Plugin_038_pixels->Color(rgbw[0], rgbw[1], rgbw[2], rgbw[3]));
    } else

    if (equals(cmd, F("neopixelall"))) { // NeoPixelAll
      for (int i = 0; i < _maxPixels; i++) {
        Plugin_038_pixels->setPixelColor(i, Plugin_038_pixels->Color(event->Par1, event->Par2, event->Par3, event->Par4));
      }
    } else

    if (equals(cmd, F("neopixelallhsv"))) { // NeoPixelAllHSV
      int rgbw[4];
      rgbw[3] = 0;

      HSV2RGBWorRGBandLog(event->Par1, event->Par2, event->Par3, rgbw);

      for (int i = 0; i < _maxPixels; i++) {
        Plugin_038_pixels->setPixelColor(i, Plugin_038_pixels->Color(rgbw[0], rgbw[1], rgbw[2], rgbw[3]));
      }
    } else

    if (equals(cmd, F("neopixelline"))) {                      // NeoPixelLine
      int brightness = 0;
      validIntFromString(parseString(string, 7), brightness); // Get 7th argument aka Par6

      for (int i = event->Par1 - 1; i < event->Par2; i++) {
        Plugin_038_pixels->setPixelColor(i, Plugin_038_pixels->Color(event->Par3, event->Par4, event->Par5, brightness));
      }
    } else

    if (equals(cmd, F("neopixellinehsv"))) { // NeoPixelLineHSV
      int rgbw[4];
      rgbw[3] = 0;

      HSV2RGBWorRGBandLog(event->Par3, event->Par4, event->Par5, rgbw);

      for (int i = event->Par1 - 1; i < event->Par2; i++) {
        Plugin_038_pixels->setPixelColor(i, Plugin_038_pixels->Color(rgbw[0], rgbw[1], rgbw[2], rgbw[3]));
      }
    } else {
      success = false;
    }

    if (success) {
      Plugin_038_pixels->show(); // This sends the updated pixel color to the hardware.
    }
  }
  return success;
}

void P038_data_struct::HSV2RGBWorRGBandLog(float H, float S, float V, int rgbw[4]) {
  if (_stripType == P038_STRIP_TYPE_RGBW) { // RGBW
    HSV2RGBW(H, S, V, rgbw);
  } else {                                  // RGB
    HSV2RGB(H, S, V, rgbw);
  }
  String log;

  if (loglevelActiveFor(LOG_LEVEL_INFO) &&
      log.reserve(48)) {
    log += F("P038 HSV converted to RGB(W):");
    log += rgbw[0];
    log += ',';
    log += rgbw[1];
    log += ',';
    log += rgbw[2];
    log += ',';
    log += rgbw[3];
    addLogMove(LOG_LEVEL_INFO, log);
  }
}

#endif // ifdef USES_P038
