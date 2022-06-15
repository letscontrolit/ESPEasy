#include <ESPeasySerial.h>


String ESPeasySerial::getLogString() const {
  String log;

  log.reserve(48);
  log = F("ESPEasy serial: ");

  if (isI2Cserial()) {
    log += F("I2C: addr:");
    log += String(_receivePin);
    log += F(" ch:");
    log += _transmitPin == 0 ? 'A' : 'B';
  } else {
    if (isSWserial()) {
      log += F("SW");
    } else {
      log += F("HW");
    }
    log += F(": rx:");
    log += String(_receivePin);
    log += F(" tx:");
    log += String(_transmitPin);
  }
  log += F(" baud:");
  log += String(_baud);
  return log;
}
