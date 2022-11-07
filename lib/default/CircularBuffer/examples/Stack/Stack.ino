#include <CircularBuffer.h>

CircularBuffer<unsigned int, 100> stack;

#define SAMPLE_PIN A0
void setup() {
	Serial.begin(9600);
	pinMode(SAMPLE_PIN, INPUT);
}

// stacks up pin analog readings then printing in reverse order when maxed
void loop() {
	unsigned int sample = analogRead(SAMPLE_PIN);
	if (sample != stack.last()) {
		stack.push(sample);
	}
	if (stack.isFull()) {
		Serial.println("Stack is full:");
		while (!stack.isEmpty()) {
			Serial.print("  # ");
			Serial.println(stack.pop());
		}
		// the following is unnecessary, still here to demonstrate its use
		stack.clear();
	}
	delay(50);
}
