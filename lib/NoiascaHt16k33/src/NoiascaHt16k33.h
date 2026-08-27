/* *******************************************************************
  HT16K33 Library
  
  copyright (c) noiasca
  
  Version 1.2.0    2024-09-13
  
  History:
  2021-03-14       extract fonts in separate file
  2020-06-05       on(), off()
  2020-06-04       Wire.begin() must be called in the user sketch. A simple hook to catch on AVR platform is implemented
  2020-09-19       fix for ESP8266
 * ******************************************************************/
 
 /*! 
 
  \mainpage Some words to the Noiasca HT16K33 Library
 
  \section intro_sec Introduction
  The "Noiasca HT16K33 Library" is an Arduino Library to control
  HT16K33 based seven segment and 14 segment LED displays.
  
  The library inherits from Print.h and therefore supports the very common interface you know from other display libraries.
  Additionally the interface is very close to the so called LCD API 1.0
    
  \section example_sec Examples
  There are several examples please use the hello world for the beginning.
  
  \section install_sec Install the library
  Download the library from http://werner.rothschopf.net/201909_arduino_ht16k33.htm
  
  In the Arduino IDE use the Menu <br>
   Sketch / Include Library / Add .ZIP Library <br>
  to install the library.
 */

#pragma once
#include <Wire.h>
#include <Print.h>
#include <Arduino.h>

#if defined(__AVR__)
#include <avr/pgmspace.h>
#elif defined(ESP8266)
#include <pgmspace.h>
#endif

/* *******************************************************************
        Settings
 * ******************************************************************/

#define HT16K33_DEBUG 0                // Library debug messages: 0 off    1 important/error     2 warning    3 info/debug   (mode 1 costs about 230 byte)
//#define HT16K33_BUFFERSIZE 8 * 8       // currently the bitmap Buffer is not needed. Could be used for scrolling text.

/* *******************************************************************
         HW Definitions of a bitmask for each LED segment:
 * ******************************************************************/
// 7 Segment
// This is the most common segment definition
// match LED segment of display to hardware
//                       .GFEDCBA
const uint8_t SEG_A  = 0b00000001;
const uint8_t SEG_B  = 0b00000010;
const uint8_t SEG_C  = 0b00000100;
const uint8_t SEG_D  = 0b00001000;
const uint8_t SEG_E  = 0b00010000;
const uint8_t SEG_F  = 0b00100000;
const uint8_t SEG_G  = 0b01000000;
const uint8_t SEG_DP = 0b10000000;

// 14 segment
//                        .PONMLKHGFEDCBA
const uint16_t SEG14_A  = (1<<0);
const uint16_t SEG14_B  = (1<<1);
const uint16_t SEG14_C  = (1<<2);
const uint16_t SEG14_D  = (1<<3);
const uint16_t SEG14_E  = (1<<4);
const uint16_t SEG14_F  = (1<<5);
const uint16_t SEG14_G  = (1<<6);
const uint16_t SEG14_H  = (1<<7);
const uint16_t SEG14_K  = (1<<8);
const uint16_t SEG14_L  = (1<<9);
const uint16_t SEG14_M  = (1<<10);
const uint16_t SEG14_N  = (1<<11);
const uint16_t SEG14_O  = (1<<12);
const uint16_t SEG14_P  = (1<<13);
const uint16_t SEG14_DP = (1<<14);

/* *******************************************************************
   Fonts
   Beginning with Version 1.0.3 the fonts are extracted to separate files
   if you don't like the existing characters duplicate the file, rename it and 
   include your modified file instead.
   The concept how to include separte files might change in a newer version
 * ******************************************************************/

//extern const uint8_t charTable7[];      // tbc https://forum.arduino.cc/index.php?topic=659107.0
//extern const uint16_t charTable14[];
 
#ifndef FONTFILE7
#include "fonts/font7legacy.h"
#endif

#ifndef FONTFILE14                        // include exact ONE 14 segment font
//#include "fonts/font14legacy.h"
#include "fonts/font14default.h"
//#include "fonts/font14netherlands.h"
#endif

/* *******************************************************************
         no need to change something below here
 * ******************************************************************/
#define HT16K33_OSCILATOR_ON    0x21   // System Setup Register 0x20 + 0x01
#define HT16K33_CMD_BRIGHTNESS  0xE0   // Dimming set (Datasheet says EF, but this is just with the set bits D12-D8 for full brightness
#define HT16K33_BLINK_CMD       0x80   // Display Setup - blink and on/off
#define HT16K33_BLINK_DISPLAYON 0x01
#define HT16K33_BLINK_OFF       0
#define HT16K33_BLINK_2HZ       1
#define HT16K33_BLINK_1HZ       2
#define HT16K33_BLINK_HALFHZ    3
//#define HT16K33_KEYDATA 0x40         // Key data Adress pointer - unused
//#define HT16K33_INTFLAG 0x60         // INT flag Adress pointer - unused
//#define HT16K33_ROWINT 0xA0          // ROW/INT set - unused
//#define HT16K33_TESTMODE 0xD9        // Testmode - unused

