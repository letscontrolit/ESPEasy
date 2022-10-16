/*
   Author: Klusjesman, supersjimmie, modified and reworked by arjenhiemstra
 */

// #define DEBUG 0

// #define BYTE_TO_BINARY_PATTERN "%c,%c,%c,%c,%c,%c,%c,%c,"

/*#define BYTE_TO_BINARY(byte)  \
   (byte & 0x80 ? '1' : '0'), \
   (byte & 0x40 ? '1' : '0'), \
   (byte & 0x20 ? '1' : '0'), \
   (byte & 0x10 ? '1' : '0'), \
   (byte & 0x08 ? '1' : '0'), \
   (byte & 0x04 ? '1' : '0'), \
   (byte & 0x02 ? '1' : '0'), \
   (byte & 0x01 ? '1' : '0')*/

#include "IthoCC1101.h"
#include <string.h>
#include <Arduino.h>
#include <SPI.h>

// #define CRC_FILTER

////original sync byte pattern
// #define STARTBYTE 6 //relevant data starts 6 bytes after the sync pattern bytes 170/171
// #define SYNC1 170
// #define SYNC0 171
// #define MDMCFG2 0x02 //16bit sync word / 16bit specific

////alternative sync byte pattern (filter much more non-itho messages out. Maybe too strict? Testing needed.
// #define STARTBYTE 0 //relevant data starts 0 bytes after the sync pattern bytes 179/42/171/42
// #define SYNC1 187 //byte11 = 179, byte13 = 171 with SYNC1 = 163, 179 and 171 differ only by 1 bit
// #define SYNC0 42
// #define MDMCFG2 0x03 //32bit sync word / 30bit specific

// alternative sync byte pattern
#define STARTBYTE 2  // relevant data starts 2 bytes after the sync pattern bytes 179/42
#define SYNC1 179
#define SYNC0 42
#define MDMCFG2 0x02 // 16bit sync word / 16bit specific

// default constructor
IthoCC1101::IthoCC1101(int8_t CSpin, int8_t MISOpin, uint8_t counter, uint8_t sendTries) : CC1101(CSpin, MISOpin)
{
  this->outIthoPacket.counter = counter;
  this->sendTries             = sendTries;

  this->outIthoPacket.deviceId[0] = 33;
  this->outIthoPacket.deviceId[1] = 66;
  this->outIthoPacket.deviceId[2] = 99;

  this->outIthoPacket.deviceType = 22;
} // IthoCC1101

// default destructor
IthoCC1101::~IthoCC1101()
{} // ~IthoCC1101

void IthoCC1101::initSendMessage(uint8_t len)
{
  // finishTransfer();
  writeCommand(CC1101_SIDLE);
  delayMicroseconds(1);
  writeRegister(CC1101_IOCFG0, 0x2E);
  delayMicroseconds(1);
  writeRegister(CC1101_IOCFG1, 0x2E);
  delayMicroseconds(1);
  writeCommand(CC1101_SIDLE);
  writeCommand(CC1101_SPWD);
  delayMicroseconds(2);

  /*
     Configuration reverse engineered from remote print. The commands below are used by IthoDaalderop.
     Base frequency    868.299866MHz
     Channel           0
     Channel spacing   199.951172kHz
     Carrier frequency 868.299866MHz
     Xtal frequency    26.000000MHz
     Data rate         38.3835kBaud
     Manchester        disabled
     Modulation        2-FSK
     Deviation         50.781250kHz
     TX power          ?
     PA ramping        enabled
     Whitening         disabled
   */
  writeCommand(CC1101_SRES);
  delayMicroseconds(1);
  writeRegister(CC1101_IOCFG0,   0x2E); // High impedance (3-state)
  writeRegister(CC1101_FREQ2,    0x21); // 00100001  878MHz-927.8MHz
  writeRegister(CC1101_FREQ1,    0x65); // 01100101
  writeRegister(CC1101_FREQ0,    0x6A); // 01101010
  writeRegister(CC1101_MDMCFG4,  0x5A); // difference compared to message1
  writeRegister(CC1101_MDMCFG3,  0x83); // difference compared to message1
  writeRegister(CC1101_MDMCFG2,  0x00); // 00000000  2-FSK, no manchester encoding/decoding, no preamble/sync
  writeRegister(CC1101_MDMCFG1,  0x22); // 00100010
  writeRegister(CC1101_MDMCFG0,  0xF8); // 11111000
  writeRegister(CC1101_CHANNR,   0x00); // 00000000
  writeRegister(CC1101_DEVIATN,  0x50); // difference compared to message1
  writeRegister(CC1101_FREND0,   0x17); // 00010111  use index 7 in PA table
  writeRegister(CC1101_MCSM0,    0x18); // 00011000  PO timeout Approx. 146microseconds - 171microseconds, Auto calibrate When going from
                                        // IDLE to RX or TX (or FSTXON)
  writeRegister(CC1101_FSCAL3,   0xA9); // 10101001
  writeRegister(CC1101_FSCAL2,   0x2A); // 00101010
  writeRegister(CC1101_FSCAL1,   0x00); // 00000000
  writeRegister(CC1101_FSCAL0,   0x11); // 00010001
  writeRegister(CC1101_FSTEST,   0x59); // 01011001  For test only. Do not write to this register.
  writeRegister(CC1101_TEST2,    0x81); // 10000001  For test only. Do not write to this register.
  writeRegister(CC1101_TEST1,    0x35); // 00110101  For test only. Do not write to this register.
  writeRegister(CC1101_TEST0,    0x0B); // 00001011  For test only. Do not write to this register.
  writeRegister(CC1101_PKTCTRL0, 0x12); // 00010010  Enable infinite length packets, CRC disabled, Turn data whitening off, Serial
                                        // Synchronous mode
  writeRegister(CC1101_ADDR,     0x00); // 00000000
  writeRegister(CC1101_PKTLEN,   0xFF); // 11111111  //Not used, no hardware packet handling

  // 0x6F,0x26,0x2E,0x8C,0x87,0xCD,0xC7,0xC0
  writeBurstRegister(CC1101_PATABLE | CC1101_WRITE_BURST, (uint8_t *)ithoPaTableSend, 8);

  // difference, message1 sends a STX here
  writeCommand(CC1101_SIDLE);
  writeCommand(CC1101_SIDLE);

  writeRegister(CC1101_MDMCFG4, 0x5A); // difference compared to message1
  writeRegister(CC1101_MDMCFG3, 0x83); // difference compared to message1
  writeRegister(CC1101_DEVIATN, 0x50); // difference compared to message1
  writeRegister(CC1101_IOCFG0,  0x2D); // GDO0_Z_EN_N. When this output is 0, GDO0 is configured as input (for serial TX data).
  writeRegister(CC1101_IOCFG1,  0x0B); // Serial Clock. Synchronous to the data in synchronous serial mode.

  writeCommand(CC1101_STX);
  writeCommand(CC1101_SIDLE);

  writeRegister(CC1101_MDMCFG4, 0x5A); // difference compared to message1
  writeRegister(CC1101_MDMCFG3, 0x83); // difference compared to message1
  writeRegister(CC1101_DEVIATN, 0x50); // difference compared to message1
  // writeRegister(CC1101_IOCFG0 ,0x2D);   //GDO0_Z_EN_N. When this output is 0, GDO0 is configured as input (for serial TX data).
  // writeRegister(CC1101_IOCFG1 ,0x0B);   //Serial Clock. Synchronous to the data in synchronous serial mode.

  // Itho is using serial mode for transmit. We want to use the TX FIFO with fixed packet length for simplicity.
  writeRegister(CC1101_IOCFG0,   0x2E);
  writeRegister(CC1101_IOCFG1,   0x2E);
  writeRegister(CC1101_PKTCTRL0, 0x00);
  writeRegister(CC1101_PKTCTRL1, 0x00);

  writeRegister(CC1101_PKTLEN,   len);
}

