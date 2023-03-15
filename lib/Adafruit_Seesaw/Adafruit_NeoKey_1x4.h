#ifndef _NEOKEY_1X4_H
#define _NEOKEY_1X4_H

#include "Adafruit_seesaw.h"
#include "seesaw_neopixel.h"
#include <Arduino.h>

#define NEOKEY_1X4_ADDR 0x30

#define NEOKEY_1X4_NEOPIN 3
#define NEOKEY_1X4_BUTTONA 4
#define NEOKEY_1X4_BUTTONB 5
#define NEOKEY_1X4_BUTTONC 6
#define NEOKEY_1X4_BUTTOND 7
#define NEOKEY_1X4_BUTTONMASK ((1 << 4) | (1 << 5) | (1 << 6) | (1 << 7))

#define NEOKEY_1X4_ROWS 1
#define NEOKEY_1X4_COLS 4
#define NEOKEY_1X4_KEYS (NEOKEY_1X4_ROWS * NEOKEY_1X4_COLS)

#define NEOKEY_1X4_MAX_CALLBACKS 32

/* NEOKEY_1X4_KEY depends on PCB routing */
// #define NEOKEY_1X4_KEY(x) (((x) / 4) * 8 + ((x) % 4))
#define NEOKEY_1X4_KEY(x) (((x) / 8) * 4 + ((x) % 8))

#define NEOKEY_1X4_X(k) ((k) % 4)
#define NEOKEY_1X4_Y(k) ((k) / 4)

#define NEOKEY_1X4_XY(x, y) ((y)*NEOKEY_1X4_ROWS + (x))

typedef void (*NeoKey1x4Callback)(keyEvent evt);

/**************************************************************************/
/*!
    @brief  Class that stores state and functions for interacting with the
   seesaw NeoKey module
*/
/**************************************************************************/
class Adafruit_NeoKey_1x4 : public Adafruit_seesaw {
public:
  Adafruit_NeoKey_1x4(uint8_t addr = NEOKEY_1X4_ADDR, TwoWire *i2c_bus = &Wire);
  virtual ~Adafruit_NeoKey_1x4(){};

  bool begin(uint8_t addr = NEOKEY_1X4_ADDR, int8_t flow = -1);

  void registerCallback(uint8_t key, NeoKey1x4Callback (*cb)(keyEvent));
  void unregisterCallback(uint8_t key);

  uint8_t read(void);

  seesaw_NeoPixel pixels; ///< the onboard neopixel matrix

  friend class Adafruit_MultiNeoKey1x4; ///< for allowing use of protected
                                        ///< methods by aggregate class

protected:
  uint8_t last_buttons = 0; ///< The last reading for the buttons
  uint8_t _addr;            ///< the I2C address of this board
  NeoKey1x4Callback (*_callbacks[NEOKEY_1X4_KEYS])(
      keyEvent); ///< the array of callback functions
};

/**************************************************************************/
/*!
    @brief  Class that stores state and functions for interacting with multiple
   neotrellis boards
*/
/**************************************************************************/
class Adafruit_MultiNeoKey1x4 {
public:
  Adafruit_MultiNeoKey1x4(Adafruit_NeoKey_1x4 *neokeys, uint8_t rows,
                          uint8_t cols);
  ~Adafruit_MultiNeoKey1x4(){};

  bool begin();

  void registerCallback(uint16_t num, NeoKey1x4Callback (*cb)(keyEvent));
  void registerCallback(uint8_t x, uint8_t y,
                        NeoKey1x4Callback (*cb)(keyEvent));
  void unregisterCallback(uint16_t num);
  void unregisterCallback(uint8_t x, uint8_t y);

  void setPixelColor(uint8_t x, uint8_t y, uint32_t color);
  void setPixelColor(uint16_t num, uint32_t color);
  void show();

  void read();

protected:
  uint8_t _rows; ///< the number of trellis boards in the Y direction
  uint8_t _cols; ///< the number of trellis boards in the X direction
  Adafruit_NeoKey_1x4 *_neokeys; ///< a multidimensional array of neokey objects
};

#endif