// Error Codes
#define HT16K33_ERR_ADDR _BV(2)        // chip not responding at adress 0b00000100 = 4
//#define HT16K33_ERR_CHIP _BV(3)      // chip signature or a specific register doesn't fit

/* *******************************************************************
   Classes
 * ******************************************************************/

/** \brief Base class for all HT16K33 displays

    This is an (abstract) class used as base for all implementations
    It holds all common variables and member functions.
    If you create your instance (your object), don't use this base class but one of implementations
    \see Noiasca_ht16k33_hw_7
    \see Noiasca_ht16k33_hw_7_4_c
    \see Noiasca_ht16k33_hw_14
    \see Noiasca_ht16k33_hw_14_4
    \see Noiasca_ht16k33_hw_14_ext
*/ 
 class Noiasca_ht16k33 : public Print{                               // Base class for all HT16K33 displays
  public:
    Noiasca_ht16k33(void);
    /** 
       \brief initialise hardware
       
       Put this method in your setup
       \param i2c_addr    the I2C address of the display
       \param numDevices  the number of used devices
       \returns 0 on success
     */    
    uint8_t begin(uint8_t i2c_addr, uint8_t numDevices = 1);
    
    /** 
      \brief set the blink rate of the display
    */
    void blinkRate(uint8_t b);
    
    /** 
      \brief Clear the display and place cursor to 0
    
      Clears the display and sets the cursor to 0.
    */ 
    void clear(void);
    
    /** 
      \brief Turn the display off
    */
    void off(void);
    
    /** 
      \brief Turn the display on
    */    
    void on(void);
    
    /** 
      \brief check if all ICs are responding
      \return true if all ICs are responding
    */
    bool isConnected(void);
    
    /** 
      \brief set the brightness of the display
      \param b new brightness from 0..15
    */ 
    void setBrightness(uint8_t b);  
    
    /** 
      \brief set the cursor for the next writing position
      \param newPosition the new position for the cursor
    */ 
    void setCursor(uint8_t newPosition);
    
    /** 
      \brief get the cursor for the next writing position
    */ 
    uint8_t getCursor();
    
    /** 
      \brief set the number of digits per device
      \param newDigits modify the used digits
    */ 
    void setDigits(uint8_t newDigits); 
    
    /** 
      \brief write to I2C
      
      I2C lowLevel write of a bitmask to the IC to a specific position
      \param position digit to be written to
      \param bitmask the bits/segments to be activated as bitmask
    */ 
    void writeLowLevel(uint8_t position, uint16_t bitmask);
    //size_t write(uint8_t);                                         // write method is depending on the HW and therefore in the first HW child classes
    //using Print::write;                                            
#ifdef HT16K33_BUFFERSIZE                                            
    void writeChar(uint8_t device, uint16_t digit);                  // I2C Ausgabe eines Zeichen aus Displaybuffer
#endif

    virtual uint16_t getCharacterBitmap(uint8_t value);

    virtual void setZeroSlash(bool state);

	// not supported LCD API 1.0 functions
	// ===================================
	// these function are defined in the LCD API 1.0 API, 
  // but as this is not a LCD library, these functions are marked as deprecated
	// note: only very new versions of gcc support setting warning message
	// it breaks on on older versions that shipped with older 1.x IDEs
	// so test for gcc 4.5 or greater for better deprecated messages

#if ( __GNUC__ >= 4) && (__GNUC_MINOR__ >= 5)
	inline void __attribute__((deprecated("Use clear() instead")))
	 home() {clear();}
  inline void __attribute__((deprecated("Use begin() instead"))) 
   init(uint8_t i2c_addr, uint8_t numDevices = 1) {begin(i2c_addr, numDevices);}
#else
	inline void __attribute__((deprecated))
	 home() {clear();}
  inline void __attribute__((deprecated))
	 init(uint8_t i2c_addr, uint8_t numDevices = 1) {begin(i2c_addr, numDevices);}
#endif

  protected:
    uint8_t _numDigits = 8;                                // digits per device
    uint8_t _i2c_addr;                                     // base adress of first device (0)
    uint8_t _numDevices;                                   // total number of devices
#ifdef HT16K33_BUFFERSIZE
    uint16_t displaybuffer[HT16K33_BUFFERSIZE];            // stores printed bitmaps
    uint8_t _currentBufferPosition;                        // current read position in buffer
#endif
    uint8_t _currentPosition;                              // current position of cursor
    uint8_t _lastPosition;                                 // last position of cursor
    bool    _zeroSlash = false;                            // Apply / to the 0 for readability
};

