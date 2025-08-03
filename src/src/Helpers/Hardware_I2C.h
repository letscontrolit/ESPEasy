#ifndef HELPERS_HARDWARE_I2C_H
#define HELPERS_HARDWARE_I2C_H

#include "../../ESPEasy_common.h"

#include "../DataTypes/TaskIndex.h"

void initI2C();

void I2CSelectHighClockSpeed(uint8_t i2cBus);
void I2CSelectLowClockSpeed(uint8_t i2cBus);
void I2CSelect_Max100kHz_ClockSpeed(uint8_t i2cBus);
void I2CSelectClockSpeed(uint8_t  i2cBus,
                         uint32_t clockFreq);
void I2CForceResetBus_swap_pins(uint8_t i2cBus,
                                uint8_t address);
void I2CBegin(int8_t   sda,
              int8_t   scl,
              uint32_t clockFreq,
              uint32_t clockStretch);

#if FEATURE_I2CMULTIPLEXER
bool    isI2CMultiplexerEnabled(uint8_t i2cBus);

void    I2CMultiplexerSelectByTaskIndex(taskIndex_t taskIndex);
void    I2CMultiplexerSelect(uint8_t i2cBus,
                             uint8_t i);

void    I2CMultiplexerOff(uint8_t i2cBus);

void    SetI2CMultiplexer(uint8_t i2cBus,
                          uint8_t toWrite);

uint8_t I2CMultiplexerMaxChannels(uint8_t i2cBus);

void    I2CMultiplexerReset(uint8_t i2cBus);

bool    I2CMultiplexerPortSelectedForTask(taskIndex_t taskIndex);
#endif // if FEATURE_I2CMULTIPLEXER

#endif // ifndef HELPERS_HARDWARE_I2C_H
