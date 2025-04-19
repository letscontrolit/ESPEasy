#include "ESPeasySerial.h"

#include "Port_ESPEasySerial_HardwareSerial.h"
#if USES_I2C_SC16IS752
# include "Port_ESPEasySerial_I2C_SC16IS752.h"
#endif // if USES_I2C_SC16IS752
#if USES_SW_SERIAL
# include "Port_ESPEasySerial_SW_Serial.h"
#endif // if USES_SW_SERIAL
#if USES_HWCDC
# include "Port_ESPEasySerial_USB_HWCDC.h"
#endif // if USES_HWCDC
#if USES_USBCDC
# include "Port_ESPEasySerial_USBCDC.h"
#endif // if USES_USBCDC


Port_ESPEasySerial_base * ESPeasySerial::ESPEasySerial_Port_factory(const ESPEasySerialConfig& config)
{
  switch (config.port) {
#if USES_SW_SERIAL
    case ESPEasySerialPort::software:
    {
      return new (std::nothrow) Port_ESPEasySerial_SW_Serial_t(config);
    }
#endif // if USES_SW_SERIAL
#if USES_I2C_SC16IS752
    case ESPEasySerialPort::sc16is752:
    {
      return new (std::nothrow) Port_ESPEasySerial_I2C_SC16IS752_t(config);
    }
#endif // if USES_I2C_SC16IS752
#if USES_HWCDC
    case ESPEasySerialPort::usb_hw_cdc:
    {
      return new (std::nothrow) Port_ESPEasySerial_USB_HWCDC_t(config);
    }
#endif // if USES_HWCDC
#if USES_USBCDC
    case ESPEasySerialPort::usb_cdc_0:
//    case ESPEasySerialPort::usb_cdc_1:
    {
      return new (std::nothrow) Port_ESPEasySerial_USBCDC_t(config);
    }
#endif // if USES_USBCDC

    default:

      if (isHWserial(config.port)) {
        Port_ESPEasySerial_HardwareSerial_t *hw = new (std::nothrow) Port_ESPEasySerial_HardwareSerial_t();

        if (hw != nullptr) {
          hw->resetConfig(config);
          return hw;
        }
      }
      break;
  }

  return nullptr;
}

ESPeasySerial::ESPeasySerial(ESPEasySerialPort port,
                             int               receivePin,
                             int               transmitPin,
                             bool              inverse_logic,
                             unsigned int      buffSize,
                             bool              forceSWserial)
  : Stream()
{
  ESPEasySerialConfig config;

  config.port          =  port;
  config.receivePin    = receivePin;
  config.transmitPin   = transmitPin;
  config.inverse_logic = inverse_logic;
  config.rxBuffSize    = buffSize;
  config.txBuffSize    = buffSize;
  config.forceSWserial = forceSWserial;
  config.validate();


  _serialPort = ESPEasySerial_Port_factory(config);


  // FIXME TD-er: Should it call begin() ???
  if (_serialPort != nullptr) {
    //    _serialPort->begin(_baud);
  }
}

ESPeasySerial::ESPeasySerial(ESPEasySerialConfig config)
: Stream()
{
  config.validate();
  _serialPort = ESPEasySerial_Port_factory(config);
}

ESPeasySerial::~ESPeasySerial() {
  flush();
  end();

  if (_serialPort != nullptr) {
    delete _serialPort;
  }
}

void ESPeasySerial::begin(unsigned long baud)
{
  if (isValid()) {
    _serialPort->begin(baud);
  }
}

#ifdef ESP8266
void ESPeasySerial::begin(unsigned long baud,
                          SerialConfig  config,
                          SerialMode    mode)
{
  if (isValid()) {
    _serialPort->setPortConfig(baud, config, mode);
    _serialPort->begin(baud);
  }
}

#endif // ifdef ESP8266

#ifdef ESP32
void ESPeasySerial::begin(unsigned long baud, uint32_t config)
{
  if (isValid()) {
    _serialPort->setPortConfig(baud, config);
    _serialPort->begin(baud);
  }
}

#endif // ifdef ESP32


void ESPeasySerial::end() {
  if (isValid()) {
    flush();
    _serialPort->end();
  }
}

int ESPeasySerial::peek(void)
{
  if (!isValid()) {
    return -1;
  }
  return _serialPort->peek();
}

size_t ESPeasySerial::write(uint8_t val)
{
  if (!isValid()) {
    return 0;
  }
  return _serialPort->write(val);
}

size_t ESPeasySerial::write(const uint8_t *buffer, size_t size)
{
  if (!isValid() || !buffer) {
    return 0;
  }
  return _serialPort->write(buffer, size);
}

size_t ESPeasySerial::write(const char *buffer)
{
  if (!buffer) { return 0; }
  return write(buffer, strlen_P(buffer));
}

int ESPeasySerial::read(void)
{
  if (!isValid()) {
    return -1;
  }
  return _serialPort->read();
}

int ESPeasySerial::available(void)
{
  if (!isValid()) {
    return 0;
  }
  return _serialPort->available();
}

int ESPeasySerial::availableForWrite(void)
{
  if (!isValid()) {
    return 0;
  }
  return _serialPort->availableForWrite();
}

void ESPeasySerial::flush(void)
{
  if (isValid()) {
    _serialPort->flush();
  }
}

int ESPeasySerial::getBaudRate() const
{
  if (!isValid()) {
    return 0;
  }
  return _serialPort->getBaudRate();
}

ESPeasySerial::operator bool() const
{
  if (isValid()) {
    const bool res = (*_serialPort);
    return res;
  }
  return false;
}

bool ESPeasySerial::connected() const
{
  if (isValid()) {
    const bool res = (*_serialPort);
    return res;
  }
  return false;
}

void ESPeasySerial::setDebugOutput(bool enable)
{
  if (isValid()) {
    _serialPort->setDebugOutput(enable);
  }
}

size_t ESPeasySerial::setRxBufferSize(size_t new_size)
{
  if (isValid()) {
    return _serialPort->setRxBufferSize(new_size);
  }
  return 0;
}

size_t ESPeasySerial::setTxBufferSize(size_t new_size)
{
  if (isValid()) {
    return _serialPort->setTxBufferSize(new_size);
  }
  return 0;
}

bool ESPeasySerial::isTxEnabled(void)
{
  return getTxPin() != -1;
}

bool ESPeasySerial::isRxEnabled(void)
{
  return getRxPin() != -1;
}

// Not supported in ESP32, since only HW serial is used.
// Function included since it is used in some libraries.
bool ESPeasySerial::listen() {
  return true;
}

String ESPeasySerial::getLogString() const {
  return getSerialConfig().getLogString();
}

bool ESPeasySerial::setRS485Mode(int8_t rtsPin, bool enableCollisionDetection) {
  if (_serialPort != nullptr) {
    #ifdef ESP32
    return _serialPort->setRS485Mode(rtsPin, enableCollisionDetection);
    #else
    return _serialPort->setRS485Mode(rtsPin, false);
    #endif
  }
  return false;
}

bool ESPeasySerial::isValid() const {
  // FIXME TD-er: Must call isValid() on the individual _serialPort types
  return _serialPort != nullptr;
}
