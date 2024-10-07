#include "LOLIN_EPD.h"
#include "Waveshare_1in54bv2.h"

///////////////////////////////////////////////////


/////////////////EPD settings Functions/////////////////////


#define BUSY_WAIT 1000 // msec

/*!
    @brief constructor if using on-chip RAM and hardware SPI
    @param width the width of the display in pixels
    @param height the height of the display in pixels
    @param DC the data/command pin to use
    @param RST the reset pin to use
    @param CS the chip select pin to use
    @param BUSY the busy pin to use
 */

Waveshare_1in54b::Waveshare_1in54b(int width, int height, int8_t DC, int8_t RST, int8_t CS, int8_t BUSY)
  : LOLIN_EPD(width, height, DC, RST, CS, BUSY) {
  if ((height % 8) > 0) {
    _height_8bit = (height / 8 + 1) * 8;
  } else {
    _height_8bit = height;
  }

  bw_bufsize  = width * _height_8bit / 8;
  bw_buf      = (uint8_t *)malloc(bw_bufsize);
  red_buf     = (uint8_t *)malloc(bw_bufsize);
  red_bufsize = bw_bufsize;
}

/**************************************************************************/

/*!
    @brief begin communication with and set up the display.
    @param reset if true the reset pin will be toggled.
 */

/**************************************************************************/
void Waveshare_1in54b::begin(bool reset) {
  LOLIN_EPD::begin(reset);

  readBusy();
  sendCmd(0x12);
  readBusy();

  sendCmd(EPD1IN54B_POWER_SETTING); // Driver output control
  sendData(0xC7);
  sendData(0x00);
  sendData(0x01);

  sendCmd(EPD1IN54B_DATA_STOP); // data entry mode
  sendData(0x01);

  sendCmd(0x44);                // set Ram-X address start/end position
  sendData(0x00);
  sendData(0x18);               // 0x18-->(24+1)*8=200

  sendCmd(0x45);                // set Ram-Y address start/end position
  sendData(0xC7);               // 0xC7-->(199+1)=200
  sendData(0x00);
  sendData(0x00);
  sendData(0x00);

  sendCmd(0x3C); // BorderWavefrom
  sendData(0x05);

  sendCmd(0x18); // Read built-in temperature sensor
  sendData(0x80);

  sendCmd(0x4E); // set RAM x address count to 0;
  sendData(0x00);
  sendCmd(0x4F); // set RAM y address count to 0X199;
  sendData(0xC7);
  sendData(0x00);

  /* EPD hardware init end */
  readBusy();
}

void Waveshare_1in54b::display() {
  sendCmd(0x24); // write RAM for black(0)/white (1)
  delay(2);

  for (uint16_t i = 0; i < bw_bufsize; i++) {
    sendData(bw_buf[i]);
  }
  delay(2);

  sendCmd(0x26); // write RAM for red(1)/white (0)

  for (uint16_t i = 0; i < red_bufsize; i++) {
    sendData(red_buf[i]);
  }
  delay(2);


  update();
}

void Waveshare_1in54b::update() {
  sendCmd(0x22);
  sendData(0xF7);
  sendCmd(0x20);
  readBusy();
}

/**************************************************************************/

/*!
    @brief draw a single pixel on the screen
    @param x the x axis position
    @param y the y axis position
    @param color the color of the pixel
 */

/**************************************************************************/
void Waveshare_1in54b::drawPixel(int16_t x, int16_t y, uint16_t color) {
  uint8_t *pBuf;

  // Transformed to native pixels
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) {
    return;
  }

  // check rotation, move pixel around if necessary
  switch (getRotation()) {
    case 0:
      EPD_swap(x, y);
      y = WIDTH - y - 1;
      break;
    case 1:
      break;
    case 2:
      EPD_swap(x, y);
      x = HEIGHT - x - 1;
      break;
    case 3:
      x = HEIGHT - x - 1;
      y = WIDTH - y - 1;
      break;
  }

  // make our buffer happy
  x = (x == 0 ? 1 : x);

  uint16_t addr = (x + _height_8bit * y) / 8;

  if (color == EPD_RED) {
    pBuf = red_buf + addr;
  } else {
    pBuf = bw_buf + addr;
  }

  // x is which column
  switch (color) {
    case EPD_WHITE:
      *pBuf |= (0x80 >> (x % 8)); // (1 << (7 - y % 8));
      break;
    case EPD_RED:
    case EPD_BLACK:
      *pBuf &= ~(0x80 >> (x % 8)); // ~(1 << (7 - y % 8));
      break;
    case EPD_INVERSE:
      *pBuf ^= (0x80 >> (x % 8));  // (1 << (7 - y % 8));
      break;
  }
}

