#include "../PluginStructs/P038_data_struct.h"

#ifdef USES_P038

// **************************************************************************/
// Constructor
// **************************************************************************/
P038_data_struct::P038_data_struct(int8_t gpioPin, uint16_t ledCount, uint8_t stripType)
  : _gpioPin(gpioPin), _maxPixels(ledCount), _stripType(stripType) {}

// **************************************************************************/
// Destructor
// **************************************************************************/
P038_data_struct::~P038_data_struct() {
  if (isInitialized()) {
    delete Plugin_038_pixels;
    Plugin_038_pixels = nullptr;
  }
}

bool P038_data_struct::plugin_init(struct EventStruct *event) {
  bool success = false;

  if (!isInitialized()) {
    if (_stripType == P038_STRIP_TYPE_RGBW) {
      Plugin_038_pixels = new (std::nothrow) Adafruit_NeoPixel(_maxPixels, _gpioPin, NEO_GRBW + NEO_KHZ800);
    }
    else {
      Plugin_038_pixels = new (std::nothrow) Adafruit_NeoPixel(_maxPixels, _gpioPin, NEO_GRB + NEO_KHZ800);
    }

    if (Plugin_038_pixels != nullptr) {
      Plugin_038_pixels->begin(); // This initializes the NeoPixel library.
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
    String log;

    if (loglevelActiveFor(LOG_LEVEL_INFO) &&
        log.reserve(64)) {
      log  = F("P038 : ");
      log += string;
    }

    String cmd = parseString(string, 1);

    if (cmd.equalsIgnoreCase(F("NeoPixel"))) {
      Plugin_038_pixels->setPixelColor(event->Par1 - 1, Plugin_038_pixels->Color(event->Par2, event->Par3, event->Par4, event->Par5));
      Plugin_038_pixels->show(); // This sends the updated pixel color to the hardware.
      success = true;
    }

    // extra function to receive HSV values (i.e. homie controler)
    if (cmd.equalsIgnoreCase(F("NeoPixelHSV"))) {
      int rgbw[4];
      rgbw[3] = 0;

      if (_stripType == P038_STRIP_TYPE_RGBW) { // RGBW
        HSV2RGBW(event->Par2, event->Par3, event->Par4, rgbw);
      } else {                                  // RGB
        HSV2RGB(event->Par2, event->Par3, event->Par4, rgbw);
      }

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log += F(" HSV converted to RGB(W):");
        log += rgbw[0];
        log += ',';
        log += rgbw[1];
        log += ',';
        log += rgbw[2];
        log += ',';
        log += rgbw[3];
        addLog(LOG_LEVEL_INFO, log);
      }
      Plugin_038_pixels->setPixelColor(event->Par1 - 1, Plugin_038_pixels->Color(rgbw[0], rgbw[1], rgbw[2], rgbw[3]));
      Plugin_038_pixels->show(); // This sends the updated pixel color to the hardware.
      success = true;
    }

    if (cmd.equalsIgnoreCase(F("NeoPixelAll"))) {
      for (int i = 0; i < _maxPixels; i++) {
        Plugin_038_pixels->setPixelColor(i, Plugin_038_pixels->Color(event->Par1, event->Par2, event->Par3, event->Par4));
      }
      Plugin_038_pixels->show();
      success = true;
    }

    if (cmd.equalsIgnoreCase(F("NeoPixelAllHSV"))) {
      int rgbw[4];
      rgbw[3] = 0;

      if (_stripType == P038_STRIP_TYPE_RGBW) { // RGBW
        HSV2RGBW(event->Par1, event->Par2, event->Par3, rgbw);
      } else {                                  // RGB
        HSV2RGB(event->Par1, event->Par2, event->Par3, rgbw);
      }

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log += F(" HSV converted to RGB(W):");
        log += rgbw[0];
        log += ',';
        log += rgbw[1];
        log += ',';
        log += rgbw[2];
        log += ',';
        log += rgbw[3];
        addLog(LOG_LEVEL_INFO, log);
      }

      for (int i = 0; i < _maxPixels; i++) {
        Plugin_038_pixels->setPixelColor(i, Plugin_038_pixels->Color(rgbw[0], rgbw[1], rgbw[2], rgbw[3]));
      }
      Plugin_038_pixels->show();
      success = true;
    }

    if (cmd.equalsIgnoreCase(F("NeoPixelLine"))) {
      for (int i = event->Par1 - 1; i < event->Par2; i++) {
        Plugin_038_pixels->setPixelColor(i, Plugin_038_pixels->Color(event->Par3, event->Par4, event->Par5));
      }
      Plugin_038_pixels->show();
      success = true;
    }

    if (cmd.equalsIgnoreCase(F("NeoPixelLineHSV"))) {
      int rgbw[4];
      rgbw[3] = 0;

      if (_stripType == P038_STRIP_TYPE_RGBW) { // RGBW
        HSV2RGBW(event->Par3, event->Par4, event->Par5, rgbw);
      } else {                                  // RGB
        HSV2RGB(event->Par3, event->Par4, event->Par5, rgbw);
      }

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log += F(" HSV converted to RGB(W):");
        log += rgbw[0];
        log += ',';
        log += rgbw[1];
        log += ',';
        log += rgbw[2];
        log += ',';
        log += rgbw[3];
        addLog(LOG_LEVEL_INFO, log);
      }

      for (int i = event->Par1 - 1; i < event->Par2; i++) {
        Plugin_038_pixels->setPixelColor(i, Plugin_038_pixels->Color(rgbw[0], rgbw[1], rgbw[2], rgbw[3]));
      }
      Plugin_038_pixels->show();
      success = true;
    }
  }
  return success;
}

#endif // ifdef USES_P038
