#ifndef PLUGINSTRUCTS_P061_DATA_STRUCT_H
#define PLUGINSTRUCTS_P061_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#if defined(USES_P061)

# include <Wire.h> // Needed for I2C

# define P061_CONFIG_I2C_ADDRESS  PCONFIG(0)
# define P061_CONFIG_KEYPAD_TYPE  PCONFIG(1)

// # define P061_DEBUG_LOG true // Set to true to enable debug out log

# define P061_ENABLE_PCF8575 // Enables support for the PCF8575 16 pin I/O expander (needs pull-ups for inputs)

# if defined(LIMIT_BUILD_SIZE) && defined(P061_ENABLE_PCF8575)
#  undef P061_ENABLE_PCF8575 // Saves ca. 800 bytes on ESP8266 (not sure if this needs to be disabled)
# endif // if defined(LIMIT_BUILD_SIZE) && defined(P061_ENABLE_PCF8575)

// MCP23017 Matrix /////////////////////////////////////////////////////////////

# define MCP23017_IODIRA         0x00 // I/O DIRECTION REGISTER   IO7 IO6 IO5 IO4 IO3 IO2 IO1 IO0 1111 1111
# define MCP23017_IODIRB         0x01 // I/O DIRECTION REGISTER   IO7 IO6 IO5 IO4 IO3 IO2 IO1 IO0 1111 1111
# define MCP23017_IPOLA          0x02 // INPUT POLARITY PORT REGISTER   IP7 IP6 IP5 IP4 IP3 IP2 IP1 IP0 0000 0000
# define MCP23017_IPOLB          0x03 // INPUT POLARITY PORT REGISTER   IP7 IP6 IP5 IP4 IP3 IP2 IP1 IP0 0000 0000
# define MCP23017_GPINTENA       0x04 // INTERRUPT-ON-CHANGE PINS   GPINT7 GPINT6 GPINT5 GPINT4 GPINT3 GPINT2 GPINT1 GPINT0 0000 0000
# define MCP23017_GPINTENB       0x05 // INTERRUPT-ON-CHANGE PINS   GPINT7 GPINT6 GPINT5 GPINT4 GPINT3 GPINT2 GPINT1 GPINT0 0000 0000
# define MCP23017_DEFVALA        0x06 // DEFAULT VALUE REGISTER   DEF7 DEF6 DEF5 DEF4 DEF3 DEF2 DEF1 DEF0 0000 0000
# define MCP23017_DEFVALB        0x07 // DEFAULT VALUE REGISTER   DEF7 DEF6 DEF5 DEF4 DEF3 DEF2 DEF1 DEF0 0000 0000
# define MCP23017_INTCONA        0x08 // INTERRUPT-ON-CHANGE CONTROL REGISTER   IOC7 IOC6 IOC5 IOC4 IOC3 IOC2 IOC1 IOC0 0000 0000
# define MCP23017_INTCONB        0x09 // INTERRUPT-ON-CHANGE CONTROL REGISTER   IOC7 IOC6 IOC5 IOC4 IOC3 IOC2 IOC1 IOC0 0000 0000
# define MCP23017_IOCON          0x0A // I/O EXPANDER CONFIGURATION REGISTER   BANK MIRROR SEQOP DISSLW HAEN ODR INTPOL — 0000 0000 - also
                                      // on addr 0x0B
# define MCP23017_GPPUA          0x0C // GPIO PULL-UP RESISTOR REGISTER   PU7 PU6 PU5 PU4 PU3 PU2 PU1 PU0 0000 0000
# define MCP23017_GPPUB          0x0D // GPIO PULL-UP RESISTOR REGISTER   PU7 PU6 PU5 PU4 PU3 PU2 PU1 PU0 0000 0000
# define MCP23017_INTFA          0x0E // INTERRUPT FLAG REGISTER   INT7 INT6 INT5 INT4 INT3 INT2 INT1 INTO 0000 0000
# define MCP23017_INTFB          0x0F // INTERRUPT FLAG REGISTER   INT7 INT6 INT5 INT4 INT3 INT2 INT1 INTO 0000 0000
# define MCP23017_INTCAPA        0x10 // INTERRUPT CAPTURED VALUE FOR PORT REGISTER   ICP7 ICP6 ICP5 ICP4 ICP3 ICP2 ICP1 ICP0 0000 0000
# define MCP23017_INTCAPB        0x11 // INTERRUPT CAPTURED VALUE FOR PORT REGISTER   ICP7 ICP6 ICP5 ICP4 ICP3 ICP2 ICP1 ICP0 0000 0000
# define MCP23017_GPIOA          0x12 // GENERAL PURPOSE I/O PORT REGISTER   GP7 GP6 GP5 GP4 GP3 GP2 GP1 GP0 0000 0000
# define MCP23017_GPIOB          0x13 // GENERAL PURPOSE I/O PORT REGISTER   GP7 GP6 GP5 GP4 GP3 GP2 GP1 GP0 0000 0000
# define MCP23017_OLATA          0x14 // OUTPUT LATCH REGISTER   OL7 OL6 OL5 OL4 OL3 OL2 OL1 OL0 0000 0000
# define MCP23017_OLATB          0x15 // OUTPUT LATCH REGISTER   OL7 OL6 OL5 OL4 OL3 OL2 OL1 OL0 0000 0000

struct P061_data_struct : public PluginTaskData_base {
public:

  P061_data_struct(uint8_t i2c_addr,
                   uint8_t keypadType)
    : _i2c_addr(i2c_addr), _keypadType(keypadType) {}
  P061_data_struct() = delete;
  virtual ~P061_data_struct() = default;

  bool plugin_init(struct EventStruct *event);
  bool plugin_fifty_per_second(struct EventStruct *event);

private:

  // MCP23017
  void    MCP23017_setReg(uint8_t addr,
                          uint8_t reg,
                          uint8_t data);
  uint8_t MCP23017_getReg(uint8_t addr,
                          uint8_t reg);
  void    MCP23017_KeyPadMatrixInit(uint8_t addr);
  uint8_t MCP23017_KeyPadMatrixScan(uint8_t addr);
  void    MCP23017_KeyPadDirectInit(uint8_t addr);
  uint8_t MCP23017_KeyPadDirectScan(uint8_t addr);

  // PCF8574
  void    PCF8574_setReg(uint8_t addr,
                         uint8_t data);
  uint8_t PCF8574_getReg(uint8_t addr);
  void    PCF8574_KeyPadMatrixInit(uint8_t addr);
  uint8_t PCF8574_KeyPadMatrixScan(uint8_t addr);
  void    PCF8574_KeyPadDirectInit(uint8_t addr);
  uint8_t PCF8574_KeyPadDirectScan(uint8_t addr);

  # ifdef P061_ENABLE_PCF8575

  // PCF8575
  void     PCF8575_setReg(uint8_t  addr,
                          uint16_t data);
  uint16_t PCF8575_getReg(uint8_t addr);
  void     PCF8575_KeyPadMatrixInit(uint8_t addr);
  uint8_t  PCF8575_KeyPadMatrixScan(uint8_t addr);
  void     PCF8575_KeyPadDirectInit(uint8_t addr);
  uint8_t  PCF8575_KeyPadDirectScan(uint8_t addr);
  # endif // ifdef P061_ENABLE_PCF8575

  uint8_t _i2c_addr;
  uint8_t _keypadType;
  uint8_t lastScanCode = 0xFF;
  uint8_t sentScanCode = 0xFF;
};

#endif // if defined(USES_P061)
#endif // ifndef PLUGINSTRUCTS_P061_DATA_STRUCT_H
