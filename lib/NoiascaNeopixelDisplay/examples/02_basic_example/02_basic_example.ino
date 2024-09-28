/*******************************************************************************
   Noiasca Neopixel Display
   02 basic example

   Shows the basic print functions
   
   http://werner.rothschopf.net/202005_arduino_neopixel_display_en.htm

   by noiasca
   2020-05-04
 *******************************************************************************/

const byte ledPin = 12;                // Which pin on the Arduino is connected to the NeoPixels?
const byte numDigits = 4;              // How many digits (numbers) are available on your display
const byte pixelPerDigit = 16;         // all Pixel, including decimal point pixels if available at each digit
const byte addPixels = 0;              // unregular additional pixels to be added to the strip (e.g. a double point for a clock 12:34)

/*
   Segments are named and orded like this

          SEG_A
   SEG_F         SEG_B
          SEG_G
   SEG_E         SEG_C
          SEG_D          SEG_DP

  in the following constant array you have to define
  which pixels belong to which segment
*/

/*
  typedef uint8_t segsize_t; // fit variable size to your needed pixels. uint8_t --> max 8 Pixel per digit
  const segsize_t segment[8] {
    0b00000001,              // SEG_A
    0b00000010,              // SEG_B
    0b00000100,              // SEG_C
    0b00001000,              // SEG_D
    0b00010000,              // SEG_E
    0b00100000,              // SEG_F
    0b01000000,              // SEG_G
    0b10000000               // SEG_DP if you don't have a decimal point, just leave it zero
  };
*/

typedef uint16_t segsize_t;  // fit variable size to your needed pixels. uint16_t --> max 16 Pixel per digit
const segsize_t segment[8] { 
  0b0000000000000011,        // SEG_A
  0b0000000000001100,        // SEG_B
  0b0000000000110000,        // SEG_C
  0b0000000011000000,        // SEG_D
  0b0000001100000000,        // SEG_E
  0b0000110000000000,        // SEG_F
  0b0011000000000000,        // SEG_G
  0b1100000000000000         // SEG_DP if you don't have a decimal point, just leave it zero
};

/*
  // match LED Segment of Display to Hardware   - Muss zur Pixel-Anordnung der Hardware passen
  // 3 individual Pixel per Segment:
  typedef uint32_t segsize_t;                              // fit variable size to your needed pixels. uint32_t --> max 32 Pixel per digit
  const segsize_t segment[8] { 
    _BV(2) | _BV(1) | _BV(0),     // SEG_A    // n of maximal 32 bits pixels per Segment
    _BV(5) | _BV(4) | _BV(3),     // SEG_B
    _BV(8) | _BV(7) | _BV(6),     // SEG_C
    _BV(11) | _BV(10) | _BV(9),   // SEG_D
    _BV(14) | _BV(13) | _BV(12),  // SEG_E
    _BV(17) | _BV(16) | _BV(15),  // SEG_F
    _BV(20) | _BV(19) | _BV(18),  // SEG_G
    _BV(21)                       // SEG_DP
  };
*/

const uint16_t ledCount(pixelPerDigit * numDigits + addPixels);      // keeps track of used pixels
#include <Adafruit_NeoPixel.h>                                       // install Adafruit library from library manager
Adafruit_NeoPixel strip(ledCount, ledPin, NEO_GRB + NEO_KHZ800);     // create neopixel object like you commonly used with Adafruit 

#include <Noiasca_NeopixelDisplay.h>                                 // include the library after your segment definitions. download library from: http://werner.rothschopf.net/202005_arduino_neopixel_display_en.htm
Noiasca_NeopixelDisplay display(strip, segment, numDigits, pixelPerDigit, addPixels);     // create display object, handover the name of your strip as first parameter!

void setup()
{
  Serial.begin(115200);
  Serial.println(F("\nNoiascaNeopixelDisplay\n02 basic example"));

  strip.begin();                       // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();                        // Turn OFF all pixels ASAP
  strip.setBrightness(50);             // Set BRIGHTNESS to about 1/5 (max = 255)

  Serial.println(F("print float"));
  display.clear();
  display.print(12.3, 1);              // print prints floats by default with two decimals. If you just need one decimal, use the print functionality to print only one decimal so if you receive 
  delay(1000);

  Serial.println(F("direct adress individual digits with low level method"));
  display.writeLowLevel(0, 255);                 // switch on the first 8 segment of digit 0
  display.setColorFont(0x00FF00);
  display.writeLowLevel(1, 0b10101010);          // 4 pixels of digit 1
  display.setColorFont(0x0000088);
  display.writeLowLevel(2, 0b01010101);          // 4 pixels of digit 1
  display.show();                                // send data to stripe - needed because writeLowLevel doesn' show data by default
  delay(1000);

  Serial.println(F(".write() ASCII Codes (if printable)"));
  display.clear();                     // delete content on display
  display.write(48);                   // 0      // basic write method, expects ASCII code, ends with an .show() by default
  display.write(49);                   // 1
  display.write(50);                   // 2
  display.write(51);                   // 3
  delay(1000);

  Serial.println(F("set a new Color and use print method - easy way"));
  display.clear();
  display.setColorFont(0x00FF00);                // change color of font (active segments)
  display.print(123);                            // print an integer on the display
}

void loop()
{
  // put here other code which needs to run:

  display.clear();
  display.setColorFont(0x00AA00);
  display.print(8888);
  delay(2000);

  display.clear();
  display.setColorFont(0x00AA88);
  display.print(1111);
  delay(2000);

  display.clear();
  display.setColorFont(0xAAAA00);
  display.print(random(10000));
  display.setPixelColor(random(40), 0x0000ff);   // direct access a pixel
  display.show();
  delay(2000);
}
