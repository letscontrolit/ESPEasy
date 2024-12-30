#include "../Helpers/Hardware_device_info.h"

#ifdef ESP32C5


const __FlashStringHelper* getChipModel(uint32_t chip_model, uint32_t chip_revision, uint32_t pkg_version, bool single_core)
{
  if (17 == chip_model) {      // ESP32-C5 beta3 (MPW)
    return F("ESP32-C5 beta3");
  }
  else if (23 == chip_model) { // ESP32-C5 MP
    return F("ESP32-C5");
  }

  return F("Unknown");
}

#endif // ifdef ESP32C5
