#ifndef PLUGINSTRUCTS_P028_DATA_STRUCT_H
#define PLUGINSTRUCTS_P028_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P028


# define BMx280_REGISTER_DIG_T1           0x88
# define BMx280_REGISTER_DIG_T2           0x8A
# define BMx280_REGISTER_DIG_T3           0x8C

# define BMx280_REGISTER_DIG_P1           0x8E
# define BMx280_REGISTER_DIG_P2           0x90
# define BMx280_REGISTER_DIG_P3           0x92
# define BMx280_REGISTER_DIG_P4           0x94
# define BMx280_REGISTER_DIG_P5           0x96
# define BMx280_REGISTER_DIG_P6           0x98
# define BMx280_REGISTER_DIG_P7           0x9A
# define BMx280_REGISTER_DIG_P8           0x9C
# define BMx280_REGISTER_DIG_P9           0x9E

# define BMx280_REGISTER_DIG_H1           0xA1
# define BMx280_REGISTER_DIG_H2           0xE1
# define BMx280_REGISTER_DIG_H3           0xE3
# define BMx280_REGISTER_DIG_H4           0xE4
# define BMx280_REGISTER_DIG_H5           0xE5
# define BMx280_REGISTER_DIG_H6           0xE7

# define BMx280_REGISTER_CHIPID           0xD0
# define BMx280_REGISTER_VERSION          0xD1
# define BMx280_REGISTER_SOFTRESET        0xE0

# define BMx280_REGISTER_CAL26            0xE1 // R calibration stored in 0xE1-0xF0

# define BMx280_REGISTER_CONTROLHUMID     0xF2
# define BMx280_REGISTER_STATUS           0xF3
# define BMx280_REGISTER_CONTROL          0xF4
# define BMx280_REGISTER_CONFIG           0xF5
# define BMx280_REGISTER_PRESSUREDATA     0xF7
# define BMx280_REGISTER_TEMPDATA         0xFA
# define BMx280_REGISTER_HUMIDDATA        0xFD

# define BME280_CONTROL_SETTING_HUMIDITY  0x02 // Oversampling: 2x H

# define BME280_TEMP_PRESS_CALIB_DATA_ADDR       0x88
# define BME280_HUMIDITY_CALIB_DATA_ADDR         0xE1
# define BME280_DATA_ADDR                        0xF7

# define BME280_TEMP_PRESS_CALIB_DATA_LEN        26
# define BME280_HUMIDITY_CALIB_DATA_LEN          7
# define BME280_P_T_H_DATA_LEN                   8

# define P028_ERROR_IGNORE        0
# define P028_ERROR_MIN_RANGE     1
# define P028_ERROR_ZERO          2
# define P028_ERROR_MAX_RANGE     3
# define P028_ERROR_NAN           4
# define P028_ERROR_MIN_K         5

# define P028_I2C_ADDRESS         PCONFIG(0)
# define P028_ALTITUDE            PCONFIG(1)
# define P028_TEMPERATURE_OFFSET  PCONFIG(2)
# define P028_ERROR_STATE_OUTPUT  PCONFIG(3)

struct P028_data_struct : public PluginTaskData_base {
  struct bme280_calib_data
  {
    uint16_t dig_T1 = 0;
    int16_t  dig_T2 = 0;
    int16_t  dig_T3 = 0;

    uint16_t dig_P1 = 0;
    int16_t  dig_P2 = 0;
    int16_t  dig_P3 = 0;
    int16_t  dig_P4 = 0;
    int16_t  dig_P5 = 0;
    int16_t  dig_P6 = 0;
    int16_t  dig_P7 = 0;
    int16_t  dig_P8 = 0;
    int16_t  dig_P9 = 0;

    uint8_t dig_H1 = 0;
    int16_t dig_H2 = 0;
    uint8_t dig_H3 = 0;
    int16_t dig_H4 = 0;
    int16_t dig_H5 = 0;
    int8_t  dig_H6 = 0;
    int32_t t_fine = 0;
  };

  struct bme280_uncomp_data {
    /*! un-compensated pressure */
    uint32_t pressure = 0;

    /*! un-compensated temperature */
    uint32_t temperature = 0;

    /*! un-compensated humidity */
    uint32_t humidity = 0;
  };

  enum BMx_ChipId {
    Unknown_DEVICE        = 0,
    BMP280_DEVICE_SAMPLE1 = 0x56,
    BMP280_DEVICE_SAMPLE2 = 0x57,
    BMP280_DEVICE         = 0x58,
    BME280_DEVICE         = 0x60
  };

  enum BMx_state {
    BMx_Uninitialized = 0,
    BMx_Initialized,
    BMx_Wait_for_samples,
    BMx_New_values,
    BMx_Values_read,
    BMx_Error
  };


  P028_data_struct(uint8_t addr,
                   float   tempOffset);
  P028_data_struct() = delete;
  virtual ~P028_data_struct() = default;

private:

  uint8_t get_config_settings() const;

  uint8_t get_control_settings() const;

public:

  const __FlashStringHelper* getDeviceName() const;

  bool                       hasHumidity() const;

  bool                       initialized() const;

private:

  void setUninitialized();

  bool measurementInProgress() const;

public:

  void startMeasurement();

  bool updateMeasurements(taskIndex_t task_index);

private:

  // **************************************************************************/
  // Check BME280 presence
  // **************************************************************************/
  bool check();

  // **************************************************************************/
  // Initialize BME280
  // **************************************************************************/
  bool begin();

private:

  // **************************************************************************/
  // Reads the factory-set coefficients
  // **************************************************************************/
  void readCoefficients();

  bool readUncompensatedData();

  // **************************************************************************/
  // Read temperature
  // Needs to be processed first as it updates calib data
  // **************************************************************************/
  float readTemperature();

  // **************************************************************************/
  // Read pressure
  // **************************************************************************/
  float readPressure() const;

  // **************************************************************************/
  // Read humidity
  // **************************************************************************/
  float readHumidity() const;

  const uint8_t i2cAddress  = 0;
  const float   temp_offset = 0.0f;

  bme280_uncomp_data uncompensated;
  bme280_calib_data  calib;

  unsigned long last_measurement = 0;

public:

  float last_hum_val      = 0.0f;
  float last_press_val    = 0.0f;
  float last_temp_val     = 0.0f;
  float last_dew_temp_val = 0.0f;

  BMx_state  state                = BMx_Uninitialized;
  BMx_ChipId sensorID             = Unknown_DEVICE;
  bool       lastMeasurementError = false; // Keep track of measurement errors
};

#endif // ifdef USES_P028

#endif // PLUGINSTRUCTS_P028_DATA_STRUCT_H
