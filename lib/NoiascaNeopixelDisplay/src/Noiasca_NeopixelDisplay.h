/* NoaiscaNeopixelDisplay
   Simulating a Seven Segment Display with Neopixels
   
   Download from: http://werner.rothschopf.net/202005_arduino_neopixel_display_en.htm

   open tasks
   - 
   
   copyright 2022 noiasca noiasca@yahoo.com
   
   Version
   2020-03-02 first version: https://forum.arduino.cc/index.php?topic=668096.0
   2020-05-01 multiple pixel per segment idea: https://forum.arduino.cc/index.php?topic=681417
   2020-05-02 reference to external created strip object (with inputs from https://forum.arduino.cc/index.php?topic=681647.0) 
              multiple displays on one strip
   2020-05-03 migration to segmentArray in user sketch, 
   2020-05-04 callback function for additional pixels
   2021-05-24 1.0.1 fixed lowLevelWrite to leftshift according segsize
   2022-01-16 updated comments for doxygen
   Some history seems to be missing...
   2024-05-12 tonhuisman: Use NeoPixelBus_wrapper instead of Adafruit_NeoPixel library (not supported on IDF 5.x)
   2024-08-05 tonhuisman: write() method: Disabled call to strip.show() to be handled externally
   2024-08-16 tonhuisman: writeLowLevel() method: Allow pixel stripes > 255 neopixels by increasing offset counter to uint16_t
   2024-08-20 tonhuisman: introduce NEOPIXEL_DISPLAY_USE_WRITE
*/

#pragma once
#define NOIASCA_NEOPIXEL_DISPLAY_VERSION "NoiascaNeopixelDisplay 1.0.1"   // this library

/** 
    \brief Neopixel Display base
    
    if you run out of memory and if you only need numbers, you can reduce the size of the sketch by setting this define
    
 * ******************************************************************/
#define NEOPIXEL_DISPLAY_CHARSET_SIZE 2          // 2 full charset (default), 1 capital letters, 0 numbers and some symbols only
#define NEOPIXEL_DISPLAY_DEBUG 0                 // Library debug messages: 0 off (default);   1 important/error;   2 warning;   3 info/debug;

//#if defined(__AVR__)
//#include <avr/pgmspace.h>
//#elif defined(ESP8266)
//#include <pgmspace.h>
//#endif

#ifndef NEOPIXEL_DISPLAY_USE_WRITE
#define NEOPIXEL_DISPLAY_USE_WRITE  1
#endif

#include <NeoPixelBus_wrapper.h>                                       // install library from Library manager

/*
   Segments are named and orded like this

          SEG_A
   SEG_F         SEG_B
          SEG_G
   SEG_E         SEG_C
          SEG_D             SEG_DP

       -   
  The mapping which pixels will be lighted for which character is basically done in 3 steps

  Step 1 
  
  The user has to assign the pixels to each segment in the user Sketch
  example: 
  
typedef uint32_t segsize_t;
const segsize_t segment[8] { 
  0b0000000000000011,  // SEG_A
  0b0000000000001100,  // SEG_B
  0b0000000000110000,  // SEG_C
  0b0000000011000000,  // SEG_D
  0b0000001100000000,  // SEG_E
  0b0000110000000000,  // SEG_F
  0b0011000000000000,  // SEG_G
  0b1100000000000000   // SEG_DP
};

 Step 2 
 the library has a character table to map characters to the needed segments
 
 Step 3
 The write method combines the users segment definition with the character segments to optain 
 a pixel bitmap to be printed to the display
*/

#if NEOPIXEL_DISPLAY_USE_WRITE

/* *******************************************************************
         character set for 7 segment displays
 * ******************************************************************/

// each segment is assigned to one of 8 positions (bitwise)

const byte SEG_A  = 1;
const byte SEG_B  = 2;
const byte SEG_C  = 4;
const byte SEG_D  = 8;
const byte SEG_E  = 16;
const byte SEG_F  = 32;
const byte SEG_G  = 64;
const byte SEG_DP = 128;

// several segments combined is one character
// The bitmap of the segment table only defines which segments belongs to which character

