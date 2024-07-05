#include "../Commands/i2c.h"

#include "../Commands/Common.h"
#include "../ESPEasyCore/Serial.h"

#include "../Globals/I2Cdev.h"
#include "../Globals/Settings.h"

#include "../Helpers/Hardware_I2C.h"
#include "../Helpers/StringConverter.h"

#include "../../ESPEasy_common.h"

void i2c_scanI2Cbus(bool dbg, int8_t channel) {
  uint8_t error, address;

  #if FEATURE_I2CMULTIPLEXER

  if (-1 == channel) {
    serialPrintln(F("Standard I2C bus"));
  } else {
    serialPrintln(concat(F("Multiplexer channel "), channel));
  }
  #endif // if FEATURE_I2CMULTIPLEXER

  for (address = 1; address <= 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      serialPrintln(strformat(F("I2C  : Found 0x%02x"), address));
    } else if ((error == 4) || dbg) {
      serialPrintln(strformat(F("I2C  : Error %d at 0x%02x"), error, address));
    }
  }
}

const __FlashStringHelper* Command_i2c_Scanner(struct EventStruct *event, const char *Line)
{
  if (Settings.isI2CEnabled()) {
    const bool dbg = equals(parseString(Line, 2), F("1"));
    I2CSelect_Max100kHz_ClockSpeed(); // Scan bus using low speed

    i2c_scanI2Cbus(dbg, -1);          // Base I2C bus

    #if FEATURE_I2CMULTIPLEXER

    if (isI2CMultiplexerEnabled()) {
      uint8_t mux_max = I2CMultiplexerMaxChannels();

      for (int8_t channel = 0; channel < mux_max; ++channel) {
        I2CMultiplexerSelect(channel);
        i2c_scanI2Cbus(dbg, channel); // Multiplexer I2C bus
      }
      I2CMultiplexerOff();
    }
    #endif // if FEATURE_I2CMULTIPLEXER
    I2CSelectHighClockSpeed(); // By default the bus is in standard speed
  } else {
    serialPrintln(F("I2C  : Not enabled."));
  }
  return return_see_serial(event);
}
