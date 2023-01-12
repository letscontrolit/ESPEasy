/*
 * Author: Klusjesman, supersjimmie, modified and reworked by arjenhiemstra
 * 2022-08-10 tonhuisman: Fixed perpetual blocking while loops by limiting these to 3000 msec.
 * 2022-08-11 tonhuisman: Change 3000 to #define ITHO_MAX_WAIT for easy adjustment
 * 2022-08-19 tonhuisman: Fix excessive memory use, comment unused methods to reduce bin size
 */

#ifndef __ITHOCC1101_H__
#define __ITHOCC1101_H__

#include <stdio.h>
#include "CC1101.h"
#include "IthoPacket.h"


// pa table settings
static const uint8_t ithoPaTableSend[8]    PROGMEM = { 0x6F, 0x26, 0x2E, 0x8C, 0x87, 0xCD, 0xC7, 0xC0 };
static const uint8_t ithoPaTableReceive[8] PROGMEM = { 0x6F, 0x26, 0x2E, 0x7F, 0x8A, 0x84, 0xCA, 0xC4 };

// message command bytes

static const uint8_t orconMessageStandByCommandBytes[] PROGMEM = { 34, 241, 3, 0, 0, 4 };
static const uint8_t orconMessageLowCommandBytes[]     PROGMEM = { 34, 241, 3, 0, 1, 4 };
static const uint8_t orconMessageMediumCommandBytes[]  PROGMEM = { 34, 241, 3, 0, 2, 4 };
static const uint8_t orconMessageFullCommandBytes[]    PROGMEM = { 34, 241, 3, 0, 3, 4 };
static const uint8_t orconMessageAutoCommandBytes[]    PROGMEM = { 34, 241, 3, 0, 4, 4 };

static const uint8_t orconMessageTimer0CommandBytes[]  PROGMEM = { 34, 243, 7, 0, 82, 12, 0, 4, 4, 4 }; //  Timer 12*60 minutes @ speed 0
static const uint8_t orconMessageTimer1CommandBytes[]  PROGMEM = { 34, 243, 7, 0, 18, 60, 1, 4, 4, 4 }; //  Timer 60 minutes @ speed 1
static const uint8_t orconMessageTimer2CommandBytes[]  PROGMEM = { 34, 243, 7, 0, 82, 13, 2, 4, 4, 4 }; //  Timer 13*60 minutes @ speed 2
static const uint8_t orconMessageTimer3CommandBytes[]  PROGMEM = { 34, 243, 7, 0, 18, 60, 3, 4, 4, 4 }; //  Timer 60 minutes @ speed 3
static const uint8_t orconMessageAutoCO2CommandBytes[] PROGMEM = { 34, 243, 7, 0, 18, 60, 4, 4, 4, 4 }; //  Timer 60 minutes @ speed auto

static const uint8_t ithoMessageRVHighCommandBytes[]   PROGMEM = { 49, 224, 4, 0, 0, 200 };
static const uint8_t ithoMessageHighCommandBytes[]     PROGMEM = { 34, 241, 3, 0, 4, 4 };
static const uint8_t ithoMessageFullCommandBytes[]     PROGMEM = { 34, 241, 3, 0, 4, 4 };
static const uint8_t ithoMessageMediumCommandBytes[]   PROGMEM = { 34, 241, 3, 0, 3, 4 };
static const uint8_t ithoMessageRVMediumCommandBytes[] PROGMEM = { 34, 241, 3, 0, 3, 7 };
static const uint8_t ithoMessageLowCommandBytes[]      PROGMEM = { 34, 241, 3, 0, 2, 4 };
static const uint8_t ithoMessageRVLowCommandBytes[]    PROGMEM = { 49, 224, 4, 0, 0, 1 };
static const uint8_t ithoMessageRVAutoCommandBytes[]   PROGMEM = { 34, 241, 3, 0, 5, 7 };
static const uint8_t ithoMessageStandByCommandBytes[]  PROGMEM = { 0, 0, 0, 0, 0, 0 };         // unkown, tbd
static const uint8_t ithoMessageTimer1CommandBytes[]   PROGMEM = { 34, 243, 3, 0, 0, 10 };     // 10 minutes full speed
static const uint8_t ithoMessageTimer2CommandBytes[]   PROGMEM = { 34, 243, 3, 0, 0, 20 };     // 20 minutes full speed
static const uint8_t ithoMessageTimer3CommandBytes[]   PROGMEM = { 34, 243, 3, 0, 0, 30 };     // 30 minutes full speed
static const uint8_t ithoMessageJoinCommandBytes[]     PROGMEM = { 31, 201, 12, 0, 34, 241 };
static const uint8_t ithoMessageJoin2CommandBytes[]    PROGMEM = { 31, 201, 12, 99, 34, 248 }; // join command of RFT AUTO Co2 remote
static const uint8_t ithoMessageRVJoinCommandBytes[]   PROGMEM = { 31, 201, 24, 0, 49, 224 };  // join command of RFT-RV
static const uint8_t ithoMessageLeaveCommandBytes[]    PROGMEM = { 31, 201, 6, 0, 31, 201 };

