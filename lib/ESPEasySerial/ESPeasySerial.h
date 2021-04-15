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
#include <ESPEasySC16IS752_Serial.h>
#endif

#if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(ESP32)
  # ifndef DISABLE_SOFTWARE_SERIAL
    #  define DISABLE_SOFTWARE_SERIAL
  # endif // ifndef DISABLE_SOFTWARE_SERIAL
#endif // if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(ESP32)

#ifndef DISABLE_SOFTWARE_SERIAL
# include <ESPEasySoftwareSerial.h>
#endif // ifndef DISABLE_SOFTWARE_SERIAL

#ifdef ESP32
  # define NR_ESPEASY_SERIAL_TYPES 4 // Serial 0, 1, 2, sc16is752
#endif // ifdef ESP32
#ifdef ESP8266
  #if !defined(DISABLE_SOFTWARE_SERIAL)
    # define NR_ESPEASY_SERIAL_TYPES 5 // Serial 0, 1, 0_swap, software, sc16is752
  #else // if !defined(DISABLE_SOFTWARE_SERIAL)
    # define NR_ESPEASY_SERIAL_TYPES 3 // Serial 0, 1, 0_swap
  #endif // if !defined(DISABLE_SOFTWARE_SERIAL)
#endif

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


#ifndef ESP32_SER0_TX
  # define ESP32_SER0_TX 1
#endif // ifndef ESP32_SER0_TX
#ifndef ESP32_SER0_RX
  # define ESP32_SER0_RX 3
#endif // ifndef ESP32_SER0_RX

#ifndef ESP32_SER1_TX
  # define ESP32_SER1_TX 15
#endif // ifndef ESP32_SER1_TX
#ifndef ESP32_SER1_RX
  # define ESP32_SER1_RX 13
#endif // ifndef ESP32_SER1_RX

#ifndef ESP32_SER2_TX
  # define ESP32_SER2_TX 17
#endif // ifndef ESP32_SER2_TX
#ifndef ESP32_SER2_RX
  # define ESP32_SER2_RX 16
#endif // ifndef ESP32_SER2_RX


// Keep value assigned as it is used in scripts and stored in the Settings.TaskDevicePort
enum class ESPEasySerialPort {
  not_set = 0,
  sc16is752 = 1,
  serial0 = 2,
  serial0_swap = 3,
  serial1 = 4,
  serial2 = 5,
  software = 6,

  MAX_SERIAL_TYPE
};


struct ESPeasySerialType {


  static bool getSerialTypePins(ESPEasySerialPort serType, int& rxPin, int& txPin) {
    rxPin = -1;
    txPin = -1;

    switch (serType) {
#ifdef ESP32
      case ESPEasySerialPort::serial0:  rxPin = ESP32_SER0_RX; txPin = ESP32_SER0_TX; return true;
      case ESPEasySerialPort::serial1:  rxPin = ESP32_SER1_RX; txPin = ESP32_SER1_TX; return true;
      case ESPEasySerialPort::serial2:  rxPin = ESP32_SER2_RX; txPin = ESP32_SER2_TX; return true;
#endif // ifdef ESP32
#ifdef ESP8266
      case ESPEasySerialPort::serial0:       rxPin = 3; txPin = 1; return true;
      case ESPEasySerialPort::serial0_swap:  rxPin = 13; txPin = 15; return true;
      case ESPEasySerialPort::serial1:       rxPin = -1; txPin = 2; return true;
    # ifndef DISABLE_SOFTWARE_SERIAL
      case ESPEasySerialPort::software:      rxPin = 14; txPin = 12; return true;
    # endif // DISABLE_SOFTWARE_SERIAL
#endif // ifdef ESP8266
      case ESPEasySerialPort::sc16is752:     rxPin = -1; txPin = -1; return true;

      default:
        break;
    }
    return false;
  }

#ifdef ESP32
  static ESPEasySerialPort getSerialType(ESPEasySerialPort typeHint, int receivePin, int transmitPin) {
    if (typeHint != ESPEasySerialPort::not_set) {
      return typeHint;
    }
    if ((receivePin == ESP32_SER0_RX) && (transmitPin == ESP32_SER0_TX)) {
      return ESPEasySerialPort::serial0; // UART0
    }

    // Serial1 on ESP32 uses default pins connected to flash
    // So must make sure to set them to other pins.
    if ((receivePin == ESP32_SER1_RX) && (transmitPin == ESP32_SER1_TX)) {
      return ESPEasySerialPort::serial1; // UART1
    }

    if ((receivePin == ESP32_SER2_RX) && (transmitPin == ESP32_SER2_TX)) {
      return ESPEasySerialPort::serial2; // UART2
    }

    if ((receivePin >= 0x48) && (receivePin <= 0x57)) {
      return ESPEasySerialPort::sc16is752; // I2C address range of SC16IS752
    }

    return ESPEasySerialPort::MAX_SERIAL_TYPE;
  }

#endif // ESP32


#ifdef ESP8266
  static ESPEasySerialPort getSerialType(ESPEasySerialPort typeHint, int receivePin, int transmitPin) {
    if (typeHint != ESPEasySerialPort::not_set) {
      return typeHint;
    }
    if ((receivePin == 3) && (transmitPin == 1)) {
      return ESPEasySerialPort::serial0; // UART0
    }

    // ESP8266
    if ((receivePin == 13) && (transmitPin == 15)) {
      return ESPEasySerialPort::serial0_swap; // UART0 remapped using Serial.swap()
    }

    if ((receivePin == -1) && (transmitPin == 2)) {
      // Serial1 uses UART1, TX pin is GPIO2.
      // UART1 can not be used to receive data because normally
      // it's RX pin is occupied for flash chip connection.
      return ESPEasySerialPort::serial1;
    }

    if ((receivePin >= 0x48) && (receivePin <= 0x57)) {
      return ESPEasySerialPort::sc16is752; // I2C address range of SC16IS752
    }

    if ((receivePin == -1) && (transmitPin == -1)) {
      // No pins set, so no serial type
      return ESPEasySerialPort::MAX_SERIAL_TYPE;
    }
    return ESPEasySerialPort::software;
  }

#endif // ESP8266
};


class ESPeasySerial : public Stream {
public:

#ifdef ESP8266

  // ESP82xx has 2 HW serial ports and option for several software serial ports.
  // Serial0:         RX: 3  TX: 1
  // Serial0 swapped  RX: 13 TX: 15
  // Serial1:         RX: -- TX: 2   (TX only)
  // SC16IS752:       Rx: I2C addr  TX: channel (A = 0, B = 1)
  ESPeasySerial(ESPEasySerialPort         port, 
                int          receivePin,
                int          transmitPin,
                bool         inverse_logic = false,
                unsigned int buffSize      = 64,
                bool         forceSWserial = false);
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
                int  receivePin,
                int  transmitPin,
                bool inverse_logic = false,
                unsigned int buffSize      = 64);
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
  bool          isTxEnabled(void);
  bool          isRxEnabled(void);
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
  HardwareSerial      * getHW();

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
#endif

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
