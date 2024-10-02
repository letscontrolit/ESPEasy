/* Noiasca Neopixel Display
   30 clock two displays
   handling HH MM SS as three separate displays
   
   http://werner.rothschopf.net/202005_arduino_neopixel_display_en.htm
   
   by noiasca
   2021-11-21
*/

const byte ledPin = 12;                // Which pin on the Arduino is connected to the NeoPixels?
const byte numDigits = 2;              // How many digits (numbers) are available on each display
const byte startPixelHH = 0;           // the display starts with HH with the first pixel
const byte startPixelMM = 32;          // the MM are in the middle (start at 0 + digits for 10H + digits for H + 2 for the colon)
const byte startPixelSS = 64;          // seconds are on the far right, therefore we need the highest pixel offset
const byte pixelPerDigit = 15;         // all pixels, including decimal point pixels if available at each digit
const byte addPixels = 4;              // unregular additional pixels to be added to the strip (e.g. a double point for a clock 12:34:56)

const uint16_t ledCount(pixelPerDigit * numDigits * 3 + addPixels);
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
// in this sketch we handle HH and MM as two individual displays:
Noiasca_NeopixelDisplay displayHH(strip, segment, numDigits, pixelPerDigit, startPixelHH);  // create display object, handover the name of your strip as first parameter!
Noiasca_NeopixelDisplay displayMM(strip, segment, numDigits, pixelPerDigit, startPixelMM);  // create display object, handover the name of your strip as first parameter!
Noiasca_NeopixelDisplay displaySS(strip, segment, numDigits, pixelPerDigit, startPixelSS);  // create display object, handover the name of your strip as first parameter!

void blinkColon() {
  static uint32_t previousTimestamp = 0;
  static bool state = 0;
  uint32_t actualTimestamp = millis() / 500;
  if (actualTimestamp != previousTimestamp)
  {
    previousTimestamp = actualTimestamp;
    strip.setPixelColor(pixelPerDigit * 2 + 0, 0x0000FF * state);  
    strip.setPixelColor(pixelPerDigit * 2 + 1, 0x0000FF * state);  
    strip.setPixelColor(pixelPerDigit * 4 + 2, 0x0000FF * state);  
    strip.setPixelColor(pixelPerDigit * 4 + 3, 0x0000FF * state);
    strip.show();
    state = !state;
  }
}

void updateSecond() {
  static uint32_t previousSecond = -1;
  uint32_t actualSecond = (millis() / 1000UL) % 60 ;
  if ( actualSecond != previousSecond) {
    previousSecond = actualSecond;
    Serial.println(actualSecond);
    displaySS.setCursor(0);
    if (actualSecond < 10) displaySS.print("0");
    displaySS.print(actualSecond);
  }
}

void updateMinute() {
  static uint32_t previousMinute = -1;
  uint32_t actualMinute = (millis() / 1000UL) / 60 % 60 ;
  if ( actualMinute != previousMinute) {
    previousMinute = actualMinute;
    Serial.println(actualMinute);
    displayMM.setCursor(0);
    if (actualMinute < 10) displayMM.print("0");
    displayMM.print(actualMinute);
  }
}

void updateHour() {
  static uint32_t previousHour = -1;
  uint32_t actualHour = (millis() / 1000UL) / 3600 % 24 ;
  if ( actualHour != previousHour) {
    previousHour = actualHour;
    Serial.println(actualHour);
    displayHH.setCursor(0);
    if (actualHour < 10) displayHH.print(" ");
    displayHH.print(actualHour);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("\nNoiascaNeopixelDisplay\n30 clock two displays"));

  strip.begin();                   // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();                    // Turn OFF all pixels ASAP
  strip.setBrightness(50);         // Set BRIGHTNESS to about 1/5 (max = 255)
  strip.clear();                   // clears the full strip (all displays assigned to this strip!)

  displayHH.setColorFont(0xAAAA00);
  displayMM.setColorFont(0x880044);

  Serial.println(F("Test: 888888 on display"));
  displayHH.print("88");
  displayMM.print("88");
  displaySS.print("88");
  delay(1000);
}

void loop() {
  blinkColon();
  updateSecond();
  updateMinute();
  updateHour();
  // put here other code which needs to run:
}
