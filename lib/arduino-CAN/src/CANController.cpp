// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "CANController.h"

CANControllerClass::CANControllerClass() :
  _onReceive(NULL),

  _packetBegun(false),
  _txId(-1),
  _txExtended(-1),
  _txRtr(false),
  _txDlc(0),
  _txLength(0),

  _rxId(-1),
  _rxExtended(false),
  _rxRtr(false),
  _rxDlc(0),
  _rxLength(0),
  _rxIndex(0)
{
  // overide Stream timeout value
  setTimeout(0);
}

CANControllerClass::~CANControllerClass()
{
}

int CANControllerClass::begin(long /*baudRate*/)
{
  _packetBegun = false;
  _txId = -1;
  _txRtr =false;
  _txDlc = 0;
  _txLength = 0;

  _rxId = -1;
  _rxRtr = false;
  _rxDlc = 0;
  _rxLength = 0;
  _rxIndex = 0;

  return 1;
}

void CANControllerClass::end()
{
}

int CANControllerClass::beginPacket(int id, int dlc, bool rtr)
{
  if (id < 0 || id > 0x7FF) {
    return 0;
  }

  if (dlc > 8) {
    return 0;
  }

  _packetBegun = true;
  _txId = id;
  _txExtended = false;
  _txRtr = rtr;
  _txDlc = dlc;
  _txLength = 0;

  memset(_txData, 0x00, sizeof(_txData));

  return 1;
}

int CANControllerClass::beginExtendedPacket(long id, int dlc, bool rtr)
{
  if (id < 0 || id > 0x1FFFFFFF) {
    return 0;
  }

  if (dlc > 8) {
    return 0;
  }

  _packetBegun = true;
  _txId = id;
  _txExtended = true;
  _txRtr = rtr;
  _txDlc = dlc;
  _txLength = 0;

  memset(_txData, 0x00, sizeof(_txData));

  return 1;
}

int CANControllerClass::endPacket(unsigned long timeoutMs)
{
  if (!_packetBegun) {
    return 0;
  }
  _packetBegun = false;

  if (_txDlc >= 0) {
    _txLength = _txDlc;
  }

  return 1;
}

int CANControllerClass::parsePacket()
{
  return 0;
}

long CANControllerClass::packetId()
{
  return _rxId;
}

bool CANControllerClass::packetExtended()
{
  return _rxExtended;
}

bool CANControllerClass::packetRtr()
{
  return _rxRtr;
}

int CANControllerClass::packetDlc()
{
  return _rxDlc;
}

size_t CANControllerClass::write(uint8_t byte)
{
  return write(&byte, sizeof(byte));
}

size_t CANControllerClass::write(const uint8_t *buffer, size_t size)
{
  if (!_packetBegun) {
    return 0;
  }

  if (size > (sizeof(_txData) - _txLength)) {
    size = sizeof(_txData) - _txLength;
  }

  memcpy(&_txData[_txLength], buffer, size);
  _txLength += size;

  return size;
}

int CANControllerClass::available()
{
  return (_rxLength - _rxIndex);
}

int CANControllerClass::read()
{
  if (!available()) {
    return -1;
  }

  return _rxData[_rxIndex++];
}

int CANControllerClass::peek()
{
  if (!available()) {
    return -1;
  }

  return _rxData[_rxIndex];
}

void CANControllerClass::flush()
{
}

void CANControllerClass::onReceive(void(*callback)(int))
{
  _onReceive = callback;
}

int CANControllerClass::filter(int /*id*/, int /*mask*/)
{
  return 0;
}

int CANControllerClass::filterExtended(long /*id*/, long /*mask*/)
{
  return 0;
}

int CANControllerClass::observe()
{
  return 0;
}

int CANControllerClass::loopback()
{
  return 0;
}

int CANControllerClass::sleep()
{
  return 0;
}

int CANControllerClass::wakeup()
{
  return 0;
}
