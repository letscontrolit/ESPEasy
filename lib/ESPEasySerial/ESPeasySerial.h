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
#ifndef DISABLE_SC16IS752_Serial
# include <ESPEasySC16IS752_Serial.h>
#endif // ifndef DISABLE_SC16IS752_Serial

#if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(ESP32)
  # ifndef DISABLE_SOFTWARE_SERIAL
    #  define DISABLE_SOFTWARE_SERIAL
  # endif // ifndef DISABLE_SOFTWARE_SERIAL
#endif // if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(ESP32)

#ifndef DISABLE_SOFTWARE_SERIAL
# include <ESPEasySoftwareSerial.h>
#endif // ifndef DISABLE_SOFTWARE_SERIAL

#ifdef ESP32
  # ifndef DISABLE_SC16IS752_Serial
    #  define NR_ESPEASY_SERIAL_TYPES 4 // Serial 0, 1, 2, sc16is752
  # else // ifndef DISABLE_SC16IS752_Serial
    #  define NR_ESPEASY_SERIAL_TYPES 3 // Serial 0, 1, 2
  # endif // ifndef DISABLE_SC16IS752_Serial
#endif // ifdef ESP32
#ifdef ESP8266
  # if !defined(DISABLE_SOFTWARE_SERIAL)
    #  ifndef DISABLE_SC16IS752_Serial
      #   define NR_ESPEASY_SERIAL_TYPES 5 // Serial 0, 1, 0_swap, software, sc16is752
    #  else // ifndef DISABLE_SC16IS752_Serial
      #   define NR_ESPEASY_SERIAL_TYPES 4 // Serial 0, 1, 0_swap, software
    #  endif // ifndef DISABLE_SC16IS752_Serial
  # else // if !defined(DISABLE_SOFTWARE_SERIAL)
    #  ifndef DISABLE_SC16IS752_Serial
      #   define NR_ESPEASY_SERIAL_TYPES 4 // Serial 0, 1, 0_swap, sc16is752
    #  else // ifndef DISABLE_SC16IS752_Serial
      #   define NR_ESPEASY_SERIAL_TYPES 3 // Serial 0, 1, 0_swap
    #  endif // ifndef DISABLE_SC16IS752_Serial
  # endif // if !defined(DISABLE_SOFTWARE_SERIAL)
#endif // ifdef ESP8266

#ifndef ESP32
  # if defined(ARDUINO_ESP8266_RELEASE_2_4_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_1)  || defined(ARDUINO_ESP8266_RELEASE_2_4_2)
    #  ifndef CORE_2_4_X
      #   define CORE_2_4_X
    #  endif // ifndef CORE_2_4_X
  # endif // if defined(ARDUINO_ESP8266_RELEASE_2_4_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_1)  || defined(ARDUINO_ESP8266_RELEASE_2_4_2)

  # if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_1)
    #  ifndef CORE_PRE_2_4_2
      #   define CORE_PRE_2_4_2
    #  endif // ifndef CORE_PRE_2_4_2
  # endif // if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_1)

  # if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(CORE_2_4_X)
    #  ifndef CORE_PRE_2_5_0
      #   define CORE_PRE_2_5_0
    #  endif // ifndef CORE_PRE_2_5_0
  # else // if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(CORE_2_4_X)
    #  ifndef CORE_POST_2_5_0
      #   define CORE_POST_2_5_0
    #  endif // ifndef CORE_POST_2_5_0
  # endif // if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(CORE_2_4_X)
#endif // ESP32


#include "ESPEasySerialPort.h"
#include "ESPEasySerialType.h"


class ESPeasySerial : public Stream {
public:

#ifdef ESP8266

  // ESP82xx has 2 HW serial ports and option for several software serial ports.
  // Serial0:         RX: 3  TX: 1
  // Serial0 swapped  RX: 13 TX: 15
  // Serial1:         RX: -- TX: 2   (TX only)
  // SC16IS752:       Rx: I2C addr  TX: channel (A = 0, B = 1)
  ESPeasySerial(ESPEasySerialPort port,
                int               receivePin,
                int               transmitPin,
                bool              inverse_logic = false,
                unsigned int      buffSize      = 64,
                bool              forceSWserial = false);
  virtual ~ESPeasySerial();

  // If baud rate is set to 0, it will perform an auto-detect on the baudrate
  void begin(unsigned long baud,
             SerialConfig  config = SERIAL_8N1,
             SerialMode    mode   = SERIAL_FULL);
#endif // ifdef ESP8266


#ifdef ESP32

