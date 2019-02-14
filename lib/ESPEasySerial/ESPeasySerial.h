/*
ESPeasySerial.h

ESPeasySerial.cpp - Wrapper for Arduino SoftwareSerial and HardwareSerial for ESP8266 and ESP32
Copyright (c) 2018 Gijs Noorlander. All rights reserved.

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

#ifndef ESPeasySerial_h
#define ESPeasySerial_h

#include <Arduino.h>
#include <HardwareSerial.h>
#include <inttypes.h>
#include <Stream.h>

#if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined (ESP32)
  #ifndef DISABLE_SOFTWARE_SERIAL
    #define DISABLE_SOFTWARE_SERIAL
  #endif
#endif

#ifndef DISABLE_SOFTWARE_SERIAL
#include <ESPEasySoftwareSerial.h>
#endif

#ifdef ESP32
  #define NR_ESPEASY_SERIAL_TYPES 3 // Serial 0, 1, 2
#endif
#if !defined(DISABLE_SOFTWARE_SERIAL) && defined(ESP8266)
  #define NR_ESPEASY_SERIAL_TYPES 4 // Serial 0, 1, 0_swap, software
#else
  #define NR_ESPEASY_SERIAL_TYPES 3 // Serial 0, 1, 0_swap
#endif


struct ESPeasySerialType {
  enum serialtype {
    software = 0,
    serial0,
    serial0_swap,
    serial1,
    serial2,

    MAX_SERIAL_TYPE
  };


  static bool getSerialTypePins(ESPeasySerialType::serialtype serType, int& rxPin, int& txPin) {
    rxPin = -1;
    txPin = -1;
    switch (serType) {
      case ESPeasySerialType::serialtype::serial0:  rxPin = 3; txPin = 1; return true;
#ifdef ESP32
      case ESPeasySerialType::serialtype::serial1:  rxPin = 13; txPin = 15; return true;
      case ESPeasySerialType::serialtype::serial2:  rxPin = 16; txPin = 17; return true;
#endif
#ifdef ESP8266
      case ESPeasySerialType::serialtype::serial0_swap:  rxPin = 13; txPin = 15; return true;
      case ESPeasySerialType::serialtype::serial1:       rxPin = -1; txPin = 2; return true;
    #ifndef DISABLE_SOFTWARE_SERIAL
      case ESPeasySerialType::serialtype::software:      rxPin = 14; txPin = 12; return true;
    #endif // DISABLE_SOFTWARE_SERIAL
#endif
      default:
        break;
    }
    return false;
  }


#ifdef ESP32
  static ESPeasySerialType::serialtype getSerialType(int receivePin, int transmitPin) {
    if (receivePin == 3 && transmitPin == 1) {
      return serialtype::serial0; // UART0
    }
    if (receivePin == 16 && transmitPin == 17) {
      return serialtype::serial2; // UART2
    }
    // Serial1 on ESP32 uses default pins connected to flash
    // So must make sure to set them to other pins.
    return serialtype::serial1;
  }

#endif // ESP32


#ifdef ESP8266
  static ESPeasySerialType::serialtype getSerialType(int receivePin, int transmitPin) {
    if (receivePin == 3 && transmitPin == 1) {
      return serialtype::serial0; // UART0
    }
    // ESP8266
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
#endif // ESP8266

};


class ESPeasySerial : public Stream
{
public:

#ifdef ESP8266
  // ESP82xx has 2 HW serial ports and option for several software serial ports.
  // Serial0:         RX: 3  TX: 1
  // Serial0 swapped  RX: 13 TX: 15
  // Serial1:         RX: -- TX: 2   (TX only)
  ESPeasySerial(int receivePin, int transmitPin, bool inverse_logic = false, unsigned int buffSize = 64);
  virtual ~ESPeasySerial();

  // If baud rate is set to 0, it will perform an auto-detect on the baudrate
  void begin(unsigned long baud, SerialConfig config=SERIAL_8N1, SerialMode mode=SERIAL_FULL);
#endif


#ifdef ESP32
  // ESP32 has 3 HW serial ports.
  // Serial0: RX: 3  TX: 1
  // Serial1: RX: 9  TX: 10  Defaults will never work, share pins with flash
  // Serial2: RX: 16 TX: 17
  // Pins set in the constructor will be used as override when not given when calling begin()
  // @param  inverse_logic can be used to set the logic in the constructor which will then be used in the call to begin.
  //         This makes the call to the constructor more in line with the constructor of SoftwareSerial.
  // @param  serialPort can be a helper to suggest the set serial port. (is needed to define Serial1)
  ESPeasySerial(int receivePin, int transmitPin, bool inverse_logic = false, int serialPort = -1);
  virtual ~ESPeasySerial();

  // If baud rate is set to 0, it will perform an auto-detect on the baudrate
  void begin(unsigned long baud, uint32_t config=SERIAL_8N1, int8_t rxPin=-1, int8_t txPin=-1, bool invert=false, unsigned long timeout_ms = 20000UL);
#endif

  void end();
  int peek(void);
  size_t write(uint8_t byte) override;
  int read(void) override;
  int available(void) override;
  void flush(void) override;

#if defined(ESP8266)
  bool overflow();        // SoftwareSerial ESP8266
  bool hasOverrun(void);  // HardwareSerial ESP8266
#endif

/*
  // FIXME TD-er: See https://www.artima.com/cppsource/safebool.html
  operator bool() {
    if (!isValid()) {
      return false;
    }
    if (isSWserial()) {
      return _swserial->bool();
    } else {
      return getHW()->bool();
    }
  }
  */

  // HardwareSerial specific:
  size_t write(const uint8_t *buffer, size_t size);
  size_t write(const char *buffer);
  int baudRate(void);

#if defined(ESP8266)
  void swap() { swap(_transmitPin); }
  void swap(uint8_t tx_pin);
  size_t readBytes(char* buffer, size_t size) override;
  size_t readBytes(uint8_t* buffer, size_t size) override;

  void setDebugOutput(bool);
  bool isTxEnabled(void);
  bool isRxEnabled(void);
#ifdef CORE_2_5_0
  bool hasRxError(void);
#endif // CORE_2_5_0

  void startDetectBaudrate();
  unsigned long testBaudrate();
  unsigned long detectBaudrate(time_t timeoutMillis);

  // SoftwareSerial specific
  void setTransmitEnablePin(uint8_t transmitEnablePin);
  // AVR compatibility methods
  bool isListening();
  bool stopListening();

  bool serial0_swap_active() const { return _serial0_swap_active; }
#endif
  bool listen();


  using Print::write;

private:

  const HardwareSerial* getHW() const;
  HardwareSerial* getHW();

  bool isValid() const;

#ifdef ESP8266
  bool doHWbegin(unsigned long baud, SerialConfig config, SerialMode mode);
#endif

#if !defined(DISABLE_SOFTWARE_SERIAL) && defined(ESP8266)
  bool isSWserial() const { return _serialtype == ESPeasySerialType::serialtype::software; }

  ESPeasySoftwareSerial* _swserial = nullptr;
#endif
#ifdef ESP8266
  static bool _serial0_swap_active;
#endif // ESP8266

  ESPeasySerialType::serialtype _serialtype = ESPeasySerialType::serialtype::MAX_SERIAL_TYPE;
  int _receivePin;
  int _transmitPin;
  unsigned long _baud = 0;
  bool _inverse_logic = false;

};

#endif
