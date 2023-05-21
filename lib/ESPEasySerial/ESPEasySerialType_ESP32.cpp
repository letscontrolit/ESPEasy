#include "ESPEasySerialType.h"

#include "ESPEasySerial_common_defines.h"

#ifdef ESP32

#ifndef ESP32_SER0_TX
#if CONFIG_IDF_TARGET_ESP32C3
  # define ESP32_SER0_TX 21
#elif CONFIG_IDF_TARGET_ESP32S2
  # define ESP32_SER0_TX 43
#elif CONFIG_IDF_TARGET_ESP32S3
  # define ESP32_SER0_TX 43
#else
  # define ESP32_SER0_TX 1
#endif
#endif // ifndef ESP32_SER0_TX
#ifndef ESP32_SER0_RX
#if CONFIG_IDF_TARGET_ESP32C3
  # define ESP32_SER0_RX 20
#elif CONFIG_IDF_TARGET_ESP32S2
  # define ESP32_SER0_RX 44
#elif CONFIG_IDF_TARGET_ESP32S3
  # define ESP32_SER0_RX 44
#else
  # define ESP32_SER0_RX 3
#endif
#endif // ifndef ESP32_SER0_RX

#if SOC_UART_NUM > 1
#ifndef ESP32_SER1_TX
#if CONFIG_IDF_TARGET_ESP32C3
  # define ESP32_SER1_TX 21
#elif CONFIG_IDF_TARGET_ESP32S2
  # define ESP32_SER1_TX 18
#elif CONFIG_IDF_TARGET_ESP32S3
  # define ESP32_SER1_TX 17
#else
  # define ESP32_SER1_TX 15
#endif
#endif
#endif // ifndef ESP32_SER1_TX
#ifndef ESP32_SER1_RX
#if CONFIG_IDF_TARGET_ESP32C3
  # define ESP32_SER1_RX 20
#elif CONFIG_IDF_TARGET_ESP32S2
  # define ESP32_SER1_RX 18
#elif CONFIG_IDF_TARGET_ESP32S3
  # define ESP32_SER1_RX 18
#else
  # define ESP32_SER1_RX 13
#endif
#endif // ifndef ESP32_SER1_RX

#if SOC_UART_NUM > 2
#ifndef ESP32_SER2_TX
  # define ESP32_SER2_TX 17
#endif // ifndef ESP32_SER2_TX
#ifndef ESP32_SER2_RX
  # define ESP32_SER2_RX 16
#endif // ifndef ESP32_SER2_RX
#endif

bool ESPeasySerialType::getSerialTypePins(ESPEasySerialPort serType, int& rxPin, int& txPin) {
  rxPin = -1;
  txPin = -1;

  switch (serType) {
    case ESPEasySerialPort::serial0:  rxPin = ESP32_SER0_RX; txPin = ESP32_SER0_TX; return true;
# if SOC_UART_NUM > 1
    case ESPEasySerialPort::serial1:  rxPin = ESP32_SER1_RX; txPin = ESP32_SER1_TX; return true;
# endif
# if SOC_UART_NUM > 2
    case ESPEasySerialPort::serial2:  rxPin = ESP32_SER2_RX; txPin = ESP32_SER2_TX; return true;
# endif
#if USES_I2C_SC16IS752
    case ESPEasySerialPort::sc16is752:     return true;
#endif // ifndef DISABLE_SC16IS752_Serial

    default:
      break;
  }
  return false;
}


ESPEasySerialPort ESPeasySerialType::getSerialType(ESPEasySerialPort typeHint, int receivePin, int transmitPin) {
  if (typeHint != ESPEasySerialPort::not_set) {
    return typeHint;
  }

  if ((receivePin == ESP32_SER0_RX) && (transmitPin == ESP32_SER0_TX)) {
    return ESPEasySerialPort::serial0; // UART0
  }

  // Serial1 on ESP32 uses default pins connected to flash
  // So must make sure to set them to other pins.
# if SOC_UART_NUM > 1
  if ((receivePin == ESP32_SER1_RX) && (transmitPin == ESP32_SER1_TX)) {
    return ESPEasySerialPort::serial1; // UART1
  }
# endif
# if SOC_UART_NUM > 2

  if ((receivePin == ESP32_SER2_RX) && (transmitPin == ESP32_SER2_TX)) {
    return ESPEasySerialPort::serial2; // UART2
  }
# endif
#if USES_I2C_SC16IS752
  if ((receivePin >= 0x48) && (receivePin <= 0x57)) {
    return ESPEasySerialPort::sc16is752; // I2C address range of SC16IS752
  }
#endif

  return ESPEasySerialPort::MAX_SERIAL_TYPE;
}

#endif // ESP32
