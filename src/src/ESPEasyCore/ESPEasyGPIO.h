#ifndef ESPEASYCORE_ESPEASYGPIO_H
#define ESPEASYCORE_ESPEASYGPIO_H

#include "../../ESPEasy_common.h"
#include "../DataStructs/PinMode.h"
#include "../DataTypes/PluginID.h"

// MCP registers
#define MCP23017_IODIRA 0x00   //!< I/O direction register A
#define MCP23017_GPPUA  0x0C   //!< Pullup resistor register A
#define MCP23017_GPIOA  0x12   //!< General purpose I/O port register A

#define MCP23017_IODIRB 0x01   //!< I/O direction register B
#define MCP23017_GPPUB  0x0D   //!< Pullup resistor register B
#define MCP23017_GPIOB  0x13   //!< General purpose I/O port register B

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
// MCP23017 pullUP
//********************************************************************************

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

void setInternalGPIOPullupMode(uint8_t port);
bool setMCPInputAndPullupMode(uint8_t Par1, bool enablePullUp);
bool setMCPOutputMode(uint8_t Par1);
bool setPCFInputMode(uint8_t pin);

bool GPIO_Write(pluginID_t pluginID, int port, byte value, byte pinMode=PIN_MODE_OUTPUT);
bool GPIO_Read(pluginID_t pluginID, int port, int8_t &value);

#endif