// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef ARDUINO_ARCH_ESP32

#include "esp_intr_alloc.h"
#include "soc/dport_reg.h"
#include "driver/gpio.h"

#include "esp32-hal-matrix.h"
#include "soc/gpio_pins.h"

#include "ESP32SJA1000.h"

#define REG_BASE                   0x3ff6b000

#define REG_MOD                    0x00
#define REG_CMR                    0x01
#define REG_SR                     0x02
#define REG_IR                     0x03
#define REG_IER                    0x04

#define REG_BTR0                   0x06
#define REG_BTR1                   0x07
#define REG_OCR                    0x08

#define REG_ALC                    0x0b
#define REG_ECC                    0x0c
#define REG_EWLR                   0x0d
#define REG_RXERR                  0x0e
#define REG_TXERR                  0x0f
#define REG_SFF                    0x10
#define REG_EFF                    0x10
#define REG_ACRn(n)                (0x10 + n)
#define REG_AMRn(n)                (0x14 + n)

#define REG_CDR                    0x1F


ESP32SJA1000Class::ESP32SJA1000Class() :
  CANControllerClass(),
  _rxPin(DEFAULT_CAN_RX_PIN),
  _txPin(DEFAULT_CAN_TX_PIN),
  _loopback(false),
  _intrHandle(NULL)
{
}

ESP32SJA1000Class::~ESP32SJA1000Class()
{
}

int ESP32SJA1000Class::begin(long baudRate)
{
  CANControllerClass::begin(baudRate);

  _loopback = false;

  DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_CAN_RST);
  DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_CAN_CLK_EN);

  // RX pin
  gpio_set_direction(_rxPin, GPIO_MODE_INPUT);
  pinMatrixInAttach(_rxPin, CAN_RX_IDX, 0);
  #if ESP_IDF_VERSION_MAJOR <5
  gpio_pad_select_gpio(_rxPin);
  #else
  esp_rom_gpio_pad_select_gpio(_rxPin);
  #endif

  // TX pin
  gpio_set_direction(_txPin, GPIO_MODE_OUTPUT);
  pinMatrixOutAttach(_txPin, CAN_TX_IDX, 0, 0);
  #if ESP_IDF_VERSION_MAJOR <5
  gpio_pad_select_gpio(_txPin);
  #else
  esp_rom_gpio_pad_select_gpio(_txPin);
  #endif
  modifyRegister(REG_CDR, 0x80, 0x80); // pelican mode
  modifyRegister(REG_BTR0, 0xc0, 0x40); // SJW = 1
  modifyRegister(REG_BTR1, 0x70, 0x10); // TSEG2 = 1

  switch (baudRate) {
    case (long)1000E3:
      modifyRegister(REG_BTR1, 0x0f, 0x04);
      modifyRegister(REG_BTR0, 0x3f, 4);
      break;

    case (long)500E3:
      modifyRegister(REG_BTR1, 0x0f, 0x0c);
      modifyRegister(REG_BTR0, 0x3f, 4);
      break;

    case (long)250E3:
      modifyRegister(REG_BTR1, 0x0f, 0x0c);
      modifyRegister(REG_BTR0, 0x3f, 9);
      break;

    case (long)200E3:
      modifyRegister(REG_BTR1, 0x0f, 0x0c);
      modifyRegister(REG_BTR0, 0x3f, 12);
      break;

    case (long)125E3:
      modifyRegister(REG_BTR1, 0x0f, 0x0c);
      modifyRegister(REG_BTR0, 0x3f, 19);
      break;

    case (long)100E3:
      modifyRegister(REG_BTR1, 0x0f, 0x0c);
      modifyRegister(REG_BTR0, 0x3f, 24);
      break;

    case (long)80E3:
      modifyRegister(REG_BTR1, 0x0f, 0x0c);
      modifyRegister(REG_BTR0, 0x3f, 30);
      break;

    case (long)50E3:
      modifyRegister(REG_BTR1, 0x0f, 0x0c);
      modifyRegister(REG_BTR0, 0x3f, 49);
      break;

/*
   Due to limitations in ESP32 hardware and/or RTOS software, baudrate can't be lower than 50kbps.
   See https://esp32.com/viewtopic.php?t=2142
*/
    default:
      return 0;
      break;
  }

  modifyRegister(REG_BTR1, 0x80, 0x80); // SAM = 1
  writeRegister(REG_IER, 0xff); // enable all interrupts

  // set filter to allow anything
  writeRegister(REG_ACRn(0), 0x00);
  writeRegister(REG_ACRn(1), 0x00);
  writeRegister(REG_ACRn(2), 0x00);
  writeRegister(REG_ACRn(3), 0x00);
  writeRegister(REG_AMRn(0), 0xff);
  writeRegister(REG_AMRn(1), 0xff);
  writeRegister(REG_AMRn(2), 0xff);
  writeRegister(REG_AMRn(3), 0xff);

  modifyRegister(REG_OCR, 0x03, 0x02); // normal output mode
  // reset error counters
  writeRegister(REG_TXERR, 0x00);
  writeRegister(REG_RXERR, 0x00);

  // clear errors and interrupts
  readRegister(REG_ECC);
  readRegister(REG_IR);

  // normal mode
  modifyRegister(REG_MOD, 0x08, 0x08);
  modifyRegister(REG_MOD, 0x17, 0x00);

  return 1;
}

