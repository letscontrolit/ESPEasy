#include "../PluginStructs/P148_data_struct.h"

#ifdef USES_P148


# include "../Helpers/StringConverter.h"

# define TM1621_PULSE_WIDTH   10   // microseconds (Sonoff = 100)

// Commands
# define TM1621_SYS_DIS       0x00 // 0b00000000 = Turn off system oscillator and LCD bias generator
# define TM1621_SYS_EN        0x01 // 0b00000001 = Turn on the system oscillator
# define TM1621_LCD_OFF       0x02 // 0b00000010 = disables the LCD bias generator
# define TM1621_LCD_ON        0x03 // 0b00000011 = enables the LCD bias generator
# define TM1621_TIMER_DIS     0x04 // 0b00000100 = disable timer
# define TM1621_WDT_DIS       0x05 // 0b00000101 = disable WDT
# define TM1621_TIMER_EN      0x06 // 0b00000110 = enable timer
# define TM1621_WDT_EN        0x07 // 0b00000111 = enable WDT
# define TM1621_TONE_OFF      0x08 // 0b00001000 = turn off sound output
# define TM1621_TONE_ON       0x09 // 0b00001001 = turn on sound output
# define TM1621_CLR_TIMER     0x0C // 0b000011XX = clear timer
# define TM1621_CLR_WDT       0x0E // 0b0000111X = clear WDT status

# define TM1621_BIAS          0x29 // 0b00101001 = LCD 1/3 bias 4 commons option
                                   // 0b0010abXc =>
                                   // c=0: optional 1/2 bias
                                   // c=1: optional 1/3 bias
                                   // ab=00: Optional 2 segment common ports
                                   // ab=01: Optional 3 segment common ports
                                   // ab=10: optional 4 segment common ports
                                   // N.B. 1/3 bias only possible with 1:3 and 1:4 mux ratio (segment common ports)
# define TM1621_IRQ_DIS       0x80 // 0b100x0xxx

// FIXME TD-er: Must make the contrast/bias configurable
// Commands sent during init.
const uint8_t tm1621_commands[] =
{ TM1621_SYS_EN, TM1621_LCD_ON, TM1621_BIAS, TM1621_TIMER_DIS, TM1621_WDT_DIS, TM1621_TONE_OFF, TM1621_IRQ_DIS };


uint8_t P148_data_struct::TM1621GetFontCharacter(char character, bool firstrow) {
  // Uncrustify may mangle the readability of the fonts
  //
  // *INDENT-OFF*

  // HEX bit values per segment
  // Row 1              Row 2
  //        1                  80
  //      -----              -----
  //     |     |            |     |
  //    2|     |10        40|     |4
  //     |     |            |     |
  //      --20-              --2--
  //     |     |            |     |
  //    4|     |40        20|     |1
  //     |     |            |     |
  //      -----              -----
  //        8                  10

  // TM1621 Font
  //                                                  0     1     2     3     4     5     6     7     8     9     -  
  static const uint8_t tm1621_digit_row[2][11] = { { 0x5F, 0x50, 0x3D, 0x79, 0x72, 0x6B, 0x6F, 0x51, 0x7F, 0x7B, 0x20 },
                                                   { 0xF5, 0x05, 0xB6, 0x97, 0x47, 0xD3, 0xF3, 0x85, 0xF7, 0xD7, 0x02 } };
  //                                                 'A'   'b'   'C'   'd'   'E'   'F'   'G'   'H'   'i'   'J'   'K'   'L'   'M'   'n'   'o'   'P'   'q'   'r'   'S'   't'   'U'   'v'   'W'   'X'   'Y'   'Z'   '?'
  static const uint8_t tm1621_char_row[2][27] = { { 0x77, 0x6E, 0x0F, 0x7C, 0x2F, 0x27, 0x4f, 0x76, 0x40, 0x5C, 0x00, 0x0E, 0x00, 0x64, 0x6C, 0x37, 0x73, 0x24, 0x6B, 0x2E, 0x5E, 0x4C, 0x00, 0x00, 0x7A, 0x00, 0x35 },
                                                  { 0xE7, 0x73, 0xF0, 0x37, 0xF2, 0xE2, 0xF1, 0x67, 0x01, 0x35, 0x00, 0x70, 0x00, 0x23, 0x33, 0xE6, 0xC7, 0x22, 0xD3, 0x72, 0x75, 0x31, 0x00, 0x00, 0x57, 0x00, 0xA6 } };
 // *INDENT-ON*

  const char c       = toLowerCase(character);
  const uint32_t row = firstrow ? 0 : 1;

  if (isdigit(c) || (c == '-')) {
    // Part of a number
    if (c == '-') {
      return tm1621_digit_row[row][10];
    } else {
      return tm1621_digit_row[row][c - '0'];
    }
  }

  // String
  if (c == '?') { return tm1621_char_row[row][26]; }

  if (('a' <= c) && (c <= 'z')) {
    return tm1621_char_row[row][c - 'a'];
  }

  // Return a 'space'
  return 0u;
}

