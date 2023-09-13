#include "../Globals/Device.h"

// TODO TD-er: Move often used functions like determine valueCount to here.
#ifdef ESP8266
int deviceCount = -1;
#else
DeviceCount_t deviceCount;
#endif

int getDeviceCount()
{
#ifdef ESP8266
    return deviceCount;
#else
    return deviceCount.value;
#endif
}

DeviceVector Device;