void ESP32SJA1000Class::end()
{
  if (_intrHandle) {
    esp_intr_free(_intrHandle);
    _intrHandle = NULL;
  }

  DPORT_SET_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_CAN_RST);
  DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_CAN_CLK_EN);

  CANControllerClass::end();
}

int ESP32SJA1000Class::endPacket(unsigned long timeoutMs)
{
  unsigned long maxTime = millis() + timeoutMs;

  if (!CANControllerClass::endPacket(timeoutMs)) {
    return SEND_BEGIN;
  }

  // wait for TX buffer to free
  while ((readRegister(REG_SR) & 0x04) != 0x04) {
    if (timeoutMs && (millis() > maxTime)) {
      return SEND_TIMEOUT;
    }
    yield();
  }

  int dataReg;

  if (_txExtended) {
    writeRegister(REG_EFF, 0x80 | (_txRtr ? 0x40 : 0x00) | (0x0f & _txLength));
    writeRegister(REG_EFF + 1, _txId >> 21);
    writeRegister(REG_EFF + 2, _txId >> 13);
    writeRegister(REG_EFF + 3, _txId >> 5);
    writeRegister(REG_EFF + 4, _txId << 3);

    dataReg = REG_EFF + 5;
  } else {
    writeRegister(REG_SFF, (_txRtr ? 0x40 : 0x00) | (0x0f & _txLength));
    writeRegister(REG_SFF + 1, _txId >> 3);
    writeRegister(REG_SFF + 2, _txId << 5);

    dataReg = REG_SFF + 3;
  }

  for (int i = 0; i < _txLength; i++) {
    writeRegister(dataReg + i, _txData[i]);
  }

  if ( _loopback) {
    // self reception request
    modifyRegister(REG_CMR, 0x1f, 0x10);
  } else {
    // transmit request
    modifyRegister(REG_CMR, 0x1f, 0x01);
  }

  // wait for TX complete
  while ((readRegister(REG_SR) & 0x08) != 0x08) {
    if (readRegister(REG_ECC) == 0xd9) {
      modifyRegister(REG_CMR, 0x1f, 0x02); // error, abort
      return SEND_ACK;
    } else if (timeoutMs && (millis() > maxTime)) {
      return SEND_TIMEOUT;
    }
    yield();
  }

  return SEND_OK;
}

int ESP32SJA1000Class::parsePacket()
{
  if ((readRegister(REG_SR) & 0x01) != 0x01) {
    // no packet
    return 0;
  }

  _rxExtended = (readRegister(REG_SFF) & 0x80) ? true : false;
  _rxRtr = (readRegister(REG_SFF) & 0x40) ? true : false;
  _rxDlc = (readRegister(REG_SFF) & 0x0f);
  _rxIndex = 0;

  int dataReg;

  if (_rxExtended) {
    _rxId = (readRegister(REG_EFF + 1) << 21) |
            (readRegister(REG_EFF + 2) << 13) |
            (readRegister(REG_EFF + 3) << 5) |
            (readRegister(REG_EFF + 4) >> 3);

    dataReg = REG_EFF + 5;
  } else {
    _rxId = (readRegister(REG_SFF + 1) << 3) | ((readRegister(REG_SFF + 2) >> 5) & 0x07);

    dataReg = REG_SFF + 3;
  }

  if (_rxRtr) {
    _rxLength = 0;
  } else {
    _rxLength = _rxDlc;

    for (int i = 0; i < _rxLength; i++) {
      _rxData[i] = readRegister(dataReg + i);
    }
  }

  // release RX buffer
  modifyRegister(REG_CMR, 0x04, 0x04);

  return _rxDlc;
}

