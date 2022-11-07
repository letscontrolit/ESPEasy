#include "CircularBuffer.h"

// the type of the record is unsigned long: we intend to store milliseconds
// the buffer can contain up to 10 records
// the buffer will use a byte for its index to reduce memory footprint
CircularBuffer<unsigned long, 10> buffer;

#define BUTTON_PIN A0
#define INTERVAL 60000

void setup() {
    Serial.begin(9600);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}
unsigned long last = 0;

void loop() {
    if (digitalRead(BUTTON_PIN) == LOW) {
        // stores button click time in the buffer, appending after the last element
        buffer.push(millis());
    }
    // checks if the buffer has reached maximum capacity to avoid overwrites
    if (buffer.isFull()) {
        Serial.println("Printing button clicks");
        // repeats until the buffer is empty
        while (!buffer.isEmpty()) {
            // prints current buffer size
            Serial.print(buffer.size());
            Serial.print(": ");
            // retrieves the last added button click and removes the event
            // this outputs the events starting from the most recent
            // switching from pop() to shift() the events would be printed
            // in chronological order, starting from the least recent
            Serial.println(buffer.pop());
        }
    }
    // prints the buffer contents every minute 
    if (millis() - last > INTERVAL) {
        last = millis();
        Serial.print("Buffer contains ");
        Serial.print(buffer.size());
        Serial.print(" elements: ");
        // iterates over the events in chronological order
        // you can replace byte with decltype(buffer)::index_t
        for (byte i = 0; i < buffer.size(); i++) {
            // retrieves the i-th element from the buffer without removing it
            Serial.print(buffer[i]);
            Serial.print(" ");
        }
        Serial.println();
    }
}