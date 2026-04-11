#include "../Helpers/Hardware_device_info.h"

#ifdef ESP32H2


const __FlashStringHelper* getChipModel(uint32_t chip_model, uint32_t chip_revision, uint32_t pkg_version, bool single_core)
{
  if (10 == chip_model) return F("ESP32-H2(beta1)");
  if (14 == chip_model) return F("ESP32-H2(beta2)");
  if (CHIP_ESP32H2 == chip_model) { // ESP32-H2
    return F("ESP32-H2");
  }

  return F("Unknown");
}

#endif // ifdef ESP32H2
