// SPDX-FileCopyrightText: 2021 Kattni Rembor for Adafruit Industries
// SPDX-License-Identifier: MIT
/*
 * This example shows how read the potentiometer on two I2C QT Slide Potentiometers
 * and make the NeoPixels change too!
 */

#include "Adafruit_seesaw.h"
#include <seesaw_neopixel.h>

#define  DEFAULT_I2C_ADDR 0x30
#define  ANALOGIN   18
#define  NEOPIXELOUT 14

Adafruit_seesaw seesaw1;
Adafruit_seesaw seesaw2;
seesaw_NeoPixel pixels1 = seesaw_NeoPixel(4, NEOPIXELOUT, NEO_GRB + NEO_KHZ800);
seesaw_NeoPixel pixels2 = seesaw_NeoPixel(4, NEOPIXELOUT, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);

  //while (!Serial) delay(10);   // wait until serial port is opened

  Serial.println(F("Adafruit PID 5295 I2C QT Slide Potentiometer test!"));

  if (!seesaw1.begin(DEFAULT_I2C_ADDR) || !seesaw2.begin(DEFAULT_I2C_ADDR+1)) {
    Serial.println(F("seesaws not found!"));
    while(1) delay(10);
  }

  uint16_t pid;
  uint8_t year, mon, day;

  seesaw1.getProdDatecode(&pid, &year, &mon, &day);
  Serial.print("seesaw found PID: ");
  Serial.print(pid);
  Serial.print(" datecode: ");
  Serial.print(2000+year); Serial.print("/");
  Serial.print(mon); Serial.print("/");
  Serial.println(day);

  if (pid != 5295) {
    Serial.println(F("Wrong seesaw PID"));
    while (1) delay(10);
  }

  if (!pixels1.begin(DEFAULT_I2C_ADDR) || !pixels2.begin(DEFAULT_I2C_ADDR+1)){
    Serial.println("seesaw pixels not found!");
    while(1) delay(10);
  }

  Serial.println(F("seesaw started OK!"));

  pixels1.setBrightness(255);  // half bright
  pixels2.setBrightness(255);  // half bright
  pixels1.show(); // Initialize all pixels to 'off'
  pixels2.show(); // Initialize all pixels to 'off'
}



void loop() {
  // read the potentiometer
  uint16_t slide1_val = seesaw1.analogRead(ANALOGIN);
  uint16_t slide2_val = seesaw2.analogRead(ANALOGIN);
  Serial.print(slide1_val);
  Serial.print(", ");
  Serial.println(slide2_val);

  for (uint8_t i=0; i< pixels1.numPixels(); i++) {
    pixels1.setPixelColor(i, Wheel(slide1_val / 4));
    pixels2.setPixelColor(i, Wheel(slide2_val / 4));
  }
  pixels1.show();
  pixels2.show();

  delay(50);
}



// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return seesaw_NeoPixel::Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return seesaw_NeoPixel::Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return seesaw_NeoPixel::Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
