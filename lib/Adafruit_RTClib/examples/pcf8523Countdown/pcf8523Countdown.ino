/**************************************************************************/
/*
  Countdown Timer using a PCF8523 RTC connected via I2C and Wire lib
  with the INT/SQW pin wired to an interrupt-capable input.

  According to the data sheet, the PCF8523 can run countdown timers
  from 244 microseconds to 10.625 days:
  https://www.nxp.com/docs/en/data-sheet/PCF8523.pdf#page=34

  This sketch sets a countdown timer, and executes code when it reaches 0,
  then blinks the built-in LED like BlinkWithoutDelay, but without millis()!

  NOTE:
  You must connect the PCF8523's interrupt pin to your Arduino or other
  microcontroller on an input pin that can handle interrupts, and that has a
  pullup resistor. The pin will be briefly pulled low each time the countdown
  reaches 0. This example will not work without the interrupt pin connected!

  On Adafruit breakout boards, the interrupt pin is labeled 'INT' or 'SQW'.
*/
/**************************************************************************/

#include "RTClib.h"

RTC_PCF8523 rtc;

// Input pin with interrupt capability
// const int timerInterruptPin = 2;  // Most Arduinos
const int timerInterruptPin = 5;  // Adafruit Feather M0/M4/nRF52840

// Variables modified during an interrupt must be declared volatile
volatile bool countdownInterruptTriggered = false;
volatile int numCountdownInterrupts = 0;

void setup () {
  Serial.begin(57600);

#ifndef ESP8266
  while (!Serial); // wait for serial port to connect. Needed for native USB
#endif

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

  pinMode(LED_BUILTIN, OUTPUT);

  // Set the pin attached to PCF8523 INT to be an input with pullup to HIGH.
  // The PCF8523 interrupt pin will briefly pull it LOW at the end of a given
  // countdown period, then it will be released to be pulled HIGH again.
  pinMode(timerInterruptPin, INPUT_PULLUP);

  Serial.println(F("\nStarting PCF8523 Countdown Timer example."));
  Serial.print(F("Configured to expect PCF8523 INT/SQW pin connected to input pin: "));
  Serial.println(timerInterruptPin);
  Serial.println(F("This example will not work without the interrupt pin connected!\n\n"));

  // Timer configuration is not cleared on an RTC reset due to battery backup!
  rtc.deconfigureAllTimers();

  Serial.println(F("First, use the PCF8523's 'Countdown Timer' with an interrupt."));
  Serial.println(F("Set the countdown for 10 seconds and we'll let it run for 2 rounds."));
  Serial.println(F("Starting Countdown Timer now..."));

  // These are the PCF8523's built-in "Timer Source Clock Frequencies".
  // They are predefined time periods you choose as your base unit of time,
  // depending on the length of countdown timer you need.
  // The minimum length of your countdown is 1 time period.
  // The maximum length of your countdown is 255 time periods.
  //
  // PCF8523_FrequencyHour   = 1 hour, max 10.625 days (255 hours)
  // PCF8523_FrequencyMinute = 1 minute, max 4.25 hours
  // PCF8523_FrequencySecond = 1 second, max 4.25 minutes
  // PCF8523_Frequency64Hz   = 1/64 of a second (15.625 milliseconds), max 3.984 seconds
  // PCF8523_Frequency4kHz   = 1/4096 of a second (244 microseconds), max 62.256 milliseconds
  //
  //
  // These are the PCF8523's optional 'Low Pulse Widths' of time the interrupt
  // pin is held LOW at the end of every countdown (frequency 64Hz or longer).
  //
  // PCF8523_LowPulse3x64Hz  =  46.875 ms   3/64ths second (default)
  // PCF8523_LowPulse4x64Hz  =  62.500 ms   4/64ths second
  // PCF8523_LowPulse5x64Hz  =  78.125 ms   5/64ths second
  // PCF8523_LowPulse6x64Hz  =  93.750 ms   6/64ths second
  // PCF8523_LowPulse8x64Hz  = 125.000 ms   8/64ths second
  // PCF8523_LowPulse10x64Hz = 156.250 ms  10/64ths second
  // PCF8523_LowPulse12x64Hz = 187.500 ms  12/64ths second
  // PCF8523_LowPulse14x64Hz = 218.750 ms  14/64ths second
  //
  //
  // Uncomment an example below:

  // rtc.enableCountdownTimer(PCF8523_FrequencyHour, 24);    // 1 day
  // rtc.enableCountdownTimer(PCF8523_FrequencyMinute, 150); // 2.5 hours
  rtc.enableCountdownTimer(PCF8523_FrequencySecond, 10);  // 10 seconds
  // rtc.enableCountdownTimer(PCF8523_Frequency64Hz, 32);    // 1/2 second
  // rtc.enableCountdownTimer(PCF8523_Frequency64Hz, 16);    // 1/4 second
  // rtc.enableCountdownTimer(PCF8523_Frequency4kHz, 205);   // 50 milliseconds

  attachInterrupt(digitalPinToInterrupt(timerInterruptPin), countdownOver, FALLING);

  // This message proves we're not blocked while counting down!
  Serial.println(F("  While we're waiting, a word of caution:"));
  Serial.println(F("  When starting a new countdown timer, the first time period is not of fixed"));
  Serial.println(F("  duration. The amount of inaccuracy for the first time period is up to one full"));
  Serial.println(F("  clock frequency. Example: just the first second of the first round of a new"));
  Serial.println(F("  countdown based on PCF8523_FrequencySecond may be off by as much as 1 second!"));
  Serial.println(F("  For critical timing, consider starting actions on the first interrupt."));
}

