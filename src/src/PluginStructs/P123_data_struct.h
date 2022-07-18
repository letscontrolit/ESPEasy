#ifndef PLUGINSTRUCTS_P123_DATA_STRUCT_H
#define PLUGINSTRUCTS_P123_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"

#ifdef USES_P123

// ======================================
// SI7013 sensor
// ======================================
#define SI7021_I2C_ADDRESS      0x40 // I2C address for the sensor
#define SI7013_I2C_ADDRESS      0x41 // I2C address for the sensor
#define SI7013_READ_TEMP_FROM_HUM    0xE0 // Read Temp only after a RH conversion done this does not have checksum byte

#define SI7013_MEASURE_HUM      0xF5 // No hold
#define SI7013_MEASURE_TEMP     0xF3 // No hold

#define SI7013_MEASURE_TEMP_HM  0xE3 // Default hold Master
#define SI7013_MEASURE_HUM_HM   0xE5 // Default hold Master
#define SI7013_WRITE_REG1       0xE6
#define SI7013_READ_REG1        0xE7
#define SI7013_SOFT_RESET       0xFE

#define SI7013_READ_ADC         0xEE
#define SI7013_READ_REG2        0x10
#define SI7013_WRITE_REG2       0x50
#define SI7013_REG2_DEFAULT     0x46 //set last three bits (VIN bufered, Vref=VDD, VOUT=GND) and No-Hold for bit 6

#define SI7013_ID1_CMD          0xFA0F      /**< Read Electronic ID 1st Byte */
#define SI7013_ID2_CMD          0xFCC9      /**< Read Electronic ID 2nd Byte */
#define SI7013_FIRMVERS_CMD     0x84B8      // Read Firmware Revision

#define SI7013_REV_1 0xff /**< Sensor revision 1 */
#define SI7013_REV_2 0x20 /**< Sensor revision 2 */

// SI7013 Sensor resolution
// default at power up is SI7013_RESOLUTION_14T_12RH
#define SI7013_RESOLUTION_14T_12RH 0x00 // 12 bits RH / 14 bits Temp
#define SI7013_RESOLUTION_13T_10RH 0x80 // 10 bits RH / 13 bits Temp
#define SI7013_RESOLUTION_12T_08RH 0x01 //  8 bits RH / 12 bits Temp
#define SI7013_RESOLUTION_11T_11RH 0x81 // 11 bits RH / 11 bits Temp
#define SI7013_RESOLUTION_MASK 0B01111110


#define SI7013_MEASURMENT_DELAY 100 //delay in milliseconds for reading the values
#define SI7013_DELAY            10  //delay 10 miliseconds between the states if we need more time we check it inside the state machine


enum class P123_state {
  Uninitialized = 0,
  Ready,
  Wait_for_HUM,
  Wait_for_Temp,
  Wait_for_ADC,
  New_values_available,
  Error
};


struct P123_data_struct : public PluginTaskData_base {
  private: 
    int8_t  begin(uint8_t i2caddr, uint8_t resolution);
    uint8_t checkCRC(uint16_t data, uint8_t check);
    int8_t  readRegister(uint8_t i2caddr, const uint8_t reg, uint8_t * value);
    int8_t  startConv(uint8_t i2caddr, uint8_t datatype, uint8_t resolution);
    int8_t  readHumidity(uint8_t i2caddr, uint8_t resolution);
    int8_t  requestTemperature(uint8_t i2caddr);
    int8_t  readTemperature(uint8_t i2caddr, uint8_t resolution);
    int8_t  readValues(uint8_t i2caddr, uint8_t resolution, uint8_t filter_power);
    int8_t  requestADC(uint8_t i2caddr);
    int8_t  readADC(uint8_t i2caddr, uint8_t filter_power);
    int8_t  setResolution(uint8_t i2caddr, uint8_t resolution);
    int8_t  softReset(uint8_t i2caddr);
    int8_t  readSerialNumber(uint8_t i2caddr);
    int8_t  readRevision(uint8_t i2caddr);

    bool    initialized() const;

    unsigned inline char ReadStatus(uint8_t address);

  // **************************************************************************/
  // Read temperature
  // **************************************************************************/
  float readTemperature();

  // **************************************************************************/
  // Read humidity
  // **************************************************************************/
  float readHumidity();

  
  public:
    P123_data_struct();

    // Only perform the measurements with big interval to prevent the sensor from warming up.
    bool update(uint8_t i2caddr, uint8_t resolution, uint8_t filter_power);
  
    float              last_hum_val      = 0.0f; //uint16_t
    float              last_temp_val     = 0.0f; //int16_t
    int32_t            last_adc_val      = 0;
    unsigned long      last_measurement  = 0;
    P123_state         state             = P123_state::Uninitialized;
};


//uint8_t MOVING_AVERAGE_power 4 //2^4


#endif // ifdef USES_P123

#endif // PLUGINSTRUCTS_P123_DATA_STRUCT_H
