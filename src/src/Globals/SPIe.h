#pragma once
#if defined(ESP32) && SOC_SPI_PERIPH_NUM > 2

#define FEATURE_HAS_SPIe  1

# include <SPI.h>

extern SPIClass SPIe;
#else 
#define FEATURE_HAS_SPIe  0
#endif // if defined(ESP32) && SOC_SPI_PERIPH_NUM > 2
