#include "P092_data_struct.h"

#ifdef USES_P092

//
// DLBus reads and decodes the DL-Bus.
// The DL-Bus is used in heating control units e.g. sold by Technische Alternative (www.ta.co.at).
// Author uwekaditz

// #define DLbus_DEBUG

// Flags for pulse width (bit 0 is the content!)
#define DLbus_FlagSingleWidth                   0x02
#define DLbus_FlagDoubleWidth                   0x04
#define DLbus_FlagShorterThanSingleWidth      0x10
#define DLbus_FlagBetweenDoubleSingleWidth    0x20
#define DLbus_FlagLongerThanDoubleWidth       0x40
#define DLbus_FlagLongerThanTwiceDoubleWidth  0x80
#define DLbus_FlagsWrongTiming                (DLbus_FlagLongerThanTwiceDoubleWidth | DLbus_FlagLongerThanDoubleWidth | \
                                               DLbus_FlagBetweenDoubleSingleWidth | DLbus_FlagShorterThanSingleWidth)

// Helper for ISR call
DLBus *DLBus::__instance                         = nullptr;
volatile  static uint8_t *ISR_PtrChangeBitStream = nullptr; // pointer to received bit change stream

DLBus::DLBus()
{
  if (__instance == nullptr)
  {
    __instance             = this;
    ISR_PtrChangeBitStream = DLbus_ChangeBitStream;
    addToLog(LOG_LEVEL_INFO, F("Class DLBus created"));
  }
}

DLBus::~DLBus()
{
  if (__instance == this)
  {
    __instance             = nullptr;
    ISR_PtrChangeBitStream = nullptr;
    addToLog(LOG_LEVEL_INFO, F("Class DLBus destroyed"));
  }
}

void DLBus::AddToInfoLog(const String& string)
{
  if ((IsLogLevelInfo) && (LogLevelInfo != 0xff)) {
    addToLog(LogLevelInfo, string);
  }
}

void DLBus::AddToErrorLog(const String& string)
{
  if (LogLevelError != 0xff) {
    addToLog(LogLevelError, string);
  }
}

void DLBus::attachDLBusInterrupt(void)
{
  ISR_Receiving = false;
  IsISRset = true;
  IsNoData = false;
  attachInterrupt(digitalPinToInterrupt(ISR_DLB_Pin), ISR, CHANGE);
}

void DLBus::StartReceiving(void)
{
  noInterrupts(); // make sure we don't get interrupted before we are ready
  ISR_PulseCount      = 0;
  ISR_Receiving       = (ISR_PtrChangeBitStream != nullptr);
  ISR_AllBitsReceived = false;
  interrupts(); // interrupts allowed now, next instruction WILL be executed
}

void ICACHE_RAM_ATTR DLBus::ISR(void)
{
  if (__instance)
  {
    __instance->ISR_PinChanged();
  }
}

void ICACHE_RAM_ATTR DLBus::ISR_PinChanged(void)
{
//  long TimeDiff = usecPassedSince(ISR_TimeLastBitChange); // time difference to previous pulse in Âµs
  uint32_t _now = micros();
  int32_t TimeDiff = (int32_t)(_now - ISR_TimeLastBitChange);

//  ISR_TimeLastBitChange = micros();                           // save last pin change time
  ISR_TimeLastBitChange = _now;                           // save last pin change time

  if (ISR_Receiving) {
    uint8_t val = digitalRead(ISR_DLB_Pin);               // read state

    // check pulse width
    if (TimeDiff >= 2 * ISR_MinDoublePulseWidth) {
      val |= DLbus_FlagLongerThanTwiceDoubleWidth; // longer then 2x double pulse width
    }
    else if (TimeDiff > ISR_MaxDoublePulseWidth) {
      val |= DLbus_FlagLongerThanDoubleWidth;      // longer then double pulse width
    }
    else if (TimeDiff >= ISR_MinDoublePulseWidth) {
      val |= DLbus_FlagDoubleWidth;                // double pulse width
    }
    else if (TimeDiff > ISR_MaxPulseWidth) {
      val |= DLbus_FlagBetweenDoubleSingleWidth;   // between double and single pulse width
    }
    else if (TimeDiff < ISR_MinPulseWidth) {
      val |= DLbus_FlagShorterThanSingleWidth;     // shorter then single pulse width
    }
    else {
      val |= DLbus_FlagSingleWidth;                // single pulse width
    }

    if (ISR_PulseCount < 2) {
      // check if sync is received
      if (val & DLbus_FlagLongerThanTwiceDoubleWidth) {
        // sync received
        *ISR_PtrChangeBitStream       = !(val & 0x01);
        *(ISR_PtrChangeBitStream + 1) = val;
        ISR_PulseCount                = 2;
      }
      else {
        ISR_PulseCount = 1; // flag that interrupt is receiving
      }
    }
    else {
      *(ISR_PtrChangeBitStream + ISR_PulseCount) = val;         // set bit
      ISR_PulseCount++;
      ISR_Receiving       = (ISR_PulseCount < ISR_PulseNumber); // stop P092_receiving when data frame is complete
      ISR_AllBitsReceived = !ISR_Receiving;
    }
  }
}

