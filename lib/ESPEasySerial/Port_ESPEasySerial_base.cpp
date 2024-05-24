#include "Port_ESPEasySerial_base.h"

Port_ESPEasySerial_base::Port_ESPEasySerial_base() {}

Port_ESPEasySerial_base::~Port_ESPEasySerial_base() {}

int Port_ESPEasySerial_base::getBaudRate() const
{
  return _config.baud;
}

String Port_ESPEasySerial_base::getPortDescription() const
{
  String res;

  res += ESPEasySerialPort_toString(_config.port);
  if (useGPIOpins(_config.port)) {
    res += F(" RX:");
    res += _config.receivePin;
    res += F(" TX:");
    res += _config.transmitPin;
  }
  res += F(" @ ");
  res += getBaudRate();
  res += F(" baud");

  return res;
}
