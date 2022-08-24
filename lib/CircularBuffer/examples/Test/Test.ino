#define CIRCULAR_BUFFER_DEBUG
#include <CircularBuffer.h>

CircularBuffer<char, 10> buffer;

void printBuffer() {
	if (buffer.isEmpty()) {
		Serial.println("empty");
	} else {
		Serial.print("[");
		for (decltype(buffer)::index_t i = 0; i < buffer.size() - 1; i++) {
			Serial.print(buffer[i]);
			Serial.print(",");
		}
		Serial.print(buffer[buffer.size() - 1]);
		Serial.print("] (");

		Serial.print(buffer.size());
		Serial.print("/");
		Serial.print(buffer.size() + buffer.available());
		if (buffer.isFull()) {
			Serial.print(" full");
		}

		Serial.println(")");
	}
}

void setup() {
	Serial.begin(9600);
	Serial.println(" ### RESET ### ");

	buffer.push('A');
	buffer.push('B');
	buffer.push('C');
	buffer.push('D');
	buffer.push('E');
	buffer.push('F');
	buffer.push('G');
	buffer.push('H');
	buffer.push('I');
	buffer.push('J');
	printBuffer();
	Serial.println(); delay(250);

	Serial.println("first()");
	Serial.print(buffer.first());
	Serial.print(" buffer is ");
	printBuffer();
	Serial.println(); delay(250);

	Serial.println("last()");
	Serial.print(buffer.last());
	Serial.print(" buffer is ");
	printBuffer();
	Serial.println(); delay(250);

	Serial.println("unshift('X')");
	Serial.print(buffer.unshift('X') ? "true" : "false");
	Serial.print(" buffer is ");
	printBuffer();
	Serial.println(); delay(250);

	Serial.println("push('Z')");
	Serial.print(buffer.push('Z') ? "true" : "false");
	Serial.print(" buffer is ");
	printBuffer();
	Serial.println(); delay(250);

	Serial.println("pop()");
	Serial.print(buffer.pop());
	Serial.print(" buffer is ");
	printBuffer();
	Serial.println(); delay(250);

	Serial.println("pop()");
	Serial.print(buffer.pop());
	Serial.print(" buffer is ");
	printBuffer();
	Serial.println(); delay(250);

	Serial.println("pop()");
	Serial.print(buffer.pop());
	Serial.print(" buffer is ");
	printBuffer();
	Serial.println(); delay(250);

	Serial.println("pop()");
	Serial.print(buffer.pop());
	Serial.print(" buffer is ");
	printBuffer();
	Serial.println(); delay(250);

	Serial.println("pop()");
	Serial.print(buffer.pop());
	Serial.print(" buffer is ");
	printBuffer();
	Serial.println(); delay(250);

	Serial.println("shift()");
	Serial.print(buffer.shift());
	Serial.print(" buffer is ");
	printBuffer();
	Serial.println(); delay(250);

	Serial.println("shift()");
	Serial.print(buffer.shift());
	Serial.print(" buffer is ");
	printBuffer();
	Serial.println(); delay(250);

	Serial.println("shift()");
	Serial.print(buffer.shift());
	Serial.print(" buffer is ");
	printBuffer();
	Serial.println(); delay(250);

	Serial.println("shift()");
	Serial.print(buffer.shift());
	Serial.print(" buffer is ");
	printBuffer();
	Serial.println(); delay(250);

	Serial.println("shift()");
	Serial.print(buffer.shift());
	Serial.print(" buffer is ");
	printBuffer();
	Serial.println(); delay(250);

	Serial.println("push('Z')");
	Serial.print(buffer.push('Z') ? "true" : "false");
	Serial.print(" buffer is ");
	printBuffer();
	Serial.println(); delay(250);

	Serial.println("push('X')");
	Serial.print(buffer.push('X') ? "true" : "false");
	Serial.print(" buffer is ");
	printBuffer();
	Serial.println(); delay(250);

	Serial.println("clear()");
	buffer.clear();
	Serial.print("buffer is ");
	printBuffer();
	Serial.println(); delay(250);

	Serial.println("unshift('A')");
	Serial.print(buffer.unshift('A') ? "true" : "false");
	Serial.print(" buffer is ");
	printBuffer();
	Serial.println(); delay(250);

	Serial.println("push('Z')");
	Serial.print(buffer.push('Z') ? "true" : "false");
	Serial.print(" buffer is ");
	printBuffer();
	Serial.println(); delay(250);

	Serial.println("buffer[5]");
	Serial.print(buffer[5]);
	Serial.print(" buffer is ");
	printBuffer();
	Serial.println(); delay(250);

	Serial.println("buffer[15]");
	Serial.print(buffer[15]);
	Serial.print(" buffer is ");
	printBuffer();
	Serial.println(); delay(250);

	Serial.println("clear()");
	buffer.clear();
	Serial.print("buffer is ");
	printBuffer();
	Serial.println(); delay(250);

	// The following operations will crash the firmware and cause a reset

	Serial.println("CRASH TEST");

	Serial.print(buffer.pop());
	Serial.print(" buffer is ");
	printBuffer();
	Serial.println(); delay(250);

	Serial.print(buffer.shift());
	Serial.print(" buffer is ");
	printBuffer();
	Serial.println(); delay(250);
}

void loop() {

}
