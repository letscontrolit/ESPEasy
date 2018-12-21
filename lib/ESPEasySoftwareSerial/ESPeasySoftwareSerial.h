/*
ESPeasySoftwareSerial.h

ESPeasySoftwareSerial.cpp - Implementation of the Arduino software serial for ESP8266.
Copyright (c) 2015-2016 Peter Lerup. All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/
#ifdef ESP8266  // Needed for precompile issues.
#ifndef ESPeasySoftwareSerial_h
#define ESPeasySoftwareSerial_h

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <inttypes.h>
#include <Stream.h>


// This class is compatible with the corresponding AVR one,
// the constructor however has an optional rx buffer size.
// Speed up to 115200 can be used.

struct ESPeasySerialType {
  enum serialtype {
    serial0,
    serial0_swap,
    serial1,
    software
  };

  static ESPeasySerialType::serialtype getSerialType(int receivePin, int transmitPin) {
    if (receivePin == 3 && transmitPin == 1) {
      return serialtype::serial0; // UART0
    }
    if (receivePin == 13 && transmitPin == 15) {
      return serialtype::serial0_swap; // UART0 remapped using Serial.swap()
    }
    if (receivePin == -1 && transmitPin == 2) {
      // Serial1 uses UART1, TX pin is GPIO2.
      // UART1 can not be used to receive data because normally
      // it's RX pin is occupied for flash chip connection.
      return serialtype::serial1;
    }
    return serialtype::software;
  }

  static void getPinsFromSerialType(ESPeasySerialType::serialtype s_type, int& receivePin, int& transmitPin) {
    switch (s_type) {
      case serialtype::serial0:      receivePin = 3;  transmitPin = 1;  break;
      case serialtype::serial0_swap: receivePin = 13; transmitPin = 15; break;
      case serialtype::serial1:      receivePin = -1; transmitPin = 2;  break;
      case serialtype::software:
      default:
        break;
    }
  }

};

class ESPeasySoftwareSerial : public Stream
{
public:

   ESPeasySoftwareSerial(int receivePin, int transmitPin, bool inverse_logic = false, unsigned int buffSize = 64);
   virtual ~ESPeasySoftwareSerial();

   void begin(unsigned long baud, SerialConfig config=SERIAL_8N1, SerialMode mode=SERIAL_FULL, uint8_t tx_pin=1);

   void end();
   int peek(void);
   size_t write(uint8_t byte) override;
   int read(void) override;
   int available(void) override;
   void flush(void) override;

   bool overflow();        // SoftwareSerial
   bool hasOverrun(void);  // HardwareSerial


   // FIXME TD-er: See https://www.artima.com/cppsource/safebool.html
   operator bool() const;

   // HardwareSerial specific:
   void swap();
   void swap(uint8_t tx_pin);
   size_t write(const uint8_t *buffer, size_t size);
   size_t write(const char *buffer);
   size_t readBytes(char* buffer, size_t size) override;
   size_t readBytes(uint8_t* buffer, size_t size) override;
   int baudRate(void);


   // SoftwareSerial specific
  void setTransmitEnablePin(uint8_t transmitEnablePin);
  // AVR compatibility methods
  bool listen() { enableRx(true); return true; }
  bool isListening() { return m_rxEnabled; }
  bool stopListening() { enableRx(false); return true; }

   using Print::write;

   bool serial0_swap_active() { return _serial0_swap_active; }

private:

  HardwareSerial* getHW();

  bool isValid();

  bool isSWserial() { return _serialtype == ESPeasySerialType::serialtype::software; }

  SoftwareSerial* _swserial = nullptr;
  ESPeasySerialType::serialtype _serialtype;
  int _receivePin; // Needed?
  int _transmitPin;// Needed?
  unsigned long _baud = 0;
  static bool _serial0_swap_active = false;

};

#endif
#endif
