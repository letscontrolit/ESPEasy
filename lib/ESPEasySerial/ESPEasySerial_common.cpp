#include <ESPeasySerial.h>

/*
#include <deque>

std::deque<ESPeasySerial *> _ESPeasySerial_instances;
*/
#ifdef ESP8266

ESPeasySerial::ESPeasySerial(ESPEasySerialPort port,
                             int               receivePin,
                             int               transmitPin,
                             bool              inverse_logic,
                             unsigned int      buffSize,
                             bool              forceSWserial)
  :
# ifndef DISABLE_SC16IS752_Serial
  _i2cserial(nullptr),
# endif // ifndef DISABLE_SC16IS752_Serial
# if !defined(DISABLE_SOFTWARE_SERIAL) && defined(ESP8266)
  _swserial(nullptr),
# endif // if !defined(DISABLE_SOFTWARE_SERIAL) && defined(ESP8266)
  _receivePin(receivePin),
  _transmitPin(transmitPin),
  _inverse_logic(inverse_logic),
  _buffSize(buffSize),
#if defined(DISABLE_SOFTWARE_SERIAL)
  _forceSWserial(false)
#else
  _forceSWserial(forceSWserial)
#endif
{
  _serialtype =  ESPeasySerialType::getSerialType(port, receivePin, transmitPin);
#ifndef DISABLE_SOFTWARE_SERIAL
  if (forceSWserial) {
#ifndef DISABLE_SC16IS752_Serial
    if (_serialtype != ESPEasySerialPort::sc16is752) {
      _serialtype = ESPEasySerialPort::software;
    }
#else
    _serialtype = ESPEasySerialPort::software;
#endif
  }
#endif

  switch (_serialtype) {
    case ESPEasySerialPort::software:
    {
#ifndef DISABLE_SOFTWARE_SERIAL
      _swserial = new ESPeasySoftwareSerial(receivePin, transmitPin, inverse_logic);
#endif
      break;
    }
    case ESPEasySerialPort::sc16is752:
    {
#ifndef DISABLE_SC16IS752_Serial
      ESPEasySC16IS752_Serial::I2C_address addr     = static_cast<ESPEasySC16IS752_Serial::I2C_address>(receivePin);
      ESPEasySC16IS752_Serial::SC16IS752_channel ch = static_cast<ESPEasySC16IS752_Serial::SC16IS752_channel>(transmitPin);

      _i2cserial = new ESPEasySC16IS752_Serial(addr, ch);
#endif
      break;
    }
    default:

      if (isValid()) {
        getHW()->pins(transmitPin, receivePin);
      }
      break;
  }
  if (isValid()) {
    begin(_baud, _config, _mode);
  }
}

#endif // ifdef ESP8266

#ifdef ESP32
ESPeasySerial::ESPeasySerial(
  ESPEasySerialPort port,
  int               receivePin,
  int               transmitPin,
  bool              inverse_logic,
  unsigned int      buffSize)
  :
# ifndef DISABLE_SC16IS752_Serial
  _i2cserial(nullptr),
# endif // ifndef DISABLE_SC16IS752_Serial
  _receivePin(receivePin),
  _transmitPin(transmitPin),
  _inverse_logic(inverse_logic),
  _buffSize(buffSize)
{
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

#endif // ifdef ESP32

ESPeasySerial::~ESPeasySerial() {
  flush();
  end();
#if !defined(DISABLE_SOFTWARE_SERIAL) && defined(ESP8266)

  if (_swserial != nullptr) {
    delete _swserial;
  }
#endif // if !defined(DISABLE_SOFTWARE_SERIAL) && defined(ESP8266)
#ifndef DISABLE_SC16IS752_Serial

  if (_i2cserial != nullptr) {
    delete _i2cserial;
  }
#endif // ifndef DISABLE_SC16IS752_Serial
}

String ESPeasySerial::getLogString() const {
  String log;

  log.reserve(48);
  log = F("ESPEasy serial: ");

  if (isI2Cserial()) {
    log += F("I2C: addr:");
    log += String(_receivePin);
    log += F(" ch:");
    log += _transmitPin == 0 ? 'A' : 'B';
  } else {
    if (isSWserial()) {
      log += F("SW");
    } else {
      log += F("HW");
    }
    log += F(": rx:");
    log += String(_receivePin);
    log += F(" tx:");
    log += String(_transmitPin);
  }
  log += F(" baud:");
  log += String(_baud);
  return log;
}