void IthoCC1101::finishTransfer()
{
  writeCommand(CC1101_SIDLE);
  delayMicroseconds(1);

  writeRegister(CC1101_IOCFG0, 0x2E);
  writeRegister(CC1101_IOCFG1, 0x2E);

  writeCommand(CC1101_SIDLE);
  writeCommand(CC1101_SPWD);
}

void IthoCC1101::initReceive()
{
  /*
     Configuration reverse engineered from RFT print.

     Base frequency    868.299866MHz
     Channel       0
     Channel spacing   199.951172kHz
     Carrier frequency 868.299866MHz
     Xtal frequency    26.000000MHz
     Data rate     38.3835kBaud
     RX filter BW    325.000000kHz
     Manchester      disabled
     Modulation      2-FSK
     Deviation     50.781250kHz
     TX power      0x6F,0x26,0x2E,0x7F,0x8A,0x84,0xCA,0xC4
     PA ramping      enabled
     Whitening     disabled
   */
  writeCommand(CC1101_SRES);

  writeRegister(CC1101_TEST0,  0x09);
  writeRegister(CC1101_FSCAL2, 0x00);

  // 0x6F,0x26,0x2E,0x7F,0x8A,0x84,0xCA,0xC4
  writeBurstRegister(CC1101_PATABLE | CC1101_WRITE_BURST, (uint8_t *)ithoPaTableReceive, 8);

  writeCommand(CC1101_SCAL);

  // wait for calibration to finish
  uint32_t maxWait = millis() + ITHO_MAX_WAIT; // Wait for max. x seconds

  while ((readRegisterWithSyncProblem(CC1101_MARCSTATE, CC1101_STATUS_REGISTER)) != CC1101_MARCSTATE_IDLE &&
         millis() < maxWait) {
    yield();
  }

  writeRegister(CC1101_FSCAL2,   0x00);
  writeRegister(CC1101_MCSM0,    0x18); // no auto calibrate
  writeRegister(CC1101_FREQ2,    0x21);
  writeRegister(CC1101_FREQ1,    0x65);
  writeRegister(CC1101_FREQ0,    0x6A);
  writeRegister(CC1101_IOCFG0,   0x2E); // High impedance (3-state)
  writeRegister(CC1101_IOCFG2,   0x06); // 0x06 Assert when sync word has been sent / received, and de-asserts at the end of the packet.
  writeRegister(CC1101_FSCTRL1,  0x06);
  writeRegister(CC1101_FSCTRL0,  0x00);
  writeRegister(CC1101_MDMCFG4,  0x5A);
  writeRegister(CC1101_MDMCFG3,  0x83);
  writeRegister(CC1101_MDMCFG2,  0x00); // Enable digital DC blocking filter before demodulator, 2-FSK, Disable Manchester
                                        // encoding/decoding, No preamble/sync
  writeRegister(CC1101_MDMCFG1,  0x22); // Disable FEC
  writeRegister(CC1101_MDMCFG0,  0xF8);
  writeRegister(CC1101_CHANNR,   0x00);
  writeRegister(CC1101_DEVIATN,  0x50);
  writeRegister(CC1101_FREND1,   0x56);
  writeRegister(CC1101_FREND0,   0x17);
  writeRegister(CC1101_MCSM0,    0x18); // no auto calibrate
  writeRegister(CC1101_FOCCFG,   0x16);
  writeRegister(CC1101_BSCFG,    0x6C);
  writeRegister(CC1101_AGCCTRL2, 0x43);
  writeRegister(CC1101_AGCCTRL1, 0x40);
  writeRegister(CC1101_AGCCTRL0, 0x91);
  writeRegister(CC1101_FSCAL3,   0xE9);
  writeRegister(CC1101_FSCAL2,   0x2A);
  writeRegister(CC1101_FSCAL1,   0x00);
  writeRegister(CC1101_FSCAL0,   0x11);
  writeRegister(CC1101_FSTEST,   0x59);
  writeRegister(CC1101_TEST2,    0x81);
  writeRegister(CC1101_TEST1,    0x35);
  writeRegister(CC1101_TEST0,    0x0B);
  writeRegister(CC1101_PKTCTRL1, 0x04); // No address check, Append two bytes with status RSSI/LQI/CRC OK,
  writeRegister(CC1101_PKTCTRL0, 0x32); // Infinite packet length mode, CRC disabled for TX and RX, No data whitening, Asynchronous serial
                                        // mode, Data in on GDO0 and data out on either of the GDOx pins
  writeRegister(CC1101_ADDR,     0x00);
  writeRegister(CC1101_PKTLEN,   0xFF);
  writeRegister(CC1101_TEST0,    0x09);

  writeCommand(CC1101_SCAL);

  // wait for calibration to finish
  maxWait = millis() + ITHO_MAX_WAIT; // Wait for max. x seconds

  while ((readRegisterWithSyncProblem(CC1101_MARCSTATE, CC1101_STATUS_REGISTER)) != CC1101_MARCSTATE_IDLE &&
         millis() < maxWait) {
    yield();
  }

  writeRegister(CC1101_MCSM0, 0x18); // no auto calibrate

  writeCommand(CC1101_SIDLE);
  writeCommand(CC1101_SIDLE);

  writeRegister(CC1101_MDMCFG2, 0x00); // Enable digital DC blocking filter before demodulator, 2-FSK, Disable Manchester encoding/decoding,
                                       // No preamble/sync
  writeRegister(CC1101_IOCFG0,  0x0D); // Serial Data Output. Used for asynchronous serial mode.

  writeCommand(CC1101_SRX);

  maxWait = millis() + ITHO_MAX_WAIT; // Wait for max. x seconds

  while ((readRegisterWithSyncProblem(CC1101_MARCSTATE, CC1101_STATUS_REGISTER)) != CC1101_MARCSTATE_RX &&
         millis() < maxWait) {
    yield();
  }

  initReceiveMessage();
}

