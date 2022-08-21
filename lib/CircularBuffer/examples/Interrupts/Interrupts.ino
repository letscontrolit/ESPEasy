#define CIRCULAR_BUFFER_INT_SAFE
#include "CircularBuffer.h"
CircularBuffer<unsigned long, 10> timings;


void count() {
  timings.push(millis());
}

void setup() {
    Serial.begin(9600);
    attachInterrupt(digitalPinToInterrupt(2), count, RISING);
}

unsigned long time = 0;

void loop() {
    Serial.print("buffer size is "); Serial.println(timings.size());
    delay(250);
    if (millis() - time >= 10000 && !timings.isEmpty()) {
        Serial.print("popping "); Serial.println(timings.pop());
        time = millis();
    }
}