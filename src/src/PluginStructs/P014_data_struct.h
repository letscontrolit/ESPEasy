#ifndef PLUGINSTRUCTS_P014_DATA_STRUCT_H
#define PLUGINSTRUCTS_P014_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P014

// #######################################################################################################
// ######################## Plugin 014 SI70xx I2C Temperature Humidity Sensor  ###########################
// #######################################################################################################
// 12-10-2015 Charles-Henri Hallard, see my projects and blog at https://hallard.me
// 07-22-2022 MFD, Adding support for SI7013 with ADC
// 10-28-2022 MFD, fixing support for HTU21D (as this sensor does not have the command read temperature from humidity) 


// SI70xx Sensor resolution
// default at power up is SI7021_RESOLUTION_14T_12RH
# define SI70xx_RESOLUTION_14T_12RH 0x00 // 12 bits RH / 14 bits Temp
# define SI70xx_RESOLUTION_13T_10RH 0x80 // 10 bits RH / 13 bits Temp
# define SI70xx_RESOLUTION_12T_08RH 0x01 //  8 bits RH / 12 bits Temp
# define SI70xx_RESOLUTION_11T_11RH 0x81 // 11 bits RH / 11 bits Temp
# define SI70xx_RESOLUTION_MASK B01111110



// ======================================
// SI7013 sensor
// ======================================
#define SI70xx_I2C_ADDRESS            0x40 // I2C address for the sensor (Si7013 address when AD0 is pulled low)
#define SI7013_I2C_ADDRESS_AD0_1      0x41 // I2C address for the sensor when AD0 is pulled high

#define SI70xx_CMD_READ_TEMP_FROM_HUM    0xE0 // Read Temp only after a RH conversion done this does not have checksum byte
#define SI70xx_CMD_MEASURE_HUM      0xF5 // No hold
#define SI70xx_CMD_MEASURE_TEMP     0xF3 // No hold
#define SI70xx_CMD_MEASURE_TEMP_HM  0xE3 // Default hold Master
#define SI70xx_CMD_MEASURE_HUM_HM   0xE5 // Default hold Master
#define SI70xx_CMD_WRITE_REG1       0xE6
#define SI70xx_CMD_READ_REG1        0xE7
#define SI70xx_CMD_SOFT_RESET       0xFE

#define SI7013_READ_ADC         0xEE
#define SI7013_READ_REG2        0x10
#define SI7013_WRITE_REG2       0x50
#define SI7013_REG2_DEFAULT     B01000110 // (MeasureMode=10) No-Hold master with no thermistor correction ; 7ms conversion;  (VIN bufered, Vref=VDD, VOUT=GND)

#define SI70xx_CMD_ID1          0xFA0F      /**< Read Electronic ID SNA Bytes */
#define SI70xx_CMD_ID2          0xFCC9      /**< Read Electronic ID SNB Bytes */
#define SI70xx_CMD_FIRMREV      0x84B8      // Read Firmware Revision

#define SI70xx_REV_1 0xff /**< Sensor revision 1 */
#define SI70xx_REV_2 0x20 /**< Sensor revision 2 */


//0x00 or 0xFF engineering samples
#define CHIP_ID_UNKNOWN 01
#define CHIP_ID_SI7006  06 //  0x06=06=SI7006
#define CHIP_ID_SI7013  13 //  0x0D=13=Si7013
#define CHIP_ID_SI7020  20 //  0x14=20=Si7020
#define CHIP_ID_SI7021  21 //  0x15=21=Si7021
#define CHIP_ID_HTU21D  50 //  as measured by the plugin (the datasheet does not have this info)

#define SI70xx_RESET_DELAY            50 //delay in miliseconds for the reset to settle down
#define SI70xx_MEASUREMENT_TIMEOUT   100 
#define SI70xx_INIT_DELAY           1000 //delay in miliseconds between inits
#define SI70xx_MEASUREMENT_DELAY     100 //delay in milliseconds for reading the values
#define SI70xx_DELAY                  10 //delay 10 miliseconds between the states if we need more time we check it inside the state machine
#define P014_MAX_RETRY               250 //Should give us over 8 hrs of retries befoe going in error 

/*
Noise and AC Pick Up
The A/D of the Si7013 is a delta sigma type converter, and the input is not sampled. Thus, it is assumed that the
input voltage is constant over the measurement period. Generally, some amount of analog filtering prior to the A/D
input is desirable. In the standard application circuit, this is accomplished with 0.1 µF capacitors. These capacitors
will form a filter at about 30 Hz, which is adequate for high-frequency noise pick up (e.g. am radio signals) but not
for 60 Hz. If 60 Hz filtering is desired, these capacitors can be increased to >1 µF, or the result could be digitally
filtered (average of several measurements). If the sampling can be synchronized to 120 Hz (use the faster
conversion time for this), then an average of just two samples would reject 60 Hz.
In the standard biasing circuit, the bias can be turned off between measurements to save power (change bit zero of
user register 2). If this is done, allow adequate settling time between enabling the bias and making the
measurement (use approximately 100 msec for the 0.1 µF filter, which has a time constant of 30 msec)
*/

enum class P014_state {
  Uninitialized = 0,
  Wait_for_reset,
  Initialized,
  Ready,
  Wait_for_temperature_samples,
  Wait_for_humidity_samples,
  Wait_for_adc_samples,
  New_Values_Available,
  RequestADC,
  Error
};

struct P014_data_struct : public PluginTaskData_base {
  private: 
    uint8_t checkCRC(uint16_t data, uint8_t check);
    bool    setResolution(uint8_t i2caddr, uint8_t resolution);
    bool    softReset(uint8_t i2caddr);
    #ifndef LIMIT_BUILD_SIZE
    int8_t  readRevision(uint8_t i2caddr);
    #endif
    int8_t  readSerialNumber(uint8_t i2caddr);
    bool    enablePowerForADC(uint8_t i2caddr);
    bool    disablePowerForADC(uint8_t i2caddr);
    bool    requestADC(uint8_t i2caddr);
    bool    readADC(uint8_t i2caddr, uint8_t filter_power);
    bool    readHumidity(uint8_t i2caddr, uint8_t resolution);
    bool    readTemperatureFromHumidity(uint8_t i2caddr, uint8_t resolution);
    bool    readTemperature(uint8_t i2caddr, uint8_t resolution);
    bool    startInit(uint8_t i2caddr);
    bool    finalizeInit(uint8_t i2caddr, uint8_t resolution);
    int16_t convertRawTemperature(uint16_t raw, uint8_t resolution);

public:
   P014_data_struct();

   virtual ~P014_data_struct() = default;

   // Only perform the measurements with big interval to prevent the sensor from warming up.
   //This method runs the FSM step by step on each call
   bool update(uint8_t i2caddr, uint8_t resolution, uint8_t filter_power);


  unsigned long last_measurement_time  = 0; // Timestamp when started reading sensor
  uint16_t      humidity               = 0; // latest humidity value read
  int16_t       temperature            = 0; // latest temperature value read (*100)
  int32_t       adc                    = 0; // latest adc value read as a moving average sum 
  P014_state    state                  = P014_state::Uninitialized;
  uint8_t       chip_id                = CHIP_ID_UNKNOWN; //CHIP ID
  uint8_t       errCount               = 0; 
};

#endif // ifdef USES_P014
#endif // ifndef PLUGINSTRUCTS_P014_DATA_STRUCT_H