const static byte charTable [] PROGMEM  = {           // if you run out of FLASH memory and you only need numbers, you can delete the characters after 57
  0,                                                       //     32   space
  SEG_B | SEG_C | SEG_DP,                                  // !   33
  SEG_B | SEG_F,                                           // "   34
  0,                                                       // #   35
  SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,                   // $   36
  SEG_A | SEG_B | SEG_F | SEG_G,                           // %   37
  0,                                                       // &   38
  SEG_B,                                                   // '   39
  SEG_A | SEG_D | SEG_E | SEG_F,                           // (   40
  SEG_A | SEG_B | SEG_C | SEG_D,                           // )   41
  0,                                                       // *   42   no character on 7segment
  0,                                                       // +   43   no character on 7segment
  0,                                                       // ,   44   will be handled in the write methode
  SEG_G,                                                   // -   45
  0,                                                       // .   46   will be handled in the write methode
  SEG_B | SEG_E | SEG_G ,                                  // /   47
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,           // 0   48
  SEG_B | SEG_C,                                           // 1   49
  SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,                   // 2   50
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_G,                   // 3   51
  SEG_B | SEG_C | SEG_F | SEG_G,                           // 4   52
  SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,                   // 5   53
  SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,           // 6   54
  SEG_A | SEG_B | SEG_C,                                   // 7   55
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,   // 8   56
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G,           // 9   57
#if NEOPIXEL_DISPLAY_CHARSET_SIZE >= 1
  0,                                                       // :   58   could be handled in the write methode
  0,                                                       // ;   59   could be handled in the write methode
  SEG_D | SEG_E | SEG_G,                                   // <   60
  SEG_G,                                                   // =   61
  SEG_C | SEG_D | SEG_G,                                   // >   62
  SEG_A | SEG_B | SEG_E | SEG_G,                           // ?   63
  0,                                                       // @   64
  SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,           // A   65
  SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,                   // B   66
  SEG_A | SEG_D | SEG_E | SEG_F,                           // C   67
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,                   // D   68
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,                   // E   69
  SEG_A | SEG_E | SEG_F | SEG_G,                           // F   70
  SEG_A | SEG_C | SEG_D | SEG_E | SEG_F,                   // G   71
  SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,                   // H   72
  SEG_B | SEG_C,                                           // I   73
  SEG_B | SEG_C | SEG_D | SEG_E,                           // J   74
  SEG_A | SEG_C | SEG_E | SEG_F | SEG_G,                   // K   75
  SEG_D | SEG_E | SEG_F,                                   // L   76
  SEG_A | SEG_C | SEG_E,                                   // M   77
  SEG_C | SEG_E | SEG_G,                                   // N   78
  SEG_C | SEG_D | SEG_E | SEG_G,                           // O   79
  SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,                   // P   80
  SEG_A | SEG_B | SEG_C | SEG_F | SEG_G,                   // Q   81
  SEG_E | SEG_G,                                           // R   82
  SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,                   // S   83
  SEG_D | SEG_E | SEG_F | SEG_G,                           // T   84
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,                   // U   85
  SEG_C | SEG_D | SEG_E,                                   // V   86
  SEG_B | SEG_D | SEG_F,                                   // W   87
  SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,                   // X   88
  SEG_B | SEG_C | SEG_D | SEG_F | SEG_G,                   // Y   89
  SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,                   // Z   90
  SEG_A | SEG_D | SEG_E | SEG_F,                           // [   91
  SEG_C | SEG_F | SEG_G,                                   /* \   92 backslash*/
  SEG_A | SEG_B | SEG_C | SEG_D,                           // ]   93
  SEG_A,                                                   // ^   94
  SEG_D,                                                   // _   95 underscore
  SEG_B,                                                   // `   96
#endif
#if NEOPIXEL_DISPLAY_CHARSET_SIZE >= 2
  SEG_C | SEG_D | SEG_E | SEG_G | SEG_DP,                  // a   97
  SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,                   // b   98
  SEG_D | SEG_E | SEG_G,                                   // c   99
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,                   // d   100
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,                   // e   101
  SEG_A | SEG_E | SEG_F | SEG_G,                           // f   102
  SEG_A | SEG_C | SEG_D | SEG_E | SEG_F,                   // g G 103 capital letter will be used
  SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,                   // h   104
  SEG_C,                                                   // i   105
  SEG_C | SEG_D,                                           // j   106
  SEG_A | SEG_C | SEG_E | SEG_F | SEG_G,                   // k   107
  SEG_E | SEG_F,                                           // l   108
  SEG_A | SEG_C | SEG_E,                                   // m n 109 n will be used
  SEG_C | SEG_E | SEG_G,                                   // n   110
  SEG_C | SEG_D | SEG_E | SEG_G,                           // o   111
  SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,                   // p P 112
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_DP,          // q Q 113
  SEG_E | SEG_G,                                           // r   114
  SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,                   // s S 115
  SEG_D | SEG_E | SEG_F | SEG_G,                           // t   116
  SEG_C | SEG_D | SEG_E,                                   // u   117
  SEG_C | SEG_D | SEG_E,                                   // v u 118 u will be used
  SEG_C | SEG_D | SEG_E,                                   // w u 119 u will be used
  SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,                   // x   120
  SEG_B | SEG_C | SEG_D | SEG_F | SEG_G,                   // y Y 121
  SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,                   // z Z 122
  SEG_A | SEG_D | SEG_E | SEG_F,                           // {   123
  SEG_B | SEG_C,                                           // |   124
  SEG_A | SEG_B | SEG_C | SEG_D,                           // }   125
  SEG_G,                                                   // ~   126
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G | SEG_DP  //   127 all segments
#endif
};