boolean DLBus::CheckTimings(void) {
  uint8_t rawval, val;
  uint8_t WrongTimeCnt = 0;
  int     i;

#ifdef DLbus_DEBUG
  uint16_t WrongTimingArray[5][6];
#endif // DLbus_DEBUG

  //  AddToInfoLog(F("Receive stopped."));

  ISR_PulseCount = 0;

  for (i = 0; i <= ISR_PulseNumber; i++) {
    // store DLbus_ChangeBitStream into ByteStream
    rawval = *(ISR_PtrChangeBitStream + i);

    if (rawval & DLbus_FlagsWrongTiming) {
      // wrong DLbus_time_diff
      if (ISR_PulseCount > 0) {
#ifdef DLbus_DEBUG
        WrongTimingArray[WrongTimeCnt][0] = i;
        WrongTimingArray[WrongTimeCnt][1] = ISR_PulseCount;
        WrongTimingArray[WrongTimeCnt][2] = BitNumber;
        WrongTimingArray[WrongTimeCnt][3] = rawval;
#endif // DLbus_DEBUG

        if ((rawval == DLbus_FlagLongerThanTwiceDoubleWidth) && (*(ISR_PtrChangeBitStream + i - 1) == (DLbus_FlagDoubleWidth | 0x01))) {
          // Add two additional short pulses (low and high), previous bit is High and contains DLbus_FlagDoubleWidth
          ProcessBit(0);
          ProcessBit(1);
#ifdef DLbus_DEBUG
          WrongTimingArray[WrongTimeCnt][4] = DLbus_FlagSingleWidth;
          WrongTimingArray[WrongTimeCnt][5] = DLbus_FlagSingleWidth + 1;
#endif // DLbus_DEBUG
        }
#ifdef DLbus_DEBUG
        else {
          WrongTimingArray[WrongTimeCnt][4] = 0xff;
          WrongTimingArray[WrongTimeCnt][5] = 0xff;
        }
#endif // DLbus_DEBUG
        WrongTimeCnt++;

        if (WrongTimeCnt >= 5) {
          return false;
        }
      }
    }
    else {
      val = rawval & 0x01;

      if ((rawval & DLbus_FlagDoubleWidth) == DLbus_FlagDoubleWidth) {
        // double pulse width
        ProcessBit(!val);
        ProcessBit(val);
      }
      else {
        // single pulse width
        ProcessBit(val);
      }
    }
  }

  //  AddToInfoLog(F("DLbus_ChangeBitStream copied."));

#ifdef DLbus_DEBUG

  if (WrongTimeCnt > 0) {
    if (IsLogLevelInfo) {
      String log = F("Wrong Timings: ");
      AddToInfoLog(log);

      for (i = 0; i < WrongTimeCnt; i++) {
        log  = i + 1;
        log += F(": PulseCount:");
        log += WrongTimingArray[i][1];
        log += F(": BitCount:");
        log += WrongTimingArray[i][2];
        log += F(" Value:0x");
        log += String(WrongTimingArray[i][3], HEX);
        log += F(" ValueBefore:0x");
        log += String(*(ISR_PtrChangeBitStream + WrongTimingArray[i][0] - 1), HEX);
        log += F(" ValueAfter:0x");
        log += String(*(ISR_PtrChangeBitStream + WrongTimingArray[i][0] + 1), HEX);

        if (WrongTimingArray[i][4] != 0xff) {
          log += F(" Added:0x");
          log += String(WrongTimingArray[i][4], HEX);
        }

        if (WrongTimingArray[i][5] != 0xff) {
          log += F(" Added:0x");
          log += String(WrongTimingArray[i][5], HEX);
        }
        AddToInfoLog(log);
      }
    }
  }
#endif // DLbus_DEBUG
  return true;
}

void DLBus::ProcessBit(uint8_t b) {
  // ignore first pulse
  ISR_PulseCount++;

  if (ISR_PulseCount % 2) {
    return;
  }
  BitNumber = (ISR_PulseCount / 2);

  if (b) {
    ByteStream[BitNumber / 8] |= (1 << (BitNumber % 8));  // set bit
  }
  else {
    ByteStream[BitNumber / 8] &= ~(1 << (BitNumber % 8)); // clear bit
  }
}

