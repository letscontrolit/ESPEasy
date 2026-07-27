#include "../PluginStructs/P073_data_struct.h"

#ifdef USES_P073
# include <GPIO_Direct_Access.h>

// ====================================
// ---- MAX7219 specific functions ----
// ====================================

# define OP_DECODEMODE   9
# define OP_INTENSITY   10
# define OP_SCANLIMIT   11
# define OP_SHUTDOWN    12
# define OP_DISPLAYTEST 15

void P073_data_struct::max7219_spiTransfer(ESPEASY_VOLATILE(uint8_t) opcode,
                                           ESPEASY_VOLATILE(uint8_t) data) {
  spidata[1] = opcode;
  spidata[0] = data;
  DIRECT_pinWrite(pin3, LOW);
  DIRECT_shiftOut(pin1, pin2, MSBFIRST, spidata[1]);
  DIRECT_shiftOut(pin1, pin2, MSBFIRST, spidata[0]);
  DIRECT_pinWrite(pin3, HIGH);
}

void P073_data_struct::max7219_ClearDisplay() {
  for (int i = 0; i < 8; i++) {
    max7219_spiTransfer(i + 1, 0);
  }
}

void P073_data_struct::max7219_SetPowerBrightness(uint8_t brightlvl,
                                                  bool    poweron) {
  max7219_spiTransfer(OP_INTENSITY, brightlvl);
  max7219_spiTransfer(OP_SHUTDOWN,  poweron ? 1 : 0);
}

void P073_data_struct::max7219_SetDigit(int     dgtpos,
                                        uint8_t dgtvalue,
                                        bool    showdot,
                                        bool    binaryData) {
  uint8_t p073_tempvalue;

  if (binaryData) {
    p073_tempvalue = dgtvalue; // Overwrite if binary data
  } else
  {
    p073_tempvalue = P073_getFontChar(dgtvalue, fontset);

    if (showdot) {
      p073_tempvalue |= 0b10000000;
    }
  }
  max7219_spiTransfer(dgtpos + 1, p073_tempvalue);
}

void P073_data_struct::max7219_InitDisplay() {
  pinMode(pin1, OUTPUT); // Use Arduino pin initialization as some ESPs don't properly set up their pins with DIRECT_GPIO_OUTPUT
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);
  DIRECT_pinWrite(pin3, HIGH);
  max7219_spiTransfer(OP_DISPLAYTEST, 0);
  max7219_spiTransfer(OP_SCANLIMIT,   7); // scanlimit setup to max at Init
  max7219_spiTransfer(OP_DECODEMODE,  0);
  max7219_ClearDisplay();
  max7219_SetPowerBrightness(0, false);
}

void P073_data_struct::max7219_ShowTime(bool sep) {
  const uint8_t idx_list[] = { 7, 6, 4, 3, 1, 0 }; // Digits in reversed order, as the loop is backward

  for (int8_t i = 5; i >= 0; --i) {
    max7219_SetDigit(idx_list[i], showbuffer[i], false);
  }

  const uint8_t sepChar = P073_mapCharToFontPosition(sep ? '-' : ' ', fontset);

  max7219_SetDigit(2, sepChar, false);
  max7219_SetDigit(5, sepChar, false);
}

void P073_data_struct::max7219_ShowTemp(int8_t firstDot,
                                        int8_t secondDot) {
  max7219_SetDigit(0, 10, false);

  if (firstDot  > -1) { showperiods[firstDot] = true; }

  if (secondDot > -1) { showperiods[secondDot] = true; }

  const int alignRight = rightAlignTempMAX7219 ? 0 : 1;

  for (int i = alignRight; i < 8; ++i) {
    const int bufIndex = (7 + alignRight) - i;

    if (bufIndex < 8) {
      max7219_SetDigit(i,
                       showbuffer[bufIndex],
                       showperiods[bufIndex]);
    }
  }
}

void P073_data_struct::max7219_ShowDate() {
  const uint8_t dotflags[8] = { false, true, false, true, false, false, false, false };

  for (int i = 0; i < 8; ++i) {
    max7219_SetDigit(i,
                     showbuffer[7 - i],
                     dotflags[7 - i]);
  }
}

void P073_data_struct::max7219_ShowBuffer() {
  if (dotpos > -1) {
    showperiods[dotpos] = true;
  }

  for (int i = 0; i < 8; i++) {
    max7219_SetDigit(i,
                     showbuffer[7 - i],
                     showperiods[7 - i]
                     # if P073_7DBIN_COMMAND
                     , binaryData
                     # endif // if P073_7DBIN_COMMAND
                     );
  }
}

#endif // ifdef USES_P073
