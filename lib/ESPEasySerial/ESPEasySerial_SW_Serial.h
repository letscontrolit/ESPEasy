#ifndef ESPEASYSERIAL_ESPEASYSERIAL_USBCDC_H
#define ESPEASYSERIAL_ESPEASYSERIAL_USBCDC_H

#if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(ESP32)
  # ifndef DISABLE_SOFTWARE_SERIAL
    #  define DISABLE_SOFTWARE_SERIAL
  # endif // ifndef DISABLE_SOFTWARE_SERIAL
#endif    // if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(ESP32)

#if !defined(DISABLE_SOFTWARE_SERIAL) && defined(ESP8266)
# include <ESPEasySoftwareSerial.h>


# include "ESPEasySerial_Port.h"

# include <Arduino.h>
# include <inttypes.h>
# include <Stream.h>


class ESPEasySerial_SW_Serial : public ESPEasySerial_Port {
public:

  ESPEasySerial_SW_Serial(int  receivePin,
                          int  transmitPin,
                          bool inverse_logic);

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


#endif // if !defined(DISABLE_SOFTWARE_SERIAL) && defined(ESP8266)


#endif // ifndef ESPEASYSERIAL_ESPEASYSERIAL_USBCDC_H
