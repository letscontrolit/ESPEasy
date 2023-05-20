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


#include "ESPEasySerial_common_defines.h"

#include "ESPEasySerialPort.h"
#include "ESPEasySerialType.h"

#include "ESPEasySerial_Port_base.h"

#include <Stream.h>


class ESPeasySerial : public Stream {
public:

  static ESPEasySerial_Port_base* ESPEasySerial_Port_factory(const ESPEasySerialConfig &config);

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

  // Same parameters as the constructor, to allow to reconfigure the ESPEasySerial object to be another type of port.
  void resetConfig(ESPEasySerialPort         port, 
                int          receivePin,
                int          transmitPin,
                bool         inverse_logic = false,
                unsigned int buffSize      = 64,
                bool         forceSWserial = false);

  // If baud rate is set to 0, it will perform an auto-detect on the baudrate
  void begin(unsigned long baud);
#ifdef ESP8266
  void begin(unsigned long baud,
             SerialConfig  config,
             SerialMode    mode   = SERIAL_FULL);
#endif
#ifdef ESP32
void begin(unsigned long baud, uint32_t config);
#endif

  void   end();
  int    peek(void);
  size_t write(uint8_t val) override;
  int    read(void) override;
  int    available(void) override;
  int    availableForWrite(void);
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
  int getBaudRate() const;

#if defined(ESP8266)
  size_t        readBytes(char  *buffer,
                          size_t size) override;
  size_t        readBytes(uint8_t *buffer,
                          size_t   size) override;
#endif

  void          setDebugOutput(bool);

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

#endif // if defined(ESP8266)
  bool   listen();

  String getLogString() const;


  using Print::write;

  int getRxPin() const {
    return getSerialConfig().receivePin;
  }

  int getTxPin() const {
    return getSerialConfig().transmitPin;
  }

  bool usesGPIOpins() const {
    return useGPIOpins(getSerialConfig().port);
  }

  ESPEasySerialPort getSerialPortType() const {
    return getSerialConfig().port;
  }

  ESPEasySerialConfig getSerialConfig() const {
    if (_serialPort != nullptr) 
      return _serialPort->getSerialConfig();
    ESPEasySerialConfig res;
    return res;
  }

private:

  bool                  isValid() const;

  ESPEasySerial_Port_base* _serialPort = nullptr;

};

#endif // ifndef ESPeasySerial_h
