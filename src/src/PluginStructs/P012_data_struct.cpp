#include "../PluginStructs/P012_data_struct.h"

// Needed also here for PlatformIO's library finder as the .h file
// is in a directory which is excluded in the src_filter
#include <LiquidCrystal_I2C.h>

#ifdef USES_P012

P012_data_struct::P012_data_struct(uint8_t addr,
                                   uint8_t lcd_size,
                                   uint8_t mode,
                                   byte    timer) :
  lcd(addr, 20, 4),
  Plugin_012_mode(mode),
  displayTimer(timer) {
  switch (lcd_size)
  {
    case 1:
      Plugin_012_rows = 2;
      Plugin_012_cols = 16;
      break;
    case 2:
      Plugin_012_rows = 4;
      Plugin_012_cols = 20;
      break;

    default:
      Plugin_012_rows = 2;
      Plugin_012_cols = 16;
      break;
  }


  // Setup LCD display
  lcd.init(); // initialize the lcd
  lcd.backlight();
  lcd.print(F("ESP Easy"));
  createCustomChars();
}

void P012_data_struct::setBacklightTimer(byte timer) {
  displayTimer = timer;
  lcd.backlight();
}

void P012_data_struct::checkTimer() {
  if (displayTimer > 0)
  {
    displayTimer--;

    if (displayTimer == 0) {
      lcd.noBacklight();
    }
  }
}

void P012_data_struct::lcdWrite(const String& text, byte col, byte row) {
  // clear line before writing new string
  if (Plugin_012_mode == 2) {
    lcd.setCursor(col, row);

    for (byte i = col; i < Plugin_012_cols; i++) {
      lcd.print(" ");
    }
  }

  lcd.setCursor(col, row);

  if ((Plugin_012_mode == 1) || (Plugin_012_mode == 2)) {
    lcd.setCursor(col, row);

    for (byte i = 0; i < Plugin_012_cols - col; i++) {
      if (text[i]) {
        lcd.print(text[i]);
      }
    }
  }

  // message exceeding cols will continue to next line
  else {
    // Fix Weird (native) lcd display behaviour that split long string into row 1,3,2,4, instead of 1,2,3,4
    bool stillProcessing = 1;
    byte charCount       = 1;

    while (stillProcessing) {
      if (++col > Plugin_012_cols) { // have we printed 20 characters yet (+1 for the logic)
        row += 1;
        lcd.setCursor(0, row);       // move cursor down
        col = 1;
      }

      // dont print if "lower" than the lcd
      if (row < Plugin_012_rows) {
        lcd.print(text[charCount - 1]);
      }

      if (!text[charCount]) { // no more chars to process?
        stillProcessing = 0;
      }
      charCount += 1;
    }

    // lcd.print(text.c_str());
    // end fix
  }
}

