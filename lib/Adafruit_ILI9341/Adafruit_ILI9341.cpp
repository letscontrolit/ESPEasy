/*!
 * @file Adafruit_ILI9341.cpp
 *
 * @mainpage Adafruit ILI9341 TFT Displays
 *
 * @section intro_sec Introduction
 *
 * This is the documentation for Adafruit's ILI9341 driver for the
 * Arduino platform.
 *
 * This library works with the Adafruit 2.8" Touch Shield V2 (SPI)
 *    http://www.adafruit.com/products/1651
 *
 * Adafruit 2.4" TFT LCD with Touchscreen Breakout w/MicroSD Socket - ILI9341
 *    https://www.adafruit.com/product/2478
 *
 * 2.8" TFT LCD with Touchscreen Breakout Board w/MicroSD Socket - ILI9341
 *    https://www.adafruit.com/product/1770
 *
 * 2.2" 18-bit color TFT LCD display with microSD card breakout - ILI9340
 *    https://www.adafruit.com/product/1770
 *
 * TFT FeatherWing - 2.4" 320x240 Touchscreen For All Feathers
 *    https://www.adafruit.com/product/3315
 *
 * These displays use SPI to communicate, 4 or 5 pins are required
 * to interface (RST is optional).
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * @section dependencies Dependencies
 *
 * This library depends on <a href="https://github.com/adafruit/Adafruit_GFX">
 * Adafruit_GFX</a> being present on your system. Please make sure you have
 * installed the latest version before using this library.
 *
 * @section author Author
 *
 * Written by Limor "ladyada" Fried for Adafruit Industries.
 *
 * @section license License
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */

#include "Adafruit_ILI9341.h"
#ifndef ARDUINO_STM32_FEATHER
# include "pins_arduino.h"
# ifndef RASPI
#  include "wiring_private.h"
# endif // ifndef RASPI
#endif  // ifndef ARDUINO_STM32_FEATHER
#include <limits.h>

#if defined(ARDUINO_ARCH_ARC32) || defined(ARDUINO_MAXIM)
# define SPI_DEFAULT_FREQ 16000000

// Teensy 3.0, 3.1/3.2, 3.5, 3.6
#elif defined(__MK20DX128__) || defined(__MK20DX256__) || \
  defined(__MK64FX512__) || defined(__MK66FX1M0__)
# define SPI_DEFAULT_FREQ 40000000
#elif defined(__AVR__) || defined(TEENSYDUINO)
# define SPI_DEFAULT_FREQ 8000000
#elif defined(ESP8266) || defined(ESP32)
# define SPI_DEFAULT_FREQ 40000000
#elif defined(RASPI)
# define SPI_DEFAULT_FREQ 80000000
#elif defined(ARDUINO_ARCH_STM32F1)
# define SPI_DEFAULT_FREQ 36000000
#else // if defined(ARDUINO_ARCH_ARC32) || defined(ARDUINO_MAXIM)
# define SPI_DEFAULT_FREQ 24000000 ///< Default SPI data clock frequency
#endif // if defined(ARDUINO_ARCH_ARC32) || defined(ARDUINO_MAXIM)

#define MADCTL_MY 0x80             ///< Bottom to top
#define MADCTL_MX 0x40             ///< Right to left
#define MADCTL_MV 0x20             ///< Reverse Mode
#define MADCTL_ML 0x10             ///< LCD refresh Bottom to top
#define MADCTL_RGB 0x00            ///< Red-Green-Blue pixel order
#define MADCTL_BGR 0x08            ///< Blue-Green-Red pixel order
#define MADCTL_MH 0x04             ///< LCD refresh right to left
#define MADCTL_SS 0x02             ///< ?? ILI9481 mode
#define MADCTL_GS 0x01             ///< ?? ILI9481 mode

/**************************************************************************/

/*!
    @brief  Instantiate Adafruit ILI9341 driver with software SPI
    @param    cs    Chip select pin #
    @param    dc    Data/Command pin #
    @param    mosi  SPI MOSI pin #
    @param    sclk  SPI Clock pin #
    @param    rst   Reset pin # (optional, pass -1 if unused)
    @param    miso  SPI MISO pin # (optional, pass -1 if unused)
 */

/**************************************************************************/

// Adafruit_ILI9341::Adafruit_ILI9341(int8_t cs, int8_t dc, int8_t mosi,
//                                    int8_t sclk, int8_t rst, int8_t miso)
//     : Adafruit_SPITFT(ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT, cs, dc, mosi, sclk,
//                       rst, miso) {}

/**************************************************************************/

/*!
    @brief  Instantiate Adafruit ILI9341 driver with hardware SPI using the
            default SPI peripheral.
    @param  cs   Chip select pin # (OK to pass -1 if CS tied to GND).
    @param  dc   Data/Command pin # (required).
    @param  rst  Reset pin # (optional, pass -1 if unused).
 */

