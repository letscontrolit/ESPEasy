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
  :
  _receivePin(receivePin),
  _transmitPin(transmitPin),
  _inverse_logic(inverse_logic),
  _buffSize(buffSize),
#if USES_SW_SERIAL
  _forceSWserial(false)
#else
  _forceSWserial(forceSWserial)
#endif
{
  _serialtype =  ESPeasySerialType::getSerialType(port, receivePin, transmitPin);
#if USES_SW_SERIAL
  if (forceSWserial) {
#if USES_I2C_SC16IS752
    if (_serialtype != ESPEasySerialPort::sc16is752) {
      _serialtype = ESPEasySerialPort::software;
    }
#else
    _serialtype = ESPEasySerialPort::software;
#endif
  }
#endif

  switch (_serialtype) {
#if USES_SW_SERIAL
    case ESPEasySerialPort::software:
    {
      _serialPort = new ESPEasySerial_SW_Serial(receivePin, transmitPin, inverse_logic);
      break;
    }
#endif
#if USES_I2C_SC16IS752
    case ESPEasySerialPort::sc16is752:
    {
      ESPEasySC16IS752_Serial::I2C_address addr     = static_cast<ESPEasySC16IS752_Serial::I2C_address>(receivePin);
      ESPEasySC16IS752_Serial::SC16IS752_channel ch = static_cast<ESPEasySC16IS752_Serial::SC16IS752_channel>(transmitPin);

      _serialPort = new ESPEasySerial_I2C_SC16IS752(addr, ch);
      break;
    }
#endif
#if USES_HWCDC
    case ESPEasySerialPort::usb_hw_cdc:
    {
      _serialPort = new ESPEasySerial_USB_WHCDC_t();
      break;
    }
#endif
#if USES_USBCDC
    case ESPEasySerialPort::usb_cdc_0:
    case ESPEasySerialPort::usb_cdc_1:
    {
      _serialPort = new ESPEasySerial_USBCDC_t(_serialtype);
      break;
    }
#endif
    default:

      if (isHWserial(_serialtype)) {
        _serialPort = new ESPEasySerial_HardwareSerial_t(_serialtype);
        if (_serialPort != nullptr) {
          _serialPort->resetConfig(_serialtype, receivePin, transmitPin, inverse_logic, buffSize);
        }
      }
      break;
  }
  if (_serialPort != nullptr) {
    _serialPort->begin(_baud);
  }
}



ESPeasySerial::~ESPeasySerial() {
  flush();
  end();

  if (_serialPort != nullptr) {
    delete _serialPort;
  }
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
