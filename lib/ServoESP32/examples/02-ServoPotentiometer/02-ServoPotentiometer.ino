#include <Servo.h>

static const int servoPin = 4;
static const int potentiometerPin = 32;

Servo servo1;

void setup() {
    Serial.begin(115200);
    servo1.attach(servoPin);
}

void loop() {
    int servoPosition = map(analogRead(potentiometerPin), 0, 4096, 0, 180);
    servo1.write(servoPosition);
    Serial.println(servoPosition);
    delay(20);
}