/**************************************************************************/
Adafruit_ILI9341::Adafruit_ILI9341(int8_t cs, int8_t dc, int8_t rst, uint8_t model, uint16_t w, uint16_t h)
  : Adafruit_SPITFT(w, h, cs, dc, rst) {
  _model = model;
  _w     = w;
  _h     = h;
}

#if !defined(ESP8266)

/**************************************************************************/

/*!
    @brief  Instantiate Adafruit ILI9341 driver with hardware SPI using
            a specific SPI peripheral (not necessarily default).
    @param  spiClass  Pointer to SPI peripheral (e.g. &SPI or &SPI1).
    @param  dc        Data/Command pin # (required).
    @param  cs        Chip select pin # (optional, pass -1 if unused and
                      CS is tied to GND).
    @param  rst       Reset pin # (optional, pass -1 if unused).
 */

/**************************************************************************/

// Adafruit_ILI9341::Adafruit_ILI9341(SPIClass *spiClass, int8_t dc, int8_t cs,
//                                    int8_t rst)
//     : Adafruit_SPITFT(ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT, spiClass, cs, dc,
//                       rst) {}
#endif // end !ESP8266

/**************************************************************************/

/*!
    @brief  Instantiate Adafruit ILI9341 driver using parallel interface.
    @param  busWidth  If tft16 (enumeration in Adafruit_SPITFT.h), is a
                      16-bit interface, else 8-bit.
    @param  d0        Data pin 0 (MUST be a byte- or word-aligned LSB of a
                      PORT register -- pins 1-n are extrapolated from this).
    @param  wr        Write strobe pin # (required).
    @param  dc        Data/Command pin # (required).
    @param  cs        Chip select pin # (optional, pass -1 if unused and CS
                      is tied to GND).
    @param  rst       Reset pin # (optional, pass -1 if unused).
    @param  rd        Read strobe pin # (optional, pass -1 if unused).
 */

/**************************************************************************/

// Adafruit_ILI9341::Adafruit_ILI9341(tftBusWidth busWidth, int8_t d0, int8_t wr,
//                                    int8_t dc, int8_t cs, int8_t rst, int8_t rd)
//     : Adafruit_SPITFT(ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT, busWidth, d0, wr, dc,
//                       cs, rst, rd) {}

// clang-format off
static const uint8_t PROGMEM initcmd[] = { // ILI9341 & ILI9340
  0xEF,             3,                0x03,                0x80,                0x02,
  0xCF,             3,                0x00,                0xC1,                0x30,
  0xED,             4,                0x64,                0x03,                0x12,               0x81,
  0xE8,             3,                0x85,                0x00,                0x78,
  0xCB,             5,                0x39,                0x2C,                0x00,               0x34,               0x02,
  0xF7,             1,                0x20,
  0xEA,             2,                0x00,                0x00,
  ILI9341_PWCTR1,   1,                0x23,                                           // Power control VRH[5:0]
  ILI9341_PWCTR2,   1,                0x10,                                           // Power control SAP[2:0];BT[3:0]
  ILI9341_VMCTR1,   2,                0x3e,                0x28,                      // VCM control
  ILI9341_VMCTR2,   1,                0x86,                                           // VCM control2
  ILI9341_MADCTL,   1,                (MADCTL_MX | MADCTL_BGR),                       // Memory Access Control
  ILI9341_VSCRSADD, 1,                0x00,                                           // Vertical scroll zero
  ILI9341_PIXFMT,   1,                0x55,
  ILI9341_FRMCTR1,  2,                0x00,                0x18,
  ILI9341_DFUNCTR,  3,                0x08,                0x82,                0x27, // Display Function Control
  0xF2,             1,                0x00,                                           // 3Gamma Function Disable
  ILI9341_GAMMASET, 1,                0x01,                                           // Gamma curve selected
  ILI9341_GMCTRP1,  15,               0x0F,                0x31,                0x2B,               0x0C,               0x0E,
  0x08,
  0x4E,             0xF1,             0x37,                0x07,                0x10,               0x03,               0x0E,
  0x09,             0x00, // Set Gamma
  ILI9341_GMCTRN1,  15,               0x00,                0x0E,                0x14,               0x03,               0x11,
  0x07,
  0x31,             0xC1,             0x48,                0x08,                0x0F,               0x0C,               0x31,
  0x36,             0x0F, // Set Gamma
  ILI9341_SLPOUT,   0x80, // Exit Sleep
  ILI9341_DISPON,   0x80, // Display on
  0x00                    // End of list
};

// clang-format on

