/******************************************************************************
SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h
By: Paul Clark
October 23rd 2020

Based extensively on:
SparkFunMAX17043.cpp
SparkFun MAX17043 Library Source File
Jim Lindblom @ SparkFun Electronics
Original Creation Date: June 22, 2015
https://github.com/sparkfun/SparkFun_MAX17043_Particle_Library

This file implements all functions of the MAX17043 class. Functions here range
from higher level stuff, like reading/writing MAX17043 registers to low-level,
hardware reads and writes.

This code is released under the MIT license.

Distributed as-is; no warranty is given.
******************************************************************************/
#include "SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h"

SFE_MAX1704X::SFE_MAX1704X(sfe_max1704x_devices_e device)
{
  // Constructor

  // Record the device type
  _device = device;

  // Define the full-scale voltage for VCELL based on the device
  switch (device)
  {
    case MAX1704X_MAX17044:
      _full_scale = 10.24; // MAX17044 VCELL is 12-bit, 2.50mV per LSB
      break;
    case MAX1704X_MAX17048:
      _full_scale = 5.12; // MAX17048 VCELL is 16-bit, 78.125uV/cell per LSB
      break;
    case MAX1704X_MAX17049:
      _full_scale = 10.24; // MAX17049 VCELL is 16-bit, 78.125uV/cell per LSB (i.e. 156.25uV per LSB)
      break;
    default: // Default is the MAX17043
      _full_scale = 5.12; // MAX17043 VCELL is 12-bit, 1.25mV per LSB
      break;
  }
}

boolean SFE_MAX1704X::begin(TwoWire &wirePort)
{
  _i2cPort = &wirePort; //Grab which port the user wants us to use

  if (isConnected() == false)
  {
    if (_printDebug == true)
    {
      _debugPort->println(F("begin: isConnected returned false"));
    }
    return (false);
  }

  return (true);
}

//Returns true if device answers on _deviceAddress
boolean SFE_MAX1704X::isConnected(void)
{
  _i2cPort->beginTransmission((uint8_t)MAX1704x_ADDRESS);
  if (_i2cPort->endTransmission() == 0)
  {
    //Get version should return 0x001_
    //Not a great test but something
    //Supported on 43/44/48/49
    if (getVersion() & (1 << 4))
      return true;
  }
  return false;
}

//Enable or disable the printing of debug messages
void SFE_MAX1704X::enableDebugging(Stream &debugPort)
{
  _debugPort = &debugPort; //Grab which port the user wants us to use for debugging
  _printDebug = true;      //Should we print the commands we send? Good for debugging
}

void SFE_MAX1704X::disableDebugging(void)
{
  _printDebug = false; //Turn off extra print statements
}

uint8_t SFE_MAX1704X::quickStart()
{
  // A quick-start allows the MAX17043 to restart fuel-gauge calculations in the
  // same manner as initial power-up of the IC. If an application’s power-up
  // sequence is exceedingly noisy such that excess error is introduced into the
  // IC’s “first guess” of SOC, the host can issue a quick-start to reduce the
  // error. A quick-start is initiated by a rising edge on the QSTRT pin, or
  // through software by writing 4000h to MODE register.

  // Note: on the MAX17048/49 this will also clear / disable EnSleep

  return write16(MAX17043_MODE_QUICKSTART, MAX17043_MODE);
}

float SFE_MAX1704X::getVoltage()
{
  uint16_t vCell;
  vCell = read16(MAX17043_VCELL);

  if (_device <= MAX1704X_MAX17044)
  {
    // On the MAX17043/44: vCell is a 12-bit register where each bit represents:
    // 1.25mV on the MAX17043
    // 2.5mV on the MAX17044
    vCell = (vCell) >> 4; // Align the 12 bits

    float divider = 4096.0 / _full_scale;

    return (((float)vCell) / divider);
  }
  else
  {
    // On the MAX17048/49: vCell is a 16-bit register where each bit represents 78.125uV/cell per LSB
    // i.e. 78.125uV per LSB on the MAX17048
    // i.e. 156.25uV per LSB on the MAX17049

    float divider = 65536.0 / _full_scale;

    return (((float)vCell) / divider);
  }
}

