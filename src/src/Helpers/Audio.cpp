#include "../Helpers/Audio.h"

#include "../ESPEasyCore/ESPEasyGPIO.h"
#include "../Globals/RamTracker.h"
#include "../Helpers/Hardware.h"


/********************************************************************************************\
   Generate a tone of specified frequency on pin
 \*********************************************************************************************/
bool tone_espEasy(int8_t _pin, unsigned int frequency, unsigned long duration) {
  if (!validGpio(_pin)) { return false; }

  // Duty cycle can be used as some kind of volume.
  if (!set_Gpio_PWM_pct(_pin, 50, frequency)) { return false; }

  if (duration > 0) {
    delay(duration);
    return set_Gpio_PWM(_pin, 0, frequency);
  }
  return true;
}

/********************************************************************************************\
   Play RTTTL string on specified pin
 \*********************************************************************************************/
#if FEATURE_RTTTL
# if FEATURE_ANYRTTTL_LIB
#  include <anyrtttl.h>
#  include <pitches.h>
#  if FEATURE_RTTTL_EVENTS
#   include "../Globals/EventQueue.h"
#   include "../Globals/Settings.h"
static bool rtttlPlaying = false;
#  endif // if FEATURE_RTTTL_EVENTS
#  if FEATURE_ANYRTTTL_ASYNC
static String rtttlMelody;

void clear_rtttl_melody() {
  // The non-blocking play will read from a char pointer.
  // So we must stop the playing before changing the string as it could otherwise lead to a crash.
  if (anyrtttl::nonblocking::isPlaying()) { // If currently playing, cancel that
    addLog(LOG_LEVEL_INFO, F("RTTTL: Cancelling running song..."));
    anyrtttl::nonblocking::stop();
    #   if FEATURE_RTTTL_EVENTS

    if (Settings.UseRules) {
      eventQueue.add(F("RTTTL#Cancelled"));
    }
    rtttlPlaying = false;
    #   endif // if FEATURE_RTTTL_EVENTS
  }

  rtttlMelody = String();
}

void set_rtttl_melody(String& melody) {
  clear_rtttl_melody();
  rtttlMelody = melody;
}

#  endif // if FEATURE_ANYRTTTL_ASYNC


bool play_rtttl(int8_t _pin, const char *p) {
  if (!validGpio(_pin)) { return false; }

  // addLog(LOG_LEVEL_INFO, F("RTTTL: Using AnyRtttl"));

  #  ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("play_rtttl"));
  #  endif // ifndef BUILD_NO_RAM_TRACKER

  anyrtttl::setNoToneFunction(&setInternalGPIOPullupMode);
  #  if FEATURE_ANYRTTTL_ASYNC

  if (!rtttlMelody.isEmpty()) {
    anyrtttl::nonblocking::begin(_pin, rtttlMelody.c_str());
  } else {
    anyrtttl::nonblocking::begin(_pin, p);
  }
  anyrtttl::nonblocking::play();
  #   if FEATURE_RTTTL_EVENTS

  if (Settings.UseRules) {
    eventQueue.add(F("RTTTL#Started"));
  }
  rtttlPlaying = true;
  #   endif // if FEATURE_RTTTL_EVENTS
  #  else // if FEATURE_ANYRTTTL_ASYNC
  anyrtttl::blocking::play(_pin, p);
  #  endif // if FEATURE_ANYRTTTL_ASYNC
  #  ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("play_rtttl2"));
  #  endif // ifndef BUILD_NO_RAM_TRACKER
  return true;
}

#  if FEATURE_ANYRTTTL_ASYNC
void update_rtttl() {
  if (anyrtttl::nonblocking::isPlaying()) {
    anyrtttl::nonblocking::play();
  } else {
    #   if FEATURE_RTTTL_EVENTS

    if (rtttlPlaying) {
      if (Settings.UseRules) {
        eventQueue.add(F("RTTTL#Finished"));
      }
      rtttlPlaying = false;
    }
    #   endif // if FEATURE_RTTTL_EVENTS
    clear_rtttl_melody(); // Release memory
  }
}

