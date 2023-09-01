#include "Adafruit_NeoKey_1x4.h"
#include "seesaw_neopixel.h"

Adafruit_NeoKey_1x4 neokey;  // Create the NeoKey object

void setup() {
  Serial.begin(115200);
  while (! Serial) delay(10);
   
  if (! neokey.begin(0x30)) {     // begin with I2C address, default is 0x30
    Serial.println("Could not start NeoKey, check wiring?");
    while(1) delay(10);
  }
  
  Serial.println("NeoKey started!");

  // Pulse all the LEDs on to show we're working
  for (uint16_t i=0; i<neokey.pixels.numPixels(); i++) {
    neokey.pixels.setPixelColor(i, 0x808080); // make each LED white
    neokey.pixels.show();
    delay(50);
  }
  for (uint16_t i=0; i<neokey.pixels.numPixels(); i++) {
    neokey.pixels.setPixelColor(i, 0x000000);
    neokey.pixels.show();
    delay(50);
  }
}

void loop() {
  uint8_t buttons = neokey.read();

  // Check each button, if pressed, light the matching neopixel
  
  if (buttons & (1<<0)) {
    Serial.println("Button A");
    neokey.pixels.setPixelColor(0, 0xFF0000); // red
  } else {
    neokey.pixels.setPixelColor(0, 0);
  }

  if (buttons & (1<<1)) {
    Serial.println("Button B");
    neokey.pixels.setPixelColor(1, 0xFFFF00); // yellow
  } else {
    neokey.pixels.setPixelColor(1, 0);
  }
  
  if (buttons & (1<<2)) {
    Serial.println("Button C");
    neokey.pixels.setPixelColor(2, 0x00FF00); // green
  } else {
    neokey.pixels.setPixelColor(2, 0);
  }

  if (buttons & (1<<3)) {
    Serial.println("Button D");
    neokey.pixels.setPixelColor(3, 0x00FFFF); // blue
  } else {
    neokey.pixels.setPixelColor(3, 0);
  }  

  neokey.pixels.show();
  
  delay(10);    // don't print too fast
}



/******************************************/

// Input a value 0 to 255 to get a color value.
// The colors are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return seesaw_NeoPixel::Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return seesaw_NeoPixel::Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return seesaw_NeoPixel::Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  return 0;
}
