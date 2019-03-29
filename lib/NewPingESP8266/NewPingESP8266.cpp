// ---------------------------------------------------------------------------
// Created by Tim Eckel - teckel@leethost.com
// Copyright 2016 License: GNU GPL v3 http://www.gnu.org/licenses/gpl.html
//
// See "NewPingESP8266.h" for purpose, syntax, version history, links, and more.
// ---------------------------------------------------------------------------

#include "NewPingESP8266.h"
//#include "ESPEasy-Globals.h"
// ---------------------------------------------------------------------------
// NewPingESP8266 constructor
// ---------------------------------------------------------------------------

NewPingESP8266::NewPingESP8266(uint32_t trigger_pin, uint32_t echo_pin, unsigned int max_cm_distance) {
	_errorState = STATUS_SENSOR_READY;
#if DO_BITWISE == true
	_triggerBit = digitalPinToBitMask(trigger_pin); // Get the port register bitmask for the trigger pin.
	_echoBit = digitalPinToBitMask(echo_pin);       // Get the port register bitmask for the echo pin.

	_triggerOutput = portOutputRegister(digitalPinToPort(trigger_pin)); // Get the output port register for the trigger pin.
	_echoInput = portInputRegister(digitalPinToPort(echo_pin));         // Get the input port register for the echo pin.

	_triggerMode = (uint32_t *) portModeRegister(digitalPinToPort(trigger_pin)); // Get the port mode register for the trigger pin.
#else
	_triggerPin = trigger_pin;
	_echoPin = echo_pin;
#endif

	set_max_distance(max_cm_distance); // Call function to set the max sensor distance.

#if (defined (__arm__) && defined (TEENSYDUINO)) || DO_BITWISE != true
	pinMode(echo_pin, INPUT);     // Set echo pin to input (on Teensy 3.x (ARM), pins default to disabled, at least one pinMode() is needed for GPIO mode).
	pinMode(trigger_pin, OUTPUT); // Set trigger pin to output (on Teensy 3.x (ARM), pins default to disabled, at least one pinMode() is needed for GPIO mode).
#endif

#if defined (ARDUINO_AVR_YUN)
	pinMode(echo_pin, INPUT);     // Set echo pin to input for the Arduino Yun, not sure why it doesn't default this way.
#endif

#if ONE_PIN_ENABLED != true && DO_BITWISE == true
	*_triggerMode |= _triggerBit; // Set trigger pin to output.
#endif
}


// ---------------------------------------------------------------------------
// Standard ping methods
// ---------------------------------------------------------------------------

unsigned int NewPingESP8266::ping(unsigned int max_cm_distance) {
	if (max_cm_distance > 0) set_max_distance(max_cm_distance); // Call function to set a new max sensor distance.

	if (!ping_trigger()) return NO_ECHO; // Trigger a ping, if it returns false, return NO_ECHO to the calling function.

#if URM37_ENABLED == true
	#if DO_BITWISE == true
		while (!(*_echoInput & _echoBit))             // Wait for the ping echo.
	#else
		while (!digitalRead(_echoPin))                // Wait for the ping echo.
	#endif
	  {
			if ((_max_cm_distance > 0) && (micros() > _max_time)) {
				_errorState = STATUS_MAX_DISTANCE_EXCEEDED;
				return NO_ECHO; // Stop the loop and return NO_ECHO (false) if we're beyond the set maximum distance.
			}
	  }
#else
	#if DO_BITWISE == true
		while (*_echoInput & _echoBit)                // Wait for the ping echo.
	#else
		while (digitalRead(_echoPin))                 // Wait for the ping echo.
	#endif
		{
			if ((_max_cm_distance > 0) && (micros() > _max_time)) {
				_errorState = STATUS_MAX_DISTANCE_EXCEEDED;
				return NO_ECHO; // Stop the loop and return NO_ECHO (false) if we're beyond the set maximum distance.
			}
		}
#endif

	_errorState = STATUS_MEASUREMENT_VALID;
	return (micros() - (_max_time - _maxEchoTime) - PING_OVERHEAD); // Calculate ping time, include overhead.
}


unsigned long NewPingESP8266::ping_cm(unsigned int max_cm_distance) {
	unsigned long echoTime = NewPingESP8266::ping(max_cm_distance); // Calls the ping method and returns with the ping echo distance in uS.
#if ROUNDING_ENABLED == false
	return (echoTime / US_ROUNDTRIP_CM);              // Call the ping method and returns the distance in centimeters (no rounding).
#else
	return NewPingESP8266Convert(echoTime, US_ROUNDTRIP_CM); // Convert uS to centimeters.
#endif
}


