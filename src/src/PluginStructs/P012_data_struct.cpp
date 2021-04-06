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

#endif // ifdef USES_P012