#  endif // if FEATURE_ANYRTTTL_ASYNC

# else // if FEATURE_ANYRTTTL_LIB
bool play_rtttl(int8_t _pin, const char *p)
{
  if (!validGpio(_pin)) { return false; }

  #  ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("play_rtttl"));
  #  endif // ifndef BUILD_NO_RAM_TRACKER
  #  define OCTAVE_OFFSET 0

  // FIXME: Absolutely no error checking in here

  const int notes[] = { 0,
                        262, 277,   294,   311,   330,  349,   370,  392,  415,  440,  466,  494,
                        523, 554,   587,   622,   659,  698,   740,  784,  831,  880,  932,  988,
                        1047,1109,  1175,  1245,  1319, 1397,  1480, 1568, 1661, 1760, 1865, 1976,
                        2093,2217,  2349,  2489,  2637, 2794,  2960, 3136, 3322, 3520, 3729, 3951
  };


  uint8_t default_dur = 4;
  uint8_t default_oct = 6;
  int     bpm         = 63;
  int     num;
  long    wholenote;
  long    duration;
  uint8_t note;
  uint8_t scale;

  // format: d=N,o=N,b=NNN:
  // find the start (skip name, etc)

  while (*p != ':') {
    p++; // ignore name

    if (*p == 0) { return false; }
  }
  p++; // skip ':'

  // get default duration
  if (*p == 'd')
  {
    p++; p++; // skip "d="
    num = 0;

    while (isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }

    if (num > 0) { default_dur = num; }
    p++; // skip comma
  }

  // get default octave
  if (*p == 'o')
  {
    p++; p++; // skip "o="
    num = *p++ - '0';

    if ((num >= 3) && (num <= 7)) { default_oct = num; }
    p++; // skip comma
  }

  // get BPM
  if (*p == 'b')
  {
    p++; p++; // skip "b="
    num = 0;

    while (isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }
    bpm = num;
    p++; // skip colon
  }

  // BPM usually expresses the number of quarter notes per minute
  wholenote = (60 * 1000L / bpm) * 4; // this is the time for whole note (in milliseconds)

  // now begin note loop
  while (*p)
  {
    // first, get note duration, if available
    num = 0;

    while (isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }

    if (num) { duration = wholenote / num; }
    else { duration = wholenote / default_dur; // we will need to check if we are a dotted note after
    }

    // now get the note
    switch (*p)
    {
      case 'c':
        note = 1;
        break;
      case 'd':
        note = 3;
        break;
      case 'e':
        note = 5;
        break;
      case 'f':
        note = 6;
        break;
      case 'g':
        note = 8;
        break;
      case 'a':
        note = 10;
        break;
      case 'b':
        note = 12;
        break;
      case 'p':
      default:
        note = 0;
    }
    p++;

    // now, get optional '#' sharp
    if (*p == '#')
    {
      note++;
      p++;
    }

    // now, get optional '.' dotted note
    if (*p == '.')
    {
      duration += duration / 2;
      p++;
    }

    // now, get scale
    if (isdigit(*p))
    {
      scale = *p - '0';
      p++;
    }
    else
    {
      scale = default_oct;
    }

    scale += OCTAVE_OFFSET;

    if (*p == ',') {
      p++; // skip comma for next note (or we may be at the end)
    }

    // now play the note
    if (note)
    {
      if (!tone_espEasy(_pin, notes[(scale - 4) * 12 + note], duration)) {
        return false;
      }
    }
    else
    {
      delay(duration / 10);
    }
  }
  setInternalGPIOPullupMode(_pin); // Turn off sound, Arduino _noTone() doesn't do that reliably
  #  ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("play_rtttl2"));
  #  endif // ifndef BUILD_NO_RAM_TRACKER
  return true;
}

# endif // if FEATURE_ANYRTTTL_LIB
#endif // if FEATURE_RTTTL
