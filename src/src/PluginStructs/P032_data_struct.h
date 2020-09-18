#ifndef PLUGINSTRUCTS_P032_DATA_STRUCT_H
#define PLUGINSTRUCTS_P032_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#include "../../ESPEasy_common.h"

#ifdef USES_P032

struct P032_data_struct : public PluginTaskData_base {
public:

  P032_data_struct(uint8_t i2c_addr);


  // **************************************************************************/
  // Initialize MS5611
  // **************************************************************************/
  bool begin();

  // **************************************************************************/
  // Reads the PROM of MS5611
  // There are in total 8 addresses resulting in a total memory of 128 bit.
  // Address 0 contains factory data and the setup, addresses 1-6 calibration
  // coefficients and address 7 contains the serial code and CRC.
  // The command sequence is 8 bits long with a 16 bit result which is
  // clocked with the MSB first.
  // **************************************************************************/
  void read_prom();

  // **************************************************************************/
  // Read analog/digital converter
  // **************************************************************************/
  unsigned long read_adc(unsigned char aCMD);

  // **************************************************************************/
  // Readout
  // **************************************************************************/
  void readout();

  // **************************************************************************/
  // MSL pressure formula
  // **************************************************************************/
  double pressureElevation(double atmospheric,
                           int    altitude);

  uint8_t      i2cAddress;
  unsigned int ms5611_prom[8]     = { 0 };
  double       ms5611_pressure    = 0;
  double       ms5611_temperature = 0;
};
#endif // ifdef USES_P032
#endif // ifndef PLUGINSTRUCTS_P032_DATA_STRUCT_H
