#include "ESPEasySerial_SW_Serial.h"


#if USES_SW_SERIAL


ESPEasySerial_SW_Serial::ESPEasySerial_SW_Serial(const ESPEasySerialConfig & config)
{
  if (config.port == ESPEasySerialPort::software) {
  _config = config;
  _swserial    = new ESPeasySoftwareSerial(
    config.receivePin, 
    config.transmitPin, 
    config.inverse_logic);
  }
}

ESPEasySerial_SW_Serial::~ESPEasySerial_SW_Serial()
{
  if (_swserial != nullptr) {
    delete _swserial;
    _swserial = nullptr;
  }
}

void ESPEasySerial_SW_Serial::begin(unsigned long baud)
{
  if (_swserial != nullptr) {
    _swserial->begin(baud);
  }
}

void ESPEasySerial_SW_Serial::end() {
  if (_swserial != nullptr) {
    _swserial->end();
  }
}

int ESPEasySerial_SW_Serial::available(void)
{
  if (_swserial != nullptr) {
    return _swserial->available();
  }
  return 0;
}

int ESPEasySerial_SW_Serial::availableForWrite(void)
{
  if (_swserial != nullptr) {
    // FIXME TD-er: Implement availableForWrite
    return 1; //  return _swserial->availableForWrite();
  }
  return 0;
}

int ESPEasySerial_SW_Serial::peek(void)
{
  if (_swserial != nullptr) {
    return _swserial->peek();
  }
  return 0;
}

int ESPEasySerial_SW_Serial::read(void)
{
  if (_swserial != nullptr) {
    return _swserial->read();
  }
  return 0;
}

size_t ESPEasySerial_SW_Serial::read(uint8_t *buffer,
                                     size_t   size)
{
  if (_swserial != nullptr) {
    return _swserial->readBytes((char *)buffer, size);
  }
  return 0;
}

void ESPEasySerial_SW_Serial::flush(void)
{
  if (_swserial != nullptr) {
    return _swserial->flush();
  }
}

void ESPEasySerial_SW_Serial::flush(bool txOnly)
{
  flush();
}

size_t ESPEasySerial_SW_Serial::write(uint8_t value)
{
  if (_swserial != nullptr) {
    return _swserial->write(value);
  }
  return 0;
}

size_t ESPEasySerial_SW_Serial::write(const uint8_t *buffer,
                                      size_t         size)
{
  if (_swserial != nullptr) {
    return _swserial->write(buffer, size);
  }
  return 0;
}

ESPEasySerial_SW_Serial::operator bool() const
{
  return _swserial != nullptr;
}

void   ESPEasySerial_SW_Serial::setDebugOutput(bool) {}

size_t ESPEasySerial_SW_Serial::setRxBufferSize(size_t new_size)
{
  if (_swserial != nullptr) {
    return 64;
  }
  return 0;
}

size_t ESPEasySerial_SW_Serial::setTxBufferSize(size_t new_size)
{
  if (_swserial != nullptr) {
    return 64;
  }
  return 0;
}

#endif // if USES_SW_SERIAL
