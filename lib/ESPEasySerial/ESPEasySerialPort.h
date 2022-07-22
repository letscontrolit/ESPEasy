#ifndef ESPEASY_SERIAL_ESPEASYSERIALPORT_H
#define ESPEASY_SERIAL_ESPEASYSERIALPORT_H

#include <Arduino.h>

// Keep value assigned as it is used in scripts and stored in the Settings.TaskDevicePort
enum class ESPEasySerialPort {
  not_set      = 0,
  sc16is752    = 1,
  serial0      = 2,
  serial0_swap = 3,
  serial1      = 4,
  serial2      = 5,
  software     = 6,

  MAX_SERIAL_TYPE
};

const __FlashStringHelper* ESPEasySerialPort_toString(ESPEasySerialPort serType);

#endif // ifndef ESPEASY_SERIAL_ESPEASYSERIALPORT_H