float SFE_MAX1704X::getSOC()
{
  uint16_t soc;
  float percent;
  soc = read16(MAX17043_SOC);
  percent = (float)((soc & 0xFF00) >> 8);
  percent += ((float)(soc & 0x00FF)) / 256.0;

  return percent;
}

uint16_t SFE_MAX1704X::getVersion()
{
  return read16(MAX17043_VERSION);
}

//Supported on MAX17048/49
uint8_t SFE_MAX1704X::getID()
{
  if (_device <= MAX1704X_MAX17044)
  {
    if (_printDebug == true)
    {
      _debugPort->println(F("getID: not supported on this device"));
    }
    return (0);
  }

  uint16_t vresetID = read16(MAX17048_VRESET_ID);
  return (vresetID & 0xFF);
}

//Default is 0x4B = 75 (7bit, shifted from 0x96__)
//40mV per bit. So default is 3.0V.
uint8_t SFE_MAX1704X::setResetVoltage(uint8_t threshold)
{
  if (_device <= MAX1704X_MAX17044)
  {
    if (_printDebug == true)
    {
      _debugPort->println(F("setResetVoltage: not supported on this device"));
    }
    return (MAX17043_GENERIC_ERROR);
  }

  uint16_t vreset = read16(MAX17048_VRESET_ID);
  vreset &= 0x01FF;                     // Mask out bits to set
  vreset |= ((uint16_t)threshold << 9); // Add new threshold

  return write16(vreset, MAX17048_VRESET_ID);
}
uint8_t SFE_MAX1704X::setResetVoltage(float threshold)
{
  // 7 bits. LSb = 40mV
  uint8_t thresh = (uint8_t)(constrain(threshold, 0.0, 5.08) / 0.04);
  return setResetVoltage(thresh);
}

uint8_t SFE_MAX1704X::getResetVoltage(void)
{
  if (_device <= MAX1704X_MAX17044)
  {
    if (_printDebug == true)
    {
      _debugPort->println(F("getResetVoltage: not supported on this device"));
    }
    return (0);
  }

  uint16_t threshold = read16(MAX17048_VRESET_ID) >> 9;
  return ((uint8_t)threshold);
}

uint8_t SFE_MAX1704X::enableComparator(void)
{
  if (_device <= MAX1704X_MAX17044)
  {
    if (_printDebug == true)
    {
      _debugPort->println(F("enableComparator: not supported on this device"));
    }
    return (MAX17043_GENERIC_ERROR);
  }

  uint16_t vresetReg = read16(MAX17048_VRESET_ID);
  vresetReg &= ~(1 << 8); //Clear bit to enable comparator
  return write16(vresetReg, MAX17048_VRESET_ID);
}

uint8_t SFE_MAX1704X::disableComparator(void)
{
  if (_device <= MAX1704X_MAX17044)
  {
    if (_printDebug == true)
    {
      _debugPort->println(F("disableComparator: not supported on this device"));
    }
    return (MAX17043_GENERIC_ERROR);
  }

  uint16_t vresetReg = read16(MAX17048_VRESET_ID);
  vresetReg |= (1 << 8); //Set bit to disable comparator
  return write16(vresetReg, MAX17048_VRESET_ID);
}

float SFE_MAX1704X::getChangeRate(void)
{
  if (_device <= MAX1704X_MAX17044)
  {
    if (_printDebug == true)
    {
      _debugPort->println(F("getChangeRate: not supported on this device"));
    }
    return (0.0);
  }

  int16_t changeRate = read16(MAX17048_CRATE);
  float changerate_f = changeRate * 0.208;
  return (changerate_f);
}

