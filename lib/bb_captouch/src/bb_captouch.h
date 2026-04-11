//
// BitBank Capacitive Touch Sensor Library
// written by Larry Bank
//
// Copyright 2023 BitBank Software, Inc. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//    http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ===========================================================================

//
// Written for the many variants of ESP32 + Capacitive touch LCDs on the market
//
// 2024-06-10 tonhuisman: Add support for CHSC5816 touchscreen, code borrowed from Lewis He SensorLib at
//                        https://github.com/lewisxhe/SensorLib
// 2024-06-02 tonhuisman: Fix FT62x6 to not return true when no data is read
//                        Formatted source using Uncrustify
//                        Adjust I2C handling so ESPEasy stays happy, ignoring SDA/SCL pins and I2C enable/disable
// ===========================================================================

#include <Arduino.h>
#include <Wire.h>

#ifndef __BB_CAPTOUCH__
# define __BB_CAPTOUCH__

# define CT_SUCCESS 0
# define CT_ERROR -1

enum {
  CT_TYPE_UNKNOWN = 0,
  CT_TYPE_FT6X36,
  CT_TYPE_GT911,
  CT_TYPE_CST820,
  CT_TYPE_CST226,
  CT_TYPE_AXS15231,
  CT_TYPE_CHSC5816,
  CT_TYPE_COUNT
};

# define GT911_ADDR1    0x5D
# define GT911_ADDR2    0x14
# define FT6X36_ADDR    0x38
# define CST820_ADDR    0x15
# define CST226_ADDR    0x5A
# define AXS15231_ADDR  0x3B
# define CHSC5816_ADDR  0x2E

// CST8xx gestures
enum {
  GESTURE_NONE = 0,
  GESTURE_SWIPE_UP,
  GESTURE_SWIPE_DOWN,
  GESTURE_SWIPE_LEFT,
  GESTURE_SWIPE_RIGHT,
  GESTURE_SINGLE_CLICK,
  GESTURE_DOUBLE_CLICK = 0x0B,
  GESTURE_LONG_PRESS   = 0x0C
};

// CST820 registers
# define CST820_TOUCH_REGS 1

// GT911 registers
# define GT911_POINT_INFO   0x814E
# define GT911_POINT_1      0x814F
# define GT911_CONFIG_FRESH 0x8100
# define GT911_CONFIG_SIZE  0xb9
# define GT911_CONFIG_START 0x8047

// FT6x36 registers
# define TOUCH_REG_MODE   0x00
# define TOUCH_REG_STATUS 0x02
# define TOUCH_REG_XH     0x03
# define TOUCH_REG_XL     0x04
# define TOUCH_REG_YH     0x05
# define TOUCH_REG_YL     0x06
# define TOUCH_REG_WEIGHT 0x07
# define TOUCH_REG_AREA   0x08
# define TOUCH_REG_CHIPID 0xA3 // !< Chip selecting
# define TOUCH_REG_VENDID 0xA8 // !< FocalTech's panel ID

// FT62x6 Vendor and Chip IDs
# define FT62XX_VENDID  0x11   // !< FocalTech's vendor ID
# define FT6206_CHIPID  0x06   // !< Chip selecting
# define FT6236_CHIPID  0x36   // !< Chip selecting
# define FT6236U_CHIPID 0x64   // !< Chip selecting
# define FT5316_CHIPID  0x0A   // !< Chip selecting

// register offset to info for the second touch point
# define PT2_OFFSET 6

union __CHSC5816_PointReg {
  struct {
    uint8_t status;
    uint8_t fingerNumber;
    uint8_t x_l8;
    uint8_t y_l8;
    uint8_t z;
    uint8_t x_h4  : 4;
    uint8_t y_h4  : 4;
    uint8_t id    : 4;
    uint8_t event : 4;
    uint8_t p2;
  }             rp;
  unsigned char data[8];
};

# ifndef __TOUCHINFO_STRUCT__
#  define __TOUCHINFO_STRUCT__
typedef struct _fttouchinfo {
  int      count;
  uint16_t x[5], y[5];
  uint8_t  pressure[5], area[5];
} TOUCHINFO;
# endif // ifndef __TOUCHINFO_STRUCT__

class BBCapTouch {
public:

  BBCapTouch() {
    _iOrientation = 0;
  }

  ~BBCapTouch() { /*Wire.end();*/
  }               // ESPEasy doesn't allow this

  int init(int      iSDA,
           int      iSCL,
           int      iRST     = -1,
           int      iINT     = -1,
           uint32_t u32Speed = 400000);
  int  getSamples(TOUCHINFO *pTI);
  void sensorType(int iType);
  int  sensorType(void) const {
    return _iType;
  }

  void setI2CAddress(int i2caddr);
  int  getI2CAddress(void) const {
    return _iAddr;
  }

  int     setOrientation(int iOrientation,
                         int iWidth,
                         int iHeight);
  void    setThreshold(int tresh);
  uint8_t getChipId(void) const {
    return _id; // Only set for FT62x6 family
  }

protected:

  void reset(int iResetPin);

private:

  int _iAddr;
  int _iType = CT_TYPE_UNKNOWN;
  int _iOrientation, _iWidth{}, _iHeight{};
  int _thresh = -1;
  uint8_t _id = 0;

  void fixSamples(TOUCHINFO *pTI);
  bool I2CTest(uint8_t u8Addr);
  int  I2CRead(uint8_t  u8Addr,
               uint8_t *pData,
               int      iLen);
  int  I2CReadRegister(uint8_t  u8Addr,
                       uint8_t  u8Register,
                       uint8_t *pData,
                       int      iLen);
  int I2CReadRegister16(uint8_t  u8Addr,
                        uint16_t u16Register,
                        uint8_t *pData,
                        int      iLen);
  int I2CWrite(uint8_t  u8Addr,
               uint8_t *pData,
               int      iLen);
}; // class BBCapTouch
#endif // __BB_CAPTOUCH__
