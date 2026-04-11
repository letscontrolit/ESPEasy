#ifndef _NEOPIXELBUS_WRAPPER_CPP
#define _NEOPIXELBUS_WRAPPER_CPP
#include "./NeoPixelBus_wrapper.h"

NeoPixelBus_wrapper::NeoPixelBus_wrapper(uint16_t     _maxPixels,
                                         int16_t      _gpioPin,
                                         neoPixelType _stripType)
  #ifndef NEOPIXEL_WRAPPER_USE_ADAFRUIT
  : numLEDs(_maxPixels)
  #else // ifndef NEOPIXEL_WRAPPER_USE_ADAFRUIT
  : Adafruit_NeoPixel(_maxPixels, _gpioPin, _stripType)
  #endif // ifndef NEOPIXEL_WRAPPER_USE_ADAFRUIT
{
  #ifndef NEOPIXEL_WRAPPER_USE_ADAFRUIT

  if (NEO_GRBW == (_stripType & NEO_GRBW)) {
    # ifdef ESP8266
    neopixels_grbw = new (std::nothrow) NEOPIXEL_LIB<NeoGrbwFeature, METHOD>(_maxPixels);
    # endif // ifdef ESP8266
    # ifdef ESP32
    neopixels_grbw = new (std::nothrow) NEOPIXEL_LIB<NeoGrbwFeature, METHOD>(_maxPixels, _gpioPin);
    # endif // ifdef ESP32
  } else if (NEO_GRB == (_stripType & NEO_GRB)) {
    # ifdef ESP8266
    neopixels_grb = new (std::nothrow) NEOPIXEL_LIB<NeoGrbFeature, METHOD>(_maxPixels);
    # endif // ifdef ESP8266
    # ifdef ESP32
    neopixels_grb = new (std::nothrow) NEOPIXEL_LIB<NeoGrbFeature, METHOD>(_maxPixels, _gpioPin);
    # endif // ifdef ESP32
  }
  #endif // ifndef NEOPIXEL_WRAPPER_USE_ADAFRUIT
}

NeoPixelBus_wrapper::~NeoPixelBus_wrapper() {
  #ifndef NEOPIXEL_WRAPPER_USE_ADAFRUIT
  delete neopixels_grb;
  neopixels_grb = nullptr;
  delete neopixels_grbw;
  neopixels_grbw = nullptr;
  #endif // ifndef NEOPIXEL_WRAPPER_USE_ADAFRUIT
}

#ifndef NEOPIXEL_WRAPPER_USE_ADAFRUIT
void NeoPixelBus_wrapper::begin() {
  if (nullptr != neopixels_grb) {
    neopixels_grb->Begin();
  } else
  if (nullptr != neopixels_grbw) {
    neopixels_grbw->Begin();
  }
}

void NeoPixelBus_wrapper::show(void) {
  if (nullptr != neopixels_grb) {
    neopixels_grb->Show();
  } else
  if (nullptr != neopixels_grbw) {
    neopixels_grbw->Show();
  }
}

void NeoPixelBus_wrapper::setBrightness(uint8_t b) {
  if (nullptr != neopixels_grb) {
    neopixels_grb->SetBrightness(b);
  } else
  if (nullptr != neopixels_grbw) {
    neopixels_grbw->SetBrightness(b);
  }
}

void NeoPixelBus_wrapper::setPixelColor(uint16_t pxl,
                                        uint8_t  r,
                                        uint8_t  g,
                                        uint8_t  b) {
  if (nullptr != neopixels_grb) {
    neopixels_grb->SetPixelColor(pxl, RgbColor(r, g, b));
  } else
  if (nullptr != neopixels_grbw) {
    neopixels_grbw->SetPixelColor(pxl, RgbwColor(r, g, b));
  }
}

void NeoPixelBus_wrapper::setPixelColor(uint16_t pxl,
                                        uint8_t  r,
                                        uint8_t  g,
                                        uint8_t  b,
                                        uint8_t  w) {
  if (nullptr != neopixels_grb) {
    neopixels_grb->SetPixelColor(pxl, RgbColor(r, g, b));
  } else
  if (nullptr != neopixels_grbw) {
    neopixels_grbw->SetPixelColor(pxl, RgbwColor(r, g, b, w));
  }
}

void NeoPixelBus_wrapper::setPixelColor(uint16_t pxl,
                                        uint32_t c) {
  if (nullptr != neopixels_grb) {
    neopixels_grb->SetPixelColor(pxl, RgbColor((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF)); // Unfold the Color(r,g,b,w) static
  } else
  if (nullptr != neopixels_grbw) {
    neopixels_grbw->SetPixelColor(pxl, RgbwColor((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, (c >> 24) & 0xFF));
  }
}

uint32_t NeoPixelBus_wrapper::getPixelColor(uint16_t n) {
  if (nullptr != neopixels_grb) {
    const RgbColor color = neopixels_grb->GetPixelColor(n);
    return Color(color.R, color.G, color.B);
  } else
  if (nullptr != neopixels_grbw) {
    const RgbwColor color = neopixels_grbw->GetPixelColor(n);
    return Color(color.R, color.G, color.B, color.W);
  }

  return 0u; // Fall-through value...
}

void NeoPixelBus_wrapper::fill(uint32_t c,
                               uint16_t first,
                               uint16_t count) {
  if (nullptr != neopixels_grb) {
    neopixels_grb->ClearTo(RgbColor((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF), first, first + count); // Unfold the Color(r,g,b,w) static
  } else
  if (nullptr != neopixels_grbw) {
    neopixels_grbw->ClearTo(RgbwColor((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, (c >> 24) & 0xFF), first, first + count);
  }
}

#endif // ifndef NEOPIXEL_WRAPPER_USE_ADAFRUIT
#endif // ifndef _NEOPIXELBUS_WRAPPER_CPP
