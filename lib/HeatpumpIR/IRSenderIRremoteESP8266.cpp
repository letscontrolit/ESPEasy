#include <Arduino.h>
#ifdef ESP8266
#include <IRSender.h>

IRSenderIRremoteESP8266::IRSenderIRremoteESP8266(uint8_t pin) : IRSender(pin), _ir(pin) 
{
  _ir.begin();
}

void IRSenderIRremoteESP8266::setFrequency(int frequency)
{
  _ir.enableIROut(frequency);
}

void IRSenderIRremoteESP8266::space(int spaceLength)
{
  _ir.space(spaceLength);
}

void IRSenderIRremoteESP8266::mark(int markLength)
{
  _ir.mark(markLength);
}
#endif
