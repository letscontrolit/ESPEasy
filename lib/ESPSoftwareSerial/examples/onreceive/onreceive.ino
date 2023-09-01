// On ESP8266:
// Runs up to 115200bps at 80MHz, 250000bps at 160MHz, with nearly zero errors.
// This example is currently not ported to ESP32, which is based on FreeRTOS.

#include <SoftwareSerial.h>

#ifndef D5
#define D8 (15)
#define D5 (14)
#define D7 (13)
#define D6 (12)
#define RX (3)
#define TX (1)
#endif

#define BAUD_RATE 115200
#define MAX_FRAMEBITS (1 + 8 + 1 + 2)

EspSoftwareSerial::UART testSerial;

// Becomes set from ISR / IRQ callback function.
std::atomic<bool> rxPending(false);

void IRAM_ATTR receiveHandler() {
	rxPending.store(true);
	esp_schedule();
}

void setup() {
	Serial.begin(115200);
	Serial.setDebugOutput(false);
	Serial.swap();
	testSerial.begin(BAUD_RATE, EspSoftwareSerial::SWSERIAL_8N1, RX, TX);
	// Only half duplex this way, but reliable TX timings for high bps
	testSerial.enableIntTx(false);
	testSerial.onReceive(receiveHandler);

	testSerial.println(PSTR("\nSoftware serial onReceive() event test started"));

	for (char ch = ' '; ch <= 'z'; ch++) {
		testSerial.write(ch);
	}
	testSerial.println();
}

void loop() {
#ifdef ESP8266
	bool isRxPending = rxPending.load();
	if (isRxPending) {
		rxPending.store(false);
	}
#else
	bool isRxPending = m_isrOverflow.exchange(false);
#endif
	auto avail = testSerial.available();
	if (isRxPending && !avail) {
		// event fired on start bit, wait until first stop bit of longest frame
		delayMicroseconds(1 + MAX_FRAMEBITS * 1000000 / BAUD_RATE);
		avail = testSerial.available();
	}
	if (!avail) {
		// On development board, idle power draw at USB:
		// with yield() 77mA, 385mW (160MHz: 82mA, 410mW)
		// with esp_suspend() 20mA, 100mW (at 160MHz, too)
		//yield();
		esp_suspend();
		return;
	}
	// try to force to half-duplex
	decltype(avail) prev_avail;
	do {
		delayMicroseconds(1 + MAX_FRAMEBITS * 1000000 / BAUD_RATE);
		prev_avail = avail;
	} while (prev_avail != (avail = testSerial.available()));
	while (avail > 0) {
		testSerial.write(testSerial.read());
		avail = testSerial.available();
	}
	testSerial.println();
}
