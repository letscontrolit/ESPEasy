#pragma once

#include <Arduino.h>

#include "../DataStructs/KeyValueStruct.h"

struct C023_timestamped_value {
  C023_timestamped_value(String&& val) :
    lastCheck(millis()),
    value(std::move(val)) {}

  void set(const String& val);

  bool expired() const;

  uint32_t lastChange{};
  uint32_t lastCheck{};
  String   value;

};

class C023_AT_commands
{
public:

  // Value is stored, so do not change numerical values
  enum class LoRaModule_e : uint8_t {
    Dragino_LA66 = 0,
    RAK_3172     = 1,


    MAX_TYPE // Leave as last, used to iterate over all enum values

  };

  static const __FlashStringHelper* toString(LoRaModule_e module);


  // Received types.
  // These commands can be used to get and/or set a value.
  enum class AT_cmd : size_t {
    UUID,          //
    VER,           // 2.4 AT+VER: Image Version and Frequency Band
    CLIVER,        // RAK: CLI Version, AT Command Version
    HWMODEL,       // RAK: HW Model
    SN,            // RAK: Serial Number
    APPEUI,        // 3.1 AT+APPEUI: Application EUI
    APPKEY,        // 3.2 AT+APPKEY: Application Key
    DEUI,          // 3.5 AT+DEUI: Device EUI
    DEVEUI,
    APPSKEY,       // 3.3 AT+APPSKEY: Application Session Key
    DADDR,         // 3.4 AT+DADDR: Device Address
    DEVADDR,
    NWKSKEY,       // 3.7 AT+NWKSKEY: Network Session Key
    NWKID,         // 3.6 AT+NWKID: Network ID(You can enter this command change only after successful network connection)
    NETID,
    CFM,           // 4.1 AT+CFM: Confirm Mode
    NJM,           // 4.3 AT+NJM: LoRa® Network Join Mode
                   //  0 = ABP
                   //  1 = OTAA
                   //  2 = P2P
    NJS,           // 4.4 AT+NJS: LoRa® Network Join Status
    RECV,          // 4.5 AT+RECV: Print Last Received Data in Raw Format
    RECVB,         // 4.6 AT+RECVB: Print Last Received Data in Binary Format
    ADR,           // 5.1 AT+ADR: Adaptive Rate
    CLASS,         // 5.2 AT+CLASS: LoRa® Class(Currently only support class A, class C)
    DCS,           // 5.3 AT+DCS: Duty Cycle Setting
    DR,            // 5.4 AT+DR: Data Rate (Can Only be Modified after ADR=0)
    FCD,           // 5.5 AT+FCD: Frame Counter Downlink
    FCU,           // 5.6 AT+FCU: Frame Counter Uplink
    JN1DL,         // 5.7 AT+JN1DL: Join Accept Delay1
    JN2DL,         // 5.8 AT+JN2DL: Join Accept Delay2
    PNM,           // 5.9 AT+PNM: Public Network Mode
    RX1DL,         // 5.10 AT+RX1DL: Receive Delay1
    RX2DL,         // 5.11 AT+RX2DL: Receive Delay2
    RX2DR,         // 5.12 AT+RX2DR: Rx2 Window Data Rate
    RX2FQ,         // 5.13 AT+RX2FQ: Rx2 Window Frequency
    TXP,           // 5.14 AT+TXP: Transmit Power
                   // TxPower(dBM) offset from Max. ERP.
                   // Region      | 0   1   2   3   4    5    6    7    8    9   10
                   // ------------|------------------------------------------------
                   // AS923       | 0  -2  -4  -6  -8  -10  -12  -14
                   // AU915       | 0  -2  -4  -6  -8  -10  -14  -14  -16  -18  -20
                   // CN470       | 0  -2  -4  -6  -8  -10  -14  -14
                   // CN779       | 0  -2  -4  -6  -8  -10
                   // EU433       | 0  -2  -4  -6  -8  -10
                   // EU868       | 0  -2  -4  -6  -8  -10  -14  -14
                   // IN865       | 0  -2  -4  -6  -8  -10  -14  -14  -16  -18  -20
                   // KR920       | 0  -2  -4  -6  -8  -10  -14  -14
                   // US915       | 0  -2  -4  -6  -8  -10  -16  -16  -16  -16  -10
                   // US915_HYBRID| 0  -2  -4  -6  -8  -10  -16  -16  -16  -16  -10
    RSSI,          // 5.15 AT+RSSI: RSSI of the Last Received Packet
    SNR,           // 5.16 AT+SNR: SNR of the Last Received Packet
    PORT,          // 5.17 AT+PORT: Application Port
    CHS,           // 5.18 AT+CHS: Single Channel Mode
    SLEEP,         // 5.20 AT+SLEEP: Set sleep mode
    BAT,           // 5.22 AT+BAT: Get the current battery voltage in Mv
    RJTDC,         // 5.23 AT+RJTDC: Get or set the ReJoin data transmission interval in min
    RPL,           // 5.24 AT+RPL: Get or set response level
                   // This feature is used to set compatible with different LoRaWAN servers. If RPL doesn;t match , user will see strange
                   // message in the
                   // server portal.
                   // RPL value:
                   //  AT+RPL=0: Device won't immediately reply any downlink commands from platform.
                   //  AT+RPL=1: Device will immediately reply message to Unconfirmed Data Down. Payload is 0x00.
                   //  AT+RPL=2: Device will immediately reply message to Confirmed Data Down. Payload is 0x00 and requied response header
                   //   for this command.
                   //  AT+RPL=3: Device will immediately reply message to MAC Command. Payload is 0x00 and requied response header for this
                   //   command.
                   //  AT+RPL=4: Device will immediately reply message to Confirmed Data Down & MAC Command. Payload is 0x00 and requied
                   //   response header for these two commands.
                   // Case Analyes:
                   //  For Class A devices, AT+RPL=0 is ok. that is defaut settings in software.
                   //  For Class C devices used in ChirpStack, need to set AT+RPL=4 because Chirpstack require immedietely reply message to
                   //   MAC Command.
                   //  For Class C devices used in TTI, need to set AT+RPL=4 because TTI require immediately reply message to Confirmed Data
                   //   Down & MAC Command.
    TIMESTAMP,     // 5.25 AT+TIMESTAMP: Get or Set UNIX timestamp in second
    LTIME,         // RAK: Get Local time
    LEAPSEC,       // 5.26 AT+LEAPSEC: Get or Set Leap Second
    SYNCMOD,       // 5.27 AT+SYNCMOD: Get or Set time synchronization method
    SYNCTDC,       // 5.28 AT+SYNCTDC: Get or Set time synchronization interval in day
    DDETECT,       // 5.29 AT+DDETECT: Get or set the downlink detection
                   //      Device offline rejoining. <Flag>,<ACK_Timout_1>,<ACK_Timout_2>  (timeout in min)
                   // Enable Online Detect, if end node doesn't receive any downlink within ACK_Timout_1( 1440 minutes or 24 hours).
                   // End node will use confirmed uplink to send packets during ACK_Timout_1 (the 24th hour) to ACK_Timout_2 ( the 48th
                   // hour). If from the 24th to 48th hour, end node got an downlink from server, it will switch back to unconfirmed uplink.
                   // end node will restart ACK_Timout_1. If from the 24th to 48th hour, end node still not got any downlink, means device
                   // doesn't get ACK from server within last 48 hours. Device will process rejoin, rejoin request interval is AT+RJTDC
                   // period. For AU915/ US915, device will use the sub-band used for last join.

    SETMAXNBTRANS, // 5.30 AT+SETMAXNBTRANS: Get or set the max nbtrans in LinkADR.
                   //      Value1: set the maximum NBTrans.
                   //      value2:  0: uplink fcnt doesn't change for each NBTrans;
                   //               1: uplink fcnt increase by 1 for each NBTrans.
    Unknown

  };

  static bool                       isVolatileValue(AT_cmd at_cmd);

  static bool                       supported(AT_cmd       at_cmd,
                                              LoRaModule_e module);

  static const __FlashStringHelper* toFlashString(AT_cmd       at_cmd,
                                                  LoRaModule_e module);

  static String                     toString(AT_cmd       at_cmd,
                                             LoRaModule_e module);
  static const __FlashStringHelper* toDisplayString(AT_cmd at_cmd);

  static AT_cmd                     determineReceivedDataType(const String& receivedData);

  static AT_cmd                     decode(const String& receivedData,
                                           String      & value);

  static KeyValueStruct             getKeyValue(AT_cmd        at_cmd,
                                                LoRaModule_e  module,
                                                const String& value,
                                                bool          extendedValue);


}; // class C023_AT_commands
