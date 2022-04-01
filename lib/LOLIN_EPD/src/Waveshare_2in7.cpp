#include "LOLIN_EPD.h"
#include "Waveshare_2in7.h"

#define ALLSCREEN_GRAGHBYTES 4000

///////////////////////////////////////////////////


/////////////////EPD settings Functions/////////////////////

/////////////////////////////////////LUT//////////////////////////////////////////////
static const unsigned char EPD_2in7_gray_lut_vcom[] = {
  0x00, 0x00,
  0x00, 0x0A,0x00,  0x00,  0x00,  0x01,
  0x60, 0x14,0x14,  0x00,  0x00,  0x01,
  0x00, 0x14,0x00,  0x00,  0x00,  0x01,
  0x00, 0x13,0x0A,  0x01,  0x00,  0x01,
  0x00, 0x00,0x00,  0x00,  0x00,  0x00,
  0x00, 0x00,0x00,  0x00,  0x00,  0x00,
  0x00, 0x00,0x00,  0x00,  0x00,  0x00,
};

// R21
static const unsigned char EPD_2in7_gray_lut_ww[] = {
  0x40, 0x0A, 0x00, 0x00, 0x00, 0x01,
  0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
  0x10, 0x14, 0x0A, 0x00, 0x00, 0x01,
  0xA0, 0x13, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// R22H  r
static const unsigned char EPD_2in7_gray_lut_bw[] = {
  0x40, 0x0A, 0x00, 0x00, 0x00, 0x01,
  0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
  0x00, 0x14, 0x0A, 0x00, 0x00, 0x01,
  0x99, 0x0C, 0x01, 0x03, 0x04, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// R23H  w
static const unsigned char EPD_2in7_gray_lut_wb[] = {
  0x40, 0x0A, 0x00, 0x00, 0x00, 0x01,
  0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
  0x00, 0x14, 0x0A, 0x00, 0x00, 0x01,
  0x99, 0x0B, 0x04, 0x04, 0x01, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// R24H  b
static const unsigned char EPD_2in7_gray_lut_bb[] = {
  0x80, 0x0A, 0x00, 0x00, 0x00, 0x01,
  0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
  0x20, 0x14, 0x0A, 0x00, 0x00, 0x01,
  0x50, 0x13, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

#define BUSY_WAIT 1000

/*!
    @brief constructor if using on-chip RAM and hardware SPI
    @param width the width of the display in pixels
    @param height the height of the display in pixels
    @param DC the data/command pin to use
    @param RST the reset pin to use
    @param CS the chip select pin to use
    @param BUSY the busy pin to use
 */

Waveshare_2in7::Waveshare_2in7(int width, int height, int8_t DC, int8_t RST, int8_t CS, int8_t BUSY)
  : LOLIN_EPD(width, height, DC, RST, CS, BUSY) {
  if ((height % 8) > 0) {
    _height_8bit = (height / 8 + 1) * 8;
  } else {
    _height_8bit = height;
  }

  bw_bufsize = width * _height_8bit / 8;
  bw_buf     = (uint8_t *)malloc(bw_bufsize);
}

/**************************************************************************/

/*!
    @brief begin communication with and set up the display.
    @param reset if true the reset pin will be toggled.
 */

/**************************************************************************/
void Waveshare_2in7::begin(bool reset) {
  LOLIN_EPD::begin(reset);

  readBusy();
  sendCmd(0x12);
  readBusy();

  sendCmd(WS2IN7_POWER_SETTING);
  sendData(0x03); // VDS_EN, VDG_EN
  sendData(0x00); // VCOM_HV, VGHL_LV[1], VGHL_LV[0]
  sendData(0x2b); // VDH
  sendData(0x2b); // VDL
  sendData(0x09); // VDHR
  sendCmd(WS2IN7_BOOSTER_SOFT_START);
  sendData(0x07);
  sendData(0x07);
  sendData(0x17);

  // Power optimization
  sendCmd(WS2IN7_POWER_OPTIMIZATION);
  sendData(0x60);
  sendData(0xA5);

  // Power optimization
  sendCmd(WS2IN7_POWER_OPTIMIZATION);
  sendData(0x89);
  sendData(0xA5);

  // Power optimization
  sendCmd(WS2IN7_POWER_OPTIMIZATION);
  sendData(0x90);
  sendData(0x00);

  // Power optimization
  sendCmd(WS2IN7_POWER_OPTIMIZATION);
  sendData(0x93);
  sendData(0x2A);

  // Power optimization
  sendCmd(WS2IN7_POWER_OPTIMIZATION);
  sendData(0xA0);
  sendData(0xA5);

  // Power optimization
  sendCmd(WS2IN7_POWER_OPTIMIZATION);
  sendData(0xA1);
  sendData(0x00);

  // Power optimization
  sendCmd(WS2IN7_POWER_OPTIMIZATION);
  sendData(0x73);
  sendData(0x41);

  sendCmd(WS2IN7_PARTIAL_DISPLAY_REFRESH);
  sendData(0x00);
  sendCmd(WS2IN7_POWER_ON);
  readBusy();

  sendCmd(WS2IN7_PANEL_SETTING);
  sendData(0xAF); // KW-BF   KWR-AF    BWROTP 0f
  sendCmd(WS2IN7_PLL_CONTROL);
  sendData(0x3A); // 3A 100HZ   29 150Hz 39 200HZ    31 171HZ
  sendCmd(WS2IN7_VCM_DC_SETTING_REGISTER);
  sendData(0x12);
  delay(2);
  setLut();

  /* EPD hardware init end */
  readBusy();
}

/**
 *  @brief: set the look-up tables
 */
void Waveshare_2in7::setLut(void) {
  unsigned int count;

  sendCmd(WS2IN7_LUT_FOR_VCOM); // vcom

  for (count = 0; count < 44; count++) {
    sendData(lut_vcom_dc[count]);
  }

  sendCmd(WS2IN7_LUT_WHITE_TO_WHITE); // ww --

  for (count = 0; count < 42; count++) {
    sendData(lut_ww[count]);
  }

  sendCmd(WS2IN7_LUT_BLACK_TO_WHITE); // bw r

  for (count = 0; count < 42; count++) {
    sendData(lut_bw[count]);
  }

  sendCmd(WS2IN7_LUT_WHITE_TO_BLACK); // wb w

  for (count = 0; count < 42; count++) {
    sendData(lut_bb[count]);
  }

  sendCmd(WS2IN7_LUT_BLACK_TO_BLACK); // bb b

  for (count = 0; count < 42; count++) {
    sendData(lut_wb[count]);
  }
}

void Waveshare_2in7::display() {
  // sendCmd(DATA_START_TRANSMISSION_1); // ?? write RAM for black(0)/white (1)
  // delay(2);

  // for (uint16_t i = 0; i < bw_bufsize; i++) {
  //   sendData(0xFF);
  // }
  // delay(2);

  sendCmd(WS2IN7_DATA_START_TRANSMISSION_2); // write RAM for black(0)/white (1)
  delay(2);

  for (uint16_t i = 0; i < bw_bufsize; i++) {
    sendData(bw_buf[i]);
  }
  delay(2);

  update();
}

void Waveshare_2in7::update() {
  sendCmd(WS2IN7_DISPLAY_REFRESH);
  delay(200);
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
void Waveshare_2in7::drawPixel(int16_t x, int16_t y, uint16_t color) {
  uint8_t *pBuf;

  // Transformed to native pixels
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) {
    return;
  }
  
  // Corrections by @kretzp (Peter Kretz) 2022-03-27
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

  // uint16_t addr = (x * height() + y) / 8;
  uint16_t addr = (x + _height_8bit * y) / 8;

  //   if (color == EPD_RED) {
  //     pBuf = red_buf + addr;
  //   } else {
  pBuf = bw_buf + addr;

  //   }

  // x is which column
  switch (color) {
    case EPD_WHITE:
      *pBuf |= (0x80 >> (x % 8)); // (1 << (7 - y % 8));
      break;
    case EPD_RED:
    case EPD_BLACK:
      *pBuf &= ~(0x80 >> (x % 8)); // ~(1 << (7 - y % 8));
      break;

      // case EPD_INVERSE:
      //   *pBuf ^= (0x80 >> (x % 8));  // (1 << (7 - y % 8));
      //   break;
  }
}

/**************************************************************************/

/*!
    @brief clear all data buffers
 */

/**************************************************************************/
void Waveshare_2in7::clearBuffer() {
  memset(bw_buf, 0xFF, bw_bufsize);
}

/**************************************************************************/

/*!
    @brief clear the display twice to remove any spooky ghost images
 */

/**************************************************************************/
void Waveshare_2in7::clearDisplay() {
  clearBuffer();
  display();
  delay(100);
  display();
}

void Waveshare_2in7::readBusy() {
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

void Waveshare_2in7::selectLUT(uint8_t *wave_data) {
  uint8_t count;

  sendCmd(0x32);

  for (count = 0; count < 70; count++) {
    sendData(pgm_read_byte(&wave_data[count]));
  }
}

void Waveshare_2in7::deepSleep() {
  sendCmd(WS2IN7_DEEP_SLEEP);
  sendData(0xa5);
}

void Waveshare_2in7::partInit()
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

void Waveshare_2in7::partBaseImg()
{
  sendCmd(0x24); ////Write Black and White image to RAM

  for (uint16_t i = 0; i < bw_bufsize; i++) {
    sendData(bw_buf[i]);
  }

  sendCmd(0x26); ////Write Black and White image to RAM

  for (uint16_t i = 0; i < bw_bufsize; i++) {
    sendData(bw_buf[i]);
  }

  update();
}

void Waveshare_2in7::partUpdate()
{
  sendCmd(0x22);
  sendData(0x0C);
  sendCmd(0x20);
  readBusy();
}

void Waveshare_2in7::partDisplay(int16_t x_start, int16_t y_start, const unsigned char *datas, int16_t PART_COLUMN, int16_t PART_LINE)
{
  int16_t i;
  int16_t x_end, y_start1, y_start2, y_end1, y_end2;

  x_start = x_start / 8; //
  x_end   = x_start + PART_LINE / 8 - 1;

  y_start1 = 0;
  y_start2 = y_start;

  if (y_start >= 256)
  {
    y_start1 = y_start2 / 256;
    y_start2 = y_start2 % 256;
  }
  y_end1 = 0;
  y_end2 = y_start + PART_COLUMN - 1;

  if (y_end2 >= 256)
  {
    y_end1 = y_end2 / 256;
    y_end2 = y_end2 % 256;
  }

  sendCmd(0x44);      // set RAM x address start/end, in page 35
  sendData(x_start);  // RAM x address start at 00h;
  sendData(x_end);    // RAM x address end at 0fh(15+1)*8->128
  sendCmd(0x45);      // set RAM y address start/end, in page 35
  sendData(y_start2); // RAM y address start at 0127h;
  sendData(y_start1); // RAM y address start at 0127h;
  sendData(y_end2);   // RAM y address end at 00h;
  sendData(y_end1);   // ????=0

  sendCmd(0x4E);      // set RAM x address count to 0;
  sendData(x_start);
  sendCmd(0x4F);      // set RAM y address count to 0X127;
  sendData(y_start2);
  sendData(y_start1);

  sendCmd(0x24); // Write Black and White image to RAM

  for (i = 0; i < PART_COLUMN * PART_LINE / 8; i++)
  {
    sendData(pgm_read_byte(&datas[i]));
  }
  partUpdate();
}

const unsigned char lut_vcom_dc[] = {
  0x00, 0x00,
  0x00, 0x0F,0x0F,  0x00,  0x00,  0x05,
  0x00, 0x32,0x32,  0x00,  0x00,  0x02,
  0x00, 0x0F,0x0F,  0x00,  0x00,  0x05,
  0x00, 0x00,0x00,  0x00,  0x00,  0x00,
  0x00, 0x00,0x00,  0x00,  0x00,  0x00,
  0x00, 0x00,0x00,  0x00,  0x00,  0x00,
  0x00, 0x00,0x00,  0x00,  0x00,  0x00,
};

// R21H
const unsigned char lut_ww[] = {
  0x50, 0x0F, 0x0F, 0x00, 0x00, 0x05,
  0x60, 0x32, 0x32, 0x00, 0x00, 0x02,
  0xA0, 0x0F, 0x0F, 0x00, 0x00, 0x05,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// R22H    r
const unsigned char lut_bw[] =
{
  0x50, 0x0F, 0x0F, 0x00, 0x00, 0x05,
  0x60, 0x32, 0x32, 0x00, 0x00, 0x02,
  0xA0, 0x0F, 0x0F, 0x00, 0x00, 0x05,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// R24H    b
const unsigned char lut_bb[] =
{
  0xA0, 0x0F, 0x0F, 0x00, 0x00, 0x05,
  0x60, 0x32, 0x32, 0x00, 0x00, 0x02,
  0x50, 0x0F, 0x0F, 0x00, 0x00, 0x05,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// R23H    w
const unsigned char lut_wb[] =
{
  0xA0, 0x0F, 0x0F, 0x00, 0x00, 0x05,
  0x60, 0x32, 0x32, 0x00, 0x00, 0x02,
  0x50, 0x0F, 0x0F, 0x00, 0x00, 0x05,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
