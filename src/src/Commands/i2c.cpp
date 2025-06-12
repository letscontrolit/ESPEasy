#include "../Commands/i2c.h"

#include "../Commands/Common.h"
#include "../ESPEasyCore/Serial.h"

#include "../Globals/I2Cdev.h"
#include "../Globals/Settings.h"

#include "../Helpers/Hardware_I2C.h"
#include "../Helpers/StringConverter.h"

#include "../../ESPEasy_common.h"

void i2c_scanI2Cbus(bool dbg, int8_t channel, uint8_t i2cBus) {
  uint8_t error, address;

  #if FEATURE_I2CMULTIPLEXER

  if (-1 == channel) {
    serialPrintln(concat(F("Standard I2C bus "), i2cBus));
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
  const bool dbg = equals(parseString(Line, 2), F("1"));

  #if !FEATURE_I2C_MULTIPLE
  const uint8_t i2cBus = 0;
  #else // if !FEATURE_I2C_MULTIPLE

  for (uint8_t i2cBus = 0; i2cBus < getI2CBusCount(); ++i2cBus)
  #endif // if !FEATURE_I2C_MULTIPLE
  {
    if (Settings.isI2CEnabled(i2cBus)) {
      I2CSelect_Max100kHz_ClockSpeed(i2cBus); // Scan bus using low speed

      i2c_scanI2Cbus(dbg, -1, i2cBus);        // Base I2C bus

      #if FEATURE_I2CMULTIPLEXER

      if (isI2CMultiplexerEnabled(i2cBus)) {
        uint8_t mux_max = I2CMultiplexerMaxChannels(i2cBus);

        for (int8_t channel = 0; channel < mux_max; ++channel) {
          I2CMultiplexerSelect(i2cBus, channel);
          i2c_scanI2Cbus(dbg, channel, i2cBus); // Multiplexer I2C bus
        }
        I2CMultiplexerOff(0);
      }
      #endif // if FEATURE_I2CMULTIPLEXER
    } else {
      serialPrintln(strformat(F("I2C %d: Not enabled."), i2cBus));
    }
  }
  I2CSelectHighClockSpeed(0); // By default the bus is in standard speed
  return return_see_serial(event);
}
