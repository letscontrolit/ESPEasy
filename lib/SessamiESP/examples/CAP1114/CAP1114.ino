#include <Arduino.h>
#include <TimeLib.h>

#include "CAP1114_Button.h"
#include "CAP1114_LED.h"
#include "CAP1114_Driver.h"

#define SESSAMI_SDA 2
#define SESSAMI_SCL 0

#define TFT_DC 16
#define TFT_CS 4

tmElements_t _time;
uint8_t debug;
uint8_t debug2;

//button constructor needs I2C connection
Sessami_Button *button;
Sessami_LED *led;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;  //wait for serial port to initialize
  }
  Wire.begin(SESSAMI_SDA, SESSAMI_SCL);
  //create button object after I2C setup
  button = new Sessami_Button;
  led = new Sessami_LED;

  /* Register Read Test */
  ReadTest();
}

void loop() {
  Background();
  button->UpdateBut();

  ButTest();
}

void Background() {
  if (second() != _time.Second) {
    _time.Second = second();
    //Update per second
    button->HeldCount();
    button->HoldCount();
  }
}

void ButTest() {
  //if (*button == B_PROX)
    //Serial.println("PROX");
  if (*button == B_UP)
    Serial.println("UP");
  if (*button == B_DOWN)
    Serial.println("DOWN");
  if (*button == B_POWER)
    Serial.println("POWER");
  if (*button == B_LEFT)
    Serial.println("LEFT");
  if (*button == B_MID)
    Serial.println("MID");
  if (*button == B_RIGHT)
    Serial.println("RIGHT");

  if (*button == S_LEFT)
    Serial.println("Slider LEFT");
  if (*button == S_RIGHT)
    Serial.println("Slider RIGHT");
}

void ReadTest() {
  //Main Status Control Register
  Serial.print("Main Status Control: ");
  Serial.println();

  //Proximity Control Register
  Serial.print("Proximity Control: ");
  Serial.print(button->GetPROXEn());
  Serial.print(" ");
  Serial.println(button->GetPROXSen());

  //Slider Position / Volumetric Data Register
  Serial.print("Slider Position / Volumetric Data Register: ");
  Serial.println();
}

