#include "../Helpers/Hardware_I2C.h"

#include "../Globals/Settings.h"
#include "../Globals/Statistics.h"
#include "../Helpers/Hardware_defines.h"
#include "../Helpers/Hardware_GPIO.h"
#include "../Helpers/I2C_access.h"
#include "../Helpers/StringConverter.h"


#include <Wire.h>


void initI2C() {
  // configure hardware pins according to eeprom settings.
  if (!Settings.isI2CEnabled(0)
      #if FEATURE_I2C_MULTIPLE
      && !Settings.isI2CEnabled(1)
      # if FEATURE_I2C_INTERFACE_3
      && !Settings.isI2CEnabled(2)
      # endif // if FEATURE_I2C_INTERFACE_3
      #endif // if FEATURE_I2C_MULTIPLE
      )
  {
    return;
  }
  #if !FEATURE_I2C_MULTIPLE
  const uint8_t i2cBus = 0;
  #else // if !FEATURE_I2C_MULTIPLE

  for (uint8_t i2cBus = 0; i2cBus < getI2CBusCount(); ++i2cBus)
  #endif // if !FEATURE_I2C_MULTIPLE
  {
    if (Settings.isI2CEnabled(i2cBus)) {
      #ifndef BUILD_MINIMAL_OTA
      #if !FEATURE_I2C_MULTIPLE
      addLog(LOG_LEVEL_INFO, F("INIT : I2C Bus"));
      #else // if !FEATURE_I2C_MULTIPLE
      addLog(LOG_LEVEL_INFO, concat(F("INIT : I2C Bus "), i2cBus));
      #endif // if !FEATURE_I2C_MULTIPLE
      #endif
      I2CSelectHighClockSpeed(i2cBus); // Set normal clock speed, on I2C Bus 1 (index 0)
    }
  }

  #if FEATURE_I2CMULTIPLEXER

  for (uint8_t i2cBus = 0; i2cBus < getI2CBusCount(); ++i2cBus) {
    const int8_t Multiplexer_ResetPin = Settings.getI2CMultiplexerResetPin(i2cBus);

    if (validGpio(Multiplexer_ResetPin)) { // Initialize Reset pin to High if configured
      pinMode(Multiplexer_ResetPin, OUTPUT);
      digitalWrite(Multiplexer_ResetPin, HIGH);
    }
  }
  #endif // if FEATURE_I2CMULTIPLEXER

  // I2C Watchdog boot status check
  if (Settings.WDI2CAddress != 0)
  {
    delay(500);

    #if FEATURE_I2C_MULTIPLE
    I2CSelectHighClockSpeed(Settings.getI2CInterfaceWDT());
    #endif // if FEATURE_I2C_MULTIPLE

    if (I2C_write8_reg(Settings.WDI2CAddress,
                       0x83, // command to set pointer
                       17))  // pointer value to status uint8_t
    {
      bool is_ok           = false;
      const uint8_t status = I2C_read8(Settings.WDI2CAddress, &is_ok);

      if (is_ok)
      {
        if (status & 0x1)
        {
          addLog(LOG_LEVEL_ERROR, F("INIT : Reset by WD!"));
          lastBootCause = BOOT_CAUSE_EXT_WD;
        }
      }
    }
  }
  I2CSelectHighClockSpeed(0); // Select first interface by default
}

void I2CSelectHighClockSpeed(uint8_t i2cBus) {
  const uint32_t clockSpeed = Settings.getI2CClockSpeed(i2cBus);

  I2CSelectClockSpeed(i2cBus, clockSpeed);
}

void I2CSelectLowClockSpeed(uint8_t i2cBus) {
  const uint32_t clockSpeed = Settings.getI2CClockSpeedSlow(i2cBus);

  I2CSelectClockSpeed(i2cBus, clockSpeed);
}

void I2CSelect_Max100kHz_ClockSpeed(uint8_t i2cBus) {
  const uint32_t clockSpeed      = Settings.getI2CClockSpeed(i2cBus);
  const uint32_t clockSpeed_Slow = Settings.getI2CClockSpeedSlow(i2cBus);

  if (clockSpeed <= 100000) {
    I2CSelectHighClockSpeed(i2cBus);
  } else if (clockSpeed_Slow <= 100000) {
    I2CSelectLowClockSpeed(i2cBus);
  } else {
    I2CSelectClockSpeed(i2cBus, 100000);
  }
}

void I2CSelectClockSpeed(uint8_t i2cBus, uint32_t clockFreq) {
  const int8_t   i2c_sda           = Settings.getI2CSdaPin(i2cBus);
  const int8_t   i2c_scl           = Settings.getI2CSclPin(i2cBus);
  const uint32_t ClockStretchLimit = Settings.getI2CClockStretch(i2cBus);

  I2CBegin(i2c_sda, i2c_scl, clockFreq, ClockStretchLimit);
}

