#include <ESPeasySerial.h>


#include "wrappers/ESPEasySerial_HardwareSerial.h"
#include "wrappers/ESPEasySerial_I2C_SC16IS752.h"
#include "wrappers/ESPEasySerial_SW_Serial.h"
#include "wrappers/ESPEasySerial_USB_HWCDC.h"
#include "wrappers/ESPEasySerial_USBCDC.h"

ESPeasySerial::ESPeasySerial(ESPEasySerialPort port,
                             int               receivePin,
                             int               transmitPin,
                             bool              inverse_logic,
                             unsigned int      buffSize,
                             bool              forceSWserial)
{
  ESPEasySerialConfig config;
  config.port =  port;
  config.receivePin = receivePin;
  config.transmitPin = transmitPin;
  config.inverse_logic = inverse_logic;
  config.buffSize = buffSize;
  config.forceSWserial = forceSWserial;
  config.validate();

  switch (config.port) {
#if USES_SW_SERIAL
    case ESPEasySerialPort::software:
    {
      _serialPort = new ESPEasySerial_SW_Serial(config);
      break;
    }
#endif
#if USES_I2C_SC16IS752
    case ESPEasySerialPort::sc16is752:
    {
      _serialPort = new ESPEasySerial_I2C_SC16IS752(config);
      break;
    }
#endif
#if USES_HWCDC
    case ESPEasySerialPort::usb_hw_cdc:
    {
      _serialPort = new ESPEasySerial_USB_WHCDC_t(config);
      break;
    }
#endif
#if USES_USBCDC
    case ESPEasySerialPort::usb_cdc_0:
    case ESPEasySerialPort::usb_cdc_1:
    {
      _serialPort = new ESPEasySerial_USBCDC_t(config);
      break;
    }
#endif
    default:

      if (isHWserial(config.port)) {
        _serialPort = new ESPEasySerial_HardwareSerial_t(config);
        if (_serialPort != nullptr) {
          reinterpret_cast<ESPEasySerial_HardwareSerial_t*>(_serialPort)->resetConfig(config);
        }
      }
      break;
  }

  // FIXME TD-er: Should it call begin() ???
  if (_serialPort != nullptr) {
//    _serialPort->begin(_baud);
  }
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
#endif

#ifdef ESP32
void ESPeasySerial::begin(unsigned long baud, uint32_t config)
{
  if (isValid()) {
    _serialPort->setPortConfig(baud, config);
    _serialPort->begin(baud);
  }
}
#endif


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

int ESPeasySerial::baudRate(void) 
{
  if (!isValid()) {
    return 0;
  }
  return getSerialConfig().baud;
}

void ESPeasySerial::setDebugOutput(bool enable) 
{
  if (isValid()) {
    _serialPort->setDebugOutput(enable);
  }
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


bool ESPeasySerial::isValid() const {
  // FIXME TD-er: Must call isValid() on the individual _serialPort types
  return _serialPort != nullptr;
}