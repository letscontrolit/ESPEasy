/* Noiasca Neopixel Display
   40 scoreboard
   a simple scoreboard for two players (A and B)
   separate displays for A and B on the same strip pin
   
   Wire three buttons from the GPIO to GND. Input pullups are activated
   
   http://werner.rothschopf.net/202005_arduino_neopixel_display_en.htm

   by noiasca
   2020-05-04
*/

const byte ledPin = 12;                // Which pin on the Arduino is connected to the NeoPixels?
const byte numDigits = 2;              // How many digits (numbers) are available on each display
const byte pixelPerDigit = 14;         // all pixels, including decimal point pixels if available at each digit
const byte addPixels = 4;              // unregular additional pixels to be added to the strip

const byte startPixelA = 0;            // start pixel of display A
const byte startPixelB = 32;           // start pixel of display B (assumption: 2 x 14 used by displayA + 4 additional Pixels)

const byte buttonApin = A3;            // button pin player A, connects to GND
const byte buttonBpin = A0;            // button pin player B, connects to GND
const byte buttonResetPin = A1;        // button to reset scores, connects to GND
byte counterA;                         // counts/goals/score for player A
byte counterB;                         // counts/goals/score for player B

const uint16_t ledCount(pixelPerDigit * numDigits * 2 + addPixels);
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

typedef uint16_t segsize_t;            // fit variable size to your needed pixels. uint16_t --> max 16 Pixel per digit
const segsize_t segment[8] {
  0b0000000000000011,                  // SEG_A
  0b0000000000001100,                  // SEG_B
  0b0000000000110000,                  // SEG_C
  0b0000000011000000,                  // SEG_D
  0b0000001100000000,                  // SEG_E
  0b0000110000000000,                  // SEG_F
  0b0011000000000000,                  // SEG_G
  0b1100000000000000                   // SEG_DP if you don't have a decimal point, just leave it zero
};

#include <Adafruit_NeoPixel.h>                                       // install Adafruit library from library manager
Adafruit_NeoPixel strip(ledCount, ledPin, NEO_GRB + NEO_KHZ800);     // create neopixel object like you commonly used with Adafruit

#include <Noiasca_NeopixelDisplay.h>                                 // download library from: http://werner.rothschopf.net/202005_arduino_neopixel_display_en.htm
// in this sketch we handle displayA and displayB as two individual displays:
Noiasca_NeopixelDisplay displayA(strip, segment, numDigits, pixelPerDigit, startPixelA);  // create display object, handover the name of your strip as first parameter!
Noiasca_NeopixelDisplay displayB(strip, segment, numDigits, pixelPerDigit, startPixelB);  // create display object, handover the name of your strip as first parameter!

#include <OneButton.h>                                               // install OneButton library from library manager
OneButton buttonA(buttonApin, true);
OneButton buttonB(buttonBpin, true);
OneButton buttonReset(buttonResetPin, true);

void clickA() {
  counterA++;
  Serial.print(F("PlayerA ")); Serial.println(counterA);
  displayA.setCursor(0);
  if (counterA < 10) displayA.print(" ");
  displayA.print(counterA);
}

void clickB() {
  counterB++;
  Serial.print(F("PlayerB ")); Serial.println(counterB);
  displayB.setCursor(0);
  if (counterB < 10) displayB.print(" ");
  displayB.print(counterB);
}

void resetScore() {
  Serial.println(F("Reset Score"));
  counterA = 0;
  counterB = 0;
  displayA.clear();
  displayA.print(" 0");
  displayB.clear();
  displayB.print(" 0");
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("\nNoiascaNeopixelDisplay\n40 scoreboard two displays"));

  strip.begin();                       // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();                        // Turn OFF all pixels ASAP
  strip.setBrightness(50);             // Set BRIGHTNESS to about 1/5 (max = 255)
  strip.clear();

  displayA.setColorFont(0xAA0000);     // each display gets its own color, e.g. corresponding to the button color
  displayB.setColorFont(0x00AA00);

  Serial.println(F("test display"));
  for (byte i = 99; i > 0; i = i - 11) {
    displayA.print(i);
    displayB.print(i);
    delay(200);
  }
  displayA.print(" 0");
  displayB.print(" 0");
  buttonA.attachClick(clickA);
  buttonB.attachClick(clickB);
  buttonReset.attachLongPressStart(resetScore);
}

void loop() {
  // put here other code which needs to run:
  buttonA.tick();
  buttonB.tick();
  buttonReset.tick();
}
