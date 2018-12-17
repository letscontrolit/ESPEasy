//**************************************************************************/
// Central functions for I2C data transfers
//**************************************************************************/
bool I2C_read_bytes(uint8_t i2caddr, I2Cdata_bytes& data) {
  const uint8_t size = data.getSize();
  return size == i2cdev.readBytes(i2caddr, data.getRegister(), size, data.get());
}

bool I2C_read_words(uint8_t i2caddr, I2Cdata_words& data) {
  const uint8_t size = data.getSize();
  return size == i2cdev.readWords(i2caddr, data.getRegister(), size, data.get());
}

// See https://github.com/platformio/platform-espressif32/issues/126
#ifdef ESP32
  #define END_TRANSMISSION_FLAG true
#else
  #define END_TRANSMISSION_FLAG false
#endif

//**************************************************************************/
// Wake up I2C device
//**************************************************************************/
void I2C_wakeup(uint8_t i2caddr) {
  Wire.beginTransmission(i2caddr);
  Wire.endTransmission();
}

//**************************************************************************/
// Writes an 8 bit value over I2C
//**************************************************************************/
bool I2C_write8(uint8_t i2caddr, byte value) {
  Wire.beginTransmission(i2caddr);
  Wire.write((uint8_t)value);
  return (Wire.endTransmission() == 0);
}


//**************************************************************************/
// Writes an 8 bit value over I2C to a register
//**************************************************************************/
bool I2C_write8_reg(uint8_t i2caddr, byte reg, byte value) {
  Wire.beginTransmission(i2caddr);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)value);
  return (Wire.endTransmission() == 0);
}

//**************************************************************************/
// Writes an 16 bit value over I2C to a register
//**************************************************************************/
bool I2C_write16_reg(uint8_t i2caddr, byte reg, uint16_t value) {
  Wire.beginTransmission(i2caddr);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)(value >> 8));
  Wire.write((uint8_t)value);
  return (Wire.endTransmission() == 0);
}

//**************************************************************************/
// Writes an 16 bit value over I2C to a register
//**************************************************************************/
bool I2C_write16_LE_reg(uint8_t i2caddr, byte reg, uint16_t value) {
  return (I2C_write16_reg(i2caddr, reg, (value << 8)|(value >> 8)));
}

//**************************************************************************/
// Reads an 8 bit value from a register over I2C
//**************************************************************************/
uint8_t I2C_read8_reg(uint8_t i2caddr, byte reg, bool * is_ok) {
  uint8_t value;

  Wire.beginTransmission(i2caddr);
  Wire.write((uint8_t)reg);
  Wire.endTransmission(END_TRANSMISSION_FLAG);
  byte count = Wire.requestFrom(i2caddr, (byte)1);
  if (is_ok != NULL) {
    *is_ok = (count == 1);
  }
  value = Wire.read();

  return value;
}

//**************************************************************************/
// Reads a 16 bit value starting at a given register over I2C
//**************************************************************************/
uint16_t I2C_read16_reg(uint8_t i2caddr, byte reg) {
  uint16_t value(0);

  Wire.beginTransmission(i2caddr);
  Wire.write((uint8_t)reg);
  Wire.endTransmission(END_TRANSMISSION_FLAG);
  Wire.requestFrom(i2caddr, (byte)2);
  value = (Wire.read() << 8) | Wire.read();

  return value;
}

//**************************************************************************/
// Reads a 24 bit value starting at a given register over I2C
//**************************************************************************/
int32_t I2C_read24_reg(uint8_t i2caddr, byte reg) {
  int32_t value;

  Wire.beginTransmission(i2caddr);
  Wire.write((uint8_t)reg);
  Wire.endTransmission(END_TRANSMISSION_FLAG);
  Wire.requestFrom(i2caddr, (byte)3);
  value = (((int32_t)Wire.read()) << 16) | (Wire.read() << 8) | Wire.read();

  return value;
}

//**************************************************************************/
// Reads a 32 bit value starting at a given register over I2C
//**************************************************************************/
int32_t I2C_read32_reg(uint8_t i2caddr, byte reg) {
  int32_t value;

  Wire.beginTransmission(i2caddr);
  Wire.write((uint8_t)reg);
  Wire.endTransmission(END_TRANSMISSION_FLAG);
  Wire.requestFrom(i2caddr, (byte)4);
  value = (((int32_t)Wire.read()) <<24) | (((uint32_t)Wire.read()) << 16) | (Wire.read() << 8) | Wire.read();

  return value;
}

//**************************************************************************/
// Reads a 16 bit value starting at a given register over I2C
//**************************************************************************/
uint16_t I2C_read16_LE_reg(uint8_t i2caddr, byte reg) {
  uint16_t temp = I2C_read16_reg(i2caddr, reg);
  return (temp >> 8) | (temp << 8);
}

//**************************************************************************/
// Reads a signed 16 bit value starting at a given register over I2C
//**************************************************************************/
int16_t I2C_readS16_reg(uint8_t i2caddr, byte reg) {
  return (int16_t)I2C_read16_reg(i2caddr, reg);
}

int16_t I2C_readS16_LE_reg(uint8_t i2caddr, byte reg) {
  return (int16_t)I2C_read16_LE_reg(i2caddr, reg);
}

#undef END_TRANSMISSION_FLAG
