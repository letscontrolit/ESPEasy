# NeoPixelBus_wrapper

**NeoPixelBus_wrapper**: A minimal wrapper to replace Adafruit_NeoPixel API to use Makuna's NeoPixelBus API.

(c) 2023..2024, Ton Huisman for [ESPEasy](https://github.com/letscontrolit/ESPEasy).

### How to use

- Add this library to your `lib` folder
- Add the [NeoPixelBus](https://github.com/Makuna/NeoPixelBus) library to your `lib` folder
- Replace the `#include <Adafruit_NeoPixel.h>` line by `#include <NeoPixelBus_wrapper.h>`
- Replace your type `Adafruit_NeoPixel` variable(s) by type `NeoPixelBus_wrapper` variable(s)
- When using runtime instantiation of the wrapper object, replace your `pixels = new Adafruit_NeoPixel(...)` call by `pixels = new NeoPixelBus_wrapper(...)`
- Compile, and presto!

### Limitations

- Currently only supports the most commonly used NeoPixel stripes `NEO_GRB` and `NEO_GRBW`, and the default `NEO_KHZ800` method. (That's all what is used in ESPEasy...)
- When using an ESP8266 and the used GPIO pin is *not* `GPIO2`, you can enable `#  define NEOPIXEL_WRAPPER_USE_ADAFRUIT` in `NeoPixelBus_wrapper.h`, but you'll then be using the Adafruit_NeoPixel library again, as that does allow to select the GPIO pin. The `Adafruit_NeoPixel` library then has to be available for compilation!

### Changelog

- 2024-08-04 tonhuisman: Add support for `fill()` method.
- 2024-01-31 tonhuisman: Add use of Adafruit_NeoPixel library for ESP8266 to restore configurable GPIO pin usage.
- 2023-10-29 tonhuisman: Initial version, wrapper for minimal use. Only what's used from Adafruit_NeoPixel to use NeoPixelBus library

### Support

For questions and improvement requests, please use the Github Issues system.

### Reference

- [NeoPixelBus](https://github.com/Makuna/NeoPixelBus)
- [Adafruit_NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel)