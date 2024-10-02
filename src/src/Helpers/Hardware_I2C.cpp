#include "../Helpers/Hardware_I2C.h"

#include "../Globals/Settings.h"
#include "../Globals/Statistics.h"
#include "../Helpers/Hardware_defines.h"
#include "../Helpers/Hardware_GPIO.h"
#include "../Helpers/I2C_access.h"


#include <Wire.h>


void initI2C() {
  // configure hardware pins according to eeprom settings.
  if (!Settings.isI2CEnabled())
  {
    return;
  }
  addLog(LOG_LEVEL_INFO, F("INIT : I2C"));
  I2CSelectHighClockSpeed(); // Set normal clock speed

  if (Settings.WireClockStretchLimit)
  {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("INIT : I2C custom clockstretchlimit:");
      log += Settings.WireClockStretchLimit;
      addLogMove(LOG_LEVEL_INFO, log);
    }
      #if defined(ESP8266)
    Wire.setClockStretchLimit(Settings.WireClockStretchLimit);
      #endif // if defined(ESP8266)
      #ifdef ESP32
    Wire.setTimeOut(Settings.WireClockStretchLimit);
      #endif
  }

  #if FEATURE_I2CMULTIPLEXER

  if (validGpio(Settings.I2C_Multiplexer_ResetPin)) { // Initialize Reset pin to High if configured
    pinMode(Settings.I2C_Multiplexer_ResetPin, OUTPUT);
    digitalWrite(Settings.I2C_Multiplexer_ResetPin, HIGH);
  }
  #endif // if FEATURE_I2CMULTIPLEXER

  // I2C Watchdog boot status check
  if (Settings.WDI2CAddress != 0)
  {
    delay(500);
    if (I2C_write8_reg(Settings.WDI2CAddress,
                       0x83, // command to set pointer
                       17))   // pointer value to status uint8_t
    {                       
      bool is_ok = false;
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
}

void I2CSelectHighClockSpeed() {
  I2CSelectClockSpeed(Settings.I2C_clockSpeed);
}

void I2CSelectLowClockSpeed() {
  I2CSelectClockSpeed(Settings.I2C_clockSpeed_Slow);
}

void I2CSelect_Max100kHz_ClockSpeed() {
  if (Settings.I2C_clockSpeed <= 100000) {
    I2CSelectHighClockSpeed();
  } else if (Settings.I2C_clockSpeed_Slow <= 100000) {
    I2CSelectLowClockSpeed();
  } else {
    I2CSelectClockSpeed(100000);
  }
}

void I2CSelectClockSpeed(uint32_t clockFreq) {
  I2CBegin(Settings.Pin_i2c_sda, Settings.Pin_i2c_scl, clockFreq);
}

void I2CForceResetBus_swap_pins(uint8_t address) {
#if FEATURE_CLEAR_I2C_STUCK
  if (!Settings.EnableClearHangingI2Cbus()) { return; }
#endif

  // As a final work-around, we temporary swap SDA and SCL, perform a scan and return pin order.
  I2CBegin(Settings.Pin_i2c_scl, Settings.Pin_i2c_sda, 100000);
  I2C_wakeup(address);
  delay(1);

  // Now we switch back to the correct pins
  I2CSelectClockSpeed(100000);
}

void I2CBegin(int8_t sda, int8_t scl, uint32_t clockFreq) {
  #ifdef ESP32
  uint32_t lastI2CClockSpeed = Wire.getClock();
  #else // ifdef ESP32
  static uint32_t lastI2CClockSpeed = 0;
  #endif // ifdef ESP32
  static int8_t last_sda = -1;
  static int8_t last_scl = -1;

  if ((clockFreq == lastI2CClockSpeed) && (sda == last_sda) && (scl == last_scl)) {
    // No need to change the clock speed.
    return;
  }
  #ifdef ESP32

  if ((sda != last_sda) || (scl != last_scl)) {
    Wire.end();
  }
  #endif // ifdef ESP32
  lastI2CClockSpeed = clockFreq;
  last_scl          = scl;
  last_sda          = sda;

  #ifdef ESP32
  Wire.begin(sda, scl, clockFreq); // Will only set the clock when not yet initialized.
  Wire.setClock(clockFreq);
  #else // ifdef ESP32
  Wire.begin(sda, scl);
  Wire.setClock(clockFreq);
  #endif // ifdef ESP32
}

#if FEATURE_I2CMULTIPLEXER

// Check if the I2C Multiplexer is enabled
bool isI2CMultiplexerEnabled() {
  return Settings.I2C_Multiplexer_Type != I2C_MULTIPLEXER_NONE
         && Settings.I2C_Multiplexer_Addr != -1;
}

// Reset the I2C Multiplexer, if a pin is assigned for that. Pulled to low to force a reset.
void I2CMultiplexerReset() {
  if (Settings.I2C_Multiplexer_ResetPin != -1) {
    digitalWrite(Settings.I2C_Multiplexer_ResetPin, LOW);
    delay(1); // minimum requirement of low for a proper reset seems to be about 6 nsec, so 1 msec should be more than sufficient
    digitalWrite(Settings.I2C_Multiplexer_ResetPin, HIGH);
  }
}

// Shift the bit in the right position when selecting a single channel
uint8_t I2CMultiplexerShiftBit(uint8_t i) {
  uint8_t toWrite = 0;

  switch (Settings.I2C_Multiplexer_Type) {
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

  if (!bitRead(Settings.I2C_Flags[taskIndex], I2C_FLAGS_MUX_MULTICHANNEL)) {
    uint8_t i = Settings.I2C_Multiplexer_Channel[taskIndex];

    if (i > 7) { return; }
    toWrite = I2CMultiplexerShiftBit(i);
  } else {
    toWrite = Settings.I2C_Multiplexer_Channel[taskIndex]; // Bitpattern is already correctly stored
  }

  SetI2CMultiplexer(toWrite);
}

void I2CMultiplexerSelect(uint8_t i) {
  if (i > 7) { return; }

  uint8_t toWrite = I2CMultiplexerShiftBit(i);

  SetI2CMultiplexer(toWrite);
}

void I2CMultiplexerOff() {
  SetI2CMultiplexer(0); // no channel selected
}

void SetI2CMultiplexer(uint8_t toWrite) {
  if (isI2CMultiplexerEnabled()) {
    // FIXME TD-er: Must check to see if we can cache the value so only change it when needed.

    I2C_write8(Settings.I2C_Multiplexer_Addr, toWrite);

    // FIXME TD-er: We must check if the chip needs some time to set the output. (delay?)
  }
}

uint8_t I2CMultiplexerMaxChannels() {
  uint channels = 0;

  switch (Settings.I2C_Multiplexer_Type) {
    case I2C_MULTIPLEXER_TCA9548A:  channels = 8; break; // TCA9548A has 8 channels
    case I2C_MULTIPLEXER_TCA9546A:  channels = 4; break; // TCA9546A has 4 channels
    case I2C_MULTIPLEXER_PCA9540:   channels = 2; break; // PCA9540 has 2 channels
    case I2C_MULTIPLEXER_TCA9543A:  channels = 2; break; // TCA9543A has 2 channels
  }
  return channels;
}

// Has this taskIndex a channel selected? Checks for both Single channel and Multiple channel mode
// taskIndex must already be validated! (0..MAX_TASKS)
bool I2CMultiplexerPortSelectedForTask(taskIndex_t taskIndex) {
  if (!validTaskIndex(taskIndex)) { return false; }

  if (!isI2CMultiplexerEnabled()) { return false; }
  return (!bitRead(Settings.I2C_Flags[taskIndex], I2C_FLAGS_MUX_MULTICHANNEL) && Settings.I2C_Multiplexer_Channel[taskIndex] != -1)
         || (bitRead(Settings.I2C_Flags[taskIndex], I2C_FLAGS_MUX_MULTICHANNEL) && Settings.I2C_Multiplexer_Channel[taskIndex] !=  0);
}

#endif // if FEATURE_I2CMULTIPLEXER