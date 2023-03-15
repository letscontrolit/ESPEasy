#ifndef PLUGINSTRUCTS_P068_DATA_STRUCT_H
#define PLUGINSTRUCTS_P068_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P068

// ==============================================
// P068_SHT3X LIBRARY - SHT3X.h
// =============================================

class P068_SHT3X : public PluginTaskData_base {
public:

  P068_SHT3X(uint8_t addr);
  P068_SHT3X() = delete;
  virtual ~P068_SHT3X() = default;

  void        readFromSensor(void);
  static bool CRC8(uint8_t MSB,
                   uint8_t LSB,
                   uint8_t CRC);

  float tmp    = 0.0f;
  float hum    = 0.0f;
  float tmpOff = 0.0f;

private:

  uint8_t _i2c_device_address;
};


#endif // ifdef USES_P068
#endif // ifndef PLUGINSTRUCTS_P068_DATA_STRUCT_H