// Do not change the order of these as it is stored.
union MonitorTaskValue_conversion {
  struct {
    uint16_t TaskIndex : 9;
    uint16_t unit      : 3;
    uint16_t taskVar   : 3;
    uint16_t showname  : 1;
  };
  int16_t pconfigval;
};

P148_data_struct::MonitorTaskValue_t::MonitorTaskValue_t(int16_t pconfigvalue) {
  MonitorTaskValue_conversion conv;

  conv.pconfigval = pconfigvalue;
  TaskIndex       = conv.TaskIndex;
  taskVar         = conv.taskVar;
  unit            = static_cast<Tm1621UnitOfMeasure>(conv.unit);
  showname        = conv.showname;
}

int16_t P148_data_struct::MonitorTaskValue_t::getPconfigValue() const {
  MonitorTaskValue_conversion conv;

  conv.TaskIndex = TaskIndex;
  conv.taskVar   = taskVar;
  conv.unit      = static_cast<int>(unit);
  conv.showname  = showname;
  return conv.pconfigval;
}

void P148_data_struct::MonitorTaskValue_t::webformLoad(int index) const {
  const bool firstrow = (index % 2) == 0;

  {
    String label = concat(F("Page "), index / 2);
    label += firstrow ? F(" Top Row") : F(" Bottom Row");
    addTableSeparator(label, 2, 2);
  }

  addRowLabel(F("Task"));
  addTaskSelect(concat(F("ptask"), index), TaskIndex);

  if (validTaskIndex(TaskIndex)) {
    addRowLabel(F("Value"));
    addTaskValueSelect(concat(F("pvalue"), index), taskVar, TaskIndex);

    addFormCheckBox(F("Show Var Name"), concat(F("pshowname"), index), showname);

    if (firstrow)
    {
      const __FlashStringHelper *options[] = {
        F("None"),
        F("Celsius"),
        F("Fahrenheit"),
        F("V & A"),
        F("kWh & W")
      };
      int optionValues[] {
        static_cast<int>(P148_data_struct::Tm1621UnitOfMeasure::None),
        static_cast<int>(P148_data_struct::Tm1621UnitOfMeasure::Celsius),
        static_cast<int>(P148_data_struct::Tm1621UnitOfMeasure::Fahrenheit),
        static_cast<int>(P148_data_struct::Tm1621UnitOfMeasure::Volt_Amp),
        static_cast<int>(P148_data_struct::Tm1621UnitOfMeasure::kWh_Watt)
      };
      constexpr size_t nrElements = sizeof(optionValues) / sizeof(optionValues[0]);

      addFormSelector(
        F("Unit Symbols"), concat(F("punit"), index),
        nrElements, options, optionValues, static_cast<int>(unit));
    } else {
      const __FlashStringHelper *options[] = {
        F("None"),
        F("%RH (humidity)"),
        F("V & A"),
        F("kWh & W")
      };
      int optionValues[] {
        static_cast<int>(P148_data_struct::Tm1621UnitOfMeasure::None),
        static_cast<int>(P148_data_struct::Tm1621UnitOfMeasure::Humidity),
        static_cast<int>(P148_data_struct::Tm1621UnitOfMeasure::Volt_Amp),
        static_cast<int>(P148_data_struct::Tm1621UnitOfMeasure::kWh_Watt)
      };
      constexpr size_t nrElements = sizeof(optionValues) / sizeof(optionValues[0]);

      addFormSelector(
        F("Unit Symbols"), concat(F("punit"), index),
        nrElements, options, optionValues, static_cast<int>(unit));
    }
  } else {
    addFormCheckBox(F("Clear Line"), concat(F("pshowname"), index), showname);
  }
}

