#include "Port_ESPEasySerial_SW_Serial.h"


#if USES_SW_SERIAL


Port_ESPEasySerial_SW_Serial_t::Port_ESPEasySerial_SW_Serial_t(const ESPEasySerialConfig& config)
{
  if (config.port == ESPEasySerialPort::software) {
    _config   = config;
    _swserial = new (std::nothrow) Driver_ESPEasySoftwareSerial_t(
      config.receivePin,
      config.transmitPin,
      config.inverse_logic,
      config.rxBuffSize);
  }
}

Port_ESPEasySerial_SW_Serial_t::~Port_ESPEasySerial_SW_Serial_t()
{
  if (_swserial != nullptr) {
    delete _swserial;
    _swserial = nullptr;
  }
}

void Port_ESPEasySerial_SW_Serial_t::begin(unsigned long baud)
{
  if (_swserial != nullptr) {
    _config.baud = baud;
    _swserial->begin(baud);
  }
}

void Port_ESPEasySerial_SW_Serial_t::end() {
  if (_swserial != nullptr) {
    _swserial->end();
  }
}

int Port_ESPEasySerial_SW_Serial_t::available(void)
{
  if (_swserial != nullptr) {
    return _swserial->available();
  }
  return 0;
}

int Port_ESPEasySerial_SW_Serial_t::availableForWrite(void)
{
  if (_swserial != nullptr) {
    // FIXME TD-er: Implement availableForWrite
    return 1; //  return _swserial->availableForWrite();
  }
  return 0;
}

int Port_ESPEasySerial_SW_Serial_t::peek(void)
{
  if (_swserial != nullptr) {
    return _swserial->peek();
  }
  return 0;
}

int Port_ESPEasySerial_SW_Serial_t::read(void)
{
  if (_swserial != nullptr) {
    return _swserial->read();
  }
  return 0;
}

size_t Port_ESPEasySerial_SW_Serial_t::read(uint8_t *buffer,
                                            size_t   size)
{
  if (_swserial != nullptr) {
    return _swserial->readBytes((char *)buffer, size);
  }
  return 0;
}

void Port_ESPEasySerial_SW_Serial_t::flush(void)
{
  if (_swserial != nullptr) {
    return _swserial->flush();
  }
}

void Port_ESPEasySerial_SW_Serial_t::flush(bool txOnly)
{
  flush();
}

size_t Port_ESPEasySerial_SW_Serial_t::write(uint8_t value)
{
  if (_swserial != nullptr) {
    return _swserial->write(value);
  }
  return 0;
}

size_t Port_ESPEasySerial_SW_Serial_t::write(const uint8_t *buffer,
                                             size_t         size)
{
  if (_swserial != nullptr) {
    return _swserial->write(buffer, size);
  }
  return 0;
}

Port_ESPEasySerial_SW_Serial_t::operator bool() const
{
  return _swserial != nullptr;
}

void   Port_ESPEasySerial_SW_Serial_t::setDebugOutput(bool) {}

size_t Port_ESPEasySerial_SW_Serial_t::setRxBufferSize(size_t new_size)
{
  if (_swserial != nullptr) {
    _config.rxBuffSize = new_size;
    return new_size;
  }
  return 0;
}

size_t Port_ESPEasySerial_SW_Serial_t::setTxBufferSize(size_t new_size)
{
  if (_swserial != nullptr) {
    _config.txBuffSize = new_size;
    return new_size;
  }
  return 0;
}

#endif // if USES_SW_SERIAL
