#ifndef HELPERS_HARDWARE_DEFINES_H
#define HELPERS_HARDWARE_DEFINES_H


#ifdef ESP32
#if ESP_IDF_VERSION_MAJOR < 5
# include <driver/adc.h>

// Needed to get ADC Vref
# include <esp_adc_cal.h>
# include <driver/adc.h>
#endif
#endif // ifdef ESP32

#ifdef ESP32
# if CONFIG_IDF_TARGET_ESP32
  #  define MAX_ADC_VALUE 4095
# else // if CONFIG_IDF_TARGET_ESP32
  #if ESP_IDF_VERSION_MAJOR < 5
  #  define MAX_ADC_VALUE ((1 << SOC_ADC_MAX_BITWIDTH) - 1)
  #else
  #  define MAX_ADC_VALUE ((1 << SOC_ADC_DIGI_MAX_BITWIDTH) - 1)
  #endif
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
#elif defined(ESP32C2) || defined(ESP32C3) || defined(ESP32C6)
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
# elif CONFIG_IDF_TARGET_ESP32C6
#  define SOC_RX0 17
# elif CONFIG_IDF_TARGET_ESP32C3
#  define SOC_RX0 20
# elif CONFIG_IDF_TARGET_ESP32C2
#  define SOC_RX0 19
# elif defined(ESP8266)
#  define SOC_RX0 3
# endif // if CONFIG_IDF_TARGET_ESP32
#endif   // ifndef SOC_RX0

#ifndef SOC_TX0
# if CONFIG_IDF_TARGET_ESP32
#  define SOC_TX0 1
# elif CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#  define SOC_TX0 43
# elif CONFIG_IDF_TARGET_ESP32C6
#  define SOC_TX0 16
# elif CONFIG_IDF_TARGET_ESP32C3
#  define SOC_TX0 21
# elif CONFIG_IDF_TARGET_ESP32C2
#  define SOC_TX0 20
# elif defined(ESP8266)
#  define SOC_TX0 1
# endif // if CONFIG_IDF_TARGET_ESP32
#endif   // ifndef SOC_TX0


#ifndef PIN_USB_D_MIN
#if defined(ESP32S2) ||  defined(ESP32S3)
#define PIN_USB_D_MIN  19
#endif
#ifdef ESP32C6
#define PIN_USB_D_MIN  12
#endif
#ifdef ESP32C3
#define PIN_USB_D_MIN  18
#endif
// ESP32-C2 doesn't seem to have USB pins
#endif

#ifndef PIN_USB_D_PLUS
#if defined(ESP32S2) ||  defined(ESP32S3)
#define PIN_USB_D_PLUS 20
#endif
#ifdef ESP32C6
#define PIN_USB_D_PLUS 13
#endif
#ifdef ESP32C3
#define PIN_USB_D_PLUS 19
#endif
// ESP32-C2 doesn't seem to have USB pins
#endif



#endif // ifndef HELPERS_HARDWARE_DEFINES_H