// itho rft-rv
// unknown, high
// 148,216,43,49,224,4,0,0,200,0,3,127,244,78,11,155,154,225,11,96,138
// 148,216,43,49,224,4,0,0,200,0,3,127,51,80,47,233,94,6,189,114,73

// low
// 148,216,43,49,224,4,0,0,1,0,202,127,242,212,160,123,15,64,7,129,33
// 148,216,43,34,241,3,0,4,4,194,127,255,189,90,107,88,72,115,49,192,105

// join
// 151,149,65,31,201,24,0,49,224,151,149,65,0,18,160,151,149,65,1,16,224


class IthoCC1101 : protected CC1101 {
private:

  // receive
  CC1101Packet inMessage;   // temp storage message2
  IthoPacket inIthoPacket;  // stores last received message data

  // send
  IthoPacket outIthoPacket; // stores state of "remote"

  // settings
  uint8_t sendTries;        // number of times a command is send at one button press

  // functions

public:

  IthoCC1101(int8_t  CSpin     = PIN_SPI_SS,
             int8_t  MISOpin   = MISO,
             uint8_t counter   = 0,
             uint8_t sendTries = 3); // set initial counter value
  ~IthoCC1101();

  // init
  void init() {
    CC1101::init();
    initReceive();
  } // init,reset CC1101

  void    initReceive();
  // uint8_t getLastCounter() const {
  //   return outIthoPacket.counter;
  // } // counter is increased before sending a command

  // void setSendTries(uint8_t sendTries) {
  //   this->sendTries = sendTries;
  // }

  void setDeviceID(uint8_t byte0, uint8_t byte1, uint8_t byte2) {
    this->outIthoPacket.deviceId[0] = byte0; this->outIthoPacket.deviceId[1] = byte1; this->outIthoPacket.deviceId[2] = byte2;
  }

  // receive
  bool       checkForNewPacket(); // check RX fifo for new data
  // IthoPacket getLastPacket() const {
  //   return inIthoPacket;
  // }                               // retrieve last received/parsed packet from remote

  IthoCommand getLastCommand() const {
    return inIthoPacket.command;
  } // retrieve last received/parsed command from remote

  // uint8_t getLastInCounter() const {
  //   return inIthoPacket.counter;
  // } // retrieve last received/parsed command from remote

  // uint8_t ReadRSSI();
  // bool    checkID(const uint8_t *id) const;
  // int*    getLastID();
  String  getLastIDstr(bool ashex      = true);
  // String  getLastMessagestr(bool ashex = true);
  // String  LastMessageDecoded() const;

  // send
  void    sendCommand(IthoCommand command,
                      uint8_t     srcId[3]  = 0,
                      uint8_t     destId[3] = 0);

  void enableOrcon(bool state);

protected:

private:

  IthoCC1101(const IthoCC1101& c);
  IthoCC1101& operator=(const IthoCC1101& c);

  // init CC1101 for receiving
  void        initReceiveMessage();

  // init CC1101 for sending
  void        initSendMessage(uint8_t len);
  void        finishTransfer();

  // parse received message
  bool        parseMessageCommand();
  bool        checkIthoCommand(IthoPacket   *itho,
                               const uint8_t commandBytes[]);

  // send
  void createMessageStart(IthoPacket   *itho,
                          CC1101Packet *packet);
  void createMessageCommand(IthoPacket   *itho,
                            CC1101Packet *packet);
  void createOrconMessageCommand(IthoPacket   *itho,
                                 CC1101Packet *packet,
                                 uint8_t       srcId[3],
                                 uint8_t       destId[3]);
  uint8_t        getCRC(IthoPacket *itho,
                        uint8_t     len);
  void           createMessageJoin(IthoPacket   *itho,
                                   CC1101Packet *packet);
  void           createMessageLeave(IthoPacket   *itho,
                                    CC1101Packet *packet);
  const uint8_t* getMessageCommandBytes(IthoCommand command);

  uint8_t        getMessageCommandLength(IthoCommand command);

  uint8_t        getCounter2(IthoPacket *itho,
                             uint8_t     len);

  uint8_t        messageEncode(IthoPacket   *itho,
                               CC1101Packet *packet);
  void           messageDecode(CC1101Packet *packet,
                               IthoPacket   *itho);
  bool _enableOrcon = false;
}; // IthoCC1101

#endif // __ITHOCC1101_H__
