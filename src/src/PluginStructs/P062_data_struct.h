#ifndef PLUGINSTRUCTS_P062_DATA_STRUCT_H
#define PLUGINSTRUCTS_P062_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P062

// #define PLUGIN_062_DEBUG  // Enable extra (debugging) log output

# include <Adafruit_MPR121.h>

#define P062_MaxTouchObjects 12 // Max. number of (separate) touch inputs

struct P062_data_struct : public PluginTaskData_base {
public:

  P062_data_struct();
  bool init(taskIndex_t taskIndex,
            uint8_t     i2c_addr,
            bool        scancode,
            bool        keepCalibrationData);

  bool readKey(uint16_t& key);
  void setThresholds(uint8_t touch, uint8_t release);
  void setThreshold(uint8_t t, uint8_t touch, uint8_t release);
  bool getCalibrationData(uint8_t t, uint16_t *current, uint16_t *min, uint16_t *max);
  void clearCalibrationData();
  void loadTouchObjects(taskIndex_t taskIndex);

  /**
   * Structs for StoredSettings
   */
  struct tP062_Sensitivity {
    uint8_t touch = 0;
    uint8_t release = 0;
  };

  struct tP062_StoredSettings_struct {
    tP062_Sensitivity TouchObjects[P062_MaxTouchObjects];
  };

  tP062_StoredSettings_struct StoredSettings;

  /**
   * Structs for Calbration values
   */
  struct tP062_CalibrationValue {
    uint16_t current = 0;
    uint16_t min = 0;
    uint16_t max = 0;
  };

  struct tP062_CalibrationData_struct {
    tP062_CalibrationValue CalibrationValues[P062_MaxTouchObjects];
  };

  tP062_CalibrationData_struct CalibrationData;

private:
  void updateCalibration(uint8_t t);

  Adafruit_MPR121 *keypad = NULL;
  uint16_t        keyLast = 0;
  int8_t          _i2c_addr = -1;
  bool            _use_scancode;
  bool            _keepCalibrationData;
};

#endif // ifdef USES_P062
#endif // ifndef PLUGINSTRUCTS_P062_DATA_STRUCT_H
