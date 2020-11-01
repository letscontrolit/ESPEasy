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

#define P099_MaxObjectNameLength 15 // 14 character objectnames + terminating 0
#define P099_MaxObjectCount      40 // This count of touchobjects should be enough, because of limited settings storage, 960 bytes + 8 bytes calibration coordinates

#define P099_FLAGS_ON_OFF_BUTTON 0 // TouchObjects.flags On/Off Button function
#define P099_FLAGS_INVERT_BUTTON 1 // TouchObjects.flags Inverted On/Off Button function

// Data structure
struct P099_data_struct : public PluginTaskData_base
{
  P099_data_struct();
  ~P099_data_struct();

  void      reset();
  bool init(taskIndex_t taskIndex,
            uint8_t     cs,
            uint8_t     rotation,
            bool        flipped,
            uint8_t     z_treshold,
            bool        send_xy,
            bool        send_z,
            bool        useCalibration,
            uint16_t    ts_x_res,
            uint16_t    ts_y_res);
  bool isInitialized() const;
  void loadTouchObjects(taskIndex_t taskIndex);
  bool touched();
	void readData(uint16_t *x, uint16_t *y, uint8_t *z);
  void setRotation(uint8_t n);
  void setRotationFlipped(bool _flipped);
  bool isCalibrationActive();
  bool isValidAndTouchedTouchObject(uint16_t x, uint16_t y, String &selectedObjectName, int &selectedObjectIndex, uint8_t checkObjectCount);
  bool setTouchObjectState(String touchObject, bool state, uint8_t checkObjectCount);
  void scaleRawToCalibrated(uint16_t &x, uint16_t &y);

  // This is initialized by calling init()
  XPT2046_Touchscreen *touchscreen;
  uint8_t  _address_ts_cs;
  uint8_t  _rotation;
  bool     _flipped;
  uint8_t  _z_treshold;
  bool     _send_xy;
  bool     _send_z;
  bool     _useCalibration;
  uint16_t _ts_x_res;
  uint16_t _ts_y_res;

  // This is filled during checking of a touchobject
  uint32_t SurfaceAreas[P099_MaxObjectCount];

  // Counters for debouncing touch button
  uint32_t TouchTimers[P099_MaxObjectCount];
  bool     TouchStates[P099_MaxObjectCount];

  // The settings structures
  // Lets define our own coordinate point
  struct tP099_Point
  {
    uint16_t x = 0;
    uint16_t y = 0;
  };

  // For touch objects we store a name and 2 coordinates
  struct tP099_Touchobjects
  {
    char        objectname[P099_MaxObjectNameLength] = { 0 };
    byte        flags = 0;
    tP099_Point top_left;
    tP099_Point bottom_right;
  };

  // Only 2 coordinates used for calibration (we must assume that the touch panel is mounted straight on to tft)
  struct tP099_Calibration
  {
    tP099_Point top_left;
    tP099_Point bottom_right;
  };

  // The stuff we want to save between settings (Calibration coordinates and touchable objects)
  struct tP099_StoredSettings_struct
  {
    tP099_Calibration  Calibration;
    tP099_Touchobjects TouchObjects[P099_MaxObjectCount];
  };

  // Stored settings data:
  tP099_StoredSettings_struct StoredSettings;
};

#endif  // ifdef USED_P099
#endif  // ifndef PLUGINSTRUCTS_P099_DATA_STRUCT_H