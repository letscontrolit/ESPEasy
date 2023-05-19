#ifndef ESPEASYSERIAL_ESPEASYSERIAL_USBCDC_H
#define ESPEASYSERIAL_ESPEASYSERIAL_USBCDC_H

// FIXME TD-er: Share this code to define USES_xxCDC
#ifdef ESP32
# if CONFIG_IDF_TARGET_ESP32S2 // support USB via USBCDC
// #if CONFIG_TINYUSB_CDC_ENABLED // This define is not recognized here so use USE_USB_CDC_CONSOLE
#  ifdef USE_USB_CDC_CONSOLE
#   if !ARDUINO_USB_MODE
#    define USES_USBCDC 1
#   endif // !ARDUINO_USB_MODE
#  endif  // ifdef USE_USB_CDC_CONSOLE
# endif   // if CONFIG_IDF_TARGET_ESP32S2
#endif    // ifdef ESP32

#ifndef USES_USBCDC
# define USES_USBCDC 0
#endif // ifndef USES_USBCDC


#if USES_USBCDC

// ESP32-S2 USB CDC interface
# include "USB.h"
# include "USBCDC.h"

# include <Arduino.h>
# include "ESPEasySerial_Port.h"
# include <inttypes.h>
# include <Stream.h>

class ESPEasySerial_USBCDC_t : public ESPEasySerial_Port {
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


#endif // ifndef ESPEASYSERIAL_ESPEASYSERIAL_USBCDC_H
