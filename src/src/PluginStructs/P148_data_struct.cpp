#include "../PluginStructs/P148_data_struct.h"

#ifdef USES_P148


# define TM1621_PULSE_WIDTH   10   // microseconds (Sonoff = 100)

// Commands sent during init
# define TM1621_SYS_EN        0x01 // 0b00000001
# define TM1621_LCD_ON        0x03 // 0b00000011
# define TM1621_TIMER_DIS     0x04 // 0b00000100
# define TM1621_WDT_DIS       0x05 // 0b00000101
# define TM1621_TONE_OFF      0x08 // 0b00001000
# define TM1621_BIAS          0x29 // 0b00101001 = LCD 1/3 bias 4 commons option
# define TM1621_IRQ_DIS       0x80 // 0b100x0xxx

// FIXME TD-er: Must make the contrast/bias configurable
const uint8_t tm1621_commands[] =
{ TM1621_SYS_EN, TM1621_LCD_ON, TM1621_BIAS, TM1621_TIMER_DIS, TM1621_WDT_DIS, TM1621_TONE_OFF, TM1621_IRQ_DIS };

// TM1621 Font
//                                          0     1     2     3     4     5     6     7     8     9     -     off
const uint8_t tm1621_digit_row[2][12] = { { 0x5F, 0x50, 0x3D, 0x79, 0x72, 0x6B, 0x6F, 0x51, 0x7F, 0x7B, 0x20, 0x00 },
  { 0xF5, 0x05, 0xB6, 0x97, 0x47, 0xD3, 0xF3, 0x85, 0xF7, 0xD7, 0x02, 0x00 } };

P148_data_struct::P148_data_struct(const Tm1621_t& config) : Tm1621(config) {
  if (Tm1621.isValid()) {
    pinMode(Tm1621.pin_da, OUTPUT);
    digitalWrite(Tm1621.pin_da, 1);
    pinMode(Tm1621.pin_cs, OUTPUT);
    digitalWrite(Tm1621.pin_cs, 1);
    pinMode(Tm1621.pin_rd, OUTPUT);
    digitalWrite(Tm1621.pin_rd, 1);
    pinMode(Tm1621.pin_wr, OUTPUT);
    digitalWrite(Tm1621.pin_wr, 1);
  }

  // FIXME TD-er: Still needed?
  Tm1621.state = 200;
}

bool P148_data_struct::init() {
  if (!Tm1621.isValid()) {
    return false;
  }
  TM1621Init();
  return true;
}

/*********************************************************************************************/

void P148_data_struct::TM1621StopSequence() {
  digitalWrite(Tm1621.pin_cs, 1); // Stop command sequence
  delayMicroseconds(TM1621_PULSE_WIDTH / 2);
  digitalWrite(Tm1621.pin_da, 1); // Reset data
}

void P148_data_struct::TM1621SendCmnd(uint16_t command) {
  uint16_t full_command = (0x0400 | command) << 5; // 0b100cccccccc00000

  digitalWrite(Tm1621.pin_cs, 0);                  // Start command sequence
  delayMicroseconds(TM1621_PULSE_WIDTH / 2);

  for (uint32_t i = 0; i < 12; i++) {
    digitalWrite(Tm1621.pin_wr, 0);                               // Start write sequence
    digitalWrite(Tm1621.pin_da, (full_command & 0x8000) ? 1 : 0); // Set data
    delayMicroseconds(TM1621_PULSE_WIDTH);
    digitalWrite(Tm1621.pin_wr, 1);                               // Read data
    delayMicroseconds(TM1621_PULSE_WIDTH);
    full_command <<= 1;
  }
  TM1621StopSequence();
}

void P148_data_struct::TM1621SendAddress(uint16_t address) {
  uint16_t full_address = (address | 0x0140) << 7; // 0b101aaaaaa0000000

  digitalWrite(Tm1621.pin_cs, 0);                  // Start command sequence
  delayMicroseconds(TM1621_PULSE_WIDTH / 2);

  for (uint32_t i = 0; i < 9; i++) {
    digitalWrite(Tm1621.pin_wr, 0);                               // Start write sequence
    digitalWrite(Tm1621.pin_da, (full_address & 0x8000) ? 1 : 0); // Set data
    delayMicroseconds(TM1621_PULSE_WIDTH);
    digitalWrite(Tm1621.pin_wr, 1);                               // Read data
    delayMicroseconds(TM1621_PULSE_WIDTH);
    full_address <<= 1;
  }
}

