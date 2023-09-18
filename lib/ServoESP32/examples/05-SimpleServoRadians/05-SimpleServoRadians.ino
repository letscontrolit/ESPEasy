#include <Servo.h>

/*
 * Description:
 * Example for using floating point angle and radians.
 */

static const int servoPin = 4;

ServoFloat servo1;

float deg2rad(float in) {
    return in * M_PI / 180.0;
}

const float minAngle = deg2rad(45.0);
const float maxAngle = deg2rad(120.0);
const float stepAngle = deg2rad(1.0);

void setup() {
    Serial.begin(115200);
    servo1.attach(servoPin, Servo::CHANNEL_NOT_ATTACHED, minAngle, maxAngle);
}

void loop() {
    for (float angleRadians = minAngle; angleRadians <= maxAngle; angleRadians += stepAngle) {
        servo1.write(angleRadians);
        Serial.println(angleRadians);
        delay(20);
    }

    for (float angleRadians = maxAngle; angleRadians >= minAngle; angleRadians -= stepAngle) {
        servo1.write(angleRadians);
        Serial.println(angleRadians);
        delay(20);
    }
}