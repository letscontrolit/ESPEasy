#include "ESPEasySerialConfig.h"

#include "ESPEasySerialType.h"

void ESPEasySerialConfig::validate()
{
  port =  ESPeasySerialType::getSerialType(port, receivePin, transmitPin);
#if USES_SW_SERIAL

  if (forceSWserial) {
# if USES_I2C_SC16IS752

    if (port != ESPEasySerialPort::sc16is752) {
      port = ESPEasySerialPort::software;
    }
# else // if USES_I2C_SC16IS752
    port = ESPEasySerialPort::software;
# endif // if USES_I2C_SC16IS752
  }
#endif // if USES_SW_SERIAL
}

#ifdef ESP8266
void ESPEasySerialConfig::setPortConfig(
  unsigned long baudrate,
  SerialConfig  portconfig,
  SerialMode    portmode)
{
  // FIXME TD-er: Must also set baudrate?
  baud = baudrate;

  config = portconfig;
  mode   = portmode;
}

#endif // ifdef ESP8266

#ifdef ESP32
void ESPEasySerialConfig::setPortConfig(
  unsigned long baudrate,
  uint32_t      portconfig)
{
  // FIXME TD-er: Must also set baudrate?
  baud = baudrate;

  // Make sure the extra bit is set for the config. The config differs between ESP32 and ESP82xx
  config = portconfig | 0x8000000;
}

#endif // ifdef ESP32


String ESPEasySerialConfig::getLogString() const
{
  String log;

  log.reserve(48);
  log  = F("ESPEasy serial: ");
  log += ESPEasySerialPort_toString(port);

#if USES_I2C_SC16IS752

  if (port == ESPEasySerialPort::sc16is752) {
    log += F(": addr:");
    log += String(receivePin);
    log += F(" ch:");
    log += transmitPin == 0 ? 'A' : 'B';
  }
#endif // if USES_I2C_SC16IS752

  if (useGPIOpins(port)) {
    log += F(": rx:");
    log += String(receivePin);
    log += F(" tx:");
    log += String(transmitPin);
  }

  log += F(" baud:");
  log += String(baud);
  return log;
}

#if USES_I2C_SC16IS752

bool ESPEasySerialConfig::getI2C_SC16IS752_Parameters(
  ESPEasySC16IS752_Serial::I2C_address      & addr,
  ESPEasySC16IS752_Serial::SC16IS752_channel& ch) const
{
  if ((receivePin >= 0x48) && (receivePin <= 0x57) && ((transmitPin >= 0) && (transmitPin < 2))) {
    addr = static_cast<ESPEasySC16IS752_Serial::I2C_address>(receivePin);
    ch   = static_cast<ESPEasySC16IS752_Serial::SC16IS752_channel>(transmitPin);
    return true;
  }
  return false;
}

#endif // if USES_I2C_SC16IS752


int ESPEasySerialConfig::getReceivePin() const
{
  if (useGPIOpins(port)) { return receivePin; }
  return -1;
}

int ESPEasySerialConfig::getTransmitPin() const
{
  if (useGPIOpins(port)) { return transmitPin; }
  return -1;
}
