
#ifdef USES_P127
#include "../PluginStructs/P127_data_struct.h"

// #######################################################################################################
// ################################## Plugin 127 I2C CDM7160 CO2 sensor ##################################
// #######################################################################################################
//
//

P127_data_struct::P127_data_struct(int8_t i2caddr)
  : _i2cAddress(i2caddr) {}

// Do all required initialization
bool P127_data_struct::init(uint16_t alt) {
  setPowerDown();

  // delay required to store config byte to EEPROM, device pulls SCL low
  delay(100);

  uint8_t elev = alt / 10; // Altitude is as 'finegrained' per 10 meter

  if (elev) {
    setAltitude(elev);
  } else {
    clearAltitude();
  }

  // delay required to store config byte to EEPROM, device pulls SCL low
  delay(100);

  // Start reading
  setContinuous();
  return true;
}

// Check status and read data if not busy
bool P127_data_struct::checkData() {
  uint8_t status = getStatus();

  if (!(status & CDM7160_FLAG_BUSY)) {
    _co2 = getCO2();
  }
  return true;
}

// Return the last measured value
uint16_t P127_data_struct::readData() {
  return _co2;
}

uint8_t P127_data_struct::getAltitude() {
  return I2C_read8_ST_reg(_i2cAddress, CDM7160_REG_HIT);
}

uint8_t P127_data_struct::getCompensation() {
  return I2C_read8_ST_reg(_i2cAddress, CDM7160_REG_FUNC);
}

bool P127_data_struct::setPowerDown(void)

// Set power down mode CDM7160 to enable correct settings modification
// Returns true (1) if successful, false (0) if there was an I2C error
{
  // Write 0x06 to control register
  return I2C_write8_reg(_i2cAddress, CDM7160_REG_CTL, CDM7160_FLAG_DOWN);
}

bool P127_data_struct::setContinuous(void)

// Set continuous operation mode CDM7160 to start measurements
// Returns true (1) if successful, false (0) if there was an I2C error
{
  // Write 0x00 to control register
  return I2C_write8_reg(_i2cAddress, CDM7160_REG_CTL, CDM7160_FLAG_CONT);
}

bool P127_data_struct::setReset(void)

// Reset CDM7160
// Returns true (1) if successful, false (0) if there was an I2C error
{
  // Write 0x01 to reset register
  return I2C_write8_reg(_i2cAddress, CDM7160_REG_RESET, CDM7160_FLAG_REST);
}

bool P127_data_struct::setAltitude(uint8_t alt)

// Set altitude compensation on CDM7160
// Returns true (1) if successful, false (0) if there was an I2C error
{
  // Write altitude and enable compensation
  I2C_write8_reg(_i2cAddress, CDM7160_REG_HIT, alt);
  return I2C_write8_reg(_i2cAddress, CDM7160_REG_FUNC, (CDM7160_FLAG_HPAE | CDM7160_FLAG_PWME));
}

bool P127_data_struct::clearAltitude(void)

// Disable altitude compensation on CDM7160
// Returns true (1) if successful, false (0) if there was an I2C error
{
  // Clear altitude and disable compensation
  I2C_write8_reg(_i2cAddress, CDM7160_REG_HIT, 0);
  return I2C_write8_reg(_i2cAddress, CDM7160_REG_FUNC, CDM7160_FLAG_PWME);
}

uint8_t P127_data_struct::getStatus()

// Retrieve CO2 data in ppm
// Returns true (1) if successful, false (0) if there was an I2C error
{
  // Get content of status register
  return I2C_read8_ST_reg(_i2cAddress, CDM7160_REG_STATUS);
}

uint16_t P127_data_struct::getCO2()

// Retrieve CO2 data in ppm
// Returns the value
{
  // Get co2 ppm data out of result registers
  return I2C_read16_LE_ST_reg(_i2cAddress, CDM7160_REG_DATA);
}

// Reads an 8 bit value from a register over I2C, no repeated start
uint8_t P127_data_struct::I2C_read8_ST_reg(uint8_t i2caddr, byte reg) {
  Wire.beginTransmission(i2caddr);
  Wire.write((uint8_t)reg);
  Wire.endTransmission();
  Wire.requestFrom(i2caddr, (byte)1);
  return Wire.read();
}

// Reads a 16 bit value starting at a given register over I2C, no repeated start
uint16_t P127_data_struct::I2C_read16_LE_ST_reg(uint8_t i2caddr, byte reg) {
  Wire.beginTransmission(i2caddr);
  Wire.write((uint8_t)reg);
  Wire.endTransmission();
  Wire.requestFrom(i2caddr, (byte)2);

  return (Wire.read()) | Wire.read() << 8;
}

#endif // ifdef USES_P127