uint8_t SFE_MAX1704X::getStatus(void)
{
  if (_device <= MAX1704X_MAX17044)
  {
    if (_printDebug == true)
    {
      _debugPort->println(F("getStatus: not supported on this device"));
    }
    return (0);
  }

  uint8_t statusReg = read16(MAX17048_STATUS) >> 8;
  return (statusReg & 0x7F); //Highest bit is don't care
}

bool SFE_MAX1704X::isReset(bool clear)
{
  uint8_t status = getStatus();
  bool flag = (status & MAX1704x_STATUS_RI) > 0;
  if (flag && clear) // Clear the flag if requested
  {
    // Clear the aligned bit in the status register
    clearStatusRegBits(MAX1704x_STATUS_RI << 8);
  }
  return (flag);
}
bool SFE_MAX1704X::isVoltageHigh(bool clear)
{
  uint8_t status = getStatus();
  bool flag = (status & MAX1704x_STATUS_VH) > 0;
  if (flag && clear) // Clear the flag if requested
  {
    // Clear the aligned bit in the status register
    clearStatusRegBits(MAX1704x_STATUS_VH << 8);
  }
  return (flag);
}
bool SFE_MAX1704X::isVoltageLow(bool clear)
{
  uint8_t status = getStatus();
  bool flag = (status & MAX1704x_STATUS_VL) > 0;
  if (flag && clear) // Clear the flag if requested
  {
    // Clear the aligned bit in the status register
    clearStatusRegBits(MAX1704x_STATUS_VL << 8);
  }
  return (flag);
}
bool SFE_MAX1704X::isVoltageReset(bool clear)
{
  uint8_t status = getStatus();
  bool flag = (status & MAX1704x_STATUS_VR) > 0;
  if (flag && clear) // Clear the flag if requested
  {
    // Clear the aligned bit in the status register
    clearStatusRegBits(MAX1704x_STATUS_VR << 8);
  }
  return (flag);
}
bool SFE_MAX1704X::isLow(bool clear)
{
  uint8_t status = getStatus();
  bool flag = (status & MAX1704x_STATUS_HD) > 0;
  if (flag && clear) // Clear the flag if requested
  {
    // Clear the aligned bit in the status register
    clearStatusRegBits(MAX1704x_STATUS_HD << 8);
  }
  return (flag);
}
bool SFE_MAX1704X::isChange(bool clear)
{
  uint8_t status = getStatus();
  bool flag = (status & MAX1704x_STATUS_SC) > 0;
  if (flag && clear) // Clear the flag if requested
  {
    // Clear the aligned bit in the status register
    clearStatusRegBits(MAX1704x_STATUS_SC << 8);
  }
  return (flag);
}

// Clear the specified bit in the MAX17048/49 status register (PRIVATE)
// This requires the bits in mask to be correctly aligned.
// MAX1704x_STATUS_RI etc. will need to be shifted left by 8 bits to become aligned.
uint8_t SFE_MAX1704X::clearStatusRegBits(uint16_t mask)
{
  uint16_t statusReg = read16(MAX17048_STATUS);
  statusReg &= ~mask; // Clear the specified bits
  return (write16(statusReg, MAX17048_STATUS)); // Write the contents back again
}

uint8_t SFE_MAX1704X::clearAlert()
{
  // Read config reg, so we don't modify any other values:
  uint16_t configReg = read16(MAX17043_CONFIG);
  configReg &= ~MAX17043_CONFIG_ALERT; // Clear ALRT bit manually.

  return write16(configReg, MAX17043_CONFIG);
}

// getAlert([clear]) - Check if the MAX1704X's ALRT alert interrupt has been
// triggered.
// INPUT: [clear] - If [clear] is true, the alert flag will be cleared if it
// was set.
// OUTPUT: Returns 1 if interrupt is/was triggered, 0 if not.
uint8_t SFE_MAX1704X::getAlert(bool clear)
{
  // Read config reg, so we don't modify any other values:
  uint16_t configReg = read16(MAX17043_CONFIG);
  if (configReg & MAX17043_CONFIG_ALERT)
  {
    if (clear) // If the clear flag is set
    {
      configReg &= ~MAX17043_CONFIG_ALERT; // Clear ALRT bit manually.
      write16(configReg, MAX17043_CONFIG);
    }
    return 1;
  }

  return 0;
}

