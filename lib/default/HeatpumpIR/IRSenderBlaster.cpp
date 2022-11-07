#include <Arduino.h>
#include <IRSender.h>

// Send IR using the IR Blaster. The IR Blaster generates the 38 kHz carrier frequency


IRSenderBlaster::IRSenderBlaster(uint8_t pin) : IRSender(pin)
{
  pinMode(_pin, OUTPUT);
}


void IRSenderBlaster::setFrequency(int frequency)
{
  (void)frequency;
}


// Send an IR 'mark' symbol, i.e. transmitter ON
void IRSenderBlaster::mark(int markLength)
{
  digitalWrite(_pin, HIGH);

  if (markLength < 16383) {
    delayMicroseconds(markLength);
  } else {
    delay(markLength/1000);
  }
}


// Send an IR 'space' symbol, i.e. transmitter OFF
void IRSenderBlaster::space(int spaceLength)
{
  digitalWrite(_pin, LOW);

  if (spaceLength < 16383) {
    delayMicroseconds(spaceLength);
  } else {
    delay(spaceLength/1000);
  }
}