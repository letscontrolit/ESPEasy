#include "../Helpers/Hardware_device_info.h"

#ifdef ESP32C61


const __FlashStringHelper* getChipModel(uint32_t chip_model, uint32_t chip_revision, uint32_t pkg_version, bool single_core)
{
  if (20 == chip_model) { // ESP32-C61
    return F("ESP32-C61");
  }

  return F("Unknown");
}

#endif // ifdef ESP32C61
