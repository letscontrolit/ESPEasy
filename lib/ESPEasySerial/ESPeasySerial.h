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

#include "ESPEasySerialConfig.h"
#include "ESPEasySerialPort.h"
#include "ESPEasySerialType.h"

#include "Port_ESPEasySerial_base.h"

#include <Stream.h>


// ESP82xx has 2 HW serial ports and option for several software serial ports.
// Serial0:         RX: 3  TX: 1
// Serial0 swapped  RX: 13 TX: 15
// Serial1:         RX: -- TX: 2   (TX only)
// SC16IS752:       Rx: I2C addr  TX: channel (A = 0, B = 1)

class ESPeasySerial : public Stream {
public:

  static Port_ESPEasySerial_base* ESPEasySerial_Port_factory(const ESPEasySerialConfig& config);

  
  // Ideal buffer size is a trade-off between bootspeed 
  // and not missing data when the ESP is busy processing stuff.
  // Mainly HWCDC and USBCDC may appear blocking if the USB device is detected 
  // by your OS but no serial port has been opened.
  // Flushing the buffers may then take buffSize * timeout to flush, which is blocking.
  ESPeasySerial(ESPEasySerialPort port,
                int               receivePin,
                int               transmitPin,
                bool              inverse_logic = false,
                unsigned int      buffSize      = SOC_UART_FIFO_LEN,
                bool              forceSWserial = false);

  // Config may be altered by running validate()
  // Thus make a deepcopy and not const reference
  ESPeasySerial(ESPEasySerialConfig config);

  virtual ~ESPeasySerial();

  // Same parameters as the constructor, to allow to reconfigure the ESPEasySerial object to be another type of port.
  void resetConfig(ESPEasySerialPort port,
                   int               receivePin,
                   int               transmitPin,
                   bool              inverse_logic = false,
                   unsigned int      buffSize      = SOC_UART_FIFO_LEN,
                   bool              forceSWserial = false);

  // If baud rate is set to 0, it will perform an auto-detect on the baudrate
  void begin(unsigned long baud);
#ifdef ESP8266
  void begin(unsigned long baud,
             SerialConfig  config,
             SerialMode    mode = SERIAL_FULL);
#endif // ifdef ESP8266
#ifdef ESP32
  void begin(unsigned long baud,
             uint32_t      config);
#endif // ifdef ESP32

  void   end();
  int    peek(void);
  size_t write(uint8_t val) override;
  int    read(void) override;
  int    available(void) override;
  int    availableForWrite(void);
  void   flush(void) override;


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
  int    getBaudRate() const;

  operator bool() const;

  bool   connected() const;


  void   setDebugOutput(bool);

  // Set buffer size, should be called before calling begin()
  size_t setRxBufferSize(size_t new_size);
  size_t setTxBufferSize(size_t new_size);


  bool   isTxEnabled(void);
  bool   isRxEnabled(void);

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
    if (_serialPort != nullptr) {
      return _serialPort->getSerialConfig();
    }
    ESPEasySerialConfig res;

    return res;
  }

  String getPortDescription() const {
    if (_serialPort != nullptr) {
      return _serialPort->getPortDescription();
    }
    return String();
  }

  // Try enabling RTS pin and set to UART_RS485_HALF_DUPLEX
  // RTS pin can then be connected to ~RE/DE pin of MAX485
  // @retval True when supported and successful.
  bool setRS485Mode(int8_t rtsPin, bool enableCollisionDetection = false);

private:

  bool isValid() const;

  Port_ESPEasySerial_base *_serialPort = nullptr;
};

#endif // ifndef ESPeasySerial_h
