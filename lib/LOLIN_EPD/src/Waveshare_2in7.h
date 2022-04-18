#ifndef __Waveshare_2in7_H
#define __Waveshare_2in7_H

#ifdef __AVR__
  # include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
 # include <pgmspace.h>
#else // ifdef __AVR__
 # define pgm_read_byte(addr) (*(const unsigned char *)(addr)) ///< read bytes from program memory
#endif // ifdef __AVR__

#include "LOLIN_EPD.h"

// EPD2IN7 commands
#define WS2IN7_PANEL_SETTING                        0x00
#define WS2IN7_POWER_SETTING                        0x01
#define WS2IN7_POWER_OFF                            0x02
#define WS2IN7_POWER_OFF_SEQUENCE_SETTING           0x03
#define WS2IN7_POWER_ON                             0x04
#define WS2IN7_POWER_ON_MEASURE                     0x05
#define WS2IN7_BOOSTER_SOFT_START                   0x06
#define WS2IN7_DEEP_SLEEP                           0x07
#define WS2IN7_DATA_START_TRANSMISSION_1            0x10
#define WS2IN7_DATA_STOP                            0x11
#define WS2IN7_DISPLAY_REFRESH                      0x12
#define WS2IN7_DATA_START_TRANSMISSION_2            0x13
#define WS2IN7_PARTIAL_DATA_START_TRANSMISSION_1    0x14 
#define WS2IN7_PARTIAL_DATA_START_TRANSMISSION_2    0x15 
#define WS2IN7_PARTIAL_DISPLAY_REFRESH              0x16
#define WS2IN7_LUT_FOR_VCOM                         0x20 
#define WS2IN7_LUT_WHITE_TO_WHITE                   0x21
#define WS2IN7_LUT_BLACK_TO_WHITE                   0x22
#define WS2IN7_LUT_WHITE_TO_BLACK                   0x23
#define WS2IN7_LUT_BLACK_TO_BLACK                   0x24
#define WS2IN7_PLL_CONTROL                          0x30
#define WS2IN7_TEMPERATURE_SENSOR_COMMAND           0x40
#define WS2IN7_TEMPERATURE_SENSOR_CALIBRATION       0x41
#define WS2IN7_TEMPERATURE_SENSOR_WRITE             0x42
#define WS2IN7_TEMPERATURE_SENSOR_READ              0x43
#define WS2IN7_VCOM_AND_DATA_INTERVAL_SETTING       0x50
#define WS2IN7_LOW_POWER_DETECTION                  0x51
#define WS2IN7_TCON_SETTING                         0x60
#define WS2IN7_TCON_RESOLUTION                      0x61
#define WS2IN7_SOURCE_AND_GATE_START_SETTING        0x62
#define WS2IN7_GET_STATUS                           0x71
#define WS2IN7_AUTO_MEASURE_VCOM                    0x80
#define WS2IN7_VCOM_VALUE                           0x81
#define WS2IN7_VCM_DC_SETTING_REGISTER              0x82
#define WS2IN7_PROGRAM_MODE                         0xA0
#define WS2IN7_ACTIVE_PROGRAM                       0xA1
#define WS2IN7_READ_OTP_DATA                        0xA2
#define WS2IN7_POWER_OPTIMIZATION                   0xF8

extern const unsigned char lut_vcom_dc[];
extern const unsigned char lut_ww[];
extern const unsigned char lut_bw[];
extern const unsigned char lut_bb[];
extern const unsigned char lut_wb[];

/**************************************************************************/

/*!
    @brief  Class for interfacing with IL3897 EPD drivers
 */

/**************************************************************************/
class Waveshare_2in7 : public LOLIN_EPD {
public:

  Waveshare_2in7(int    width,
                 int    height,
                 int8_t DC,
                 int8_t RST,
                 int8_t CS,
                 int8_t BUSY = -1);

  void begin(bool reset = true);

  void drawPixel(int16_t  x,
                 int16_t  y,
                 uint16_t color);

  void display();
  void update();

  void clearBuffer();
  void clearDisplay();

  void deepSleep();

  void partBaseImg();
  void partInit();
  void partDisplay(int16_t              x_start,
                   int16_t              y_start,
                   const unsigned char *datas,
                   int16_t              PART_COLUMN,
                   int16_t              PART_LINE);
  void partUpdate();

protected:

  void readBusy();
  void selectLUT(uint8_t *wave_data);
  void setLut();
  int _height_8bit; // height 8-bit alignment
};

#endif // ifndef __Waveshare_2in7_H
