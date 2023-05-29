#ifndef ESPEASYSERIAL_PORT_ESPEASYSERIAL_SW_SERIAL_H
#define ESPEASYSERIAL_PORT_ESPEASYSERIAL_SW_SERIAL_H


#include "ESPEasySerial_common_defines.h"


#if USES_SW_SERIAL

# include "Port_ESPEasySerial_base.h"

#if USES_LATEST_SOFTWARE_SERIAL_LIBRARY
# include <SoftwareSerial.h>
#else 
# include "Driver_ESPEasySoftwareSerial.h"
#endif

class Port_ESPEasySerial_SW_Serial_t : public Port_ESPEasySerial_base {
public:

  Port_ESPEasySerial_SW_Serial_t(const ESPEasySerialConfig& config);

  virtual ~Port_ESPEasySerial_SW_Serial_t();

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

#if USES_LATEST_SOFTWARE_SERIAL_LIBRARY
  SoftwareSerial *_swserial = nullptr;
#else
  Driver_ESPEasySoftwareSerial_t *_swserial = nullptr;
#endif

};


#endif // if USES_SW_SERIAL


#endif // ifndef ESPEASYSERIAL_PORT_ESPEASYSERIAL_SW_SERIAL_H