void I2CForceResetBus_swap_pins(uint8_t i2cBus, uint8_t address) {
#if FEATURE_CLEAR_I2C_STUCK

  if (!Settings.EnableClearHangingI2Cbus()) { return; }
#endif // if FEATURE_CLEAR_I2C_STUCK

  // As a final work-around, we temporary swap SDA and SCL, perform a scan and return pin order.
  const int8_t   i2c_sda           = Settings.getI2CSdaPin(i2cBus);
  const int8_t   i2c_scl           = Settings.getI2CSclPin(i2cBus);
  const uint32_t ClockStretchLimit = Settings.getI2CClockStretch(i2cBus);

  I2CBegin(i2c_scl, i2c_sda, 100000, ClockStretchLimit);
  I2C_wakeup(address);
  delay(1);

  // Now we switch back to the correct pins
  I2CSelectClockSpeed(i2cBus, 100000);
}

void I2CBegin(int8_t sda, int8_t scl, uint32_t clockFreq, uint32_t clockStretch) {
  #ifdef ESP32
  uint32_t lastI2CClockSpeed = Wire.getClock();
  #else // ifdef ESP32
  static uint32_t lastI2CClockSpeed = 0;
  #endif // ifdef ESP32
  static int8_t   last_sda     = -1;
  static int8_t   last_scl     = -1;
  static uint32_t last_stretch = 0;

  if ((clockFreq == lastI2CClockSpeed) && (sda == last_sda) && (scl == last_scl) && (clockStretch == last_stretch)) {
    // No need to change the clock speed.
    return;
  }
  if (sda == -1 || scl == -1) {
#ifdef ESP32
    Wire.end();
#endif
    last_sda = sda;
    last_scl = scl;
    return;
  }


  #ifdef ESP32

  if (((sda != last_sda) || (scl != last_scl)) && (last_sda != -1) && (last_scl != -1)) {
    if (!Wire.end()) {
      addLog(LOG_LEVEL_ERROR, strformat(F("I2C  : Wire.end() failed on SDA: %d SCL: %d"), last_sda, last_scl));
    }
    gpio_reset_pin((gpio_num_t)last_sda); // Force-release pins to allow GPIO-multiplexing the Wire object
    gpio_reset_pin((gpio_num_t)last_scl);
    delay(0);                             // Release some cycles
  }
  #endif // ifdef ESP32

  #ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_DEV)) {
    addLog(LOG_LEVEL_DEBUG_DEV, strformat(F("I2C  : SDA: %d, SCL: %d, Clock: %d (%d) (Stretch: %d)"),
                                          sda, scl, clockFreq, lastI2CClockSpeed, clockStretch));
  }
  #endif // ifndef BUILD_NO_DEBUG

  if ((sda != last_sda) || (scl != last_scl) || (clockFreq != lastI2CClockSpeed)) {
    #ifdef ESP32
    Wire.setPins(sda, scl);
    Wire.begin();
    Wire.setClock(clockFreq);
    #else // ifdef ESP32
    Wire.begin(sda, scl);
    Wire.setClock(clockFreq);
    #endif // ifdef ESP32
  }
  lastI2CClockSpeed = clockFreq;
  last_scl          = scl;
  last_sda          = sda;


  if (clockStretch != last_stretch) {
    #if defined(ESP8266)
    Wire.setClockStretchLimit(clockStretch);
    #endif // if defined(ESP8266)
    #ifdef ESP32
    Wire.setTimeOut(clockStretch);
    #endif // ifdef ESP32
    last_stretch = clockStretch;
  }
}

#if FEATURE_I2CMULTIPLEXER

// Check if the I2C Multiplexer is enabled
bool isI2CMultiplexerEnabled(uint8_t i2cBus) {
  return Settings.getI2CMultiplexerType(i2cBus) != I2C_MULTIPLEXER_NONE
         && Settings.getI2CMultiplexerAddr(i2cBus) != -1;
}

// Reset the I2C Multiplexer, if a pin is assigned for that. Pulled to low to force a reset.
void I2CMultiplexerReset(uint8_t i2cBus) {
  const uint8_t Multiplexer_ResetPin = Settings.getI2CMultiplexerResetPin(i2cBus);

  if (Multiplexer_ResetPin != -1) {
    digitalWrite(Multiplexer_ResetPin, LOW);
    delay(1); // minimum requirement of low for a proper reset seems to be about 6 nsec, so 1 msec should be more than sufficient
    digitalWrite(Multiplexer_ResetPin, HIGH);
  }
}

