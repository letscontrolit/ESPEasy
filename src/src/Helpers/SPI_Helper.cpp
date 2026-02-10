#include "../Helpers/SPI_Helper.h"

void SPIInterfaceSelector(String  label,
                          String  id,
                          uint8_t choice,
                          bool    disabled) {
  const uint8_t spiCount       = getSPIBusCount();
  const bool    spi0valid      = Settings.isSPI_valid(0);
  const bool    spi1valid      = Settings.isSPI_valid(1);
  const uint8_t spiBitmap      = (spi0valid ? 1 : 0) | (spi1valid ? 2 : 0);
  const uint8_t spiMaxBusCount = (spiCount > 1
                                  ? (spi1valid ? 1 : 0)
                                  : 0) + (spi0valid ? 1 : 0);

  if ((spiMaxBusCount > 1) || spi1valid) { // Show selector if only bus 1 is enabled
    static uint8_t spiBusCount{};
    static uint8_t spiBusBitmap{};
    static int     spiBusNumbers[2];
    static String  spiBusAttr[2];

    if (spiBusBitmap != spiBitmap) {
      spiBusCount                = 0;
      spiBusNumbers[spiBusCount] = 0;
      spiBusAttr[0]              = spi0valid ? EMPTY_STRING : F("disabled");
      spiBusBitmap               = spi0valid ? 1 : 0;
      ++spiBusCount;

      if ((spiCount > 1) && spi1valid) {
        spiBusNumbers[spiBusCount] = 1;
        spiBusBitmap              |= 2;
        ++spiBusCount;
      }
    }
    FormSelectorOptions selector(spiBusCount,
                                 spiBusNumbers,
                                 spiBusAttr);
    selector.default_index = 0;
    selector.enabled       = !disabled;
    selector.addFormSelector(label, id, choice);
  }
}
