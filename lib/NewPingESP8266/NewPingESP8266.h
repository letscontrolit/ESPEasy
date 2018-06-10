// ---------------------------------------------------------------------------
// NewPingESP8266 Library - v1.8 - 07/30/2016
//
// AUTHOR/LICENSE:
// Created by Tim Eckel - teckel@leethost.com
// Copyright 2016 License: GNU GPL v3 http://www.gnu.org/licenses/gpl.html
//
// LINKS:
// Project home: https://bitbucket.org/teckel12/arduino-new-ping/wiki/Home
// Blog: http://arduino.cc/forum/index.php/topic,106043.0.html
//
// DISCLAIMER:
// This software is furnished "as is", without technical support, and with no
// warranty, express or implied, as to its usefulness for any purpose.
//
// BACKGROUND:
// When I first received an ultrasonic sensor I was not happy with how poorly
// it worked. Quickly I realized the problem wasn't the sensor, it was the
// available ping and ultrasonic libraries causing the problem. The NewPingESP8266
// library totally fixes these problems, adds many new features, and breaths
// new life into these very affordable distance sensors.
//
// FEATURES:
// * Works with many different ultrasonic sensors: SR04, SRF05, SRF06, DYP-ME007, URM37 & Parallax PING))).
// * Compatible with the entire Arduino line-up (and clones), Teensy family (including $19 96Mhz 32 bit Teensy 3.2) and non-AVR microcontrollers.
// * Interface with all but the SRF06 sensor using only one Arduino pin.
// * Doesn't lag for a full second if no ping/echo is received.
// * Ping sensors consistently and reliably at up to 30 times per second.
// * Timer interrupt method for event-driven sketches.
// * Built-in digital filter method ping_median() for easy error correction.
// * Uses port registers for a faster pin interface and smaller code size.
// * Allows you to set a maximum distance where pings beyond that distance are read as no ping "clear".
// * Ease of using multiple sensors (example sketch with 15 sensors).
// * More accurate distance calculation (cm, inches & uS).
// * Doesn't use pulseIn, which is slow and gives incorrect results with some ultrasonic sensor models.
// * Actively developed with features being added and bugs/issues addressed.
//
// CONSTRUCTOR:
//   NewPingESP8266 sonar(trigger_pin, echo_pin [, max_cm_distance])
//     trigger_pin & echo_pin - Arduino pins connected to sensor trigger and echo.
//       NOTE: To use the same Arduino pin for trigger and echo, specify the same pin for both values.
//     max_cm_distance - [Optional] Maximum distance you wish to sense. Default=500cm.
//
// METHODS:
//   sonar.ping([max_cm_distance]) - Send a ping and get the echo time (in microseconds) as a result. [max_cm_distance] allows you to optionally set a new max distance.
//   sonar.ping_in([max_cm_distance]) - Send a ping and get the distance in whole inches. [max_cm_distance] allows you to optionally set a new max distance.
//   sonar.ping_cm([max_cm_distance]) - Send a ping and get the distance in whole centimeters. [max_cm_distance] allows you to optionally set a new max distance.
//   sonar.ping_median(iterations [, max_cm_distance]) - Do multiple pings (default=5), discard out of range pings and return median in microseconds. [max_cm_distance] allows you to optionally set a new max distance.
//   NewPingESP8266::convert_in(echoTime) - Convert echoTime from microseconds to inches (rounds to nearest inch).
//   NewPingESP8266::convert_cm(echoTime) - Convert echoTime from microseconds to centimeters (rounds to nearest cm).
//   sonar.ping_timer(function [, max_cm_distance]) - Send a ping and call function to test if ping is complete. [max_cm_distance] allows you to optionally set a new max distance.
//   sonar.check_timer() - Check if ping has returned within the set distance limit.
//   NewPingESP8266::timer_us(frequency, function) - Call function every frequency microseconds.
//   NewPingESP8266::timer_ms(frequency, function) - Call function every frequency milliseconds.
//   NewPingESP8266::timer_stop() - Stop the timer.
//
// HISTORY:
// 07/30/2016 v1.8 - Added support for non-AVR microcontrollers. For non-AVR
//   microcontrollers, advanced ping_timer() timer methods are disabled due to
//   inconsistencies or no support at all between platforms. However, standard
//   ping methods are all supported. Added new optional variable to ping(),
//   ping_in(), ping_cm(), ping_median(), and ping_timer() methods which allows
//   you to set a new maximum distance for each ping. Added support for the
//   ATmega16, ATmega32 and ATmega8535 microcontrollers. Changed convert_cm()
//   and convert_in() methods to static members. You can now call them without
//   an object. For example: cm = NewPingESP8266::convert_cm(distance);
//
// 09/29/2015 v1.7 - Removed support for the Arduino Due and Zero because
//   they're both 3.3 volt boards and are not 5 volt tolerant while the HC-SR04
//   is a 5 volt sensor.  Also, the Due and Zero don't support pin manipulation
//   compatibility via port registers which can be done (see the Teensy 3.2).
//
// 06/17/2014 v1.6 - Corrected delay between pings when using ping_median()
//   method. Added support for the URM37 sensor (must change URM37_ENABLED from
//   false to true). Added support for Arduino microcontrollers like the $20
//   32 bit ARM Cortex-M4 based Teensy 3.2. Added automatic support for the
//   Atmel ATtiny family of microcontrollers. Added timer support for the
//   ATmega8 microcontroller. Rounding disabled by default, reduces compiled
//   code size (can be turned on with ROUNDING_ENABLED switch). Added
//   TIMER_ENABLED switch to get around compile-time "__vector_7" errors when
//   using the Tone library, or you can use the toneAC, NewTone or
//   TimerFreeTone libraries: https://bitbucket.org/teckel12/arduino-toneac/
//   Other speed and compiled size optimizations.
//
// 08/15/2012 v1.5 - Added ping_median() method which does a user specified
//   number of pings (default=5) and returns the median ping in microseconds
//   (out of range pings ignored). This is a very effective digital filter.
//   Optimized for smaller compiled size (even smaller than sketches that
//   don't use a library).
//
// 07/14/2012 v1.4 - Added support for the Parallax PING)))� sensor. Interface
//   with all but the SRF06 sensor using only one Arduino pin. You can also
//   interface with the SRF06 using one pin if you install a 0.1uf capacitor
//   on the trigger and echo pins of the sensor then tie the trigger pin to
//   the Arduino pin (doesn't work with Teensy). To use the same Arduino pin
//   for trigger and echo, specify the same pin for both values. Various bug
//   fixes.
//
// 06/08/2012 v1.3 - Big feature addition, event-driven ping! Uses Timer2
//   interrupt, so be mindful of PWM or timing conflicts messing with Timer2
//   may cause (namely PWM on pins 3 & 11 on Arduino, PWM on pins 9 and 10 on
//   Mega, and Tone library). Simple to use timer interrupt functions you can
//   use in your sketches totally unrelated to ultrasonic sensors (don't use if
//   you're also using NewPingESP8266's ping_timer because both use Timer2 interrupts).
//   Loop counting ping method deleted in favor of timing ping method after
//   inconsistent results kept surfacing with the loop timing ping method.
//   Conversion to cm and inches now rounds to the nearest cm or inch. Code
//   optimized to save program space and fixed a couple minor bugs here and
//   there. Many new comments added as well as line spacing to group code
//   sections for better source readability.
//
// 05/25/2012 v1.2 - Lots of code clean-up thanks to Arduino Forum members.
//   Rebuilt the ping timing code from scratch, ditched the pulseIn code as it
//   doesn't give correct results (at least with ping sensors). The NewPingESP8266
//   library is now VERY accurate and the code was simplified as a bonus.
//   Smaller and faster code as well. Fixed some issues with very close ping
//   results when converting to inches. All functions now return 0 only when
//   there's no ping echo (out of range) and a positive value for a successful
//   ping. This can effectively be used to detect if something is out of range
//   or in-range and at what distance. Now compatible with Arduino 0023.
//
// 05/16/2012 v1.1 - Changed all I/O functions to use low-level port registers
//   for ultra-fast and lean code (saves from 174 to 394 bytes). Tested on both
//   the Arduino Uno and Teensy 2.0 but should work on all Arduino-based
//   platforms because it calls standard functions to retrieve port registers
//   and bit masks. Also made a couple minor fixes to defines.
//
// 05/15/2012 v1.0 - Initial release.
// ---------------------------------------------------------------------------

