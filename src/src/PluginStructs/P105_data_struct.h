#ifndef PLUGINSTRUCTS_P105_DATA_STRUCT_H
#define PLUGINSTRUCTS_P105_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P105

enum class AHTx_device_type : uint8_t {
  AHT10_DEVICE = 10,
  AHT20_DEVICE = 20,
  AHT21_DEVICE = 21,
};

enum class AHTx_state : uint8_t {
  AHTx_Uninitialized = 0,
  AHTx_Initialized,
  AHTx_Trigger_measurement,
  AHTx_Wait_for_samples,
  AHTx_New_values,
  AHTx_Values_read
};

class AHTx_Device {
public:

  AHTx_Device(uint8_t          addr,
              AHTx_device_type type);
  AHTx_Device() = delete;

  const __FlashStringHelper* getDeviceName() const;

  inline float               getHumidity() const {
    return last_hum_val;
  }

  inline float getTemperature() const {
    return last_temp_val;
  }

  bool    initialize();
  bool    triggerMeasurement();
  bool    softReset();
  uint8_t readStatus();
  bool    readData();

protected:

  const uint8_t i2cAddress;
  const AHTx_device_type device_type;
  float last_hum_val  = 0.0f;
  float last_temp_val = 0.0f;
};

struct P105_data_struct : public PluginTaskData_base {
  P105_data_struct(uint8_t          addr,
                   AHTx_device_type dev);
  P105_data_struct()          = delete;
  virtual ~P105_data_struct() = default;

  inline String getDeviceName() const {
    return device.getDeviceName();
  }

  inline float getHumidity() const {
    return device.getHumidity();
  }

  inline float getTemperature() const {
    return device.getTemperature();
  }

  bool initialized() const;

  void setUninitialized();

  // Perform the measurements with interval
  bool updateMeasurements(taskIndex_t task_index);

  AHTx_Device   device;
  AHTx_state    state;
  unsigned long last_measurement;
  unsigned long trigger_time;
};

#endif // ifdef USES_P105

#endif // PLUGINSTRUCTS_P105_DATA_STRUCT_H