// clang-format off
static const uint8_t PROGMEM initcmd_9342[] = { // ILI9342
  0xEF,             3,                0x03,                0x80,                0x02,
  0xCF,             3,                0x00,                0xC1,                0x30,
  0xED,             4,                0x64,                0x03,                0x12,               0x81,
  0xE8,             3,                0x85,                0x00,                0x78,
  0xCB,             5,                0x39,                0x2C,                0x00,               0x34,               0x02,
  0xF7,             1,                0x20,
  0xEA,             2,                0x00,                0x00,
  ILI9341_PWCTR1,   1,                0x23,                                           // Power control VRH[5:0]
  ILI9341_PWCTR2,   1,                0x10,                                           // Power control SAP[2:0];BT[3:0]
  ILI9341_VMCTR1,   2,                0x3e,                0x28,                      // VCM control
  ILI9341_VMCTR2,   1,                0x86,                                           // VCM control2
  ILI9341_MADCTL,   1,                (MADCTL_MY | MADCTL_MV | MADCTL_BGR),           // Memory Access Control
  ILI9341_VSCRSADD, 1,                0x00,                                           // Vertical scroll zero
  ILI9341_PIXFMT,   1,                0x55,
  ILI9341_FRMCTR1,  2,                0x00,                0x18,
  ILI9341_DFUNCTR,  3,                0x08,                0x82,                0x27, // Display Function Control
  0xF2,             1,                0x00,                                           // 3Gamma Function Disable
  ILI9341_GAMMASET, 1,                0x01,                                           // Gamma curve selected
  ILI9341_GMCTRP1,  15,               0x0F,                0x31,                0x2B,               0x0C,               0x0E,
  0x08,
  0x4E,             0xF1,             0x37,                0x07,                0x10,               0x03,               0x0E,
  0x09,             0x00, //
                          // Set
                          // Gamma
  ILI9341_GMCTRN1,  15,               0x00,                0x0E,                0x14,               0x03,               0x11,
  0x07,
  0x31,             0xC1,             0x48,                0x08,                0x0F,               0x0C,               0x31,
  0x36,             0x0F, // Set Gamma
  ILI9341_SLPOUT,   0x80, // Exit Sleep
  ILI9341_DISPON,   0x80, // Display on
  0x00                    // End of list
};

// clang-format on

// clang-format off
static const uint8_t PROGMEM initcmd_9481[] = { // ILI9481
  ILI9341_SLPOUT, 0x80,                         // Exit Sleep
  0xD0,           3,              0x07,              0x42,              0x18,
  0xD1,           3,              0x00,              0x07,              0x10,
  0xD2,           2,              0x01,              0x02,
  ILI9341_PWCTR1, 5,              0x10,              0x3B,              0x00,             0x02,              0x11, // Power control VRH[5:0]
  ILI9341_VMCTR1, 1,              0x03,                                                                            // VCM control
  0xC8,           12,             0x00,              0x32,              0x36,             0x45,              0x06,   0x16,   0x37,
  0x75,           0x77,           0x54,              0x0C,              0x00,
  ILI9341_MADCTL, 1,              0x0A,                                                                            // Memory Access Control
  ILI9341_PIXFMT, 1,              0x55,                                                                            // Pixel format
                                                                                                                   // 0x55=16bit, 0x66=18bit
  ILI9341_INVON,  0,
  ILI9341_CASET,  4,              0x00,              0x00,              0x01,             0x3F,
  ILI9341_PASET,  0x84,           0x00,              0x00,              0x01,             0xDF,
  ILI9341_DISPON, 0x80, // Display on
  0x00                  // End of list
};

// clang-format on

// clang-format off
static const uint8_t PROGMEM initcmd_9481_CPT29[] = { // ILI9481 CPT29 variation
  ILI9341_SLPOUT, 0x80,                               // Exit Sleep
  0xD0,           3,              0x07,              0x41,              0x1D,
  0xD1,           3,              0x00,              0x2b,              0x1f,
  0xD2,           2,              0x01,              0x11,
  ILI9341_PWCTR1, 5,              0x10,              0x3B,              0x00,             0x02,              0x11, // Power control VRH[5:0]
  ILI9341_VMCTR1, 1,              0x03,                                                                            // VCM control
  0xC8,           12,             0x00,              0x14,              0x33,             0x10,              0x00,   0x16,   0x44,
  0x36,           0x77,           0x00,              0x0F,              0x00,
  0xB0,           1,              0x00,
  0xE4,           1,              0xA0,
  0xF0,           1,              0x01,
  0xF3,           2,              0x02,              0x1A,
  ILI9341_MADCTL, 1,              0x0A, // Memory Access Control
  ILI9341_PIXFMT, 1,              0x55, // Pixel format 0x55=16bit,
                                        // 0x66=18bit
  ILI9341_INVON,  0,
  ILI9341_CASET,  4,              0x00,              0x00,              0x01,             0x3F,
  ILI9341_PASET,  0x84,           0x00,              0x00,              0x01,             0xDF,
  ILI9341_DISPON, 0x80, // Display on
  0x00                  // End of list
};

// clang-format on

