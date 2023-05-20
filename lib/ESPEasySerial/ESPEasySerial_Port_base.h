#ifndef ESPEASYSERIAL_WRAPPERS_ESPEASYSERIAL_PORT_BASE_H
#define ESPEASYSERIAL_WRAPPERS_ESPEASYSERIAL_PORT_BASE_H

#include "ESPEasySerialPort.h"

#include "ESPEasySerial_common_defines.h"
#include "ESPEasySerialConfig.h"

#include <Stream.h>

class ESPEasySerial_Port_base {
public:

  ESPEasySerial_Port_base();
  virtual ~ESPEasySerial_Port_base();

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


  unsigned long  getBaudRate() const {
    return _config.baud;
  }

  const ESPEasySerialConfig& getSerialConfig() const {
    return _config;
  }

#ifdef ESP8266
  void setPortConfig(unsigned long baud,
             SerialConfig  config,
             SerialMode    mode)
  {
    _config.setPortConfig(baud, config, mode);
  }
#endif

#ifdef ESP32
  void setPortConfig(unsigned long baud, uint32_t config)
  {
    _config.setPortConfig(baud, config);
  }

#endif


protected:

  ESPEasySerialConfig _config;
};


#endif // ifndef ESPEASYSERIAL_WRAPPERS_ESPEASYSERIAL_PORT_BASE_H
