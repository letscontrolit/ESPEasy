#include "../PluginStructs/P073_data_struct.h"

#ifdef USES_P073
# include <GPIO_Direct_Access.h>

P073_MAX7219::P073_MAX7219(struct EventStruct *event) : P073_data_struct(event) {
  //
}

bool P073_MAX7219::init(struct EventStruct *event) {
  if (P073_data_struct::init(event)) {
    initDisplay();
    delay(10); // small poweroff/poweron delay
    setPowerBrightness(brightness, true);

    if (output == P073_DISP_MANUAL) {
      clearDisplay();
    }
    return true;
  }
  return false;
}

void P073_MAX7219::showTime(bool sep) { max7219_ShowTime(sep); }

void P073_MAX7219::showDate()         { max7219_ShowDate(); }

void P073_MAX7219::showNumber()       { toOutputBuffer(); }

void P073_MAX7219::showTemperature(int8_t firstDot,
                                   int8_t secondDot) { max7219_ShowTemp(firstDot, secondDot); }

void P073_MAX7219::showBuffer() {} // n.a.

// ====================================
// ---- MAX7219 specific functions ----
// ====================================

# define OP_DECODEMODE   9
# define OP_INTENSITY   10
# define OP_SCANLIMIT   11
# define OP_SHUTDOWN    12
# define OP_DISPLAYTEST 15

void P073_MAX7219::max7219_spiTransfer(ESPEASY_VOLATILE(uint8_t) opcode,
                                       ESPEASY_VOLATILE(uint8_t) data) {
  spidata[0] = opcode;
  spidata[1] = data;
  DIRECT_pinWrite(pin3, LOW);
  DIRECT_shiftOut(pin1, pin2, MSBFIRST, spidata[0]);
  DIRECT_shiftOut(pin1, pin2, MSBFIRST, spidata[1]);
  DIRECT_pinWrite(pin3, HIGH);
}

void P073_MAX7219::clearDisplay() {
  for (int i = 0; i < 8; i++) {
    max7219_spiTransfer(i + 1, 0);
  }
}

void P073_MAX7219::setPowerBrightness(uint8_t brightlvl,
                                      bool    poweron) {
  max7219_spiTransfer(OP_INTENSITY, brightlvl);
  max7219_spiTransfer(OP_SHUTDOWN,  poweron ? 1 : 0);
}

void P073_MAX7219::setDigit(int     dgtpos,
                            uint8_t dgtvalue,
                            bool    showdot,
                            bool    binaryData) {
  uint8_t data;

  if (binaryData) {
    data = dgtvalue; // Overwrite if binary data
  } else
  {
    data = P073_getFontChar(dgtvalue, fontset);

    if (showdot) {
      data |= 0b10000000;
    }
  }
  max7219_spiTransfer(dgtpos + 1, data);
}

void P073_MAX7219::initDisplay() {
  pinMode(pin1, OUTPUT); // Use Arduino pin initialization as some ESPs don't properly set up their pins with DIRECT_GPIO_OUTPUT
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);
  DIRECT_pinWrite(pin3, HIGH);
  max7219_spiTransfer(OP_DISPLAYTEST, 0);
  max7219_spiTransfer(OP_SCANLIMIT,   7); // scanlimit setup to max at Init
  max7219_spiTransfer(OP_DECODEMODE,  0);
  clearDisplay();
  setPowerBrightness(0, false);
}

void P073_MAX7219::max7219_ShowTime(bool sep) {
  const uint8_t idx_list[] = { 7, 6, 4, 3, 1, 0 }; // Digits in reversed order, as the loop is backward

  for (int8_t i = 5; i >= 0; --i) {
    setDigit(idx_list[i], showbuffer[i], false);
  }

  const uint8_t sepChar = P073_mapCharToFontPosition(sep ? '-' : ' ', fontset);

  setDigit(2, sepChar, false);
  setDigit(5, sepChar, false);
}

void P073_MAX7219::max7219_ShowTemp(int8_t firstDot,
                                    int8_t secondDot) {
  setDigit(0, 10, false); // FIXME Not sure about doing this every time...

  if (firstDot  > -1) { showperiods[firstDot] = true; }

  if (secondDot > -1) { showperiods[secondDot] = true; }

  digitOffset = rightAlignTempMAX7219 ? 0 : 1;

  toOutputBuffer();
}

void P073_MAX7219::max7219_ShowDate() {
  // const uint8_t dotflags[8] = { false, true, false, true, false, false, false, false };

  showperiods[1] = true;
  showperiods[3] = true;

  toOutputBuffer();
}

void P073_MAX7219::toOutputBuffer() {
  for (uint8_t i = digitOffset; i < 8; i++) {
    const uint8_t bufIndex = (7 + digitOffset) - i;
    setDigit(i,
             showbuffer[bufIndex],
             showperiods[bufIndex]
             # if P073_7DBIN_COMMAND
             , binaryData
             # endif // if P073_7DBIN_COMMAND
             );
  }
  digitOffset = 0; // Reset every time.
}

#endif // ifdef USES_P073
