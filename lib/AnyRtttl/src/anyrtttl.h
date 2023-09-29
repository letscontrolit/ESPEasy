// ---------------------------------------------------------------------------
// AUTHOR/LICENSE:
//  The following code was written by Antoine Beauchamp. For other authors, see AUTHORS file.
//  The code & updates for the library can be found at https://github.com/end2endzone/AnyRtttl
//  MIT License: http://www.opensource.org/licenses/mit-license.php
// ---------------------------------------------------------------------------

#ifndef ANY_RTTTL_H
#define ANY_RTTTL_H

#define ANY_RTTTL_VERSION 2.3

#include "Arduino.h"
#include "pitches.h"

//#define ANY_RTTTL_DEBUG
//#define ANY_RTTTL_INFO

namespace anyrtttl
{

/****************************************************************************
 * Custom functions
 ****************************************************************************/


/****************************************************************************
 * Description:
 *   Defines a function pointer to a tone() function
 ****************************************************************************/
typedef void (*ToneFuncPtr)(uint8_t _pin, unsigned int, unsigned long);

/****************************************************************************
 * Description:
 *   Defines a function pointer to a noTone() function
 ****************************************************************************/
typedef void (*NoToneFuncPtr)(uint8_t);

/****************************************************************************
 * Description:
 *   Defines a function pointer to a delay() function
 ****************************************************************************/
#if defined(ESP32)
typedef void (*DelayFuncPtr)(uint32_t);
#else
typedef void (*DelayFuncPtr)(unsigned long);
#endif

/****************************************************************************
 * Description:
 *   Defines a function pointer to a millis() function
 ****************************************************************************/
typedef unsigned long (*MillisFuncPtr)(void);

/****************************************************************************
 * Description:
 *   Defines the tone() function used by AnyRtttl.
 * Parameters:
 *   iFunc: Pointer to a tone() replacement function.
 ****************************************************************************/
void setToneFunction(ToneFuncPtr iFunc);

/****************************************************************************
 * Description:
 *   Defines the noTone() function used by AnyRtttl.
 * Parameters:
 *   iFunc: Pointer to a noTone() replacement function.
 ****************************************************************************/
void setNoToneFunction(NoToneFuncPtr iFunc);

/****************************************************************************
 * Description:
 *   Defines the delay() function used by AnyRtttl.
 * Parameters:
 *   iFunc: Pointer to a delay() replacement function.
 ****************************************************************************/
void setDelayFunction(DelayFuncPtr iFunc);

/****************************************************************************
 * Description:
 *   Defines the millis() function used by AnyRtttl.
 * Parameters:
 *   iFunc: Pointer to a millis() replacement function.
 ****************************************************************************/
void setMillisFunction(MillisFuncPtr iFunc);



/****************************************************************************
 * Description:
 *   Defines a function pointer to read a single char from a buffer
 ****************************************************************************/
typedef char (*ReadCharFuncPtr)(const char *);

/****************************************************************************
 * Description:
 *   Read the first byte of a buffer stored in RAM.
 * Parameters:
 *   iBuffer: A string buffer stored in RAM.
 ****************************************************************************/
char readChar(const char * iBuffer);

/****************************************************************************
 * Description:
 *   Read the first byte of a buffer stored in PROGMEM.
 * Parameters:
 *   iBuffer: A string buffer stored in PROGMEM.
 ****************************************************************************/
char readChar_P(const char * iBuffer);


/****************************************************************************
 * Blocking API
 ****************************************************************************/
namespace blocking
{

/****************************************************************************
 * Description:
 *   Plays a native RTTTL melody.
 * Parameters:
 *   iPin:          The pin which is connected to the piezo buffer.
 *   iBuffer:       The string buffer of the RTTTL melody.
 *   iReadCharFunc: A function pointer to read 1 byte (char) from the given buffer.
 ****************************************************************************/
void play(byte iPin, const char * iBuffer, ReadCharFuncPtr iReadCharFunc);

/****************************************************************************
 * Description:
 *   Plays a native RTTTL melody which is stored in RAM.
 * Parameters:
 *   iPin:    The pin which is connected to the piezo buffer.
 *   iBuffer: The string buffer of the RTTTL melody.
 ****************************************************************************/
void play(byte iPin, const char * iBuffer);

/****************************************************************************
 * Description:
 *   Plays a native RTTTL melody which is stored in Program Memory (PROGMEM).
 * Parameters:
 *   iPin:    The pin which is connected to the piezo buffer.
 *   iBuffer: The string buffer of the RTTTL melody.
 ****************************************************************************/
void play(byte iPin, const __FlashStringHelper* str);
void playProgMem(byte iPin, const char * iBuffer);
void play_P(byte iPin, const char * iBuffer);
void play_P(byte iPin, const __FlashStringHelper* str);

/****************************************************************************
 * Description:
 *   Plays a RTTTL melody which is encoded as 16 bits per notes.
 * Parameters:
 *   iPin:      The pin which is connected to the piezo buffer.
 *   iBuffer:   The binary buffer of the RTTTL melody. See remarks for details.
 *   iNumNotes: The number of notes within the given melody buffer.
 * Remarks:
 *   The first 16 bits of the buffer are reserved for the default section.
 *   See the definition of RTTTL_DEFAULT_VALUE_SECTION union for details.
 *   Each successive notes are encoded as 16 bits per note as defined by
 *   RTTTL_NOTE union.
 ****************************************************************************/
void play16Bits(int iPin, const unsigned char * iBuffer, int iNumNotes);

/****************************************************************************
 * Description:
 *   Defines a function pointer which is used by the play10Bits() function as 
 *   a bit provider function. The signature of a compatible function must be
 *   the following: uint16_t foo(uint8_t iNumBits);
 ****************************************************************************/
typedef uint16_t (*BitReadFuncPtr)(uint8_t);

/****************************************************************************
 * Description:
 *   Plays a RTTTL melody which is encoded as 10 bits per notes.
 * Parameters:
 *   iPin:      The pin which is connected to the piezo buffer.
 *   iNumNotes: The number of notes within the given melody.
 *   iFuncPtr:  Pointer to a function which is used by play10Bits() as a bit  The binary buffer of the RTTTL melody. See remarks for details.
 * Remarks:
 *   The first 16 bits of the buffer are reserved for the default section.
 *   See the definition of RTTTL_DEFAULT_VALUE_SECTION union for details.
 *   Each successive notes are encoded as 10 bits per note as defined by
 *   RTTTL_NOTE union.
 ****************************************************************************/
void play10Bits(int iPin, int iNumNotes, BitReadFuncPtr iFuncPtr);

}; //blocking namespace



/****************************************************************************
 * Non-blocking API
 ****************************************************************************/
namespace nonblocking
{

/****************************************************************************
 * Description:
 *   Setups the AnyRtttl library for non-blocking mode and ready to
 *   decode a new RTTTL song.
 * Parameters:
 *   iPin:          The pin which is connected to the piezo buffer.
 *   iBuffer:       The string buffer of the RTTTL song.
 *   iReadCharFunc: A function pointer to read 1 byte (char) from the given buffer.
 ****************************************************************************/
void begin(byte iPin, const char * iBuffer, ReadCharFuncPtr iReadCharFunc);

/****************************************************************************
 * Description:
 *   Setups the AnyRtttl library for non-blocking mode and ready to
 *   decode a new RTTTL song stored in RAM.
 * Parameters:
 *   iPin:      The pin which is connected to the piezo buffer.
 *   iBuffer:   The string buffer of the RTTTL song.
 ****************************************************************************/
void begin(byte iPin, const char * iBuffer);

/****************************************************************************
 * Description:
 *   Setups the AnyRtttl library for non-blocking mode and ready to
 *   decode a new RTTTL song stored in Program Memory (PROGMEM).
 * Parameters:
 *   iPin:    The pin which is connected to the piezo buffer.
 *   iBuffer: The string buffer of the RTTTL melody.
 ****************************************************************************/
void begin(byte iPin, const __FlashStringHelper* str);
void beginProgMem(byte iPin, const char * iBuffer);
void begin_P(byte iPin, const char * iBuffer);
void begin_P(byte iPin, const __FlashStringHelper* str);

/****************************************************************************
 * Description:
 *   Automatically plays a new note when required.
 *   This function must constantly be called within the loop() function.
 *   Warning: inserting too long delays within the loop function may
 *   disrupt the NON-BLOCKING RTTTL library from playing properly.
 ****************************************************************************/
void play();

/****************************************************************************
 * Description:
 *   Stops playing the current song.
 ****************************************************************************/
void stop();

/****************************************************************************
 * Description:
 *   Return true when the library is playing the given RTTTL melody.
 ****************************************************************************/
bool isPlaying();

/****************************************************************************
 * Description:
 *   Return true when the library is done playing the given RTTTL song.
 ****************************************************************************/
bool done();

}; //nonblocking namespace

}; //anyrtttl namespace

#endif //ANY_RTTTL_H