int16_t P148_data_struct::MonitorTaskValue_t::webformSave(int index) {
  TaskIndex = getFormItemInt(concat(F("ptask"), index), INVALID_TASK_INDEX);
  taskVar   = getFormItemInt(concat(F("pvalue"), index), INVALID_TASKVAR_INDEX);
  unit      =
    static_cast<P148_data_struct::Tm1621UnitOfMeasure>(
      getFormItemInt(concat(F("punit"), index), static_cast<int>(P148_data_struct::Tm1621UnitOfMeasure::None))
      );
  showname = isFormItemChecked(concat(F("pshowname"), index));

  return getPconfigValue();
}

bool P148_data_struct::MonitorTaskValue_t::isValid() const {
  return validTaskIndex(TaskIndex) && validTaskVarIndex(taskVar);
}

String P148_data_struct::MonitorTaskValue_t::formatTaskValue(bool& writeToDisplay) const {
  if (!isValid()) {
    writeToDisplay = showname;
    return EMPTY_STRING;
  }
  writeToDisplay = true;

  if (showname) {
    return getTaskValueName(TaskIndex, taskVar);
  }
  return formatUserVarNoCheck(TaskIndex, taskVar);
}

bool P148_data_struct::Tm1621_t::isValid() const {
  // FIXME TD-er: Must check if the selected pins also are usable
  return pin_da != -1 &&
         pin_wr != -1 &&
         pin_rd != -1 &&
         pin_cs != -1;
}

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
}

bool P148_data_struct::init() {
  if (!Tm1621.isValid()) {
    return false;
  }
  TM1621Init();
  return true;
}

/*********************************************************************************************/

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

  constexpr uint32_t nr_commands = sizeof(tm1621_commands) / sizeof(tm1621_commands[0]);

  for (uint32_t command = 0; command < nr_commands; command++) {
    TM1621SendCmnd(tm1621_commands[command]);
  }

  // Clear entire display buffer
  TM1621SendAddress(0x00);

  for (uint32_t segment = 0; segment < 16; segment++) {
    TM1621SendCommon(0);
  }
  TM1621StopSequence();

  snprintf_P(Tm1621.row[0], sizeof(Tm1621.row[0]), PSTR("----"));
  snprintf_P(Tm1621.row[1], sizeof(Tm1621.row[1]), PSTR("----"));
  TM1621SendRows();
}

void P148_data_struct::TM1621WriteBit(bool value) const {
  digitalWrite(Tm1621.pin_wr, 0);             // Start write sequence
  digitalWrite(Tm1621.pin_da, value ? 1 : 0); // Set data
  delayMicroseconds(TM1621_PULSE_WIDTH);
  digitalWrite(Tm1621.pin_wr, 1);             // On the rising edge of the /WR signal,
                                              // the data on the DATA line is written to the TM1621
  delayMicroseconds(TM1621_PULSE_WIDTH);
}

void P148_data_struct::TM1621StartSequence() const {
  digitalWrite(Tm1621.pin_cs, 0); // Start command sequence
  delayMicroseconds(TM1621_PULSE_WIDTH / 2);
}

void P148_data_struct::TM1621StopSequence() const {
  digitalWrite(Tm1621.pin_cs, 1); // Stop command sequence
  delayMicroseconds(TM1621_PULSE_WIDTH / 2);
  digitalWrite(Tm1621.pin_da, 1); // Reset data
}

