#ifndef ESPEASYSERIAL_ESPEASYSERIAL_HARDWARESERIAL_H
#define ESPEASYSERIAL_ESPEASYSERIAL_HARDWARESERIAL_H

#include "ESPEasySerial_common_defines.h"

#include "Port_ESPEasySerial_base.h"

#include <HardwareSerial.h>

#ifdef ESP32
# include <esp32-hal-uart.h>
#endif // ifdef ESP32

class Port_ESPEasySerial_HardwareSerial_t : public Port_ESPEasySerial_base {
public:

  Port_ESPEasySerial_HardwareSerial_t();

  ~Port_ESPEasySerial_HardwareSerial_t();


  // Allow for resetConfig, instead of end() and begin().
  // This can otherwise cause issues with some sensors when starting/stopping an ESPEasy task
  void   resetConfig(const ESPEasySerialConfig& config);

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

  int    getBaudRate() const override;
  operator bool() const;

  void   setDebugOutput(bool);

  size_t setRxBufferSize(size_t new_size);
  size_t setTxBufferSize(size_t new_size);

private:

  HardwareSerial *_serial = nullptr;
};


#endif // ifndef ESPEASYSERIAL_ESPEASYSERIAL_HARDWARESERIAL_H
