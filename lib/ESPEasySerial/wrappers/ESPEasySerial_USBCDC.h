#ifndef ESPEASYSERIAL_WRAPPERS_ESPEASYSERIAL_USBCDC_H
#define ESPEASYSERIAL_WRAPPERS_ESPEASYSERIAL_USBCDC_H

#include "../ESPEasySerial_common_defines.h"

#if USES_USBCDC

// ESP32-S2 USB CDC interface
# include <USB.h>
# include <USBCDC.h>

# include "ESPEasySerial_Port_base.h"

class ESPEasySerial_USBCDC_t : public ESPEasySerial_Port_base {
public:

  ESPEasySerial_USBCDC_t(ESPEasySerialPort port);

  virtual ~ESPEasySerial_USBCDC_t();

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

  USBCDC *_serial  = nullptr;
  bool _mustDelete = false;
};


#endif // if USES_USBCDC


#endif // ifndef ESPEASYSERIAL_WRAPPERS_ESPEASYSERIAL_USBCDC_H
