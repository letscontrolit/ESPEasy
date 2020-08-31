
#include "P022_data_struct.h"

#ifdef USES_P022

bool P022_data_struct::p022_is_init(uint8_t address) {
  if ((address < PCA9685_ADDRESS) || (address > PCA9685_MAX_ADDRESS)) { return false; }
  uint32_t address_offset = address - PCA9685_ADDRESS;

  if (address_offset < 32) {
    return initializeState_lo & (1 << address_offset);
  } else {
    return initializeState_hi & (1 << (address_offset - 32));
  }
}

bool P022_data_struct::p022_set_init(uint8_t address) {
  if ((address < PCA9685_ADDRESS) || (address > PCA9685_MAX_ADDRESS)) { return false; }
  uint32_t address_offset = address - PCA9685_ADDRESS;

  if (address_offset < 32) {
    initializeState_lo |= (1 << address_offset);
  } else {
    initializeState_hi |= (1 << (address_offset - 32));
  }
  return true;
}

bool P022_data_struct::p022_clear_init(uint8_t address) {
  if ((address < PCA9685_ADDRESS) || (address > PCA9685_MAX_ADDRESS)) { return false; }
  uint32_t address_offset = address - PCA9685_ADDRESS;

  if (address_offset < 32) {
    initializeState_lo &= ~(1 << address_offset);
  } else {
    initializeState_hi &= ~(1 << (address_offset - 32));
  }
  return true;
}

// ********************************************************************************
// PCA9685 config
// ********************************************************************************
void P022_data_struct::Plugin_022_writeRegister(int i2cAddress, int regAddress, byte data) {
  Wire.beginTransmission(i2cAddress);
  Wire.write(regAddress);
  Wire.write(data);
  Wire.endTransmission();
}

uint8_t P022_data_struct::Plugin_022_readRegister(int i2cAddress, int regAddress) {
  uint8_t res = 0;

  Wire.requestFrom(i2cAddress, 1, 1);

  while (Wire.available()) {
    res = Wire.read();
  }
  return res;
}

// ********************************************************************************
// PCA9685 write
// ********************************************************************************
void P022_data_struct::Plugin_022_Off(int address, int pin)
{
  Plugin_022_Write(address, pin, 0);
}

void P022_data_struct::Plugin_022_On(int address, int pin)
{
  Plugin_022_Write(address, pin, PCA9685_MAX_PWM);
}

void P022_data_struct::Plugin_022_Write(int address, int Par1, int Par2)
{
  int i2cAddress = address;

  // boolean success = false;
  int regAddress = Par1 == -1
                   ? PCA9685_ALLLED_REG
                   : PCA9685_LED0 + 4 * Par1;
  uint16_t LED_ON  = 0;
  uint16_t LED_OFF = Par2;

  Wire.beginTransmission(i2cAddress);
  Wire.write(regAddress);
  Wire.write(lowByte(LED_ON));
  Wire.write(highByte(LED_ON));
  Wire.write(lowByte(LED_OFF));
  Wire.write(highByte(LED_OFF));
  Wire.endTransmission();
}

void P022_data_struct::Plugin_022_Frequency(int address, uint16_t freq)
{
  int i2cAddress = address;

  Plugin_022_writeRegister(i2cAddress, PLUGIN_022_PCA9685_MODE1, (byte)0x0);
  freq *= 0.9;

  //  prescale = 25000000 / 4096;
  uint16_t prescale = 6103;

  prescale /=  freq;
  prescale -= 1;
  uint8_t oldmode = Plugin_022_readRegister(i2cAddress, 0);
  uint8_t newmode = (oldmode & 0x7f) | 0x10;

  Plugin_022_writeRegister(i2cAddress, PLUGIN_022_PCA9685_MODE1, (byte)newmode);
  Plugin_022_writeRegister(i2cAddress, 0xfe,                     (byte)prescale); // prescale register
  Plugin_022_writeRegister(i2cAddress, PLUGIN_022_PCA9685_MODE1, (byte)oldmode);
  delayMicroseconds(5000);
  Plugin_022_writeRegister(i2cAddress, PLUGIN_022_PCA9685_MODE1, (byte)oldmode | 0xa1);
}

void P022_data_struct::Plugin_022_initialize(int address)
{
  int i2cAddress = address;

  // default mode is open drain output, drive leds connected to VCC
  Plugin_022_writeRegister(i2cAddress, PLUGIN_022_PCA9685_MODE1, (byte)0x01);      // reset the device
  delay(1);
  Plugin_022_writeRegister(i2cAddress, PLUGIN_022_PCA9685_MODE1, (byte)B10100000); // set up for auto increment
  // Plugin_022_writeRegister(i2cAddress, PCA9685_MODE2, (byte)0x10); // set to output
  p022_set_init(address);
}

#endif // ifdef USES_P022
