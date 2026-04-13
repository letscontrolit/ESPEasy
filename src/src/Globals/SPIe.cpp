#include "../Globals/SPIe.h"

#if defined(ESP32) && SOC_SPI_PERIPH_NUM > 2
SPIClass SPIe(HSPI);
#endif
