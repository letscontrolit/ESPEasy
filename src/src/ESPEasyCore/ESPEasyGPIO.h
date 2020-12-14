#ifndef ESPEASYCORE_ESPEASYGPIO_H
#define ESPEASYCORE_ESPEASYGPIO_H

#include "../../ESPEasy_common.h"
#include "../DataStructs/PinMode.h"
#include "../DataTypes/PluginID.h"

// MCP registers
#define MCP23017_IODIRA 0x00   //!< I/O direction register A
#define MCP23017_IPOLA 0x02    //!< Input polarity port register A
#define MCP23017_GPINTENA 0x04 //!< Interrupt-on-change pins A
#define MCP23017_DEFVALA 0x06  //!< Default value register A
#define MCP23017_INTCONA 0x08  //!< Interrupt-on-change control register A
#define MCP23017_IOCONA 0x0A   //!< I/O expander configuration register A
#define MCP23017_GPPUA 0x0C    //!< GPIO pull-up resistor register A
#define MCP23017_INTFA 0x0E    //!< Interrupt flag register A
#define MCP23017_INTCAPA 0x10  //!< Interrupt captured value for port register A
#define MCP23017_GPIOA 0x12    //!< General purpose I/O port register A
#define MCP23017_OLATA 0x14    //!< Output latch register 0 A

#define MCP23017_IODIRB 0x01   //!< I/O direction register B
#define MCP23017_IPOLB 0x03    //!< Input polarity port register B
#define MCP23017_GPINTENB 0x05 //!< Interrupt-on-change pins B
#define MCP23017_DEFVALB 0x07  //!< Default value register B
#define MCP23017_INTCONB 0x09  //!< Interrupt-on-change control register B
#define MCP23017_IOCONB 0x0B   //!< I/O expander configuration register B
#define MCP23017_GPPUB 0x0D    //!< GPIO pull-up resistor register B
#define MCP23017_INTFB 0x0F    //!< Interrupt flag register B
#define MCP23017_INTCAPB 0x11  //!< Interrupt captured value for port register B
#define MCP23017_GPIOB 0x13    //!< General purpose I/O port register B
#define MCP23017_OLATB 0x15    //!< Output latch register 0 B

#define MCP23017_INT_ERR 255 //!< Interrupt error

//********************************************************************************
// Internal GPIO write
//********************************************************************************
void GPIO_Internal_Write(int pin, byte value);

//********************************************************************************
// Internal GPIO read
//********************************************************************************
bool GPIO_Internal_Read(int pin);

bool GPIO_Read_Switch_State(struct EventStruct *event);

bool GPIO_Read_Switch_State(int pinNumber, byte pinMode);

//********************************************************************************
// MCP23017 read
//********************************************************************************
int8_t GPIO_MCP_Read(int Par1);
bool GPIO_MCP_ReadRegister(byte mcpAddr, uint8_t regAddr, uint8_t *retValue);

//********************************************************************************
// MCP23017 write
//********************************************************************************
bool GPIO_MCP_Write(int Par1, byte Par2);
void GPIO_MCP_WriteRegister(byte mcpAddr, uint8_t regAddr, uint8_t regValue);

//********************************************************************************
// MCP23017 config
//********************************************************************************
void GPIO_MCP_Config(int Par1, byte Par2);

//********************************************************************************
// PCF8574 read
//********************************************************************************
//@giig1967g-20181023: changed to int8_t
int8_t GPIO_PCF_Read(int Par1);
bool GPIO_PCF_ReadAllPins(uint8_t address, uint8_t *retValue);

//********************************************************************************
// PCF8574 write
//********************************************************************************
bool GPIO_PCF_Write(int Par1, byte Par2);
void GPIO_PCF_WriteAllPins(uint8_t Par1, uint8_t Par2);

//*********************************************************
// GPIO_Monitor10xSec:
// What it does:
// a) It updates the pinstate structure
// b) Returns an EVENT in case monitor is enabled
// c) Does not update the pinstate state if the port is defined in a task, as it will be picked up by thePLUGIN_TEN_PER_SECOND event
// called from: run10TimesPerSecond in ESPEasy.ino
//*********************************************************

void GPIO_Monitor10xSec();

// prefix should be either "GPIO", "PCF", "MCP"
void sendMonitorEvent(const char* prefix, int port, int8_t state);

bool checkValidPortRange(pluginID_t pluginID, int port);
bool checkValidPortAddress(pluginID_t pluginID, byte address);

void setInternalGPIOPullupMode(int port);

bool GPIO_Write(pluginID_t pluginID, int port, byte value, byte pinMode=PIN_MODE_OUTPUT);
bool GPIO_Read(pluginID_t pluginID, int port, int8_t &value);


#endif