#include <Arduino.h>

// IRSender implementation for ESP32
// Tested on R51M/E control with SENSEI air conditioner 
// Maksym Krasovskyi

#if defined ESP32
#include <IRSender.h>
IRSenderESP32::IRSenderESP32(uint8_t pin, uint8_t pwmChannel) : IRSender(pin)
{
  _pwmChannel = pwmChannel;
  pinMode(_pin, OUTPUT);
}

void IRSenderESP32::setFrequency(int frequency)
{
  _frequency = frequency * 1000;
}

// Send an IR 'mark' symbol, i.e. transmitter ON
void IRSenderESP32::mark(int markLength)
{
  ledcSetup(_pwmChannel, _frequency, 8);
  ledcAttachPin(_pin, _pwmChannel);
  long beginning = micros();
  ledcWrite(_pwmChannel, 127);
  while((int)(micros() - beginning) < markLength);
  gpio_reset_pin(static_cast<gpio_num_t>(_pin));
  digitalWrite(_pin, LOW);
}

// Send an IR 'space' symbol, i.e. transmitter OFF
void IRSenderESP32::space(int spaceLength)
{
  digitalWrite(_pin, LOW);

  if (spaceLength < 16383) {
    delayMicroseconds(spaceLength);
  } else {
    delay(spaceLength/1000);
  }
}

#endif