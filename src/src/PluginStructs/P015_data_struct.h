#ifndef PLUGINSTRUCTS_P015_DATA_STRUCT_H
#define PLUGINSTRUCTS_P015_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P015

#include "../../ESPEasy_common.h"


# define TSL2561_ADDR_0 0x29 // address with '0' shorted on board
# define TSL2561_ADDR   0x39 // default address
# define TSL2561_ADDR_1 0x49 // address with '1' shorted on board


# define P015_NO_GAIN          0
# define P015_16X_GAIN         1
# define P015_AUTO_GAIN        2
# define P015_EXT_AUTO_GAIN    3

struct P015_data_struct : public PluginTaskData_base {
  P015_data_struct(byte         i2caddr,
                   unsigned int gain,
                   byte         integration);

  bool begin();

  bool useAutoGain() const;

  bool performRead(float& luxVal,
                   float& infraredVal,
                   float& broadbandVal,
                   float& ir_broadband_ratio);

  // Reads a byte from a TSL2561 address
  // Address: TSL2561 address (0 to 15)
  // Value will be set to stored byte
  // Returns true (1) if successful, false (0) if there was an I2C error
  bool readByte(unsigned char  address,
                unsigned char& value);

  // Write a byte to a TSL2561 address
  // Address: TSL2561 address (0 to 15)
  // Value: byte to write to address
  // Returns true (1) if successful, false (0) if there was an I2C error
  // (Also see getError() above)
  bool writeByte(unsigned char address,
                 unsigned char value);

  // Reads an unsigned integer (16 bits) from a TSL2561 address (low byte first)
  // Address: TSL2561 address (0 to 15), low byte first
  // Value will be set to stored unsigned integer
  // Returns true (1) if successful, false (0) if there was an I2C error
  // (Also see getError() above)
  bool readUInt(unsigned char address,
                unsigned int& value);

  // Write an unsigned integer (16 bits) to a TSL2561 address (low byte first)
  // Address: TSL2561 address (0 to 15), low byte first
  // Value: unsigned int to write to address
  // Returns true (1) if successful, false (0) if there was an I2C error
  // (Also see getError() above)
  bool writeUInt(unsigned char address,
                 unsigned int  value);


  // If gain = false (0), device is set to low gain (1X)
  // If gain = high (1), device is set to high gain (16X)
  // If time = 0, integration will be 13.7ms
  // If time = 1, integration will be 101ms
  // If time = 2, integration will be 402ms
  // If time = 3, use manual start / stop
  // Returns true (1) if successful, false (0) if there was an I2C error
  // (Also see getError() below)
  bool plugin_015_setTiming(bool          gain,
                            unsigned char time);

  // If gain = false (0), device is set to low gain (1X)
  // If gain = high (1), device is set to high gain (16X)
  // If time = 0, integration will be 13.7ms
  // If time = 1, integration will be 101ms
  // If time = 2, integration will be 402ms
  // If time = 3, use manual start / stop (ms = 0)
  // ms will be set to integration time
  // Returns true (1) if successful, false (0) if there was an I2C error
  // (Also see getError() below)
  bool plugin_015_setTiming(bool          gain,
                            unsigned char time,
                            float       & ms);

  // Determine if either sensor saturated (max depends on clock freq. and integration time)
  // If so, abandon ship (calculation will not be accurate)
  bool ADC_saturated(unsigned char time,
                     unsigned int  value);

  // Turn on TSL2561, begin integrations
  // Returns true (1) if successful, false (0) if there was an I2C error
  // (Also see getError() below)
  bool setPowerUp(void);

  // Turn off TSL2561
  // Returns true (1) if successful, false (0) if there was an I2C error
  // (Also see getError() below)
  bool setPowerDown(void);


  // Retrieve raw integration results
  // data0 and data1 will be set to integration results
  // Returns true (1) if successful, false (0) if there was an I2C error
  // (Also see getError() below)
  bool getData(unsigned int& data0,
               unsigned int& data1);

  // Convert raw data to lux
  // gain: 0 (1X) or 1 (16X), see setTiming()
  // ms: integration time in ms, from setTiming() or from manual integration
  // CH0, CH1: results from getData()
  // lux will be set to resulting lux calculation
  // returns true (1) if calculation was successful
  // RETURNS false (0) AND lux = 0.0 IF EITHER SENSOR WAS SATURATED (0XFFFF)
  void getLux(unsigned char gain,
              float         ms,
              unsigned int  CH0,
              unsigned int  CH1,
              double      & lux,
              double      & infrared,
              double      & broadband);


  unsigned int _gain; // Gain setting, 0 = X1, 1 = X16, 2 = auto, 3 = extended auto;
  byte         _i2cAddr       = 0;
  byte         _integration   = 0;
  byte         _error         = 0;
  bool         _gain16xActive = false;
};

#endif // ifdef USES_P015
#endif // ifndef PLUGINSTRUCTS_P015_DATA_STRUCT_H
