#ifndef HELPERS_I2C_ACCESS_H
#define HELPERS_I2C_ACCESS_H

#include "../DataStructs/I2CTypes.h"

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
void I2C_wakeup(uint8_t i2caddr);

// **************************************************************************/
// Writes an 8 bit value over I2C
// **************************************************************************/
bool I2C_write8(uint8_t i2caddr,
                byte    value);

// **************************************************************************/
// Writes an 8 bit value over I2C to a register
// **************************************************************************/
bool I2C_write8_reg(uint8_t i2caddr,
                    byte    reg,
                    byte    value);

// **************************************************************************/
// Writes an 16 bit value over I2C to a register
// **************************************************************************/
bool I2C_write16_reg(uint8_t  i2caddr,
                     byte     reg,
                     uint16_t value);

// **************************************************************************/
// Writes an 16 bit value over I2C to a register
// **************************************************************************/
bool I2C_write16_LE_reg(uint8_t  i2caddr,
                        byte     reg,
                        uint16_t value);

// **************************************************************************/
// Reads an 8 bit value over I2C
// **************************************************************************/
uint8_t I2C_read8(uint8_t i2caddr,
                  bool   *is_ok);

// **************************************************************************/
// Reads an 8 bit value from a register over I2C
// **************************************************************************/
uint8_t I2C_read8_reg(uint8_t i2caddr,
                      byte    reg,
                      bool   *is_ok = nullptr);

// **************************************************************************/
// Reads a 16 bit value starting at a given register over I2C
// **************************************************************************/
uint16_t I2C_read16_reg(uint8_t i2caddr,
                        byte    reg);

// **************************************************************************/
// Reads a 24 bit value starting at a given register over I2C
// **************************************************************************/
int32_t I2C_read24_reg(uint8_t i2caddr,
                       byte    reg);

// **************************************************************************/
// Reads a 32 bit value starting at a given register over I2C
// **************************************************************************/
int32_t I2C_read32_reg(uint8_t i2caddr,
                       byte    reg);

// **************************************************************************/
// Reads a 16 bit value starting at a given register over I2C
// **************************************************************************/
uint16_t I2C_read16_LE_reg(uint8_t i2caddr,
                           byte    reg);

// **************************************************************************/
// Reads a signed 16 bit value starting at a given register over I2C
// **************************************************************************/
int16_t I2C_readS16_reg(uint8_t i2caddr,
                        byte    reg);

int16_t I2C_readS16_LE_reg(uint8_t i2caddr,
                           byte    reg);


#endif // HELPERS_I2C_ACCESS_H
