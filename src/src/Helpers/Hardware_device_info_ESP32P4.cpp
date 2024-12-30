#include "../Helpers/Hardware_device_info.h"

#ifdef ESP32P4


const __FlashStringHelper* getChipModel(uint32_t chip_model, uint32_t chip_revision, uint32_t pkg_version, bool single_core)
{
  if (18 == chip_model) { // ESP32-P4
    return F("ESP32-P4");
  }

  return F("Unknown");
}

#endif // ifdef ESP32P4
