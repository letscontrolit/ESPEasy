#ifndef HELPERS_PORTSTATUS_H
#define HELPERS_PORTSTATUS_H

#include <Arduino.h>

#include "../DataStructs/PortStatusStruct.h"
#include "../Globals/Plugins.h"


#ifdef ESP32
void checkAndClearPWM(uint32_t key);
#endif


/**********************************************************
*                                                         *
* Helper Functions for managing the status data structure *
*                                                         *
**********************************************************/
void savePortStatus(uint32_t key, struct portStatusStruct& tempStatus);

bool existPortStatus(uint32_t key);

void removeTaskFromPort(uint32_t key);

void removeMonitorFromPort(uint32_t key);

void addMonitorToPort(uint32_t key);

uint32_t createKey(uint16_t pluginNumber, uint16_t portNumber);

pluginID_t getPluginFromKey(uint32_t key);

uint16_t getPortFromKey(uint32_t key);



/*********************************************************************************************\
   set pin mode & state (info table)
\*********************************************************************************************/
/*
   void setPinState(byte plugin, byte index, byte mode, uint16_t value);
 */

/*********************************************************************************************\
   get pin mode & state (info table)
\*********************************************************************************************/

/*
   bool getPinState(byte plugin, byte index, byte *mode, uint16_t *value);

 */
/*********************************************************************************************\
   check if pin mode & state is known (info table)
\*********************************************************************************************/
/*
   bool hasPinState(byte plugin, byte index);

 */


/*********************************************************************************************\
   report pin mode & state (info table) using json
\*********************************************************************************************/
String getPinStateJSON(bool          search,
                       uint32_t      key,
                       const String& log,
                       int16_t       noSearchValue);

const __FlashStringHelper * getPinModeString(byte mode);

#endif