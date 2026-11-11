#include "../PluginStructs/P141_ST7567.h"

#ifdef USES_P141

/****************************************************************************
 * Constructor, hardware SPI
 ***************************************************************************/
P141_ST7567::P141_ST7567(int8_t    dc_pin,
                         int8_t    cs_pin,
                         int8_t    rst_pin,
                         SPIClass *theSPI)
  : Adafruit_GFX(ST7567_LCDWIDTH, ST7567_LCDHEIGHT), _dcpin(dc_pin), _rstpin(rst_pin) {
  _spi_dev = new (std::nothrow) Adafruit_SPIDevice(cs_pin, 4000000, SPI_BITORDER_MSBFIRST, SPI_MODE0, theSPI);
}

P141_ST7567::~P141_ST7567() {
  delete _spi_dev;
}

/****************************************************************************
 * begin: Set up SPI and initialize the display
 ***************************************************************************/
bool P141_ST7567::begin(uint8_t contrast) {
  if ((nullptr == _spi_dev) || !_spi_dev->begin()) {
    return false;
  }

  pinMode(_dcpin, OUTPUT);

  if (_rstpin >= 0) { // Hardware reset
    pinMode(_rstpin, OUTPUT);
    digitalWrite(_rstpin, LOW);
    delay(1);
    digitalWrite(_rstpin, HIGH);
    delay(1);
  }

  command(0xE2); // Soft reset
  command(0xAE); // Display off
  command(0x40); // Display start line 0
  command(0xA1); // SEG direction reversed (ADC)
  command(0xC0); // COM direction normal
  command(0xA6); // Display normal
  command(0xA3); // LCD bias 1/7
  command(0x2C); // Power control: booster on
  delay(50);
  command(0x2E); // Power control: + voltage regulator
  delay(50);
  command(0x2F); // Power control: + voltage follower
  delay(50);
  command(0x23); // V0 regulation resistor ratio 4.5 (u8g2 uses 6.0, but then the usable contrast range is only ~0..10%)
  setContrast(contrast);
  command(0xA4); // All pixels normal (not all on)

  clearDisplay();
  display();
  command(0xAF); // Display on
  return true;
}

/****************************************************************************
 * command: Send a command byte
 ***************************************************************************/
void P141_ST7567::command(uint8_t c) {
  digitalWrite(_dcpin, LOW);
  _spi_dev->write(&c, 1);
}

/****************************************************************************
 * setContrast: Electronic volume 0..63
 ***************************************************************************/
void P141_ST7567::setContrast(uint8_t val) {
  if (val > ST7567_CONTRAST_MAX) { val = ST7567_CONTRAST_MAX; }
  command(0x81);
  command(val);
}

/****************************************************************************
 * invertDisplay: Invert all pixels in hardware
 ***************************************************************************/
void P141_ST7567::invertDisplay(bool i) {
  command(i ? 0xA7 : 0xA6);
}

/****************************************************************************
 * clearDisplay: Clear the buffer
 ***************************************************************************/
void P141_ST7567::clearDisplay() {
  memset(_buffer, 0, sizeof(_buffer));
  cursor_y = cursor_x = 0;
}

/****************************************************************************
 * drawPixel: Set a single pixel in the buffer, rotation applied
 ***************************************************************************/
void P141_ST7567::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) {
    return;
  }

  int16_t t;

  switch (rotation) {
    case 1:
      t = x;
      x = y;
      y = ST7567_LCDHEIGHT - 1 - t;
      break;
    case 2:
      x = ST7567_LCDWIDTH - 1 - x;
      y = ST7567_LCDHEIGHT - 1 - y;
      break;
    case 3:
      t = x;
      x = ST7567_LCDWIDTH - 1 - y;
      y = t;
      break;
  }

  if (color) {
    _buffer[x + (y / 8) * ST7567_LCDWIDTH] |= (1 << (y & 7));
  } else {
    _buffer[x + (y / 8) * ST7567_LCDWIDTH] &= ~(1 << (y & 7));
  }
}

/****************************************************************************
 * display: Transfer the buffer to the display, page by page
 ***************************************************************************/
void P141_ST7567::display() {
  for (uint8_t page = 0; page < (ST7567_LCDHEIGHT / 8); ++page) {
    command(0xB0 | page);                    // Page address
    command(0x10 | (ST7567_X_OFFSET >> 4));  // Column address high nibble
    command(0x00 | (ST7567_X_OFFSET & 0x0F)); // Column address low nibble
    digitalWrite(_dcpin, HIGH);
    _spi_dev->write(_buffer + (page * ST7567_LCDWIDTH), ST7567_LCDWIDTH);
  }
}

#endif // ifdef USES_P141
