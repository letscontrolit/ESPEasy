#include "../PluginStructs/P073_data_struct.h"

#ifdef USES_P073
# include <GPIO_Direct_Access.h>

// ===================================
// ---- TM1637 specific functions ----
// ===================================

# define CLK_HIGH() DIRECT_pinWrite(this->pin1, HIGH)
# define CLK_LOW() DIRECT_pinWrite(this->pin1, LOW)
# define DIO_HIGH() DIRECT_pinWrite(this->pin2, HIGH)
# define DIO_LOW() DIRECT_PINMODE_OUTPUT(this->pin2); DIRECT_pinWrite(this->pin2, LOW)
# define DIO_INPUT() DIRECT_PINMODE_INPUT(this->pin2)
# define DIO_OUTPUT() DIRECT_PINMODE_OUTPUT(this->pin2)

void P073_data_struct::tm1637_i2cStart() {
  # if defined(P073_DEBUG) && !defined(BUILD_NO_DEBUG)
  addLog(LOG_LEVEL_DEBUG, F("7DGT : Comm Start"));
  # endif // if defined(P073_DEBUG) && !defined(BUILD_NO_DEBUG)
  DIO_LOW();
  delayMicroseconds(TM1637_CLOCKDELAY);
}

void P073_data_struct::tm1637_i2cStop() {
  # if defined(P073_DEBUG) && !defined(BUILD_NO_DEBUG)
  addLog(LOG_LEVEL_DEBUG, F("7DGT : Comm Stop"));
  # endif // if defined(P073_DEBUG) && !defined(BUILD_NO_DEBUG)
  DIO_LOW();
  delayMicroseconds(TM1637_CLOCKDELAY);
  CLK_HIGH();
  delayMicroseconds(TM1637_CLOCKDELAY);
  DIO_HIGH();
  delayMicroseconds(TM1637_CLOCKDELAY);
}

bool P073_data_struct::tm1637_i2cAck() {
  CLK_LOW();
  DIO_INPUT();

  delayMicroseconds(TM1637_CLOCKDELAY);
  CLK_HIGH();
  const uint32_t start_wait = micros();

  const bool acknowledged = -1 !=
                            DIRECT_measureWaitForPinState_ISR(this->pin2, start_wait, TM1637_CLOCKDELAY, 0);

  const int32_t timePassed = usecPassedSince_fast(start_wait);

  if (timePassed < TM1637_CLOCKDELAY) {
    delayMicroseconds(TM1637_CLOCKDELAY - timePassed);
  }

  # if defined(P073_DEBUG) && !defined(BUILD_NO_DEBUG)

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("7DGT : Comm ACK=");

    if (acknowledged) {
      log += F("TRUE");
    } else {
      log += F("FALSE");
    }
    addLogMove(LOG_LEVEL_DEBUG, log);
  }
  # endif // if defined(P073_DEBUG) && !defined(BUILD_NO_DEBUG)
  CLK_HIGH();

  delayMicroseconds(TM1637_CLOCKDELAY);
  CLK_LOW();
  delayMicroseconds(TM1637_CLOCKDELAY);
  DIO_OUTPUT();

  return acknowledged;
}

void P073_data_struct::tm1637_i2cWrite_ack(uint8_t bytesToPrint[],
                                           uint8_t length) {
  # ifdef P073_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, concat(F("7DGT : TM1637 databuffer: 0x"), formatToHex_array(bytesToPrint, length)));
  }
  # endif // ifdef P073_DEBUG
  tm1637_i2cStart();

  for (uint8_t i = 0; i < length; ++i) {
    tm1637_i2cWriteByte_ack(bytesToPrint[i]);
  }
  tm1637_i2cStop();
}

void P073_data_struct::tm1637_i2cWriteByte_ack(uint8_t bytetoprint) {
  tm1637_i2cWrite(bytetoprint);
  tm1637_i2cAck();
}

void P073_data_struct::tm1637_i2cWrite(uint8_t bytetoprint) {
  # if defined(P073_DEBUG) && !defined(BUILD_NO_DEBUG)
  addLog(LOG_LEVEL_DEBUG, F("7DGT : WriteByte"));
  # endif // if defined(P073_DEBUG) && !defined(BUILD_NO_DEBUG)

  for (uint8_t i = 0; i < 8; ++i) {
    CLK_LOW();
    delayMicroseconds(TM1637_CLOCKDELAY >> 1);

    if (bytetoprint & 0b00000001) {
      DIO_HIGH();
    } else {
      DIO_LOW();
    }
    delayMicroseconds(TM1637_CLOCKDELAY >> 1);
    bytetoprint = bytetoprint >> 1;
    CLK_HIGH();
    delayMicroseconds(TM1637_CLOCKDELAY);
  }
}

void P073_data_struct::tm1637_ClearDisplay() {
  uint8_t bytesToPrint[7]{};

  bytesToPrint[0] = 0xC0;
  tm1637_i2cWrite_ack(bytesToPrint, 7);
}

void P073_data_struct::tm1637_SetPowerBrightness(uint8_t brightlvl,
                                                 bool    poweron) {
  # ifdef P073_DEBUG
  addLog(LOG_LEVEL_INFO, F("7DGT : Set BRIGHT"));
  # endif // ifdef P073_DEBUG
  brightlvl &= 0b111;

  if (poweron) {
    brightlvl |= TM1637_POWER_ON;
  } else {
    brightlvl |= TM1637_POWER_OFF;
  }

  uint8_t bytesToPrint[]{ brightlvl };
  tm1637_i2cWrite_ack(bytesToPrint, NR_ELEMENTS(bytesToPrint));
}