const byte lastCharacter = sizeof(charTable)/sizeof(charTable[0]) + 32 - 1;    // if you use the full charset the lastCharacter will be 127

#endif // if NEOPIXEL_DISPLAY_USE_WRITE

/** *******************************************************************
    \brief Neopixel Display base
    
    The Noiasca Neopixel Display class can be used for 7 segment displays
    made of neopixel / WS2812 LEDs.
    
 * ******************************************************************/

class Noiasca_NeopixelDisplay : public Print {
  private:
    NeoPixelBus_wrapper& strip;
    const uint16_t startPixel;                             // the first pixel to be used for this display
    const segsize_t *segment;  
    const byte numDigits;                                  // digits per device
    const byte pixelPerDigit;                              // all Pixel,  include double point pixels if they are available at each digit
    //const byte segPerDigit = 7;                          // How many segments per digit (not implemented, has to be 7)
    const byte addPixels;                                  // unregular additional pixels to be added to the strip
    using CallBack = int (*)(uint16_t value);
    CallBack funcPtr;
    const uint16_t ledCount;                               // How many NeoPixels are attached to the Arduino inkluding additional digits
  
    uint16_t currentPosition;                              // current position of cursor, the positions are order from LEFT (0) to RIGHT (highest)
    uint16_t lastPosition;                                 // last position of cursor - needed for dot and comma
    uint32_t colorFont = 0xFF0000;                         // default color of visible segment
    uint32_t colorBack = 0x000000;                         // default background color 0=black
    //segsize_t lastBitmap;                                // stores the last printed segments - for future use
    int8_t order = 1;                                      // leftToRight = 1, rightToLeft=-1

