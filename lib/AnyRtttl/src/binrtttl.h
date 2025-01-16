// ---------------------------------------------------------------------------
// AUTHOR/LICENSE:
//  The following code was written by Antoine Beauchamp. For other authors, see AUTHORS file.
//  The code & updates for the library can be found at https://github.com/end2endzone/AnyRtttl
//  MIT License: http://www.opensource.org/licenses/mit-license.php
// ---------------------------------------------------------------------------

#ifndef BINRTTTL_H
#define BINRTTTL_H

#include "Arduino.h"

#define RTTTL_SONG_NAME_SIZE 11
#define RTTTL_NOTE_SIZE_BITS 10

namespace anyrtttl
{

typedef unsigned char DURATION_INDEX;
typedef unsigned char NOTE_LETTER_INDEX;
typedef unsigned char OCTAVE_INDEX;
typedef unsigned char BPM_INDEX;

// static DURATION_INDEX       INVALID_DURATION_INDEX        = (DURATION_INDEX)-1;
// static NOTE_LETTER_INDEX    INVALID_NOTE_LETTER_INDEX     = (NOTE_LETTER_INDEX)-1;
// static OCTAVE_INDEX         INVALID_OCTAVE_INDEX          = (OCTAVE_INDEX)-1;
// static BPM_INDEX            INVALID_BPM_INDEX             = (BPM_INDEX)-1;

typedef unsigned short RTTTL_DURATION;
typedef          char  RTTTL_NOTE_LETTER;
typedef unsigned char  RTTTL_OCTAVE_VALUE;
typedef unsigned short RTTTL_BPM;

#pragma pack(push, 1) // exact fit - no padding
union RTTTL_NOTE
{
  unsigned short raw;
  struct
  {
    DURATION_INDEX durationIdx    : 3; //ranges from 0 to 7. Matches index of getNoteDurationFromIndex()
    NOTE_LETTER_INDEX noteIdx     : 3; //ranges from 0 to 7. Matches index of getNoteLetterFromIndex()
    bool pound                    : 1; //ranges from 0 to 1. True if the note is pound
    bool dotted                   : 1; //ranges from 0 to 1. True if the duration is dotted
    OCTAVE_INDEX octaveIdx        : 2; //ranges from 0 to 3. Matches index of getNoteOctaveFromIndex()
    unsigned char padding         : 6;
  };
};

union RTTTL_DEFAULT_VALUE_SECTION
{
  unsigned short raw;
  //struct 
  //{
  //  DURATION_INDEX durationIdx :  3; //ranges from 0 to 7.  Matches index of getNoteDurationFromIndex()
  //  OCTAVE_INDEX octaveIdx     :  2; //ranges from 0 to 3.  Matches index of getNoteOctaveFromIndex()
  //  RTTTL_BPM bpm              : 10; //ranges from 0 to 900.
  //  bool               padding :  1;
  //};
  struct //aligned on 8 bits types
  {
    DURATION_INDEX durationIdx :  3; //ranges from 0 to 7.  Matches index of getNoteDurationFromIndex()
    OCTAVE_INDEX     octaveIdx :  2; //ranges from 0 to 3.  Matches index of getNoteOctaveFromIndex()
    unsigned char              :  3; //padding for bpm
    unsigned char              :  7; //padding for bpm
    unsigned char              :  1; //padding
  };
  struct //aligned on 16 bits types
  {
    unsigned short        :  5; //padding for durationIdx and octaveIdx
    RTTTL_BPM         bpm : 10; //ranges from 1 to 900.
    unsigned short        :  1; //padding
  };
};
#pragma pack(pop) //back to whatever the previous packing mode was

//RTTTL note letters
//allowed values: 'c','d','e','f','g','a','b','p'
RTTTL_NOTE_LETTER getNoteLetterFromIndex(NOTE_LETTER_INDEX iIndex);
uint16_t getNoteLettersCount();
NOTE_LETTER_INDEX findNoteLetterIndex(RTTTL_NOTE_LETTER n);

//RTTTL note offsets
//allowed values: 1, 3, 5, 6, 8, 10, 12, 0
int getNoteOffsetFromLetterIndex(NOTE_LETTER_INDEX iIndex);
int getNoteOffsetFromLetter(RTTTL_NOTE_LETTER n);

//RTTTL durations
//allowed values: 1, 2, 4, 8, 16, 32
RTTTL_DURATION getNoteDurationFromIndex(DURATION_INDEX iIndex);
uint16_t getNoteDurationsCount();
DURATION_INDEX findNoteDurationIndex(RTTTL_DURATION n);

//RTTTL octaves
//allowed values: 4, 5, 6, 7
RTTTL_OCTAVE_VALUE getNoteOctaveFromIndex(OCTAVE_INDEX iIndex);
uint16_t getNoteOctavesCount();
OCTAVE_INDEX findNoteOctaveIndex(RTTTL_OCTAVE_VALUE n);

//RTTTL BPM values
//allowed values: 25, 28, 31, 35, 40, 45, 50, 56, 63, 70, 80, 90, 100, 112, 125, 140, 160, 180, 200, 225, 250, 285, 320, 355, 400, 450, 500, 565, 635, 715, 800 and 900.
RTTTL_BPM getBpmFromIndex(BPM_INDEX iIndex);
uint16_t getBpmsCount();
// BPM_INDEX findBpmIndex(RTTTL_BPM n); // tonhuisman: Disabled as we don't use this, and we can avoid a compilation warning

}; //anyrtttl namespace

#endif //BINRTTTL_H