#ifndef NewPingESP8266_h
#define NewPingESP8266_h

#if defined (ARDUINO) && ARDUINO >= 100
	#include <Arduino.h>
#else
	#include <WProgram.h>
	#include <pins_arduino.h>
#endif

#if defined (__AVR__)
	#include <avr/io.h>
	#include <avr/interrupt.h>
#endif

// Shouldn't need to change these values unless you have a specific need to do so.
#define MAX_SENSOR_DISTANCE 500 // Maximum sensor distance can be as high as 500cm, no reason to wait for ping longer than sound takes to travel this distance and back. Default=500
#define US_ROUNDTRIP_CM 57      // Microseconds (uS) it takes sound to travel round-trip 1cm (2cm total), uses integer to save compiled code space. Default=57
#define US_ROUNDTRIP_IN 146     // Microseconds (uS) it takes sound to travel round-trip 1 inch (2 inches total), uses integer to save compiled code space. Defalult=146
#define ONE_PIN_ENABLED true    // Set to "false" to disable one pin mode which saves around 14-26 bytes of binary size. Default=true
#define ROUNDING_ENABLED false  // Set to "true" to enable distance rounding which also adds 64 bytes to binary size. Default=false
#define URM37_ENABLED false     // Set to "true" to enable support for the URM37 sensor in PWM mode. Default=false

