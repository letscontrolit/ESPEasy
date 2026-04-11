#include <Adafruit_SSD1306.h>
#include "Adafruit_VEML7700.h"

Adafruit_VEML7700 veml = Adafruit_VEML7700();
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

void setup() {
  Serial.begin(115200);
  //while (!Serial);
  Serial.println("VEML7700 demo");

  if (veml.begin()) {
    Serial.println("Found a VEML7700 sensor");
  } else {
    Serial.println("No sensor found ... check your wiring?");
    while (1);
  }

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  delay(500); // Pause for 2 seconds
  display.setTextSize(2);
  display.setTextColor(WHITE);

  veml.setGain(VEML7700_GAIN_1);
  veml.setIntegrationTime(VEML7700_IT_100MS);
}


void loop() {
  display.clearDisplay();
  display.setCursor(0,8);
  display.print("Lux "); display.println(veml.readLux());
  display.display();
  delay(50);
}
