#ifndef HELPERS_HARDWARE_DEFINES_H
#define HELPERS_HARDWARE_DEFINES_H


#ifdef ESP32
# include <driver/adc.h>

// Needed to get ADC Vref
# include <esp_adc_cal.h>
# include <driver/adc.h>
#endif // ifdef ESP32

#ifdef ESP32
# if CONFIG_IDF_TARGET_ESP32
  #  define MAX_ADC_VALUE 4095
# else // if CONFIG_IDF_TARGET_ESP32
  #  define MAX_ADC_VALUE ((1 << SOC_ADC_MAX_BITWIDTH) - 1)
# endif  // if CONFIG_IDF_TARGET_ESP32
#endif  // ifdef ESP32
#ifdef ESP8266
  # if FEATURE_ADC_VCC

// Vcc in units of 1/1024 V
  #  define MAX_ADC_VALUE 4095
  # else // if FEATURE_ADC_VCC
  #  define MAX_ADC_VALUE 1023
  # endif // if FEATURE_ADC_VCC
#endif // ifdef ESP8266


#ifdef ESP32_CLASSIC
# define MAX_TX_PWR_DBM_11b  19.5f
# define MAX_TX_PWR_DBM_54g  16.0f
# define MAX_TX_PWR_DBM_n    14.0f
# define WIFI_SENSITIVITY_11b  -88
# define WIFI_SENSITIVITY_54g  -75
# define WIFI_SENSITIVITY_n    -70
#elif defined(ESP32S2)
# define MAX_TX_PWR_DBM_11b  19.5f
# define MAX_TX_PWR_DBM_54g  15.0f
# define MAX_TX_PWR_DBM_n    13.0f
# define WIFI_SENSITIVITY_11b  -88
# define WIFI_SENSITIVITY_54g  -75
# define WIFI_SENSITIVITY_n    -72
#elif defined(ESP32S3)
# define MAX_TX_PWR_DBM_11b  21.0f
# define MAX_TX_PWR_DBM_54g  19.0f
# define MAX_TX_PWR_DBM_n    18.5f
# define WIFI_SENSITIVITY_11b  -88
# define WIFI_SENSITIVITY_54g  -76
# define WIFI_SENSITIVITY_n    -72
#elif defined(ESP32C3)
# define MAX_TX_PWR_DBM_11b  21.0f
# define MAX_TX_PWR_DBM_54g  19.0f
# define MAX_TX_PWR_DBM_n    18.5f
# define WIFI_SENSITIVITY_11b  -88
# define WIFI_SENSITIVITY_54g  -76
# define WIFI_SENSITIVITY_n    -73
#elif defined(ESP8266)
# define MAX_TX_PWR_DBM_11b  20.0f
# define MAX_TX_PWR_DBM_54g  17.0f
# define MAX_TX_PWR_DBM_n    14.0f
# define WIFI_SENSITIVITY_11b  -91
# define WIFI_SENSITIVITY_54g  -75
# define WIFI_SENSITIVITY_n    -72
#else // ifdef ESP32_CLASSIC
static_assert(false, "Implement processor architecture");
#endif // ifdef ESP32_CLASSIC


#ifndef SOC_RX0
# if CONFIG_IDF_TARGET_ESP32
#  define SOC_RX0 3
# elif CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#  define SOC_RX0 44
# elif CONFIG_IDF_TARGET_ESP32C3
#  define SOC_RX0 20
# elif defined(ESP8266)
#  define SOC_RX0 3
# endif // if CONFIG_IDF_TARGET_ESP32
#endif   // ifndef SOC_RX0

#ifndef SOC_TX0
# if CONFIG_IDF_TARGET_ESP32
#  define SOC_TX0 1
# elif CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#  define SOC_TX0 43
# elif CONFIG_IDF_TARGET_ESP32C3
#  define SOC_TX0 21
# elif defined(ESP8266)
#  define SOC_TX0 1
# endif // if CONFIG_IDF_TARGET_ESP32
#endif   // ifndef SOC_TX0


