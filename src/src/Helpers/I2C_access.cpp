#include "../Helpers/I2C_access.h"

#include "../DataStructs/TimingStats.h"
#include "../Globals/I2Cdev.h"
#include "../Globals/Settings.h"
#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/StringConverter.h"

enum class I2C_clear_bus_state {
  Start,
  Wait_SCL_become_high,     // Wait for 2.5 seconds for SCL to become high after enabling pull-up resistors
  Wait_SDA_become_high,
  Wait_SCL_SDA_become_high, // SDA is low, try to toggle SCL and wait for it to be freed.
};


// Code to clear I2C bus as described here:
// http://www.forward.com.au/pfod/ArduinoProgramming/I2C_ClearBus/index.html
// Changed into a state machine for use in a non blocking way to be used in ESPEasy.
I2C_bus_state I2C_check_bus(int8_t scl, int8_t sda) {
  static I2C_clear_bus_state clearing_state = I2C_clear_bus_state::Start;
  static unsigned long last_state_change    = 0;
  static int clockCount                     = 20; // > 2x9 clock


  switch (clearing_state) {
    case I2C_clear_bus_state::Start:
    {
      // FIXME TD-er: Check for proper I2C pins
      if ((sda < 0) || (scl < 0)) {
        last_state_change = 0;
        return I2C_bus_state::NotConfigured;
      }

      if ((digitalRead(scl) == HIGH) && (digitalRead(sda) == HIGH)) {
        last_state_change = 0;
        return I2C_bus_state::OK;
      }

      pinMode(sda, INPUT_PULLUP); // Make SDA (data) and SCL (clock) pins Inputs with pullup.
      pinMode(scl, INPUT_PULLUP);

      clockCount        = 20;
      clearing_state    = I2C_clear_bus_state::Wait_SCL_become_high;
      last_state_change = millis();
      break;
    }

    case I2C_clear_bus_state::Wait_SCL_become_high:
    {
      // Wait 2.5 secs. This is strictly only necessary on the first power
      // up of the DS3231 module to allow it to initialize properly,
      // but is also assists in reliable programming of FioV3 boards as it gives the
      // IDE a chance to start uploaded the program
      // before existing sketch confuses the IDE by sending Serial data.
      if (digitalRead(scl) == LOW) {
        if (timePassedSince(last_state_change) > 2500) {
          clearing_state = I2C_clear_bus_state::Start;
          return I2C_bus_state::SCL_Low; // I2C bus error. Could not clear SCL clock line held low
        }
        return I2C_bus_state::ClearingProcessActive;
      }
      clearing_state    = I2C_clear_bus_state::Wait_SDA_become_high;
      last_state_change = millis();
      break;
    }

    case I2C_clear_bus_state::Wait_SDA_become_high:
    {
      boolean SDA_LOW = (digitalRead(sda) == LOW); // vi. Check SDA input.

      while (SDA_LOW && (clockCount > 0)) {        //  vii. If SDA is Low,
        clockCount--;

        // Note: I2C bus is open collector so do NOT drive SCL or SDA high.
        pinMode(scl, INPUT);        // release SCL pullup so that when made output it will be LOW
        pinMode(scl, OUTPUT);       // then clock SCL Low
        delayMicroseconds(10);      //  for >5uS
        pinMode(scl, INPUT);        // release SCL LOW
        pinMode(scl, INPUT_PULLUP); // turn on pullup resistors again
        // do not force high as slave may be holding it low for clock stretching.
        delayMicroseconds(10);      //  for >5uS

        // The >5uS is so that even the slowest I2C devices are handled.
        if (digitalRead(scl) == LOW) {
          //  loop waiting for SCL to become High only wait 2sec.
          clearing_state    = I2C_clear_bus_state::Wait_SCL_SDA_become_high;
          last_state_change = millis();
          return I2C_bus_state::ClearingProcessActive;
        }
        SDA_LOW = (digitalRead(sda) == LOW);     //   and check SDA input again and loop
      }

      if (SDA_LOW) {                             // still low
        clearing_state = I2C_clear_bus_state::Start;
        return I2C_bus_state::SDA_Low_20_clocks; // I2C bus error. Could not clear. SDA data line held low
      }

      // else pull SDA line low for Start or Repeated Start
      pinMode(sda, INPUT);              // remove pullup.
      pinMode(sda, OUTPUT);             // and then make it LOW i.e. send an I2C Start or Repeated start control.
      // When there is only one I2C master a Start or Repeat Start has the same function as a Stop and clears the bus.
      // A Repeat Start is a Start occurring after a Start with no intervening Stop.
      delayMicroseconds(10);            // wait >5uS
      pinMode(sda, INPUT);              // remove output low
      pinMode(sda, INPUT_PULLUP);       // and make SDA high i.e. send I2C STOP control.
      delayMicroseconds(10);            // x. wait >5uS
      pinMode(sda, INPUT);              // and reset pins as tri-state inputs which is the default state on reset
      pinMode(scl, INPUT);
      clearing_state = I2C_clear_bus_state::Start;
      return I2C_bus_state::BusCleared; // all ok
    }

    case I2C_clear_bus_state::Wait_SCL_SDA_become_high:
    {
      if (digitalRead(scl) == LOW) {
        if (timePassedSince(last_state_change) > 2000) {
          // I2C bus error. Could not clear.
          // SCL clock line held low by slave clock stretch for >2sec
          clearing_state = I2C_clear_bus_state::Start;
          return I2C_bus_state::SDA_Low_over_2_sec;
        }
        return I2C_bus_state::ClearingProcessActive;
      }
      clearing_state    = I2C_clear_bus_state::Wait_SDA_become_high;
      last_state_change = millis();
      break;
    }
  }

  if (timePassedSince(last_state_change) > 5000) {
    // Just to prevent infinite loop
    // Should not be needed.
    clearing_state    = I2C_clear_bus_state::Start;
    last_state_change = millis();
  }
  return I2C_bus_state::ClearingProcessActive;
}