void P148_data_struct::TM1621SendCmnd(uint16_t command) const {
  uint16_t full_command = (0x0400 | command) << 5; // 0b100cccccccc00000

  TM1621StartSequence();

  for (uint32_t i = 0; i < 12; i++) {
    TM1621WriteBit(full_command & 0x8000);
    full_command <<= 1;
  }
  TM1621StopSequence();
}

void P148_data_struct::TM1621SendAddress(uint16_t address) const {
  uint16_t full_address = (address | 0x0140) << 7; // 0b101aaaaaa0000000

  TM1621StartSequence();

  for (uint32_t i = 0; i < 9; i++) {
    TM1621WriteBit(full_address & 0x8000);
    full_address <<= 1;
  }
}

void P148_data_struct::TM1621SendCommon(uint8_t common) const {
  for (uint32_t i = 0; i < 8; i++) {
    TM1621WriteBit(common & 1);
    common >>= 1;
  }
}

void P148_data_struct::TM1621WritePixelBuffer(const uint8_t *buf, size_t size, uint16_t address) const {
  TM1621SendAddress(address);

  for (uint32_t i = 0; i < size; i++) {
    TM1621SendCommon(buf[i]);
  }
  TM1621StopSequence();
}

void P148_data_struct::TM1621SendRows() const {
  // Tm1621.row[x] = "text", "----", "    " or a number with one decimal like "0.4", "237.5", "123456.7"
  // "123456.7" will be shown as "9999" being a four digit overflow

  uint8_t buffer[8] = { 0 }; // TM1621 16-segment 4-bit common buffer

  for (uint32_t j = 0; j < 2; j++) {
    const bool firstrow = 0 == j;

    float value = 0.0;

    if (validFloatFromString(Tm1621.row[j], value)) {
      // 0.4V => "  04", 0.0A => "  ", 1234.5V => "1234"

      // Check to see if the original string had a dot.
      bool hadDot = false;

      for (size_t i = 0; i < 4 && !hadDot; ++i) {
        if (Tm1621.row[j][i] == '.') { hadDot = true; }
      }

      char row[4]{};

      if (value > 9999.0f) { value = 9999.0f; }

      if (value < -999.0f) { value = -999.0f; }
      bool dot = false;

      if ((-99.9f < value) && (value < 999.9f)) {
        if (hadDot) {
          dot    = true;
          value *= 10.0f;
        }
      }
      const String value_str(static_cast<int>(value));
      const size_t strlen = value_str.length();
      size_t pos          = 0;

      for (size_t i = 4 - strlen; i < 4 && pos < strlen; ++i, ++pos) {
        row[i] = value_str[pos];
      }

      for (uint32_t i = 0; i < 4; i++) {
        buffer[bufferIndex(firstrow, i)] = TM1621GetFontCharacter(row[i], firstrow);
      }

      if (dot) {
        if (firstrow) {
          buffer[2] |= 0x80; // Row 1 decimal point
        } else {
          buffer[5] |= 0x08; // Row 2 decimal point
        }
      }
    } else {
      for (uint32_t i = 0; i < 4; i++) {
        buffer[bufferIndex(firstrow, i)] = TM1621GetFontCharacter(Tm1621.row[j][i], firstrow);
      }
    }
  }

  if (Tm1621.fahrenheit) { buffer[1] |= 0x80; }

  if (Tm1621.celsius) { buffer[3] |= 0x80; }

  if (Tm1621.kwh) { buffer[4] |= 0x08; }

  if (Tm1621.humidity) { buffer[6] |= 0x08; }

  if (Tm1621.voltage) { buffer[7] |= 0x08; }

  TM1621WritePixelBuffer(buffer, 8, 0x10); // Sonoff only uses the upper 16 Segments
}

