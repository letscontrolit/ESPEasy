/* Noiasca Neopixel Display
   20: additional pixels
   
   http://werner.rothschopf.net/202005_arduino_neopixel_display_en.htm

   by noiasca
   2020-05-04
*/

const byte ledPin = 12;                // Which pin on the Arduino is connected to the NeoPixels?
const byte numDigits = 4;              // How many digits (numbers) are available on your display
const byte pixelPerDigit = 16;         // all pixels, including decimal point pixels if available at each digit
const byte startPixel = 0;             // start with this pixel on the strip
const byte addPixels = 2;              // unregular additional pixels to be added to the strip (e.g. a double point for a clock 12:34)

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

typedef uint16_t segsize_t;            // fit variable size to your needed pixels. uint16_t --> max 16 Pixel per digit
const segsize_t segment[8] { 
  0b0000000000000011,                  // SEG_A
  0b0000000000001100,                  // SEG_B
  0b0000000000110000,                  // SEG_C
  0b0000000011000000,                  // SEG_D
  0b0000001100000000,                  // SEG_E
  0b0000110000000000,                  // SEG_F
  0b0011000000000000,                  // SEG_G
  0b1100000000000000                   // SEG_DP   if you don't have a decimal point, just leave it zero
};          

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

void blinkSecond() {
  static uint32_t previousTimestamp = 0;
  static bool state = 0;
  uint32_t actualTimestamp = millis() / 500;
  if (actualTimestamp != previousTimestamp) {
    previousTimestamp = actualTimestamp;
    display.setPixelColor(pixelPerDigit * 2 + 0, 0x0000FF * state);  // Assumption: the additional pixel is after two digits
    display.setPixelColor(pixelPerDigit * 2 + 1, 0x0000FF * state);  // Assumpiton: there are two additional pixels
    display.show();
    state = !state;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("\nNoiascaNeopixelDisplay\n20 additional pixels"));
  
  strip.begin();                       // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();                        // Turn OFF all pixels ASAP
  strip.setBrightness(50);             // Set BRIGHTNESS to about 1/5 (max = 255)

  Serial.println(F("Print 1234 on your display"));
  display.print("12");
  display.print("34");
}

void loop() {
  // put here other code which needs to run:
  blinkSecond();
}
