#include <ESPeasySerial.h>


// ****************************************
// ESP32 implementation wrapper
// Only support HW serial on Serial 0 .. 2
// ****************************************

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
    case  ESPEasySerialPort::serial1: return receivePin != receivePin1 || transmitPin != transmitPin1;
    # if HAS_SERIAL2
    case  ESPEasySerialPort::serial2: return receivePin != receivePin2 || transmitPin != transmitPin2;
    # endif 
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
    case  ESPEasySerialPort::serial1:
      receivePin1  = receivePin;
      transmitPin1 = transmitPin;
      break;

    # if HAS_SERIAL2
    case  ESPEasySerialPort::serial2:
      receivePin2  = receivePin;
      transmitPin2 = transmitPin;
      break;

    # endif 
    default:
      // No other hardware serial ports
      break;
  }
}

// End of messy work-around.

void ESPeasySerial::resetConfig(
  ESPEasySerialPort port, 
  int receivePin, 
  int transmitPin, 
  bool inverse_logic,
  unsigned int buffSize)
{
#ifndef DISABLE_SC16IS752_Serial
  if (_i2cserial != nullptr) {
    _i2cserial->end();
    delete _i2cserial;
  }
#endif
#ifndef DISABLE_SC16IS752_Serial
  _i2cserial = nullptr;
#endif
  _receivePin = receivePin;
  _transmitPin = transmitPin;
  _inverse_logic = inverse_logic;
  _buffSize = buffSize;

  switch (port) {
    case  ESPEasySerialPort::serial0:
    case  ESPEasySerialPort::serial1:
    # if HAS_SERIAL2
    case  ESPEasySerialPort::serial2:
    # endif 
      _serialtype = port;
      break;
    default:
      _serialtype = ESPeasySerialType::getSerialType(port, receivePin, transmitPin);
  }

# ifndef DISABLE_SC16IS752_Serial

  switch (_serialtype) {
    case ESPEasySerialPort::sc16is752:
    {
      ESPEasySC16IS752_Serial::I2C_address addr     = static_cast<ESPEasySC16IS752_Serial::I2C_address>(receivePin);
      ESPEasySC16IS752_Serial::SC16IS752_channel ch = static_cast<ESPEasySC16IS752_Serial::SC16IS752_channel>(transmitPin);
      _i2cserial = new ESPEasySC16IS752_Serial(addr, ch);
      break;
    }
    default:
      break;
  }
# endif // ifndef DISABLE_SC16IS752_Serial
}

void ESPeasySerial::begin(unsigned long baud, uint32_t config
                          , int8_t rxPin, int8_t txPin, bool invert, unsigned long timeout_ms) {
  _baud = baud;

  if (rxPin != -1) { _receivePin = rxPin; }

  if (txPin != -1) { _transmitPin = txPin; }

  _inverse_logic = invert;

  if (!isValid()) {
    _baud = 0;
    return;
  }

  if (isI2Cserial()) {
# ifndef DISABLE_SC16IS752_Serial

    if (_i2cserial != nullptr) {
      _i2cserial->begin(baud);
    }
# endif // ifndef DISABLE_SC16IS752_Serial
  } else {
    // Make sure the extra bit is set for the config. The config differs between ESP32 and ESP82xx
    config = config | 0x8000000;

    if (isValid()) {
      // Timeout added for 1.0.1
      // See: https://github.com/espressif/arduino-esp32/commit/233d31bed22211e8c85f82bcf2492977604bbc78
      // getHW()->begin(baud, config, _receivePin, _transmitPin, invert, timeout_ms);
      if (pinsChanged(_serialtype, _receivePin, _transmitPin)) {
        setPinsCache(_serialtype, _receivePin, _transmitPin);
        // Allow to flush data from the serial buffers
        // When not opening the USB serial port, the ESP may hang at boot.
        delay(10); 
        getHW()->end();
        delay(10); 

        getHW()->begin(baud, config, _receivePin, _transmitPin, _inverse_logic);
      }
    }
  }
}

void ESPeasySerial::end() {
  if (!isValid()) {
    return;
  }
  flush();

  if (isI2Cserial()) {
# ifndef DISABLE_SC16IS752_Serial
    _i2cserial->end();
# endif // ifndef DISABLE_SC16IS752_Serial
  } else {
    // Work-around to fix proper detach RX pin for older ESP32 core versions
    // For now do not call end()
    // getHW()->end();
  }
}

HardwareSerial * ESPeasySerial::getHW() {
  switch (_serialtype) {
#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SERIAL) && ARDUINO_USB_CDC_ON_BOOT //Serial used for USB CDC
    case ESPEasySerialPort::serial0: return &Serial0;
#else
    case ESPEasySerialPort::serial0: return &Serial;
#endif
    case ESPEasySerialPort::serial1: return &Serial1;
    case ESPEasySerialPort::serial2:
    #if HAS_SERIAL2
      return &Serial2;
    #endif

    default: break;
  }
  return nullptr;
}

