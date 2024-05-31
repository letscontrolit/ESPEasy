#include <Servo.h>

/*
 * Description:
 * Example for setting the minimal and maximal angle.
 */ 

static const int servoPin = 4;

Servo servo1;

void setup() {
    Serial.begin(115200);
    servo1.attach(
        servoPin, 
        Servo::CHANNEL_NOT_ATTACHED, 
        45,
        120
    );
}

void loop() {
    for(int posDegrees = 0; posDegrees <= 180; posDegrees++) {
        servo1.write(posDegrees);
        Serial.println(posDegrees);
        delay(20);
    }

    for(int posDegrees = 180; posDegrees >= 0; posDegrees--) {
        servo1.write(posDegrees);
        Serial.println(posDegrees);
        delay(20);
    }
}