// Perform some specific changes for LCD display
// https://www.letscontrolit.com/forum/viewtopic.php?t=2368
String P012_data_struct::P012_parseTemplate(String& tmpString, byte lineSize) {
  String result            = parseTemplate_padded(tmpString, lineSize);
  const char degree[3]     = { 0xc2, 0xb0, 0 }; // Unicode degree symbol
  const char degree_lcd[2] = { 0xdf, 0 };       // P012_LCD degree symbol

  result.replace(degree, degree_lcd);

  char unicodePrefix = 0xc4;

# ifdef USES_P012_POLISH_CHARS

  if (result.indexOf(unicodePrefix) != -1) {
    const char znak_a_uni[3] = { 0xc4, 0x85, 0 }; // Unicode znak a
    const char znak_a_lcd[2] = { 0x05, 0 };       // P012_LCD znak a
    result.replace(znak_a_uni, znak_a_lcd);

    const char znak_A_uni[3] = { 0xc4, 0x84, 0 }; // Unicode znak A
    result.replace(znak_A_uni, znak_a_lcd);

    const char znak_c_uni[3] = { 0xc4, 0x87, 0 }; // Unicode znak c
    const char znak_c_lcd[2] = { 0x03, 0 };       // P012_LCD znak c
    result.replace(znak_c_uni, znak_c_lcd);

    const char znak_C_uni[3] = { 0xc4, 0x86, 0 }; // Unicode znak C
    result.replace(znak_C_uni, znak_c_lcd);

    const char znak_e_uni[3] = { 0xc4, 0x99, 0 }; // Unicode znak e
    const char znak_e_lcd[2] = { 0x02, 0 };       // P012_LCD znak e
    result.replace(znak_e_uni, znak_e_lcd);

    const char znak_E_uni[3] = { 0xc4, 0x98, 0 }; // Unicode znak E
    result.replace(znak_E_uni, znak_e_lcd);
  }

  unicodePrefix = 0xc5;

  if (result.indexOf(unicodePrefix) != -1) {
    const char znak_l_uni[3] = { 0xc5, 0x82, 0 };  // Unicode znak l
    const char znak_l_lcd[2] = { 0x01, 0 };        // P012_LCD znak l
    result.replace(znak_l_uni, znak_l_lcd);

    const char znak_L_uni[3] = { 0xc5, 0x81, 0 };  // Unicode znak L
    result.replace(znak_L_uni, znak_l_lcd);

    const char znak_n_uni[3] = { 0xc5, 0x84, 0 };  // Unicode znak n
    const char znak_n_lcd[2] = { 0x04, 0 };        // P012_LCD znak n
    result.replace(znak_n_uni, znak_n_lcd);

    const char znak_N_uni[3] = { 0xc5, 0x83, 0 };  // Unicode znak N
    result.replace(znak_N_uni, znak_n_lcd);

    const char znak_s_uni[3] = { 0xc5, 0x9b, 0 };  // Unicode znak s
    const char znak_s_lcd[2] = { 0x06, 0 };        // P012_LCD znak s
    result.replace(znak_s_uni, znak_s_lcd);

    const char znak_S_uni[3] = { 0xc5, 0x9a, 0 };  // Unicode znak S
    result.replace(znak_S_uni, znak_s_lcd);

    const char znak_z1_uni[3] = { 0xc5, 0xba, 0 }; // Unicode znak z z kreska
    const char znak_z1_lcd[2] = { 0x07, 0 };       // P012_LCD znak z z kropka
    result.replace(znak_z1_uni, znak_z1_lcd);

    const char znak_Z1_uni[3] = { 0xc5, 0xb9, 0 }; // Unicode znak Z z kreska
    result.replace(znak_Z1_uni, znak_z1_lcd);

    const char znak_z2_uni[3] = { 0xc5, 0xbc, 0 }; // Unicode znak z z kropka
    const char znak_z2_lcd[2] = { 0x07, 0 };       // P012_LCD znak z z kropka
    result.replace(znak_z2_uni, znak_z2_lcd);

    const char znak_Z2_uni[3] = { 0xc5, 0xbb, 0 }; // Unicode znak Z z kropka
    result.replace(znak_Z2_uni, znak_z2_lcd);
  }

  unicodePrefix = 0xc3;

  if (result.indexOf(unicodePrefix) != -1) {
    const char znak_o_uni[3] = { 0xc3, 0xB3, 0 }; // Unicode znak o
    const char znak_o_lcd[2] = { 0x08, 0 };       // P012_LCD znak o
    result.replace(znak_o_uni, znak_o_lcd);

    const char znak_O_uni[3] = { 0xc3, 0x93, 0 }; // Unicode znak O
    result.replace(znak_O_uni, znak_o_lcd);
  }
# endif // USES_P012_POLISH_CHARS

  unicodePrefix = 0xc3;

  if (result.indexOf(unicodePrefix) != -1) {
    // See: https://github.com/letscontrolit/ESPEasy/issues/2081

    const char umlautAE_uni[3] = { 0xc3, 0x84, 0 };  // Unicode Umlaute AE
    const char umlautAE_lcd[2] = { 0xe1, 0 };        // P012_LCD Umlaute
    result.replace(umlautAE_uni,  umlautAE_lcd);

    const char umlaut_ae_uni[3] = { 0xc3, 0xa4, 0 }; // Unicode Umlaute ae
    result.replace(umlaut_ae_uni, umlautAE_lcd);

    const char umlautOE_uni[3] = { 0xc3, 0x96, 0 };  // Unicode Umlaute OE
    const char umlautOE_lcd[2] = { 0xef, 0 };        // P012_LCD Umlaute
    result.replace(umlautOE_uni,  umlautOE_lcd);

    const char umlaut_oe_uni[3] = { 0xc3, 0xb6, 0 }; // Unicode Umlaute oe
    result.replace(umlaut_oe_uni, umlautOE_lcd);

    const char umlautUE_uni[3] = { 0xc3, 0x9c, 0 };  // Unicode Umlaute UE
    const char umlautUE_lcd[2] = { 0xf5, 0 };        // P012_LCD Umlaute
    result.replace(umlautUE_uni,  umlautUE_lcd);

    const char umlaut_ue_uni[3] = { 0xc3, 0xbc, 0 }; // Unicode Umlaute ue
    result.replace(umlaut_ue_uni, umlautUE_lcd);

    const char umlaut_sz_uni[3] = { 0xc3, 0x9f, 0 }; // Unicode Umlaute sz
    const char umlaut_sz_lcd[2] = { 0xe2, 0 };       // P012_LCD Umlaute
    result.replace(umlaut_sz_uni, umlaut_sz_lcd);
  }
  return result;
}

