#include <Arduino.h>
#include <IRSender.h>

// Send IR using the 'bit banging' on ESP8266 etc.


IRSenderBitBang::IRSenderBitBang(uint8_t pin) : IRSender(pin)
{
  pinMode(_pin, OUTPUT);
}


void IRSenderBitBang::setFrequency(int frequency)
{
  // Enables IR output.  The khz value controls the modulation frequency in kilohertz.
  _halfPeriodicTime = 500/frequency; // T = 1/f but we need T/2 in microsecond and f is in kHz
  (void)frequency;
}


// Send an IR 'mark' symbol, i.e. transmitter ON
void IRSenderBitBang::mark(int markLength)
{
  long beginning = micros();

  while((int)(micros() - beginning) < markLength){
    digitalWrite(_pin, HIGH);
    delayMicroseconds(_halfPeriodicTime);
    digitalWrite(_pin, LOW);
    delayMicroseconds(_halfPeriodicTime); //38 kHz -> T = 26.31 microsec (periodic time), half of it is 13
  }
}


// Send an IR 'space' symbol, i.e. transmitter OFF
void IRSenderBitBang::space(int spaceLength)
{
  digitalWrite(_pin, LOW);

  if (spaceLength < 16383) {
    delayMicroseconds(spaceLength);
  } else {
    delay(spaceLength/1000);
  }
}