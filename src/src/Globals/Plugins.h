#ifndef GLOBALS_PLUGIN_H
#define GLOBALS_PLUGIN_H

#include <vector>
#include "../DataStructs/ESPEasyLimits.h"


int getPluginId_from_TaskIndex(byte taskIndex);

String getPluginNameFromDeviceIndex(byte deviceIndex);

extern boolean (*Plugin_ptr[PLUGIN_MAX])(byte, struct EventStruct*, String&);

extern std::vector<byte> Plugin_id;
extern std::vector<int> Task_id_to_Plugin_id;
extern unsigned long countFindPluginId;

#endif // GLOBALS_PLUGIN_H