void P073_data_struct::tm1637_InitDisplay() {
  pinMode(this->pin1, OUTPUT); // Use Arduino pin initialization as some ESPs don't properly set up their pins with DIRECT_GPIO_OUTPUT
  pinMode(this->pin2, OUTPUT);

  DIRECT_pinWrite(this->pin1, HIGH);
  DIRECT_pinWrite(this->pin2, HIGH);

  delayMicroseconds(TM1637_CLOCKDELAY);
  uint8_t bytesToPrint[]{ 0x40 };
  tm1637_i2cWrite_ack(bytesToPrint, NR_ELEMENTS(bytesToPrint));
  tm1637_ClearDisplay();
}

uint8_t P073_data_struct::tm1637_separator(uint8_t value,
                                           bool    sep) {
  if (sep) {
    value |= 0b10000000;
  }
  return value;
}

void P073_data_struct::tm1637_ShowTime6() {
  tm1637_ShowDate6(true); // deduplicated
}

void P073_data_struct::tm1637_ShowDate6(bool showTime) {
  uint8_t bytesToPrint[7]{};

  bytesToPrint[0] = 0xC0;
  bytesToPrint[1] = tm1637_getFontChar(showbuffer[2], fontset);
  bytesToPrint[2] = tm1637_separator(tm1637_getFontChar(showbuffer[1], fontset), timesep);
  bytesToPrint[3] = tm1637_getFontChar(showbuffer[0], fontset);

  if (showTime) {
    bytesToPrint[4] = tm1637_getFontChar(showbuffer[5], fontset);
    bytesToPrint[5] = tm1637_getFontChar(showbuffer[4], fontset);
  } else {
    bytesToPrint[4] = tm1637_getFontChar(showbuffer[7], fontset);
    bytesToPrint[5] = tm1637_getFontChar(showbuffer[6], fontset);
  }
  bytesToPrint[6] = tm1637_separator(tm1637_getFontChar(showbuffer[3], fontset), timesep);

  tm1637_i2cWrite_ack(bytesToPrint, 7);
}

void P073_data_struct::tm1637_ShowTemp6(bool sep) {
  uint8_t bytesToPrint[7]{};

  bytesToPrint[0] = 0xC0;
  bytesToPrint[1] = tm1637_separator(tm1637_getFontChar(showbuffer[5], fontset), sep);
  bytesToPrint[2] = tm1637_getFontChar(showbuffer[4], fontset);
  bytesToPrint[3] = tm1637_getFontChar(showbuffer[3], fontset); // Fill first digit of display too
  bytesToPrint[4] = tm1637_getFontChar(10, fontset);
  bytesToPrint[5] = tm1637_getFontChar(showbuffer[7], fontset);
  bytesToPrint[6] = tm1637_getFontChar(showbuffer[6], fontset);

  tm1637_i2cWrite_ack(bytesToPrint, 7);
}

void P073_data_struct::tm1637_ShowTimeTemp4(bool    sep,
                                            uint8_t bufoffset) {
  uint8_t bytesToPrint[5]{};

  bytesToPrint[0] = 0xC0;
  bytesToPrint[1] = tm1637_getFontChar(showbuffer[0 + bufoffset], fontset);
  bytesToPrint[2] = tm1637_separator(tm1637_getFontChar(showbuffer[1 + bufoffset], fontset), sep);
  bytesToPrint[3] = tm1637_getFontChar(showbuffer[2 + bufoffset], fontset);
  bytesToPrint[4] = tm1637_getFontChar(showbuffer[3 + bufoffset], fontset);

  tm1637_i2cWrite_ack(bytesToPrint, 5);
}

void P073_data_struct::tm1637_SwapDigitInBuffer(uint8_t startPos) {
  std::swap(showbuffer[2 + startPos],  showbuffer[0 + startPos]);
  std::swap(showbuffer[3 + startPos],  showbuffer[5 + startPos]);

  std::swap(showperiods[2 + startPos], showperiods[0 + startPos]);
  std::swap(showperiods[3 + startPos], showperiods[5 + startPos]);

  if (dotpos > -1) {
    const uint8_t dotPositionSwap[] = { 0, 1, 4, 3, 2, 7, 6, 5, 8 };

    dotpos = dotPositionSwap[dotpos];
  }
}

void P073_data_struct::tm1637_ShowBuffer(uint8_t firstPos,
                                         uint8_t lastPos,
                                         bool    useBinaryData) {
  uint8_t bytesToPrint[8]{};

  bytesToPrint[0] = 0xC0;
  uint8_t length = 1;

  if (dotpos > -1) {
    showperiods[dotpos] = true;
  }

  uint8_t p073_datashowpos1;

  for (int i = firstPos; i < lastPos; ++i) {
    if (useBinaryData) {
      bytesToPrint[length] = showbuffer[i];
    } else {
      p073_datashowpos1 = tm1637_separator(
        tm1637_getFontChar(showbuffer[i], fontset),
        showperiods[i]);
      bytesToPrint[length] = p073_datashowpos1;
    }
    length++;
  }
  # ifdef P073_DEBUG
  addLog(LOG_LEVEL_INFO, strformat(F("TM1673: Write bytes: %d buffer %d to %d"), length, firstPos, lastPos));
  # endif // ifdef P073_DEBUG
  tm1637_i2cWrite_ack(bytesToPrint, length);
}

#endif // ifdef USES_P073
