#ifndef ESPEASYSERIAL_WRAPPERS_ESPEASYSERIAL_SW_SERIAL_H
#define ESPEASYSERIAL_WRAPPERS_ESPEASYSERIAL_SW_SERIAL_H


#include "../ESPEasySerial_common_defines.h"


#if USES_SW_SERIAL

# include "../drivers/ESPEasySoftwareSerial.h"

# include "ESPEasySerial_Port_base.h"


class ESPEasySerial_SW_Serial : public ESPEasySerial_Port_base {
public:

  ESPEasySerial_SW_Serial(const ESPEasySerialConfig & config);

  virtual ~ESPEasySerial_SW_Serial();

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

  ESPeasySoftwareSerial *_swserial = nullptr;
};


#endif // if USES_SW_SERIAL


#endif // ifndef ESPEASYSERIAL_WRAPPERS_ESPEASYSERIAL_SW_SERIAL_H