void P148_data_struct::TM1621SendCommon(uint8_t common) {
  for (uint32_t i = 0; i < 8; i++) {
    digitalWrite(Tm1621.pin_wr, 0);          // Start write sequence
    digitalWrite(Tm1621.pin_da, common & 1); // Set data
    delayMicroseconds(TM1621_PULSE_WIDTH);
    digitalWrite(Tm1621.pin_wr, 1);          // Read data
    delayMicroseconds(TM1621_PULSE_WIDTH);
    common >>= 1;
  }
}

void P148_data_struct::TM1621SendRows() {
  // Tm1621.row[x] = "text", "----", "    " or a number with one decimal like "0.4", "237.5", "123456.7"
  // "123456.7" will be shown as "9999" being a four digit overflow

  uint8_t buffer[8] = { 0 }; // TM1621 16-segment 4-bit common buffer
  char    row[4];

  for (uint32_t j = 0; j < 2; j++) {
    // 0.4V => "  04", 0.0A => "  ", 1234.5V => "1234"
    uint32_t len = strlen(Tm1621.row[j]);
    char    *dp  = nullptr; // Expect number larger than "123"
    int row_idx  = len - 3; // "1234.5"

    if (len <= 5) {         // "----", "    ", "0.4", "237.5"
      dp      = strchr(Tm1621.row[j], '.');
      row_idx = len - 1;
    }
    else if (len > 6) { // "12345.6"
      snprintf_P(Tm1621.row[j], sizeof(Tm1621.row[j]), PSTR("9999"));
      row_idx = 3;
    }

    for (int i = 3; i >= 0; --i) {
      row[i] = (row_idx >= 0) ? Tm1621.row[j][row_idx--] : ' ';

      if ((i == 3) && (row_idx >= 0) && dp) {
        // Skip the '.'
        row_idx--;
      }
    }

    //    AddLog(LOG_LEVEL_DEBUG, PSTR("TM1: Dump%d %4_H"), j +1, row);

    char command[10];

    for (uint32_t i = 0; i < 4; i++) {
      int index = 11; // Empty

      if (row[i] == '-') {
        index = 10;
      } else if (isDigit(row[i])) {
        index = row[i] - '0';
      }

      if (-1 == index) { index = 11; }
      uint32_t bidx = (0 == j) ? i : 7 - i;
      buffer[bidx] = tm1621_digit_row[j][index];
    }

    if (dp) {
      if (0 == j) {
        buffer[2] |= 0x80; // Row 1 decimal point
      } else {
        buffer[5] |= 0x08; // Row 2 decimal point
      }
    }
  }

  if (Tm1621.fahrenheit) { buffer[1] |= 0x80; }

  if (Tm1621.celsius) { buffer[3] |= 0x80; }

  if (Tm1621.kwh) { buffer[4] |= 0x08; }

  if (Tm1621.humidity) { buffer[6] |= 0x08; }

  if (Tm1621.voltage) { buffer[7] |= 0x08; }

  //  AddLog(LOG_LEVEL_DEBUG, PSTR("TM1: Dump3 %8_H"), buffer);

  TM1621SendAddress(0x10); // Sonoff only uses the upper 16 Segments

  for (uint32_t i = 0; i < 8; i++) {
    TM1621SendCommon(buffer[i]);
  }
  TM1621StopSequence();
}

void P148_data_struct::TM1621Init() {
  digitalWrite(Tm1621.pin_cs, 0);
  delayMicroseconds(80);
  digitalWrite(Tm1621.pin_rd, 0);
  delayMicroseconds(15);
  digitalWrite(Tm1621.pin_wr, 0);
  delayMicroseconds(25);
  digitalWrite(Tm1621.pin_da, 0);
  delayMicroseconds(TM1621_PULSE_WIDTH);
  digitalWrite(Tm1621.pin_da, 1);

  for (uint32_t command = 0; command < sizeof(tm1621_commands); command++) {
    TM1621SendCmnd(tm1621_commands[command]);
  }

  TM1621SendAddress(0x00);

  for (uint32_t segment = 0; segment < 16; segment++) {
    TM1621SendCommon(0);
  }
  TM1621StopSequence();

  snprintf_P(Tm1621.row[0], sizeof(Tm1621.row[0]), PSTR("----"));
  snprintf_P(Tm1621.row[1], sizeof(Tm1621.row[1]), PSTR("----"));
  TM1621SendRows();
}

#endif // ifdef USES_P148
