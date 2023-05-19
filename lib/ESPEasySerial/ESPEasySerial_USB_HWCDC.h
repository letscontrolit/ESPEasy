#ifndef ESPEASYSERIAL_ESPEASYSERIAL_USBCDC_H
#define ESPEASYSERIAL_ESPEASYSERIAL_USBCDC_H


// FIXME TD-er: Share this code to define USES_xxCDC

#ifdef ESP32

/*
 #if CONFIG_IDF_TARGET_ESP32C3 ||  // support USB via HWCDC using JTAG interface
     CONFIG_IDF_TARGET_ESP32S3     // support USB via HWCDC using JTAG interface or USBCDC
 */
# if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3

// #if CONFIG_TINYUSB_CDC_ENABLED              // This define is not recognized here so use USE_USB_CDC_CONSOLE
#  ifdef USE_USB_CDC_CONSOLE
#   if ARDUINO_USB_MODE

// ESP32C3/S3 embedded USB using JTAG interface
#    define USES_HWCDC 1
#   endif // ARDUINO_USB_MODE
#  endif  // ifdef USE_USB_CDC_CONSOLE
# endif   // if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#endif    // ifdef ESP32

#ifndef USES_HWCDC
# define USES_HWCDC 0
#endif // ifndef USES_HWCDC

#if USES_HWCDC

// ESP32C3/S3 embedded USB using JTAG interface
# include "HWCDC.h"
# include "ESPEasySerial_Port.h"

# include <Arduino.h>
# include <inttypes.h>
# include <Stream.h>


class ESPEasySerial_USB_WHCDC_t : public ESPEasySerial_Port {
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


#endif // ifndef ESPEASYSERIAL_ESPEASYSERIAL_USBCDC_H
