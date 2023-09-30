// ---------------------------------------------------------------------------
// AUTHOR/LICENSE:
//  The following code was written by Antoine Beauchamp. For other authors, see AUTHORS file.
//  The code & updates for the library can be found at https://github.com/end2endzone/AnyRtttl
//  MIT License: http://www.opensource.org/licenses/mit-license.php
// ---------------------------------------------------------------------------
#include "Arduino.h"
#include "anyrtttl.h"
#include "binrtttl.h"

/*********************************************************
 * RTTTL Library data
 *********************************************************/

namespace anyrtttl
{

const uint16_t notes[] = { NOTE_SILENT,
NOTE_C4, NOTE_CS4, NOTE_D4, NOTE_DS4, NOTE_E4, NOTE_F4, NOTE_FS4, NOTE_G4, NOTE_GS4, NOTE_A4, NOTE_AS4, NOTE_B4,
NOTE_C5, NOTE_CS5, NOTE_D5, NOTE_DS5, NOTE_E5, NOTE_F5, NOTE_FS5, NOTE_G5, NOTE_GS5, NOTE_A5, NOTE_AS5, NOTE_B5,
NOTE_C6, NOTE_CS6, NOTE_D6, NOTE_DS6, NOTE_E6, NOTE_F6, NOTE_FS6, NOTE_G6, NOTE_GS6, NOTE_A6, NOTE_AS6, NOTE_B6,
NOTE_C7, NOTE_CS7, NOTE_D7, NOTE_DS7, NOTE_E7, NOTE_F7, NOTE_FS7, NOTE_G7, NOTE_GS7, NOTE_A7, NOTE_AS7, NOTE_B7
};

#define isdigit(n) (n >= '0' && n <= '9')
typedef uint16_t TONE_DURATION;
static const byte NOTES_PER_OCTAVE = 12;

const char * buffer = "";
ReadCharFuncPtr readCharFunc = &readChar;
int bufferIndex = -32760;
byte default_dur = 4;
byte default_oct = 5;
RTTTL_BPM bpm = 63;
RTTTL_DURATION wholenote;
byte pin = -1;
unsigned long delayToNextNote = 0; //milliseconds before playing the next note
bool playing = false;
TONE_DURATION duration;
byte noteOffset;
RTTTL_OCTAVE_VALUE scale;
int tmpNumber;

const char * readNumber(const char * iBuffer, int & oValue, ReadCharFuncPtr iReadCharFunc)
{
  oValue = 0;
  while(isdigit(iReadCharFunc(iBuffer)))
  {
    oValue = (oValue * 10) + (readCharFunc(iBuffer) - '0');
    iBuffer++;
  }
  return iBuffer;
}

void serialPrint(const char * iBuffer, ReadCharFuncPtr iReadCharFunc)
{
  char c = readCharFunc(iBuffer);
  while(c) {
    Serial.print(c);
    iBuffer++;
    c = readCharFunc(iBuffer);
  }
}

/****************************************************************************
 * Custom functions
 ****************************************************************************/

ToneFuncPtr _tone = &tone;
NoToneFuncPtr _noTone = &noTone;
DelayFuncPtr _delay = &delay;
MillisFuncPtr _millis = &millis;

void setToneFunction(ToneFuncPtr iFunc) {
  _tone = iFunc;
}

void setNoToneFunction(NoToneFuncPtr iFunc) {
  _noTone = iFunc;
}

void setDelayFunction(DelayFuncPtr iFunc) {
  _delay = iFunc;
}

void setMillisFunction(MillisFuncPtr iFunc) {
  _millis = iFunc;
}

char readChar(const char * iBuffer) {
  return *iBuffer;
}

char readChar_P(const char * iBuffer) {
  return pgm_read_byte_near(iBuffer);
}



/****************************************************************************
 * Blocking API
 ****************************************************************************/
namespace blocking
{

void play(byte iPin, const char * iBuffer, ReadCharFuncPtr iReadCharFunc) {
  // Absolutely no error checking in here

  default_dur = 4;
  default_oct = 6;
  bpm = 63;
  buffer = iBuffer;
  readCharFunc = iReadCharFunc;
  
  #ifdef ANY_RTTTL_DEBUG
  Serial.print("playing: ");
  serialPrint(buffer, readCharFunc);
  Serial.println();
  #endif

  // format: d=N,o=N,b=NNN:
  // find the start (skip name, etc)

  while(readCharFunc(buffer) != ':') buffer++; // ignore name
  buffer++;                        // skip ':'

  // get default duration
  if(readCharFunc(buffer) == 'd')
  {
    buffer++; buffer++;           // skip "d="
    buffer = readNumber(buffer, tmpNumber, readCharFunc);
    if(tmpNumber > 0)
      default_dur = tmpNumber;
    buffer++;                      // skip comma
  }

  #ifdef ANY_RTTTL_INFO
  Serial.print("ddur: "); Serial.println(default_dur, 10);
  #endif

  // get default octave
  if(readCharFunc(buffer) == 'o')
  {
    buffer++; buffer++;           // skip "o="
    buffer = readNumber(buffer, tmpNumber, readCharFunc);
    if(tmpNumber >= 3 && tmpNumber <= 7)
      default_oct = tmpNumber;
    buffer++;                      // skip comma
  }

  #ifdef ANY_RTTTL_INFO
  Serial.print("doct: "); Serial.println(default_oct, 10);
  #endif

  // get BPM
  if(readCharFunc(buffer) == 'b')
  {
    buffer++; buffer++;         // skip "b="
    buffer = readNumber(buffer, tmpNumber, readCharFunc);
    bpm = tmpNumber;
    buffer++;                    // skip colon
  }

  #ifdef ANY_RTTTL_INFO
  Serial.print("bpm: "); Serial.println(bpm, 10);
  #endif

  // BPM usually expresses the number of quarter notes per minute
  wholenote = (60 * 1000L / bpm) * 4;  // this is the time for whole noteOffset (in milliseconds)

  #ifdef ANY_RTTTL_INFO
  Serial.print("wn: "); Serial.println(wholenote, 10);
  #endif

  // now begin note loop
  while(readCharFunc(buffer))
  {
    // first, get note duration, if available
    buffer = readNumber(buffer, tmpNumber, readCharFunc);
    
    if(tmpNumber)
      duration = wholenote / tmpNumber;
    else
      duration = wholenote / default_dur;  // we will need to check if we are a dotted noteOffset after

    // now get the note
    noteOffset = getNoteOffsetFromLetter(readCharFunc(buffer));
    buffer++;

    // now, get optional '#' sharp
    if(readCharFunc(buffer) == '#')
    {
      noteOffset++;
      buffer++;
    }

    // now, get optional '.' dotted note
    if(readCharFunc(buffer) == '.')
    {
      duration += duration/2;
      buffer++;
    }
  
    // now, get scale
    if(isdigit(readCharFunc(buffer)))
    {
      scale = readCharFunc(buffer) - '0';
      buffer++;
    }
    else
    {
      scale = default_oct;
    }

    if(readCharFunc(buffer) == ',')
      buffer++;       // skip comma for next note (or we may be at the end)

    // now play the note
    if(noteOffset)
    {
      uint16_t frequency = notes[(scale - 4) * NOTES_PER_OCTAVE + noteOffset];

      #ifdef ANY_RTTTL_INFO
      Serial.print("Playing: ");
      Serial.print(scale, 10); Serial.print(' ');
      Serial.print(noteOffset, 10); Serial.print(" (");
      Serial.print(frequency, 10);
      Serial.print(") ");
      Serial.println(duration, 10);
      #endif

      _tone(iPin, frequency, duration);
      _delay(duration+1);
      _noTone(iPin);
    }
    else
    {
      #ifdef ANY_RTTTL_INFO
      Serial.print("Pausing: ");
      Serial.println(duration, 10);
      #endif
      _delay(duration);
    }
  }
}

void play(byte iPin, const char * iBuffer)              { play(iPin, iBuffer, &readChar); }

void play(byte iPin, const __FlashStringHelper* str)    { play(iPin, (const char *)str, &readChar_P); }
void playProgMem(byte iPin, const char * iBuffer)       { play(iPin, iBuffer, &readChar_P); }
void play_P(byte iPin, const char * iBuffer)            { play(iPin, iBuffer, &readChar_P); }
void play_P(byte iPin, const __FlashStringHelper* str)  { play(iPin, (const char *)str, &readChar_P); }

void play16Bits(int iPin, const unsigned char * iBuffer, int iNumNotes) {
  // Absolutely no error checking in here

  RTTTL_DEFAULT_VALUE_SECTION * defaultSection = (RTTTL_DEFAULT_VALUE_SECTION *)iBuffer;
  RTTTL_NOTE * notesBuffer = (RTTTL_NOTE *)iBuffer;

  bpm = defaultSection->bpm;

  #ifdef ANY_RTTTL_DEBUG
  Serial.print("numNotes=");
  Serial.println(iNumNotes);
  // format: d=N,o=N,b=NNN:
  Serial.print("d=");
  Serial.print(getNoteDurationFromIndex(defaultSection->durationIdx));
  Serial.print(",o=");
  Serial.print(getNoteOctaveFromIndex(defaultSection->octaveIdx));
  Serial.print(",b=");
  Serial.println(bpm);
  #endif
  
  // BPM usually expresses the number of quarter notes per minute
  wholenote = (60 * 1000L / bpm) * 4;  // this is the time for whole noteOffset (in milliseconds)

  // now begin note loop
  for(int i=0; i<iNumNotes; i++) {
    const RTTTL_NOTE & n = notesBuffer[i+1]; //offset by 16 bits for RTTTL_DEFAULT_VALUE_SECTION

    // first, get note duration, if available
    duration = wholenote / getNoteDurationFromIndex(n.durationIdx);

    // now get the note
    //noteOffset = noteOffsets[n.noteIdx];
    noteOffset = getNoteOffsetFromLetterIndex(n.noteIdx);

    // now, get optional '#' sharp
    if(n.pound)
    {
      noteOffset++;
    }

    // now, get optional '.' dotted note
    if(n.dotted)
    {
      duration += duration/2;
    }

    // now, get scale
    scale = getNoteOctaveFromIndex(n.octaveIdx);

    if(noteOffset)
    {
      #ifdef ANY_RTTTL_DEBUG
      Serial.print(getNoteDurationFromIndex(n.durationIdx));
      static const char noteCharacterValues[] =   {'c','d','e','f','g','a','b','p'};
      Serial.print(noteCharacterValues[n.noteIdx]);
      Serial.print( (n.pound ? "#" : "") );
      Serial.print( (n.dotted ? "." : "") );
      Serial.println(getNoteOctaveFromIndex(n.octaveIdx));
      #endif
      
      uint16_t frequency = notes[(scale - 4) * NOTES_PER_OCTAVE + noteOffset];

      _tone(iPin, frequency, duration);
      _delay(duration+1);
      _noTone(iPin);
    }
    else
    {
      #ifdef ANY_RTTTL_DEBUG
      Serial.print(getNoteDurationFromIndex(n.durationIdx));
      static const char noteCharacterValues[] =   {'c','d','e','f','g','a','b','p'};
      Serial.print(noteCharacterValues[n.noteIdx]);
      Serial.print( (n.pound ? "#" : "") );
      Serial.print( (n.dotted ? "." : "") );
      Serial.println();
      #endif

      _delay(duration);
    }
  }
}

void play10Bits(int iPin, int iNumNotes, BitReadFuncPtr iFuncPtr) {
  // Absolutely no error checking in here

  //read default section
  RTTTL_DEFAULT_VALUE_SECTION defaultSection;
  defaultSection.raw = iFuncPtr(16);

  bpm = defaultSection.bpm;

  #ifdef ANY_RTTTL_DEBUG
  Serial.print("numNotes=");
  Serial.println(iNumNotes);
  // format: d=N,o=N,b=NNN:
  Serial.print("d=");
  Serial.print(getNoteDurationFromIndex(defaultSection.durationIdx));
  Serial.print(",o=");
  Serial.print(getNoteOctaveFromIndex(defaultSection.octaveIdx));
  Serial.print(",b=");
  Serial.println(bpm);
  #endif
  
  // BPM usually expresses the number of quarter notes per minute
  wholenote = (60 * 1000L / bpm) * 4;  // this is the time for whole noteOffset (in milliseconds)

  // now begin note loop
  for(int i=0; i<iNumNotes; i++) {
    RTTTL_NOTE n;
    n.raw = iFuncPtr(10);

    // first, get note duration, if available
    duration = wholenote / getNoteDurationFromIndex(n.durationIdx);

    // now get the note
    noteOffset = getNoteOffsetFromLetterIndex(n.noteIdx);

    // now, get optional '#' sharp
    if(n.pound)
    {
      noteOffset++;
    }

    // now, get optional '.' dotted note
    if(n.dotted)
    {
      duration += duration/2;
    }

    // now, get scale
    scale = getNoteOctaveFromIndex(n.octaveIdx);

    if(noteOffset)
    {
      #ifdef ANY_RTTTL_DEBUG
      Serial.print(getNoteDurationFromIndex(n.durationIdx));
      static const char noteCharacterValues[] =   {'c','d','e','f','g','a','b','p'};
      Serial.print(noteCharacterValues[n.noteIdx]);
      Serial.print( (n.pound ? "#" : "") );
      Serial.print( (n.dotted ? "." : "") );
      Serial.println(getNoteOctaveFromIndex(n.octaveIdx));
      #endif
      
      uint16_t frequency = notes[(scale - 4) * 12 + noteOffset];
      _tone(iPin, frequency, duration);
      _delay(duration+1);
      _noTone(iPin);
    }
    else
    {
      #ifdef ANY_RTTTL_DEBUG
      Serial.print(getNoteDurationFromIndex(n.durationIdx));
      static const char noteCharacterValues[] =   {'c','d','e','f','g','a','b','p'};
      Serial.print(noteCharacterValues[n.noteIdx]);
      Serial.print( (n.pound ? "#" : "") );
      Serial.print( (n.dotted ? "." : "") );
      Serial.println();
      #endif

      _delay(duration);
    }
  }
}



}; //blocking namespace


/****************************************************************************
 * Non-blocking API
 ****************************************************************************/
namespace nonblocking
{


//pre-declaration
void nextnote();

void begin(byte iPin, const char * iBuffer, ReadCharFuncPtr iReadCharFunc)
{
  //init values
  pin = iPin;
  buffer = iBuffer;
  bufferIndex = 0;
  default_dur = 4;
  default_oct = 6;
  bpm=63;
  playing = true;
  delayToNextNote = 0;
  readCharFunc = iReadCharFunc;
  
  #ifdef ANY_RTTTL_DEBUG
  Serial.print("playing: ");
  serialPrint(buffer, readCharFunc);
  Serial.println();
  #endif

  //stop current note
  noTone(pin);

  // format: d=N,o=N,b=NNN:
  // find the start (skip name, etc)

  //read buffer until first note
  while(readCharFunc(buffer) != ':') buffer++;     // ignore name
  buffer++;                           // skip ':'

  // get default duration
  if(readCharFunc(buffer) == 'd')
  {
    buffer++; buffer++;               // skip "d="
    buffer = readNumber(buffer, tmpNumber, readCharFunc);
    if(tmpNumber > 0)
      default_dur = tmpNumber;
    buffer++;                         // skip comma
  }

  #ifdef ANY_RTTTL_INFO
  Serial.print("ddur: "); Serial.println(default_dur, 10);
  #endif
  
  // get default octave
  if(readCharFunc(buffer) == 'o')
  {
    buffer++; buffer++;               // skip "o="
    buffer = readNumber(buffer, tmpNumber, readCharFunc);
    if(tmpNumber >= 3 && tmpNumber <= 7)
      default_oct = tmpNumber;
    buffer++;                         // skip comma
  }

  #ifdef ANY_RTTTL_INFO
  Serial.print("doct: "); Serial.println(default_oct, 10);
  #endif
  
  // get BPM
  if(readCharFunc(buffer) == 'b')
  {
    buffer++; buffer++;              // skip "b="
    buffer = readNumber(buffer, tmpNumber, readCharFunc);
    bpm = tmpNumber;
    buffer++;                   // skip colon
  }

  #ifdef ANY_RTTTL_INFO
  Serial.print("bpm: "); Serial.println(bpm, 10);
  #endif

  // BPM usually expresses the number of quarter notes per minute
  wholenote = (60 * 1000L / bpm) * 4;  // this is the time for whole noteOffset (in milliseconds)

  #ifdef ANY_RTTTL_INFO
  Serial.print("wn: "); Serial.println(wholenote, 10);
  #endif
}

void begin(byte iPin, const char * iBuffer)             { begin(iPin, iBuffer, &readChar); }

void begin(byte iPin, const __FlashStringHelper* str)   { begin(iPin, (const char *)str, &readChar_P); }
void beginProgMem(byte iPin, const char * iBuffer)      { begin(iPin, iBuffer, &readChar_P); }
void begin_P(byte iPin, const char * iBuffer)           { begin(iPin, iBuffer, &readChar_P); }
void begin_P(byte iPin, const __FlashStringHelper* str) { begin(iPin, (const char *)str, &readChar_P); }

void nextnote()
{
  //stop current note
  _noTone(pin);

  // first, get note duration, if available
  buffer = readNumber(buffer, tmpNumber, readCharFunc);
  
  if(tmpNumber)
    duration = wholenote / tmpNumber;
  else
    duration = wholenote / default_dur;  // we will need to check if we are a dotted noteOffset after

  // now get the note
  noteOffset = getNoteOffsetFromLetter(readCharFunc(buffer));
  buffer++;

  // now, get optional '#' sharp
  if(readCharFunc(buffer) == '#')
  {
    noteOffset++;
    buffer++;
  }

  // now, get optional '.' dotted note
  if(readCharFunc(buffer) == '.')
  {
    duration += duration/2;
    buffer++;
  }

  // now, get scale
  if(isdigit(readCharFunc(buffer)))
  {
    scale = readCharFunc(buffer) - '0';
    buffer++;
  }
  else
  {
    scale = default_oct;
  }

  if(readCharFunc(buffer) == ',')
    buffer++;       // skip comma for next note (or we may be at the end)

  // now play the note
  if(noteOffset)
  {
    #ifdef ANY_RTTTL_INFO
    Serial.print("Playing: ");
    Serial.print(scale, 10); Serial.print(' ');
    Serial.print(noteOffset, 10); Serial.print(" (");
    Serial.print(notes[(scale - 4) * NOTES_PER_OCTAVE + noteOffset], 10);
    Serial.print(") ");
    Serial.println(duration, 10);
    #endif
    
    uint16_t frequency = notes[(scale - 4) * NOTES_PER_OCTAVE + noteOffset];
    _tone(pin, frequency, duration);
    
    delayToNextNote = _millis() + (duration+1);
  }
  else
  {
    #ifdef ANY_RTTTL_INFO
    Serial.print("Pausing: ");
    Serial.println(duration, 10);
    #endif
    
    delayToNextNote = _millis() + (duration);
  }
}

void play()
{
  //if done playing the song, return
  if (!playing)
  {
    #ifdef ANY_RTTTL_DEBUG
    Serial.println("done playing...");
    #endif
    
    return;
  }
  
  //are we still playing a note ?
  unsigned long m = _millis();
  if (m < delayToNextNote)
  {
    #ifdef ANY_RTTTL_DEBUG
    Serial.println("still playing a note...");
    #endif
    
    //wait until the note is completed
    return;
  }

  //ready to play the next note
  if (readCharFunc(buffer) == '\0')
  {
    //no more notes. Reached the end of the last note

    #ifdef ANY_RTTTL_DEBUG
    Serial.println("end of note...");
    #endif
    
    playing = false;

    //stop current note (if any)
    _noTone(pin);

    return; //end of the song
  }
  else
  {
    //more notes to play...

    #ifdef ANY_RTTTL_DEBUG
    Serial.println("next note...");
    #endif
    
    nextnote();
  }
}

void stop()
{
  if (playing)
  {
    //increase song buffer until the end
    while (readCharFunc(buffer) != '\0')
    {
      buffer++;
    }
  }

  playing = false;

  //stop current note (if any)
  _noTone(pin);
}

bool done()
{
  return !playing;
}

bool isPlaying()
{
  return playing;
}

}; //nonblocking namespace

}; //anyrtttl namespace
