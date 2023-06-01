#include "Port_ESPEasySerial_HardwareSerial.h"


#include "ESPEasySerialType.h"


#ifdef ESP32

// Temporary work-around for bug in ESP32 code, where the pin matrix is not cleaned up between calling end() and begin()
// Work-around is to keep track of the last used pins for a serial port,
// as it is likely that a node will always use the same pins most of the time.
// If not, a reboot may be OK to fix it.
// Another idea is to swap pins among UART ports.
// e.g. use the same pins on Serial1 if it was used on Serial2 before.

// PR to fix it: https://github.com/espressif/arduino-esp32/pull/5385
// See: https://github.com/espressif/arduino-esp32/issues/3878


static int receivePin0  = -1;
static int transmitPin0 = -1;
static int receivePin1  = -1;
static int transmitPin1 = -1;
static int receivePin2  = -1;
static int transmitPin2 = -1;

bool pinsChanged(ESPEasySerialPort port,
                 int               receivePin,
                 int               transmitPin)
{
  switch (port) {
    case  ESPEasySerialPort::serial0: return receivePin != receivePin0 || transmitPin != transmitPin0;
    # if SOC_UART_NUM > 1
    case  ESPEasySerialPort::serial1: return receivePin != receivePin1 || transmitPin != transmitPin1;
    # endif // if SOC_UART_NUM > 1
    # if SOC_UART_NUM > 2
    case  ESPEasySerialPort::serial2: return receivePin != receivePin2 || transmitPin != transmitPin2;
    # endif // if SOC_UART_NUM > 2
    default:
      // No other hardware serial ports
      break;
  }
  return false;
}

void setPinsCache(ESPEasySerialPort port,
                  int               receivePin,
                  int               transmitPin)
{
  switch (port) {
    case  ESPEasySerialPort::serial0:
      receivePin0  = receivePin;
      transmitPin0 = transmitPin;
      break;
    # if SOC_UART_NUM > 1
    case  ESPEasySerialPort::serial1:
      receivePin1  = receivePin;
      transmitPin1 = transmitPin;
      break;
    # endif // if SOC_UART_NUM > 1

    # if SOC_UART_NUM > 2
    case  ESPEasySerialPort::serial2:
      receivePin2  = receivePin;
      transmitPin2 = transmitPin;
      break;

    # endif // if SOC_UART_NUM > 2
    default:
      // No other hardware serial ports
      break;
  }
}

#endif // ifdef ESP32

Port_ESPEasySerial_HardwareSerial_t::Port_ESPEasySerial_HardwareSerial_t() {}

Port_ESPEasySerial_HardwareSerial_t::~Port_ESPEasySerial_HardwareSerial_t() {}

void Port_ESPEasySerial_HardwareSerial_t::resetConfig(const ESPEasySerialConfig& config)
{
  if (!isHWserial(config.port)) { return; }

  /*
     if (_config == config) return;

     // First call end()
     // Then create new instance.

     _config.receivePin    = receivePin;
     _config.transmitPin   = transmitPin;
     _config.inverse_logic = inverse_logic;
     _config.rxBuffSize    = buffSize;
     _config.txBuffSize    = buffSize;
   */

  _config = config;


  switch (config.port) {
    case  ESPEasySerialPort::serial0:
    #if SOC_UART_NUM > 1
    case  ESPEasySerialPort::serial1:
    #endif // if SOC_UART_NUM > 1
    #if SOC_UART_NUM > 2
    case  ESPEasySerialPort::serial2:
    #endif // if SOC_UART_NUM > 2
      _config.port = config.port;
      break;
    default:
      _config.port = ESPeasySerialType::getSerialType(config.port, config.receivePin, config.transmitPin);
  }

  if (_config.port == ESPEasySerialPort::serial0) {
#if defined(ESP32) && !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SERIAL) && ARDUINO_USB_CDC_ON_BOOT // Serial used for USB CDC
    _serial = &Serial0;
#else // if defined(ESP32) && !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SERIAL) && ARDUINO_USB_CDC_ON_BOOT
    _serial = &Serial;
#endif // if defined(ESP32) && !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SERIAL) && ARDUINO_USB_CDC_ON_BOOT
#ifdef ESP8266
  } else if (_config.port == ESPEasySerialPort::serial0_swap) {
    _serial = &Serial;
#endif // ifdef ESP8266
#if SOC_UART_NUM > 1
  } else if (_config.port == ESPEasySerialPort::serial1) {
    _serial = &Serial1;
#endif // if SOC_UART_NUM > 1
#if SOC_UART_NUM > 2
  } else if (_config.port == ESPEasySerialPort::serial2) {
    _serial = &Serial2;
#endif // if SOC_UART_NUM > 2
  } else {
    _config.port = ESPEasySerialPort::not_set;
  }
}