// **************************************************************************/
// Central functions for I2C data transfers
// **************************************************************************/
bool I2C_read_bytes(uint8_t i2caddr, I2Cdata_bytes& data) {
  const uint8_t size = data.getSize();

  return size == i2cdev.readBytes(i2caddr, data.getRegister(), size, data.get());
}

bool I2C_read_words(uint8_t i2caddr, I2Cdata_words& data) {
  const uint8_t size = data.getSize();

  return size == i2cdev.readWords(i2caddr, data.getRegister(), size, data.get());
}

// See https://github.com/platformio/platform-espressif32/issues/126
#ifdef ESP32

// ESP32: uint8_t TwoWire::endTransmission(bool sendStop)
  # define END_TRANSMISSION_FLAG true
#else // ifdef ESP32
// ESP8266: uint8_t TwoWire::endTransmission(uint8_t sendStop)
  # define END_TRANSMISSION_FLAG 0
#endif // ifdef ESP32

// **************************************************************************/
// Wake up I2C device
// **************************************************************************/
unsigned char I2C_wakeup(uint8_t i2caddr) {
  Wire.beginTransmission(i2caddr);
  return Wire.endTransmission();
}

// **************************************************************************/
// Writes an 8 bit value over I2C
// **************************************************************************/
bool I2C_write8(uint8_t i2caddr, uint8_t value) {
  Wire.beginTransmission(i2caddr);
  Wire.write((uint8_t)value);
  return Wire.endTransmission() == 0;
}

// **************************************************************************/
// Writes an 8 bit value over I2C to a register
// **************************************************************************/
bool I2C_write8_reg(uint8_t i2caddr, uint8_t reg, uint8_t value) {
  Wire.beginTransmission(i2caddr);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)value);
  return Wire.endTransmission() == 0;
}

// **************************************************************************/
// Writes an 16 bit value over I2C to a register
// **************************************************************************/
bool I2C_write16_reg(uint8_t i2caddr, uint8_t reg, uint16_t value) {
  Wire.beginTransmission(i2caddr);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)(value >> 8));
  Wire.write((uint8_t)value);
  return Wire.endTransmission() == 0;
}

// **************************************************************************/
// Writes an 16 bit value over I2C to a register
// **************************************************************************/
bool I2C_write16_LE_reg(uint8_t i2caddr, uint8_t reg, uint16_t value) {
  return I2C_write16_reg(i2caddr, reg, (value << 8) | (value >> 8));
}

// **************************************************************************/
// Writes length bytes over I2C to a register
// **************************************************************************/
bool I2C_writeBytes_reg(uint8_t i2caddr, uint8_t reg, uint8_t *buffer, uint8_t length) {
  Wire.beginTransmission(i2caddr);
  Wire.write(reg);

  for (int i = 0; i < length; i++) {
    Wire.write(*(buffer + i));
  }
  return Wire.endTransmission() == 0;
}

// **************************************************************************/
// Helper functions for read functions
// **************************************************************************/
bool I2C_setRegister(uint8_t i2caddr, uint8_t reg, bool *is_ok) {
  Wire.beginTransmission(i2caddr);
  Wire.write((uint8_t)reg);

  const bool res = Wire.endTransmission(END_TRANSMISSION_FLAG) == 0;

  /*
     0:success
     1:data too long to fit in transmit buffer
     2:received NACK on transmit of address
     3:received NACK on transmit of data
     4:other error
     See https://www.arduino.cc/en/Reference/WireEndTransmission
   */
  if (is_ok != nullptr) {
    *is_ok = res;
  }
  return res;
}

bool I2C_requestFrom(uint8_t i2caddr, uint8_t nrBytes, bool *is_ok) {
  const bool res = Wire.requestFrom(i2caddr, nrBytes) == nrBytes;

  if (is_ok != nullptr) {
    *is_ok = res;
  }
  return res;
}

