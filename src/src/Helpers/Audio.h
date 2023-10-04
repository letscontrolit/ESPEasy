#ifndef HELPERS_AUDIO_H
#define HELPERS_AUDIO_H


#include "../../ESPEasy_common.h"

/********************************************************************************************\
   Generate a tone of specified frequency on pin
 \*********************************************************************************************/
bool tone_espEasy(int8_t        _pin,
                  unsigned int  frequency,
                  unsigned long duration);

/********************************************************************************************\
   Play RTTTL string on specified pin
 \*********************************************************************************************/
#if FEATURE_RTTTL
bool play_rtttl(int8_t      _pin,
                const char *p);
# if FEATURE_ANYRTTTL_LIB && FEATURE_ANYRTTTL_ASYNC
void update_rtttl();
void clear_rtttl_melody();
void set_rtttl_melody(String& melody);
# endif // if FEATURE_ANYRTTTL_LIB && FEATURE_ANYRTTTL_ASYNC
#endif // if FEATURE_RTTTL


#endif // ifndef HELPERS_AUDIO_H