#ifdef ESP8266
void Port_ESPEasySerial_HardwareSerial_t::begin(unsigned long baud)
{
  _config.baud = 0;
  if (_serial == nullptr) {
    return;
  }

  /* Order: TX/RX
   * UART 0 possible options are (1, 3), (2, 3) or (15, 13)
   * UART 1 allows only TX on 2 if UART 0 is not (2, 3)
   */

  if (_config.port == ESPEasySerialPort::serial0_swap) {
    _config.transmitPin = 15;
    _config.receivePin  = 13;
  } else if (_config.port == ESPEasySerialPort::serial0) {
    _config.transmitPin = 1;
    _config.receivePin  = 3;
  } else if (_config.port == ESPEasySerialPort::serial1) {
    _config.transmitPin = 2;
    _config.receivePin  = -1;
  } else {
    _config.transmitPin = -1;
    _config.receivePin  = -1;
    return;
  }

  _config.baud = baud;

  _serial->setRxBufferSize(_config.rxBuffSize);

  _serial->begin(_config.baud, _config.config, _config.mode, _config.transmitPin, _config.inverse_logic);
  _serial->pins(_config.transmitPin, _config.receivePin);
}

#endif // ifdef ESP8266

#ifdef ESP32
void Port_ESPEasySerial_HardwareSerial_t::begin(unsigned long baud)
{
  if (_serial == nullptr) {
    _config.baud = 0;
    return;
  }

  // Timeout added for 1.0.1
  // See: https://github.com/espressif/arduino-esp32/commit/233d31bed22211e8c85f82bcf2492977604bbc78
  // getHW()->begin(baud, config, _config.receivePin, _config.transmitPin, invert, timeout_ms);
  if (pinsChanged(_config.port, _config.receivePin, _config.transmitPin) || (_config.baud != baud)) {
    setPinsCache(_config.port, _config.receivePin, _config.transmitPin);
    _config.baud = baud;

    // Allow to flush data from the serial buffers
    // Otherwise the ESP may hang at boot.
    delay(10);
    _serial->end();
    delay(10);

    if (_config.rxBuffSize > 256) {
      _config.rxBuffSize = _serial->setRxBufferSize(_config.rxBuffSize);
    }

    if (_config.txBuffSize > 256) {
      _config.txBuffSize = _serial->setRxBufferSize(_config.txBuffSize);
    }

    _serial->begin(baud, _config.config, _config.receivePin, _config.transmitPin, _config.inverse_logic);
    _serial->flush();
  }
}

#endif // ifdef ESP32

void Port_ESPEasySerial_HardwareSerial_t::end() {
  if (_serial != nullptr) {
    _serial->end();

    // FIXME TD-er: Should we restore the pins for ESP8266?
  }
}

int Port_ESPEasySerial_HardwareSerial_t::available(void)
{
  if (_serial != nullptr) {
    return _serial->available();
  }
  return 0;
}

int Port_ESPEasySerial_HardwareSerial_t::availableForWrite(void)
{
  if (_serial != nullptr) {
    return _serial->availableForWrite();
  }
  return 0;
}

int Port_ESPEasySerial_HardwareSerial_t::peek(void)
{
  if (_serial != nullptr) {
    return _serial->peek();
  }
  return 0;
}

int Port_ESPEasySerial_HardwareSerial_t::read(void)
{
  if (_serial != nullptr) {
    return _serial->read();
  }
  return 0;
}

size_t Port_ESPEasySerial_HardwareSerial_t::read(uint8_t *buffer,
                                                 size_t   size)
{
  if (_serial != nullptr) {
    #ifdef ESP32
    return _serial->read(buffer, size);
    #endif // ifdef ESP32
    #ifdef ESP8266
    return _serial->read((char *)buffer, size);
    #endif // ifdef ESP8266
  }
  return 0;
}

void Port_ESPEasySerial_HardwareSerial_t::flush(void)
{
  if (_serial != nullptr) {
    _serial->flush();
  }
}

void Port_ESPEasySerial_HardwareSerial_t::flush(bool txOnly)
{
  if (_serial != nullptr) {
    #ifdef ESP32
    _serial->flush(txOnly);
    #endif // ifdef ESP32
    #ifdef ESP8266
    _serial->flush();
    #endif // ifdef ESP8266
  }
}

size_t Port_ESPEasySerial_HardwareSerial_t::write(uint8_t value)
{
  if (_serial != nullptr) {
    return _serial->write(value);
  }
  return 0;
}

size_t Port_ESPEasySerial_HardwareSerial_t::write(const uint8_t *buffer,
                                                  size_t         size)
{
  if (_serial != nullptr) {
    return _serial->write(buffer, size);
  }
  return 0;
}

int Port_ESPEasySerial_HardwareSerial_t::getBaudRate() const
{
  if (_serial != nullptr) {
    return _serial->baudRate();
  }
  return 0;
}

Port_ESPEasySerial_HardwareSerial_t::operator bool() const
{
  if (_serial != nullptr) {
    return _serial->operator bool();
  }
  return false;
}

void Port_ESPEasySerial_HardwareSerial_t::setDebugOutput(bool enabled) {
  if (_serial != nullptr) {
    return _serial->setDebugOutput(enabled);
  }
}

size_t Port_ESPEasySerial_HardwareSerial_t::setRxBufferSize(size_t new_size)
{
  if (_serial != nullptr) {
    return _serial->setRxBufferSize(new_size);
  }
  return 0;
}

size_t Port_ESPEasySerial_HardwareSerial_t::setTxBufferSize(size_t new_size)
{
  if (_serial != nullptr) {
    #ifdef ESP32
    return _serial->setTxBufferSize(new_size);
    #endif // ifdef ESP32
    #ifdef ESP8266
    return new_size;
    #endif // ifdef ESP8266
  }
  return 0;
}
