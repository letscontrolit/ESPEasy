#ifndef ESPEASYSERIAL_PORT_ESPEASYSERIAL_USB_HWCDC_H
#define ESPEASYSERIAL_PORT_ESPEASYSERIAL_USB_HWCDC_H

#include "ESPEasySerial_common_defines.h"

#if USES_HWCDC

// ESP32C3/S3 embedded USB using JTAG interface
# include <HWCDC.h>
# include "Port_ESPEasySerial_base.h"


class Port_ESPEasySerial_USB_HWCDC_t : public Port_ESPEasySerial_base {
public:

  explicit Port_ESPEasySerial_USB_HWCDC_t(const ESPEasySerialConfig& config);

  virtual ~Port_ESPEasySerial_USB_HWCDC_t();

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

  bool setRS485Mode(int8_t rtsPin, bool enableCollisionDetection = false);

private:

  HWCDC *_hwcdc_serial= nullptr;
};


#endif // if USES_HWCDC


#endif // ifndef ESPEASYSERIAL_PORT_ESPEASYSERIAL_USB_HWCDC_H