void ESP32SJA1000Class::onReceive(void(*callback)(int))
{
  CANControllerClass::onReceive(callback);

  if (_intrHandle) {
    esp_intr_free(_intrHandle);
    _intrHandle = NULL;
  }

  if (callback) {
    esp_intr_alloc(ETS_CAN_INTR_SOURCE, 0, ESP32SJA1000Class::onInterrupt, this, &_intrHandle);
  }
}

int ESP32SJA1000Class::filter(int id, int mask)
{
  id &= 0x7ff;
  mask = ~(mask & 0x7ff);

  modifyRegister(REG_MOD, 0x17, 0x01); // reset

  writeRegister(REG_ACRn(0), id >> 3);
  writeRegister(REG_ACRn(1), id << 5);
  writeRegister(REG_ACRn(2), 0x00);
  writeRegister(REG_ACRn(3), 0x00);

  writeRegister(REG_AMRn(0), mask >> 3);
  writeRegister(REG_AMRn(1), (mask << 5) | 0x1f);
  writeRegister(REG_AMRn(2), 0xff);
  writeRegister(REG_AMRn(3), 0xff);

  modifyRegister(REG_MOD, 0x17, 0x00); // normal

  return 1;
}

int ESP32SJA1000Class::filterExtended(long id, long mask)
{
  id &= 0x1FFFFFFF;
  mask &= ~(mask & 0x1FFFFFFF);

  modifyRegister(REG_MOD, 0x17, 0x01); // reset

  writeRegister(REG_ACRn(0), id >> 21);
  writeRegister(REG_ACRn(1), id >> 13);
  writeRegister(REG_ACRn(2), id >> 5);
  writeRegister(REG_ACRn(3), id << 3);

  writeRegister(REG_AMRn(0), mask >> 21);
  writeRegister(REG_AMRn(1), mask >> 13);
  writeRegister(REG_AMRn(2), mask >> 5);
  writeRegister(REG_AMRn(3), (mask << 3) | 0x1f);

  modifyRegister(REG_MOD, 0x17, 0x00); // normal

  return 1;
}

int ESP32SJA1000Class::observe()
{
  modifyRegister(REG_MOD, 0x17, 0x01); // reset
  modifyRegister(REG_MOD, 0x17, 0x02); // observe

  return 1;
}

int ESP32SJA1000Class::loopback()
{
  _loopback = true;

  modifyRegister(REG_MOD, 0x17, 0x01); // reset
  modifyRegister(REG_MOD, 0x17, 0x04); // self test mode

  return 1;
}

int ESP32SJA1000Class::sleep()
{
  modifyRegister(REG_MOD, 0x1f, 0x10);

  return 1;
}

int ESP32SJA1000Class::wakeup()
{
  modifyRegister(REG_MOD, 0x1f, 0x00);

  return 1;
}

void ESP32SJA1000Class::setPins(int rx, int tx)
{
  _rxPin = (gpio_num_t)rx;
  _txPin = (gpio_num_t)tx;
}

void ESP32SJA1000Class::dumpRegisters(Stream& out)
{
  for (int i = 0; i < 32; i++) {
    byte b = readRegister(i);

    out.print("0x");
    if (i < 16) {
      out.print('0');
    }
    out.print(i, HEX);
    out.print(": 0x");
    if (b < 16) {
      out.print('0');
    }
    out.println(b, HEX);
  }
}

void ESP32SJA1000Class::handleInterrupt()
{
  uint8_t ir = readRegister(REG_IR);

  if (ir & 0x01) {
    // received packet, parse and call callback
    parsePacket();

    _onReceive(available());
  }
}

uint8_t ESP32SJA1000Class::readRegister(uint8_t address)
{
  volatile uint32_t* reg = (volatile uint32_t*)(REG_BASE + address * 4);

  return *reg;
}

void ESP32SJA1000Class::modifyRegister(uint8_t address, uint8_t mask, uint8_t value)
{
  volatile uint32_t* reg = (volatile uint32_t*)(REG_BASE + address * 4);

  *reg = (*reg & ~mask) | value;
}

void ESP32SJA1000Class::writeRegister(uint8_t address, uint8_t value)
{
  volatile uint32_t* reg = (volatile uint32_t*)(REG_BASE + address * 4);

  *reg = value;
}

void ESP32SJA1000Class::onInterrupt(void* arg)
{
  ((ESP32SJA1000Class*)arg)->handleInterrupt();
}

ESP32SJA1000Class CAN;

#endif
