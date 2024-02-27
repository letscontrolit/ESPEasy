#include "../Helpers/Hardware_device_info.h"

#ifdef ESP32_CLASSIC

int32_t getEmbeddedFlashSize()
{
  return 0;
}

int32_t getEmbeddedPSRAMSize()
{
  // FIXME TD-er: Need to implement
  return 0;
}

#endif
