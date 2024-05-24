// ---------------------------------------------------------------------------
// AUTHOR/LICENSE:
//  The following code was written by Antoine Beauchamp. For other authors, see AUTHORS file.
//  The code & updates for the library can be found at https://github.com/end2endzone/AnyRtttl
//  MIT License: http://www.opensource.org/licenses/mit-license.php
// ---------------------------------------------------------------------------

#include "binrtttl.h"

namespace anyrtttl
{

static const RTTTL_NOTE_LETTER gNoteLetters[] =   {'c','d','e','f','g','a','b','p'};
static const uint16_t gNoteLettersCount = sizeof(gNoteLetters)/sizeof(gNoteLetters[0]);

static const int gNoteOffsets[] = { 1, 3, 5, 6, 8, 10, 12, 0};

static const RTTTL_DURATION gNoteDurations[] = {1, 2, 4, 8, 16, 32};
static const uint16_t gNoteDurationsCount = sizeof(gNoteDurations)/sizeof(gNoteDurations[0]);

static const RTTTL_OCTAVE_VALUE gNoteOctaves[] = {4, 5, 6, 7};
static const uint16_t gNoteOctavesCount = sizeof(gNoteOctaves)/sizeof(gNoteOctaves[0]);

static const RTTTL_BPM gNoteBpms[] = {25, 28, 31, 35, 40, 45, 50, 56, 63, 70, 80, 90, 100, 112, 125, 140, 160, 180, 200, 225, 250, 285, 320, 355, 400, 450, 500, 565, 635, 715, 800, 900};
static const uint16_t gNoteBpmsCount = sizeof(gNoteBpms)/sizeof(gNoteBpms[0]);

RTTTL_NOTE_LETTER getNoteLetterFromIndex(NOTE_LETTER_INDEX iIndex)
{
  if (iIndex >= 0 && iIndex < gNoteLettersCount)
    return gNoteLetters[iIndex];
  return -1;
}

uint16_t getNoteLettersCount()
{
  return gNoteLettersCount;
}

NOTE_LETTER_INDEX findNoteLetterIndex(RTTTL_NOTE_LETTER n)
{
  for(NOTE_LETTER_INDEX i=0; i<gNoteLettersCount; i++)
  {
    if (getNoteLetterFromIndex(i) == n)
    {
      return i;
    }
  }
  return -1;
}

int getNoteOffsetFromLetterIndex(NOTE_LETTER_INDEX iIndex)
{
  if (iIndex >= 0 && iIndex < gNoteLettersCount)
    return gNoteOffsets[iIndex];
  return 0;
}

int getNoteOffsetFromLetter(RTTTL_NOTE_LETTER n)
{
  NOTE_LETTER_INDEX index = findNoteLetterIndex(n);
  return getNoteOffsetFromLetterIndex(index);
}

RTTTL_DURATION getNoteDurationFromIndex(DURATION_INDEX iIndex)
{
  if (iIndex >= 0 && iIndex < gNoteDurationsCount)
    return gNoteDurations[iIndex];
  return -1;
}

uint16_t getNoteDurationsCount()
{
  return gNoteDurationsCount;
}

DURATION_INDEX findNoteDurationIndex(RTTTL_DURATION n)
{
  for(DURATION_INDEX i=0; i<gNoteDurationsCount; i++)
  {
    if (getNoteDurationFromIndex(i) == n)
    {
      return i;
    }
  }
  return -1;
}

RTTTL_OCTAVE_VALUE getNoteOctaveFromIndex(OCTAVE_INDEX iIndex)
{
  if (iIndex >= 0 && iIndex < gNoteOctavesCount)
    return gNoteOctaves[iIndex];
  return -1;
}

uint16_t getNoteOctavesCount()
{
  return gNoteOctavesCount;
}

OCTAVE_INDEX findNoteOctaveIndex(RTTTL_OCTAVE_VALUE n)
{
  for(OCTAVE_INDEX i=0; i<gNoteOctavesCount; i++)
  {
    if (getNoteOctaveFromIndex(i) == n)
    {
      return i;
    }
  }
  return -1;
}

RTTTL_BPM getBpmFromIndex(BPM_INDEX iIndex)
{
  if (iIndex >= 0 && iIndex < gNoteBpmsCount)
    return gNoteBpms[iIndex];
  return -1;
}

uint16_t getBpmsCount()
{
  return gNoteBpmsCount;
}

BPM_INDEX findBpmIndex(RTTTL_BPM n)
{
  for(BPM_INDEX i=0; i<gNoteBpmsCount; i++)
  {
    if (getBpmFromIndex(i) == n)
    {
      return i;
    }
  }
  return INVALID_BPM_INDEX;
}

}; //anyrtttl namespace
