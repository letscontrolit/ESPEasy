#include "../PluginStructs/P061_data_struct.h"

#if defined(USES_P061)

bool P061_data_struct::plugin_init(struct EventStruct *event) {
  switch (P061_CONFIG_KEYPAD_TYPE) {
    case 0: MCP23017_KeyPadMatrixInit(_i2c_addr); break;
    case 1: PCF8574_KeyPadMatrixInit(_i2c_addr); break;
    case 2: PCF8574_KeyPadDirectInit(_i2c_addr); break;
  }

  return true;
}

bool P061_data_struct::plugin_fifty_per_second(struct EventStruct *event) {
  uint8_t actScanCode = 0;

  switch (P061_CONFIG_KEYPAD_TYPE) {
    case 0: actScanCode = MCP23017_KeyPadMatrixScan(_i2c_addr); break;
    case 1: actScanCode = PCF8574_KeyPadMatrixScan(_i2c_addr); break;
    case 2: actScanCode = PCF8574_KeyPadDirectScan(_i2c_addr); break;
  }

  if (lastScanCode == actScanCode) {   // debounced? - two times the same value?
    if (sentScanCode != actScanCode) { // any change to last sent data?
      UserVar[event->BaseVarIndex] = actScanCode;
      event->sensorType            = Sensor_VType::SENSOR_TYPE_SWITCH;

      String log = F("KPad : ScanCode=0x");
      log += String(actScanCode, HEX);
      addLog(LOG_LEVEL_INFO, log);

      sendData(event);

      sentScanCode = actScanCode;
    }
  } else {
    lastScanCode = actScanCode;
  }

  return true;
}

void P061_data_struct::MCP23017_setReg(uint8_t addr, uint8_t reg, uint8_t data) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

uint8_t P061_data_struct::MCP23017_getReg(uint8_t addr, uint8_t reg) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(addr, (uint8_t)0x1);

  if (Wire.available()) {
    return Wire.read();
  }
  return 0xFF;
}

void P061_data_struct::MCP23017_KeyPadMatrixInit(uint8_t addr) {
  MCP23017_setReg(addr, MCP23017_IODIRA, 0x00); // port A to output
  MCP23017_setReg(addr, MCP23017_GPIOA,  0x00); // port A to low
  MCP23017_setReg(addr, MCP23017_IODIRB, 0xFF); // port B to input
  MCP23017_setReg(addr, MCP23017_GPPUA,  0xFF); // port A pullup on
  MCP23017_setReg(addr, MCP23017_GPPUB,  0xFF); // port B pullup on
}

uint8_t P061_data_struct::MCP23017_KeyPadMatrixScan(uint8_t addr) {
  uint8_t rowMask = 1;
  uint8_t colData;

  colData = MCP23017_getReg(addr, MCP23017_GPIOB);

  if (colData == 0xFF) { // no key pressed?
    return 0;            // no key pressed!
  }

  for (uint8_t row = 0; row <= 8; row++) {
    if (row == 0) {
      MCP23017_setReg(addr, MCP23017_IODIRA, 0xFF);     // no bit of port A to output
    } else {
      MCP23017_setReg(addr, MCP23017_IODIRA, ~rowMask); // one bit of port A to output 0
      rowMask <<= 1;
    }

    colData = MCP23017_getReg(addr, MCP23017_GPIOB);

    if (colData != 0xFF) { // any key pressed?
      uint8_t colMask = 1;

      for (uint8_t col = 1; col <= 8; col++) {
        if ((colData & colMask) == 0) {                 // this key pressed?
          MCP23017_setReg(addr, MCP23017_IODIRA, 0x00); // port A to output 0
          return (row << 4) | col;
        }
        colMask <<= 1;
      }
    }
  }

  MCP23017_setReg(addr, MCP23017_IODIRA, 0x00); // port A to output 0
  return 0;                                     // no key pressed!
}

// PCF8574 Matrix //////////////////////////////////////////////////////////////

void P061_data_struct::PCF8574_setReg(uint8_t addr, uint8_t data) {
  Wire.beginTransmission(addr);
  Wire.write(data);
  Wire.endTransmission();
}

uint8_t P061_data_struct::PCF8574_getReg(uint8_t addr) {
  Wire.requestFrom(addr, (uint8_t)0x1);

  if (Wire.available()) {
    return Wire.read();
  }
  return 0xFF;
}

void P061_data_struct::PCF8574_KeyPadMatrixInit(uint8_t addr) {
  PCF8574_setReg(addr, 0xF0); // low nibble to output 0
}

uint8_t P061_data_struct::PCF8574_KeyPadMatrixScan(uint8_t addr) {
  uint8_t rowMask = 1;
  uint8_t colData;

  colData = PCF8574_getReg(addr) & 0xF0;

  if (colData == 0xF0) { // no key pressed?
    return 0;            // no key pressed!
  }

  for (uint8_t row = 0; row <= 4; row++) {
    if (row == 0) {
      PCF8574_setReg(addr, 0xFF);     // no bit of port A to output
    } else {
      PCF8574_setReg(addr, ~rowMask); // one bit of port A to output 0
      rowMask <<= 1;
    }

    colData = PCF8574_getReg(addr) & 0xF0;

    if (colData != 0xF0) { // any key pressed?
      uint8_t colMask = 0x10;

      for (uint8_t col = 1; col <= 4; col++) {
        if ((colData & colMask) == 0) { // this key pressed?
          PCF8574_setReg(addr, 0xF0);   // low nibble to output 0
          return (row << 4) | col;
        }
        colMask <<= 1;
      }
    }
  }

  PCF8574_setReg(addr, 0xF0); // low nibble to output 0
  return 0;                   // no key pressed!
}

// PCF8574 Direct //////////////////////////////////////////////////////////////

void P061_data_struct::PCF8574_KeyPadDirectInit(uint8_t addr) {
  PCF8574_setReg(addr, 0xFF); // all to input
}

uint8_t P061_data_struct::PCF8574_KeyPadDirectScan(uint8_t addr) {
  uint8_t colData;

  colData = PCF8574_getReg(addr);

  if (colData == 0xFF) { // no key pressed?
    return 0;            // no key pressed!
  }
  uint8_t colMask = 0x01;

  for (uint8_t col = 1; col <= 8; col++) {
    if ((colData & colMask) == 0) { // this key pressed?
      return col;
    }
    colMask <<= 1;
  }

  return 0; // no key pressed!
}

#endif // if defined(USES_P061)
