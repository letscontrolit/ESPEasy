#ifndef ESPEASY_SERIAL_ESPEASYSERIALTYPE_H
#define ESPEASY_SERIAL_ESPEASYSERIALTYPE_H

#include "ESPEasySerialPort.h"

#ifdef ESP32

# ifndef SOC_UART_FIFO_LEN
#  define SOC_UART_FIFO_LEN (128)
# endif // ifndef SOC_UART_FIFO_LEN


// Default pins taken from here:
// https://github.com/espressif/arduino-esp32/blob/3ec5f4efa1de4342aaab742008dc630091e5e035/cores/esp32/HardwareSerial.cpp#L24-L92
// Changed RX1 and TX1 for ESP32-classic, as the defaults chosen by Espressif are flash pins.
// Changed RX1 and TX1 for ESP32-C3 to lower pin nrs to have proper options for small boards.

# ifndef SOC_RX0
#  if CONFIG_IDF_TARGET_ESP32
#   define SOC_RX0 3
#  elif CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#   define SOC_RX0 44
#  elif CONFIG_IDF_TARGET_ESP32C6
#   define SOC_RX0 17
#  elif CONFIG_IDF_TARGET_ESP32C3
#   define SOC_RX0 20
#  elif CONFIG_IDF_TARGET_ESP32C2
#   define SOC_RX0 19
#  endif // if CONFIG_IDF_TARGET_ESP32
# endif  // ifndef SOC_RX0

# ifndef SOC_TX0
#  if CONFIG_IDF_TARGET_ESP32
#   define SOC_TX0 1
#  elif CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#   define SOC_TX0 43
#  elif CONFIG_IDF_TARGET_ESP32C6
#   define SOC_TX0 16
#  elif CONFIG_IDF_TARGET_ESP32C3
#   define SOC_TX0 21
#  elif CONFIG_IDF_TARGET_ESP32C2
#   define SOC_TX0 20
#  endif // if CONFIG_IDF_TARGET_ESP32
# endif  // ifndef SOC_TX0

# if SOC_UART_NUM > 1

#  ifndef SOC_RX1
#   if CONFIG_IDF_TARGET_ESP32
#    define SOC_RX1 13
#   elif CONFIG_IDF_TARGET_ESP32S2
#    define SOC_RX1 18
#   elif CONFIG_IDF_TARGET_ESP32C2 || CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32C6
#    define SOC_RX1 4
#   elif CONFIG_IDF_TARGET_ESP32S3
#    define SOC_RX1 15
#   endif // if CONFIG_IDF_TARGET_ESP32
#  endif  // ifndef SOC_RX1

#  ifndef SOC_TX1
#   if CONFIG_IDF_TARGET_ESP32
#    define SOC_TX1 15
#   elif CONFIG_IDF_TARGET_ESP32S2
#    define SOC_TX1 17
#   elif CONFIG_IDF_TARGET_ESP32C2 || CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32C6
#    define SOC_TX1 5
#   elif CONFIG_IDF_TARGET_ESP32S3
#    define SOC_TX1 16
#   endif // if CONFIG_IDF_TARGET_ESP32
#  endif  // ifndef SOC_TX1

# endif   // if SOC_UART_NUM > 1

# if SOC_UART_NUM > 2

#  ifndef SOC_RX2
#   if CONFIG_IDF_TARGET_ESP32
#    define SOC_RX2 16
#   elif CONFIG_IDF_TARGET_ESP32S3
#    define SOC_RX2 19
#   elif CONFIG_IDF_TARGET_ESP32C6
#    define SOC_RX2 6
#   else // if CONFIG_IDF_TARGET_ESP32
#    define SOC_RX2 -1
#   endif // if CONFIG_IDF_TARGET_ESP32
#  endif  // ifndef SOC_RX2

#  ifndef SOC_TX2
#   if CONFIG_IDF_TARGET_ESP32
#    define SOC_TX2 17
#   elif CONFIG_IDF_TARGET_ESP32S3
#    define SOC_TX2 20
#   elif CONFIG_IDF_TARGET_ESP32C6
#    define SOC_TX2 7
#   else // if CONFIG_IDF_TARGET_ESP32
#    define SOC_TX2 -1
#   endif // if CONFIG_IDF_TARGET_ESP32
#  endif  // ifndef SOC_TX2

# endif   // if SOC_UART_NUM > 2

#endif // ifdef ESP32

struct ESPeasySerialType {
  static bool getSerialTypePins(ESPEasySerialPort serType,
                                int             & rxPin,
                                int             & txPin);

  static ESPEasySerialPort getSerialType(ESPEasySerialPort typeHint,
                                         int               receivePin,
                                         int               transmitPin);
};


#endif // ifndef ESPEASY_SERIAL_ESPEASYSERIALTYPE_H
