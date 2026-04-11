/* 
   Noiasca Neopixel Display
   64 large display 64 bit - with lot of pixels per segment
   
   There was a user request, for a display with 7 pixels per segment,
   which will give 35 pixels per digit.
   obviously, 35 pixels don't fit in a 32bit bitmap, therefore you you must
   use a unsigned 64bit variable for the segsize_t

   64bit variables come with several restrictions. For example you will not be able to 
   print such a large variable to the serial, because it's not implemented in the print.h class.

   Theoretically 9pixels x 7segments = 63 would fit into a 64bit Variable, but be careful with 
   any bitshift you will need in your sketch.

   Warning: There a two default makros:

   _BV is part of Libc. The intended use is to create bitmasks for manipulating register values. 
   #define _BV(bit) (1 << (bit))

   bit() is part of the Arduino implementation: 
   #define bit(b) (1UL << (b))

   so non of them is good for 64bit. Therefore you must define our own makro if needed (see the code below)

   http://werner.rothschopf.net/202005_arduino_neopixel_display_en.htm
   
   by noiasca
   2021-05-44 4958/332
*/

const byte ledPin = 12;                // Which pin on the Arduino is connected to the NeoPixels?
const byte numDigits = 2;              // How many digits (numbers) are available on your display
const byte pixelPerDigit = 39;         // all pixels, including decimal point pixels if available at each digit
const byte startPixel = 0;             // start with this pixel on the strip
const byte addPixels = 4;              // unregular additional pixels to be added to the strip (e.g. a double point for a clock 12:34)

/*
   Segments are named and orded like this

          SEG_A
   SEG_F         SEG_B
          SEG_G
   SEG_E         SEG_C
          SEG_D

  in the following constants you have to define
  which pixel number belongs to which segment
*/

typedef uint64_t segsize_t;            // define the type of segsize_t to 64bit

#define B64(b) (1ULL << (b))           // remark: don't use the _BV(n) nor bit() makro for 64 bit variables

const segsize_t segment[8] {
  B64(0) | B64(1) | B64(2) | B64(3) | B64(4),              // SEG_A
  B64(5) | B64(6) | B64(7) | B64(8) | B64(9),              // SEG_B
  B64(10) | B64(11) | B64(12) | B64(13) | B64(14),         // SEG_C
  B64(15) | B64(16) | B64(17) | B64(18) | B64(19),         // SEG_D
  B64(20) | B64(21) | B64(22) | B64(23) | B64(24),         // SEG_E
  B64(25) | B64(26) | B64(27) | B64(28) | B64(29),         // SEG_F
  B64(30) | B64(31) | B64(32) | B64(33) | B64(34),         // SEG_G
  B64(35) | B64(36) | B64(37) | B64(38),                   // SEG_DP
};

/*
// yes you can use the bit notation but it might becomes VERY LONG if you need 64 bits

const segsize_t segment[8] { 
  0b00000000000000000000000000000011111,  // SEG_A
  0b00000000000000000000000001111100000,  // SEG_B
  0b00000000000000000000111110000000000,  // SEG_C
  0b00000000000000011111000000000000000,  // SEG_D
  0b00000000001111100000000000000000000,  // SEG_E
  0b00000111110000000000000000000000000,  // SEG_F
  0b11111000000000000000000000000000000,  // SEG_G
  0                                       // SEG_DP   if you don't have a decimal point, just leave it zero
};          
*/

const uint16_t ledCount(pixelPerDigit * numDigits + addPixels);

int offsetLogic_cb(uint16_t position)                                // your callback function to keep track of additional pixels
{
  uint16_t offset = 0;
  if (position > 1 ) offset = addPixels;                             // example: the additional Pixels are between 2nd and 3rd digit
  return offset;                                                     // you MUST return a value. It can be 0
}

#include <Adafruit_NeoPixel.h>                                       // install Adafruit library from library manager
Adafruit_NeoPixel strip(ledCount, ledPin, NEO_GRB + NEO_KHZ800);     // create neopixel object like you commonly used with Adafruit

#include <Noiasca_NeopixelDisplay.h>                                 // download library from: http://werner.rothschopf.net/202005_arduino_neopixel_display_en.htm
Noiasca_NeopixelDisplay display(strip, segment, numDigits, pixelPerDigit, startPixel, addPixels, offsetLogic_cb); // create display object, handover the name of your strip as first parameter!


void setup() {
  Serial.begin(115200);
  Serial.println(F("\nNoiascaNeopixelDisplay\n"));

  strip.begin();                   // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();                    // Turn OFF all pixels ASAP
  strip.setBrightness(30);         // Set BRIGHTNESS (max = 255)
}

void loop() {
    display.clear();
    display.setColorFont(0x008800);
    display.print("12");
    delay(3000);

    display.setColorFont(0x880000);
    display.print("34");
    delay(3000);
}