// enableSOCAlert() - (MAX17048/49) Enable the SOC change alert
// Returns true if the SOC change alert was enabled successfully
bool SFE_MAX1704X::enableSOCAlert()
{
  if (_device <= MAX1704X_MAX17044)
  {
    if (_printDebug == true)
    {
      _debugPort->println(F("enableSOCAlert: not supported on this device"));
    }
    return (false);
  }

  // Read config reg, so we don't modify any other values:
  uint16_t configReg = read16(MAX17043_CONFIG);
  configReg |= MAX17043_CONFIG_ALSC; // Set the ALSC bit
  // Update the config register, return false if the write fails
  if (write16(configReg, MAX17043_CONFIG) > 0)
    return (false);
  // Re-Read the config reg
  configReg = read16(MAX17043_CONFIG);
  // Return true if the ALSC bit is set, otherwise return false
  return ((configReg & MAX17043_CONFIG_ALSC) > 0);
}

// disableSOCAlert() - (MAX17048/49) Disable the SOC change alert
// Returns true if the SOC change alert was disabled successfully
bool SFE_MAX1704X::disableSOCAlert()
{
  if (_device <= MAX1704X_MAX17044)
  {
    if (_printDebug == true)
    {
      _debugPort->println(F("disableSOCAlert: not supported on this device"));
    }
    return (false);
  }

  // Read config reg, so we don't modify any other values:
  uint16_t configReg = read16(MAX17043_CONFIG);
  configReg &= ~MAX17043_CONFIG_ALSC; // Clear the ALSC bit
  // Update the config register, return false if the write fails
  if (write16(configReg, MAX17043_CONFIG) > 0)
    return (false);
  // Re-Read the config reg
  configReg = read16(MAX17043_CONFIG);
  // Return true if the ALSC bit is clear, otherwise return false
  return ((configReg & MAX17043_CONFIG_ALSC) == 0);
}

// Enable or Disable MAX17048 VRESET Alert:
//  EnVr (enable voltage reset alert) when set to 1 asserts
//  the ALRT pin when a voltage-reset event occurs under
//  the conditions described by the VRESET/ ID register.
uint8_t SFE_MAX1704X::enableAlert(void)
{
  if (_device <= MAX1704X_MAX17044)
  {
    if (_printDebug == true)
    {
      _debugPort->println(F("enableAlert: not supported on this device"));
    }
    return (MAX17043_GENERIC_ERROR);
  }

  uint16_t statusReg = read16(MAX17048_STATUS);
  statusReg |= MAX1704x_STATUS_EnVR; // Set EnVR bit
  return write16(statusReg, MAX17048_STATUS);
}

uint8_t SFE_MAX1704X::disableAlert(void)
{
  if (_device <= MAX1704X_MAX17044)
  {
    if (_printDebug == true)
    {
      _debugPort->println(F("disableAlert: not supported on this device"));
    }
    return (MAX17043_GENERIC_ERROR);
  }

  uint16_t statusReg = read16(MAX17048_STATUS);
  statusReg &= ~MAX1704x_STATUS_EnVR; // Clear EnVR bit
  return write16(statusReg, MAX17048_STATUS);
}

uint8_t SFE_MAX1704X::getThreshold()
{
  uint16_t configReg = read16(MAX17043_CONFIG);
  uint8_t threshold = (configReg & 0x001F);

  // It has an LSb weight of 1%, and can be programmed from 1% to 32%.
  // The value is (32 - ATHD)%, e.g.: 00000=32%, 00001=31%, 11111=1%.
  // Let's convert our percent to that first:
  threshold = 32 - threshold;
  return threshold;
}