void P148_data_struct::showPage() {
  if (pagenr >= 3) {
    pagenr = 0;
  }

  // Skip pages which do not change the display.
  bool somethingWritten = false;

  while (!somethingWritten && pagenr < 3) {
    for (int row = 2 * pagenr; row <= (2 * pagenr + 1); ++row) {
      const bool firstrow   = (row % 2) == 0;
      bool   writeToDisplay = true;
      String value          = MonitorTaskValues[row].formatTaskValue(writeToDisplay);

      if (writeToDisplay) {
        somethingWritten = true;
        setUnit(MonitorTaskValues[row].unit, firstrow);
        writeString(firstrow, value);
      }
    }
    ++pagenr;
  }
}

void P148_data_struct::writeString(bool firstrow, const String& str) {
  safe_strncpy(Tm1621.row[firstrow ? 0 : 1], str, sizeof(Tm1621.row[0]));
  TM1621SendRows();
}

void P148_data_struct::writeStrings(const String& str1, const String& str2) {
  safe_strncpy(Tm1621.row[0], str1, sizeof(Tm1621.row[0]));
  safe_strncpy(Tm1621.row[1], str2, sizeof(Tm1621.row[0]));
  TM1621SendRows();
}

void P148_data_struct::writeFloats(float value1, float value2) {
  writeStrings(String(value1, 1), String(value2, 1));
}

void P148_data_struct::writeFloat(bool firstrow, float value) {
  writeString(firstrow, String(value, 1));
}

void P148_data_struct::writeRawData(uint64_t rawdata) const {
  uint8_t buffer[8] = { 0 }; // TM1621 16-segment 4-bit common buffer

  for (uint32_t j = 0; j < 2; j++) {
    for (uint32_t i = 0; i < 4; i++) {
      buffer[bufferIndex((0 == j), i)] = ((rawdata >> 56) & 0xFF);
      rawdata                        <<= 8;
    }
  }

  TM1621WritePixelBuffer(buffer, 8, 0x10); // Sonoff only uses the upper 16 Segments
}

void P148_data_struct::setUnit(Tm1621UnitOfMeasure unit) {
  setUnit(unit, true);
  setUnit(unit, false);
}

void P148_data_struct::setUnit(P148_data_struct::Tm1621UnitOfMeasure unit, bool firstrow) {
  switch (unit) {
    case P148_data_struct::Tm1621UnitOfMeasure::None:

      if (firstrow) {
        Tm1621.celsius    = false;
        Tm1621.fahrenheit = false;
        Tm1621.voltage    = false;
        Tm1621.kwh        = false;
      } else {
        Tm1621.humidity = false;
      }
      break;
    case P148_data_struct::Tm1621UnitOfMeasure::Celsius:

      if (firstrow) {
        Tm1621.celsius    = true;
        Tm1621.fahrenheit = false;
        Tm1621.voltage    = false;
        Tm1621.kwh        = false;
      }
      break;
    case P148_data_struct::Tm1621UnitOfMeasure::Fahrenheit:

      if (firstrow) {
        Tm1621.fahrenheit = true;
        Tm1621.celsius    = false;
        Tm1621.voltage    = false;
        Tm1621.kwh        = false;
      }
      break;
    case P148_data_struct::Tm1621UnitOfMeasure::kWh_Watt:
      Tm1621.kwh = true;

      if (firstrow) {
        Tm1621.celsius    = false;
        Tm1621.fahrenheit = false;
        Tm1621.voltage    = false;
      } else {
        Tm1621.humidity = false;
      }

      break;
    case P148_data_struct::Tm1621UnitOfMeasure::Humidity:

      if (!firstrow) {
        Tm1621.humidity = true;
        Tm1621.voltage  = false;
        Tm1621.kwh      = false;
      }
      break;
    case P148_data_struct::Tm1621UnitOfMeasure::Volt_Amp:
      Tm1621.voltage = true;
      Tm1621.kwh     = false;

      if (firstrow) {
        Tm1621.celsius    = false;
        Tm1621.fahrenheit = false;
      } else {
        Tm1621.humidity = false;
      }
      break;
  }
}

#endif // ifdef USES_P148
