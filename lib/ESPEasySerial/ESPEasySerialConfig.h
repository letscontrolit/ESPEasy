#ifndef ESPEASYSERIAL_ESPEASYSERIALCONFIG_H
#define ESPEASYSERIAL_ESPEASYSERIALCONFIG_H

#include "ESPEasySerial_common_defines.h"

#include "ESPEasySerialPort.h"

#if USES_I2C_SC16IS752
# include "drivers/ESPEasySC16IS752_Serial.h"
#endif // if USES_I2C_SC16IS752

#include <HardwareSerial.h>

struct ESPEasySerialConfig {

  ESPEasySerialConfig() = default;

  void validate();

  #ifdef ESP8266
  void setPortConfig(unsigned long baud,
             SerialConfig  config,
             SerialMode    mode);
#endif

#ifdef ESP32
  void setPortConfig(unsigned long baud, uint32_t config);
#endif


  String getLogString() const;

#if USES_I2C_SC16IS752

  bool getI2C_SC16IS752_Parameters(ESPEasySC16IS752_Serial::I2C_address      & addr,
                                   ESPEasySC16IS752_Serial::SC16IS752_channel& ch) const;


#endif // if USES_I2C_SC16IS752

  int getReceivePin() const;
  int getTransmitPin() const;

  ESPEasySerialPort port          = ESPEasySerialPort::not_set;
  unsigned long     baud          = 115200;
  int               receivePin    = -1;
  int               transmitPin   = -1;
  bool              inverse_logic = false;
  unsigned int      buffSize      = 64;
  bool              forceSWserial = false;
  unsigned long     timeout_ms    = 20000UL;

#ifdef ESP32
  uint32_t config = SERIAL_8N1;
#endif // ifdef ESP32

#ifdef ESP8266
  SerialConfig config = SERIAL_8N1;
  SerialMode mode     = SERIAL_FULL;
#endif // ifdef ESP8266

};


#endif // ifndef ESPEASYSERIAL_ESPEASYSERIALCONFIG_H
