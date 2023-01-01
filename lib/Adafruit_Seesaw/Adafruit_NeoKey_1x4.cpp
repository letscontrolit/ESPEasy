#include "Adafruit_NeoKey_1x4.h"

/**************************************************************************/
/*!
    @brief  Class constructor
    @param  addr the I2C address this neotrellis object uses
    @param  i2c_bus the I2C bus connected to this neokey, defaults to "Wire"
*/
/**************************************************************************/
Adafruit_NeoKey_1x4::Adafruit_NeoKey_1x4(uint8_t addr, TwoWire *i2c_bus)
    : Adafruit_seesaw(i2c_bus), pixels(NEOKEY_1X4_KEYS, NEOKEY_1X4_NEOPIN,
                                       NEO_GRB + NEO_KHZ800, i2c_bus) {
  for (int i = 0; i < NEOKEY_1X4_KEYS; i++) {
    _callbacks[i] = NULL;
  }
  this->_addr = addr;
}

/**************************************************************************/
/*!
    @brief  Begin communication with the RGB trellis.
    @param  addr optional i2c address where the device can be found. Defaults to
   NEOKEY_1X4_ADDR
    @param  flow optional flow control pin
    @returns true on success, false on error.
*/
/**************************************************************************/
bool Adafruit_NeoKey_1x4::begin(uint8_t addr, int8_t flow) {
  _addr = addr;

  bool ret = pixels.begin(addr, flow);
  if (!ret)
    return ret;

  ret = Adafruit_seesaw::begin(addr, flow, false);
  if (!ret)
    return ret;

  pixels.setBrightness(40);
  pixels.show(); // Initialize all pixels to 'off'
  delay(5);
  pinModeBulk(NEOKEY_1X4_BUTTONMASK, INPUT_PULLUP);
  setGPIOInterrupts(NEOKEY_1X4_BUTTONMASK, 1);

  return ret;
}

/**************************************************************************/
/*!
    @brief  register a callback function on the passed key.
    @param  key the key number to register the callback on
    @param  cb the callback function that should be called when an event on that
   key happens
*/
/**************************************************************************/
void Adafruit_NeoKey_1x4::registerCallback(uint8_t key,
                                           NeoKey1x4Callback (*cb)(keyEvent)) {
  _callbacks[key] = cb;
}

/**************************************************************************/
/*!
    @brief  unregister a callback on a given key
    @param  key the key number the callback is currently mapped to.
*/
/**************************************************************************/
void Adafruit_NeoKey_1x4::unregisterCallback(uint8_t key) {
  _callbacks[key] = NULL;
}

/**************************************************************************/
/*!
    @brief  Read key GPIO pins, possibly generating callback events
    @returns Byte with the bottom 4 bits corresponding to each keypress status
*/
/**************************************************************************/
uint8_t Adafruit_NeoKey_1x4::read(void) {

  uint32_t buttons = digitalReadBulk(NEOKEY_1X4_BUTTONMASK);
  buttons ^= NEOKEY_1X4_BUTTONMASK;
  buttons &= NEOKEY_1X4_BUTTONMASK;
  buttons >>= NEOKEY_1X4_BUTTONA;

  uint8_t just_pressed = (buttons ^ last_buttons) & buttons;
  uint8_t just_released = (buttons ^ last_buttons) & ~buttons;
  if (just_pressed | just_released) {
    // Serial.print("pressed 0x"); Serial.println(just_pressed, HEX);
    // Serial.print("released 0x"); Serial.println(just_released, HEX);

    for (int b = 0; b < 4; b++) {
      if (just_pressed & (1 << b)) { // if button b is pressed
        if (_callbacks[b] != NULL) {
          keyEvent evt = {SEESAW_KEYPAD_EDGE_RISING, (uint16_t)b};
          _callbacks[b](evt);
        }
      }

      if (just_released & (1 << b)) { // if button b is released
        if (_callbacks[b] != NULL) {
          keyEvent evt = {SEESAW_KEYPAD_EDGE_FALLING, (uint16_t)b};
          _callbacks[b](evt);
        }
      }
    }
  }
  last_buttons = buttons;
  return buttons;
}

