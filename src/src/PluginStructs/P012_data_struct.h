#ifndef PLUGINSTRUCTS_P012_DATA_STRUCT_H
#define PLUGINSTRUCTS_P012_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"

#ifdef USES_P012

# include <LiquidCrystal_I2C.h>

enum class P012_DisplaySize_e : uint8_t {
  LCD_2x16 = 1u, // Value is stored in user settings, so don't change!
  LCD_4x20,
  LCD_2x16_ST7032,
};

const __FlashStringHelper* toString(P012_DisplaySize_e display);

struct P012_data_struct : public PluginTaskData_base {
  P012_data_struct(uint8_t            addr,
                   P012_DisplaySize_e lcd_size,
                   uint8_t            mode,
                   uint8_t            timer);
  P012_data_struct()          = delete;
  virtual ~P012_data_struct() = default;

  void init();

  void setBacklightTimer(uint8_t timer);

  void checkTimer();

  void lcdWrite(const String& text,
                uint8_t       col,
                uint8_t       row);

  String P012_parseTemplate(String& tmpString,
                            uint8_t lineSize);

  void   createCustomChars();

  bool   isValid() {
    return nullptr != lcd;
  }

  LiquidCrystal_I2C *lcd             = nullptr;
  int                Plugin_012_cols = 16;
  int                Plugin_012_rows = 2;
  int                Plugin_012_mode = 1;
  uint8_t            displayTimer    = 0;
};

#endif // ifdef USES_P012

#endif // ifndef PLUGINSTRUCTS_P012_DATA_STRUCT_H
