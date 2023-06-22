// On ESP8266:
// At 80MHz runs up 57600bps, and at 160MHz CPU frequency up to 115200bps with only negligible errors.
// Connect pin 13 to 15.
// For verification and as a example for how to use SW serial on the USB to PC connection,
// which allows the use of HW Serial on GPIO13 and GPIO15 instead, #define SWAPSERIAL below.
// Notice how the bitrates are also swapped then between RX/TX and GPIO13/GPIO15.
// Builtin debug output etc. must be stopped on HW Serial in this case, as it would interfere with the
// external communication on GPIO13/GPIO15.

#include <SoftwareSerial.h>

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

#ifdef ESP32
#define BAUD_RATE 57600
#else
#define BAUD_RATE 57600
#endif

#undef SWAPSERIAL

#ifndef SWAPSERIAL
auto& usbSerial = Serial;
EspSoftwareSerial::UART testSerial;
#else
EspSoftwareSerial::UART usbSerial;
auto& testSerial = Serial;
#endif

void setup() {
#ifndef SWAPSERIAL
    usbSerial.begin(115200);
    // Important: the buffer size optimizations here, in particular the isrBufSize (11) that is only sufficiently
    // large to hold a single word (up to start - 8 data - parity - stop), are on the basis that any char written
    // to the loopback EspSoftwareSerial adapter gets read before another write is performed.
    // Block writes with a size greater than 1 would usually fail. Do not copy this into your own project without
    // reading the documentation.
    testSerial.begin(BAUD_RATE, EspSoftwareSerial::SWSERIAL_8N1, D7, D8, false, 95, 11);
#else
    testSerial.begin(115200);
    testSerial.setDebugOutput(false);
    testSerial.swap();
    usbSerial.begin(BAUD_RATE, EspSoftwareSerial::SWSERIAL_8N1, RX, TX, false, 95);
#endif

    usbSerial.println(PSTR("\nSoftware serial test started"));

    for (char ch = ' '; ch <= 'z'; ch++) {
        testSerial.write(ch);
    }
    testSerial.println();
}

void loop() {
    while (testSerial.available() > 0) {
        usbSerial.write(testSerial.read());
        yield();
    }
    while (usbSerial.available() > 0) {
        testSerial.write(usbSerial.read());
        yield();
    }
}
