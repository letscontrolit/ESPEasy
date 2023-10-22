#ifndef HELPERS_HARDWARE_PWM_H
#define HELPERS_HARDWARE_PWM_H

#include "../../ESPEasy_common.h"


// ********************************************************************************
// Manage PWM state of GPIO pins.
// ********************************************************************************
#ifndef ESPEASY_PWM_DEFAULT_FREQUENCY
# ifdef ESP32
#  if ESP_IDF_VERSION_MAJOR >= 5
#   define ESPEASY_PWM_DEFAULT_FREQUENCY 1000
#  else // if ESP_IDF_VERSION_MAJOR >= 5
#   define ESPEASY_PWM_DEFAULT_FREQUENCY 0
#  endif // if ESP_IDF_VERSION_MAJOR >= 5
# endif // ifdef ESP32
# ifdef ESP8266
#  define ESPEASY_PWM_DEFAULT_FREQUENCY 0
# endif // ifdef ESP8266
#endif // ifndef ESPEASY_PWM_DEFAULT_FREQUENCY


void initAnalogWrite();

#if defined(ESP32)

int8_t   attachLedChannel(int      pin,
                          uint32_t frequency  = ESPEASY_PWM_DEFAULT_FREQUENCY,
                          uint8_t  resolution = 10);
void     detachLedChannel(int pin);
uint32_t analogWriteESP32(int      pin,
                          int      value,
                          uint32_t frequency = ESPEASY_PWM_DEFAULT_FREQUENCY);
#endif // if defined(ESP32)

// Duty cycle 0..100%
bool set_Gpio_PWM_pct(int      gpio,
                      float    dutyCycle_f,
                      uint32_t frequency = ESPEASY_PWM_DEFAULT_FREQUENCY);

bool set_Gpio_PWM(int      gpio,
                  uint32_t dutyCycle,
                  uint32_t frequency = ESPEASY_PWM_DEFAULT_FREQUENCY);
bool set_Gpio_PWM(int       gpio,
                  uint32_t  dutyCycle,
                  uint32_t  fadeDuration_ms,
                  uint32_t& frequency,
                  uint32_t& key);


#endif // ifndef HELPERS_HARDWARE_PWM_H
