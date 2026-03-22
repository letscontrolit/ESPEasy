#include "../Helpers/Hardware_device_info.h"

#ifdef ESP32P4

#include <soc/efuse_reg.h>

const __FlashStringHelper* getChipModel(uint32_t chip_model, uint32_t chip_revision, uint32_t pkg_version, bool single_core)
{
  if (18 == chip_model) { // ESP32-P4
    const uint16_t rev = getChipRevision_val();
    return rev < 300 ? F("ESP32-P4") : F("ESP32-P4r3");
  }

  return F("Unknown");
}

int32_t getEmbeddedFlashSize()
{
  // Returned size in MB
  return  ESP.getFlashChipSize() >> 20;
}

int32_t getEmbeddedPSRAMSize()
{
  const uint32_t psram_cap = REG_GET_FIELD(EFUSE_RD_MAC_SYS_2_REG, EFUSE_PSRAM_CAP);

 // TODO TD-er: Must check whether this is true
  switch (psram_cap) {
    case 0: return 0;
    case 1: return 16;
    case 2: return 32;
  }

  // Unknown value, thus mark as negative value
  return -1 * static_cast<int32_t>(psram_cap);
}

# ifndef isPSRAMInterfacePin
bool isPSRAMInterfacePin(int gpio) {
  // PSRAM and flash do not use GPIO pins
  // The ESP32-P4 chip has dedicated pins for external flash and in-package PSRAM. 
  // Such pins can not be used for other purpose.
  return false;
}

# endif // ifndef isPSRAMInterfacePin

bool isFlashInterfacePin_ESPEasy(int gpio) {
  // PSRAM and flash do not use GPIO pins
  // The ESP32-P4 chip has dedicated pins for external flash and in-package PSRAM. 
  // Such pins can not be used for other purpose.
  return false;
}

#endif // ifdef ESP32P4
