#include "SoftwareSerial.h"

#ifndef D5
#if defined(ESP8266)
#define D8 (15)
#define D5 (14)
#define D7 (13)
#define D6 (12)
#define RX (3)
#define TX (1)
#elif defined(ESP32)
#define D8 (5)
#define D5 (18)
#define D7 (23)
#define D6 (19)
#define RX (3)
#define TX (1)
#endif
#endif

EspSoftwareSerial::UART swSer;
#ifdef ESP8266
auto logSer = EspSoftwareSerial::UART(-1, TX);
auto hwSer = Serial;
#else
auto logSer = Serial;
auto hwSer = Serial1;
#endif

constexpr uint32_t TESTBPS = 115200;

void setup() {
	delay(2000);
#ifdef ESP8266
	hwSer.begin(TESTBPS, ::SERIAL_8N1);
	hwSer.swap();
#else
	hwSer.begin(TESTBPS, ::SERIAL_8N1, D6, D5);
#endif
	logSer.begin(115200);
	logSer.println(PSTR("\nOne Wire Half Duplex Bitpattern and Datarate Test"));
	swSer.begin(TESTBPS, EspSoftwareSerial::SWSERIAL_8N1, D6, D5);
	swSer.enableIntTx(true);
	logSer.println(PSTR("Tx on swSer"));
}

uint8_t val = 0xff;

void loop() {
	swSer.write((uint8_t)0x00);
	swSer.write(val);
	swSer.write(val);
	auto start = ESP.getCycleCount();
	int rxCnt = 0;
	while (ESP.getCycleCount() - start < ESP.getCpuFreqMHz() * 1000000 / 10) {
		if (hwSer.available()) {
			auto rxVal = hwSer.read();
			if ((!rxCnt && rxVal) || (rxCnt && rxVal != val)) {
				logSer.printf(PSTR("Rx bit error: tx = 0x%02x, rx = 0x%02x\n"), val, rxVal);
			}
			++rxCnt;
		}
	}
	if (rxCnt != 3) {
		logSer.printf(PSTR("Rx cnt error, tx = 0x%02x\n"), val);
	}
	++val;
	if (!val) {
		logSer.println("Starting over");
	}
}
