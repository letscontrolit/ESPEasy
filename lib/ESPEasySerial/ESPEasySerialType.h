#ifndef ESPEASY_SERIAL_ESPEASYSERIALTYPE_H
#define ESPEASY_SERIAL_ESPEASYSERIALTYPE_H


#include "ESPEasySerialPort.h"



struct ESPeasySerialType {
  static bool getSerialTypePins(ESPEasySerialPort serType,
                                int             & rxPin,
                                int             & txPin);

  static ESPEasySerialPort getSerialType(ESPEasySerialPort typeHint,
                                         int               receivePin,
                                         int               transmitPin);
};


#endif // ifndef ESPEASY_SERIAL_ESPEASYSERIALTYPE_H