// Probably shouldn't change these values unless you really know what you're doing.
#define NO_ECHO 0               // Value returned if there's no ping echo within the specified MAX_SENSOR_DISTANCE or max_cm_distance. Default=0
#define MAX_SENSOR_DELAY 5800   // Maximum uS it takes for sensor to start the ping. Default=5800
#define ECHO_TIMER_FREQ 24      // Frequency to check for a ping echo (every 24uS is about 0.4cm accuracy). Default=24
#define PING_MEDIAN_DELAY 29000 // Microsecond delay between pings in the ping_median method. Default=29000
#define PING_OVERHEAD 5         // Ping overhead in microseconds (uS). Default=5
#define PING_TIMER_OVERHEAD 13  // Ping timer overhead in microseconds (uS). Default=13
#if URM37_ENABLED == true
	#undef  US_ROUNDTRIP_CM
	#undef  US_ROUNDTRIP_IN
	#define US_ROUNDTRIP_CM 50  // Every 50uS PWM signal is low indicates 1cm distance. Default=50
	#define US_ROUNDTRIP_IN 127 // If 50uS is 1cm, 1 inch would be 127uS (50 x 2.54 = 127). Default=127
#endif

// Conversion from uS to distance (round result to nearest cm or inch).
#define NewPingESP8266Convert(echoTime, conversionFactor) (max(((unsigned int)echoTime + conversionFactor / 2) / conversionFactor, (echoTime ? 1 : 0)))

// Detect non-AVR microcontrollers (Teensy 3.x, Arduino DUE, etc.) and don't use port registers or timer interrupts as required.
#if (defined (__arm__) && defined (TEENSYDUINO))
	#undef  PING_OVERHEAD
	#define PING_OVERHEAD 1
	#undef  PING_TIMER_OVERHEAD
	#define PING_TIMER_OVERHEAD 1
	#define DO_BITWISE true
#elif !defined (__AVR__)
	#undef  PING_OVERHEAD
	#define PING_OVERHEAD 1
	#undef  PING_TIMER_OVERHEAD
	#define PING_TIMER_OVERHEAD 1
	#define DO_BITWISE false
#else
	#define DO_BITWISE true
#endif

// Define timers when using ATmega8, ATmega16, ATmega32 and ATmega8535 microcontrollers.
#if defined (__AVR_ATmega8__) || defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__) || defined (__AVR_ATmega8535__)
	#define OCR2A OCR2
	#define TIMSK2 TIMSK
	#define OCIE2A OCIE2
#endif

class NewPingESP8266 {
	public:
		NewPingESP8266(uint32_t trigger_pin, uint32_t echo_pin, unsigned int max_cm_distance = MAX_SENSOR_DISTANCE);
		unsigned int ping(unsigned int max_cm_distance = 0);
		unsigned long ping_cm(unsigned int max_cm_distance = 0);
		unsigned long ping_in(unsigned int max_cm_distance = 0);
		unsigned long ping_median(uint32_t it = 5, unsigned int max_cm_distance = 0);
		static unsigned int convert_cm(unsigned int echoTime);
		static unsigned int convert_in(unsigned int echoTime);

		unsigned int getMaxEchoTime() { return _maxEchoTime; }
	private:
		boolean ping_trigger();
		void set_max_distance(unsigned int max_cm_distance);
#if DO_BITWISE == true
		uint32_t _triggerBit;
		uint32_t _echoBit;
		volatile uint32_t *_triggerOutput;
		volatile uint32_t *_echoInput;
		volatile uint32_t *_triggerMode;
#else
		uint32_t _triggerPin;
		uint32_t _echoPin;
#endif
		unsigned int _maxEchoTime;
		unsigned long _max_time;
};


#endif
