#include "Port_ESPEasySerial_base.h"

Port_ESPEasySerial_base::Port_ESPEasySerial_base() {}

Port_ESPEasySerial_base::~Port_ESPEasySerial_base() {}

int Port_ESPEasySerial_base::getBaudRate() const
{
  return _config.baud;
}
