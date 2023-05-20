#include "ESPEasySerial_I2C_SC16IS752.h"


#if USES_I2C_SC16IS752


ESPEasySerial_I2C_SC16IS752::ESPEasySerial_I2C_SC16IS752(const ESPEasySerialConfig & config)
{
  if (config.port ==  ESPEasySerialPort::sc16is752) {
    ESPEasySC16IS752_Serial::I2C_address       addr;
    ESPEasySC16IS752_Serial::SC16IS752_channel ch;
    if (config.getI2C_SC16IS752_Parameters(addr, ch)) {
      _config    = config;
      _i2cserial = new ESPEasySC16IS752_Serial(addr, ch);
    }  
  }
}

ESPEasySerial_I2C_SC16IS752::~ESPEasySerial_I2C_SC16IS752()
{
  if (_i2cserial != nullptr) {
    delete _i2cserial;
    _i2cserial = nullptr;
  }
}

void ESPEasySerial_I2C_SC16IS752::begin(unsigned long baud)
{
  if (_i2cserial != nullptr) {
    _i2cserial->begin(baud);
  }
}

void ESPEasySerial_I2C_SC16IS752::end() {
  if (_i2cserial != nullptr) {
    _i2cserial->end();
  }
}

int ESPEasySerial_I2C_SC16IS752::available(void)
{
  if (_i2cserial != nullptr) {
    return _i2cserial->available();
  }
  return 0;
}

int ESPEasySerial_I2C_SC16IS752::availableForWrite(void)
{
  if (_i2cserial != nullptr) {
    // FIXME TD-er: Implement availableForWrite
    return 4; // _i2cserial->availableForWrite();
  }
  return 0;
}

int ESPEasySerial_I2C_SC16IS752::peek(void)
{
  if (_i2cserial != nullptr) {
    return _i2cserial->peek();
  }
  return 0;
}

int ESPEasySerial_I2C_SC16IS752::read(void)
{
  if (_i2cserial != nullptr) {
    return _i2cserial->read();
  }
  return 0;
}

size_t ESPEasySerial_I2C_SC16IS752::read(uint8_t *buffer,
                                         size_t   size)
{
  if (_i2cserial != nullptr) {
    return _i2cserial->readBytes((char *)buffer, size);
  }
  return 0;
}

void ESPEasySerial_I2C_SC16IS752::flush(void)
{
  if (_i2cserial != nullptr) {
    return _i2cserial->flush();
  }
}

void ESPEasySerial_I2C_SC16IS752::flush(bool txOnly)
{
  flush();
}

size_t ESPEasySerial_I2C_SC16IS752::write(uint8_t value)
{
  if (_i2cserial != nullptr) {
    return _i2cserial->write(value);
  }
  return 0;
}

size_t ESPEasySerial_I2C_SC16IS752::write(const uint8_t *buffer,
                                          size_t         size)
{
  if (_i2cserial != nullptr) {
    return _i2cserial->write(buffer, size);
  }
  return 0;
}

ESPEasySerial_I2C_SC16IS752::operator bool() const
{
  return _i2cserial != nullptr;
}

void   ESPEasySerial_I2C_SC16IS752::setDebugOutput(bool) {}

size_t ESPEasySerial_I2C_SC16IS752::setRxBufferSize(size_t new_size)
{
  if (_i2cserial != nullptr) {
    return 64;
  }
  return 0;
}

size_t ESPEasySerial_I2C_SC16IS752::setTxBufferSize(size_t new_size)
{
  if (_i2cserial != nullptr) {
    return 64;
  }
  return 0;
}

#endif // ifndef DISABLE_SC16IS752_Serial