  // ESP32 has 3 HW serial ports.
  // Serial0: RX: 3  TX: 1
  // Serial1: RX: 9  TX: 10  Defaults will never work, share pins with flash
  // Serial2: RX: 16 TX: 17
  // Pins set in the constructor will be used as override when not given when calling begin()
  // @param  inverse_logic can be used to set the logic in the constructor which will then be used in the call to begin.
  //         This makes the call to the constructor more in line with the constructor of SoftwareSerial.
  // buffsize is for compatibility reasons. ESP32 cannot set the buffer size.
  ESPeasySerial(ESPEasySerialPort port,
                int               receivePin,
                int               transmitPin,
                bool              inverse_logic = false,
                unsigned int      buffSize      = 64);
  virtual ~ESPeasySerial();

  // If baud rate is set to 0, it will perform an auto-detect on the baudrate
  void begin(unsigned long baud,
             uint32_t      config     = SERIAL_8N1,
             int8_t        rxPin      = -1,
             int8_t        txPin      = -1,
             bool          invert     = false,
             unsigned long timeout_ms = 20000UL);
#endif // ifdef ESP32

  void   end();
  int    peek(void);
  size_t write(uint8_t val) override;
  int    read(void) override;
  int    available(void) override;
  void   flush(void) override;

#if defined(ESP8266)
  bool   overflow();       // SoftwareSerial ESP8266
  bool   hasOverrun(void); // HardwareSerial ESP8266
#endif // if defined(ESP8266)

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
  size_t write(const uint8_t *buffer,
               size_t         size);
  size_t write(const char *buffer);
  int    baudRate(void);

#if defined(ESP8266)
  void   swap() {
    swap(_transmitPin);
  }

  void          swap(uint8_t tx_pin);
  size_t        readBytes(char  *buffer,
                          size_t size) override;
  size_t        readBytes(uint8_t *buffer,
                          size_t   size) override;

  void          setDebugOutput(bool);
#endif // if defined(ESP8266)

  bool          isTxEnabled(void);
  bool          isRxEnabled(void);

#if defined(ESP8266)
  bool          hasRxError(void);

  void          startDetectBaudrate();
  unsigned long testBaudrate();
  unsigned long detectBaudrate(time_t timeoutMillis);

  // SoftwareSerial specific
  void          setTransmitEnablePin(uint8_t transmitEnablePin);

  // AVR compatibility methods
  bool          isListening();
  bool          stopListening();

  bool          serial0_swap_active() const {
    return _serial0_swap_active;
  }

#endif // if defined(ESP8266)
  bool   listen();

  String getLogString() const;


  using Print::write;

  int getRxPin() const {
    return _receivePin;
  }

  int getTxPin() const {
    return _transmitPin;
  }

  unsigned long getBaudRate() const {
    return _baud;
  }

  bool useGPIOpins() const {
    return _serialtype != ESPEasySerialPort::sc16is752;
  }

private:

  const HardwareSerial* getHW() const;
  HardwareSerial*       getHW();

  bool                  isValid() const;

#ifdef ESP8266
  bool                  doHWbegin(unsigned long baud,
                                  SerialConfig  config,
                                  SerialMode    mode);
#endif // ifdef ESP8266

  bool isI2Cserial() const {
    return _serialtype == ESPEasySerialPort::sc16is752;
  }

#ifndef DISABLE_SC16IS752_Serial
  ESPEasySC16IS752_Serial *_i2cserial = nullptr;
#endif // ifndef DISABLE_SC16IS752_Serial

#if !defined(DISABLE_SOFTWARE_SERIAL) && defined(ESP8266)
  bool isSWserial() const {
    return _serialtype == ESPEasySerialPort::software;
  }

  ESPeasySoftwareSerial *_swserial = nullptr;
#else // if !defined(DISABLE_SOFTWARE_SERIAL) && defined(ESP8266)
  bool isSWserial() const {
    return false;
  }

#endif // if !defined(DISABLE_SOFTWARE_SERIAL) && defined(ESP8266)
#ifdef ESP8266
  static bool _serial0_swap_active;
#endif // ESP8266

  ESPEasySerialPort _serialtype = ESPEasySerialPort::MAX_SERIAL_TYPE;
  int _receivePin;
  int _transmitPin;
  unsigned long _baud = 0;
  bool _inverse_logic = false;
};

#endif // ifndef ESPeasySerial_h