// Triggered by the PCF8523 Countdown Timer interrupt at the end of a countdown
// period. Meanwhile, the PCF8523 immediately starts the countdown again.
void countdownOver () {
  // Set a flag to run code in the loop():
  countdownInterruptTriggered = true;
  numCountdownInterrupts++;
}

// Triggered by the PCF8523 Second Timer every second.
void toggleLed () {
  // Run certain types of fast executing code here:
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void loop () {
  if (countdownInterruptTriggered && numCountdownInterrupts == 1) {
    Serial.println(F("1st countdown interrupt triggered. Accurate timekeeping starts now."));
    countdownInterruptTriggered = false; // don't come in here again
  } else if (countdownInterruptTriggered && numCountdownInterrupts == 2) {
    Serial.println(F("2nd countdown interrupt triggered. Disabling countdown and detaching interrupt.\n\n"));
    rtc.disableCountdownTimer();
    detachInterrupt(digitalPinToInterrupt(timerInterruptPin));
    delay(2000);


    Serial.println(F("Now, set up the PCF8523's 'Second Timer' to toggle the built-in LED at 1Hz..."));
    attachInterrupt(digitalPinToInterrupt(timerInterruptPin), toggleLed, FALLING);
    rtc.enableSecondTimer();
    Serial.println(F("Look for the built-in LED to flash 1 second ON, 1 second OFF, repeat. "));
    Serial.println(F("Meanwhile this program will use delay() to block code execution briefly"));
    Serial.println(F("before moving on to the last example. Notice the LED keeps blinking!\n\n"));
    delay(20000); // less accurate, blocks execution here. Meanwhile Second Timer keeps running.
    rtc.disableSecondTimer();
    detachInterrupt(digitalPinToInterrupt(timerInterruptPin));


    Serial.println(F("Lastly, set up a Countdown Timer that works without attaching an interrupt..."));
    rtc.enableCountdownTimer(PCF8523_Frequency64Hz, 32, PCF8523_LowPulse8x64Hz);
    Serial.println(F("Look for the LED to turn on every 1/2 second and stay lit for 1/8th of a second."));
    Serial.println(F("The countdown was set to a source clock frequency of 64 Hz (1/64th of a second)"));
    Serial.println(F("for a length of 32 time periods. 32 * 1/64th of a second is 1/2 of a second."));
    Serial.println(F("The low pulse duration was set to 125 ms, or 1/8th of a second."));
    Serial.println(F("The loop() keeps the built-in LED set to the opposite state of the INT/SQW pin."));


    countdownInterruptTriggered = false; // don't come in here again
  }

  // While countdown running, INT/SQW pullup to HIGH, set LED to LOW (off)
  // When countdown is over, INT/SQW pulled down LOW, set LED to HIGH (on)
  digitalWrite(LED_BUILTIN, !digitalRead(timerInterruptPin));
}
