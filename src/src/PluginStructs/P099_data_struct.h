#ifndef PLUGINSTRUCTS_P099_DATA_STRUCT_H
#define PLUGINSTRUCTS_P099_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#include "../../ESPEasy_common.h"

#ifdef USES_P099

#include <XPT2046_Touchscreen.h>

// #define PLUGIN_099_DEBUG    // Additional debugging information

// Define default values for both ESP32/lolin32 and D1 Mini
#ifdef ESP32
  #define P099_TS_CS 12
#else // ESP8266/ESP8285
  #define P099_TS_CS  0 // D3
#endif // ESP32

#define P099_TS_TRESHOLD        15    // Treshold before the value is registered as a proper touch
#define P099_TS_ROTATION        2     // Rotation 0-3 = 180/270/0/90 degrees, should be set to the same rotation degrees, not 0-3 value!, as the screen it is mounted on
#define P099_TS_SEND_XY         true  // Enable X/Y events
#define P099_TS_SEND_Z          false // Disable Z events
#define P099_TS_SEND_OBJECTNAME true  // Enable objectname events
#define P099_TS_USE_CALIBRATION false // Disable calibration
#define P099_TS_LOG_CALIBRATION true  // Enable calibration logging
#define P099_TS_X_RES           240   // Pixels, should match with the screen it is mounted on
#define P099_TS_Y_RES           320
#define P099_INIT_OBJECTCOUNT   8     // Initial setting

#define P099_TOUCH_X_INVALID  4095 // When picking up spurious noise (or an open/not connected TS-CS pin), these are the values that turn up
#define P099_TOUCH_Y_INVALID  4095
#define P099_TOUCH_Z_INVALID  255

#define P099_MaxObjectNameLength 16 // 15 character objectnames + terminating 0
#define P099_MaxObjectCount      40 // This count of touchobjects should be enough, because of limited settings storage, 960 bytes + 8 bytes calibration coordinates

// The setting structures
typedef struct
{
  uint16_t x = 0;
  uint16_t y = 0;
} tP099_Point;

typedef struct
{
  char        objectname[P099_MaxObjectNameLength] = { 0 };
  tP099_Point top_left;
  tP099_Point bottom_right;
} tP099_Touchobjects;

typedef struct {
  tP099_Point top_left;
  tP099_Point bottom_right;
} tP099_Calibration;

struct P099_data_struct : public PluginTaskData_base
{
  P099_data_struct();
  ~P099_data_struct();

  void      reset();
  bool init(taskIndex_t taskIndex,
            uint8_t  _cs,
            uint8_t  _rotation,
            uint8_t  _z_treshold,
            bool     _send_xy,
            bool     _send_z,
            bool     _useCalibration,
            uint16_t _ts_x_res,
            uint16_t _ts_y_res);
  bool isInitialized() const;
  void loadTouchObjects(taskIndex_t taskIndex);
  bool touched();
	void readData(uint16_t *x, uint16_t *y, uint8_t *z);
  void setRotation(uint8_t n);
  bool isCalibrationActive();
  bool isValidAndTouchedTouchObject(uint16_t x, uint16_t y, String &selectedObjectName, uint8_t checkObjectCount);
  void scaleRawToCalibrated(uint16_t &x, uint16_t &y);

  XPT2046_Touchscreen *touchscreen;
  uint8_t  address_ts_cs;
  uint8_t  rotation;
  uint8_t  z_treshold;
  bool     send_xy;
  bool     send_z;
  bool     useCalibration;
  uint16_t ts_x_res;
  uint16_t ts_y_res;
  uint32_t           SurfaceAreas[P099_MaxObjectCount];
  // Keep these 2 struct implementations together and in this order, as they are saved/restored as 1 block.
  tP099_Calibration  Calibration;
  tP099_Touchobjects TouchObjects[P099_MaxObjectCount];
  // End of saved data
};

#endif  // ifdef USED_P099
#endif  // ifndef PLUGINSTRUCTS_P099_DATA_STRUCT_H