// clang-format off
static const uint8_t PROGMEM initcmd_9481_PVI35[] = { // ILI9481 PVI35 variation
  ILI9341_SLPOUT, 0x80,                               // Exit Sleep
  0xD0,           3,              0x07,              0x41,              0x1D,
  0xD1,           3,              0x00,              0x2b,              0x1f,
  0xD2,           2,              0x01,              0x11,
  ILI9341_PWCTR1, 5,              0x10,              0x3B,              0x00,             0x02,              0x11, // Power control VRH[5:0]
  ILI9341_VMCTR1, 1,              0x03,                                                                            // VCM control
  0xC8,           12,             0x00,              0x14,              0x33,             0x10,              0x00,   0x16,   0x44,
  0x36,           0x77,           0x00,              0x0F,              0x00,
  0xB0,           1,              0x00,
  0xE4,           1,              0xA0,
  0xF0,           1,              0x01,
  0xF3,           2,              0x40,              0x0A,
  ILI9341_MADCTL, 1,              0x0A, // Memory Access Control
  ILI9341_PIXFMT, 1,              0x55, // Pixel format 0x55=16bit,
                                        // 0x66=18bit
  ILI9341_INVON,  0,
  ILI9341_CASET,  4,              0x00,              0x00,              0x01,             0x3F,
  ILI9341_PASET,  0x84,           0x00,              0x00,              0x01,             0xDF,
  ILI9341_DISPON, 0x80, // Display on
  0x00                  // End of list
};

// clang-format on

// clang-format off
static const uint8_t PROGMEM initcmd_9481_AUO317[] = { // ILI9481 AUO317 variation
  ILI9341_SLPOUT, 0x80,                                // Exit Sleep
  0xD0,           3,              0x07,              0x40,              0x1D,
  0xD1,           3,              0x00,              0x18,              0x13,
  0xD2,           2,              0x01,              0x11,
  ILI9341_PWCTR1, 5,              0x10,              0x3B,              0x00,             0x02,              0x11, // Power control VRH[5:0]
  ILI9341_VMCTR1, 1,              0x03,                                                                            // VCM control
  0xC8,           12,             0x00,              0x44,              0x06,             0x44,              0x0A,   0x08,   0x17,
  0x33,           0x77,           0x44,              0x08,              0x0C,
  0xB0,           1,              0x00,
  0xE4,           1,              0xA0,
  0xF0,           1,              0x01,
  ILI9341_MADCTL, 1,              0x0A, // Memory Access Control
  ILI9341_PIXFMT, 1,              0x55, // Pixel format 0x55=16bit,
                                        // 0x66=18bit
  ILI9341_INVON,  0,
  ILI9341_CASET,  4,              0x00,              0x00,              0x01,             0x3F,
  ILI9341_PASET,  0x84,           0x00,              0x00,              0x01,             0xDF,
  ILI9341_DISPON, 0x80, // Display on
  0x00                  // End of list
};

// clang-format on

// clang-format off
static const uint8_t PROGMEM initcmd_9481_CM35[] = { // ILI9481 CMO36 variation
  ILI9341_SLPOUT, 0x80,                              // Exit Sleep
  0xD0,           3,              0x07,              0x41,              0x1D,
  0xD1,           3,              0x00,              0x1C,              0x1F,
  0xD2,           2,              0x01,              0x11,
  ILI9341_PWCTR1, 5,              0x10,              0x3B,              0x00,             0x02,              0x11, // Power control VRH[5:0]
  ILI9341_VMCTR1, 1,              0x03,                                                                            // VCM control
  0xC6,           1,              0x83,
  0xC8,           12,             0x00,              0x26,              0x21,             0x00,              0x00,   0x1F,   0x65,
  0x23,           0x77,           0x00,              0x0F,              0x00,
  0xB0,           1,              0x00,
  0xE4,           1,              0xA0,
  0xF0,           1,              0x01,
  ILI9341_MADCTL, 1,              0x0A, // Memory Access Control
  ILI9341_PIXFMT, 1,              0x55, // Pixel format 0x55=16bit, 0x66=18bit
  ILI9341_INVON,  0,
  ILI9341_CASET,  4,              0x00,              0x00,              0x01,             0x3F,
  ILI9341_PASET,  0x84,           0x00,              0x00,              0x01,             0xDF,
  ILI9341_DISPON, 0x80, // Display on
  0x00                  // End of list
};

// clang-format on