void IthoCC1101::initReceiveMessage()
{
  uint8_t marcState;

  writeCommand(CC1101_SIDLE);           // idle

  // set datarate
  writeRegister(CC1101_MDMCFG4,  0x5A); // set kBaud
  writeRegister(CC1101_MDMCFG3,  0x83); // set kBaud
  writeRegister(CC1101_DEVIATN,  0x50);

  // set fifo mode with fixed packet length and sync bytes
  writeRegister(CC1101_PKTLEN,   63); // 63 bytes message (sync at beginning of message is removed by CC1101)

  // set fifo mode with fixed packet length and sync bytes
  writeRegister(CC1101_PKTCTRL0, 0x00);
  writeRegister(CC1101_SYNC1,    SYNC1);
  writeRegister(CC1101_SYNC0,    SYNC0);
  writeRegister(CC1101_MDMCFG2,  MDMCFG2);
  writeRegister(CC1101_PKTCTRL1, 0x00);

  writeCommand(CC1101_SRX);                    // switch to RX state

  // Check that the RX state has been entered
  uint32_t maxWait = millis() + ITHO_MAX_WAIT; // Wait for max. x seconds

  while (((marcState =
             readRegisterWithSyncProblem(CC1101_MARCSTATE, CC1101_STATUS_REGISTER)) & CC1101_BITS_MARCSTATE) != CC1101_MARCSTATE_RX &&
         millis() < maxWait)
  {
    if (marcState == CC1101_MARCSTATE_RXFIFO_OVERFLOW) { // RX_OVERFLOW
      writeCommand(CC1101_SFRX);                         // flush RX buffer
    }
  }
}

bool IthoCC1101::checkForNewPacket() {
  if (receiveData(&inMessage, 63) && parseMessageCommand()) {
    initReceiveMessage();
    return true;
  }
  return false;
}

