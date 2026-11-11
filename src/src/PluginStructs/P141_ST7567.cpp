#include "../PluginStructs/P141_data_struct.h" // Defines P141_FEATURE_ST7567 and includes P141_ST7567.h

#if defined(USES_P141) && P141_FEATURE_ST7567

/****************************************************************************
 * Constructor, hardware SPI, height 64 or 32 pixels
 ***************************************************************************/
P141_ST7567::P141_ST7567(uint8_t   height,
                         int8_t    dc_pin,
                         int8_t    cs_pin,
                         int8_t    rst_pin,
                         SPIClass *theSPI)
  : Adafruit_GFX(ST7567_LCDWIDTH, height), _dcpin(dc_pin), _rstpin(rst_pin) {
  _spi_dev = new (std::nothrow) Adafruit_SPIDevice(cs_pin, 4000000, SPI_BITORDER_MSBFIRST, SPI_MODE0, theSPI);
  _buffer  = new (std::nothrow) uint8_t[WIDTH * HEIGHT / 8]();
}

P141_ST7567::~P141_ST7567() {
  delete _spi_dev;
  delete[] _buffer;
}

/****************************************************************************
 * begin: Set up SPI and initialize the display
 ***************************************************************************/
bool P141_ST7567::begin(uint8_t contrast) {
  if ((nullptr == _spi_dev) || (nullptr == _buffer) || !_spi_dev->begin()) {
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

  // Same sequence for 128x64 and 128x32 displays
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
  command(0x23); // V0 regulation resistor ratio 4.5 (gives a usable contrast range)
  setContrast(contrast);
  command(0xA4); // All pixels normal (not all on)

  clearRAM();    // Clear the complete display RAM, also outside the visible area
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
 * setAddress: Set page and column address for the next data
 ***************************************************************************/
void P141_ST7567::setAddress(uint8_t page, uint8_t column) {
  command(0xB0 | page);          // Page address
  command(0x10 | (column >> 4)); // Column address high nibble
  command(0x00 | (column & 0x0F)); // Column address low nibble
}

/****************************************************************************
 * clearRAM: Clear all display RAM, some displays show garbage at the borders otherwise
 ***************************************************************************/
void P141_ST7567::clearRAM() {
  const uint8_t zero[ST7567_RAM_COLUMNS]{};

  for (uint8_t page = 0; page < ST7567_RAM_PAGES; ++page) {
    setAddress(page, 0);
    digitalWrite(_dcpin, HIGH);
    _spi_dev->write(zero, ST7567_RAM_COLUMNS);
  }
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
  if (nullptr != _buffer) {
    memset(_buffer, 0, WIDTH * HEIGHT / 8);
  }
  cursor_y = cursor_x = 0;
}

/****************************************************************************
 * drawPixel: Set a single pixel in the buffer, rotation applied
 ***************************************************************************/
void P141_ST7567::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((nullptr == _buffer) || (x < 0) || (x >= _width) || (y < 0) || (y >= _height)) {
    return;
  }

  int16_t t;

  switch (rotation) { // Same rotation direction as Adafruit_PCD8544
    case 1:
      t = x;
      x = y;
      y = HEIGHT - 1 - t;
      break;
    case 2:
      x = WIDTH - 1 - x;
      y = HEIGHT - 1 - y;
      break;
    case 3:
      t = x;
      x = WIDTH - 1 - y;
      y = t;
      break;
  }

  if (color) {
    _buffer[x + (y / 8) * WIDTH] |= (1 << (y & 7));
  } else {
    _buffer[x + (y / 8) * WIDTH] &= ~(1 << (y & 7));
  }
}

/****************************************************************************
 * display: Transfer the buffer to the display, page by page
 ***************************************************************************/
void P141_ST7567::display() {
  if (nullptr == _buffer) { return; }

  for (uint8_t page = 0; page < (HEIGHT / 8); ++page) {
    setAddress(page, ST7567_X_OFFSET);
    digitalWrite(_dcpin, HIGH);
    _spi_dev->write(_buffer + (page * WIDTH), WIDTH);
  }
}

#endif // if defined(USES_P141) && P141_FEATURE_ST7567
