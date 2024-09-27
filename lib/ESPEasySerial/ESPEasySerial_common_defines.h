#ifndef ESPEASYSERIAL_ESPEASYSERIAL_COMMON_DEFINES_H
#define ESPEASYSERIAL_ESPEASYSERIAL_COMMON_DEFINES_H

#include <Arduino.h>
#include <inttypes.h>


#ifndef SOC_UART_NUM
# ifdef ESP8266
#  define SOC_UART_NUM 2
# elif defined(ESP32_CLASSIC) || defined(ESP32S2) || defined(ESP32S3) || defined(ESP32C2) || defined(ESP32C3) || defined(ESP32C6)
#  include <soc/soc_caps.h>
# else // ifdef ESP8266
static_assert(false, "Implement processor architecture");
# endif // ifdef ESP8266
#endif // ifndef SOC_UART_NUM

#ifndef USABLE_SOC_UART_NUM
# ifdef SOC_UART_HP_NUM

// In ESP-IDF 5.3 the actual difference in high-power and low-power UART ports was defined.
#  define USABLE_SOC_UART_NUM SOC_UART_HP_NUM
# else // ifdef SOC_UART_HP_NUM
#  ifdef ESP32C6

// ESP32-C6 has 3 UARTs (2 HP UART, and 1 LP UART)
// We can only use the high-power ones
#   define USABLE_SOC_UART_NUM 2
#  else // ifdef ESP32C6
#   define USABLE_SOC_UART_NUM SOC_UART_NUM
#  endif // ifdef ESP32C6
# endif // ifdef SOC_UART_HP_NUM
#endif // ifndef USABLE_SOC_UART_NUM

#ifdef ESP32

/*
 #if CONFIG_IDF_TARGET_ESP32C6 ||  // support USB via HWCDC using JTAG interface
     CONFIG_IDF_TARGET_ESP32C3 ||  // support USB via HWCDC using JTAG interface
     CONFIG_IDF_TARGET_ESP32S2 ||  // support USB via USBCDC
     CONFIG_IDF_TARGET_ESP32S3     // support USB via HWCDC using JTAG interface or USBCDC
 */
# if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32C6 || CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3

// #if CONFIG_TINYUSB_CDC_ENABLED              // This define is not recognized here so use USE_USB_CDC_CONSOLE
#  ifdef USE_USB_CDC_CONSOLE
#   if ARDUINO_USB_MODE

// ESP32C3/S3 embedded USB using JTAG interface
#    ifndef USES_HWCDC
#     define USES_HWCDC 1
#    endif // ifndef USES_HWCDC
#   else // No ARDUINO_USB_MODE
#    ifndef USES_USBCDC
#     define USES_USBCDC 1
#    endif // ifndef USES_USBCDC
#   endif  // if ARDUINO_USB_MODE
#  endif   // ifdef USE_USB_CDC_CONSOLE
# endif   // if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#endif    // ifdef ESP32


#ifndef ESP32
# if defined(ARDUINO_ESP8266_RELEASE_2_4_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_1)  || defined(ARDUINO_ESP8266_RELEASE_2_4_2)
#  ifndef CORE_2_4_X
#   define CORE_2_4_X
#  endif // ifndef CORE_2_4_X
# endif  // if defined(ARDUINO_ESP8266_RELEASE_2_4_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_1)  || defined(ARDUINO_ESP8266_RELEASE_2_4_2)

# if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_1)
#  ifndef CORE_PRE_2_4_2
#   define CORE_PRE_2_4_2
#  endif // ifndef CORE_PRE_2_4_2
# endif  // if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_1)

# if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(CORE_2_4_X)
#  ifndef CORE_PRE_2_5_0
#   define CORE_PRE_2_5_0
#  endif // ifndef CORE_PRE_2_5_0
# else // if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(CORE_2_4_X)
#  ifndef CORE_POST_2_5_0
#   define CORE_POST_2_5_0
#  endif // ifndef CORE_POST_2_5_0
# endif  // if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(CORE_2_4_X)
#endif   // ifndef ESP32


#if defined(ARDUINO_ESP8266_RELEASE_2_3_0)
# ifndef DISABLE_SOFTWARE_SERIAL
#  define DISABLE_SOFTWARE_SERIAL
# endif // ifndef DISABLE_SOFTWARE_SERIAL
#endif  // if defined(ARDUINO_ESP8266_RELEASE_2_3_0)

#ifndef USES_HWCDC
# define USES_HWCDC 0
#endif // ifndef USES_HWCDC

#ifndef USES_USBCDC
# define USES_USBCDC 0
#endif // ifndef USES_USBCDC


#ifndef USES_SW_SERIAL
# ifndef DISABLE_SOFTWARE_SERIAL
#  define USES_SW_SERIAL 1
#  ifndef USES_LATEST_SOFTWARE_SERIAL_LIBRARY
#   ifdef ESP32
#    define USES_LATEST_SOFTWARE_SERIAL_LIBRARY 1
#   elif defined(ESP8266)
#    define USES_LATEST_SOFTWARE_SERIAL_LIBRARY 0
#   else // ifdef ESP32
#    define USES_LATEST_SOFTWARE_SERIAL_LIBRARY 1
#   endif // ifdef ESP32
#  endif // ifndef USES_LATEST_SOFTWARE_SERIAL_LIBRARY
# else // ifndef DISABLE_SOFTWARE_SERIAL
#  define USES_SW_SERIAL 0
# endif // ifndef DISABLE_SOFTWARE_SERIAL
#endif // ifndef USES_SW_SERIAL

#ifndef USES_I2C_SC16IS752
# ifndef DISABLE_SC16IS752_Serial
#  define USES_I2C_SC16IS752 1
# else // ifndef DISABLE_SC16IS752_Serial
#  define USES_I2C_SC16IS752 0
# endif // ifndef DISABLE_SC16IS752_Serial
#endif // ifndef USES_I2C_SC16IS752

#endif // ifndef ESPEASYSERIAL_ESPEASYSERIAL_COMMON_DEFINES_H