// **************************************************************************/
// Reads an 8 bit value over I2C
// **************************************************************************/
uint8_t I2C_read8(uint8_t i2caddr, bool *is_ok) {
  uint8_t value{};

  if (I2C_requestFrom(i2caddr, 1, is_ok)) {
    value = Wire.read();
  }

  return value;
}

// **************************************************************************/
// Reads an 8 bit value from a register over I2C
// **************************************************************************/
uint8_t I2C_read8_reg(uint8_t i2caddr, uint8_t reg, bool *is_ok) {
  uint8_t value{};

  if (I2C_setRegister(i2caddr, reg, is_ok) && I2C_requestFrom(i2caddr, 1, is_ok)) {
    value = Wire.read();
  }

  return value;
}

// **************************************************************************/
// Reads a 16 bit value starting at a given register over I2C
// **************************************************************************/
uint16_t I2C_read16_reg(uint8_t i2caddr, uint8_t reg, bool *is_ok) {
  uint16_t value(0);

  if (I2C_setRegister(i2caddr, reg, is_ok) && I2C_requestFrom(i2caddr, 2, is_ok)) {
    value = (Wire.read() << 8) | Wire.read();
  }

  return value;
}

// **************************************************************************/
// Reads a 24 bit value starting at a given register over I2C
// **************************************************************************/
int32_t I2C_read24_reg(uint8_t i2caddr, uint8_t reg, bool *is_ok) {
  int32_t value{};

  if (I2C_setRegister(i2caddr, reg, is_ok) && I2C_requestFrom(i2caddr, 3, is_ok)) {
    value = (((int32_t)Wire.read()) << 16) | (Wire.read() << 8) | Wire.read();
  }

  return value;
}

// **************************************************************************/
// Reads a 32 bit value starting at a given register over I2C
// **************************************************************************/
int32_t I2C_read32_reg(uint8_t i2caddr, uint8_t reg, bool *is_ok) {
  int32_t value{};

  if (I2C_setRegister(i2caddr, reg, is_ok) && I2C_requestFrom(i2caddr, 4, is_ok)) {
    value = (((int32_t)Wire.read()) << 24) | (((uint32_t)Wire.read()) << 16) | (Wire.read() << 8) | Wire.read();
  }

  return value;
}

// **************************************************************************/
// Reads a 16 bit value starting at a given register over I2C
// **************************************************************************/
uint16_t I2C_read16_LE_reg(uint8_t i2caddr, uint8_t reg, bool *is_ok) {
  const uint16_t temp = I2C_read16_reg(i2caddr, reg, is_ok);

  return (temp >> 8) | (temp << 8);
}

// **************************************************************************/
// Reads a signed 16 bit value starting at a given register over I2C
// **************************************************************************/
int16_t I2C_readS16_reg(uint8_t i2caddr, uint8_t reg, bool *is_ok) {
  return (int16_t)I2C_read16_reg(i2caddr, reg, is_ok);
}

int16_t I2C_readS16_LE_reg(uint8_t i2caddr, uint8_t reg, bool *is_ok) {
  return (int16_t)I2C_read16_LE_reg(i2caddr, reg, is_ok);
}

// *************************************************************************/
// Checks if a device is responding on the address
// Should be used in any I2C plugin case PLUGIN_INIT: before any initialization
// Can be used in any I2C plugin case PLUGIN_READ: to check if the device is still connected/responding
// if (!I2C_deviceCheck(configured_I2C_address)) {
//   break; // Will return the default false for success
// }
// *************************************************************************/

#if FEATURE_I2C_DEVICE_CHECK
static uint8_t deviceCheckI2C[TASKS_MAX]{};

bool I2C_deviceCheck(uint8_t     i2caddr,
                     taskIndex_t taskIndex,
                     uint8_t     maxRetries,
                     uint8_t     function) {
  if (!Settings.CheckI2Cdevice()) { return true; } // Check disabled, continue

  bool retval = true;

  if (validTaskIndex(taskIndex)) {
    if (Settings.TaskDeviceDataFeed[taskIndex] != 0) { return true; } // Remote device can't be checked

    START_TIMER;
    Wire.beginTransmission(i2caddr);

    retval = 0 == Wire.endTransmission(); // Only 0 indicates Success

    if (retval) {
      deviceCheckI2C[taskIndex] = 0;
    } else {
      if (maxRetries > 0) {
        deviceCheckI2C[taskIndex]++;

        if (deviceCheckI2C[taskIndex] >= maxRetries) {
          Settings.TaskDeviceEnabled[taskIndex] = false; // If the number of retries is reached, disable the device
          # ifndef BUILD_NO_DEBUG
          addLog(LOG_LEVEL_ERROR, concat(F("I2C  : Device doesn't respond for task: "), static_cast<int>(taskIndex + 1)));
          # endif // ifndef BUILD_NO_DEBUG
        }
      }
    }

    if (function != 0) {
      STOP_TIMER_TASK(getDeviceIndex_from_TaskIndex(taskIndex), function);
    }
  }
  return retval;
}

#endif // if FEATURE_I2C_DEVICE_CHECK

#undef END_TRANSMISSION_FLAG
