#include "../PluginStructs/P073_data_struct.h"

#ifdef USES_P073
# if P073_USE_74HC595
#  include <GPIO_Direct_Access.h>

bool P073_data_struct::is74HC595Multiplex() { return P073_74HC595_2_8DGT == displayModel && P073_HC595_MULTIPLEX; }

// ====================================
// ---- 74HC595 specific functions ----
// ====================================

void P073_data_struct::hc595_ShowBuffer() {
  #  if P073_USE_74HCMULTIPLEX
  const uint8_t hc595digit4[] = {
    0b00001000, // left segment
    0b00000100,
    0b00000010,
    0b00000001, // right segment
  };

  const uint8_t hc595digit8[] = {
    0b00010000, // left segment
    0b00100000,
    0b01000000,
    0b10000000,
    0b00000001,
    0b00000010,
    0b00000100,
    0b00001000, // right segment
  };
  #  endif // if P073_USE_74HCMULTIPLEX

  int8_t i    = digits - 1;
  int8_t stop = -1;
  int8_t incr = -1;

  #  if P073_USE_74HCMULTIPLEX

  if (P073_HC595_MULTIPLEX) {
    i    =  dspDgt;
    stop =  dspDgt + 1;
    incr =  1;
  }
  #  endif // if P073_USE_74HCMULTIPLEX

  for (; i != stop && i >= 0; i += incr) {
    DIRECT_shiftOut(pin1, pin2, MSBFIRST, outputbuffer[i]); // Digit data out

    // 2, 3 and some 4 digit modules use sequential digit values (in reversed order)
    // 4, 6 and 8 digit modules use multiplexing in LTR order
    #  if P073_USE_74HCMULTIPLEX
    uint8_t digit = 0xFF;

    if (P073_HC595_MULTIPLEX) {
      if (4 == digits) {
        digit = hc595digit4[i];
      } else
      if (6 == digits) {
        digit = hc595digit8[i + (i > 2 ? 1 : 0)];
      } else
      if (8 == digits) {
        digit = hc595digit8[i];
      }
    }

    if (digit != 0xFF) { // Select multiplexer digit, 0xFF is invalid
      DIRECT_shiftOut(pin1, pin2, MSBFIRST, digit);
    }
    #  endif // if P073_USE_74HCMULTIPLEX

    if ((P073_HC595_SEQUENTIAL && (0 == i)) || P073_HC595_MULTIPLEX) {
      DIRECT_pinWrite(pin3, LOW); // Clock data
      DIRECT_pinWrite(pin3, HIGH);
    }
  }

  if (i >= digits) {
    dspDgt = 0;
  } else {
    dspDgt = i;
  }

  #  ifdef P073_DEBUG

  // TODO disable log
  // if ((counter50 % 200 == 0) || P073_HC595_SEQUENTIAL) {
  //   addLog(LOG_LEVEL_INFO, strformat(F("P073: hc595_ShowBuffer (end) dgt:%d i:%d stop:%d incr:%d pin1: %d pin2: %d pin3: %d"),
  //                                    digits, i, stop, incr, pin1, pin2, pin3));
  // }
  #  endif // ifdef P073_DEBUG
}

void P073_data_struct::hc595_ToOutputBuffer() {
  for (uint8_t i = 0; i < 8; ++i) {
    uint8_t value;

    // 74HC595 uses inverted data, compared to MAX7219/TM1637
    value = ~P073_getFontChar(showbuffer[i], fontset);

    if (showperiods[i]) {
      value &= 0x7F;
    }
    outputbuffer[i] = P073_revert7bits(value); // Rotate bits 6..0
  }

  if (hc595_Sequential()) {                    // Sequential displays don't need continuous refreshing
    hc595_ShowBuffer();
  }
}

void P073_data_struct::hc595_ShiftinView() {
  if (digits < 8) {
    uint8_t n = 0;

    for (uint8_t i = 8 - digits; i < 8; ++i, ++n) {
      showbuffer[n] = showbuffer[i];
    }
  }
}

void P073_data_struct::hc595_InitDisplay() {
  pinMode(pin1, OUTPUT); // Use Arduino pin initialization as some ESPs don't properly set up their pins with DIRECT_GPIO_OUTPUT
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);
  DIRECT_pinWrite(pin3, HIGH);
}

# endif // if P073_USE_74HC595

#endif // ifdef USES_P073