unsigned long NewPingESP8266::ping_in(unsigned int max_cm_distance) {
	unsigned long echoTime = NewPingESP8266::ping(max_cm_distance); // Calls the ping method and returns with the ping echo distance in uS.
#if ROUNDING_ENABLED == false
	return (echoTime / US_ROUNDTRIP_IN);              // Call the ping method and returns the distance in inches (no rounding).
#else
	return NewPingESP8266Convert(echoTime, US_ROUNDTRIP_IN); // Convert uS to inches.
#endif
}


unsigned long NewPingESP8266::ping_median(uint32_t it, unsigned int max_cm_distance) {
	unsigned int uS[it], last;
	uint32_t j, i = 0;
	unsigned long t;
	uS[0] = NO_ECHO;

	while (i < it) {
		t = micros();                  // Start ping timestamp.
		last = ping(max_cm_distance);  // Send ping.

		if (last != NO_ECHO) {         // Ping in range, include as part of median.
			if (i > 0) {               // Don't start sort till second ping.
				for (j = i; j > 0 && uS[j - 1] < last; j--) // Insertion sort loop.
					uS[j] = uS[j - 1];                      // Shift ping array to correct position for sort insertion.
			} else j = 0;              // First ping is sort starting point.
			uS[j] = last;              // Add last ping to array in sorted position.
			i++;                       // Move to next ping.
		} else it--;                   // Ping out of range, skip and don't include as part of median.

		if (i < it && micros() - t < PING_MEDIAN_DELAY)
			delay((PING_MEDIAN_DELAY + t - micros()) / 1000); // Millisecond delay between pings.

	}
	return (uS[it >> 1]); // Return the ping distance median.
}


// ---------------------------------------------------------------------------
// Standard and timer interrupt ping method support functions (not called directly)
// ---------------------------------------------------------------------------

boolean NewPingESP8266::ping_trigger() {
	unsigned long start = millis();
#if DO_BITWISE == true
	#if ONE_PIN_ENABLED == true
		*_triggerMode |= _triggerBit;  // Set trigger pin to output.
	#endif

	*_triggerOutput &= ~_triggerBit;   // Set the trigger pin low, should already be low, but this will make sure it is.
	delayMicroseconds(4);              // Wait for pin to go low.
	*_triggerOutput |= _triggerBit;    // Set trigger pin high, this tells the sensor to send out a ping.
	delayMicroseconds(10);             // Wait long enough for the sensor to realize the trigger pin is high. Sensor specs say to wait 10uS.
	*_triggerOutput &= ~_triggerBit;   // Set trigger pin back to low.

	#if ONE_PIN_ENABLED == true
		*_triggerMode &= ~_triggerBit; // Set trigger pin to input (when using one Arduino pin, this is technically setting the echo pin to input as both are tied to the same Arduino pin).
	#endif

	#if URM37_ENABLED == true
		if (!(*_echoInput & _echoBit)) {								      	// Previous ping hasn't finished, abort.{
			_errorState = STATUS_ECHO_STATE_ERROR;
			return false;
		}
		_max_time = micros() + _maxEchoTime + MAX_SENSOR_DELAY; // Maximum time we'll wait for ping to start (most sensors are <450uS, the SRF06 can take up to 34,300uS!)
		while (*_echoInput & _echoBit)                          // Wait for ping to start.
		{
			if (millis() > (start + 50)) {
				_errorState = STATUS_ECHO_START_TIMEOUT_50ms;
				return false;
			}
			if ((_max_cm_distance > 0) && (micros() > _max_time)) {	// check max. distance if != 0 cm
				_errorState = STATUS_ECHO_START_TIMEOUT_DISTANCE;
				return false;             													// echo too late, max distance exceeded
			}
		}
	#else
		if (*_echoInput & _echoBit) { 				              		// Previous ping hasn't finished, abort.
			_errorState = STATUS_ECHO_STATE_ERROR;
			return false;
		}
		_max_time = micros() + _maxEchoTime + MAX_SENSOR_DELAY; // Maximum time we'll wait for ping to start (most sensors are <450uS, the SRF06 can take up to 34,300uS!)
		while (!(*_echoInput & _echoBit))                       // Wait for ping to start.
		{
			if (millis() > (start + 50)) {
				_errorState = STATUS_ECHO_START_TIMEOUT_50ms;
				return false;
			}
			if ((_max_cm_distance > 0) && (micros() > _max_time)) {	// check max. distance if != 0 cm
				_errorState = STATUS_ECHO_START_TIMEOUT_DISTANCE;
				return false;             													// echo too late, max distance exceeded
			}
		}
	#endif
#else
	#if ONE_PIN_ENABLED == true
		pinMode(_triggerPin, OUTPUT); // Set trigger pin to output.
	#endif

	digitalWrite(_triggerPin, LOW);   // Set the trigger pin low, should already be low, but this will make sure it is.
	delayMicroseconds(4);             // Wait for pin to go low.
	digitalWrite(_triggerPin, HIGH);  // Set trigger pin high, this tells the sensor to send out a ping.
	delayMicroseconds(10);            // Wait long enough for the sensor to realize the trigger pin is high. Sensor specs say to wait 10uS.
	digitalWrite(_triggerPin, LOW);   // Set trigger pin back to low.

	#if ONE_PIN_ENABLED == true
		pinMode(_triggerPin, INPUT);  // Set trigger pin to input (when using one Arduino pin, this is technically setting the echo pin to input as both are tied to the same Arduino pin).
	#endif

	#if URM37_ENABLED == true
		if (!digitalRead(_echoPin)) {
			_errorState = STATUS_ECHO_STATE_ERROR;
			return false;                // Previous ping hasn't finished, abort.
		}
		_max_time = micros() + _maxEchoTime + MAX_SENSOR_DELAY; // Maximum time we'll wait for ping to start (most sensors are <450uS, the SRF06 can take up to 34,300uS!)
		while (digitalRead(_echoPin))                           // Wait for ping to start.
		{
			if (millis() > (start + 50)) {
				_errorState = STATUS_ECHO_START_TIMEOUT_50ms;
				return false;
			}
			if ((_max_cm_distance > 0) && (micros() > _max_time)) {	// check max. distance if != 0 cm
				_errorState = STATUS_ECHO_START_TIMEOUT_DISTANCE;
				return false;             													// echo too late, max distance exceeded
			}
		}
	#else
		if (digitalRead(_echoPin)) {
			_errorState = STATUS_ECHO_STATE_ERROR;
			return false;                // Previous ping hasn't finished, abort.
		}
		_max_time = micros() + _maxEchoTime + MAX_SENSOR_DELAY; // Maximum time we'll wait for ping to start (most sensors are <450uS, the SRF06 can take up to 34,300uS!)
		while (!digitalRead(_echoPin))                          // Wait for ping to start.
		{
			if (millis() > (start + 50)) {
				_errorState = STATUS_ECHO_START_TIMEOUT_50ms;
				return false;
			}
			if ((_max_cm_distance > 0) && (micros() > _max_time)) {	// check max. distance if != 0 cm
				_errorState = STATUS_ECHO_START_TIMEOUT_DISTANCE;
				return false;             													// echo too late, max distance exceeded
			}
  	}
	#endif
#endif

	_max_time = micros() + _maxEchoTime; // Ping started, set the time-out.
	_errorState = STATUS_ECHO_TRIGGERED;

	return true;                         // Ping started successfully.
}


