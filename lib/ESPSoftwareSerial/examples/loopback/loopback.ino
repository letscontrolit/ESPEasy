#include <SoftwareSerial.h>

// On ESP8266:
// Local EspSoftwareSerial loopback, connect D5 (rx) and D6 (tx).
// For local hardware loopback, connect D5 to D8 (tx), D6 to D7 (rx).
// For hardware send/sink, connect D7 (rx) and D8 (tx).
// Hint: The logger is run at 9600bps such that enableIntTx(true) can remain unchanged. Blocking
// interrupts severely impacts the ability of the EspSoftwareSerial devices to operate concurrently
// and/or in duplex mode.
// Operating in software serial full duplex mode, runs at 19200bps and few errors (~2.5%).
// Operating in software serial half duplex mode (both loopback and repeater),
// runs at 57600bps with nearly no errors.
// Operating loopback in full duplex, and repeater in half duplex, runs at 38400bps with nearly no errors.
// On ESP32:
// For EspSoftwareSerial or hardware send/sink, connect D5 (rx) and D6 (tx).
// Hardware Serial2 defaults to D4 (rx), D3 (tx).
// For local hardware loopback, connect D5 (rx) to D3 (tx), D6 (tx) to D4 (rx).

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

// Pick only one of HWLOOPBACK, HWSOURCESWSINK, or HWSOURCESINK
//#define HWLOOPBACK 1
//#define HWSOURCESWSINK 1
//#define HWSOURCESINK 1
#define HALFDUPLEX 1

#ifdef ESP32
constexpr int IUTBITRATE = 19200;
#else
constexpr int IUTBITRATE = 19200;
#endif

#if defined(ESP8266)
constexpr EspSoftwareSerial::Config swSerialConfig = EspSoftwareSerial::SWSERIAL_8E1;
constexpr SerialConfig hwSerialConfig = ::SERIAL_8E1;
#elif defined(ESP32)
constexpr EspSoftwareSerial::Config swSerialConfig = EspSoftwareSerial::SWSERIAL_8E1;
constexpr uint32_t hwSerialConfig = ::SERIAL_8E1;
#else
constexpr unsigned swSerialConfig = 3;
#endif
constexpr bool invert = false;

constexpr int BLOCKSIZE = 16; // use fractions of 256

unsigned long start;
const char effTxTxt[] PROGMEM = "eff. tx: ";
const char effRxTxt[] PROGMEM = "eff. rx: ";
int txCount;
int rxCount;
int expected;
int rxErrors;
int rxParityErrors;
constexpr int ReportInterval = IUTBITRATE / 8;

#if defined(ESP8266)
#if defined(HWLOOPBACK) || defined(HWSOURCESWSINK)
HardwareSerial& hwSerial(Serial);
EspSoftwareSerial::UART serialIUT;
EspSoftwareSerial::UART logger;
#elif defined(HWSOURCESINK)
HardwareSerial& serialIUT(Serial);
EspSoftwareSerial::UART logger;
#else
EspSoftwareSerial::UART serialIUT;
HardwareSerial& logger(Serial);
#endif
#elif defined(ESP32)
#if defined(HWLOOPBACK) || defined (HWSOURCESWSINK)
HardwareSerial& hwSerial(Serial2);
EspSoftwareSerial::UART serialIUT;
#elif defined(HWSOURCESINK)
HardwareSerial& serialIUT(Serial2);
#else
EspSoftwareSerial::UART serialIUT;
#endif
HardwareSerial& logger(Serial);
#else
EspSoftwareSerial::UART serialIUT(14, 12);
HardwareSerial& logger(Serial);
#endif

void setup() {
#if defined(ESP8266)
#if defined(HWLOOPBACK) || defined(HWSOURCESINK) || defined(HWSOURCESWSINK)
    Serial.begin(IUTBITRATE, hwSerialConfig, ::SERIAL_FULL, 1, invert);
    Serial.swap();
    Serial.setRxBufferSize(2 * BLOCKSIZE);
    logger.begin(9600, EspSoftwareSerial::SWSERIAL_8N1, -1, TX);
#else
    logger.begin(9600);
#endif
#if !defined(HWSOURCESINK)
    serialIUT.begin(IUTBITRATE, swSerialConfig, D5, D6, invert, 2 * BLOCKSIZE);
#ifdef HALFDUPLEX
    serialIUT.enableIntTx(false);
#endif
#endif
#elif defined(ESP32)
#if defined(HWLOOPBACK) || defined(HWSOURCESWSINK)
    Serial2.begin(IUTBITRATE, hwSerialConfig, D4, D3, invert);
    Serial2.setRxBufferSize(2 * BLOCKSIZE);
#elif defined(HWSOURCESINK)
    serialIUT.begin(IUTBITRATE, hwSerialConfig, D5, D6, invert);
    serialIUT.setRxBufferSize(2 * BLOCKSIZE);
#endif
#if !defined(HWSOURCESINK)
    serialIUT.begin(IUTBITRATE, swSerialConfig, D5, D6, invert, 2 * BLOCKSIZE);
#ifdef HALFDUPLEX
    serialIUT.enableIntTx(false);
#endif
#endif
    logger.begin(9600);
#else
#if !defined(HWSOURCESINK)
    serialIUT.begin(IUTBITRATE);
#endif
    logger.begin(9600);
#endif

    logger.println(PSTR("Loopback example for EspEspSoftwareSerial"));

    start = micros();
    txCount = 0;
    rxCount = 0;
    rxErrors = 0;
    rxParityErrors = 0;
    expected = -1;
}

