#include <Arduino.h>
#ifdef ESP8266
#include <IRSender.h>

// Send IR using the 'bit banging' on ESP8266 using alternative ESP.getCycleCount() method

// Cycles compensation due to while cycles and polling ESP.getCycleCount()
#define ESP8266_CYCLES_COMPENSATION 33

IRSenderESP8266Alt::IRSenderESP8266Alt(uint8_t pin) : IRSender(pin)
{
  pinMode(_pin, OUTPUT);
}


void IRSenderESP8266Alt::setFrequency(int frequency)
{
  // Enables IR output.  The khz value controls the modulation frequency in kilohertz.
  _halfPeriodicTime = F_CPU / (frequency * 1000 * 2); // This one stores half period time in CPU ticks
}


// Send an IR 'mark' symbol, i.e. transmitter ON
void IRSenderESP8266Alt::mark(int markLength)
{
  long beginning = micros();

  while((int)(micros() - beginning) < markLength){
    uint32_t periodStart = ESP.getCycleCount();
    digitalWrite(_pin, HIGH);
    while (ESP.getCycleCount() - periodStart < _halfPeriodicTime - ESP8266_CYCLES_COMPENSATION);

    periodStart = ESP.getCycleCount();
    digitalWrite(_pin, LOW);
    while (ESP.getCycleCount() - periodStart < _halfPeriodicTime - ESP8266_CYCLES_COMPENSATION);
  }
}


// Send an IR 'space' symbol, i.e. transmitter OFF
void IRSenderESP8266Alt::space(int spaceLength)
{
  digitalWrite(_pin, LOW);

  if (spaceLength < 16383) {
    delayMicroseconds(spaceLength);
  } else {
    delay(spaceLength/1000);
  }
}
#endif