void NewPingESP8266::set_max_distance(unsigned int max_cm_distance) {
	_max_cm_distance = max_cm_distance;
#if ROUNDING_ENABLED == false
	_maxEchoTime = min(max_cm_distance + 1, (unsigned int) MAX_SENSOR_DISTANCE + 1) * US_ROUNDTRIP_CM; // Calculate the maximum distance in uS (no rounding).
#else
	_maxEchoTime = min(max_cm_distance, (unsigned int) MAX_SENSOR_DISTANCE) * US_ROUNDTRIP_CM + (US_ROUNDTRIP_CM / 2); // Calculate the maximum distance in uS.
#endif
}


// ---------------------------------------------------------------------------
// Conversion methods (rounds result to nearest cm or inch).
// ---------------------------------------------------------------------------

unsigned int NewPingESP8266::convert_cm(unsigned int echoTime) {
#if ROUNDING_ENABLED == false
	return (echoTime / US_ROUNDTRIP_CM);              // Convert uS to centimeters (no rounding).
#else
	return NewPingESP8266Convert(echoTime, US_ROUNDTRIP_CM); // Convert uS to centimeters.
#endif
}


unsigned int NewPingESP8266::convert_in(unsigned int echoTime) {
#if ROUNDING_ENABLED == false
	return (echoTime / US_ROUNDTRIP_IN);              // Convert uS to inches (no rounding).
#else
	return NewPingESP8266Convert(echoTime, US_ROUNDTRIP_IN); // Convert uS to inches.
#endif
}

float NewPingESP8266::convert_cm_F(unsigned int echoTime) {
	return ((float)echoTime / US_ROUNDTRIP_CM);              // Convert uS to centimeters (no rounding).
}


float NewPingESP8266::convert_in_F(unsigned int echoTime) {
	return ((float)echoTime / US_ROUNDTRIP_IN);              // Convert uS to inches (no rounding).
}
