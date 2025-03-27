#ifndef COMMAND_I2C_H
#define COMMAND_I2C_H

#include "../../ESPEasy_common.h"

void i2c_scanI2Cbus(bool    dbg,
                    int8_t  channel,
                    uint8_t i2cBus);

const __FlashStringHelper* Command_i2c_Scanner(struct EventStruct *event,
                                               const char         *Line);

#endif // COMMAND_I2C_H