  public:
/** 
    \brief Constructor with 4 parameters
    
    The Noiasca Neopixel Display class can be used for 7 segment displays
    made of neopixel / WS2812 LEDs.
    @param   strip          a reference to your strip.
    @param   segment        an array with the needed pixels for each segment.
    @param   numDigits      how many digits you have on your display. 
    @param   pixelPerDigit  number of used pixels per digit.
*/   
    Noiasca_NeopixelDisplay(NeoPixelBus_wrapper& strip, const segsize_t segment[8], byte numDigits, byte pixelPerDigit):
      strip(strip),
      startPixel(0),  
      segment{segment},
      numDigits(numDigits),
      pixelPerDigit(pixelPerDigit),
      addPixels(0),
      funcPtr(nullptr),
      ledCount(pixelPerDigit * numDigits + addPixels)
    {}
    
/** 
    \brief Constructor with 5 parameters
    
    @param   strip          a reference to your strip.
    @param   segment        an array with the needed pixels for each segment.
    @param   numDigits      how many digits you have on your display. 
    @param   pixelPerDigit  number of used pixels per digit.
    @param   startPixel     if don't start with pixel 0 on the display use the 5th parameter to define the first pixel on the display.
*/ 
    Noiasca_NeopixelDisplay(NeoPixelBus_wrapper& strip, const segsize_t segment[8], byte numDigits, byte pixelPerDigit, uint16_t startPixel):
      strip(strip),
      startPixel(startPixel),
      segment{segment},
      numDigits(numDigits),
      pixelPerDigit(pixelPerDigit),
      addPixels(0),
      funcPtr(nullptr),
      ledCount(pixelPerDigit * numDigits + addPixels)
    {}
    
/** 
    \brief Constructor with 7 parameters
    
    @param   strip          a reference to your strip.
    @param   segment        an array with the needed pixels for each segment.
    @param   numDigits      how many digits you have on your display.
    @param   pixelPerDigit  number of used pixels per digit.
    @param   startPixel     if don't start with pixel 0 on the display use the 5th parameter to define the first pixel on the display.
    @param   addPixels      if there are more pixels in the chain than used for numbers on the display use this parameter to inform the library about the additional pixels.
    @param   funcPtr        a callback function to calculate the start pixel for each digit. 
*/ 
    Noiasca_NeopixelDisplay(NeoPixelBus_wrapper& strip, segsize_t segment[8], byte numDigits, byte pixelPerDigit, uint16_t startPixel, byte addPixels, CallBack funcPtr):
      strip(strip),
      startPixel(startPixel),
      segment{segment},
      numDigits(numDigits),
      pixelPerDigit(pixelPerDigit),
      addPixels(addPixels),
      funcPtr(funcPtr),
      ledCount(pixelPerDigit * numDigits + addPixels)
    {}
 
    virtual ~Noiasca_NeopixelDisplay() = default;
    
/**
  @brief   clear all pixels of the NeoPixel display.
  
  This method uses Adafruits fill method to clear the definied  
  pixels of the display. 
  @note    Clearing is done by overwriting all pixels (startPixel to length) with the current background color.
*/
    void clear()
    {
      //strip.clear();                 // library clear uses the fill method
      strip.fill(colorBack, startPixel, ledCount);
      currentPosition = 0;
    }
    
/*!
  @brief   set the background color
  
  The background color is used if a pixel is "off" according to the character bitmap.
  @param   newColor  the new color to be used for the next write
*/
    void setColorBack(uint32_t newColor)
    {
      colorBack = newColor;
    }
    
/*!
  @brief   set the font color
  
  The font color is used if a pixel is definied in the bitmap.
  @param   newColor   the new color to be used for the next write.  
*/    
    void setColorFont(uint32_t newColor)
    {
      colorFont = newColor;
    }
    
/*!
  @brief   set cursor to the specifed cursor
  
  The order of cursor positions is from LEFT to RIGHT, starting with 0 on the LEFT.
  @param   newPosition   the new cursor position for the display  
*/        
    void setCursor(uint8_t newPosition)
    {
      currentPosition = newPosition;
    }
    
/*!
  @brief   set a pixel to a specific color
  
  This gives you direct access to the strip object. 
  It is a imple passthrough method to the Neopixel strip
  @param   pixel the pixel you want to change
  @param   color the new color for this pixel
*/ 
    void setPixelColor(uint16_t pixel, uint32_t color)
    {
      strip.setPixelColor(pixel, color);
    }
    
/*!
  @brief   reverses the numbering of digits 
  
  If the pixels are wired from RIGHT to LEFT 
  use this command to reverse the display
*/  
    void setRightToLeft()
    {
       order = -1;
    }

/*!
  @brief   show the current buffer on the display
  
  It is a simple passthrough method to the Neopixel strip.
*/     
    void show()
    {
      strip.show();
    }

/*!
  @brief   write a bitmap to the display
  
  This low level write will write a bitmap to the given position of your display.
  @param   position the position where the bitmap should be printed
  @param   bitmask  the bitmask to be printed
  @param   addOnly  if set to true, the bitmap will be added to the position. By default (false) only the new bitmap will be shown.
*/      
    void writeLowLevel(uint8_t position, segsize_t bitmask, bool addOnly = false) {
      uint16_t offset = 0; // 2024-08-16 tonhuisman: Changed byte to uint16_t so we can have > 255 pixels on a strip
      if (order == 1)  // ascending order of digits
      {
        offset = position * pixelPerDigit + startPixel;              // pixel offset = first pixel of this digit
        if (funcPtr)                                                 // only if available (adress doesn't point to NULL)
          offset = offset + funcPtr(position);                       // the user can define his own ruleset for offset calculation due to additional pixels
      }
      else             // descending order of digits
      {
        uint16_t additional = 0;
        if (funcPtr)
          additional = funcPtr(position);
        offset = ledCount - pixelPerDigit - position * pixelPerDigit - additional + startPixel;
       }
#if NEOPIXEL_DISPLAY_DEBUG >= 3
      Serial.print("p="); 
      Serial.print(position);
      Serial.print(" o=");
      Serial.print(offset);
      Serial.print(" ");
      Serial.println(bitmask, BIN);
#endif
      for (uint16_t i = 0; i < pixelPerDigit; i++) // 2024-08-16 tonhuisman: Changed byte to uint16_t
      {
        if (bitmask & ((segsize_t)1 << i))                           // was if (bitmask & (1UL << i)) till 1.0.0
          strip.setPixelColor(i + offset, colorFont);
        else
          if (!addOnly) strip.setPixelColor(i + offset, colorBack);
      }
    }

#if NEOPIXEL_DISPLAY_USE_WRITE

