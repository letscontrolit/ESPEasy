#ifndef PLUGINSTRUCTS_P148_DATA_STRUCT_H
#define PLUGINSTRUCTS_P148_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"

#ifdef USES_P148


// Special thanks to Theo Arends from Tasmota for disecting the very odd
// pixel addressing of this display used in the Sonoff POWR3xxD and THR3xxD
// The "font" and functions to address the TM1621 are directly taken from
// the Tasmota source code:
// https://github.com/arendst/Tasmota/blob/master/tasmota/tasmota_xdrv_driver/xdrv_87_esp32_sonoff_tm1621.ino
//
// This automatically applies to the license set by Theo: GPL-3.0-only

// Display layout
//         digits  symbols
// Row 1:  NNN.N   C/F/V/kWh
// Row 2:  NNN.N   %RH/A/W
//
// The decimal dots (1 per row) and the symbols are addressed as the "decimal dot" on regular 7-segment displays.
// However, the V & A symbols are connected.
// The same applies to kWh & W symbols.

// Default pins when used on Sonoff devices:
// Sonoff      THR3xxD  POWR3xxD
// TM1621 DAT  GPIO05   GPIO14
// TM1621 WR   GPIO18   GPIO27
// TM1621 RD   GPIO23   GPIO26
// TM1621 CS   GPIO17   GPIO25
//
// Sources:
// https://templates.blakadder.com/sonoff_THR316D.html
// https://templates.blakadder.com/sonoff_POWR316D.html

// Other sources:
// https://github.com/emsyscode/TM1621/blob/master/TM1621.cpp


struct P148_data_struct : public PluginTaskData_base {
public:

  // Value is being stored, so do not change values.
  enum class Tm1621Device {
    USER     = 0,
    POWR3xxD = 1, // Sonoff POWR316D / POWR320D
    THR3xxD  = 2  // Sonoff THR316D / THR320D
  };

  enum class Tm1621UnitOfMeasure {
    None,
    Celsius,
    Fahrenheit,
    kWh_Watt,
    Humidity,
    Volt_Amp
  };

  struct Tm1621_t {
    bool isValid() const;

    // "Text" to write to the display
    char         row[2][12]          = { {}, {} };
    int8_t       pin_da              = -1;
    int8_t       pin_cs              = -1;
    int8_t       pin_rd              = -1;
    int8_t       pin_wr              = -1;
    uint8_t      state               = {}; // FIXME TD-er: Still needed?
    Tm1621Device device              = Tm1621Device::USER;
    uint8_t      display_rotate      = {};
    uint8_t      temp_sensors        = {};
    uint8_t      temp_sensors_rotate = {};

    // Symbols
    bool celsius    = false;
    bool fahrenheit = false;
    bool humidity   = false;
    bool voltage    = false;
    bool kwh        = false;
    bool present    = false;
  };

private:

  static uint8_t TM1621GetFontCharacter(char character,
                                        bool firstrow);

public:

  P148_data_struct(const Tm1621_t& config);
  P148_data_struct()          = delete;
  virtual ~P148_data_struct() = default;

  bool init();

private:

  void            TM1621Init();
  void            TM1621WriteBit(bool value) const;
  void            TM1621StartSequence() const;
  void            TM1621StopSequence() const;
  void            TM1621SendCmnd(uint16_t command) const;
  void            TM1621SendAddress(uint16_t address) const;
  void            TM1621SendCommon(uint8_t common) const;
  void            TM1621WritePixelBuffer(const uint8_t *buf,
                                         size_t         size,
                                         uint16_t       address) const;
  void            TM1621SendRows() const;

  static uint32_t bufferIndex(bool firstrow, uint32_t col) {
    return firstrow ? col : 7 - col;
  }

public:

  void writeString(bool          firstrow,
                   const String& str);
  void writeStrings(const String& str1,
                    const String& str2);

  void writeFloats(float value1,
                   float value2);

  void writeFloat(bool  firstrow,
                  float value);

  void writeRawData(uint64_t rawdata) const;

  void setUnit(Tm1621UnitOfMeasure unit);

  void writeVoltAmp(float volt,
                    float amp);
  void writeEnergy(float kWh,
                   float watt);
  void writeTemp(float temp,
                 bool  Celsius = true);
  void writeHumidity(float humidity);

private:

  Tm1621_t Tm1621;
};

#endif // ifdef USES_P148
#endif // ifndef PLUGINSTRUCTS_P148_DATA_STRUCT_H