bool IthoCC1101::parseMessageCommand() {
  // TODO nl0pvm: make this orcon proof?
  #if defined(CRC_FILTER)
  uint8_t mLen = 0;
  # define SET_MLEN(n) mLen = n;
  #else // if defined(CRC_FILTER)
  # define SET_MLEN(n)
  #endif // if defined(CRC_FILTER)

  messageDecode(&inMessage, &inIthoPacket);

  // deviceType of message type?
  inIthoPacket.deviceType = inIthoPacket.dataDecoded[0];

  // deviceID
  inIthoPacket.deviceId[0] = inIthoPacket.dataDecoded[1];
  inIthoPacket.deviceId[1] = inIthoPacket.dataDecoded[2];
  inIthoPacket.deviceId[2] = inIthoPacket.dataDecoded[3];

  // counter1
  inIthoPacket.counter = inIthoPacket.dataDecoded[4];

  // determine command
  inIthoPacket.command = IthoUnknown;

  // TODO: When enabling CRC_FILTER, most likely commands without SET_MLEN() need that check too
  if (checkIthoCommand(&inIthoPacket, ithoMessageHighCommandBytes)) {
    inIthoPacket.command = IthoHigh;
    SET_MLEN(11)
  } else if (checkIthoCommand(&inIthoPacket, ithoMessageRVHighCommandBytes)) {
    inIthoPacket.command = IthoHigh;
  } else if (checkIthoCommand(&inIthoPacket, ithoMessageMediumCommandBytes)) {
    inIthoPacket.command = IthoMedium;
    SET_MLEN(11)
  } else if (checkIthoCommand(&inIthoPacket, ithoMessageRVMediumCommandBytes)) {
    inIthoPacket.command = IthoMedium;
  } else if (checkIthoCommand(&inIthoPacket, ithoMessageLowCommandBytes)) {
    inIthoPacket.command = IthoLow;
    SET_MLEN(11)
  } else if (checkIthoCommand(&inIthoPacket, ithoMessageRVLowCommandBytes)) {
    inIthoPacket.command = IthoLow;
  } else if (checkIthoCommand(&inIthoPacket, ithoMessageRVAutoCommandBytes)) {
    inIthoPacket.command = IthoStandby;
  } else if (checkIthoCommand(&inIthoPacket, ithoMessageStandByCommandBytes)) {
    inIthoPacket.command = IthoStandby;
    SET_MLEN(11)
  } else if (checkIthoCommand(&inIthoPacket, ithoMessageTimer1CommandBytes)) {
    inIthoPacket.command = IthoTimer1;
    SET_MLEN(11)
  } else if (checkIthoCommand(&inIthoPacket, ithoMessageTimer2CommandBytes)) {
    inIthoPacket.command = IthoTimer2;
    SET_MLEN(11)
  } else if (checkIthoCommand(&inIthoPacket, ithoMessageTimer3CommandBytes)) {
    inIthoPacket.command = IthoTimer3;
    SET_MLEN(11)
  } else if (checkIthoCommand(&inIthoPacket, ithoMessageJoinCommandBytes)) {
    inIthoPacket.command = IthoJoin;
    SET_MLEN(20)
  } else if (checkIthoCommand(&inIthoPacket, ithoMessageJoin2CommandBytes)) {
    inIthoPacket.command = IthoJoin;
    SET_MLEN(20)
  } else if (checkIthoCommand(&inIthoPacket, ithoMessageRVJoinCommandBytes)) {
    inIthoPacket.command = IthoJoin;
  } else if (checkIthoCommand(&inIthoPacket, ithoMessageLeaveCommandBytes)) {
    inIthoPacket.command = IthoLeave;
    SET_MLEN(14)
  } else if (_enableOrcon) {
    if (checkIthoCommand(&inIthoPacket, orconMessageStandByCommandBytes)) {
      inIthoPacket.command = OrconStandBy;
    } else if (checkIthoCommand(&inIthoPacket, orconMessageLowCommandBytes)) {
      inIthoPacket.command = OrconLow;
    } else if (checkIthoCommand(&inIthoPacket, orconMessageMediumCommandBytes)) {
      inIthoPacket.command = OrconMedium;
    } else if (checkIthoCommand(&inIthoPacket, orconMessageFullCommandBytes)) {
      inIthoPacket.command = OrconHigh;
    } else if (checkIthoCommand(&inIthoPacket, orconMessageAutoCommandBytes)) {
      inIthoPacket.command = OrconAuto;
    } else if (checkIthoCommand(&inIthoPacket, orconMessageTimer0CommandBytes)) {
      inIthoPacket.command = OrconTimer0;
    } else if (checkIthoCommand(&inIthoPacket, orconMessageTimer1CommandBytes)) {
      inIthoPacket.command = OrconTimer1;
    } else if (checkIthoCommand(&inIthoPacket, orconMessageTimer2CommandBytes)) {
      inIthoPacket.command = OrconTimer2;
    } else if (checkIthoCommand(&inIthoPacket, orconMessageTimer3CommandBytes)) {
      inIthoPacket.command = OrconTimer3;
    } else if (checkIthoCommand(&inIthoPacket, orconMessageAutoCO2CommandBytes)) {
      inIthoPacket.command = OrconAutoCO2;
    }
  }
  #undef SET_MLEN

  #if defined(CRC_FILTER)

  if ((mLen != 0) && (getCounter2(&inIthoPacket, mLen) != inIthoPacket.dataDecoded[mLen])) {
    inIthoPacket.command = IthoUnknown;
    return false;
  }
  #endif // if defined(CRC_FILTER)

  return true;
}

bool IthoCC1101::checkIthoCommand(IthoPacket *itho, const uint8_t commandBytes[]) {
  uint8_t offset = 0;

  // this is quite hacky as not even the opcode is checked for itho. Because of that orcon 31E0 messages are wrongly recognised as itho
  // standby messages.
  // TODO nl0pvm: FIX THIS :D

  // first byte is the header of the message, this determines the structure of the rest of the message
  // The bits are used as follows <00TTAAPP>
  // 00 - Unused
  // TT - Message type
  // AA - Present DeviceID fields
  // PP - Present Params

  if ((itho->deviceType == 28) || (itho->deviceType == 24)) {
    offset = 2;
  }

  // for (int i = 4; i < 6; i++)
  // for Orcon: the code above makes that only 3 bytes (byte 4, 5 and 6) are checked. That gives false positves
  for (int i = 0; i < 6; i++) {
    // this is required for differentiating between Orcon and Itho commands. However I don't know what the reason was to comment this out.
    // thus this needs to be verified by Itho users
    if ((i == 2) || (i == 3)) {
      continue; // skip byte3 and byte4, rft-rv and co2-auto remote device seem to sometimes have a different number there
    }

    if ((itho->dataDecoded[i + 5 + offset] != pgm_read_byte(&(commandBytes[i]))) &&
        (itho->dataDecodedChk[i + 5 + offset] != pgm_read_byte(&(commandBytes[i])))) {
      return false;
    }
  }
  return true;
}

