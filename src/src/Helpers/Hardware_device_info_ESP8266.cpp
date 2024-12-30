#include "../Helpers/Hardware_device_info.h"


#ifdef ESP8266



bool isESP8285(uint32_t& pkg_version, bool& high_temp_version)
{
  // Original code from Tasmota:
  // https://github.com/arendst/Tasmota/blob/62675a37a0e7b46283e2fdfe459bb8fd29d1cc2a/tasmota/tasmota_support/support_esp.ino#L151

  /*
     ESP8266 SoCs
     - 32-bit MCU & 2.4 GHz Wi-Fi
     - High-performance 160 MHz single-core CPU
     - +19.5 dBm output power ensures a good physical range
     - Sleep current is less than 20 μA, making it suitable for battery-powered and wearable-electronics applications
     - Peripherals include UART, GPIO, I2C, I2S, SDIO, PWM, ADC and SPI
   */

  // esptool.py get_efuses
  uint32_t efuse0 = *(uint32_t *)(0x3FF00050);

  //  uint32_t efuse1 = *(uint32_t*)(0x3FF00054);
  uint32_t efuse2 = *(uint32_t *)(0x3FF00058);
  uint32_t efuse3 = *(uint32_t *)(0x3FF0005C);

  bool r0_4  = efuse0 & (1 << 4);    // ESP8285
  bool r2_16 = efuse2 & (1 << 16);   // ESP8285

  if (r0_4 || r2_16) {               // ESP8285
    //                                                              1M 2M 2M 4M flash size
    //   r0_4                                                       1  1  0  0
    bool r3_25 = efuse3 & (1 << 25); // flash matrix  0  0  1  1
    bool r3_26 = efuse3 & (1 << 26); // flash matrix  0  1  0  1
    bool r3_27 = efuse3 & (1 << 27); // flash matrix  0  0  0  0
    pkg_version = 0;

    if (!r3_27) {
      if (r0_4 && !r3_25) {
        pkg_version = (r3_26) ? 2 : 1;
      }
      else if (!r0_4 && r3_25) {
        pkg_version = (r3_26) ? 4 : 2;
      }
    }
    high_temp_version = efuse0 & (1 << 5); // Max flash temperature (0 = 85C, 1 = 105C)
    return true;
  }
  return false;
}

bool isESP8285() {
  uint32_t pkg_version{};
  bool     high_temp_version{};
  return isESP8285(pkg_version, high_temp_version);
}

bool isFlashInterfacePin_ESPEasy(int gpio) {
  if (isESP8285()) {
    return (gpio) == 6 || (gpio) == 7 || (gpio) == 8 || (gpio) == 11;
  }
  return (gpio) >= 6 && (gpio) <= 11;
}

const __FlashStringHelper* getChipModel() {
  uint32_t pkg_version{};
  bool     high_temp_version{};

  if (isESP8285(pkg_version, high_temp_version)) {
    switch (pkg_version) {
      case 1:
        return (high_temp_version)
          ? F("ESP8285H08") // 1M flash
          : F("ESP8285N08");
      case 2:
        return (high_temp_version)
          ? F("ESP8285H16") // 2M flash
          : F("ESP8285N16");
      case 4:
        return (high_temp_version)
          ? F("ESP8285H32") // 4M flash
          : F("ESP8285N32");
    }
    return F("ESP8285");
  }
  return F("ESP8266EX");
}

#endif