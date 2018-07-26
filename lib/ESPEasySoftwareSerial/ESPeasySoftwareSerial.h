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

#include <inttypes.h>
#include <Stream.h>


// This class is compatible with the corresponding AVR one,
// the constructor however has an optional rx buffer size.
// Speed up to 115200 can be used.


class ESPeasySoftwareSerial : public Stream
{
public:
   ESPeasySoftwareSerial(uint8_t receivePin, uint8_t transmitPin, bool inverse_logic = false, uint16_t buffSize = 64);
   virtual ~ESPeasySoftwareSerial();

   void begin(long speed);
   void setTransmitEnablePin(uint8_t transmitEnablePin);

   int peek();

   virtual size_t write(uint8_t byte);
   virtual int read();
   virtual int available();
   virtual void flush();
   operator bool() {return m_rxValid || m_txValid;}

   // Disable or enable interrupts on the rx pin
   void enableRx(bool on);

   void rxRead();

   using Print::write;

private:
   bool isValidGPIOpin(uint8_t pin);
   uint8_t pinToIndex(uint8_t pin);

   // Member variables
   uint8_t m_rxPin, m_txPin, m_txEnablePin;
   bool m_rxValid, m_txValid, m_txEnableValid;
   bool m_invert;
   unsigned long m_bitTime;
   uint16_t m_inPos, m_outPos;
   uint16_t m_buffSize;
   uint8_t *m_buffer;

};

// If only one tx or rx wanted then use this as parameter for the unused pin
#define SW_SERIAL_UNUSED_PIN -1


#endif
#endif
