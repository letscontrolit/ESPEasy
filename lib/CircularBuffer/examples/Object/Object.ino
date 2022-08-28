#include <CircularBuffer.h>
#include "Record.h"

CircularBuffer<Record*, 10> buffer;

#define SAMPLE_PIN A0

void setup() {
	Serial.begin(9600);
	pinMode(SAMPLE_PIN, INPUT);
	Serial.println("STARTING UP");
}

void loop() {
	unsigned int sample = analogRead(SAMPLE_PIN);
	if (sample != buffer.first()->value()) {
		Record* record = new Record(millis(), sample);
		buffer.unshift(record);
		Serial.println("---");
		delay(50);
	}
	if (buffer.isFull()) {
		Serial.println("Queue is full:");
		while (!buffer.isEmpty()) {
			Record* record = buffer.shift();
			record->print(&Serial);
			delete record;
			Serial.println();
		}
		Serial.println("START AGAIN");
	}
}
