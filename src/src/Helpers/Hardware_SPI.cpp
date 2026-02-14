
#include "../ESPEasy/net/DataTypes/NetworkMedium.h"
#include "../Helpers/Hardware_SPI.h"
#include "../Globals/Settings.h"

#if FEATURE_SD
# include <SD.h>
#endif // if FEATURE_SD

#include <SPI.h>

#include "../Globals/SPIe.h"

#if FEATURE_ETHERNET
# include <ETH.h>
#endif // if FEATURE_ETHERNET

void initializeSPIBuses() {
  // SPI Init
  uint8_t SPI_initialized = 0;

  #ifdef ESP32
  uint8_t skipInitSPI = 0;
  #endif // ifdef ESP32

  #if FEATURE_ETHERNET

  if ((Settings.NetworkMedium == ESPEasy::net::NetworkMedium_t::Ethernet) &&
      isValid(Settings.ETH_Phy_Type) &&
      isSPI_EthernetType(Settings.ETH_Phy_Type))
  {
    # if !ETH_SPI_SUPPORTS_CUSTOM
    skipInitSPI = Settings.getSPIBusForEth() + 1; // Increment by 1
    # endif // if !ETH_SPI_SUPPORTS_CUSTOM
  }
  #endif // if FEATURE_ETHERNET


  if (Settings.getNrConfiguredSPI_buses())
  {
    #ifdef ESP32

    const SPI_Options_e SPI_selection = static_cast<SPI_Options_e>(Settings.InitSPI);
    int8_t spi_gpios[3]               = {};

    if (skipInitSPI != 1) {
      SPI.end(); // Disconnect current GPIO mapping
    }

    if ((skipInitSPI != 1) && Settings.getSPI_pins(spi_gpios, 0u)) {
      SPI.setHwCs(false);
      SPI.begin(spi_gpios[0], spi_gpios[1], spi_gpios[2]); // Use explicit GPIO configuration
      SPI_initialized |= 1;
    }

    delay(1);

    // Init second SPI interface (SPIe)
    const SPI_Options_e SPI1_selection = static_cast<SPI_Options_e>(Settings.InitSPI1);

    if (skipInitSPI != 2) {
      SPIe.end(); // Disconnect current GPIO mapping
    }

    if ((skipInitSPI != 2) && Settings.getSPI_pins(spi_gpios, 1u)) {
      SPIe.setHwCs(false);
      SPIe.begin(spi_gpios[0], spi_gpios[1], spi_gpios[2]);
      SPI_initialized |= 2;
    }

    #else // ifdef ESP32
    SPI.setHwCs(false);
    SPI.begin(); // Use default GPIO configuration
    SPI_initialized = 1;
    #endif // ifdef ESP32
  }

  if (SPI_initialized)
  {
    #ifdef ESP32

    for (uint8_t i = 1; i < 3; ++i) {
      if (SPI_initialized & i) {
        addLog(LOG_LEVEL_INFO, strformat(F("INIT : SPI Bus %d Init (without CS)"), i - 1));
      }
    }
    #endif // ifdef ESP32
    #ifdef ESP8266
    addLog(LOG_LEVEL_INFO, F("INIT : SPI Init (without CS)"));
    #endif // ifdef ESP8266

    #if FEATURE_SD
    initSDcard();
    #endif // if FEATURE_SD
  } else {
    addLog(LOG_LEVEL_INFO, F("INIT : SPI not enabled"));
  }
}

#if FEATURE_SD
bool initSDcard() {
  bool result = false;

  if (Settings.Pin_sd_cs >= 0)
  {
    # ifdef ESP32
    const uint8_t sdspi = Settings.getSPIBusForSDCard();
    # else // ifdef ESP32
    constexpr uint8_t sdspi{};
    # endif // ifdef ESP32

    if (Settings.isSPI_enabled(sdspi)) {
      # ifdef ESP32

      if (SD.begin(Settings.Pin_sd_cs, (0 == sdspi) ? SPI : SPIe))
      # endif // ifdef ESP32
      # ifdef ESP8266

      if (SD.begin(Settings.Pin_sd_cs))
      # endif // ifdef ESP8266
      {
        # ifdef ESP32
        addLog(LOG_LEVEL_INFO, strformat(F("SD   : Init on SPI Bus %d OK"), sdspi));
        # endif // ifdef ESP32
        # ifdef ESP8266
        addLog(LOG_LEVEL_INFO, F("SD   : Init OK"));
        # endif // ifdef ESP8266
        result = true;
      }
      else
      {
        SD.end();
        addLog(LOG_LEVEL_ERROR, F("SD   : Init failed"));
      }
    }
  }
  return result;
}

#endif // if FEATURE_SD
