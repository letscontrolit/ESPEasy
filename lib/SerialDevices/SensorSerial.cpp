#include <SensorSerial.h>


SensorSerial::SensorSerial(int receivePin, int transmitPin, bool inverse_logic, unsigned int buffSize) : SoftwareSerial(receivePin, transmitPin, inverse_logic, buffSize)
{
  //boolean sws = isValidGPIOpin(receivePin);
  _hw = receivePin < 0;
}

void SensorSerial::begin(long speed)
{
  if (_hw)
    Serial.begin(speed);
  else
    SoftwareSerial::begin(speed);
}

int SensorSerial::peek()
{
  if (_hw)
    return Serial.peek();
  else
    return SoftwareSerial::peek();
}

size_t SensorSerial::write(uint8_t byte)
{
  if (_hw)
    return Serial.write(byte);
  else
    return SoftwareSerial::write(byte);
}

int SensorSerial::read()
{
  if (_hw)
    return Serial.read();
  else
    return SoftwareSerial::read();
}

int SensorSerial::available()
{
  if (_hw)
    return Serial.available();
  else
    return SoftwareSerial::available();
}

void SensorSerial::flush()
{
  if (_hw)
    Serial.flush();
  else
    SoftwareSerial::flush();
}