// clang-format off
static const uint8_t PROGMEM initcmd_9481_RGB[] = { // ILI9481 RGB variation
  ILI9341_SLPOUT,   0x80,                           // Exit Sleep
  0xD0,             3,                0x07,                0x41,                0x1D,
  0xD1,             3,                0x00,                0x2B,                0x1F,
  0xD2,             2,                0x01,                0x11,
  ILI9341_PWCTR1,   6,                0x10,                0x3B,                0x00,               0x02,                0x11,
  0x00,                                     // Power control VRH[5:0]
  ILI9341_VMCTR1,   1,                0x03, // VCM control
  0xC6,             1,                0x80,
  0xC8,             12,               0x00,                0x14,                0x33,               0x10,                0x00,
  0x16,
  0x44,
  0x36,             0x77,             0x00,                0x0F,                0x00,
  0xB0,             1,                0x00,
  0xE4,             1,                0xA0,
  0xF0,             1,                0x08,
  0xF3,             2,                0x40,                0x0A,
  0xF6,             1,                0x84,
  0xF7,             1,                0x80,
  ILI9341_FRMCTR3,  4,                0x00,                0x01,                0x06,               0x30,
  ILI9341_INVCTR,   1,                0x00,
  ILI9341_RDPIXFMT, 2,                0x00,                0x55,
  ILI9341_MADCTL,   1,                0x0A, // Memory Access Control
  ILI9341_PIXFMT,   1,                0x55, // Pixel format 0x55=16bit, 0x66=18bit
  ILI9341_INVON,    0,
  ILI9341_CASET,    4,                0x00,                0x00,                0x01,               0x3F,
  ILI9341_PASET,    0x84,             0x00,                0x00,                0x01,               0xDF,
  ILI9341_DISPON,   0x80, // Display on
  0x00                    // End of list
};

// clang-format on

// clang-format off
static const uint8_t PROGMEM initcmd_9481_CMI7[] = { // ILI9481 CMI7 (TFT_eSPI INIT_7) variation
  ILI9341_SLPOUT, 0x80,                              // Exit Sleep
  0xD0,           3,              0x07,              0x42,              0x1B,
  0xD1,           3,              0x00,              0x14,              0x1B,
  0xD2,           2,              0x01,              0x12,
  ILI9341_PWCTR1, 5,              0x10,              0x3B,              0x00,             0x02,              0x01, // Power control VRH[5:0]
  ILI9341_VMCTR1, 1,              0x03,                                                                            // VCM control
  0xC8,           12,             0x00,              0x46,              0x44,             0x50,              0x04,   0x16,   0x33,
  0x13,           0x77,           0x05,              0x0F,              0x00,
  ILI9341_MADCTL, 1,              0x0A,                                                                            // Memory Access Control
  ILI9341_PIXFMT, 1,              0x55,                                                                            // Pixel format
                                                                                                                   // 0x55=16bit,
                                                                                                                   // 0x66=18bit
  ILI9341_INVON,  0,
  0x22,           4,              0x00,              0x00,              0x01,             0x3F,
  ILI9341_PASET,  0x84,           0x00,              0x00,              0x01,             0xE0,
  ILI9341_DISPON, 0x80, // Display on
  0x00                  // End of list
};

// clang-format on

// clang-format off
static const uint8_t PROGMEM initcmd_9481_CMI8[] = { // ILI9481 CMI8 (TFT_eSPI INIT_8) variation
  ILI9341_SLPOUT, 0x80,                              // Exit Sleep
  0xD0,           0x83,           0x07,              0x44,              0x1E,
  0xD1,           3,              0x00,              0x0C,              0x1A,
  ILI9341_VMCTR1, 1,              0x03,              // VCM control
  0xD2,           2,              0x01,              0x11,
  0xE4,           1,              0xA0,
  0xF3,           2,              0x00,              0x2A,
  0xC8,           12,             0x00,              0x26,              0x21,             0x00,              0x00,              0x1F,
  0x65,
  0x23,           0x77,           0x00,              0x0F,              0x00,
  ILI9341_PWCTR1, 5,              0x00,              0x3B,              0x00,             0x02,              0x11, // Power control VRH[5:0]
  0xC6,           1,              0x83,
  0xF0,           1,              0x01,
  0xE4,           1,              0xA0,
  ILI9341_MADCTL, 1,              0x0A, // Memory Access Control
  ILI9341_PIXFMT, 1,              0x55, // Pixel format 0x55=16bit,
                                        // 0x66=18bit
  ILI9341_INVCTR, 0x84,           0x02,              0x00,              0x00,             0x01,
  ILI9341_CASET,  4,              0x00,              0x00,              0x01,             0x3F,
  ILI9341_PASET,  4,              0x00,              0x00,              0x01,             0xDF,
  ILI9341_DISPON, 0x80, // Display on
  ILI9341_RAMWR,  0,
  0x00                  // End of list
};

// clang-format on

#ifdef ILI9341_ENABLE_ILI948X

// clang-format off
static const uint8_t PROGMEM initcmd_9486[] = {   // ILI9486
  ILI9341_SLPOUT,  0x80,                          // Exit Sleep
  ILI9341_PIXFMT,  1,  0x55,                      // Pixel format 0x55=16bit, 0x66=18bit
  ILI9341_PWCTR3,  1,  0x44,                      // Power control3
  ILI9341_VMCTR1,  4,  0x00,  0x00,  0x00,  0x00, // VCM control
  ILI9341_GMCTRP1, 15, 0x0F,  0x1F,  0x1c,  0x0C, 0x0F, 0x08, 0x48, 0x98, 0x37, 0x0A, 0x13, 0x04, 0x11, 0x0D, 0x00,
  ILI9341_GMCTRN1, 15, 0x0F,  0x32,  0x2E,  0x0B, 0x0D, 0x05, 0x47, 0x75, 0x37, 0x06, 0x10, 0x03, 0x24, 0x20, 0x00,
  ILI9341_INVOFF,  0,
  ILI9341_MADCTL,  1,  0x48, // Memory Access Control
  ILI9341_DISPON,  0x80,     // Display on
  0x00                       // End of list
};

