#ifndef PLUGINSTRUCTS_P130_DATA_STRUCT_H
#define PLUGINSTRUCTS_P130_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P130

#undef P130_DEBUG_DEV
#define P130_ADS1015_RATE_3300SPS           (0x00C0) // < 3300 samples per second
#define P130_ADS1015_RATE_2400SPS           (0x00A0) // < 2400 samples per second
#define P130_ADS1015_RATE_1600SPS           (0x0080) // < 1600 samples per second

struct P130_data_struct : public PluginTaskData_base {
public:

  P130_data_struct(uint8_t i2c_addr,
                   uint8_t _calCurrent1,
                   uint8_t _calCurrent2,
                   float_t _calVoltage,
                   uint8_t _currentFreq,
                   uint8_t _nbSinus,
                   uint8_t _convModeContinuous,
                   uint8_t _sps);

  void setDebug(uint8_t _debug);
  uint8_t getDebug();
  boolean readCurrent(uint8_t canal, float_t& currentValue);
  float_t estimatePower(float_t current);

private:

  uint8_t i2cAddress;
  uint8_t calCurrent1;
  uint8_t calCurrent2;
  float_t calVoltage;
  uint8_t currentFreq;
  uint8_t nbSinus;
  uint8_t convModeContinuous;
  uint8_t debug;
  uint8_t sps;

  uint16_t getDefaultADS1015ReadConfig();
  boolean writeRegister(uint8_t registerAddr, uint16_t registerValue);
  boolean readRegister(uint8_t registerAddr, uint16_t& registerValue);
  uint16_t readRegisterFacility(uint8_t registerAddr);
  boolean readAdcSingleValue(uint16_t muxConf, int16_t& adcValue);
  boolean readAdcContinuousRmsValue(uint16_t muxConf, uint16_t period_ms, float_t& adcIrms, uint16_t& nbSample);
};

#endif // ifdef USES_P130
#endif // ifndef PLUGINSTRUCTS_P130_DATA_STRUCT_H