uint8_t SFE_MAX1704X::setThreshold(uint8_t percent)
{
  // The alert threshold is a 5-bit value that sets the state of charge level
  // where an interrupt is generated on the ALRT pin.

  // It has an LSb weight of 1%, and can be programmed from 1% to 32%.
  // The value is (32 - ATHD)%, e.g.: 00000=32%, 00001=31%, 11111=1%.
  // Let's convert our percent to that first:
  percent = constrain(percent, 0, 32);
  percent = 32 - percent;

  // Read config reg, so we don't modify any other values:
  uint16_t configReg = read16(MAX17043_CONFIG);
  configReg &= 0xFFE0;  // Mask out threshold bits
  configReg |= percent; // Add new threshold

  return write16(configReg, MAX17043_CONFIG);
}

// In sleep mode, the IC halts all operations, reducing current
// consumption to below 1μA. After exiting sleep mode,
// the IC continues normal operation. In sleep mode, the
// IC does not detect self-discharge. If the battery changes
// state while the IC sleeps, the IC cannot detect it, causing
// SOC error. Wake up the IC before charging or discharging.
uint8_t SFE_MAX1704X::sleep()
{
  if (_device > MAX1704X_MAX17044)
  {
    // On the MAX17048, we also have to set the EnSleep bit in the MODE register
    uint8_t result =  write16(MAX17048_MODE_ENSLEEP, MAX17043_MODE);
    if (result)
      return (result); // Write failed. Bail.
  }

  // Read config reg, so we don't modify any other values:
  uint16_t configReg = read16(MAX17043_CONFIG);
  if (configReg & MAX17043_CONFIG_SLEEP)
  {
    if (_printDebug == true)
    {
      _debugPort->println(F("sleep: MAX17043 is already sleeping!"));
    }
    return MAX17043_GENERIC_ERROR; // Already sleeping, do nothing but return an error
  }

  configReg |= MAX17043_CONFIG_SLEEP; // Set sleep bit

  return write16(configReg, MAX17043_CONFIG);
}

uint8_t SFE_MAX1704X::wake()
{
  // Read config reg, so we don't modify any other values:
  uint16_t configReg = read16(MAX17043_CONFIG);
  if (!(configReg & MAX17043_CONFIG_SLEEP))
  {
    if (_printDebug == true)
    {
      _debugPort->println(F("wake: MAX17043 is already awake!"));
    }
    return MAX17043_GENERIC_ERROR; // Already sleeping, do nothing but return an error
  }
  configReg &= ~MAX17043_CONFIG_SLEEP; // Clear sleep bit

  uint8_t result = write16(configReg, MAX17043_CONFIG);

  if (result)
    return (result); // Write failed. Bail.

  if (_device > MAX1704X_MAX17044)
  {
    // On the MAX17048, we should also clear the EnSleep bit in the MODE register
    // Strictly, this will clear the QuickStart bit too. Which is probably a good thing,
    // as I don't think we can do a read-modify-write?
    return write16(0x0000, MAX17043_MODE);
  }
  else
  {
    return (result);
  }
}

// Writing a value of 0x5400 to the CMD Register causes
// the device to completely reset as if power had been
// removed (see the Power-On Reset (POR) section). The
// reset occurs when the last bit has been clocked in. The
// IC does not respond with an I2C ACK after this command
// sequence.
// Output: Positive integer on success, 0 on fail.
uint8_t SFE_MAX1704X::reset()
{
  return write16(MAX17043_COMMAND_POR, MAX17043_COMMAND);
}

uint8_t SFE_MAX1704X::getCompensation()
{
  uint16_t configReg = read16(MAX17043_CONFIG);
  uint8_t compensation = (configReg & 0xFF00) >> 8;
  return compensation;
}

uint16_t SFE_MAX1704X::getConfigRegister()
{
  return read16(MAX17043_CONFIG);
}

