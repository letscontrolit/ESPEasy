#ifndef PLUGINSTRUCTS_P014_DATA_STRUCT_H
#define PLUGINSTRUCTS_P014_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P014

// #######################################################################################################
// ######################## Plugin 014 SI7021 I2C Temperature Humidity Sensor  ###########################
// #######################################################################################################
// 12-10-2015 Charles-Henri Hallard, see my projects and blog at https://hallard.me


// ======================================
// SI7021 sensor
// ======================================
# define SI7021_I2C_ADDRESS      0x40 // I2C address for the sensor
# define SI7021_MEASURE_TEMP_HUM 0xE0 // Measure Temp only after a RH conversion done
# define SI7021_MEASURE_TEMP_HM  0xE3 // Default hold Master
# define SI7021_MEASURE_HUM_HM   0xE5 // Default hold Master
# define SI7021_MEASURE_TEMP     0xF3 // No hold
# define SI7021_MEASURE_HUM      0xF5 // No hold
# define SI7021_WRITE_REG        0xE6
# define SI7021_READ_REG         0xE7
# define SI7021_SOFT_RESET       0xFE

// SI7021 Sensor resolution
// default at power up is SI7021_RESOLUTION_14T_12RH
# define SI7021_RESOLUTION_14T_12RH 0x00 // 12 bits RH / 14 bits Temp
# define SI7021_RESOLUTION_13T_10RH 0x80 // 10 bits RH / 13 bits Temp
# define SI7021_RESOLUTION_12T_08RH 0x01 //  8 bits RH / 12 bits Temp
# define SI7021_RESOLUTION_11T_11RH 0x81 // 11 bits RH / 11 bits Temp
# define SI7021_RESOLUTION_MASK 0B01111110


# define SI7021_TIMEOUT         1000

enum class SI7021_state {
  Uninitialized = 0,
  Initialized,
  Wait_for_temperature_samples,
  Wait_for_humidity_samples,
  New_values,
  Values_read
};

struct P014_data_struct : public PluginTaskData_base {
  P014_data_struct(uint8_t resolution);

  void reset();

  bool init();

  bool loop();

  bool getReadValue(float& temperature,
                    float& humidity);

  /* ======================================================================
     Function: checkCRC
     Purpose : check the CRC of received data
     Input   : value read from sensor
     Output  : CRC read from sensor
     Comments: 0 if okay
     ====================================================================== */
  static uint8_t checkCRC(uint16_t data,
                          uint8_t  check);

  /* ======================================================================
     Function: si7021_readRegister
     Purpose : read the user register from the sensor
     Input   : user register value filled by function
     Output  : 0 if okay
     Comments: -
     ====================================================================== */
  static int8_t  readRegister(uint8_t *value);

  /* ======================================================================
     Function: startConv
     Purpose : return temperature or humidity measured
     Input   : data type SI7021_READ_HUM or SI7021_READ_TEMP
     Output  : 0 if okay
     Comments: -
     ====================================================================== */
  static uint8_t startConv(uint8_t datatype);

  /* ======================================================================
     Function: readValues
     Purpose : read temperature and humidity from SI7021 sensor
     Input   : current config resolution
     Output  : 0 if okay
     Comments: -
     ====================================================================== */
  int8_t         readValues(uint8_t datatype,
                            uint8_t resolution);

  /* ======================================================================
     Function: setResolution
     Purpose : Sets the sensor resolution to one of four levels
     Input   : see #define default is SI7021_RESOLUTION_14T_12RH
     Output  : 0 if okay
     Comments: -
     ====================================================================== */
  int8_t setResolution(uint8_t res);

  unsigned long timeStartRead      = 0; // Timestamp when started reading sensor
  uint16_t      si7021_humidity    = 0; // latest humidity value read
  int16_t       si7021_temperature = 0; // latest temperature value read (*100)
  SI7021_state  state              = SI7021_state::Uninitialized;
  uint8_t       res                = 0;
};

#endif // ifdef USES_P014
#endif // ifndef PLUGINSTRUCTS_P014_DATA_STRUCT_H
