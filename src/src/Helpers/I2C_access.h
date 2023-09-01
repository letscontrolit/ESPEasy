#ifndef HELPERS_I2C_ACCESS_H
#define HELPERS_I2C_ACCESS_H

#include "../DataStructs/I2CTypes.h"

#include "../DataTypes/TaskIndex.h"

#include "../Globals/Plugins.h"

#include <vector>

I2C_bus_state I2C_check_bus(int8_t scl,
                            int8_t sda);

// **************************************************************************/
// Central functions for I2C data transfers
// **************************************************************************/
bool I2C_read_bytes(uint8_t        i2caddr,
                    I2Cdata_bytes& data);

bool I2C_read_words(uint8_t        i2caddr,
                    I2Cdata_words& data);

// **************************************************************************/
// Wake up I2C device
// **************************************************************************/
unsigned char I2C_wakeup(uint8_t i2caddr);

// **************************************************************************/
// Writes an 8 bit value over I2C
// **************************************************************************/
bool I2C_write8(uint8_t i2caddr,
                uint8_t value);

// **************************************************************************/
// Writes an 8 bit value over I2C to a register
// **************************************************************************/
bool I2C_write8_reg(uint8_t i2caddr,
                    uint8_t reg,
                    uint8_t value);

// **************************************************************************/
// Writes an 16 bit value over I2C to a register
// **************************************************************************/
bool I2C_write16_reg(uint8_t  i2caddr,
                     uint8_t  reg,
                     uint16_t value);

// **************************************************************************/
// Writes an 16 bit value over I2C to a register
// **************************************************************************/
bool I2C_write16_LE_reg(uint8_t  i2caddr,
                        uint8_t  reg,
                        uint16_t value);

// **************************************************************************/
// Writes length bytes over I2C to a register
// **************************************************************************/
bool I2C_writeBytes_reg(uint8_t  i2caddr,
                        uint8_t  reg,
                        uint8_t *buffer,
                        uint8_t  length);

// **************************************************************************/
// Reads an 8 bit value over I2C
// **************************************************************************/
uint8_t I2C_read8(uint8_t i2caddr,
                  bool   *is_ok);

// **************************************************************************/
// Reads an 8 bit value from a register over I2C
// **************************************************************************/
uint8_t I2C_read8_reg(uint8_t i2caddr,
                      uint8_t reg,
                      bool   *is_ok = nullptr);

// **************************************************************************/
// Reads a 16 bit value starting at a given register over I2C
// **************************************************************************/
uint16_t I2C_read16_reg(uint8_t i2caddr,
                        uint8_t reg,
                        bool   *is_ok = nullptr);

// **************************************************************************/
// Reads a 24 bit value starting at a given register over I2C
// **************************************************************************/
int32_t I2C_read24_reg(uint8_t i2caddr,
                       uint8_t reg,
                       bool   *is_ok = nullptr);

// **************************************************************************/
// Reads a 32 bit value starting at a given register over I2C
// **************************************************************************/
int32_t I2C_read32_reg(uint8_t i2caddr,
                       uint8_t reg,
                       bool   *is_ok = nullptr);

// **************************************************************************/
// Reads a 16 bit value starting at a given register over I2C
// **************************************************************************/
uint16_t I2C_read16_LE_reg(uint8_t i2caddr,
                           uint8_t reg,
                           bool   *is_ok = nullptr);

// **************************************************************************/
// Reads a signed 16 bit value starting at a given register over I2C
// **************************************************************************/
int16_t I2C_readS16_reg(uint8_t i2caddr,
                        uint8_t reg,
                        bool   *is_ok = nullptr);

int16_t I2C_readS16_LE_reg(uint8_t i2caddr,
                           uint8_t reg,
                           bool   *is_ok = nullptr);

// *************************************************************************/
// Checks if a device is responding on the address
// Should be used in any I2C plugin case PLUGIN_INIT: before any initialization
// Can be used in any I2C plugin case PLUGIN_READ: to check if the device is still connected/responding
// if (!I2C_deviceCheck(configured_I2C_address)) {
//   break; // Will return the default false for success
// }
// *************************************************************************/
#if FEATURE_I2C_DEVICE_CHECK

bool I2C_deviceCheck(uint8_t     i2caddr,
                     taskIndex_t taskIndex  = INVALID_TASK_INDEX,
                     uint8_t     maxRetries = 0,
                     uint8_t     function   = 0);
#endif // if FEATURE_I2C_DEVICE_CHECK

#endif // HELPERS_I2C_ACCESS_H