void IthoCC1101::sendCommand(IthoCommand command, uint8_t srcId[3], uint8_t destId[3])
{
  CC1101Packet outMessage;
  uint8_t maxTries  = sendTries;
  uint8_t delaytime = 40;

  // update itho packet data
  outIthoPacket.command  = command;
  outIthoPacket.counter += 1;

  // get message2 bytes
  switch (command)
  {
    case IthoJoin:
      createMessageJoin(&outIthoPacket, &outMessage);
      break;

    case IthoLeave:
      createMessageLeave(&outIthoPacket, &outMessage);

      // the leave command needs to be transmitted for 1 second according the manual
      maxTries  = 30;
      delaytime = 4;
      break;

    case OrconStandBy:
    case OrconLow:
    case OrconMedium:
    case OrconHigh:
    case OrconAuto:
    case OrconTimer0:
    case OrconTimer1:
    case OrconTimer2:
    case OrconTimer3:
    case OrconAutoCO2:

      if (_enableOrcon) {
        maxTries = 1;
        createOrconMessageCommand(&outIthoPacket, &outMessage, srcId, destId);
      }
      break;

    default:
      createMessageCommand(&outIthoPacket, &outMessage);
      break;
  }

  // send messages
  for (int i = 0; i < maxTries; i++)
  {
    // message2
    initSendMessage(outMessage.length);
    sendData(&outMessage);

    finishTransfer();
    delay(delaytime);
  }

  // initReceive(); SV - I call this from the ESPEasy plugin to prevent crashes
}

void IthoCC1101::createMessageStart(IthoPacket *itho, CC1101Packet *packet)
{
  // fixed, set start structure in data buffer manually
  for (uint8_t i = 0; i < 7; i++) {
    packet->data[i] = 170;
  }
  packet->data[7]  = 171;
  packet->data[8]  = 254;
  packet->data[9]  = 0;
  packet->data[10] = 179;
  packet->data[11] = 42;
  packet->data[12] = 171;
  packet->data[13] = 42;

  // [start of command specific data]
}

void IthoCC1101::createOrconMessageCommand(IthoPacket *itho, CC1101Packet *packet, uint8_t srcId[3], uint8_t destId[3])
{
  // set start message structure
  createMessageStart(itho, packet);

  // first byte is the header of the message, this determines the structure of the rest of the message
  // The bits are used as follows <00TTAAPP>
  // 00 - Unused
  // TT - Message type
  // AA - Present DeviceID fields
  // PP - Present Params
  uint8_t header = 0b00011100;

  itho->dataDecoded[0] = header; // 00TTAAPP
  // set source deviceID
  itho->dataDecoded[1] = srcId[0];
  itho->dataDecoded[2] = srcId[1];
  itho->dataDecoded[3] = srcId[2];

  // set destination deviceID
  itho->dataDecoded[4] = destId[0];
  itho->dataDecoded[5] = destId[1];
  itho->dataDecoded[6] = destId[2];

  const uint8_t *commandBytes  = getMessageCommandBytes(itho->command);
  const uint8_t  commandLength = getMessageCommandLength(itho->command);

  for (uint8_t i = 0; i < commandLength; i++) {
    itho->dataDecoded[i + 7] = pgm_read_byte(&(commandBytes[i]));
  }

  itho->length = 7 + 1 + commandLength;

  itho->dataDecoded[itho->length - 1] = getCRC(itho, itho->length - 1);
  itho->length                       += 1;

  packet->length = messageEncode(itho, packet) - 2; // delete the last two itho bytes (0x55, 0x95) so we can reuse messageEncode() without
                                                    // modifications

  // set compex orcon specific end bytes
  packet->data[packet->length] = 0xAC;
  packet->length              += 1;
  packet->data[packet->length] = 0xAA;
  packet->length              += 1;
  packet->data[packet->length] = 0xBF;
  packet->length              += 1;
  packet->data[packet->length] = 0x0E;
  packet->length              += 1;
}

uint8_t IthoCC1101::getCRC(IthoPacket *itho, uint8_t len) {
  uint8_t val = 0;

  for (uint8_t i = 0; i < len; i++) {
    val += itho->dataDecoded[i];
  }

  return 0x100 - (val & 0xFF);
}

void IthoCC1101::createMessageCommand(IthoPacket *itho, CC1101Packet *packet)
{
  // set start message structure
  createMessageStart(itho, packet);

  // set deviceType? (or messageType?), not sure what this is
  itho->dataDecoded[0] = itho->deviceType;

  // set deviceID
  itho->dataDecoded[1] = itho->deviceId[0];
  itho->dataDecoded[2] = itho->deviceId[1];
  itho->dataDecoded[3] = itho->deviceId[2];

  // set counter1
  itho->dataDecoded[4] = itho->counter;

  // set command bytes on dataDecoded[5 - 10]
  const uint8_t *commandBytes = getMessageCommandBytes(itho->command);

  for (uint8_t i = 0; i < 6; i++) {
    itho->dataDecoded[i + 5] = pgm_read_byte(&(commandBytes[i]));
  }

  // set counter2
  itho->dataDecoded[11] = getCounter2(itho, 11);

  itho->length = 12;

  packet->length  = messageEncode(itho, packet);
  packet->length += 1;

  // set end byte
  packet->data[packet->length] = 172;
  packet->length              += 1;

  // set end 'noise'
  for (uint8_t i = packet->length; i < packet->length + 7; i++) {
    packet->data[i] = 170;
  }
  packet->length += 7;
}