const HardwareSerial * ESPeasySerial::getHW() const {
  switch (_serialtype) {
#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SERIAL) && ARDUINO_USB_CDC_ON_BOOT //Serial used for USB CDC
    case ESPEasySerialPort::serial0: return &Serial0;
#else
    case ESPEasySerialPort::serial0: return &Serial;
#endif
    case ESPEasySerialPort::serial1: return &Serial1;
    case ESPEasySerialPort::serial2:
    #if HAS_SERIAL2
      return &Serial2;
    #endif
    default: break;
  }
  return nullptr;
}

bool ESPeasySerial::isValid() const {
  switch (_serialtype) {
    case ESPEasySerialPort::serial0:
    case ESPEasySerialPort::serial1:
      return true;
    case ESPEasySerialPort::serial2:
      #if HAS_SERIAL2
      return true;
      #else 
      return false;
      #endif 
    case ESPEasySerialPort::sc16is752:
    # ifndef DISABLE_SC16IS752_Serial
      return _i2cserial != nullptr;
    # else // ifndef DISABLE_SC16IS752_Serial
      return false;
    # endif // ifndef DISABLE_SC16IS752_Serial

    // FIXME TD-er: Must perform proper check for GPIO pins here.
    default: break;
  }
  return false;
}

int ESPeasySerial::peek(void) {
  if (!isValid()) {
    return -1;
  }

  if (isI2Cserial()) {
# ifndef DISABLE_SC16IS752_Serial
    return _i2cserial->peek();
# else // ifndef DISABLE_SC16IS752_Serial
    return false;
# endif // ifndef DISABLE_SC16IS752_Serial
  }
  return getHW()->peek();
}

size_t ESPeasySerial::write(uint8_t val) {
  if (!isValid()) {
    return 0;
  }

  if (isI2Cserial()) {
# ifndef DISABLE_SC16IS752_Serial
    return _i2cserial->write(val);
# else // ifndef DISABLE_SC16IS752_Serial
    return 0;
# endif // ifndef DISABLE_SC16IS752_Serial
  }
  return getHW()->write(val);
}

size_t ESPeasySerial::write(const uint8_t *buffer, size_t size) {
  if (!isValid() || !buffer) {
    return 0;
  }

  if (isI2Cserial()) {
# ifndef DISABLE_SC16IS752_Serial
    return _i2cserial->write(buffer, size);
# else // ifndef DISABLE_SC16IS752_Serial
    return 0;
# endif // ifndef DISABLE_SC16IS752_Serial
  }
  return getHW()->write(buffer, size);
}

size_t ESPeasySerial::write(const char *buffer) {
  if (!buffer) { return 0; }
  return write(buffer, strlen_P(buffer));
}

int ESPeasySerial::read(void) {
  if (!isValid()) {
    return -1;
  }

  if (isI2Cserial()) {
# ifndef DISABLE_SC16IS752_Serial
    return _i2cserial->read();
# else // ifndef DISABLE_SC16IS752_Serial
    return -1;
# endif // ifndef DISABLE_SC16IS752_Serial
  }
  return getHW()->read();
}

int ESPeasySerial::available(void) {
  if (!isValid()) {
    return 0;
  }

  if (isI2Cserial()) {
# ifndef DISABLE_SC16IS752_Serial
    return _i2cserial->available();
# else // ifndef DISABLE_SC16IS752_Serial
    return 0;
# endif // ifndef DISABLE_SC16IS752_Serial
  }
  return getHW()->available();
}

int ESPeasySerial::availableForWrite(void) {
  if (!isValid()) {
    return 0;
  }

  if (isI2Cserial()) {
#ifndef DISABLE_SC16IS752_Serial
    // FIXME TD-er: Implement availableForWrite
    return 64; // _i2cserial->availableForWrite();
#else
    return 0;
#endif
  } else {
    return getHW()->availableForWrite();
  }
}

void ESPeasySerial::flush(void) {
  if (!isValid()) {
    return;
  }

  if (isI2Cserial()) {
# ifndef DISABLE_SC16IS752_Serial
    _i2cserial->flush();
# endif // ifndef DISABLE_SC16IS752_Serial
  } else {
    getHW()->flush();
  }
}

int ESPeasySerial::baudRate(void) {
  if (!isValid()) {
    return 0;
  }

  if (isI2Cserial()) {
    return _baud;
  }
  return getHW()->baudRate();
}

void ESPeasySerial::setDebugOutput(bool enable) {
  if (!isValid() || isI2Cserial()) {
    return;
  }
  getHW()->setDebugOutput(enable);
}

bool ESPeasySerial::isTxEnabled(void) {
  if (!isValid()) {
    return false;
  }

  if (isI2Cserial()) {
    return true;
  }
  return _transmitPin != -1;
}

bool ESPeasySerial::isRxEnabled(void) {
  if (!isValid()) {
    return false;
  }

  if (isI2Cserial()) {
    return true;
  }
  return _receivePin != -1;
}

// Not supported in ESP32, since only HW serial is used.
// Function included since it is used in some libraries.
bool ESPeasySerial::listen() {
  return true;
}

#endif // ESP32
