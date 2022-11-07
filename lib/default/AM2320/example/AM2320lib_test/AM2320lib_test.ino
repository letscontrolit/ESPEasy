#include <Wire.h>
#include <AM2320.h>

AM2320 th;

void setup() {
  Serial.begin(9600);
  Wire.begin();
}

void loop() {
  switch(th.Read()) {
    case 2:
      Serial.println("CRC failed");
      break;
    case 1:
      Serial.println("Sensor offline");
      break;
    case 0:
      Serial.print("humidity: ");
      Serial.print(th.h);
      Serial.print("%, temperature: ");
      Serial.print(th.t);
      Serial.println("*C");
      break;
  }

  delay(200);
}
