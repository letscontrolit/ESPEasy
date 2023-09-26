#ifndef GLOBALS_DEVICE_H
#define GLOBALS_DEVICE_H

#include "../DataStructs/DeviceStruct.h"

#ifdef ESP8266
extern int deviceCount;
#else
extern DeviceCount_t deviceCount;
#endif

int getDeviceCount();

extern DeviceVector Device;

#endif // GLOBALS_DEVICE_H