boolean DLBus::Processing(void) {
  boolean inverted = false;
  int16_t StartBit; // first bit of data frame (-1 not recognized)
  String  log;

  AddToInfoLog(F("Processing..."));
  StartBit = Analyze(); // find the data frame's beginning

  // inverted signal?
  while (StartBit == -1) {
    if (inverted) {
      AddToErrorLog(F("Error: Already inverted!"));
      return false;
    }
    Invert(); // invert again
    inverted = true;
    StartBit = Analyze();

    if (StartBit == -1) {
      AddToErrorLog(F("Error: No data frame available!"));
      return false;
    }
    uint16_t RequiredBitStreamLength = (ISR_PulseNumber - DLBus_ReserveBytes) / DLBus_BitChangeFactor;

    if ((BitNumber - StartBit) < RequiredBitStreamLength) {
      // no complete data frame available (difference between start_bit and received bits is < RequiredBitStreamLength)
      AddToErrorLog(F("Start bit too close to end of stream!"));

      if (IsLogLevelInfo) {
        log  = F("# Required bits: ");
        log += RequiredBitStreamLength;
        log += F(" StartBit: ");
        log += StartBit;
        log += F(" / EndBit: ");
        log += BitNumber;
        AddToInfoLog(log);
      }
      return false;
    }
  }

  if (IsLogLevelInfo) {
    log  = F("StartBit: ");
    log += StartBit;
    log += F(" / EndBit: ");
    log += BitNumber;
    AddToInfoLog(log);
  }
  Trim(StartBit);      // remove start and stop bits

  if (CheckDevice()) { // check connected device
    return true;
  }
  else {
    AddToErrorLog(F("Error: Device not found!"));
    return false;
  }
}

int DLBus::Analyze(void) {
  uint8_t sync = 0;

  // find SYNC (16 * sequential 1)
  for (int i = 0; i < BitNumber; i++) {
    if (ReadBit(i)) {
      sync++;
    }
    else {
      sync = 0;
    }

    if (sync == DLBus_SyncBits) {
      // finde erste 0 // find first 0
      while (ReadBit(i) == 1) {
        i++;
      }
      return i; // beginning of data frame
    }
  }

  // no data frame available. check signal?
  return -1;
}

void DLBus::Invert(void) {
  AddToInfoLog(F("Invert bit stream..."));

  for (int i = 0; i < BitNumber; i++) {
    WriteBit(i, ReadBit(i) ? 0 : 1); // invert every bit
  }
}

uint8_t DLBus::ReadBit(int pos) {
  int row = pos / 8;                          // detect position in bitmap
  int col = pos % 8;

  return ((ByteStream[row]) >> (col)) & 0x01; // return bit
}

void DLBus::WriteBit(int pos, uint8_t set) {
  int row = pos / 8; // detect position in bitmap
  int col = pos % 8;

  if (set) {
    ByteStream[row] |= 1 << col;    // set bit
  }
  else {
    ByteStream[row] &= ~(1 << col); // clear bit
  }
}

void DLBus::Trim(int start_bit) {
  for (int i = start_bit, bit = 0; i < BitNumber; i++) {
    int offset = i - start_bit;

    // ignore start and stop bits:
    // start bits: 0 10 20 30, also  x    % 10 == 0
    // stop bits:  9 19 29 39, also (x+1) % 10 == 0
    if (offset % 10 && (offset + 1) % 10) {
      WriteBit(bit, ReadBit(i));
      bit++;
    }
  }
}

boolean DLBus::CheckDevice(void) {
  // Data frame of a device?
  if (ByteStream[0] == DeviceBytes[0]) {
    if ((DeviceBytes[1] == 0) || (ByteStream[1] == DeviceBytes[1])) {
      return true;
    }
  }

  if (IsLogLevelInfo) {
    String log = F("# Received DeviceByte(s): 0x");
    log += String(ByteStream[0], HEX);

    if (DeviceBytes[1] != 0) {
      log += String(ByteStream[1], HEX);
    }
    log += F(" Requested: 0x");
    log += String(DeviceBytes[0], HEX);

    if (DeviceBytes[1] != 0) {
      log += String(DeviceBytes[1], HEX);
    }
    AddToInfoLog(log);
  }
  return false;
}

boolean DLBus::CheckCRC(uint8_t IdxCRC) {
  // CRC check sum
  if (IdxCRC == 0) {
    return true;
  }
  AddToInfoLog(F("Check CRC..."));
  uint16_t dataSum = 0;

  for (int i = 0; i < IdxCRC; i++) {
    dataSum = dataSum + ByteStream[i];
  }
  dataSum = dataSum & 0xff;

  if (dataSum == ByteStream[IdxCRC]) {
    return true;
  }
  AddToErrorLog(F("Check CRC failed!"));

  if (IsLogLevelInfo) {
    String log = F("# Calculated CRC: 0x");
    log += String(dataSum, HEX);
    log += F(" Received: 0x");
    log += String(ByteStream[IdxCRC], HEX);
    AddToInfoLog(log);
  }
  return false;
}

#endif