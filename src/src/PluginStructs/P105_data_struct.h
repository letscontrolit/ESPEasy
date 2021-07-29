#ifndef PLUGINSTRUCTS_P105_DATA_STRUCT_H
#define PLUGINSTRUCTS_P105_DATA_STRUCT_H

#ifdef USES_P105

# include "../../_Plugin_Helper.h"

// https://wiki.liutyi.info/display/ARDUINO/AHT10
// https://github.com/enjoyneering/AHT10/blob/master/src/AHT10.h
// https://github.com/adafruit/Adafruit_AHTX0
// https://github.com/arendst/Tasmota/blob/0650744ac27108931a070918f08173d71ddfd68d/tasmota/xsns_63_aht1x.ino

# define AHT10_ADDRESS_0X38         0x38  // chip I2C address no.1 for AHT10/AHT15/AHT20, address pin connected to GND
# define AHT10_ADDRESS_0X39         0x39  // chip I2C address no.2 for AHT10 only, address pin connected to Vcc

# define AHT10_INIT_CMD             0xE1  // initialization command for AHT10/AHT15
# define AHT20_INIT_CMD             0xBE  // initialization command for AHT20
# define AHT10_START_MEASURMENT_CMD 0xAC  // start measurment command
# define AHT10_NORMAL_CMD           0xA8  // normal cycle mode command, no info in datasheet!!!
# define AHT10_SOFT_RESET_CMD       0xBA  // soft reset command

# define AHT10_INIT_NORMAL_MODE     0x00  // enable normal mode
# define AHT10_INIT_CYCLE_MODE      0x20  // enable cycle mode
# define AHT10_INIT_CMD_MODE        0x40  // enable command mode
# define AHT10_INIT_CAL_ENABLE      0x08  // load factory calibration coeff


# define AHT10_DATA_MEASURMENT_CMD  0x33  // no info in datasheet!!! my guess it is DAC resolution, saw someone send 0x00 instead
# define AHT10_DATA_NOP             0x00  // no info in datasheet!!!


# define AHT10_MEASURMENT_DELAY     80    // at least 75 milliseconds
# define AHT10_POWER_ON_DELAY       40    // at least 20..40 milliseconds
# define AHT10_CMD_DELAY            350   // at least 300 milliseconds, no info in datasheet!!!
# define AHT10_SOFT_RESET_DELAY     20    // less than 20 milliseconds

# define AHT10_FORCE_READ_DATA      true  // force to read data
# define AHT10_USE_READ_DATA        false // force to use data from previous read
# define AHT10_ERROR                0xFF  // returns 255, if communication error is occurred


# define AHTX0_I2CADDR_DEFAULT 0x38       ///< AHT default i2c address
# define AHTX0_CMD_CALIBRATE 0xE1         ///< Calibration command
# define AHTX0_CMD_TRIGGER 0xAC           ///< Trigger reading command
# define AHTX0_CMD_SOFTRESET 0xBA         ///< Soft reset command
# define AHTX0_STATUS_BUSY 0x80           ///< Status bit for busy
# define AHTX0_STATUS_CALIBRATED 0x08     ///< Status bit for calibrated


# ifdef USE_AHT2x
  #  define AHTX_CMD     0xB1 // Cmd for AHT2x
# else // ifdef USE_AHT2x
  #  define AHTX_CMD     0xE1 // Cmd for AHT1x
# endif // ifdef USE_AHT2x


enum AHT_state {
  Uninitialized = 0,
  Initialized,
  Wait_for_samples,
  New_values,
  Values_read
};


struct P105_data_struct : public PluginTaskData_base {
  P105_data_struct(uint8_t addr);

  bool                 initialized() const;

  bool                 begin();

  // Only perform the measurements with big interval to prevent the sensor from warming up.
  bool                 update(unsigned long task_index);

  bool                 startMeasurement();
  bool                 readMeasurement();
  unsigned inline char readStatus(uint8_t address);

  // **************************************************************************/
  // Read temperature
  // **************************************************************************/
  float readTemperature();

  // **************************************************************************/
  // Read humidity
  // **************************************************************************/
  float readHumidity();


  uint8_t       AHT10_rawDataBuffer[6] = { AHT10_ERROR, 0, 0, 0, 0, 0 };
  float         last_hum_val           = 0.0f;
  float         last_temp_val          = 0.0f;
  unsigned long last_measurement       = 0;
  uint8_t       i2cAddress             = 0;
  AHT_state     state                  = AHT_state::Uninitialized;
};

#endif // ifdef USES_P105

#endif // PLUGINSTRUCTS_P105_DATA_STRUCT_H
