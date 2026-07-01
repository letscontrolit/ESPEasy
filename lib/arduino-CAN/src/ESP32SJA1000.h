// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef ARDUINO_ARCH_ESP32

#ifndef ESP32_SJA1000_H
#define ESP32_SJA1000_H

#include "CANController.h"

#define DEFAULT_CAN_RX_PIN GPIO_NUM_4
#define DEFAULT_CAN_TX_PIN GPIO_NUM_5

class ESP32SJA1000Class : public CANControllerClass {

public:
  ESP32SJA1000Class();
  virtual ~ESP32SJA1000Class();

  virtual int begin(long baudRate);
  virtual void end();

  virtual int endPacket(unsigned long timeoutMs = 0);

  virtual int parsePacket();

  virtual void onReceive(void(*callback)(int));

  using CANControllerClass::filter;
  virtual int filter(int id, int mask);
  using CANControllerClass::filterExtended;
  virtual int filterExtended(long id, long mask);

  virtual int observe();
  virtual int loopback();
  virtual int sleep();
  virtual int wakeup();

  void setPins(int rx, int tx);

  void dumpRegisters(Stream& out);

private:
  void reset();

  void handleInterrupt();

  uint8_t readRegister(uint8_t address);
  void modifyRegister(uint8_t address, uint8_t mask, uint8_t value);
  void writeRegister(uint8_t address, uint8_t value);

  static void onInterrupt(void* arg);

private:
  gpio_num_t _rxPin;
  gpio_num_t _txPin;
  bool _loopback;
  intr_handle_t _intrHandle;
};

extern ESP32SJA1000Class CAN;

#endif

#endif
