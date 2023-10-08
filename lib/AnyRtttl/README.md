![AnyRtttl logo](https://github.com/end2endzone/AnyRtttl/raw/master/docs/AnyRtttl-splashscreen.png)


[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Github Releases](https://img.shields.io/github/release/end2endzone/AnyRtttl.svg)](https://github.com/end2endzone/AnyRtttl/releases)



# AnyRtttl #

AnyRtttl is a feature rich arduino library for playing [RTTTL](http://www.end2endzone.com/anyrtttl-a-feature-rich-arduino-library-for-playing-rtttl-melodies/#Quick_recall_of_the_RTTTL_format) melodies. The library offers much more interesting features than relying on the widely available `void play_rtttl(char *p)` function. The library supports all best RTTTL features.

Library features:

* Really small increase in memory & code footprint compared to the usual blocking algorithm.
* Blocking & Non-Blocking modes available.
* Support custom `tone()`, `noTone()`, `delay()` and `millis()` functions.
* Compatible with external Tone libraries.
* Supports highly compressed RTTTL binary format.
* Supports RTTTL melodies stored in Program Memory (`PROGMEM`).
* Play two monolithic melodies on two different pins using 2 piezo buzzer with the help of an external Tone library.



## Status ##

Build:

| Service | Build | Tests |
|----|-------|-------|
| AppVeyor | [![Build status](https://img.shields.io/appveyor/ci/end2endzone/AnyRtttl/master.svg?logo=appveyor)](https://ci.appveyor.com/project/end2endzone/AnyRtttl) | [![Tests status](https://img.shields.io/appveyor/tests/end2endzone/AnyRtttl/master.svg?logo=appveyor)](https://ci.appveyor.com/project/end2endzone/AnyRtttl/branch/master/tests) |
| Windows Server 2019 | [![Build on Windows](https://github.com/end2endzone/AnyRtttl/actions/workflows/build_windows.yml/badge.svg)](https://github.com/end2endzone/AnyRtttl/actions/workflows/build_windows.yml) | [![Tests on Windows](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/end2endzone/58cf6c72c08e706335337d5ef9ca48e8/raw/AnyRtttl.master.Windows.json)](https://github.com/end2endzone/AnyRtttl/actions/workflows/build_windows.yml) |
| Ubuntu 22.04        | [![Build on Linux](https://github.com/end2endzone/AnyRtttl/actions/workflows/build_linux.yml/badge.svg)](https://github.com/end2endzone/AnyRtttl/actions/workflows/build_linux.yml)       | [![Tests on Linux](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/end2endzone/58cf6c72c08e706335337d5ef9ca48e8/raw/AnyRtttl.master.Linux.json)](https://github.com/end2endzone/AnyRtttl/actions/workflows/build_linux.yml)       |

Statistics:

| AppVeyor                                                                                                                                         | GitHub                                                                                                                          |
|--------------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------|
| [![Statistics](https://buildstats.info/appveyor/chart/end2endzone/AnyRtttl)](https://ci.appveyor.com/project/end2endzone/AnyRtttl/branch/master) | [![Statistics](https://buildstats.info/github/chart/end2endzone/AnyRtttl)](https://github.com/end2endzone/AnyRtttl/actions) |




# Purpose #

After publishing [NonBlockingRtttl](https://github.com/end2endzone/NonBlockingRTTTL) arduino library, I started using the library in more complex projects which was requiring other libraries. I quickly ran into the hell of library dependencies and library conflicts. I realized that I needed more features that could help me prototype faster.

Other libraries available which allows you to "play" a melody in [RTTTL](http://www.end2endzone.com/anyrtttl-a-feature-rich-arduino-library-for-playing-rtttl-melodies/#Quick_recall_of_the_RTTTL_format) format suffer the same issue: they are based on blocking APIs or the RTTTL data is not optimized for space.

AnyRtttl is different since it packs multiple RTTTL related features in a single library. It supports [blocking](https://en.wikipedia.org/wiki/Blocking_(computing)) & [non-blocking](http://en.wikipedia.org/wiki/Non-blocking_algorithm) API which makes it suitable to be used by more advanced algorithm. For instance, when using the non-blocking API, the melody can be stopped when a button is pressed. The library is also compatible with external Tone libraries and it supports highly compressed RTTTL binary formats.



## Non-Blocking ##

Most of the code that can "play" a melody on internet are build the same way: sequential calls to `tone()` and `delay()` functions using hardcoded values. This type of implementation might be good for robots but not for realtime application or projects that needs to monitor pins while the song is playing.

With AnyRtttl non-blocking mode, your program can read/write IOs pins while playing and react on changes. Implementing a "stop" or "next song" push button is easy!



## External Tone or Timer #0 libraries ##

The AnyRtttl library is also flexible by allowing you to use the build-in arduino `tone()` and `noTone()` functions or an implementation from any external library which makes it compatible with any Tone library in the market.

The library also supports custom `delay()` and `millis()` functions. If a project requires modification to the microcontroller's build-in Timer #0, the `millis()` function may be impacted and behave incorrectly. To maximize compatibility, one can supply a custom function which behaves like the original to prevent altering playback.



## Binary RTTTL ##

The AnyRtttl library also supports playing RTTTL data which is stored as binary data instead of text. This is actually a custom implementation of the RTTTL format. Using this format, one can achieve storing an highly compressed RTTTL melody which saves memory.

See below for details on RTTTL binary format.




# Usage #

The following instructions show how to use the library.

Define `ANY_RTTTL_INFO` to enable the debugging of the library state on the serial port.

Use `ANY_RTTTL_VERSION` to get the current version of the library.

Note, the specified macros must be defined before including `anyrtttl.h` in your sketches.



## Non-blocking mode ##

      anyrtttl::nonblocking::begin(BUZZER_PIN, mario);
Call `anyrtttl::nonblocking::begin()` to setup AnyRtttl library in non-blocking mode.

Then call `anyrtttl::nonblocking::play()` to update the library's state and play notes as required.

Use `anyrtttl::done()` or `anyrtttl::nonblocking::isPlaying()` to know if the library is done playing the given song.

Anytime, one can call `anyrtttl::nonblocking::stop()` to stop playing the current song.

The following code shows how to use the library in non-blocking mode:

```cpp
#include <anyrtttl.h>
#include <binrtttl.h>
#include <pitches.h>

//project's constants
#define BUZZER_PIN 8
const char * tetris = "tetris:d=4,o=5,b=160:e6,8b,8c6,8d6,16e6,16d6,8c6,8b,a,8a,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,2a,8p,d6,8f6,a6,8g6,8f6,e6,8e6,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,a";
const char * arkanoid = "Arkanoid:d=4,o=5,b=140:8g6,16p,16g.6,2a#6,32p,8a6,8g6,8f6,8a6,2g6";
const char * mario = "mario:d=4,o=5,b=100:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p,8g,8p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,16p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16c7,16p,16c7,16c7,p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16d#6,8p,16d6,8p,16c6";
byte songIndex = 0; //which song to play when the previous one finishes

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);

  Serial.begin(115200);
  Serial.println();
}

void loop() {
  // If we are not playing something 
  if ( !anyrtttl::nonblocking::isPlaying() )
  {
    // Play a song based on songIndex.
    if (songIndex == 0)
      anyrtttl::nonblocking::begin(BUZZER_PIN, tetris);
    else if (songIndex == 1)
      anyrtttl::nonblocking::begin(BUZZER_PIN, arkanoid);
    else if (songIndex == 2)
      anyrtttl::nonblocking::begin(BUZZER_PIN, mario);

    //Set songIndex ready for next song
    songIndex++;
  }
  else
  {
    anyrtttl::nonblocking::play();
  }
}
```



## Playing RTTTL data stored in flash (program) memory ##

AnyRtttl also supports RTTTL melodies stored in flash or Program Memory (PROGMEM).

The `anyrtttl::nonblocking::begin()` function supports _Program Memory_ macros such as `FPSTR()` or `F()`.

The following code shows how to use the library with RTTTL data stored in flash (program) memory instead of SRAM:

```cpp
#include <anyrtttl.h>
#include <binrtttl.h>
#include <pitches.h>

//project's constants
#define BUZZER_PIN 8
const char tetris[] PROGMEM = "tetris:d=4,o=5,b=160:e6,8b,8c6,8d6,16e6,16d6,8c6,8b,a,8a,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,2a,8p,d6,8f6,a6,8g6,8f6,e6,8e6,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,a";
const char arkanoid[] PROGMEM = "Arkanoid:d=4,o=5,b=140:8g6,16p,16g.6,2a#6,32p,8a6,8g6,8f6,8a6,2g6";
const char mario[] PROGMEM = "mario:d=4,o=5,b=100:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p,8g,8p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,16p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16c7,16p,16c7,16c7,p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16d#6,8p,16d6,8p,16c6";
// James Bond theme defined in inline code below (also stored in flash memory) 
byte songIndex = 0; //which song to play when the previous one finishes

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);

  Serial.begin(115200);
  Serial.println();
}

void loop() {
  // If we are not playing something 
  if ( !anyrtttl::nonblocking::isPlaying() )
  {
    // Play a song based on songIndex.
    if (songIndex == 0)
      anyrtttl::nonblocking::beginProgMem(BUZZER_PIN, tetris);
    else if (songIndex == 1)
      anyrtttl::nonblocking::begin_P(BUZZER_PIN, arkanoid);
    else if (songIndex == 2)
      anyrtttl::nonblocking::begin(BUZZER_PIN, FPSTR(mario));
    else if (songIndex == 3)
      anyrtttl::nonblocking::begin(BUZZER_PIN, F("Bond:d=4,o=5,b=80:32p,16c#6,32d#6,32d#6,16d#6,8d#6,16c#6,16c#6,16c#6,16c#6,32e6,32e6,16e6,8e6,16d#6,16d#6,16d#6,16c#6,32d#6,32d#6,16d#6,8d#6,16c#6,16c#6,16c#6,16c#6,32e6,32e6,16e6,8e6,16d#6,16d6,16c#6,16c#7,c.7,16g#6,16f#6,g#.6"));

    //Set songIndex ready for next song
    songIndex++;
  }
  else
  {
    anyrtttl::nonblocking::play();
  }
}
```



# Advanced Usage #



## Custom Tone function (a.k.a. RTTTL 2 code) ##

This example shows how custom functions can be used by the AnyRtttl library to convert an RTTTL melody to arduino code.

First define replacement functions like the following:

```cpp
void serialTone(byte pin, uint16_t frequency, uint32_t duration) {
  Serial.print("tone(");
  Serial.print(pin);
  Serial.print(",");
  Serial.print(frequency);
  Serial.print(",");
  Serial.print(duration);
  Serial.println(");");
}

void serialNoTone(byte pin) {
  Serial.print("noTone(");
  Serial.print(pin);
  Serial.println(");");
}

void serialDelay(uint32_t duration) {
  Serial.print("delay(");
  Serial.print(duration);
  Serial.println(");");
}
```

Each new functions prints the function call & arguments to the serial port.

In the `setup()` function, setup the AnyRtttl library to use the new functions:

```cpp
//Use custom functions
anyrtttl::setToneFunction(&serialTone);
anyrtttl::setNoToneFunction(&serialNoTone);
anyrtttl::setDelayFunction(&serialDelay);
```

Use the `anyrtttl::blocking::play()` API for "playing" an RTTTL melody and monitor the output of the serial port to see the actual arduino code generated by the library.

The following code shows how to use the library with custom functions:

```cpp
#include <anyrtttl.h>
#include <binrtttl.h>
#include <pitches.h>

//project's constants
#define BUZZER_PIN 8
const char * tetris = "tetris:d=4,o=5,b=160:e6,8b,8c6,8d6,16e6,16d6,8c6,8b,a,8a,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,2a,8p,d6,8f6,a6,8g6,8f6,e6,8e6,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,a";

//*******************************************************************************************************************
//  The following replacement functions prints the function call & parameters to the serial port.
//*******************************************************************************************************************
void serialTone(byte pin, uint16_t frequency, uint32_t duration) {
  Serial.print("tone(");
  Serial.print(pin);
  Serial.print(",");
  Serial.print(frequency);
  Serial.print(",");
  Serial.print(duration);
  Serial.println(");");
}

void serialNoTone(byte pin) {
  Serial.print("noTone(");
  Serial.print(pin);
  Serial.println(");");
}

void serialDelay(uint32_t duration) {
  Serial.print("delay(");
  Serial.print(duration);
  Serial.println(");");
}

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);

  Serial.begin(115200);
  Serial.println();

  //Use custom functions
  anyrtttl::setToneFunction(&serialTone);
  anyrtttl::setNoToneFunction(&serialNoTone);
  anyrtttl::setDelayFunction(&serialDelay);
}

void loop() {
  anyrtttl::blocking::play(BUZZER_PIN, tetris);

  while(true)
  {
  }
}
```


## Play 16 bits per note RTTTL ##

Note that this mode requires that an RTTTL melody be already converted to 16-bits per note binary format.

Use the `anyrtttl::blocking::play16Bits()` API for playing an RTTTL melody encoded as 16 bits per note.

The following code shows how to use the library with 16-bits per note binary RTTTL:

```cpp
#include <anyrtttl.h>
#include <binrtttl.h>
#include <pitches.h>

//project's constants
#define BUZZER_PIN 8

//RTTTL 16 bits binary format for the following: tetris:d=4,o=5,b=160:e6,8b,8c6,8d6,16e6,16d6,8c6,8b,a,8a,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,2a,8p,d6,8f6,a6,8g6,8f6,e6,8e6,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,a
const unsigned char tetris16[] = {0x0A, 0x14, 0x12, 0x02, 0x33, 0x01, 0x03, 0x02, 0x0B, 0x02, 0x14, 0x02, 0x0C, 0x02, 0x03, 0x02, 0x33, 0x01, 0x2A, 0x01, 0x2B, 0x01, 0x03, 0x02, 0x12, 0x02, 0x0B, 0x02, 0x03, 0x02, 0x32, 0x01, 0x33, 0x01, 0x03, 0x02, 0x0A, 0x02, 0x12, 0x02, 0x02, 0x02, 0x2A, 0x01, 0x29, 0x01, 0x3B, 0x01, 0x0A, 0x02, 0x1B, 0x02, 0x2A, 0x02, 0x23, 0x02, 0x1B, 0x02, 0x12, 0x02, 0x13, 0x02, 0x03, 0x02, 0x12, 0x02, 0x0B, 0x02, 0x03, 0x02, 0x32, 0x01, 0x33, 0x01, 0x03, 0x02, 0x0A, 0x02, 0x12, 0x02, 0x02, 0x02, 0x2A, 0x01, 0x2A, 0x01};
const int tetris16_length = 42;

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);

  Serial.begin(115200);
  Serial.println();
}

void loop() {
  anyrtttl::blocking::play16Bits(BUZZER_PIN, tetris16, tetris16_length);

  while(true)
  {
  }
}
```



## Play 10 bits per note RTTTL ##

Note that this mode requires that an RTTTL melody be already converted to 10-bits per note binary format.

Create a function that will be used by AnyRtttl library to read bits as required. The signature of the library must look like this: `uint16_t function_name(uint8_t numBits)`.

Note that this code requires the [BitReader](https://github.com/end2endzone/BitReader) library to extract bits from the RTTTL binary buffer. The implementation of `readNextBits()` function delegates the job to the BitReader's `read()` method.

In the `setup()` function, setup the external library that is used for reading bits: `bitreader.setBuffer(tetris10);`.

Use the `anyrtttl::blocking::play10Bits()` API for playing an RTTTL melody encoded as 10 bits per note. The 3rd argument of the function requires a pointer to the function extracting bits: `&function_name`.

The following code shows how to use the library with 10-bits per note binary RTTTL:

```cpp
#include <anyrtttl.h>
#include <binrtttl.h>
#include <pitches.h>

//The BitReader library is required for extracting 10 bit blocks from the RTTTL buffer.
//It can be installed from Arduino Library Manager or from https://github.com/end2endzone/BitReader/releases
#include <bitreader.h>

//project's constants
#define BUZZER_PIN 8

//RTTTL 10 bits binary format for the following: tetris:d=4,o=5,b=160:e6,8b,8c6,8d6,16e6,16d6,8c6,8b,a,8a,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,2a,8p,d6,8f6,a6,8g6,8f6,e6,8e6,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,a
const unsigned char tetris10[] = {0x0A, 0x14, 0x12, 0xCE, 0x34, 0xE0, 0x82, 0x14, 0x32, 0x38, 0xE0, 0x4C, 0x2A, 0xAD, 0x34, 0xA0, 0x84, 0x0B, 0x0E, 0x28, 0xD3, 0x4C, 0x03, 0x2A, 0x28, 0xA1, 0x80, 0x2A, 0xA5, 0xB4, 0x93, 0x82, 0x1B, 0xAA, 0x38, 0xE2, 0x86, 0x12, 0x4E, 0x38, 0xA0, 0x84, 0x0B, 0x0E, 0x28, 0xD3, 0x4C, 0x03, 0x2A, 0x28, 0xA1, 0x80, 0x2A, 0xA9, 0x04};
const int tetris10_length = 42;

//bit reader support
#ifndef USE_BITADDRESS_READ_WRITE
BitReader bitreader;
#else
BitAddress bitreader;
#endif
uint16_t readNextBits(uint8_t numBits)
{
  uint16_t bits = 0;
  bitreader.read(numBits, &bits);
  return bits;
}

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);

  bitreader.setBuffer(tetris10);
  
  Serial.begin(115200);
  Serial.println();
}

void loop() {
  anyrtttl::blocking::play10Bits(BUZZER_PIN, tetris10_length, &readNextBits);

  while(true)
  {
  }
}
```



## Other ##

More AnyRtttl examples are also available:

* [Basic](examples/Basic/Basic.ino)
* [BlockingProgramMemoryRtttl](examples/BlockingProgramMemoryRtttl/BlockingProgramMemoryRtttl.ino)
* [BlockingRtttl](examples/BlockingRtttl/BlockingRtttl.ino)
* [BlockingWithNonBlocking](examples/BlockingWithNonBlocking/BlockingWithNonBlocking.ino)
* [NonBlockingProgramMemoryRtttl](examples/NonBlockingProgramMemoryRtttl/NonBlockingProgramMemoryRtttl.ino)
* [NonBlockingRtttl](examples/NonBlockingRtttl/NonBlockingRtttl.ino)
* [NonBlockingStopBeforeEnd](examples/NonBlockingStopBeforeEnd/NonBlockingStopBeforeEnd.ino)
* [Play10Bits](examples/Play10Bits/Play10Bits.ino)
* [Play16Bits](examples/Play16Bits/Play16Bits.ino)
* [Rtttl2Code](examples/Rtttl2Code/Rtttl2Code.ino)




# Binary RTTTL format definition #

The following section defines the field order and size (in bits) required for encoding / decoding of each melody as binary RTTTL.

This is actually a custom implementation of the RTTTL format. Using this format, one can achieve storing an highly compressed RTTTL melody which saves memory.

Note that all fields definition are defined in LSB to MSB order.

## Header ##

The first 16 bits stores the RTTTL default section (a.k.a header) which is defined as the following:

| Field name              | Size (bits) | Range    | Description                                                 |
|-------------------------|:-----------:|----------|-------------------------------------------------------------|
| Default duration index  |      3      | [0, 7]   | Matches the index used for `getNoteDurationFromIndex()` API |
| Default octave index    |      2      | [0, 3]   | Matches the index used for `getNoteOctaveFromIndex()` API.  |
| Beats per minutes (BPM) |      10     | [1, 900] |                                                             |
| Padding                 |      1      |          |                                                             |

## Notes ##

Next is each note's of the melody. Each note is encoded as 10 bits (or 16 bits) per note. Notes are defined as the following:

| Field name         | Size (bits) | Range   | Description                                                  |
|--------------------|:-----------:|---------|--------------------------------------------------------------|
| Duration index     |      3      | [0, 7]  | Matches the index used for `getNoteDurationFromIndex()` API. |
| Note letter index  |      3      | [0, 7]  | Matches the index used for `getNoteLetterFromIndex()` API.   |
| Pound              |      1      | boolean | Defines if the note is pounded or not.                       |
| Dotted             |      1      | boolean | Defines if the note is dotted or not.                        |
| Octave index       |      2      | [0, 3]  | Matches the index used for `getNoteOctaveFromIndex()` API.   |
| Padding (optional) |      6      |         | See description below.                                       |

The last field of a note (defined as `Padding`) is an optional 6 bits field. The AnyRtttl library supports both 10 bits per note and 16 bits per note definitions. Use the appropriate API for playing both format.



## 10 bits per note (no padding) ##

Each RTTTL note is encoded into 10 bits which is the minimum size of a note. This storage method is the best compression method for storing RTTTL melodies and reduces the usage of the dynamic memory to the minimum.

However, since all notes are not aligned on multiple of 8 bits, addressing each note by an offset is impossible which makes the playback harder. Each notes must be deserialized one after the other from a buffer using blocks of 10 bits which increases the program storage space footprint.

An external arduino library (or custom code) is required to allow AnyRtttl library to consume bits as needed. The arduino [BitReader](https://github.com/end2endzone/BitReader) library may be used for handling bit deserialization but any library that can extract a given number of bits from a buffer would work.



## 16 bits per note (with padding) ##

Each RTTTL note is encoded into 16 bits which is much better than the average 3.28 bytes per note text format. This storage method is optimum for storing RTTTL melodies and reduces the usage of the dynamic memory without increasing to much program storage space.

All notes are aligned on 16 bits. Addressing each note by an offset allows an easy playback. Only the first 10 bits of each 16 bits block is used. The value of the padding field is undefined.



## Playback ##

The following AnyRtttl blocking APIs are available for playing both binary formats:
* 10 bits per note: `anyrtttl::blocking::play10Bits()`.
* 16 bits per note: `anyrtttl::blocking::play16Bits()`.




# Building #

Please refer to file [INSTALL.md](INSTALL.md) for details on how installing/building the application.




# Platforms #

AnyRtttl has been tested with the following platform:

  * Linux x86/x64
  * Windows x86/x64




# Versioning #

We use [Semantic Versioning 2.0.0](http://semver.org/) for versioning. For the versions available, see the [tags on this repository](https://github.com/end2endzone/AnyRtttl/tags).




# Authors #

* **Antoine Beauchamp** - *Initial work* - [end2endzone](https://github.com/end2endzone)

See also the list of [contributors](https://github.com/end2endzone/AnyRtttl/blob/master/AUTHORS) who participated in this project.




# License #

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details
