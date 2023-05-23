#include "ESPEasySerialType.h"

#include "ESPEasySerial_common_defines.h"

#ifdef ESP32

bool ESPeasySerialType::getSerialTypePins(ESPEasySerialPort serType, int& rxPin, int& txPin) {
  rxPin = -1;
  txPin = -1;

  switch (serType) {
    case ESPEasySerialPort::serial0:  rxPin = SOC_RX0; txPin = SOC_TX0; return true;
# if SOC_UART_NUM > 1
    case ESPEasySerialPort::serial1:  rxPin = SOC_RX1; txPin = SOC_TX1; return true;
# endif // if SOC_UART_NUM > 1
# if SOC_UART_NUM > 2
    case ESPEasySerialPort::serial2:  rxPin = SOC_RX2; txPin = SOC_TX2; return true;
# endif // if SOC_UART_NUM > 2
# if USES_I2C_SC16IS752
    case ESPEasySerialPort::sc16is752:     return true;
# endif // if USES_I2C_SC16IS752

    default:
      break;
  }
  return false;
}

ESPEasySerialPort ESPeasySerialType::getSerialType(ESPEasySerialPort typeHint, int receivePin, int transmitPin) {
  if (typeHint != ESPEasySerialPort::not_set) {
    return typeHint;
  }

  if ((receivePin == SOC_RX0) && (transmitPin == SOC_TX0)) {
    return ESPEasySerialPort::serial0; // UART0
  }

  // Serial1 on ESP32 uses default pins connected to flash
  // So must make sure to set them to other pins.
# if SOC_UART_NUM > 1

  if ((receivePin == SOC_RX1) && (transmitPin == SOC_TX1)) {
    return ESPEasySerialPort::serial1; // UART1
  }
# endif // if SOC_UART_NUM > 1
# if SOC_UART_NUM > 2

  if ((receivePin == SOC_RX2) && (transmitPin == SOC_TX2)) {
    return ESPEasySerialPort::serial2; // UART2
  }
# endif // if SOC_UART_NUM > 2
# if USES_I2C_SC16IS752

  if ((receivePin >= 0x48) && (receivePin <= 0x57)) {
    return ESPEasySerialPort::sc16is752; // I2C address range of SC16IS752
  }
# endif // if USES_I2C_SC16IS752

  return ESPEasySerialPort::MAX_SERIAL_TYPE;
}

#endif // ESP32
