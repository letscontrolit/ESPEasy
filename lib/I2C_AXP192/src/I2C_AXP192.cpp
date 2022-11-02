#include "I2C_AXP192.h"

I2C_AXP192::I2C_AXP192(uint8_t deviceAddress, TwoWire& i2cPort) {
  _deviceAddress = deviceAddress;
  _i2cPort       = &i2cPort;
}

void I2C_AXP192::begin(I2C_AXP192_InitDef initDef) {
  ESP_LOGD("AXP192", "Begin");

  setEXTEN(initDef.EXTEN);
  setBACKUP(initDef.BACKUP);

  setDCDC1(initDef.DCDC1);
  setDCDC2(initDef.DCDC2);
  setDCDC3(initDef.DCDC3);

  setLDO2(initDef.LDO2);
  setLDO3(initDef.LDO3);

  writeByte(0x90, 0b00000000); // GPIO0 open drain output
  writeByte(0x92, 0b00000000); // GPIO1 open drain output
  writeByte(0x93, 0b00000000); // GPIO2 open drain output
  writeByte(0x95, 0b10000101); // GPIO3/4 open drain output

  setLDOIO(initDef.LDOIO);
  setGPIO0(initDef.GPIO0);
  setGPIO1(initDef.GPIO1);
  setGPIO2(initDef.GPIO2);
  setGPIO3(initDef.GPIO3);
  setGPIO4(initDef.GPIO4);

  writeByte(0x84, 0b11110010); // ADC 200Hz
  writeByte(0x82, 0b11111111); // ADC All Enable
  writeByte(0x33, 0b11000000); // Charge 4.2V, 100mA
  writeByte(0x36, 0b00001100); // PEK 128ms, PW OFF 4S
  writeByte(0x30, 0b10000000); // VBUS Open
  writeByte(0x39, 0b11111100); // Temp Protection
  writeByte(0x31, 0b00000100); // Power Off 3.0V
}

uint8_t I2C_AXP192::readByte(uint8_t address) {
  _i2cPort->beginTransmission(_deviceAddress);
  _i2cPort->write(address);
  _i2cPort->endTransmission();
  _i2cPort->requestFrom(_deviceAddress, 1);
  uint8_t val = _i2cPort->read();

  ESP_LOGD("AXP192", "readByte(%02X) = %02X", address, val);
  return val;
}

void I2C_AXP192::writeByte(uint8_t address, uint8_t data) {
  _i2cPort->beginTransmission(_deviceAddress);
  _i2cPort->write(address);
  _i2cPort->write(data);
  _i2cPort->endTransmission();
  ESP_LOGD("AXP192", "writeByte(%02X) = %02X", address, data);
}

void I2C_AXP192::bitOn(uint8_t address, uint8_t bit) {
  uint8_t add = address;
  uint8_t val = readByte(add) | bit;

  writeByte(add, val);
}

void I2C_AXP192::bitOff(uint8_t address, uint8_t bit) {
  uint8_t add = address;
  uint8_t val = readByte(add) & ~bit;

  writeByte(add, val);
}

// Borrowed from Core2 code
uint8_t I2C_AXP192::calcVoltageData(uint16_t value,
                                    uint16_t maxv,
                                    uint16_t minv,
                                    uint16_t step) {
  uint8_t data = 0;

  if (value > maxv) { value = maxv; }

  if (value > minv) { data = (value - minv) / step; }
  return data;
}

/// @param number 0=DCDC1 / 1=DCDC2 / 2=DCDC3 (Borrowed from Core2 code)
void I2C_AXP192::setDCVoltage(uint8_t  number,
                              uint16_t voltage) {
  if (voltage < 0) { return; }
  uint8_t addr;
  uint8_t vdata;

  if (number > 2) { return; }

  switch (number) {
    case 0:
      addr  = 0x26;
      vdata = calcVoltageData(voltage, 3500, 700, 25) & 0x7F;
      break;
    case 1:
      addr  = 0x25;
      vdata = calcVoltageData(voltage, 2275, 700, 25) & 0x3F;
      break;
    case 2:
      addr  = 0x27;
      vdata = calcVoltageData(voltage, 3500, 700, 25) & 0x7F;
      break;
  }
  writeByte(addr, (readByte(addr) & 0x80) | vdata);
}

void I2C_AXP192::setLDOVoltage(uint8_t  number,
                               uint16_t voltage) {
  if (voltage < 0) { return; }
  uint8_t vdata = calcVoltageData(voltage, 3300, 1800, 100) & 0x0F;

  switch (number) {
    // uint8_t reg, data;
    case 2:
      writeByte(0x28, (readByte(0x28) & 0x0F) | (vdata << 4));
      break;
    case 3:
      writeByte(0x28, (readByte(0x28) & 0xF0) | vdata);
      break;
  }
}

void I2C_AXP192::setDCDC1(int16_t voltage) {
  if (voltage < 0) { return; }
  uint8_t add = 0x12;

  if ((voltage < 700) || (3500 < voltage)) {
    // Disable
    bitOff(add, (1 << 0));
    return;
  } else {
    // Enable
    bitOn(add, (1 << 0));
  }

  // Set
  setDCVoltage(0, voltage);
}

void I2C_AXP192::setDCDC2(int16_t voltage) {
  if (voltage < 0) { return; }
  uint8_t add = 0x12;

  if ((voltage < 700) || (2750 < voltage)) {
    // Disable
    bitOff(add, (1 << 4));
    return;
  } else {
    // Enable
    bitOn(add, (1 << 4));
  }

  // Set
  setDCVoltage(1, voltage);
}

