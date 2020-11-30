#ifndef ESPEASYCORE_ESPEASYGPIO_H
#define ESPEASYCORE_ESPEASYGPIO_H

#include "../../ESPEasy_common.h"
#include "../DataStructs/PinMode.h"
#include "../DataTypes/PluginID.h"

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


//********************************************************************************
// MCP23017 write
//********************************************************************************
bool GPIO_MCP_Write(int Par1, byte Par2);


//********************************************************************************
// MCP23017 config
//********************************************************************************
void GPIO_MCP_Config(int Par1, byte Par2);

//********************************************************************************
// PCF8574 read
//********************************************************************************
//@giig1967g-20181023: changed to int8_t
int8_t GPIO_PCF_Read(int Par1);

uint8_t GPIO_PCF_ReadAllPins(uint8_t address);

//********************************************************************************
// PCF8574 write
//********************************************************************************
bool GPIO_PCF_Write(int Par1, byte Par2);

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

void setInternalGPIOPullupMode(int port);

bool GPIO_Write(pluginID_t pluginID, int port, byte value, byte pinMode=PIN_MODE_OUTPUT);
bool GPIO_Read(pluginID_t pluginID, int port, int8_t &value);


#endif