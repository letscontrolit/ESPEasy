#ifndef _HELPERS_NEOPIXELBUS_WRAPPER_H
#define _HELPERS_NEOPIXELBUS_WRAPPER_H

#ifdef ESP8266
# ifndef NEOPIXEL_WRAPPER_USE_ADAFRUIT

#  define NEOPIXEL_WRAPPER_USE_ADAFRUIT // Enable this line to use on ESP8266 with configurable GPIO using Adafruit_NeoPixel
# endif // ifndef NEOPIXEL_WRAPPER_USE_ADAFRUIT
#endif // ifdef ESP8266

#if defined(ESP32) && defined(NEOPIXEL_WRAPPER_USE_ADAFRUIT)
# undef NEOPIXEL_WRAPPER_USE_ADAFRUIT // Shouldn't (or can't) be used on newer ESP32 chip types like -C2, -C3 and -C6
#endif // if defined(ESP32) && defined(NEOPIXEL_WRAPPER_USE_ADAFRUIT)

#ifdef NEOPIXEL_WRAPPER_USE_ADAFRUIT
# include <Adafruit_NeoPixel.h>
#else // ifdef NEOPIXEL_WRAPPER_USE_ADAFRUIT
# include <NeoPixelBus.h>
# include <NeoPixelBrightnessBus.h> // Be sure to keep this header file when upgrading the NeoPixelBus library,
                                    // and remove the deprecation warning if needed

// Some stuff from Adafruit_NeoPixel.h used in plugins
# ifndef NEO_GRB
#  define NEO_GRB ((1 << 6) | (1 << 4) | (0 << 2) | (2))  ///< Transmit as G,R,B
#  define NEO_GRBW ((3 << 6) | (1 << 4) | (0 << 2) | (2)) ///< Transmit as G,R,B,W
#  define NEO_KHZ800 0x0000                               ///< 800 KHz data transmission
typedef uint16_t neoPixelType;                            ///< 3rd arg to Adafruit_NeoPixel constructor
# endif // ifndef NEO_GRB


# define NEOPIXEL_LIB NeoPixelBrightnessBus   // Neopixel library type
# if defined(ESP32)
#  define METHOD NeoWs2812xMethod             // Automatic method, user selected pin
# endif // if defined(ESP32)
# if defined(ESP8266)
#  define METHOD NeoEsp8266Uart1800KbpsMethod // GPIO2 - use NeoEsp8266Uart0800KbpsMethod for GPIO1(TX)
# endif // if defined(ESP8266)

#endif // ifndef NEOPIXEL_WRAPPER_USE_ADAFRUIT

struct NeoPixelBus_wrapper
#ifdef NEOPIXEL_WRAPPER_USE_ADAFRUIT
  : Adafruit_NeoPixel
#endif // ifdef NEOPIXEL_WRAPPER_USE_ADAFRUIT
{
public:

  NeoPixelBus_wrapper(uint16_t     _maxPixels,
                      int16_t      _gpioPin,
                      neoPixelType _stripType);
  virtual ~NeoPixelBus_wrapper();
  #ifndef NEOPIXEL_WRAPPER_USE_ADAFRUIT
  void begin();
  void show(void);
  void setBrightness(uint8_t);
  void setPixelColor(uint16_t pxl,
                     uint8_t  r,
                     uint8_t  g,
                     uint8_t  b);
  void setPixelColor(uint16_t pxl,
                     uint8_t  r,
                     uint8_t  g,
                     uint8_t  b,
                     uint8_t  w);
  void     setPixelColor(uint16_t pxl,
                         uint32_t c);
  uint32_t getPixelColor(uint16_t n);
  uint16_t numPixels(void) const {
    return numLEDs;
  }

  void fill(uint32_t c     = 0,
            uint16_t first = 0,
            uint16_t count = 0);

  static uint32_t Color(uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0) {
    return static_cast<uint32_t>(w) << 24 |
           static_cast<uint32_t>(r) << 16 |
           static_cast<uint32_t>(g) << 8 |
           static_cast<uint32_t>(b);
  }

private:

  NEOPIXEL_LIB<NeoGrbFeature, METHOD>  *neopixels_grb  = nullptr;
  NEOPIXEL_LIB<NeoGrbwFeature, METHOD> *neopixels_grbw = nullptr;
  uint16_t                              numLEDs        = 0;
  #endif // ifndef NEOPIXEL_WRAPPER_USE_ADAFRUIT
};

#endif // ifndef _HELPERS_NEOPIXELBUS_WRAPPER_H
