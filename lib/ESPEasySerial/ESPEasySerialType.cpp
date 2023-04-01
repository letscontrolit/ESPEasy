#include "ESPEasySerialType.h"

#ifndef ESP32_SER0_TX
  # define ESP32_SER0_TX 1
#endif // ifndef ESP32_SER0_TX
#ifndef ESP32_SER0_RX
  # define ESP32_SER0_RX 3
#endif // ifndef ESP32_SER0_RX

#ifndef ESP32_SER1_TX
  # define ESP32_SER1_TX 15
#endif // ifndef ESP32_SER1_TX
#ifndef ESP32_SER1_RX
  # define ESP32_SER1_RX 13
#endif // ifndef ESP32_SER1_RX

#ifndef ESP32_SER2_TX
  # define ESP32_SER2_TX 17
#endif // ifndef ESP32_SER2_TX
#ifndef ESP32_SER2_RX
  # define ESP32_SER2_RX 16
#endif // ifndef ESP32_SER2_RX



bool ESPeasySerialType::getSerialTypePins(ESPEasySerialPort serType, int& rxPin, int& txPin) {
  rxPin = -1;
  txPin = -1;

  switch (serType) {
#ifdef ESP32
    case ESPEasySerialPort::serial0:  rxPin = ESP32_SER0_RX; txPin = ESP32_SER0_TX; return true;
    case ESPEasySerialPort::serial1:  rxPin = ESP32_SER1_RX; txPin = ESP32_SER1_TX; return true;
    case ESPEasySerialPort::serial2:  rxPin = ESP32_SER2_RX; txPin = ESP32_SER2_TX; return true;
#endif // ifdef ESP32
#ifdef ESP8266
    case ESPEasySerialPort::serial0:       rxPin = 3; txPin = 1; return true;
    case ESPEasySerialPort::serial0_swap:  rxPin = 13; txPin = 15; return true;
    case ESPEasySerialPort::serial1:       rxPin = -1; txPin = 2; return true;
    # ifndef DISABLE_SOFTWARE_SERIAL
    case ESPEasySerialPort::software:      rxPin = 14; txPin = 12; return true;
    # endif // DISABLE_SOFTWARE_SERIAL
#endif      // ifdef ESP8266
#ifndef DISABLE_SC16IS752_Serial
    case ESPEasySerialPort::sc16is752:     return true;
#endif // ifndef DISABLE_SC16IS752_Serial

    default:
      break;
  }
  return false;
}

#ifdef ESP32
ESPEasySerialPort ESPeasySerialType::getSerialType(ESPEasySerialPort typeHint, int receivePin, int transmitPin) {
  if (typeHint != ESPEasySerialPort::not_set) {
    return typeHint;
  }

  if ((receivePin == ESP32_SER0_RX) && (transmitPin == ESP32_SER0_TX)) {
    return ESPEasySerialPort::serial0; // UART0
  }

  // Serial1 on ESP32 uses default pins connected to flash
  // So must make sure to set them to other pins.
  if ((receivePin == ESP32_SER1_RX) && (transmitPin == ESP32_SER1_TX)) {
    return ESPEasySerialPort::serial1; // UART1
  }

  if ((receivePin == ESP32_SER2_RX) && (transmitPin == ESP32_SER2_TX)) {
    return ESPEasySerialPort::serial2; // UART2
  }

  if ((receivePin >= 0x48) && (receivePin <= 0x57)) {
    return ESPEasySerialPort::sc16is752; // I2C address range of SC16IS752
  }

  return ESPEasySerialPort::MAX_SERIAL_TYPE;
}

#endif // ESP32


#ifdef ESP8266
ESPEasySerialPort ESPeasySerialType::getSerialType(ESPEasySerialPort typeHint, int receivePin, int transmitPin) {
  if (typeHint != ESPEasySerialPort::not_set) {
    return typeHint;
  }

  if ((receivePin == 3) && (transmitPin == 1)) {
    return ESPEasySerialPort::serial0; // UART0
  }

  // ESP8266
  if ((receivePin == 13) && (transmitPin == 15)) {
    return ESPEasySerialPort::serial0_swap; // UART0 remapped using Serial.swap()
  }

  if ((receivePin == -1) && (transmitPin == 2)) {
    // Serial1 uses UART1, TX pin is GPIO2.
    // UART1 can not be used to receive data because normally
    // it's RX pin is occupied for flash chip connection.
    return ESPEasySerialPort::serial1;
  }

  if ((receivePin >= 0x48) && (receivePin <= 0x57)) {
    return ESPEasySerialPort::sc16is752; // I2C address range of SC16IS752
  }

  if ((receivePin == -1) && (transmitPin == -1)) {
    // No pins set, so no serial type
    return ESPEasySerialPort::MAX_SERIAL_TYPE;
  }
  return ESPEasySerialPort::software;
}

#endif // ESP8266
