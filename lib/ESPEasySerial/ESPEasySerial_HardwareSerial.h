#ifndef ESPEASYSERIAL_ESPEASYSERIAL_HARDWARESERIAL_H
#define ESPEASYSERIAL_ESPEASYSERIAL_HARDWARESERIAL_H

#include "ESPEasySerial_Port.h"

#include <Arduino.h>
#include <inttypes.h>
#include <Stream.h>
#include <HardwareSerial.h>

#ifdef ESP32
# include <esp32-hal-uart.h>
#endif // ifdef ESP32

class ESPEasySerial_HardwareSerial_t : public ESPEasySerial_Port {
public:

  ESPEasySerial_HardwareSerial_t(ESPEasySerialPort port);

  virtual ~ESPEasySerial_HardwareSerial_t() {}


  // Allow for resetConfig, instead of end() and begin().
  // This can otherwise cause issues with some sensors when starting/stopping an ESPEasy task
  void resetConfig(
    ESPEasySerialPort port,
    int               receivePin,
    int               transmitPin,
    bool              inverse_logic = false,
    unsigned int      buffSize      = 64);

#ifdef ESP8266
  void setSerialConfig(SerialConfig config = SERIAL_8N1,
                       SerialMode   mode   = SERIAL_FULL);
#endif // ifdef ESP8266

#ifdef ESP32
  void setSerialConfig(uint32_t config = SERIAL_8N1);

#endif // ifdef ESP32

  void   begin(unsigned long baud);

  void   end();
  int    available(void);
  int    availableForWrite(void);
  int    peek(void);
  int    read(void);
  size_t read(uint8_t *buffer,
              size_t   size);

  void   flush(void);
  void   flush(bool txOnly);
  size_t write(uint8_t);
  size_t write(const uint8_t *buffer,
               size_t         size);

  operator bool() const;

  void   setDebugOutput(bool);

  size_t setRxBufferSize(size_t new_size);
  size_t setTxBufferSize(size_t new_size);

private:

  HardwareSerial *_serial = nullptr;

#ifdef ESP32
  uint32_t _config = SERIAL_8N1;
#endif // ifdef ESP32

#ifdef ESP8266
  SerialConfig _config = SERIAL_8N1;
  SerialMode _mode     = SERIAL_FULL;
#endif // ifdef ESP8266

  uint32_t _buffSize = 128;
  int8_t _rxPin      = -1;
  int8_t _txPin      = -1;
  bool _invert       = false;
};


#endif // ifndef ESPEASYSERIAL_ESPEASYSERIAL_HARDWARESERIAL_H
