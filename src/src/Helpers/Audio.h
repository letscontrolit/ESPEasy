#ifndef HELPERS_AUDIO_H
#define HELPERS_AUDIO_H

#include <Arduino.h>

#ifdef ESP32

// MFD: adding tone support here while waiting for the Arduino Espressif implementation to catch up
// As recomandation is not to use external libraries the following code was taken from: https://github.com/lbernstone/Tone Thanks
  # define TONE_CHANNEL 15

void noToneESP32(uint8_t pin,
                 uint8_t channel = TONE_CHANNEL);

void toneESP32(uint8_t       pin,
               unsigned int  frequency,
               unsigned long duration,
               uint8_t       channel = TONE_CHANNEL);

#endif // ifdef ESP32



/********************************************************************************************\
   Generate a tone of specified frequency on pin
 \*********************************************************************************************/
void tone_espEasy(uint8_t       _pin,
                  unsigned int  frequency,
                  unsigned long duration);

/********************************************************************************************\
   Play RTTTL string on specified pin
 \*********************************************************************************************/
void play_rtttl(uint8_t     _pin,
                const char *p);


#endif