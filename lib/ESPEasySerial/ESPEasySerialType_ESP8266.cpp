#include "ESPEasySerialType.h"

#include "ESPEasySerial_common_defines.h"


#ifdef ESP8266


bool ESPeasySerialType::getSerialTypePins(ESPEasySerialPort serType, int& rxPin, int& txPin) {
  rxPin = -1;
  txPin = -1;

  switch (serType) {
    case ESPEasySerialPort::serial0:       rxPin = 3; txPin = 1; return true;
    case ESPEasySerialPort::serial0_swap:  rxPin = 13; txPin = 15; return true;
    case ESPEasySerialPort::serial1:       rxPin = -1; txPin = 2; return true;
# if USES_SW_SERIAL
    case ESPEasySerialPort::software:      rxPin = 14; txPin = 12; return true;
# endif // if USES_SW_SERIAL
# if USES_I2C_SC16IS752
    case ESPEasySerialPort::sc16is752:     return true;
# endif // ifndef DISABLE_SC16IS752_Serial

    default:
      break;
  }
  return false;
}

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
# if USES_I2C_SC16IS752

  if ((receivePin >= 0x48) && (receivePin <= 0x57)) {
    return ESPEasySerialPort::sc16is752; // I2C address range of SC16IS752
  }
# endif // if USES_I2C_SC16IS752

  if ((receivePin == -1) && (transmitPin == -1)) {
    // No pins set, so no serial type
    return ESPEasySerialPort::MAX_SERIAL_TYPE;
  }
  # if USES_SW_SERIAL
  return ESPEasySerialPort::software;
  # else // if USES_SW_SERIAL
  return ESPEasySerialPort::MAX_SERIAL_TYPE;
  # endif // if USES_SW_SERIAL
}

#endif // ESP8266