/**************************************************************************/

/*!
    @brief clear all data buffers
 */

/**************************************************************************/
void Waveshare_1in54b::clearBuffer() {
  memset(bw_buf,  0xFF, bw_bufsize);
  memset(red_buf, 0x00, red_bufsize);
}

/**************************************************************************/

/*!
    @brief clear the display twice to remove any spooky ghost images
 */

/**************************************************************************/
void Waveshare_1in54b::clearDisplay() {
  clearBuffer();
  display();

  // delay(100);
  update();
}

void Waveshare_1in54b::readBusy() {
  if (busy >= 0) {
    while (1) {
      if (digitalRead(busy) == 0) {
        delay(1);
        break;
      }
      delay(0);
    }
  } else {
    delay(BUSY_WAIT);
  }
}

// void Waveshare_1in54b::selectLUT(uint8_t *wave_data) {
//   uint8_t count;

//   sendCmd(0x32);

//   for (count = 0; count < 70; count++) {
//     sendData(pgm_read_byte(&wave_data[count]));
//   }
// }

void Waveshare_1in54b::deepSleep() {
  sendCmd(EPD1IN54B_DEEP_SLEEP);
  sendData(0xa5);
}

void Waveshare_1in54b::partInit()
{
  // partBaseImg();

  // sendCmd(0x2C); //VCOM Voltage
  // sendData(0x26);

  // readBusy();
  // selectLUT((unsigned char *)LUT_DATA_part);
  // sendCmd(0x37);
  // sendData(0x00);
  // sendData(0x00);
  // sendData(0x00);
  // sendData(0x00);
  // sendData(0x40);
  // sendData(0x00);
  // sendData(0x00);

  // sendCmd(0x22);
  // sendData(0xC0);
  // sendCmd(0x20);
  // readBusy();

  // sendCmd(0x3C); //BorderWavefrom
  // sendData(0x01);
}

void Waveshare_1in54b::partBaseImg()
{
  // sendCmd(0x24); ////Write Black and White image to RAM

  // for (uint16_t i = 0; i < bw_bufsize; i++) {
  //   sendData(bw_buf[i]);
  // }

  // sendCmd(0x26); ////Write Black and White image to RAM

  // for (uint16_t i = 0; i < bw_bufsize; i++) {
  //   sendData(bw_buf[i]);
  // }

  // update();
}

void Waveshare_1in54b::partUpdate()
{
  // sendCmd(0x22);
  // sendData(0x0C);
  // sendCmd(0x20);
  // readBusy();
}

void Waveshare_1in54b::partDisplay(int16_t x_start, int16_t y_start, const unsigned char *datas, int16_t PART_COLUMN, int16_t PART_LINE)
{
  // int16_t i;
  // int16_t x_end, y_start1, y_start2, y_end1, y_end2;

  // x_start = x_start / 8; //
  // x_end   = x_start + PART_LINE / 8 - 1;

  // y_start1 = 0;
  // y_start2 = y_start;

  // if (y_start >= 256)
  // {
  //   y_start1 = y_start2 / 256;
  //   y_start2 = y_start2 % 256;
  // }
  // y_end1 = 0;
  // y_end2 = y_start + PART_COLUMN - 1;

  // if (y_end2 >= 256)
  // {
  //   y_end1 = y_end2 / 256;
  //   y_end2 = y_end2 % 256;
  // }

  // sendCmd(0x44);      // set RAM x address start/end, in page 35
  // sendData(x_start);  // RAM x address start at 00h;
  // sendData(x_end);    // RAM x address end at 0fh(15+1)*8->128
  // sendCmd(0x45);      // set RAM y address start/end, in page 35
  // sendData(y_start2); // RAM y address start at 0127h;
  // sendData(y_start1); // RAM y address start at 0127h;
  // sendData(y_end2);   // RAM y address end at 00h;
  // sendData(y_end1);   // ????=0

  // sendCmd(0x4E);      // set RAM x address count to 0;
  // sendData(x_start);
  // sendCmd(0x4F);      // set RAM y address count to 0X127;
  // sendData(y_start2);
  // sendData(y_start1);

  // sendCmd(0x24); // Write Black and White image to RAM

  // for (i = 0; i < PART_COLUMN * PART_LINE / 8; i++)
  // {
  //   sendData(pgm_read_byte(&datas[i]));
  // }
  // partUpdate();
}