unsigned char c = 0;

void loop() {
#ifdef HALFDUPLEX
    char block[BLOCKSIZE];
#endif
    char inBuf[BLOCKSIZE];
    for (int i = 0; i < BLOCKSIZE; ++i) {
#ifndef HALFDUPLEX
#ifdef HWSOURCESWSINK
        hwSerial.write(c);
#else
        serialIUT.write(c);
#endif
#ifdef HWLOOPBACK
        int avail = hwSerial.available();
        while ((0 == (i % 8)) && avail > 0) {
            int inCnt = hwSerial.read(inBuf, min(avail, min(BLOCKSIZE, hwSerial.availableForWrite())));
            hwSerial.write(inBuf, inCnt);
            avail -= inCnt;
        }
#endif
#else
        block[i] = c;
#endif
        c = (c + 1) % 256;
        ++txCount;
    }
#ifdef HALFDUPLEX
#ifdef HWSOURCESWSINK
    hwSerial.write(block, BLOCKSIZE);
#else
    serialIUT.write(block, BLOCKSIZE);
#endif
#endif
#ifdef HWSOURCESINK
#if defined(ESP8266)
    if (serialIUT.hasOverrun()) { logger.println(PSTR("serialIUT.overrun")); }
#endif
#else
    if (serialIUT.overflow()) { logger.println(PSTR("serialIUT.overflow")); }
#endif

    int inCnt;
    uint32_t deadlineStart;

#ifdef HWLOOPBACK
    // starting deadline for the first bytes to become readable
    deadlineStart = ESP.getCycleCount();
    inCnt = 0;
    while ((ESP.getCycleCount() - deadlineStart) < (1000000UL * 12 * BLOCKSIZE) / IUTBITRATE * 24 * ESP.getCpuFreqMHz()) {
        int avail = hwSerial.available();
        inCnt += hwSerial.read(&inBuf[inCnt], min(avail, min(BLOCKSIZE - inCnt, hwSerial.availableForWrite())));
        if (inCnt >= BLOCKSIZE) { break; }
        // wait for more outstanding bytes to trickle in
        if (avail) deadlineStart = ESP.getCycleCount();
    }
    hwSerial.write(inBuf, inCnt);
#endif

    // starting deadline for the first bytes to come in
    deadlineStart = ESP.getCycleCount();
    inCnt = 0;
    while ((ESP.getCycleCount() - deadlineStart) < (1000000UL * 12 * BLOCKSIZE) / IUTBITRATE * 8 * ESP.getCpuFreqMHz()) {
        int avail;
        if (0 != (swSerialConfig & 070))
            avail = serialIUT.available();
        else
            avail = serialIUT.read(inBuf, BLOCKSIZE);
        for (int i = 0; i < avail; ++i)
        {
            unsigned char r;
            if (0 != (swSerialConfig & 070))
                r = serialIUT.read();
            else
                r = inBuf[i];
            if (expected == -1) { expected = r; }
            else {
                expected = (expected + 1) % (1UL << (5 + swSerialConfig % 4));
            }
            if (r != expected) {
                ++rxErrors;
                expected = -1;
            }
#ifndef HWSOURCESINK
            if (serialIUT.readParity() != (static_cast<bool>(swSerialConfig & 010) ? serialIUT.parityOdd(r) : serialIUT.parityEven(r)))
            {
                ++rxParityErrors;
            }
#elif defined(ESP8266)
            // current ESP8266 API does not flag parity errors separately
            if (serialIUT.hasRxError())
            {
                ++rxParityErrors;
            }
#endif
            ++rxCount;
            ++inCnt;
        }

        if (inCnt >= BLOCKSIZE) { break; }
        // wait for more outstanding bytes to trickle in
        if (avail) deadlineStart = ESP.getCycleCount();
    }

    const uint32_t interval = micros() - start;
    if (txCount >= ReportInterval && interval) {
        uint8_t wordBits = (5 + swSerialConfig % 4) + static_cast<bool>(swSerialConfig & 070) + 1 + ((swSerialConfig & 0300) ? 1 : 0);
        logger.println(String(PSTR("tx/rx: ")) + txCount + PSTR("/") + rxCount);
        const long txCps = txCount * (1000000.0 / interval);
        const long rxCps = rxCount * (1000000.0 / interval);
        logger.print(String(FPSTR(effTxTxt)) + wordBits * txCps + PSTR("bps, ")
            + effRxTxt + wordBits * rxCps + PSTR("bps, ")
            + rxErrors + PSTR(" errors (") + 100.0 * rxErrors / (!rxErrors ? 1 : rxCount) + PSTR("%)"));
        if (0 != (swSerialConfig & 070))
        {
            logger.print(PSTR(" (")); logger.print(rxParityErrors); logger.println(PSTR(" parity errors)"));
        }
        else
        {
            logger.println();
        }
        txCount = 0;
        rxCount = 0;
        rxErrors = 0;
        rxParityErrors = 0;
        expected = -1;
        // resync
        delay(1000UL * 12 * BLOCKSIZE / IUTBITRATE * 16);
        serialIUT.flush();
        start = micros();
    }
}
