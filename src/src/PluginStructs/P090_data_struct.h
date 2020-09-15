#ifndef PLUGINSTRUCTS_P090_DATA_STRUCT_H
#define PLUGINSTRUCTS_P090_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#include "../../ESPEasy_common.h"

#ifdef USES_P090

# include "stdint.h"

// This is the core operational class of the driver.
//  CCS811Core contains only read and write operations towards the sensor.
//  To use the higher level functions, use the class CCS811 which inherits
//  this class.

class CCS811Core {
public:

  // Return values
  typedef enum {
    SENSOR_SUCCESS,
    SENSOR_ID_ERROR,
    SENSOR_I2C_ERROR,
    SENSOR_INTERNAL_ERROR,
    SENSOR_GENERIC_ERROR

    // ...
  } status;

  CCS811Core(uint8_t);
  virtual ~CCS811Core() = default;

  status beginCore(void);
  void   setAddress(uint8_t);

  // ***Reading functions***//

  // readRegister reads one 8-bit register
  status readRegister(uint8_t  offset,
                      uint8_t *outputPointer);

  // ***Writing functions***//

  // Writes an 8-bit byte;
  status writeRegister(uint8_t offset,
                       uint8_t dataToWrite);

  // TODO TD-er: Must move this to I2C.ino.
  // multiWriteRegister takes a uint8 array address as input and performs
  //  a number of consecutive writes
  status multiWriteRegister(uint8_t  offset,
                            uint8_t *inputPointer,
                            uint8_t  length);

protected:

  uint8_t I2CAddress;
};


// This is the highest level class of the driver.
//  class CCS811 inherits the CCS811Core and makes use of the beginCore()
// method through its own begin() method.  It also contains user settings/values.
class CCS811 : public CCS811Core {
public:

  CCS811(uint8_t);

  // Call to check for errors, start app, and set default mode 1
  status   begin(void);

  status   readAlgorithmResults(void);
  bool     checkForStatusError(void);
  bool     dataAvailable(void);
  bool     appValid(void);
  uint8_t  getErrorRegister(void);
  uint16_t getBaseline(void);
  status   setBaseline(uint16_t);
  status   enableInterrupts(void);
  status   disableInterrupts(void);
  status   setDriveMode(uint8_t mode);
  status   setEnvironmentalData(float relativeHumidity,
                                float temperature);
  void     setRefResistance(float);
  status   readNTC(void);
  uint16_t getTVOC(void);
  uint16_t getCO2(void);
  float    getResistance(void);
  float    getTemperature(void);
  String   getDriverError(CCS811Core::status);
  String   getSensorError(void);

private:

  // These are the air quality values obtained from the sensor
  float refResistance = 0.0f;
  float resistance    = 0.0f;
  uint16_t tVOC       = 0;
  uint16_t CO2        = 0;
  uint16_t vrefCounts = 0;
  uint16_t ntcCounts  = 0;
  float _temperature  = 0.0f;
};

struct P090_data_struct : public PluginTaskData_base {
public:

  P090_data_struct(uint8_t i2cAddr);

  CCS811 myCCS811;
  bool   compensation_set    = false;
  bool   newReadingAvailable = false;
};

#endif // ifdef USES_P090
#endif // ifndef PLUGINSTRUCTS_P090_DATA_STRUCT_H
