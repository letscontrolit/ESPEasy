#ifndef ESPEASY_FWD_DECL_H
#define ESPEASY_FWD_DECL_H

#include "ESPEasy_common.h"

// FIXME TD-er: This header file should only be included from .ino or .cpp files
// This is only needed until the classes that need these can include the appropriate .h files to have these forward declared.


void     backgroundtasks();


void flushAndDisconnectAllClients();


float getCPUload();
int getLoopCountPerSec();




void Blynk_Run_c015();




//********************************************************
// Helper Functions for managing the status data structure
//********************************************************

void savePortStatus(uint32_t key, struct portStatusStruct &tempStatus);
bool existPortStatus(uint32_t key);
void removeTaskFromPort(uint32_t key);
void removeMonitorFromPort(uint32_t key);
void addMonitorToPort(uint32_t key);
uint32_t createKey(uint16_t pluginNumber, uint16_t portNumber);
pluginID_t getPluginFromKey(uint32_t key);
uint16_t getPortFromKey(uint32_t key);

void SendStatusOnlyIfNeeded(EventValueSource::Enum eventSource, bool param1, uint32_t key, const String& param2, int16_t param3);
#endif // ESPEASY_FWD_DECL_H
