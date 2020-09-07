#ifndef PLUGINSTRUCTS_P045_DATA_STRUCT_H
#define PLUGINSTRUCTS_P045_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#include "../../ESPEasy_common.h"

#ifdef USES_P045

struct P045_data_struct : public PluginTaskData_base {
public:

  P045_data_struct(uint8_t i2c_addr);

  void loop();

private:

  void trackMinMax(int16_t  current,
                   int16_t *min,
                   int16_t *max);

  /** Get raw 6-axis motion sensor readings (accel/gyro).
   * Retrieves all currently available motion sensor values.
   * @param ax 16-bit signed integer container for accelerometer X-axis value
   * @param ay 16-bit signed integer container for accelerometer Y-axis value
   * @param az 16-bit signed integer container for accelerometer Z-axis value
   * @param gx 16-bit signed integer container for gyroscope X-axis value
   * @param gy 16-bit signed integer container for gyroscope Y-axis value
   * @param gz 16-bit signed integer container for gyroscope Z-axis value
   */
  void getRaw6AxisMotion(int16_t *ax,
                         int16_t *ay,
                         int16_t *az,
                         int16_t *gx,
                         int16_t *gy,
                         int16_t *gz);

  /** Write multiple bits in an 8-bit device register.
   * @param regAddr Register regAddr to write to
   * @param bitStart First bit position to write (0-7)
   * @param length Number of bits to write (not more than 8)
   * @param data Right-aligned value to write
   */
  void writeBits(uint8_t regAddr,
                 uint8_t bitStart,
                 uint8_t length,
                 uint8_t data);

public:

  int16_t _axis[3][5]; // [xyz], [min/max/range,a,g]

private:

  unsigned long _timer = 0; // Timer to check values each 5 seconds
  uint8_t       i2cAddress;
};
#endif // ifdef USES_P045
#endif // ifndef PLUGINSTRUCTS_P045_DATA_STRUCT_H
