#include "Port_ESPEasySerial_I2C_SC16IS752.h"


#if USES_I2C_SC16IS752


Port_ESPEasySerial_I2C_SC16IS752_t::Port_ESPEasySerial_I2C_SC16IS752_t(const ESPEasySerialConfig& config)
{
  if (config.port ==  ESPEasySerialPort::sc16is752) {
    ESPEasySC16IS752_Serial::I2C_address addr;
    ESPEasySC16IS752_Serial::SC16IS752_channel ch;

    if (config.getI2C_SC16IS752_Parameters(addr, ch)) {
      _config            = config;
      _config.rxBuffSize = 64; // Fixed size
      _config.txBuffSize = 64; // Fixed size
      _i2cserial         = new (std::nothrow) ESPEasySC16IS752_Serial(addr, ch);
    }
  }
}

Port_ESPEasySerial_I2C_SC16IS752_t::~Port_ESPEasySerial_I2C_SC16IS752_t()
{
  if (_i2cserial != nullptr) {
    delete _i2cserial;
    _i2cserial = nullptr;
  }
}

void Port_ESPEasySerial_I2C_SC16IS752_t::begin(unsigned long baud)
{
  if (_i2cserial != nullptr) {
    _config.baud = baud;
    _i2cserial->begin(baud);
  }
}

void Port_ESPEasySerial_I2C_SC16IS752_t::end() {
  if (_i2cserial != nullptr) {
    _i2cserial->end();
  }
}

int Port_ESPEasySerial_I2C_SC16IS752_t::available(void)
{
  if (_i2cserial != nullptr) {
    return _i2cserial->available();
  }
  return 0;
}

int Port_ESPEasySerial_I2C_SC16IS752_t::availableForWrite(void)
{
  if (_i2cserial != nullptr) {
    // FIXME TD-er: Implement availableForWrite
    return 4; // _i2cserial->availableForWrite();
  }
  return 0;
}

int Port_ESPEasySerial_I2C_SC16IS752_t::peek(void)
{
  if (_i2cserial != nullptr) {
    return _i2cserial->peek();
  }
  return 0;
}

int Port_ESPEasySerial_I2C_SC16IS752_t::read(void)
{
  if (_i2cserial != nullptr) {
    return _i2cserial->read();
  }
  return 0;
}

size_t Port_ESPEasySerial_I2C_SC16IS752_t::read(uint8_t *buffer,
                                                size_t   size)
{
  if (_i2cserial != nullptr) {
    return _i2cserial->readBytes((char *)buffer, size);
  }
  return 0;
}

void Port_ESPEasySerial_I2C_SC16IS752_t::flush(void)
{
  if (_i2cserial != nullptr) {
    return _i2cserial->flush();
  }
}

void Port_ESPEasySerial_I2C_SC16IS752_t::flush(bool txOnly)
{
  flush();
}

size_t Port_ESPEasySerial_I2C_SC16IS752_t::write(uint8_t value)
{
  if (_i2cserial != nullptr) {
    return _i2cserial->write(value);
  }
  return 0;
}

size_t Port_ESPEasySerial_I2C_SC16IS752_t::write(const uint8_t *buffer,
                                                 size_t         size)
{
  if (_i2cserial != nullptr) {
    return _i2cserial->write(buffer, size);
  }
  return 0;
}

Port_ESPEasySerial_I2C_SC16IS752_t::operator bool() const
{
  return _i2cserial != nullptr;
}

void   Port_ESPEasySerial_I2C_SC16IS752_t::setDebugOutput(bool) {}

size_t Port_ESPEasySerial_I2C_SC16IS752_t::setRxBufferSize(size_t new_size)
{
  if (_i2cserial != nullptr) {
    return 64;
  }
  return 0;
}

size_t Port_ESPEasySerial_I2C_SC16IS752_t::setTxBufferSize(size_t new_size)
{
  if (_i2cserial != nullptr) {
    return 64;
  }
  return 0;
}

bool Port_ESPEasySerial_I2C_SC16IS752_t::setRS485Mode(int8_t rtsPin, bool enableCollisionDetection)
{
  // TODO TD-er: Check if we can enable RTS on this chip
  return false;
}

#endif // ifndef DISABLE_SC16IS752_Serial