uint8_t SFE_MAX1704X::setCompensation(uint8_t newCompensation)
{
  // The CONFIG register compensates the ModelGauge algorith. The upper 8 bits
  // of the 16-bit register control the compensation.
  // Read the original configReg, so we can leave the lower 8 bits alone:
  uint16_t configReg = read16(MAX17043_CONFIG);
  configReg &= 0x00FF; // Mask out compensation bits
  configReg |= ((uint16_t)newCompensation) << 8;
  return write16(configReg, MAX17043_CONFIG);
}

// VALRT Register:
//  This register is divided into two thresholds: Voltage alert
//  maximum (VALRT.MAX) and minimum (VALRT. MIN).
//  Both registers have 1 LSb = 20mV. The IC alerts while
//  VCELL > VALRT.MAX or VCELL < VALRT.MIN
uint8_t SFE_MAX1704X::setVALRTMax(uint8_t threshold)
{
  if (_device <= MAX1704X_MAX17044)
  {
    if (_printDebug == true)
    {
      _debugPort->println(F("setVALRTMax: not supported on this device"));
    }
    return (MAX17043_GENERIC_ERROR);
  }

  uint16_t valrt = read16(MAX17048_CVALRT);
  valrt &= 0xFF00; // Mask off max bits
  valrt |= (uint16_t)threshold;
  return write16(valrt, MAX17048_CVALRT);
}
uint8_t SFE_MAX1704X::setVALRTMax(float threshold)
{
  uint8_t thresh = (uint8_t)(constrain(threshold, 0.0, 5.1) / 0.02);
  return setVALRTMax(thresh);
}

uint8_t SFE_MAX1704X::getVALRTMax()
{
  if (_device <= MAX1704X_MAX17044)
  {
    if (_printDebug == true)
    {
      _debugPort->println(F("getVALRTMax: not supported on this device"));
    }
    return (0);
  }

  uint16_t valrt = read16(MAX17048_CVALRT);
  valrt &= 0x00FF; // Mask off max bits
  return ((uint8_t)valrt);
}

uint8_t SFE_MAX1704X::setVALRTMin(uint8_t threshold)
{
  if (_device <= MAX1704X_MAX17044)
  {
    if (_printDebug == true)
    {
      _debugPort->println(F("setVALRTMin: not supported on this device"));
    }
    return (MAX17043_GENERIC_ERROR);
  }

  uint16_t valrt = read16(MAX17048_CVALRT);
  valrt &= 0x00FF; // Mask off min bits
  valrt |= ((uint16_t)threshold) << 8;
  return write16(valrt, MAX17048_CVALRT);
}
uint8_t SFE_MAX1704X::setVALRTMin(float threshold)
{
  uint8_t thresh = (uint8_t)(constrain(threshold, 0.0, 5.1) / 0.02);
  return setVALRTMin(thresh);
}

uint8_t SFE_MAX1704X::getVALRTMin()
{
  if (_device <= MAX1704X_MAX17044)
  {
    if (_printDebug == true)
    {
      _debugPort->println(F("getVALRTMin: not supported on this device"));
    }
    return (0);
  }

  uint16_t valrt = read16(MAX17048_CVALRT);
  valrt >>= 8; // Shift min into LSB
  return ((uint8_t)valrt);
}

bool SFE_MAX1704X::isHibernating()
{
  if (_device <= MAX1704X_MAX17044)
  {
    if (_printDebug == true)
    {
      _debugPort->println(F("isHibernating: not supported on this device"));
    }
    return (false);
  }

  uint16_t mode = read16(MAX17043_MODE);
  return ((mode & MAX17048_MODE_HIBSTAT) > 0);
}

uint8_t SFE_MAX1704X::getHIBRTActThr()
{
  if (_device <= MAX1704X_MAX17044)
  {
    if (_printDebug == true)
    {
      _debugPort->println(F("getHIBRTActThr: not supported on this device"));
    }
    return (0);
  }

  uint16_t hibrt = read16(MAX17048_HIBRT);
  hibrt &= 0x00FF; // Mask off Act bits
  return ((uint8_t)hibrt);
}

