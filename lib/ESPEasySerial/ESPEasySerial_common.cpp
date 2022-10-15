#include <ESPeasySerial.h>

#include <deque>

std::deque<ESPeasySerial *> _ESPeasySerial_instances;
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
  _preferredSerialtype(port),
  _receivePin(receivePin),
  _transmitPin(transmitPin),
  _inverse_logic(inverse_logic),
  _buffSize(buffSize)
{
  register_instance();
  resetConfig(port, receivePin, transmitPin, inverse_logic, buffSize, forceSWserial);
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
  _preferredSerialtype(port),
  _receivePin(receivePin),
  _transmitPin(transmitPin),
  _inverse_logic(inverse_logic),
  _buffSize(buffSize)
{
  register_instance();
  resetConfig(port, receivePin, transmitPin, inverse_logic, buffSize);
}

#endif // ifdef ESP32

ESPeasySerial::~ESPeasySerial() {
  deregister_instance();
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

void ESPeasySerial::update_instance()
{
  ESPeasySerial *conflict = find_conflicting_ESPeasySerial(_serialtype, _receivePin, _transmitPin);

  if (conflict != nullptr) {
    // This is not the lower priority one, so let the other resolve the conflict
    if (!_isLowerPriority) {
//      conflict->resolveConflict();
    }
  }
}

void ESPeasySerial::register_instance()
{
  ESPeasySerial *ptr = dynamic_cast<ESPeasySerial *>(this);

  _ESPeasySerial_instances.push_back(ptr);
}

void ESPeasySerial::deregister_instance()
{
  // Check to see if there was a conflicting instance
  ESPeasySerial *conflict = find_conflicting_ESPeasySerial(_serialtype, _receivePin, _transmitPin);

  if (conflict != nullptr) {
    if (!conflict->isLowerPriority()) {
      // No need to let it resolve the conflict.
      conflict = nullptr;
    }
  }

  for (auto it = _ESPeasySerial_instances.begin(); it != _ESPeasySerial_instances.end(); ++it) {
    ESPeasySerial *ptr      = dynamic_cast<ESPeasySerial *>(this);
    ESPeasySerial *instance = dynamic_cast<ESPeasySerial *>(*it);

    if (instance == ptr) {
      _ESPeasySerial_instances.erase(it);

      if (conflict != nullptr) {
        conflict->resolveConflict();
      }
      return;
    }
  }
}

void ESPeasySerial::resolveConflict()
{
  if (isLowerPriority()) {
    resetConfig(_preferredSerialtype, _receivePin, _transmitPin, _inverse_logic, _buffSize
   #ifdef ESP8266
                , _forceSWserial
   #endif // ifdef ESP8266
                );
  }
}

ESPeasySerial * ESPeasySerial::find_conflicting_ESPeasySerial(ESPEasySerialPort port, int rx, int tx)
{
  for (auto it = _ESPeasySerial_instances.begin(); it != _ESPeasySerial_instances.end(); ++it) {
    if (*it != nullptr) {
      ESPeasySerial *instance = dynamic_cast<ESPeasySerial *>(*it);
      //ESPeasySerial *instance = nullptr;
      ESPeasySerial *ptr      = dynamic_cast<ESPeasySerial *>(this);

      if ((instance != nullptr) && (instance != ptr)) {
//        const ESPEasySerialPort porttype = instance->getSerialPortType();
        switch (instance->getSerialPortType()) {
          case ESPEasySerialPort::serial0:
          case ESPEasySerialPort::serial0_swap:

            if ((port == ESPEasySerialPort::serial0) ||
                (port == ESPEasySerialPort::serial0_swap)) {
              return instance;
            }
            break;
          case ESPEasySerialPort::software:
            // Must try to match on pins
          {
            const int _rx = instance->getRxPin();
            const int _tx = instance->getTxPin();

            if ((_rx != -1) && ((_rx == rx) || (_rx == tx))) { return instance; }

            if ((_tx != -1) && ((_tx == rx) || (_tx == tx))) { return instance; }
            break;
          }
          case ESPEasySerialPort::sc16is752:

            // Must try to match on address and channel
            if ((instance->getRxPin() == rx) && (instance->getTxPin() == tx)) {
              return instance;
            }
            break;
          default:

            if (instance->getSerialPortType() == port) {
              return instance;
            }
            break;
        }
      }
    }
  }
  return nullptr;
}
