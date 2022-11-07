#include <Arduino.h>
#include <LowPower.h>
#include "HLW8012.h"

#define SERIAL_BAUDRATE                 115200

// GPIOs
#define SEL_PIN                         5
#define CF1_PIN                         13
#define CF_PIN                          14

// Set SEL_PIN to HIGH to sample current
// This is the case for Itead's Sonoff POW, where a
// the SEL_PIN drives a transistor that pulls down
// the SEL pin in the HLW8012 when closed
#define CURRENT_MODE                    HIGH

// Maximum pulse width, a 2Hz pulse means a precission of ~24W
#define MAX_PULSE_WIDTH                 500000

HLW8012 hlw8012;

void setup() {

    // Init serial port and clean garbage
    Serial.begin(SERIAL_BAUDRATE);
    Serial.println();
    Serial.println();

    // Initialize HLW8012
    hlw8012.begin(CF_PIN, CF1_PIN, SEL_PIN, CURRENT_MODE, false, MAX_PULSE_WIDTH);

}

void loop() {

    // Sleep for 20 seconds
    for (unsigned short i = 0; i < 5; i++ ) {
        LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);
    }

    unsigned long start = millis();
    Serial.print("[HLW] Active Power (W) : "); Serial.println(hlw8012.getActivePower());
    Serial.print("[HLW] Awake for (ms)   : "); Serial.println(millis() - start);

}
