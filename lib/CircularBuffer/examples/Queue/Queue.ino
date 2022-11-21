#include <CircularBuffer.h>

CircularBuffer<int, 100> queue;

unsigned long time = 0;

#define BUTTON_PIN 3

void setup() {
	Serial.begin(9600);
	pinMode(BUTTON_PIN, INPUT);
	time = millis();
}

// enqueue button press dequeueing every 2 seconds
void loop() {
	if (digitalRead(BUTTON_PIN) == HIGH) {
		queue.unshift(millis());
	}
	if (millis() - time >= 2000) {
		time = millis();
		Serial.print("Button pressed ");
		Serial.print(queue.size());
		Serial.println("times (or more)");
		while (!queue.isEmpty()) {
			Serial.print("  @");
			Serial.println(queue.pop());
		}
	}
}
