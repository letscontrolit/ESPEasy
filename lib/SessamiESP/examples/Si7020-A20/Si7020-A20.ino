#include <Arduino.h>
#include "Si7020.h"

Si7020 tp;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;  //wait for serial port to initialize
  }
  Wire.begin(2, 0);
  delay(500);
}

void loop() {
  Serial.print("RH : ");
  Serial.println(tp.GetRH());
  Serial.print("Tp : ");
  Serial.println(tp.GetTp());
  Serial.print("UserRegister1 : ");
  Serial.println(tp.ReadUserReg1(), 2);
  Serial.print("HeaterControl Register : ");
  Serial.println(tp.ReadHCReg(), 2);
  delay(100);
  Serial.println("clrscr");
}