#ifndef PIN_USB_D_MIN
#if defined(ESP32S2) ||  defined(ESP32S3)
#define PIN_USB_D_MIN  19
#endif
#ifdef ESP32C3
#define PIN_USB_D_MIN  18
#endif
#endif

#ifndef PIN_USB_D_PLUS
#if defined(ESP32S2) ||  defined(ESP32S3)
#define PIN_USB_D_PLUS 20
#endif
#ifdef ESP32C3
#define PIN_USB_D_PLUS 19
#endif
#endif

#ifndef isFlashInterfacePin
# if CONFIG_IDF_TARGET_ESP32
// GPIO-6 ... 11: SPI flash and PSRAM
// GPIO-16 & 17: CS for PSRAM, thus only unuable when PSRAM is present
#define isFlashInterfacePin(p)      ((p) >= 6 && (p) <= 11)

# elif CONFIG_IDF_TARGET_ESP32S2
// GPIO-22 ... 25: SPI flash and PSRAM
// GPIO-26: CS for PSRAM, thus only unuable when PSRAM is present
// GPIO-27 ... 32: SPI 8 ­line mode (OPI) pins for flash or PSRAM (e.g. ESP32-S2FH2 and ESP32-S2FH4)
#define isFlashInterfacePin(p)      ((p) >= 22 && (p) <= 25)

# elif CONFIG_IDF_TARGET_ESP32S3
// GPIO-26 ... 32: SPI flash and PSRAM
// GPIO-33 ... 37: SPI 8 ­line mode (OPI) pins for flash or PSRAM, like ESP32-S3R8 / ESP32-S3R8V.
#define isFlashInterfacePin(p)       ((p) >= 26 && (p) <= 32)

# elif CONFIG_IDF_TARGET_ESP32C3
// GPIO-11: Flash voltage selector
// GPIO-12 ... 17: Connected to flash
#define isFlashInterfacePin(p)      ((p) >= 12 && (p) <= 17)

# elif defined(ESP8266)
#define isFlashInterfacePin(p)      ((p) == 6 || (p) == 7 || (p) == 8 || (p) == 11)

# endif // if CONFIG_IDF_TARGET_ESP32
#endif


#ifndef isPSRAMInterfacePin
# if CONFIG_IDF_TARGET_ESP32
// GPIO-6 ... 11: SPI flash and PSRAM
// GPIO-16 & 17: CS for PSRAM, thus only unuable when PSRAM is present
#define isPSRAMInterfacePin(p)      (FoundPSRAM() ? ((p) == 16 || (p) == 17) : false)

# elif CONFIG_IDF_TARGET_ESP32S2
// GPIO-22 ... 25: SPI flash and PSRAM
// GPIO-26: CS for PSRAM, thus only unuable when PSRAM is present
// GPIO-27 ... 32: SPI 8 ­line mode (OPI) pins for flash or PSRAM (e.g. ESP32-S2FH2 and ESP32-S2FH4)
// GPIO-27 ... 32: are never made accessible
#define isPSRAMInterfacePin(p)      (FoundPSRAM() ? ((p) >= 26 && (p) <= 32) : false)

# elif CONFIG_IDF_TARGET_ESP32S3
// GPIO-26 ... 32: SPI flash and PSRAM
// GPIO-33 ... 37: SPI 8 ­line mode (OPI) pins for flash or PSRAM, like ESP32-S3R8 / ESP32-S3R8V.
// See Appendix A, page 71: https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf
#define isPSRAMInterfacePin(p)      (FoundPSRAM() ? ((p) >= 33 && (p) <= 37) : false)

# elif CONFIG_IDF_TARGET_ESP32C3
// GPIO-11: Flash voltage selector
// GPIO-12 ... 17: Connected to flash
// #define isPSRAMInterfacePin(p)      (false)

# elif defined(ESP8266)
// #define isPSRAMInterfacePin(p)      (false)

# endif // if CONFIG_IDF_TARGET_ESP32
#endif




#endif // ifndef HELPERS_HARDWARE_DEFINES_H