void IthoCC1101::createMessageJoin(IthoPacket *itho, CC1101Packet *packet)
{
  // set start message structure
  createMessageStart(itho, packet);

  // set deviceType? (or messageType?)
  itho->dataDecoded[0] = itho->deviceType;

  // set deviceID
  itho->dataDecoded[1] = itho->deviceId[0];
  itho->dataDecoded[2] = itho->deviceId[1];
  itho->dataDecoded[3] = itho->deviceId[2];

  // set counter1
  itho->dataDecoded[4] = itho->counter;

  // set command bytes on dataDecoded[5 - ?]
  const uint8_t *commandBytes = getMessageCommandBytes(itho->command);

  for (uint8_t i = 0; i < 6; i++) {
    itho->dataDecoded[i + 5] = pgm_read_byte(&(commandBytes[i]));
  }

  // set deviceID
  itho->dataDecoded[11] = itho->deviceId[0];
  itho->dataDecoded[12] = itho->deviceId[1];
  itho->dataDecoded[13] = itho->deviceId[2];

  itho->dataDecoded[14] = 1;
  itho->dataDecoded[15] = 16;
  itho->dataDecoded[16] = 224;

  // set deviceID
  itho->dataDecoded[17] = itho->deviceId[0];
  itho->dataDecoded[18] = itho->deviceId[1];
  itho->dataDecoded[19] = itho->deviceId[2];

  // set counter2
  itho->dataDecoded[20] = getCounter2(itho, 20);

  itho->length = 21;

  packet->length  = messageEncode(itho, packet);
  packet->length += 1;

  // set end byte
  packet->data[packet->length] = 202;
  packet->length              += 1;

  // set end 'noise'
  for (uint8_t i = packet->length; i < packet->length + 7; i++) {
    packet->data[i] = 170;
  }
  packet->length += 7;
}

void IthoCC1101::createMessageLeave(IthoPacket *itho, CC1101Packet *packet)
{
  // set start message structure
  createMessageStart(itho, packet);

  // set deviceType? (or messageType?)
  itho->dataDecoded[0] = itho->deviceType;

  // set deviceID
  itho->dataDecoded[1] = itho->deviceId[0];
  itho->dataDecoded[2] = itho->deviceId[1];
  itho->dataDecoded[3] = itho->deviceId[2];

  // set counter1
  itho->dataDecoded[4] = itho->counter;

  // set command bytes on dataDecoded[5 - 10]
  const uint8_t *commandBytes = getMessageCommandBytes(itho->command);

  for (uint8_t i = 0; i < 6; i++) {
    itho->dataDecoded[i + 5] = pgm_read_byte(&(commandBytes[i]));
  }

  // set deviceID
  itho->dataDecoded[11] = itho->deviceId[0];
  itho->dataDecoded[12] = itho->deviceId[1];
  itho->dataDecoded[13] = itho->deviceId[2];

  // set counter2
  itho->dataDecoded[14] = getCounter2(itho, 14);

  itho->length = 15;

  packet->length  = messageEncode(itho, packet);
  packet->length += 1;

  // set end byte
  packet->data[packet->length] = 202;
  packet->length              += 1;

  // set end 'noise'
  for (uint8_t i = packet->length; i < packet->length + 7; i++) {
    packet->data[i] = 170;
  }
  packet->length += 7;
}

const uint8_t * IthoCC1101::getMessageCommandBytes(IthoCommand command)
{
  switch (command)
  {
    case IthoStandby:
      return &ithoMessageStandByCommandBytes[0];
    case IthoHigh:
      return &ithoMessageHighCommandBytes[0];
    case IthoFull:
      return &ithoMessageFullCommandBytes[0];
    case IthoMedium:
      return &ithoMessageMediumCommandBytes[0];
    case IthoLow:
      return &ithoMessageLowCommandBytes[0];
    case IthoTimer1:
      return &ithoMessageTimer1CommandBytes[0];
    case IthoTimer2:
      return &ithoMessageTimer2CommandBytes[0];
    case IthoTimer3:
      return &ithoMessageTimer3CommandBytes[0];
    case IthoJoin:
      return &ithoMessageJoinCommandBytes[0];
    case IthoLeave:
      return &ithoMessageLeaveCommandBytes[0];

    case OrconStandBy:
      return &orconMessageStandByCommandBytes[0];
    case OrconLow:
      return &orconMessageLowCommandBytes[0];
    case OrconMedium:
      return &orconMessageMediumCommandBytes[0];
    case OrconHigh:
      return &orconMessageFullCommandBytes[0];
    case OrconAuto:
      return &orconMessageAutoCommandBytes[0];
    case OrconTimer0:
      return &orconMessageTimer0CommandBytes[0];
    case OrconTimer1:
      return &orconMessageTimer1CommandBytes[0];
    case OrconTimer2:
      return &orconMessageTimer2CommandBytes[0];
    case OrconTimer3:
      return &orconMessageTimer3CommandBytes[0];
    case OrconAutoCO2:
      return &orconMessageAutoCO2CommandBytes[0];

    default:
      return &ithoMessageLowCommandBytes[0];
  }
}

