#include "LOLIN_EPD.h"
#include "MH_ET_Live_1in54.h"

///////////////////////////////////////////////////
const unsigned char MH_ET_Live_lut_full_update[] PROGMEM =
{
  0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22,
  0x66, 0x69, 0x69, 0x59, 0x58, 0x99, 0x99, 0x88,
  0x00, 0x00, 0x00, 0x00, 0xF8, 0xB4, 0x13, 0x51,
  0x35, 0x51, 0x51, 0x19, 0x01, 0x00
};


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

MH_ET_Live_1in54::MH_ET_Live_1in54(int width, int height, int8_t DC, int8_t RST, int8_t CS, int8_t BUSY)
  : LOLIN_EPD(width, height, DC, RST, CS, BUSY), _width(width), _height(height) {
  if ((_height % 8) > 0) {
    _height_8bit = (_height / 8 + 1) * 8;
  } else {
    _height_8bit = _height;
  }

  bw_bufsize  = _width * _height_8bit / 8;
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
void MH_ET_Live_1in54::begin(bool reset) {
  LOLIN_EPD::begin(reset);

  sendCmd(MH_ET_DRIVER_OUTPUT_CONTROL);
  sendData((_height - 1));
  sendData((_height - 1) >> 8);
  sendData(0x00);
  sendCmd(MH_ET_BOOSTER_SOFT_START_CONTROL);
  sendData(0xD7);
  sendData(0xD6);
  sendData(0x9D);
  sendCmd(MH_ET_WRITE_VCOM_REGISTER);
  sendData(0xA8);
  sendCmd(MH_ET_SET_DUMMY_LINE_PERIOD);
  sendData(0x1A);
  sendCmd(MH_ET_SET_GATE_LINE_WIDTH);
  sendData(0x08);
  sendCmd(MH_ET_DATA_ENTRY_MODE_SETTING);
  sendData(0x03);
  setLut(MH_ET_Live_lut_full_update);

  /* EPD hardware init end */
  readBusy();
}

/**
 *  @brief: set the look-up table register
 */
void MH_ET_Live_1in54::setLut(const unsigned char *lut)
{
  sendCmd(MH_ET_WRITE_LUT_REGISTER);

  /* the length of look-up table is 30 bytes */
  for (int i = 0; i < 30; i++) {
    sendData(pgm_read_byte(&(lut[i])));
  }
}

void MH_ET_Live_1in54::display() {
  sendCmd(MH_ET_WRITE_RAM); // write RAM for black(0)/white (1)
  delay(2);

  for (uint16_t i = 0; i < bw_bufsize; i++) {
    sendData(bw_buf[i]);
  }
  delay(2);

  sendCmd(MH_ET_WRITE_RAM_RED); // write RAM for red(1)/white (0)

  for (uint16_t i = 0; i < red_bufsize; i++) {
    sendData(red_buf[i]);
  }
  delay(2);


  update();
}

void MH_ET_Live_1in54::update() {
  sendCmd(MH_ET_DISPLAY_UPDATE_CONTROL_2);
  sendData(0xF7);
  sendCmd(MH_ET_MASTER_ACTIVATION);
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
void MH_ET_Live_1in54::drawPixel(int16_t x, int16_t y, uint16_t color) {
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
void MH_ET_Live_1in54::clearBuffer() {
  memset(bw_buf,  0xFF, bw_bufsize);
  memset(red_buf, 0x00, red_bufsize);
}

/**************************************************************************/

/*!
    @brief clear the display twice to remove any spooky ghost images
 */

/**************************************************************************/
void MH_ET_Live_1in54::clearDisplay() {
  clearBuffer();
  display();

  // delay(100);
  update();
}

void MH_ET_Live_1in54::readBusy() {
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

// void MH_ET_Live_1in54::selectLUT(uint8_t *wave_data) {
//   uint8_t count;

//   sendCmd(0x32);

//   for (count = 0; count < 70; count++) {
//     sendData(pgm_read_byte(&wave_data[count]));
//   }
// }

void MH_ET_Live_1in54::deepSleep() {
  sendCmd(EPD1IN54B_DEEP_SLEEP);
  sendData(0xa5);
}

void MH_ET_Live_1in54::partInit()
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

void MH_ET_Live_1in54::partBaseImg()
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

void MH_ET_Live_1in54::partUpdate()
{
  // sendCmd(0x22);
  // sendData(0x0C);
  // sendCmd(0x20);
  // readBusy();
}

void MH_ET_Live_1in54::partDisplay(int16_t x_start, int16_t y_start, const unsigned char *datas, int16_t PART_COLUMN, int16_t PART_LINE)
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