void P012_data_struct::createCustomChars() {
# ifdef USES_P012_POLISH_CHARS
/*
  static const char LETTER_null[8] PROGMEM = { // spacja
    0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000
  };
*/
  static const char LETTER_a[8] PROGMEM = {    // a
    0b00000, 0b00000, 0b01110, 0b00001, 0b01111, 0b10001, 0b01111, 0b00010
  };
  static const char LETTER_c[8] PROGMEM = {    // c
    0b00010, 0b00100, 0b01110, 0b10000, 0b10000, 0b10001, 0b01110, 0b00000
  };
  static const char LETTER_e[8] PROGMEM = {    // e
    0b00000, 0b00000, 0b01110, 0b10001, 0b11111, 0b10000, 0b01110, 0b00010
  };
  static const char LETTER_l[8] PROGMEM = {    // l
    0b01100, 0b00100, 0b00101, 0b00110, 0b01100, 0b00100, 0b01110, 0b00000
  };
  static const char LETTER_n[8] PROGMEM = {    // n
    0b00010, 0b00100, 0b10110, 0b11001, 0b10001, 0b10001, 0b10001, 0b00000
  };
  static const char LETTER_o[8] PROGMEM = {    // o
    0b00010, 0b00100, 0b01110, 0b10001, 0b10001, 0b10001, 0b01110, 0b00000
  };
  static const char LETTER_s[8] PROGMEM = {    // s
    0b00010, 0b00100, 0b01110, 0b10000, 0b01110, 0b00001, 0b11110, 0b00000
  };
  /*
  static const char LETTER_z1[8] PROGMEM = {   // z z kreska
    0b00010, 0b00100, 0b11111, 0b00010, 0b00100, 0b01000, 0b11111, 0b00000
  };
  */
  static const char LETTER_z2[8] PROGMEM = {   // z z kropka
    0b00100, 0b00000, 0b11111, 0b00010, 0b00100, 0b01000, 0b11111, 0b00000
  };
  lcd.createChar(0, LETTER_o);  // probably defected memory cell
  lcd.createChar(1, LETTER_l);
  lcd.createChar(2, LETTER_e);
  lcd.createChar(3, LETTER_c);
  lcd.createChar(4, LETTER_n);
  lcd.createChar(5, LETTER_a);
  lcd.createChar(6, LETTER_s);
  lcd.createChar(7, LETTER_z2);
  lcd.createChar(8, LETTER_o);
# endif // ifdef USES_P012_POLISH_CHARS
}

#endif  // ifdef USES_P012
