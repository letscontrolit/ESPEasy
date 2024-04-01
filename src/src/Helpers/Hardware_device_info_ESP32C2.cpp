#include "../Helpers/Hardware_device_info.h"

#ifdef ESP32C2

int32_t getEmbeddedFlashSize()
{
  // ESP32-C2 doesn't have eFuse field FLASH_CAP.
  // Can't get info about the flash chip.
  return 0;
}

int32_t getEmbeddedPSRAMSize()
{
  // Doesn't have PSRAM
  return 0;
}
#endif