/** \brief class for  HT16K33 displays with 7 segment LEDs

    This is for displays with 7 segment LEDs
*/ 
class Noiasca_ht16k33_hw_7 : public Noiasca_ht16k33 {      // 7 segment display
  public:
    Noiasca_ht16k33_hw_7();
    /**
      activates/deactivtes a separate colon digit
      \param activate set to 1 if you want to display the colon
    */
    void setColonDigit(uint8_t activate = 1);               
    size_t write(uint8_t);                                 // write using a byte char table
    using Print::write;                                    
    uint16_t getCharacterBitmap(uint8_t value);
    // using Noiasca_ht16k33::getCharacterBitmap;
    void setZeroSlash(bool state);
  protected:
    uint8_t _hasColonDigit;                                // is set to 1 if display has a separate colon digit at digit 2 (the third column). Is on this level to have one common .write() method
    uint8_t _lastBitmap;                                   // last written bitmap (if we have to reprint for the decimal point)
};

/** \brief class for  HT16K33 displays with 7 segment LEDs

    This is for displays with 7 segment LEDs, 4 digits and a colon.
    For example Adafruit 0.56" 4-Digit 7-Segment Display w/I2C Backpack
*/ 
class Noiasca_ht16k33_hw_7_4_c : public Noiasca_ht16k33_hw_7 {       // 7 Segment Display with 4 visible Digits and Colon Sign. In fact this display has 5 colons
  public:
    Noiasca_ht16k33_hw_7_4_c();
    /**
      activate the colon digit
      \param device the device you want to control the colon
      \param colon TRUE will switch on, FALSE will switch off the colon
    */
    void showColon(uint8_t device, bool colon);            // switch on / off colon for one specific device
    void showColon(bool colon);                            // switch on / off colon for all devices
};

/** \brief class for  HT16K33 displays with 14 segment LEDs

    This is for displays with up to eight 14 segment LEDs
    For example the "HT16K33 AlphaNumeric 0.54" 8-Digit 14 Segment LED I2C Interface" displays from WtihK
*/ 
class Noiasca_ht16k33_hw_14 : public Noiasca_ht16k33 {     // 14 Segment Display
  public:
    Noiasca_ht16k33_hw_14();
    size_t write(uint8_t);                                 // write using a word char table (14bit char table)
    using Print::write; 
    uint16_t getCharacterBitmap(uint8_t value);
    // using Noiasca_ht16k33::getCharacterBitmap;
    void setZeroSlash(bool state);
  protected:
    uint16_t _lastBitmap;                                  // last written bitmap (if we have to reprint for the decimal point)
};

/** \brief class for  HT16K33 displays with 14 segment LEDs

    This is for displays with four 14 segment LEDs
    For example the Adafruit Quad Alphanumeric Display - 0.54" Digits w/ I2C Backpack
*/ 
class Noiasca_ht16k33_hw_14_4 : public Noiasca_ht16k33_hw_14 {       // 14 Segment Display with 4 digits only
  public:
    Noiasca_ht16k33_hw_14_4();
};

/** \brief class for  HT16K33 displays with 14 segment LEDs

    This is for displays with up to eight 14 segment LEDs.
    This class can be used to scroll the text in the display.
*/ 
class Noiasca_ht16k33_hw_14_ext : public Noiasca_ht16k33_hw_14 {
  public:
    Noiasca_ht16k33_hw_14_ext();
/** \brief scroll the text

    The tick/run/do method for scrolling text.
    Call this function in your loop().
*/     
    uint32_t scroll();
    
/** \brief set scroll interval

    Set the speed of the scroll effect. 
    \param newInterval speed in milliseconds
*/    
    void setScrollInterval(uint16_t newInterval);
    
/** \brief set wait time between scrolling

    sets the wait/timeout after all letters are scrolled to the display
    \param newWait the new wait time in milliseconds
*/      
    void setScrollWait(uint16_t newWait);
    
/** \brief set a scrolltext   

    sets the new scrolltext to be displayed and scrolled
    \param *newText the new text (or number)
*/    
    void setScrolltext(const char *newText);          
    void setScrolltext(int32_t myNumber);  
    void setScrolltext(double myNumber);                   // sets a new scrolltext - a float
    
  private:
    uint16_t interval = 400;                               // shift interval
    uint16_t wait = 3000;                                  // wait time after last letter
    uint32_t previousMillis = 0;                           // stores last print out
    uint8_t counter = 1;                                   // used as start position
    static const uint8_t scrollBufferSize = 65;            // maximum size of scroll text
    char myText[scrollBufferSize] =  {'\0'};               // buffer for the scroll text
    uint32_t rounds = 0;                                   // how many scroll overs where already performed with the current scroll text
};
