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
                                                                   const uint8_t latchPin) :
  // set attributes
  _size(size), _clockPin(clockPin), _serialDataPin(serialDataPin), _latchPin(latchPin) {

  // define pins as outputs
  pinMode(_clockPin,      OUTPUT);
  pinMode(_serialDataPin, OUTPUT);
  pinMode(_latchPin,      OUTPUT);

  // set pins low
  digitalWrite(_clockPin,      LOW);
  digitalWrite(_serialDataPin, LOW);
  digitalWrite(_latchPin,      LOW);

  // allocates the specified number of bytes and initializes them to zero
  _digitalValues.resize(_size, 0);

  // Do _not_ update the empty buffer to the register, we want to retain the current state
  // updateRegisters(); // reset shift register
}

// Set a new size for the mnumber of shift registers
void ShiftRegister74HC595_NonTemplate::setSize(const uint8_t size) {
  _size = size;
  _digitalValues.resize(_size, 0); // Reset new values to 0 when enlarging
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
void ShiftRegister74HC595_NonTemplate::set(const uint8_t pin, const uint16_t value, const bool update) {
  (value) ? bitSet(_digitalValues[pin / 8], pin % 8) : bitClear(_digitalValues[pin / 8], pin % 8);

  if (update) {
    updateRegisters();
  }
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

// Returns the state of the given pin.
// Either HIGH (1) or LOW (0)
const uint8_t ShiftRegister74HC595_NonTemplate::get(const uint8_t pin) const {
  return (_digitalValues[pin / 8] >> (pin % 8)) & 1;
}

// Sets all pins of all shift registers to HIGH (1).
void ShiftRegister74HC595_NonTemplate::setAllHigh() {
  _digitalValues.assign(_size, 255);
  updateRegisters();
}

// Sets all pins of all shift registers to LOW (0).
void ShiftRegister74HC595_NonTemplate::setAllLow() {
  _digitalValues.assign(_size, 0);
  updateRegisters();
}
