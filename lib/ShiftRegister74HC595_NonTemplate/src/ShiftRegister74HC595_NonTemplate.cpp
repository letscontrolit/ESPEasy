/*
   ShiftRegister74HC595_NonTemplate.cpp Modified for ESPEasy, enabling runtime sizing
   Based on ShiftRegister74HC595.cpp - Library for simplified control of 74HC595 shift registers.
   Developed and maintained by Timo Denk and contributers, since Nov 2014.
   Additional information is available at https://timodenk.com/blog/shift-register-arduino-library/
   Released into the public domain.
 */

#include <Arduino.h>
#include "ShiftRegister74HC595_NonTemplate.h"

/*
   Code, sans template arguments, from ShiftRegister74HC595.hpp - Library for simplified control of 74HC595 shift registers.
   Developed and maintained by Timo Denk and contributers, since Nov 2014.
   Additional information is available at https://timodenk.com/blog/shift-register-arduino-library/
   Released into the public domain.
 */

// ShiftRegister74HC595_NonTemplate constructor
// Size is the number of shiftregisters stacked in serial
ShiftRegister74HC595_NonTemplate::ShiftRegister74HC595_NonTemplate(const uint8_t size,
                                                                   const uint8_t serialDataPin,
                                                                   const uint8_t clockPin,
                                                                   const uint8_t latchPin) {
  // set attributes
  _size          = size;
  _clockPin      = clockPin;
  _serialDataPin = serialDataPin;
  _latchPin      = latchPin;

  // define pins as outputs
  pinMode(clockPin,      OUTPUT);
  pinMode(serialDataPin, OUTPUT);
  pinMode(latchPin,      OUTPUT);

  // set pins low
  digitalWrite(clockPin,      LOW);
  digitalWrite(serialDataPin, LOW);
  digitalWrite(latchPin,      LOW);

  // allocates the specified number of bytes and initializes them to zero
  _digitalValues.resize(_size, 0);

  updateRegisters(); // reset shift register
}

// Set all pins of the shift registers at once. Do not yet update the registers if update is false
// digitalVAlues is a uint8_t array where the length is equal to the number of shift registers.
void ShiftRegister74HC595_NonTemplate::setAll(const uint8_t *digitalValues, bool update) {
  for (int i = 0; i < _size; i++) {
    _digitalValues[i] = digitalValues[i]; // Length check should be done elsewhere (prerequisite)
  }

  if (update) {
    updateRegisters();
  }
}

// Retrieve all states of the shift registers' output pins.
// The returned array's length is equal to the number of shift registers.
const uint8_t * ShiftRegister74HC595_NonTemplate::getAll() const {
  return &_digitalValues[0];
}

// Set a specific pin to either HIGH (1) or LOW (0).
// The pin parameter is a positive, zero-based integer, indicating which pin to set.
void ShiftRegister74HC595_NonTemplate::set(const uint8_t pin, const uint8_t value) {
  setNoUpdate(pin, value);
  updateRegisters();
}

// Updates the shift register pins to the stored output values.
// This is the function that actually writes data into the shift registers of the 74HC595.
void ShiftRegister74HC595_NonTemplate::updateRegisters() {
  for (int i = _size - 1; i >= 0; i--) {
    shiftOut(_serialDataPin, _clockPin, MSBFIRST, _digitalValues[i]);
  }

  digitalWrite(_latchPin, HIGH);
  digitalWrite(_latchPin, LOW);
}

// Equivalent to set(int pin, uint8_t value), except the physical shift register is not updated.
// Should be used in combination with updateRegisters().
void ShiftRegister74HC595_NonTemplate::setNoUpdate(const uint8_t pin, const uint8_t value) {
  (value) ? bitSet(_digitalValues[pin / 8], pin % 8) : bitClear(_digitalValues[pin / 8], pin % 8);
}

// Returns the state of the given pin.
// Either HIGH (1) or LOW (0)
const uint8_t ShiftRegister74HC595_NonTemplate::get(const uint8_t pin) const {
  return (_digitalValues[pin / 8] >> (pin % 8)) & 1;
}

// Sets all pins of all shift registers to HIGH (1).
void ShiftRegister74HC595_NonTemplate::setAllHigh() {
  _digitalValues.clear();
  _digitalValues.resize(_size, 255);
  updateRegisters();
}

// Sets all pins of all shift registers to LOW (0).
void ShiftRegister74HC595_NonTemplate::setAllLow() {
  _digitalValues.clear();
  _digitalValues.resize(_size, 0);
  updateRegisters();
}