uint8_t SFE_MAX1704X::setHIBRTActThr(uint8_t threshold)
{
  if (_device <= MAX1704X_MAX17044)
  {
    if (_printDebug == true)
    {
      _debugPort->println(F("setHIBRTActThr: not supported on this device"));
    }
    return (MAX17043_GENERIC_ERROR);
  }

  uint16_t hibrt = read16(MAX17048_HIBRT);
  hibrt &= 0xFF00; // Mask off Act bits
  hibrt |= (uint16_t)threshold;
  return write16(hibrt, MAX17048_HIBRT);
}
uint8_t SFE_MAX1704X::setHIBRTActThr(float threshold)
{
  // LSb = 1.25mV
  uint8_t thresh = (uint8_t)(constrain(threshold, 0.0, 0.31875) / 0.00125);
  return setHIBRTActThr(thresh);
}

uint8_t SFE_MAX1704X::getHIBRTHibThr()
{
  if (_device <= MAX1704X_MAX17044)
  {
    if (_printDebug == true)
    {
      _debugPort->println(F("getHIBRTHibThr: not supported on this device"));
    }
    return (0);
  }

  uint16_t hibrt = read16(MAX17048_HIBRT);
  hibrt >>= 8; // Shift HibThr into LSB
  return ((uint8_t)hibrt);
}

uint8_t SFE_MAX1704X::setHIBRTHibThr(uint8_t threshold)
{
  if (_device <= MAX1704X_MAX17044)
  {
    if (_printDebug == true)
    {
      _debugPort->println(F("setHIBRTHibThr: not supported on this device"));
    }
    return (MAX17043_GENERIC_ERROR);
  }

  uint16_t hibrt = read16(MAX17048_HIBRT);
  hibrt &= 0x00FF; // Mask off Hib bits
  hibrt |= ((uint16_t)threshold) << 8;
  return write16(hibrt, MAX17048_HIBRT);
}
uint8_t SFE_MAX1704X::setHIBRTHibThr(float threshold)
{
  // LSb = 0.208%/hr
  uint8_t thresh = (uint8_t)(constrain(threshold, 0.0, 53.04) / 0.208);
  return setHIBRTHibThr(thresh);
}

uint8_t SFE_MAX1704X::enableHibernate()
{
  if (_device <= MAX1704X_MAX17044)
  {
    if (_printDebug == true)
    {
      _debugPort->println(F("enableHibernate: not supported on this device"));
    }
    return (MAX17043_GENERIC_ERROR);
  }

  return write16(MAX17048_HIBRT_ENHIB, MAX17048_HIBRT);
}

uint8_t SFE_MAX1704X::disableHibernate()
{
  if (_device <= MAX1704X_MAX17044)
  {
    if (_printDebug == true)
    {
      _debugPort->println(F("disableHibernate: not supported on this device"));
    }
    return (MAX17043_GENERIC_ERROR);
  }

  return write16(MAX17048_HIBRT_DISHIB, MAX17048_HIBRT);
}

uint8_t SFE_MAX1704X::write16(uint16_t data, uint8_t address)
{
  uint8_t msb, lsb;
  msb = (data & 0xFF00) >> 8;
  lsb = (data & 0x00FF);
  _i2cPort->beginTransmission(MAX1704x_ADDRESS);
  _i2cPort->write(address);
  _i2cPort->write(msb);
  _i2cPort->write(lsb);
  return (_i2cPort->endTransmission());
}

uint16_t SFE_MAX1704X::read16(uint8_t address)
{
  uint8_t msb, lsb;
  int16_t timeout = 1000;

  _i2cPort->beginTransmission(MAX1704x_ADDRESS);
  _i2cPort->write(address);
  _i2cPort->endTransmission(false);

  _i2cPort->requestFrom(MAX1704x_ADDRESS, 2);
  while ((_i2cPort->available() < 2) && (timeout-- > 0))
    delay(1);
  msb = _i2cPort->read();
  lsb = _i2cPort->read();

  return ((uint16_t)msb << 8) | lsb;
}
