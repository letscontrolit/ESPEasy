
#include "../ESPEasy/net/DataTypes/NetworkMedium.h"
#include "../Helpers/Hardware_SPI.h"
#include "../Globals/Settings.h"

#if FEATURE_SPI
# if FEATURE_SD
#  include <SD.h>
# endif // if FEATURE_SD

# include <SPI.h>

# include "../Globals/SPIe.h"

# if FEATURE_ETHERNET
#  include <ETH.h>
# endif // if FEATURE_ETHERNET

void initializeSPIBuses() {
  // SPI Init
  uint8_t SPI_initialized = 0;

  # ifdef ESP32
  uint8_t skipInitSPI = 0xff;
  # endif // ifdef ESP32

  # if FEATURE_ETHERNET

  if ((Settings.NetworkMedium == ESPEasy::net::NetworkMedium_t::Ethernet) &&
      isValid(Settings.ETH_Phy_Type) &&
      isSPI_EthernetType(Settings.ETH_Phy_Type))
  {
    #  if !ETH_SPI_SUPPORTS_CUSTOM
    skipInitSPI = Settings.getSPIBusForEth();
    #  endif // if !ETH_SPI_SUPPORTS_CUSTOM
  }
  # endif // if FEATURE_ETHERNET


  if (Settings.getNrConfiguredSPI_buses())
  {
    # ifdef ESP32

    for (int spi_bus = 0; (spi_bus + 1) < SOC_SPI_PERIPH_NUM; ++spi_bus) {
      if (skipInitSPI != spi_bus) {
        auto spi_ptr = getSPI(spi_bus);

        if (spi_ptr) {
          spi_ptr->end(); // Disconnect current GPIO mapping

          int8_t spi_gpios[3]{};

          if (Settings.getSPI_pins(spi_gpios, spi_bus)) {
            spi_ptr->setHwCs(false);

            if (spi_ptr->begin(spi_gpios[0], spi_gpios[1], spi_gpios[2])) { // Use explicit GPIO configuration
              bitSet(SPI_initialized, spi_bus);
            }
          }
          delay(1);
        }
      }
    }
    # else // ifdef ESP32
    SPI.setHwCs(false);
    SPI.begin(); // Use default GPIO configuration
    SPI_initialized = 1;
    # endif // ifdef ESP32
  }

  if (SPI_initialized)
  {
    # ifdef ESP32

    for (int spi_bus = 0; (spi_bus + 1) < SOC_SPI_PERIPH_NUM; ++spi_bus) {
      if (bitRead(SPI_initialized, spi_bus)) {
        addLog(LOG_LEVEL_INFO, strformat(F("INIT : SPI Bus %d Init (without CS)"), spi_bus));
      }
    }
    # endif // ifdef ESP32
    # ifdef ESP8266
    addLog(LOG_LEVEL_INFO, F("INIT : SPI Init (without CS)"));
    # endif // ifdef ESP8266

    # if FEATURE_SD
    initSDcard();
    # endif // if FEATURE_SD
  } else {
    addLog(LOG_LEVEL_INFO, F("INIT : SPI not enabled"));
  }
}

SPIClass* getSPI(uint8_t spi_bus)
{
  const SPI_Options_e SPI_selection = Settings.getSPISelection(spi_bus);

  if (SPI_Options_e::None != SPI_selection) {
    if ((SPI_selection == SPI_Options_e::Vspi_Fspi) ||
        (SPI_selection == SPI_Options_e::UserDefined_VSPI)) {
      return &SPI;
    }
# if FEATURE_HAS_SPIe
    return &SPIe;
# endif
  }
  return nullptr;
}

bool getSPI(SPIClass& spi, uint8_t spi_bus)
{
  auto spi_ptr = getSPI(spi_bus);

  if (!spi_ptr) {
    spi = SPI;
    return false;
  }
  spi = *spi_ptr;
  return true;
}

SPIClass* getSPIBusForTask(taskIndex_t TaskIndex)
{
  if (!validTaskIndex(TaskIndex)) { return nullptr; }
  return getSPI(Settings.getSPIBusForTask(TaskIndex));
}

# if FEATURE_SD

bool initSDcard() {
  bool result = false;

  if (Settings.Pin_sd_cs >= 0)
  {
    #  ifdef ESP32
    const uint8_t sdspi = Settings.getSPIBusForSDCard();
    #  else // ifdef ESP32
    constexpr uint8_t sdspi{};
    #  endif // ifdef ESP32

    if (Settings.isSPI_enabled(sdspi)) {
      #  ifdef ESP32
      auto spi_ptr = getSPI(sdspi);

      if (spi_ptr && SD.begin(Settings.Pin_sd_cs, *spi_ptr))
      #  endif // ifdef ESP32
      #  ifdef ESP8266

      if (SD.begin(Settings.Pin_sd_cs))
      #  endif // ifdef ESP8266
      {
        #  ifdef ESP32
        addLog(LOG_LEVEL_INFO, strformat(F("SD   : Init on SPI Bus %d OK"), sdspi));
        #  endif // ifdef ESP32
        #  ifdef ESP8266
        addLog(LOG_LEVEL_INFO, F("SD   : Init OK"));
        #  endif // ifdef ESP8266
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

# endif // if FEATURE_SD
#endif // if FEATURE_SPI