void I2C_AXP192::setDCDC3(int16_t voltage) {
  if (voltage < 0) { return; }
  uint8_t add = 0x12;

  if ((voltage < 700) || (3500 < voltage)) {
    // Disable
    bitOff(add, (1 << 1));
    return;
  } else {
    // Enable
    bitOn(add, (1 << 1));
  }

  // Set
  setDCVoltage(2, voltage);
}

void I2C_AXP192::setLDO2(int16_t voltage) {
  if (voltage < 0) { return; }
  uint8_t add = 0x12;

  if ((voltage < 1800) || (3300 < voltage)) {
    // Disable
    bitOff(add, (1 << 2));
    return;
  } else {
    // Enable
    bitOn(add, (1 << 2));
  }

  // Set
  setLDOVoltage(2, voltage);
}

void I2C_AXP192::setLDO3(int16_t voltage) {
  if (voltage < 0) { return; }
  uint8_t add = 0x12;

  if ((voltage < 1800) || (3300 < voltage)) {
    // Disable
    bitOff(add, (1 << 3));
    return;
  } else {
    // Enable
    bitOn(add, (1 << 3));
  }

  // Set
  setLDOVoltage(3, voltage);
}

void I2C_AXP192::setEXTEN(bool enable) {
  uint8_t add = 0x12;

  if (enable) {
    // Enable
    bitOn(add, (1 << 6));
  } else {
    // Disable
    bitOff(add, (1 << 6));
  }
}

void I2C_AXP192::setBACKUP(bool enable) {
  uint8_t add = 0x35;

  if (enable) {
    // Enable
    bitOn(add, (1 << 7));
  } else {
    // Disable
    bitOff(add, (1 << 7));
  }
}

void I2C_AXP192::setLDOIO(int16_t voltage) {
  if (voltage < 0) { return; }
  uint8_t add = 0x91;

  if ((voltage >= 1800) || (3300 >= voltage)) {
    // Set voltage
    uint8_t val = (calcVoltageData(voltage, 3300, 1800, 100) & 0x0F) << 4;
    writeByte(add, val);
  }
}

void I2C_AXP192::setGPIO0(int state) {
  if (state < 0) { return; }
  const uint8_t add = 0x90;

  if (state == 0) {
    bitOff(add, 1u);
  } else {
    bitOn(add, 1u);
  }
}

void I2C_AXP192::setGPIO1(int state) {
  if (state < 0) { return; }
  const uint8_t add = 0x94;

  if (state == 0) {
    bitOff(add, 2u);
  } else {
    bitOn(add, 2u);
  }
}

void I2C_AXP192::setGPIO2(int state) {
  if (state < 0) { return; }
  const uint8_t add = 0x94;

  if (state == 0) {
    bitOff(add, 4u);
  } else {
    bitOn(add, 4u);
  }
}

void I2C_AXP192::setGPIO3(int state) {
  if (state < 0) { return; }
  const uint8_t add = 0x96;

  if (state == 0) {
    bitOff(add, 1u);
  } else {
    bitOn(add, 1u);
  }
}

void I2C_AXP192::setGPIO4(int state) {
  if (state < 0) { return; }
  const uint8_t add = 0x96;

  if (state == 0) {
    bitOff(add, 2u);
  } else {
    bitOn(add, 2u);
  }
}

float I2C_AXP192::getBatteryVoltage() {
  uint16_t val = readByte(0x78) << 4;

  val |= readByte(0x79);
  return val * 1.1;
}

float I2C_AXP192::getBatteryDischargeCurrent() {
  uint16_t val = readByte(0x7c) << 5;

  val |= readByte(0x7d);
  return val * 0.5;
}

float I2C_AXP192::getBatteryChargeCurrent() {
  uint16_t val = readByte(0x7a) << 5;

  val |= readByte(0x7b);
  return val * 0.5;
}

float I2C_AXP192::getBatteryPower() {
  uint32_t val = (readByte(0x70) << 16) | (readByte(0x71) << 8) | readByte(0x72);

  return 1.1 * 0.5 * val / 1000.0;
}

float I2C_AXP192::getAcinVolatge() {
  uint16_t val = readByte(0x56) << 4;

  val |= readByte(0x57);
  return val * 1.7;
}

float I2C_AXP192::getAcinCurrent() {
  uint16_t val = readByte(0x58) << 4;

  val |= readByte(0x59);
  return val * 0.625;
}

float I2C_AXP192::getVbusVoltage() {
  uint16_t val = readByte(0x5a) << 4;

  val |= readByte(0x5b);
  return val * 1.7;
}

float I2C_AXP192::getVbusCurrent() {
  uint16_t val = readByte(0x5c) << 4;

  val |= readByte(0x5d);
  return val * 0.375;
}

float I2C_AXP192::getInternalTemperature() {
  uint16_t val = readByte(0x5e) << 4;

  val |= readByte(0x5f);
  return -144.7 + val * 0.1;
}

float I2C_AXP192::getApsVoltage() {
  uint16_t val = readByte(0x7e) << 4;

  val |= readByte(0x7f);
  return val * 1.4;
}

void I2C_AXP192::powerOff() {
  bitOn(0x32, (1 << 7));
}

uint8_t I2C_AXP192::getPekPress() {
  uint8_t val = readByte(0x46);

  writeByte(0x46, 0x03);
  return val;
}
