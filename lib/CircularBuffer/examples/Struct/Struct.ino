#include <CircularBuffer.h>

namespace data {
	typedef struct {
		unsigned long time;
		unsigned int value;
		bool flag;
	} record;

	void print(record r) {
		Serial.print(r.time);
		Serial.print("  ");
		Serial.print(r.value);
		Serial.print("  ");
		Serial.print(r.flag);
	}
}

CircularBuffer<data::record, 10> structs;

#define SAMPLE_PIN A0

void setup() {
	Serial.begin(9600);
	pinMode(SAMPLE_PIN, INPUT);
	Serial.println("STARTING UP");
}

void loop() {
	unsigned int sample = analogRead(SAMPLE_PIN);
	if (sample != structs.last().value) {
		structs.push(data::record{millis(), sample, sample > 512});
		Serial.println("---");
		delay(50);
	}
	if (structs.isFull()) {
		Serial.println("Stack is full:");
		while (!structs.isEmpty()) {
			data::print(structs.shift());
			Serial.println();
		}
		Serial.println("START AGAIN");
	}
}
