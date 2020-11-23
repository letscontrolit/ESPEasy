#include <Servo.h>

static const int servosPins[5] = {4, 16, 18, 19, 21};

Servo servos[5];

void setServos(int degrees) {
    for(int i = 0; i < 5; ++i) {
        servos[i].write((degrees + (35 * i)) % 180);
    }
}

void setup() {
    Serial.begin(115200);

    for(int i = 0; i < 5; ++i) {
        if(!servos[i].attach(servosPins[i])) {
            Serial.print("Servo ");
            Serial.print(i);
            Serial.println("attach error");
        }
    }
}

void loop() {
    for(int posDegrees = 0; posDegrees <= 180; posDegrees++) {
        setServos(posDegrees);
        Serial.println(posDegrees);
        delay(20);
    }

    for(int posDegrees = 180; posDegrees >= 0; posDegrees--) {
        setServos(posDegrees);
        Serial.println(posDegrees);
        delay(20);
    }
}