// clang-format on

// clang-format off
static const uint8_t PROGMEM initcmd_9488[] = { // ILI9488
  // Set gamma
  ILI9341_GMCTRP1, 15,              0x00,               0x03,               0x09,               0x08,               0x16,               0x0A,
  0x3F,            0x78,            0x4C,               0x09,               0x0A,               0x08,               0x16,               0x1A,
  0x0F,

  // Set gamma
  ILI9341_GMCTRN1, 15,              0x00,               0x16,               0x19,               0x03,               0x0F,               0x05,
  0x32,            0x45,            0x46,               0x04,               0x0E,               0x0D,               0x35,               0x37,
  0x0F,

  // Power control  VRH[5:0]
  ILI9341_PWCTR1,  2,               0x17,               0x15,

  // Power control SAP[2:0];BT[3:0]
  ILI9341_PWCTR2,  1,               0x41,

  // VCM control
  ILI9341_VMCTR1,  3,               0x00,               0x12,               0x80,

  // Memory access Control
  ILI9341_MADCTL,  1,               0x48,

  // Pixel format  0x55=16bit, 0x66=18bit
  ILI9341_PIXFMT,  1,               0x55,

  // Interface control Mode
  0xB0,            1,               0x80,

  // Frame rate
  ILI9341_FRMCTR1, 1,               0xA0,

  // Display onversion control
  ILI9341_INVCTR,  1,               0x02,

  // Display function control
  ILI9341_DFUNCTR, 2,               0x02,               0x02,

  // Disable 24 bit data
  0xE9,            1,               0x00,

  // Adjust control
  0xF7,            4,               0xA9,               0x51,               0x2C,               0x82,

  // Exit sleep
  ILI9341_SLPOUT,  0x80,

  // Display on
  ILI9341_DISPON,  0x80,

  // End of list
  0x00
};
#endif // ifdef ILI9341_ENABLE_ILI948X

// clang-format on

/**************************************************************************/

/*!
    @brief   Initialize ILI9341 chip
    Connects to the ILI9341 over SPI and sends initialization procedure commands
    @param    freq  Desired SPI clock frequency
 */

/**************************************************************************/
void Adafruit_ILI9341::begin(uint32_t freq) {
  if (!freq) {
    freq = SPI_DEFAULT_FREQ;
  }
  initSPI(freq);

  if (_rst < 0) {                 // If no hardware reset pin...
    sendCommand(ILI9341_SWRESET); // Engage software reset
    delay(150);
  }

  uint8_t cmd, x, numArgs;
  const uint8_t *addr;         // = initcmd;

  switch (_model) {
    case ILI_TYPE_9341:        // ILI9341
      addr = initcmd;
      break;
    case ILI_TYPE_9342:        // ILI9342 M5STACK
      addr = initcmd_9342;
      break;
    case ILI_TYPE_9481:        // ILI9481
      addr = initcmd_9481;
      break;
    case ILI_TYPE_9481_CPT29:  // ILI9481 CPT29
      addr = initcmd_9481_CPT29;
      break;
    case ILI_TYPE_9481_PVI35:  // ILI9481 PVI35
      addr = initcmd_9481_PVI35;
      break;
    case ILI_TYPE_9481_AUO317: // ILI9481 AUO317
      addr = initcmd_9481_AUO317;
      break;
    case ILI_TYPE_9481_CMO35:  // ILI9481 CMO35
      addr = initcmd_9481_CM35;
      break;
    case ILI_TYPE_9481_RGB:    // ILI9481 RGB
      addr = initcmd_9481_RGB;
      break;
    case ILI_TYPE_9481_CMI7:   // ILI9481 CMI7
      addr = initcmd_9481_CMI7;
      break;
    case ILI_TYPE_9481_CMI8:   // ILI9481 CMI8
      addr = initcmd_9481_CMI8;
      break;
    #ifdef ILI9341_ENABLE_ILI948X
    case ILI_TYPE_9486: // ILI9486
      addr = initcmd_9486;
      break;
    case ILI_TYPE_9488: // ILI9488
      addr = initcmd_9488;
      break;
    #endif // ifdef ILI9341_ENABLE_ILI948X
    default:
      addr = initcmd;
      break;
  }

  while ((cmd = pgm_read_byte(addr++)) > 0) {
    x       = pgm_read_byte(addr++);
    numArgs = x & 0x7F;
    sendCommand(cmd, addr, numArgs);
    addr += numArgs;

    if (x & 0x80) {
      delay(150);
    }
  }

  _width  = _w;
  _height = _h;
}

