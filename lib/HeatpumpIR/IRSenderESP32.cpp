#include <Arduino.h>

// IRSender implementation for ESP32
// Tested on R51M/E control with SENSEI air conditioner 
// Maksym Krasovskyi

#if defined ESP32
#include <IRSender.h>
#if ( defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3) )
#include <driver/gpio.h>
#endif  // ESP_ARDUINO_VERSION_MAJOR >= 3
IRSenderESP32::IRSenderESP32(uint8_t pin, uint8_t pwmChannel) : IRSender(pin)
{
  _pwmChannel = pwmChannel;
  pinMode(_pin, OUTPUT);
  // If we have an inverted signal, we need to set the pin from default LOW
  // to HIGH to make it off
  if (_inverted)
    digitalWrite(_pin, HIGH);
}

void IRSenderESP32::setFrequency(int frequency)
{
  _frequency = frequency * 1000;
}

// Send an IR 'mark' symbol, i.e. transmitter ON
void IRSenderESP32::mark(int markLength)
{
#if ( defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3) )
  ledcAttach(_pin, _frequency, 8);
#else   // ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcSetup(_pwmChannel, _frequency, 8);
  ledcAttachPin(_pin, _pwmChannel);
#endif  // ESP_ARDUINO_VERSION_MAJOR >= 3
  long beginning = micros();
#if ( defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3) )
  ledcWrite(_pin, 127);
#else   // ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(_pwmChannel, 127);
#endif  // ESP_ARDUINO_VERSION_MAJOR >= 3
  while((int)(micros() - beginning) < markLength);
#if ( defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3) )
  ledcDetach(_pin);
  pinMode(_pin, OUTPUT);
#else   // ESP_ARDUINO_VERSION_MAJOR >= 3
  gpio_reset_pin(static_cast<gpio_num_t>(_pin));
#endif  // ESP_ARDUINO_VERSION_MAJOR >= 3
  if (_inverted)
    digitalWrite(_pin, HIGH);
  else
    digitalWrite(_pin, LOW);
}

// Send an IR 'space' symbol, i.e. transmitter OFF
void IRSenderESP32::space(int spaceLength)
{
  if (_inverted)
    digitalWrite(_pin, HIGH);
  else
    digitalWrite(_pin, LOW);

  if (spaceLength < 16383) {
    delayMicroseconds(spaceLength);
  } else {
    delay(spaceLength/1000);
  }
}

#endif