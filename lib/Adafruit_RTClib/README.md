# RTClib [![Build Status](https://github.com/adafruit/RTClib/workflows/Arduino%20Library%20CI/badge.svg)](https://github.com/adafruit/RTClib/actions)[![Documentation](https://github.com/adafruit/ci-arduino/blob/master/assets/doxygen_badge.svg)](http://adafruit.github.io/RTClib/html/index.html)

This is a fork of JeeLab's fantastic real time clock library for Arduino.

Works great with Adafruit RTC breakouts:

- [DS3231 Precision RTC](https://www.adafruit.com/product/3013)
- [PCF8523 RTC](https://www.adafruit.com/product/3295)
- [DS1307 RTC](https://www.adafruit.com/product/3296)

Please note that dayOfTheWeek() ranges from 0 to 6 inclusive with 0 being 'Sunday'.

<!-- START COMPATIBILITY TABLE -->

## Compatibility

MCU                | Tested Works | Doesn't Work | Not Tested  | Notes
------------------ | :----------: | :----------: | :---------: | -----
Atmega328 @ 16MHz  |      X       |             |            |
Atmega328 @ 12MHz  |      X       |             |            |
Atmega32u4 @ 16MHz |      X       |             |            | Use SDA/SCL on pins D3 &amp; D2
Atmega32u4 @ 8MHz  |      X       |             |            | Use SDA/SCL on pins D3 &amp; D2
ESP8266            |      X       |             |            | SDA/SCL default to pins 4 &amp; 5 but any two pins can be assigned as SDA/SCL using Wire.begin(SDA,SCL)
Atmega2560 @ 16MHz |      X       |             |            | Use SDA/SCL on Pins 20 &amp; 21
ATSAM3X8E          |      X       |             |            | Use SDA1 and SCL1
ATSAM21D           |      X       |             |            |
ATtiny85 @ 16MHz   |      X       |             |            |
ATtiny85 @ 8MHz    |      X       |             |            |
Intel Curie @ 32MHz |             |             |     X       |
STM32F2            |             |             |     X       |

  * ATmega328 @ 16MHz : Arduino UNO, Adafruit Pro Trinket 5V, Adafruit Metro 328, Adafruit Metro Mini
  * ATmega328 @ 12MHz : Adafruit Pro Trinket 3V
  * ATmega32u4 @ 16MHz : Arduino Leonardo, Arduino Micro, Arduino Yun, Teensy 2.0
  * ATmega32u4 @ 8MHz : Adafruit Flora, Bluefruit Micro
  * ESP8266 : Adafruit Huzzah
  * ATmega2560 @ 16MHz : Arduino Mega
  * ATSAM3X8E : Arduino Due
  * ATSAM21D : Arduino Zero, M0 Pro
  * ATtiny85 @ 16MHz : Adafruit Trinket 5V
  * ATtiny85 @ 8MHz : Adafruit Gemma, Arduino Gemma, Adafruit Trinket 3V

<!-- END COMPATIBILITY TABLE -->
Adafruit invests time and resources providing this open source code, please support Adafruit and open-source hardware by purchasing products from Adafruit!

# Dependencies
 * [TinyWireM](https://github.com/adafruit/TinyWireM)

# Contributing

Contributions are welcome! Please read our [Code of Conduct](https://github.com/adafruit/RTClib/blob/master/code-of-conduct.md)
before contributing to help this project stay welcoming.

## Documentation and doxygen
For the detailed API documentation, see https://adafruit.github.io/RTClib/html/index.html
Documentation is produced by doxygen. Contributions should include documentation for any new code added.

Some examples of how to use doxygen can be found in these guide pages:

https://learn.adafruit.com/the-well-automated-arduino-library/doxygen

https://learn.adafruit.com/the-well-automated-arduino-library/doxygen-tips

## Code formatting and clang-format
The code should be formatted according to the [LLVM Coding Standards](https://llvm.org/docs/CodingStandards.html), which is the default of the clang-format tool.  The easiest way to ensure conformance is to [install clang-format](https://llvm.org/builds/) and run

```shell
clang-format -i <source_file>`
```

Written by JeeLabs
MIT license, check license.txt for more information
All text above must be included in any redistribution

To install, use the Arduino Library Manager and search for "RTClib" and install the library.
