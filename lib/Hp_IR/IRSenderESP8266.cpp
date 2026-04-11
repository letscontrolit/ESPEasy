#include <Arduino.h>
#ifdef ESP8266
#include <IRSender.h>
#include <core_esp8266_waveform.h>

// Send IR using the 'bit banging' with startWaveform function on ESP8266 etc.

IRSenderESP8266::IRSenderESP8266(uint8_t pin) : IRSender(pin)
{
  pinMode(_pin, OUTPUT);
}


void IRSenderESP8266::setFrequency(int frequency)
{
  // Enables IR output.  The khz value controls the modulation frequency in kilohertz.
  _halfPeriodicTime = 500/frequency; // T = 1/f but we need T/2 in microsecond and f is in kHz
}


// Send an IR 'mark' symbol, i.e. transmitter ON
void IRSenderESP8266::mark(int markLength)
{
  long beginning = micros();

  startWaveform(_pin, _halfPeriodicTime, _halfPeriodicTime, markLength);
  while((int)(micros() - beginning) < markLength);

  stopWaveform(_pin);
  digitalWrite(_pin, LOW);
}


// Send an IR 'space' symbol, i.e. transmitter OFF
void IRSenderESP8266::space(int spaceLength)
{
  digitalWrite(_pin, LOW);

  if (spaceLength < 16383) {
    delayMicroseconds(spaceLength);
  } else {
    delay(spaceLength/1000);
  }
}
#endif
