/*******************************************************************************
   Noiasca Neopixel Display
   03: Counter

   Just a simple counter, counting upwards
   
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

typedef uint16_t segsize_t;                               // fit variable size to your needed pixels. uint16_t --> max 16 Pixel per digit
const segsize_t segment[8] {
  0b0000000000000011,  // SEG_A
  0b0000000000001100,  // SEG_B
  0b0000000000110000,  // SEG_C
  0b0000000011000000,  // SEG_D
  0b0000001100000000,  // SEG_E
  0b0000110000000000,  // SEG_F
  0b0011000000000000,  // SEG_G
  0b1100000000000000   // SEG_DP if you don't have a decimal point, just leave it zero
};

const uint16_t ledCount(pixelPerDigit * numDigits + addPixels);      // keeps track of used pixels

#include <Adafruit_NeoPixel.h>                                       // install Adafruit library from library manager
Adafruit_NeoPixel strip(ledCount, ledPin, NEO_GRB + NEO_KHZ800);     // create neopixel object like you commonly used with Adafruit

#include <Noiasca_NeopixelDisplay.h>                                 // download library from: http://werner.rothschopf.net/202005_arduino_neopixel_display_en.htm
Noiasca_NeopixelDisplay display(strip, segment, numDigits, pixelPerDigit, addPixels);     // create display object, handover the name of your strip as first parameter!

void setup() {
  Serial.begin(115200);
  Serial.println(F("\nNoiascaNeopixelDisplay\n03 counter"));

  strip.begin();                   // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();                    // Turn OFF all pixels ASAP
  strip.setBrightness(50);         // Set BRIGHTNESS to about 1/5 (max = 255)
}

void loop() {
  // just counting upwards
  for (uint16_t i = 0; i < pow(10, numDigits); i++) {
    display.clear();
    display.print(i);
    Serial.println(i);
    delay(100);
  }
}
