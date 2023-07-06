#ifndef ESPEASYSERIAL_PORT_ESPEASYSERIAL_USBCDC_H
#define ESPEASYSERIAL_PORT_ESPEASYSERIAL_USBCDC_H

#include "ESPEasySerial_common_defines.h"

#if USES_USBCDC

// ESP32-S2 USB CDC interface
# include <USB.h>
# include <USBCDC.h>

# include "Port_ESPEasySerial_base.h"

class Port_ESPEasySerial_USBCDC_t : public Port_ESPEasySerial_base {
public:

  Port_ESPEasySerial_USBCDC_t(const ESPEasySerialConfig& config);

  virtual ~Port_ESPEasySerial_USBCDC_t();

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

  void        setDebugOutput(bool);

  size_t      setRxBufferSize(size_t new_size);
  size_t      setTxBufferSize(size_t new_size);

  virtual int getBaudRate() const override;

private:

# if ARDUINO_USB_CDC_ON_BOOT
  USBCDC *_serial = &Serial;
# else // if ARDUINO_USB_CDC_ON_BOOT
  USBCDC *_serial = nullptr;
# endif // if ARDUINO_USB_CDC_ON_BOOT
};


// Need to define these objects as extern as they need to be defined before setup() is being called.
# if ARDUINO_USB_CDC_ON_BOOT
# else
extern USBCDC ESPEasySerial_USBCDC_port0;

// extern USBCDC ESPEasySerial_USBCDC_port1;
# endif // if ARDUINO_USB_CDC_ON_BOOT


#endif // if USES_USBCDC


#endif // ifndef ESPEASYSERIAL_PORT_ESPEASYSERIAL_USBCDC_H
