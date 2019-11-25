/****************************************************************************/
// Central functions for GPIO handling
// **************************************************************************/
#include "src/DataStructs/PinMode.h"
#include "src/Globals/GlobalMapPortStatus.h"
#include "ESPEasy_fdwdecl.h"

bool GPIO_Write(byte pluginID, byte port, byte value, byte pinMode=PIN_MODE_OUTPUT);
bool GPIO_Read(byte pluginID, byte port, int8_t &value);

void GPIO_Internal_Write(byte pin, byte value);
bool GPIO_Internal_Read(byte pin);
bool GPIO_Read_Switch_State(struct EventStruct *event);
bool GPIO_Read_Switch_State(byte pinNumber, byte pinMode);

int8_t GPIO_MCP_Read(byte Par1);
bool GPIO_MCP_Write(byte Par1, byte Par2);
void GPIO_MCP_Config(byte Par1, byte Par2);

int8_t GPIO_PCF_Read(byte Par1);
uint8_t GPIO_PCF_ReadAllPins(uint8_t address);
bool GPIO_PCF_Write(byte Par1, byte Par2);

void GPIO_Monitor10xSec();
void sendMonitorEvent(const char* prefix, byte port, int8_t state);
bool checkValidPortRange(byte pluginID, byte port);
void setGPIOPullupMode(byte port);
