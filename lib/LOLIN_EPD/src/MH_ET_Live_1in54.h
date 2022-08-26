#ifndef __MH_ET_LIVE_1in54_H
#define __MH_ET_LIVE_1in54_H

#ifdef __AVR__
  # include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
 # include <pgmspace.h>
#else // ifdef __AVR__
 # define pgm_read_byte(addr) (*(const unsigned char *)(addr)) ///< read bytes from program memory
#endif // ifdef __AVR__

#include "LOLIN_EPD.h"

// EPD1IN54B commands
// #define EPD1IN54B_PANEL_SETTING                    0x00
// #define EPD1IN54B_POWER_SETTING                    0x01
// #define EPD1IN54B_POWER_OFF                        0x02
// #define EPD1IN54B_POWER_OFF_SEQUENCE_SETTING       0x03
// #define EPD1IN54B_POWER_ON                         0x04
// #define EPD1IN54B_POWER_ON_MEASURE                 0x05
// #define EPD1IN54B_BOOSTER_SOFT_START               0x06
// #define EPD1IN54B_DEEP_SLEEP                       0x07
// #define EPD1IN54B_DATA_START_TRANSMISSION_1        0x10
// #define EPD1IN54B_DATA_STOP                        0x11
// #define EPD1IN54B_DISPLAY_REFRESH                  0x12
// #define EPD1IN54B_DATA_START_TRANSMISSION_2        0x13
// #define EPD1IN54B_PLL_CONTROL                      0x30
// #define EPD1IN54B_TEMPERATURE_SENSOR_COMMAND       0x40
// #define EPD1IN54B_TEMPERATURE_SENSOR_CALIBRATION   0x41
// #define EPD1IN54B_TEMPERATURE_SENSOR_WRITE         0x42
// #define EPD1IN54B_TEMPERATURE_SENSOR_READ          0x43
// #define EPD1IN54B_VCOM_AND_DATA_INTERVAL_SETTING   0x50
// #define EPD1IN54B_LOW_POWER_DETECTION              0x51
// #define EPD1IN54B_TCON_SETTING                     0x60
// #define EPD1IN54B_TCON_RESOLUTION                  0x61
// #define EPD1IN54B_SOURCE_AND_GATE_START_SETTING    0x62
// #define EPD1IN54B_GET_STATUS                       0x71
// #define EPD1IN54B_AUTO_MEASURE_VCOM                0x80
// #define EPD1IN54B_VCOM_VALUE                       0x81
// #define EPD1IN54B_VCM_DC_SETTING_REGISTER          0x82
// #define EPD1IN54B_PROGRAM_MODE                     0xA0
// #define EPD1IN54B_ACTIVE_PROGRAM                   0xA1
// #define EPD1IN54B_READ_OTP_DATA                    0xA2
// MH-ET Live EPD 1.54 Inch commands
#define MH_ET_DRIVER_OUTPUT_CONTROL                       0x01
#define MH_ET_BOOSTER_SOFT_START_CONTROL                  0x0C
#define MH_ET_GATE_SCAN_START_POSITION                    0x0F
#define MH_ET_DEEP_SLEEP_MODE                             0x10
#define MH_ET_DATA_ENTRY_MODE_SETTING                     0x11
#define MH_ET_SW_RESET                                    0x12
#define MH_ET_TEMPERATURE_SENSOR_CONTROL                  0x1A
#define MH_ET_MASTER_ACTIVATION                           0x20
#define MH_ET_DISPLAY_UPDATE_CONTROL_1                    0x21
#define MH_ET_DISPLAY_UPDATE_CONTROL_2                    0x22
#define MH_ET_WRITE_RAM                                   0x24
#define MH_ET_WRITE_RAM_RED                               0x26
#define MH_ET_WRITE_VCOM_REGISTER                         0x2C
#define MH_ET_WRITE_LUT_REGISTER                          0x32
#define MH_ET_SET_DUMMY_LINE_PERIOD                       0x3A
#define MH_ET_SET_GATE_LINE_WIDTH                         0x3B
#define MH_ET_BORDER_WAVEFORM_CONTROL                     0x3C
#define MH_ET_SET_RAM_X_ADDRESS_START_END_POSITION        0x44
#define MH_ET_SET_RAM_Y_ADDRESS_START_END_POSITION        0x45
#define MH_ET_SET_RAM_X_ADDRESS_COUNTER                   0x4E
#define MH_ET_SET_RAM_Y_ADDRESS_COUNTER                   0x4F
#define MH_ET_NOP                                         0xFF

/**************************************************************************/

/*!
    @brief  Class for interfacing with MH-ET Live 1.54" 200x200 drivers
 */

/**************************************************************************/
class MH_ET_Live_1in54 : public LOLIN_EPD {
public:

  MH_ET_Live_1in54(int    width,
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
  int _height_8bit; // height 8-bit alignment

private:

  int _width;
  int _height;
  void setLut(const unsigned char *lut);
};

#endif // ifndef __MH_ET_LIVE_1in54_H
