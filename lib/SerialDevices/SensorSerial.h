#ifndef _SENSORSERIAL_H_
#define _SENSORSERIAL_H_

#include "Arduino.h"
#include <SoftwareSerial.h>

class SensorSerial : public SoftwareSerial
{
public:
  SensorSerial(int receivePin, int transmitPin = -1, bool inverse_logic = false, unsigned int buffSize = 64);

  void begin(long speed);

  int peek();

  virtual size_t write(uint8_t byte);

  virtual int read();

  virtual int available();

  virtual void flush();

protected:
  boolean _hw;
};

#endif
