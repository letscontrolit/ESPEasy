#ifndef DLBus_H
#define DLBus_H

#include <Arduino.h>
#include "../../_Plugin_Helper.h"

/*********************************************************************************************\
   DLBus subs to get values from the receiving bitstream
\*********************************************************************************************/

// one data frame has <P092_DataSettings.DataBytes> data bytes + SYNC, e.g. 64 * (8+1+1) + 16 = 656
// 656 * 2 = 1312 (twice as much as a data frame is saved
// so there's one complete data frame

#define DLbus_MaxDataBytes 64
#define DLbus_AdditionalRecBytes 2
#define DLbus_StopBits 1
#define DLbus_StartBits 1
#define DLBus_SyncBits 16
#define DLBus_ReserveBytes 20
#define DLBus_BitChangeFactor 2
#define DLbus_MaxDataBits (((DLbus_MaxDataBytes + DLbus_AdditionalRecBytes) * (DLbus_StartBits + 8 + DLbus_StopBits) + DLBus_SyncBits) * \
                           DLBus_BitChangeFactor) + DLBus_ReserveBytes

// MaxDataBits is double of the maximum bit length because each bit change is stored
// (((64+2) * (8+1+1) + 16) * 2) + 50 = 1402 bytes

class DLBus {
public:

  DLBus();                                // constructor of DLBus object
  ~DLBus();                               // destructor of DLBus object

  volatile uint8_t ISR_DLB_Pin = 0xFF;
  volatile boolean ISR_Receiving = false; // receiving flag
  volatile boolean ISR_AllBitsReceived = false;
  volatile uint16_t ISR_PulseCount = 0;   // number of received pulses
  volatile uint16_t ISR_PulseNumber = 0;  // max naumber of the received pulses
  volatile uint16_t ISR_MinPulseWidth, ISR_MaxPulseWidth, ISR_MinDoublePulseWidth, ISR_MaxDoublePulseWidth = 0;

  // identification bytes for each DL bus device
  uint8_t DeviceBytes[2] = { 0 };
  uint8_t ByteStream[DLbus_MaxDataBits / 8 + 1]; // every bit gets sorted into a bitmap
  boolean IsLogLevelInfo = false;
  uint8_t LogLevelInfo   = 0xff;
  uint8_t LogLevelError  = 0xFF;
  void    attachDLBusInterrupt(void);
  void    StartReceiving(void);
  boolean CheckTimings(void);
  boolean Processing(void);
  boolean CheckCRC(uint8_t IdxCRC);

private:

  volatile uint32_t ISR_TimeLastBitChange = 0;      // remember time of last transition
  uint8_t DLbus_ChangeBitStream[DLbus_MaxDataBits]; // received bit change stream (each bit change is extended to uint8_t, containing the
                                                    // timing flags)
  uint16_t BitNumber = 0;                           // bit number of the received DLbus_ChangeBitStream
  static void ISR(void);
  void        ISR_PinChanged(void);
  void        ProcessBit(uint8_t b);
  int         Analyze(void);
  void        Invert(void);
  uint8_t     ReadBit(int pos);
  void        WriteBit(int     pos,
                       uint8_t set);
  void        Trim(int start_bit);
  boolean     CheckDevice(void);
  static DLBus *__instance;
  void        AddToInfoLog(const String& string);
  void        AddToErrorLog(const String& string);
};

#endif // ifndef DLBus_H