    size_t write(uint8_t value)
    {
      if (value == '.' || value == ',')
      {
         writeLowLevel(lastPosition, segment[7], true);              // add decimal point to the last printed digit
      }
      else if (value > 31 && value <= lastCharacter)                 // write printable ASCII characters to display
      {
        segsize_t currentBitmap = 0;                                 // the current bitmap
        byte segments = 0;                                           // all segments for this character
        segments = pgm_read_byte_near(charTable + value - 32);       // the table starts with the first printable character at 32
        // step 3: combine the segmentCharacterMapping with the pixels from the users pixelSegmentMapping
        for (byte i = 0; i < 8; i++) {
          if (segments & (1UL << i))                                 // UL not necessary, but uses less Flash than if you leave it away
            currentBitmap |= segment[i];
        }
        writeLowLevel(currentPosition, currentBitmap);
        lastPosition = currentPosition;                              // remember this position just in case next print would be a (decimal) point or comma
        currentPosition++;
        if (currentPosition >= numDigits) currentPosition = 0;       // wrap around
      }
      // 2024-08-05 tonhuisman: Disabled strip.show() to do it after writing several digits
      // strip.show();          // force strip.show() after each single write     MISSING tbd if that's really a good idea, alternative: do it manually after the last print
      return 1;              // assume sucess
    }

#else // if NEOPIXEL_DISPLAY_USE_WRITE

    size_t write(uint8_t value) {
#if NEOPIXEL_DISPLAY_DEBUG >= 3
      Serial.println("write() not implemented..."); 
#endif
      return 0;
    }

#endif // if NEOPIXEL_DISPLAY_USE_WRITE 

};


/*! \mainpage Some words to the Noiasca Neopixel Display library
 
  \section intro_sec Introduction
 
  The "Noiasca Neopixel Display" library is an addon to Adafruits Neopixel library. 
  It uses the functionalities from Adafruit and makes handling of large displays very easy. 
  Currently the main focus is the simulation of "seven segment displays". 
  You can use this library for big clocks, scoreboards and similar use cases.
  
  The API/the interface uses the official print.h library. Additional comands are roughly based on LCD API 1.0.
  For example you can use clear() to clear the display or setCursor() to write on a specific position.
  
  \section purpose_sec Mapping of Pixels to Segments
  To use this library you must assign the used pixels for each segment and define a bitmap array.
  This is explaind in detail on the homepage of the Noiasca Neopixel Display.  
 
  \section install_sec Installation
  
  \subsection step0 Download the library
  The newest version of this library can be downloaded from https://werner.rothschopf.net/202005_arduino_neopixel_display_en.htm .
 
  \section example_sec Examples
  
  There are several examples please use the hello world for the beginning.
  
  \subsection step1 Install the library
 
  In the Arduino IDE use the Menu <br>
   Sketch / Include Library / Add .ZIP Library <br>
  to install the library.
 */
 