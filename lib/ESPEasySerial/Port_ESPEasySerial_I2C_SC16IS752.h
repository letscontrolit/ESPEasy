#ifndef ESPEASYSERIAL_PORT_ESPEASYSERIAL_I2C_SC16IS752_H
#define ESPEASYSERIAL_PORT_ESPEASYSERIAL_I2C_SC16IS752_H


#include "ESPEasySerial_common_defines.h"

#if USES_I2C_SC16IS752
# include "Driver_ESPEasySC16IS752_Serial.h"


# include "Port_ESPEasySerial_base.h"


class Port_ESPEasySerial_I2C_SC16IS752_t : public Port_ESPEasySerial_base {
public:

  //       ESPEasySC16IS752_Serial::I2C_address addr     = static_cast<ESPEasySC16IS752_Serial::I2C_address>(receivePin);
  //       ESPEasySC16IS752_Serial::SC16IS752_channel ch = static_cast<ESPEasySC16IS752_Serial::SC16IS752_channel>(transmitPin);


  Port_ESPEasySerial_I2C_SC16IS752_t(const ESPEasySerialConfig& config);

  virtual ~Port_ESPEasySerial_I2C_SC16IS752_t();

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

  ESPEasySC16IS752_Serial *_i2cserial = nullptr;
};


#endif // if USES_I2C_SC16IS752


#endif // ifndef ESPEASYSERIAL_PORT_ESPEASYSERIAL_I2C_SC16IS752_H