/**************************************************************************/
/*!
    @brief  class constructor
    @param  neokeys pointer to a multidimensional array of Adafruit_NeoKey_1x4
   objects. these object must have their I2C addresses specified in the class
            constructors.
    @param  rows the number of individual neokey boards in the Y direction
            of your matrix.
    @param  cols the number of individual neokey boards in the X direction
            of your matrix.
*/
/**************************************************************************/
Adafruit_MultiNeoKey1x4::Adafruit_MultiNeoKey1x4(Adafruit_NeoKey_1x4 *neokeys,
                                                 uint8_t rows, uint8_t cols) {
  this->_rows = rows;
  this->_cols = cols;
  this->_neokeys = neokeys;
}

/**************************************************************************/
/*!
    @brief  begin communication with the matrix of neotrellis boards.
    @returns true on success, false otherwise.
*/
/**************************************************************************/
bool Adafruit_MultiNeoKey1x4::begin() {
  Adafruit_NeoKey_1x4 *t;
  for (int n = 0; n < _rows; n++) {
    for (int m = 0; m < _cols; m++) {
      t = (_neokeys + n * _cols) + m;
      if (!t->begin(t->_addr))
        return false;
    }
  }

  return true;
}

/**************************************************************************/
/*!
    @brief  register a callback for a key addressed by key index.
    @param  x the column index of the key. column 0 is on the lefthand side of
   the matix.
    @param  y the row index of the key. row 0 is at the top of the matrix and
   the numbers increase downwards.
    @param  cb the function to be called when an event from the specified key is
            detected.
*/
/**************************************************************************/
void Adafruit_MultiNeoKey1x4::registerCallback(
    uint8_t x, uint8_t y, NeoKey1x4Callback (*cb)(keyEvent)) {
  Adafruit_NeoKey_1x4 *t =
      (_neokeys + y / NEOKEY_1X4_ROWS * _cols) + x / NEOKEY_1X4_COLS;
  int xkey = NEOKEY_1X4_X(x);
  int ykey = NEOKEY_1X4_Y(y % NEOKEY_1X4_ROWS * NEOKEY_1X4_COLS);

  t->registerCallback(NEOKEY_1X4_XY(xkey, ykey), cb);
}

/**************************************************************************/
/*!
    @brief  register a callback for a key addressed by key number.
    @param  num the keynumber to set the color of. Key 0 is in the top left
            corner of the trellis matrix, key 1 is directly to the right of it,
            and the last key number is in the bottom righthand corner.
    @param  cb the function to be called when an event from the specified key is
            detected.
*/
/**************************************************************************/
void Adafruit_MultiNeoKey1x4::registerCallback(
    uint16_t num, NeoKey1x4Callback (*cb)(keyEvent)) {
  uint8_t x = num % (NEOKEY_1X4_COLS * _cols);
  uint8_t y = num / (NEOKEY_1X4_COLS * _cols);

  registerCallback(x, y, cb);
}

/**************************************************************************/
/*!
    @brief  Unregister a callback for a key addressed by key index.
    @param  x the column index of the key. column 0 is on the lefthand side of
   the matix.
    @param  y the row index of the key. row 0 is at the top of the matrix and
   the numbers increase downwards.
*/
/**************************************************************************/
void Adafruit_MultiNeoKey1x4::unregisterCallback(uint8_t x, uint8_t y) {
  Adafruit_NeoKey_1x4 *t =
      (_neokeys + y / NEOKEY_1X4_ROWS * _cols) + x / NEOKEY_1X4_COLS;
  int xkey = NEOKEY_1X4_X(x);
  int ykey = NEOKEY_1X4_Y(y % NEOKEY_1X4_ROWS * NEOKEY_1X4_COLS);

  t->unregisterCallback(NEOKEY_1X4_XY(xkey, ykey));
}

/**************************************************************************/
/*!
    @brief  Unregister a callback for a key addressed by key number.
    @param  num the keynumber to set the color of. Key 0 is in the top left
            corner of the trellis matrix, key 1 is directly to the right of it,
            and the last key number is in the bottom righthand corner.
*/
/**************************************************************************/
void Adafruit_MultiNeoKey1x4::unregisterCallback(uint16_t num) {
  uint8_t x = num % (NEOKEY_1X4_COLS * _cols);
  uint8_t y = num / (NEOKEY_1X4_COLS * _cols);

  unregisterCallback(x, y);
}

