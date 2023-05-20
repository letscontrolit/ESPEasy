#ifndef ESPEASYSERIAL_WRAPPERS_ESPEASYSERIAL_USB_HWCDC_H
#define ESPEASYSERIAL_WRAPPERS_ESPEASYSERIAL_USB_HWCDC_H

#include "../ESPEasySerial_common_defines.h"

#if USES_HWCDC

// ESP32C3/S3 embedded USB using JTAG interface
# include <HWCDC.h>
# include "ESPEasySerial_Port_base.h"


class ESPEasySerial_USB_WHCDC_t : public ESPEasySerial_Port_base {
public:

  ESPEasySerial_USB_WHCDC_t();

  virtual ~ESPEasySerial_USB_WHCDC_t() {}

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
};


#endif // if USES_HWCDC


#endif // ifndef ESPEASYSERIAL_WRAPPERS_ESPEASYSERIAL_USB_HWCDC_H
