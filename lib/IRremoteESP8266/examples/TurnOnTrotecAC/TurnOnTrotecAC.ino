/* Copyright 2017 stufisher
* An IR LED circuit *MUST* be connected to the ESP8266 on a pin
* as specified by kIrLed below.
*
* TL;DR: The IR LED needs to be driven by a transistor for a good result.
*
* Suggested circuit:
*     https://github.com/crankyoldgit/IRremoteESP8266/wiki#ir-sending
*
* Common mistakes & tips:
*   * Don't just connect the IR LED directly to the pin, it won't
*     have enough current to drive the IR LED effectively.
*   * Make sure you have the IR LED polarity correct.
*     See: https://learn.sparkfun.com/tutorials/polarity/diode-and-led-polarity
*   * Typical digital camera/phones can be used to see if the IR LED is flashed.
*     Replace the IR LED with a normal LED if you don't have a digital camera
*     when debugging.
*   * Avoid using the following pins unless you really know what you are doing:
*     * Pin 0/D3: Can interfere with the boot/program mode & support circuits.
*     * Pin 1/TX/TXD0: Any serial transmissions from the ESP8266 will interfere.
*     * Pin 3/RX/RXD0: Any serial transmissions to the ESP8266 will interfere.
*   * ESP-01 modules are tricky. We suggest you use a module with more GPIOs
*     for your first time. e.g. ESP-12 etc.
*/

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Trotec.h>

const uint16_t kIrLed = 4;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).
IRTrotecESP ac(kIrLed);  // Set the GPIO to be used for sending messages.

void setup() {
  ac.begin();
  Serial.begin(115200);
}

void loop() {
  Serial.println("Sending...");

  // Set up what we want to send. See ir_Trotec.cpp for all the options.
  ac.setPower(true);
  ac.setSpeed(kTrotecFanLow);
  ac.setMode(kTrotecCool);
  ac.setTemp(25);

  // Now send the IR signal.
#if SEND_TROTEC
  ac.send();
#else  // SEND_TROTEC
  Serial.println("Can't send because SEND_TROTEC has been disabled.");
#endif  // SEND_TROTEC

  delay(5000);
}