/**************************************************************************/
/*!
    @brief  set the color of a neopixel at a key index.
    @param  x the column index of the key. column 0 is on the lefthand side of
   the matix.
    @param  y the row index of the key. row 0 is at the top of the matrix and
   the numbers increase downwards.
    @param  color the color to set the pixel to. This is a 24 bit RGB value.
            for example, full brightness red would be 0xFF0000, and full
   brightness blue would be 0x0000FF.
*/
/**************************************************************************/
void Adafruit_MultiNeoKey1x4::setPixelColor(uint8_t x, uint8_t y,
                                            uint32_t color) {
  Adafruit_NeoKey_1x4 *t =
      (_neokeys + y / NEOKEY_1X4_ROWS * _cols) + x / NEOKEY_1X4_COLS;
  int xkey = NEOKEY_1X4_X(x);
  int ykey = NEOKEY_1X4_Y(y % NEOKEY_1X4_ROWS * NEOKEY_1X4_COLS);

  t->pixels.setPixelColor(NEOKEY_1X4_XY(xkey, ykey), color);
}

/**************************************************************************/
/*!
    @brief  set the color of a neopixel at a key number.
    @param  num the keynumber to set the color of. Key 0 is in the top left
            corner of the trellis matrix, key 1 is directly to the right of it,
            and the last key number is in the bottom righthand corner.
    @param  color the color to set the pixel to. This is a 24 bit RGB value.
            for example, full brightness red would be 0xFF0000, and full
   brightness blue would be 0x0000FF.
*/
/**************************************************************************/
void Adafruit_MultiNeoKey1x4::setPixelColor(uint16_t num, uint32_t color) {
  uint8_t x = num % (NEOKEY_1X4_COLS * _cols);
  uint8_t y = num / (NEOKEY_1X4_COLS * _cols);

  setPixelColor(x, y, color);
}

/**************************************************************************/
/*!
    @brief  call show for all connected neotrellis boards to show all neopixels
*/
/**************************************************************************/
void Adafruit_MultiNeoKey1x4::show() {
  Adafruit_NeoKey_1x4 *t;
  for (int n = 0; n < _rows; n++) {
    for (int m = 0; m < _cols; m++) {
      t = (_neokeys + n * _cols) + m;
      t->pixels.show();
    }
  }
}

/**************************************************************************/
/*!
    @brief  read all events currently stored in the seesaw fifo and call any
   callbacks.
*/
/**************************************************************************/
void Adafruit_MultiNeoKey1x4::read() {
  Adafruit_NeoKey_1x4 *nk;

  for (int n = 0; n < _rows; n++) {
    for (int m = 0; m < _cols; m++) {
      // get the individual breakout
      nk = (_neokeys + n * _cols) + m;

      // query what buttons are pressed
      nk->digitalReadBulk(
          NEOKEY_1X4_BUTTONMASK); // not sure why we have to do it 2ce
      uint32_t buttons = nk->digitalReadBulk(NEOKEY_1X4_BUTTONMASK);

      // Serial.print("Neokey number "); Serial.print(n * _cols + m);
      // Serial.print(" buttons: 0x"); Serial.println(buttons, HEX);
      buttons ^= NEOKEY_1X4_BUTTONMASK;
      buttons &= NEOKEY_1X4_BUTTONMASK;
      buttons >>= NEOKEY_1X4_BUTTONA;

      // compared to last time
      uint8_t just_pressed = (buttons ^ nk->last_buttons) & buttons;
      uint8_t just_released = (buttons ^ nk->last_buttons) & ~buttons;
      // stash for next run
      nk->last_buttons = buttons;
      if (just_pressed | just_released) {
        // Serial.print("pressed 0x"); Serial.println(just_pressed, HEX);
        // Serial.print("released 0x"); Serial.println(just_released, HEX);

        // for each key, process the event
        for (int b = 0; b < 4; b++) {
          int x = b; // column is the button
          int y = 0; // a 1x4 neokey only has one row

          // extend into whole grid
          x = x + m * NEOKEY_1X4_COLS;
          y = y + n * NEOKEY_1X4_ROWS;

          int event_key = y * NEOKEY_1X4_COLS * _cols + x;

          if (just_pressed & (1 << b)) { // if button b is pressed
            if (nk->_callbacks[b] != NULL) {
              keyEvent evt = {SEESAW_KEYPAD_EDGE_RISING, (uint16_t)event_key};
              nk->_callbacks[b](evt);
            }
          }
          if (just_released & (1 << b)) { // if button b is pressed
            if (nk->_callbacks[b] != NULL) {
              keyEvent evt = {SEESAW_KEYPAD_EDGE_FALLING, (uint16_t)event_key};
              nk->_callbacks[b](evt);
            }
          }
        }
      }
    }
  }
}
