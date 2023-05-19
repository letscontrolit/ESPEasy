#ifndef ESPEASY_SERIAL_PORT_H
#define ESPEASY_SERIAL_PORT_H

#include "ESPEasySerialPort.h"

#include <Arduino.h>
#include <inttypes.h>
#include <Stream.h>

class ESPEasySerial_Port : public Stream {
public:

  ESPEasySerial_Port();
  virtual ~ESPEasySerial_Port() {}

  virtual void   begin(unsigned long baud) = 0;

  virtual void   end()                   = 0;
  virtual int    available(void)         = 0;
  virtual int    availableForWrite(void) = 0;

  // return -1 when data is unavailable (arduino api)
  virtual int    peek(void) = 0;
  virtual int    read(void) = 0;
  virtual size_t read(uint8_t *buffer,
                      size_t   size) = 0;

  inline size_t  read(char *buffer, size_t size)
  {
    return read((uint8_t *)buffer, size);
  }

  virtual void   flush(void)        = 0;
  virtual void   flush(bool txOnly) = 0;
  virtual size_t write(uint8_t)     = 0;
  virtual size_t write(const uint8_t *buffer,
                       size_t         size) = 0;


  inline size_t write(const char *buffer, size_t size)
  {
    return write((uint8_t *)buffer, size);
  }

  inline size_t write(const char *s)
  {
    return write((uint8_t *)s, strlen(s));
  }

  inline size_t write(unsigned long n)
  {
    return write((uint8_t)n);
  }

  inline size_t write(long n)
  {
    return write((uint8_t)n);
  }

  inline size_t write(unsigned int n)
  {
    return write((uint8_t)n);
  }

  inline size_t write(int n)
  {
    return write((uint8_t)n);
  }

  virtual operator bool() const = 0;

  virtual void   setDebugOutput(bool) = 0;

  virtual size_t setRxBufferSize(size_t new_size) = 0;
  virtual size_t setTxBufferSize(size_t new_size) = 0;

protected:

  ESPEasySerialPort _port = ESPEasySerialPort::not_set;

  unsigned long _baud = 115200;
};


#endif // ifndef ESPEASY_SERIAL_PORT_H