uint8_t IthoCC1101::getMessageCommandLength(IthoCommand command)
{
  switch (command)
  {
    case IthoStandby:
      return sizeof(ithoMessageStandByCommandBytes) / sizeof(uint8_t);
    case IthoHigh:
      return sizeof(ithoMessageHighCommandBytes) / sizeof(uint8_t);
    case IthoFull:
      return sizeof(ithoMessageFullCommandBytes) / sizeof(uint8_t);
    case IthoMedium:
      return sizeof(ithoMessageMediumCommandBytes) / sizeof(uint8_t);
    case IthoLow:
      return sizeof(ithoMessageLowCommandBytes) / sizeof(uint8_t);
    case IthoTimer1:
      return sizeof(ithoMessageTimer1CommandBytes) / sizeof(uint8_t);
    case IthoTimer2:
      return sizeof(ithoMessageTimer2CommandBytes) / sizeof(uint8_t);
    case IthoTimer3:
      return sizeof(ithoMessageTimer3CommandBytes) / sizeof(uint8_t);
    case IthoJoin:
      return sizeof(ithoMessageJoinCommandBytes) / sizeof(uint8_t);
    case IthoLeave:
      return sizeof(ithoMessageLeaveCommandBytes) / sizeof(uint8_t);

    case OrconStandBy:
      return sizeof(orconMessageStandByCommandBytes) / sizeof(uint8_t);
    case OrconLow:
      return sizeof(orconMessageLowCommandBytes) / sizeof(uint8_t);
    case OrconMedium:
      return sizeof(orconMessageMediumCommandBytes) / sizeof(uint8_t);
    case OrconHigh:
      return sizeof(orconMessageFullCommandBytes) / sizeof(uint8_t);
    case OrconAuto:
      return sizeof(orconMessageAutoCommandBytes) / sizeof(uint8_t);
    case OrconTimer0:
      return sizeof(orconMessageTimer0CommandBytes) / sizeof(uint8_t);
    case OrconTimer1:
      return sizeof(orconMessageTimer1CommandBytes) / sizeof(uint8_t);
    case OrconTimer2:
      return sizeof(orconMessageTimer2CommandBytes) / sizeof(uint8_t);
    case OrconTimer3:
      return sizeof(orconMessageTimer3CommandBytes) / sizeof(uint8_t);
    case OrconAutoCO2:
      return sizeof(orconMessageAutoCO2CommandBytes) / sizeof(uint8_t);


    default:
      return sizeof(ithoMessageLowCommandBytes) / sizeof(uint8_t);
  }
}

/*
   Counter2 is the decimal sum of all bytes in decoded form from
   deviceType up to the last byte before counter2 subtracted
   from zero.
 */
uint8_t IthoCC1101::getCounter2(IthoPacket *itho, uint8_t len) {
  uint8_t val = 0;

  for (uint8_t i = 0; i < len; i++) {
    val += itho->dataDecoded[i];
  }

  return 0 - val;
}

uint8_t IthoCC1101::messageEncode(IthoPacket *itho, CC1101Packet *packet) {
  // FIXME TD-er: lenOutbuf not used????

  /*
     uint8_t lenOutbuf = 0;

     if ((itho->length * 20) % 8 == 0) { // inData len fits niecly in out buffer length
     lenOutbuf = itho->length * 2.5;
     }
     else {                              // is this an issue? inData last byte does not fill out buffer length, add 1 out byte extra,
        padding
                                      // is done after encode
     lenOutbuf = (uint8_t)(itho->length * 2.5) + 0.5;
     }
   */

  uint8_t out_bytecounter    = 14; // index of Outbuf, start at offset 14, first part of the message is set manually
  uint8_t out_bitcounter     = 0;  // bit position of current outbuf byte
  uint8_t out_patterncounter = 0;  // bit counter to add 1 0 bit pattern after every 8 bits
  uint8_t bitSelect          = 4;  // bit position of the inData byte (4 - 7, 0 - 3)
  uint8_t out_shift          = 7;  // bit shift inData bit in position of outbuf byte

  // we need to zero the out buffer first cause we are using bitshifts
  for (unsigned int i = out_bytecounter; i < sizeof(packet->data) / sizeof(packet->data[0]); i++) {
    packet->data[i] = 0;
  }

  // Serial.println();
  for (uint8_t dataByte = 0; dataByte < itho->length; dataByte++) {
    for (uint8_t dataBit = 0; dataBit < 8; dataBit++) { // process a full dataByte at a time resulting in 20 output bits (2.5 bytes) with
                                                        // the pattern 7x6x5x4x 10 3x2x1x0x 10 7x6x5x4x 10 3x2x1x0x 10 etc
      if (out_bitcounter == 8) {                        // check if new byte is needed
        out_bytecounter++;
        out_bitcounter = 0;
      }

      if (out_patterncounter == 8) { // check if we have to start with a 1 0 pattern
        out_patterncounter            = 0;
        packet->data[out_bytecounter] = packet->data[out_bytecounter] | 1 << out_shift;
        out_shift--;
        out_bitcounter++;
        packet->data[out_bytecounter] = packet->data[out_bytecounter] | 0 << out_shift;

        if (out_shift == 0) { out_shift = 8; }
        out_shift--;
        out_bitcounter++;
      }

      if (out_bitcounter == 8) { // check if new byte is needed
        out_bytecounter++;
        out_bitcounter = 0;
      }

      // set the even bit
      uint8_t bit = (itho->dataDecoded[dataByte] & (1 << bitSelect)) >> bitSelect; // select bit and shift to bit pos 0
      bitSelect++;

      if (bitSelect == 8) { bitSelect = 0; }

      packet->data[out_bytecounter] = packet->data[out_bytecounter] | bit << out_shift; // shift bit in corect pos of current outbuf byte
      out_shift--;
      out_bitcounter++;
      out_patterncounter++;

      // set the odd bit (inverse of even bit)
      bit                           = ~bit & 0b00000001;
      packet->data[out_bytecounter] = packet->data[out_bytecounter] | bit << out_shift;

      if (out_shift == 0) { out_shift = 8; }
      out_shift--;
      out_bitcounter++;
      out_patterncounter++;
    }
  }

  if (out_bitcounter < 8) { // add closing 1 0 pattern to fill last packet->data byte and ensure DC balance in the message
    for (uint8_t i = out_bitcounter; i < 8; i += 2) {
      packet->data[out_bytecounter] = packet->data[out_bytecounter] | 1 << out_shift;
      out_shift--;
      packet->data[out_bytecounter] = packet->data[out_bytecounter] | 0 << out_shift;

      if (out_shift == 0) { out_shift = 8; }
      out_shift--;
    }
  }

  return out_bytecounter;
}