/**************************************************************************/

/*!
    @brief   Set origin of (0,0) and orientation of TFT display
    @param   m  The index for rotation, from 0-3 inclusive
 */

/**************************************************************************/
void Adafruit_ILI9341::setRotation(uint8_t m) {
  rotation = m % 4; // can't be higher than 3

  switch (rotation) {
    case 0:

      switch (_model) {
        case ILI_TYPE_9342:        // ILI9342 M5STACK
          m = (MADCTL_MY | MADCTL_MV | MADCTL_BGR);
          break;
        case ILI_TYPE_9481:        // ILI9481
        case ILI_TYPE_9481_CPT29:  // ILI9481 CPT29
        case ILI_TYPE_9481_PVI35:  // ILI9481 PVI35
        case ILI_TYPE_9481_AUO317: // ILI9481 AUO317
        case ILI_TYPE_9481_CMO35:  // ILI9481 CMO35
        case ILI_TYPE_9481_RGB:    // ILI9481 RGB
        case ILI_TYPE_9481_CMI7:   // ILI9481 CMI7
        case ILI_TYPE_9481_CMI8:   // ILI9481 CMI8
          m = (MADCTL_SS | MADCTL_BGR);
          break;
        case ILI_TYPE_9341:        // ILI9341
          // m = (MADCTL_MX | MADCTL_BGR);
          // break;
        #ifdef ILI9341_ENABLE_ILI948X
        case ILI_TYPE_9486: // ILI9486
        // m = (MADCTL_MX | MADCTL_BGR);
        // break;
        case ILI_TYPE_9488: // ILI9488
          // m = (MADCTL_MX | MADCTL_BGR);
          // break;
        #endif // ifdef ILI9341_ENABLE_ILI948X
        default:
          m = (MADCTL_MX | MADCTL_BGR);
          break;
      }
      _width  = _w;
      _height = _h;
      break;
    case 1:

      switch (_model) {
        case ILI_TYPE_9342:        // ILI9342 M5STACK
          m = (MADCTL_BGR);
          break;
        case ILI_TYPE_9481:        // ILI9481
        case ILI_TYPE_9481_CPT29:  // ILI9481 CPT29
        case ILI_TYPE_9481_PVI35:  // ILI9481 PVI35
        case ILI_TYPE_9481_AUO317: // ILI9481 AUO317
        case ILI_TYPE_9481_CMO35:  // ILI9481 CMO35
        case ILI_TYPE_9481_RGB:    // ILI9481 RGB
        case ILI_TYPE_9481_CMI7:   // ILI9481 CMI7
        case ILI_TYPE_9481_CMI8:   // ILI9481 CMI8
        //  m = (MADCTL_MV | MADCTL_BGR);
        //  break;
        case ILI_TYPE_9341:        // ILI9341
          // m = (MADCTL_MV | MADCTL_BGR);
          // break;
        #ifdef ILI9341_ENABLE_ILI948X
        case ILI_TYPE_9486: // ILI9486
        // m = (MADCTL_MV | MADCTL_BGR);
        // break;
        case ILI_TYPE_9488: // ILI9488
          // m = (MADCTL_MV | MADCTL_BGR);
          // break;
        #endif // ifdef ILI9341_ENABLE_ILI948X
        default:
          m = (MADCTL_MV | MADCTL_BGR);
          break;
      }
      _width  = _h;
      _height = _w;
      break;
    case 2:

      switch (_model) {
        case ILI_TYPE_9342:        // ILI9342 M5STACK
          m = (MADCTL_MV | MADCTL_MX | MADCTL_BGR);
          break;
        case ILI_TYPE_9481:        // ILI9481
        case ILI_TYPE_9481_CPT29:  // ILI9481 CPT29
        case ILI_TYPE_9481_PVI35:  // ILI9481 PVI35
        case ILI_TYPE_9481_AUO317: // ILI9481 AUO317
        case ILI_TYPE_9481_CMO35:  // ILI9481 CMO35
        case ILI_TYPE_9481_RGB:    // ILI9481 RGB
        case ILI_TYPE_9481_CMI7:   // ILI9481 CMI7
        case ILI_TYPE_9481_CMI8:   // ILI9481 CMI8
          m = (MADCTL_GS | MADCTL_BGR);
          break;
        case ILI_TYPE_9341:        // ILI9341
          // m = (MADCTL_MY | MADCTL_BGR);
          // break;
        #ifdef ILI9341_ENABLE_ILI948X
        case ILI_TYPE_9486: // ILI9486
        // m = (MADCTL_MY | MADCTL_BGR);
        // break;
        case ILI_TYPE_9488: // ILI9488
          // m = (MADCTL_MY | MADCTL_BGR);
          // break;
        #endif // ifdef ILI9341_ENABLE_ILI948X
        default:
          m = (MADCTL_MY | MADCTL_BGR);
          break;
      }
      _width  = _w;
      _height = _h;
      break;
    case 3:

      switch (_model) {
        case ILI_TYPE_9342:        // ILI9342 M5STACK
          m = (MADCTL_MX | MADCTL_MY | MADCTL_BGR);
          break;
        case ILI_TYPE_9481:        // ILI9481
        case ILI_TYPE_9481_CPT29:  // ILI9481 CPT29
        case ILI_TYPE_9481_PVI35:  // ILI9481 PVI35
        case ILI_TYPE_9481_AUO317: // ILI9481 AUO317
        case ILI_TYPE_9481_CMO35:  // ILI9481 CMO35
        case ILI_TYPE_9481_RGB:    // ILI9481 RGB
        case ILI_TYPE_9481_CMI7:   // ILI9481 CMI7
        case ILI_TYPE_9481_CMI8:   // ILI9481 CMI8
          m = (MADCTL_SS | MADCTL_GS | MADCTL_MV | MADCTL_BGR);
          break;
        case ILI_TYPE_9341:        // ILI9341
          // m = (MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
          // break;
        #ifdef ILI9341_ENABLE_ILI948X
        case ILI_TYPE_9486: // ILI9486
        // m = (MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
        // break;
        case ILI_TYPE_9488: // ILI9488
          // m = (MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
          // break;
        #endif // ifdef ILI9341_ENABLE_ILI948X
        default:
          m = (MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
          break;
      }
      _width  = _h;
      _height = _w;
      break;
  }

  sendCommand(ILI9341_MADCTL, &m, 1);
}

/**************************************************************************/

/*!
    @brief   Enable/Disable display color inversion
    @param   invert True to invert, False to have normal color
 */

/**************************************************************************/
void Adafruit_ILI9341::invertDisplay(bool invert) {
  sendCommand(invert ? ILI9341_INVON : ILI9341_INVOFF);
}

/**************************************************************************/

/*!
    @brief   Scroll display memory
    @param   y How many pixels to scroll display by
 */

/**************************************************************************/
void Adafruit_ILI9341::scrollTo(uint16_t y) {
  uint8_t data[2];

  data[0] = y >> 8;
  data[1] = y & 0xff;
  sendCommand(ILI9341_VSCRSADD, (uint8_t *)data, 2);
}

/**************************************************************************/

/*!
    @brief   Set the height of the Top and Bottom Scroll Margins
    @param   top The height of the Top scroll margin
    @param   bottom The height of the Bottom scroll margin
 */

/**************************************************************************/
void Adafruit_ILI9341::setScrollMargins(uint16_t top, uint16_t bottom) {
  // TFA+VSA+BFA must equal 320
  if (top + bottom <= ILI9341_TFTHEIGHT) {
    uint16_t middle = ILI9341_TFTHEIGHT - (top + bottom);
    uint8_t  data[6];
    data[0] = top >> 8;
    data[1] = top & 0xff;
    data[2] = middle >> 8;
    data[3] = middle & 0xff;
    data[4] = bottom >> 8;
    data[5] = bottom & 0xff;
    sendCommand(ILI9341_VSCRDEF, (uint8_t *)data, 6);
  }
}

/**************************************************************************/

/*!
    @brief   Set the "address window" - the rectangle we will write to RAM with
   the next chunk of      SPI data writes. The ILI9341 will automatically wrap
   the data as each row is filled
    @param   x1  TFT memory 'x' origin
    @param   y1  TFT memory 'y' origin
    @param   w   Width of rectangle
    @param   h   Height of rectangle
 */

/**************************************************************************/
void Adafruit_ILI9341::setAddrWindow(uint16_t x1, uint16_t y1, uint16_t w,
                                     uint16_t h) {
  uint16_t x2 = (x1 + w - 1), y2 = (y1 + h - 1);

  writeCommand(ILI9341_CASET); // Column address set
  SPI_WRITE16(x1);
  SPI_WRITE16(x2);
  writeCommand(ILI9341_PASET); // Row address set
  SPI_WRITE16(y1);
  SPI_WRITE16(y2);
  writeCommand(ILI9341_RAMWR); // Write to RAM
}

/**************************************************************************/

/*!
    @brief  Read 8 bits of data from ILI9341 configuration memory. NOT from RAM!
            This is highly undocumented/supported, it's really a hack but kinda
   works?
    @param    commandByte  The command register to read data from
    @param    index  The byte index into the command to read from
    @return   Unsigned 8-bit data read from ILI9341 register
 */

/**************************************************************************/
uint8_t Adafruit_ILI9341::readcommand8(uint8_t commandByte, uint8_t index) {
  uint8_t data = 0x10 + index;

  sendCommand(0xD9, &data, 1); // Set Index Register
  return Adafruit_SPITFT::readcommand8(commandByte);
}