// Shift the bit in the right position when selecting a single channel
uint8_t I2CMultiplexerShiftBit(uint8_t i2cBus, uint8_t i) {
  uint8_t toWrite               = 0;
  const int8_t Multiplexer_Type = Settings.getI2CMultiplexerType(i2cBus);

  switch (Multiplexer_Type) {
    case I2C_MULTIPLEXER_TCA9543A: // TCA9543/6/8 addressing
    case I2C_MULTIPLEXER_TCA9546A:
    case I2C_MULTIPLEXER_TCA9548A:
      toWrite = (1 << i);
      break;
    case I2C_MULTIPLEXER_PCA9540: // PCA9540 needs bit 2 set to write the channel
      toWrite = 0b00000100;

      if (i == 1) {
        toWrite |= 0b00000010; // And bit 0 not set when selecting channel 0...
      }
      break;
  }
  return toWrite;
}

// As initially constructed by krikk in PR#254, quite adapted
// utility method for the I2C multiplexer
// select the multiplexer port given as parameter, if taskIndex < 0 then take that abs value as the port to select (to allow I2C scanner)
void I2CMultiplexerSelectByTaskIndex(taskIndex_t taskIndex) {
  if (!validTaskIndex(taskIndex)) { return; }

  if (!I2CMultiplexerPortSelectedForTask(taskIndex)) { return; }

  uint8_t toWrite = 0;

  # if FEATURE_I2C_MULTIPLE
  const uint8_t i2cBus = Settings.getI2CInterface(taskIndex);
  # else // if FEATURE_I2C_MULTIPLE
  const uint8_t i2cBus = 0;
  # endif // if FEATURE_I2C_MULTIPLE

  if (!bitRead(Settings.I2C_Flags[taskIndex], I2C_FLAGS_MUX_MULTICHANNEL)) {
    uint8_t i = Settings.I2C_Multiplexer_Channel[taskIndex];

    if (i > 7) { return; }
    toWrite = I2CMultiplexerShiftBit(i2cBus, i);
  } else {
    toWrite = Settings.I2C_Multiplexer_Channel[taskIndex]; // Bitpattern is already correctly stored
  }

  SetI2CMultiplexer(i2cBus, toWrite);
}

void I2CMultiplexerSelect(uint8_t i2cBus, uint8_t i) {
  if (i > 7) { return; }

  uint8_t toWrite = I2CMultiplexerShiftBit(i2cBus, i);

  SetI2CMultiplexer(i2cBus, toWrite);
}

void I2CMultiplexerOff(uint8_t i2cBus) {
  SetI2CMultiplexer(i2cBus, 0); // no channel selected
}

void SetI2CMultiplexer(uint8_t i2cBus, uint8_t toWrite) {
  if (isI2CMultiplexerEnabled(i2cBus)) {
    const int8_t Multiplexer_Addr = Settings.getI2CMultiplexerAddr(i2cBus);

    I2C_write8(Multiplexer_Addr, toWrite);

    // FIXME TD-er: We must check if the chip needs some time to set the output. (delay?)
  }
}

uint8_t I2CMultiplexerMaxChannels(uint8_t i2cBus) {
  uint   channels         = 0;
  int8_t Multiplexer_Type = Settings.getI2CMultiplexerType(i2cBus);

  switch (Multiplexer_Type) {
    case I2C_MULTIPLEXER_TCA9548A: channels = 8; break; // TCA9548A has 8 channels
    case I2C_MULTIPLEXER_TCA9546A: channels = 4; break; // TCA9546A has 4 channels
    case I2C_MULTIPLEXER_PCA9540:                       // PCA9540 has 2 channels
    case I2C_MULTIPLEXER_TCA9543A: channels = 2; break; // TCA9543A has 2 channels
  }
  return channels;
}

// Has this taskIndex a channel selected? Checks for both Single channel and Multiple channel mode
// taskIndex must already be validated! (0..MAX_TASKS)
bool I2CMultiplexerPortSelectedForTask(taskIndex_t taskIndex) {
  if (!validTaskIndex(taskIndex)) { return false; }

  # if FEATURE_I2C_MULTIPLE
  const uint8_t i2cBus = Settings.getI2CInterface(taskIndex);
  # else // if FEATURE_I2C_MULTIPLE
  const uint8_t i2cBus = 0;
  # endif // if FEATURE_I2C_MULTIPLE

  if (!isI2CMultiplexerEnabled(i2cBus)) { return false; }
  return (!bitRead(Settings.I2C_Flags[taskIndex], I2C_FLAGS_MUX_MULTICHANNEL) && Settings.I2C_Multiplexer_Channel[taskIndex] != -1)
         || (bitRead(Settings.I2C_Flags[taskIndex], I2C_FLAGS_MUX_MULTICHANNEL) && Settings.I2C_Multiplexer_Channel[taskIndex] !=  0);
}

#endif // if FEATURE_I2CMULTIPLEXER