void IthoCC1101::messageDecode(CC1101Packet *packet, IthoPacket *itho) {
  itho->length = 0;
  int lenInbuf = packet->length;

  lenInbuf -= STARTBYTE; // correct for sync byte pos

  while (lenInbuf >= 5) {
    lenInbuf     -= 5;
    itho->length += 2;
  }

  if (lenInbuf >= 3) {
    itho->length++;
  }

  for (unsigned int i = 0; i < sizeof(itho->dataDecoded) / sizeof(itho->dataDecoded[0]); i++) {
    itho->dataDecoded[i] = 0;
  }

  for (unsigned int i = 0; i < sizeof(itho->dataDecodedChk) / sizeof(itho->dataDecodedChk[0]); i++) {
    itho->dataDecodedChk[i] = 0;
  }

  uint8_t out_i         = 0; // byte index
  uint8_t out_j         = 4; // bit index
  uint8_t out_i_chk     = 0; // byte index
  uint8_t out_j_chk     = 4; // bit index
  uint8_t in_bitcounter = 0; // process per 10 input bits

  for (int i = STARTBYTE; i < packet->length; i++) {
    for (int j = 7; j > -1; j--) {
      if ((in_bitcounter == 0) || (in_bitcounter == 2) || (in_bitcounter == 4) || (in_bitcounter == 6)) { // select input bits for output
        uint8_t x = packet->data[i];                                                                      // select input byte
        x                        = x >> j;                                                                // select input bit
        x                        = x & 0b00000001;
        x                        = x << out_j;                                                            // set value for output bit
        itho->dataDecoded[out_i] = itho->dataDecoded[out_i] | x;
        out_j                   += 1;                                                                     // next output bit

        if (out_j > 7) { out_j = 0; }

        if (out_j == 4) { out_i += 1; }
      }

      if ((in_bitcounter == 1) || (in_bitcounter == 3) || (in_bitcounter == 5) || (in_bitcounter == 7)) { // select input bits for check
                                                                                                          // output
        uint8_t x = packet->data[i];                                                                      // select input byte
        x                               = x >> j;                                                         // select input bit
        x                               = x & 0b00000001;
        x                               = x << out_j_chk;                                                 // set value for output bit
        itho->dataDecodedChk[out_i_chk] = itho->dataDecodedChk[out_i_chk] | x;
        out_j_chk                      += 1;                                                              // next output bit

        if (out_j_chk > 7) { out_j_chk = 0; }

        if (out_j_chk == 4) {
          itho->dataDecodedChk[out_i_chk] = ~itho->dataDecodedChk[out_i_chk]; // inverse bits
          out_i_chk                      += 1;
        }
      }
      in_bitcounter += 1; // continue cyling in groups of 10 bits

      if (in_bitcounter > 9) { in_bitcounter = 0; }
    }
  }
}

// uint8_t IthoCC1101::ReadRSSI()
// {
//   uint8_t rssi  = 0;
//   uint8_t value = 0;

//   rssi = (readRegister(CC1101_RSSI, CC1101_STATUS_REGISTER));

//   if (rssi >= 128)
//   {
//     value  = 255 - rssi;
//     value /= 2;
//     value += 74;
//   }
//   else
//   {
//     value  = rssi / 2;
//     value += 74;
//   }
//   return value;
// }

// bool IthoCC1101::checkID(const uint8_t *id) const
// {
//   for (uint8_t i = 0; i < 3; i++) {
//     if (id[i] != inIthoPacket.deviceId[i]) {
//       return false;
//     }
//   }
//   return true;
// }

String IthoCC1101::getLastIDstr(bool ashex) {
  String str;

  for (uint8_t i = 0; i < 3; i++) {
    if (ashex) { str += String(inIthoPacket.deviceId[i], HEX); }
    else { str += String(inIthoPacket.deviceId[i]); }

    if (i < 2) { str += ','; }
  }
  return str;
}

// int * IthoCC1101::getLastID() {
//   static int id[3];

//   for (uint8_t i = 0; i < 3; i++) {
//     id[i] = inIthoPacket.deviceId[i];
//   }
//   return id;
// }

// String IthoCC1101::getLastMessagestr(bool ashex) {
//   String str = F("Length=");

//   str += inMessage.length;
//   str += '.';

//   for (uint8_t i = 0; i < inMessage.length; i++) {
//     if (ashex) { str += String(inMessage.data[i], HEX); }
//     else { str += String(inMessage.data[i]); }

//     if (i < inMessage.length - 1) { str += ':'; }
//   }
//   return str;
// }

// String IthoCC1101::LastMessageDecoded() const {
//   String str;

//   if (inIthoPacket.length > 11) {
//     str += F("Device type?: ");
//     str += String(inIthoPacket.deviceType);
//     str += F(" - CMD: ");

//     for (int i = 4; i < inIthoPacket.length; i++) {
//       str += String(inIthoPacket.dataDecoded[i]);

//       if (i < inIthoPacket.length - 1) { str += ','; }
//     }
//   }
//   else {
//     for (uint8_t i = 0; i < inIthoPacket.length; i++) {
//       str += String(inIthoPacket.dataDecoded[i]);

//       if (i < inIthoPacket.length - 1) { str += ','; }
//     }
//   }
//   str += '\n';
//   return str;
// }
