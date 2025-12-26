/*!
 * @file INA.cpp
 *
 * @section INAcpp_intro_section Description
 *
 * Arduino Library for accessing the INA2xx Family of power measurement devices\n\n
 * See main library header file "INA.h" for details and license information
 *
 */
#include <INA.h>   ///< Include the header definition
#include <Wire.h>  ///< I2C Library definition
#if defined(__AVR__) || defined(CORE_TEENSY) || defined(ESP32) || defined(ESP8266) || \
    defined(STM32F1)
  #if LIB_INA_EEPROM
  #include <EEPROM.h>  ///< Include the EEPROM library for AVR-Boards
  #endif // if LIB_INA_EEPROM
#endif
inaDet::inaDet() {}  ///< constructor for INA Detail class
inaDet::inaDet(inaEEPROM &inaEE) {
  /*! @brief     INA Detail Class Constructor (Overloaded)
      @details   Construct the class using the saved EEPROM data structure
      @param[in] inaEE Saved EEPROM Values */
  type          = inaEE.type;
  operatingMode = inaEE.operatingMode;
  address       = inaEE.address;
  maxBusAmps    = inaEE.maxBusAmps;
  microOhmR     = inaEE.microOhmR;
  current_LSB   = (uint64_t)maxBusAmps * 1000000000 / 32767;  // Get the best possible LSB in nA
  power_LSB     = (uint32_t)20 * current_LSB;                 // Default multiplier per device
  switch (type) {
    case INA219:
      busVoltageRegister   = INA_BUS_VOLTAGE_REGISTER;
      shuntVoltageRegister = INA219_SHUNT_VOLTAGE_REGISTER;
      currentRegister      = INA219_CURRENT_REGISTER;
      busVoltage_LSB       = INA219_BUS_VOLTAGE_LSB;
      shuntVoltage_LSB     = INA219_SHUNT_VOLTAGE_LSB;
      break;
    case INA226:
    case INA230:
    case INA231:
      power_LSB            = (uint32_t)25 * current_LSB;  // issue #66 corrected multiplier
      busVoltageRegister   = INA_BUS_VOLTAGE_REGISTER;
      shuntVoltageRegister = INA226_SHUNT_VOLTAGE_REGISTER;
      currentRegister      = INA226_CURRENT_REGISTER;
      busVoltage_LSB       = INA226_BUS_VOLTAGE_LSB;
      shuntVoltage_LSB     = INA226_SHUNT_VOLTAGE_LSB;
      break;

    case INA228:
      power_LSB            = 0;  // TODO
      busVoltageRegister   = INA228_BUS_VOLTAGE_REGISTER;
      busVoltage_LSB       = INA228_BUS_VOLTAGE_LSB;
      shuntVoltageRegister = INA228_SHUNT_VOLTAGE_REGISTER;

      currentRegister  = INA226_CURRENT_REGISTER;
      shuntVoltage_LSB = INA226_SHUNT_VOLTAGE_LSB;
      break;

    case INA260:
      busVoltageRegister   = INA_BUS_VOLTAGE_REGISTER;
      shuntVoltageRegister = INA260_SHUNT_VOLTAGE_REGISTER;  // Register not present
      currentRegister      = INA260_CURRENT_REGISTER;
      busVoltage_LSB       = INA260_BUS_VOLTAGE_LSB;
      current_LSB          = 1250000;   // Fixed LSB of 1.25mv
      power_LSB            = 10000000;  // Fixed multiplier per device
      break;
    case INA3221_0:
    case INA3221_1:
    case INA3221_2:
      busVoltageRegister   = INA_BUS_VOLTAGE_REGISTER;
      shuntVoltageRegister = INA3221_SHUNT_VOLTAGE_REGISTER;
      currentRegister      = 0;  // INA3221 has no current Reg
      busVoltage_LSB       = INA3221_BUS_VOLTAGE_LSB;
      shuntVoltage_LSB     = INA3221_SHUNT_VOLTAGE_LSB;
      current_LSB          = 0;  // INA3221 has no current reg.
      power_LSB            = 0;  // INA3221 has no power reg.
      if (type == INA3221_1) {
        busVoltageRegister += 2;    // Reg for 2nd bus voltage
        shuntVoltageRegister += 2;  // Reg for 2nd shunt voltage
      } else {
        if (type == INA3221_2) {
          busVoltageRegister += 4;    // Reg for 3rd bus voltage
          shuntVoltageRegister += 4;  // Reg for 3rd shunt voltage
        }                             // of if-then INA322_2
      }                               // of if-then-else INA3221_1
      break;
  }  // of switch type
}  // of constructor
INA_Class::INA_Class(uint8_t expectedDevices) : _expectedDevices(expectedDevices) {
  /*!
@brief   Class constructor
@details If called without a parameter or with a 0 value, then the constructor does nothing,
         but if a value is passed then using EEPROM is disabled and each INA-Device found
         has its data (inaEEPROM structure size) stored in a array dynamically allocated during
         library instatiation here. If there is not enough space then the pointer isn't init-
         ialized and the program will abort later on. No error checking can be done here
@param[in] expectedDevices Number of elements to initialize array to if non-zero
*/
  if (_expectedDevices) {
    _DeviceArray = new inaEEPROM[_expectedDevices];
  }  // if-then use memory rather than EEPROM
}  // of class constructor
INA_Class::~INA_Class() {
  /*!
  @brief   Class destructor
  @details If dynamic memory has been allocated for device storage rather than the default EEPROM,
           then that memory is freed here; otherwise the destructor does nothing
  */
  if (_expectedDevices) { delete[] _DeviceArray; }  // if-then use memory rather than EEPROM
}  // of class destructor
int16_t INA_Class::readWord(const uint8_t addr, const uint8_t deviceAddress) const {
  /*! @brief     Read one word (2 bytes) from the specified I2C address
      @details   Standard I2C protocol is used, but a delay of I2C_DELAY microseconds has been
                 added to let the INAxxx devices have sufficient time to get the return data ready
      @param[in] addr I2C address to read from
      @param[in] deviceAddress Address on the I2C device to read from
      @return    integer value read from the I2C device */
  Wire.beginTransmission(deviceAddress);        // Address the I2C device
  Wire.write(addr);                             // Send register address to read
  Wire.endTransmission();                       // Close transmission
  delayMicroseconds(I2C_DELAY);                 // delay required for sync
  Wire.requestFrom(deviceAddress, (uint8_t)2);  // Request 2 consecutive bytes
  return ((uint16_t)Wire.read() << 8) | Wire.read();
}  // of method readWord()
int32_t INA_Class::read3Bytes(const uint8_t addr, const uint8_t deviceAddress) const {
  /*! @brief     Read 3 bytes from the specified I2C address
      @details   Standard I2C protocol is used, but a delay of I2C_DELAY microseconds has been
                 added to let the INAxxx devices have sufficient time to get the return data ready
      @param[in] addr I2C address to read from
      @param[in] deviceAddress Address on the I2C device to read from
      @return    integer value read from the I2C device */
  Wire.beginTransmission(deviceAddress);        // Address the I2C device
  Wire.write(addr);                             // Send register address to read
  Wire.endTransmission();                       // Close transmission
  delayMicroseconds(I2C_DELAY);                 // delay required for sync
  Wire.requestFrom(deviceAddress, (uint8_t)3);  // Request 3 consecutive bytes
  return ((uint32_t)Wire.read() << 16) | ((uint32_t)Wire.read() << 8) | ((uint32_t)Wire.read());
}  // of method readWord()
void INA_Class::writeWord(const uint8_t addr, const uint16_t data,
                          const uint8_t deviceAddress) const {
  /*! @brief     Write 2 bytes to the specified I2C address
      @details   Standard I2C protocol is used, but a delay of I2C_DELAY microseconds has been
                 added to let the INAxxx devices have sufficient time to process the data
      @param[in] addr I2C address to write to
      @param[in] data 2 Bytes to write to the device
      @param[in] deviceAddress Address on the I2C device to write to */
  Wire.beginTransmission(deviceAddress);  // Address the I2C device
  Wire.write(addr);                       // Send register address to write
  Wire.write((uint8_t)(data >> 8));       // Write the first (MSB) byte
  Wire.write((uint8_t)data);              // and then the second byte
  Wire.endTransmission();                 // Close transmission and actually send data
  delayMicroseconds(I2C_DELAY);           // delay required for sync
}  // of method writeWord()
void INA_Class::readInafromEEPROM(const uint8_t deviceNumber) {
  /*! @brief     Read INA device information from EEPROM
      @details   Retrieve the stored information for a device from EEPROM. Since this method is
                 private and access is controlled, no range error checking is performed
      @param[in] deviceNumber Index to device array */
  if (deviceNumber == _currentINA || deviceNumber > _DeviceCount) return;  // Skip if correct device
#if LIB_INA_EEPROM
  if (_expectedDevices == 0) {
#if defined(__AVR__) || defined(CORE_TEENSY) || defined(ESP32) || defined(ESP8266) || (__STM32F1__)
  #ifdef __STM32F1__                                          // STM32F1 has no built-in EEPROM
    uint16_t  e   = deviceNumber * sizeof(inaEE);             // it uses flash memory to emulate
    uint16_t *ptr = (uint16_t *)&inaEE;                       // "EEPROM" calls are uint16_t type
    for (uint8_t n = sizeof(inaEE) + _EEPROM_offset; n; --n)  // Implement EEPROM.get template
    {
      EEPROM.read(e++, ptr++);  // for ina (inaDet type)
    }                           // of for-next each byte
  #else
    EEPROM.get(_EEPROM_offset + (deviceNumber * sizeof(inaEE)), inaEE);  // Read EEPROM values
  #endif
#else
    inaEE                          = _EEPROMEmulation[deviceNumber];
#endif
  } else 
#endif // if LIB_INA_EEPROM
  {
    inaEE = _DeviceArray[deviceNumber];
  }  // if-then-else use EEPROM
  _currentINA = deviceNumber;
  ina         = inaEE;  // see inaDet constructor
}  // of method readInafromEEPROM()
void INA_Class::writeInatoEEPROM(const uint8_t deviceNumber) {
  /*! @brief     Write INA device information to EEPROM
      @details   Write the stored information for a device from EEPROM. Since this method is
                 private and access is controlled, no range error checking is performed
      @param[in] deviceNumber Index to device array */
  inaEE = ina;  // only save relevant part of ina to EEPROM
#if LIB_INA_EEPROM
  if (_expectedDevices == 0) {
#if defined(__AVR__) || defined(CORE_TEENSY) || defined(ESP32) || defined(ESP8266) || (__STM32F1__)
  #ifdef __STM32F1__                                          // STM32F1 has no built-in EEPROM
    uint16_t        e   = deviceNumber * sizeof(inaEE);       // it uses flash memory to emulate
    const uint16_t *ptr = (const uint16_t *)&inaEE;           // "EEPROM" calls are uint16_t type
    for (uint8_t n = sizeof(inaEE) + _EEPROM_offset; n; --n)  // Implement EEPROM.put template
    {
      EEPROM.update(e++, *ptr++);  // for ina (inaDet type)
    }                              // for-next
  #else
    EEPROM.put(_EEPROM_offset + (deviceNumber * sizeof(inaEE)), inaEE);  // Write the structure
    #ifdef ESP32
    EEPROM.commit();                                                     // Force write to EEPROM when ESP32
    #endif
  #endif
#else
    _EEPROMEmulation[deviceNumber] = inaEE;
#endif
  } else
#endif // if LIB_INA_EEPROM
  {
    _DeviceArray[deviceNumber] = inaEE;
  }  // if-then-else use EEPROM to store data
}  // of method writeInatoEEPROM()
void INA_Class::setI2CSpeed(const uint32_t i2cSpeed) const {
  /*! @brief     Set a new I2C speed
      @details   I2C allows various bus speeds, see the enumerated type I2C_MODES for the standard
                 speeds. The valid speeds are  100KHz, 400KHz, 1MHz and 3.4MHz. Default to 100KHz
                 when not specified. No range checking is done.
      @param[in] i2cSpeed [optional] changes the I2C speed to the rate specified in Herz */
  // Wire.setClock(i2cSpeed); // Ignore, I2C speed set on ESPEasy Hardware page
}  // of method setI2CSpeed
uint8_t INA_Class::begin(const uint16_t maxBusAmps, const uint32_t microOhmR,
                         const uint8_t deviceNumber) {
  /*! @brief     Initializes the contents of the class
      @details   Searches for possible devices and sets the INA Configuration details, without
                 which meaningful readings cannot be made. If it is called without the optional
                 deviceNumber parameter then the settings are applied to all devices, otherwise
                 just that specific device is targeted. If the optional third parameter, devNo, is
                 specified that specific device gets the two specified values set for it. Can be
                 called multiple times, but the 3 parameter version will only function after the 2
                 parameter version finds all devices.\n
      @param[in] maxBusAmps Integer value holding the maximum expected bus amperage, this value is
                 used to compute a device's internal power register
      @param[in] microOhmR Shunt resistance in micro-ohms, this value is used to compute a
                 device's internal power register
      @param[in] deviceNumber Device number to explicitly set the maxBusAmps and microOhmR values,
                 by default all devices found get set to the same initial values for these 2 params
      @return    The integer number of INAxxxx devices found on the I2C bus
  */
  uint16_t originalRegister, tempRegister;
  if (_DeviceCount == 0)  // Enumerate all devices on first call
  {
    uint16_t maxDevices = 32;
/***************************************************************************************************
** The AVR devices need to use EEPROM to save memory, some other devices have emulation for EEPROM**
** functionality while some devices have no such function calls. This library caters for these    **
** differences, with specialized calls for those platforms which have EEPROM calls and it makes   **
** the assumption that if the platform has no EEPROM call then it has sufficient RAM available at **
** runtime to allocate sufficient space for 32 devices.                                           **
***************************************************************************************************/
#if defined(ESP32) || defined(ESP8266)
    #if LIB_INA_EEPROM
    EEPROM.begin(_EEPROM_size + _EEPROM_offset);  // If ESP32 then allocate 512 Bytes
    #endif // if LIB_INA_EEPROM
    maxDevices = (_EEPROM_size) / sizeof(inaEE);  // and compute number of devices
#elif defined(__STM32F1__)                        // Emulated EEPROM for STM32F1
    maxDevices                     = (EEPROM.maxcount() - _EEPROM_offset) / sizeof(inaEE);  // Compute max possible
#elif defined(CORE_TEENSY)                        // TEENSY doesn't have EEPROM.length
    maxDevices = (2048 - _EEPROM_offset) / sizeof(inaEE);  // defined, so use 2Kb as value
#elif defined(__AVR__)
    maxDevices = (EEPROM.length() - _EEPROM_offset) / sizeof(inaEE);  // Compute max possible
#else
    maxDevices = 32;
#endif
    // Wire.begin(); // I2C is already started in ESPEasy core

    if (maxDevices > 255)  // Limit number of devices to an 8-bit number
    {
      maxDevices = 255;
    }  // of if-then more than 255 devices possible
    for (uint8_t deviceAddress = 0x40; deviceAddress <= 0x4F;
         deviceAddress++)  // Loop for each I2C addr
    {
      Wire.beginTransmission(deviceAddress);
      uint8_t good = Wire.endTransmission();
      if (good == 0 && _DeviceCount < maxDevices)  // If no error and EEPROM has space
      {
        originalRegister = readWord(INA_CONFIGURATION_REGISTER, deviceAddress);  // Save settings
        writeWord(INA_CONFIGURATION_REGISTER, INA_RESET_DEVICE, deviceAddress);  // Force reset
        tempRegister = readWord(INA_CONFIGURATION_REGISTER, deviceAddress);      // Read reset reg.
        if (tempRegister == INA_RESET_DEVICE)  // If the register wasn't reset then not an INA
        {
          writeWord(INA_CONFIGURATION_REGISTER, originalRegister, deviceAddress);  // restore value
        } else {
          if (tempRegister == 0x399F) {
            inaEE.type = INA219;
          } else {
            if (tempRegister == 0x4127)  // INA226, INA230, INA231
            {
              tempRegister = readWord(INA_DIE_ID_REGISTER, deviceAddress);  // Read the INA high-reg
              if (tempRegister == INA226_DIE_ID_VALUE) {
                inaEE.type = INA226;
              } else {
                if (tempRegister != 0) {
                  inaEE.type = INA230;
                } else {
                  inaEE.type = INA231;
                }  // of if-then-else a INA230 or INA231
              }    // of if-then-else an INA226
            } else {
              if (tempRegister == 0x6127) {
                inaEE.type = INA260;
              } else {
                if (tempRegister == 0x7127) {
                  inaEE.type = INA3221_0;
                } else {
                  if (tempRegister == 0x0) {
                    inaEE.type = INA228;
                  } else {
                    inaEE.type = INA_UNKNOWN;
                  }                       // of if-then-else it is an INA228
                }                         // of if-then-else it is an INA3221
              }                           // of if-then-else it is an INA260
            }                             // of if-then-else it is an INA226, INA230, INA231
          }                               // of if-then-else it is an INA209, INA219, INA220
          if (inaEE.type != INA_UNKNOWN)  // Increment device if valid INA2xx
          {
            inaEE.address    = deviceAddress;
            inaEE.maxBusAmps = maxBusAmps > 1022 ? 1022 : maxBusAmps;  // Clamp to maximum of 1022A
            inaEE.microOhmR  = microOhmR;
            ina              = inaEE;  // see inaDet constructor
            if (inaEE.type == INA3221_0) {
              ina.type = INA3221_0;  // Set to INA3221 1st channel
              initDevice(_DeviceCount);
              _DeviceCount = ((_DeviceCount + 1) % maxDevices);
              ina.type     = INA3221_1;  // Set to INA3221 2nd channel
              initDevice(_DeviceCount);
              _DeviceCount = ((_DeviceCount + 1) % maxDevices);
              ina.type     = INA3221_2;  // Set to INA3221 3rd channel
              initDevice(_DeviceCount);
              _DeviceCount = ((_DeviceCount + 1) % maxDevices);
            } else {
              initDevice(_DeviceCount);                          // perform initialization on device
              _DeviceCount = ((_DeviceCount + 1) % maxDevices);  // start again at 0 if overflow
            }                                                    // of if-then inaEE.type
          }                                                      // of if-then we can add device
        }  // of if-then-else we have an INA-Type device
      }    // of if-then we have a device
    }      // for-next each possible I2C address
  } else {
    readInafromEEPROM(deviceNumber);                         // Load EEPROM to ina structure
    ina.maxBusAmps = maxBusAmps > 1022 ? 1022 : maxBusAmps;  // Clamp to maximum of 1022A
    ina.microOhmR  = microOhmR;
    initDevice(deviceNumber);
  }                         // of if-then-else first call
  _currentINA = UINT8_MAX;  // Force read on next call
  return _DeviceCount;
}  // of method begin()
void INA_Class::initDevice(const uint8_t deviceNumber) {
  /*! @brief     Initializes the the given devices using the settings from the internal structure
      @details   This includes (re)computing the device's calibration values.
      @param[in] deviceNumber Device number to explicitly initialize. */
  ina.operatingMode = INA_DEFAULT_OPERATING_MODE;  // Default to continuous mode
  writeInatoEEPROM(deviceNumber);                  // Store the structure to EEPROM
  uint8_t  programmableGain;                       // work variable for the programmable gain
  uint16_t calibration, maxShuntmV, tempRegister;  // Calibration temporary variables
  switch (ina.type) {
    case INA219:  // Set up INA219 or INA220
      // Compute calibration register
      calibration = (uint64_t)409600000 /
                    ((uint64_t)ina.current_LSB * (uint64_t)ina.microOhmR / (uint64_t)100000);
      writeWord(INA_CALIBRATION_REGISTER, calibration, ina.address);  // Write calibration
      /* Determine optimal programmable gain with maximum accuracy so no chance of an overflow */
      maxShuntmV = ina.maxBusAmps * ina.microOhmR / 1000;  // Compute maximum shunt mV
      if (maxShuntmV <= 40)
        programmableGain = 0;  // gain x1 for +- 40mV
      else if (maxShuntmV <= 80)
        programmableGain = 1;  // gain x2 for +- 80mV
      else if (maxShuntmV <= 160)
        programmableGain = 2;  // gain x4 for +- 160mV
      else
        programmableGain = 3;                                   // dflt gain x8 for +- 320mV
      tempRegister = 0x399F & INA219_CONFIG_PG_MASK;            // Zero programmable gain
      tempRegister |= programmableGain << INA219_PG_FIRST_BIT;  // Overwrite the new values
      bitSet(tempRegister, INA219_BRNG_BIT);                    // set to 1 for 0-32 volts
      writeWord(INA_CONFIGURATION_REGISTER, tempRegister, ina.address);  // Write to config register
      break;
    case INA226:
    case INA230:
    case INA231:
      // Compute calibration register
      calibration = (uint64_t)51200000 /
                    ((uint64_t)ina.current_LSB * (uint64_t)ina.microOhmR / (uint64_t)100000);
      writeWord(INA_CALIBRATION_REGISTER, calibration, ina.address);  // Write calibration
      break;
    case INA260:
    case INA3221_0:
    case INA3221_1:
    case INA3221_2: break;
  }  // of switch type
}  // of method initDevice()
void INA_Class::setBusConversion(const uint32_t convTime, const uint8_t deviceNumber) {
  /*! @brief     specifies the conversion rate in microseconds, rounded to the nearest valid value
      @details   INA devices can have a conversion rate of up to 68100 microseconds
      @param[in] convTime The conversion time in microseconds, invalid values are rounded to the
                 nearest valid value
      @param[in] deviceNumber [optional] When specified, only that specified device number gets
                 changed, otherwise all devices are set to the same averaging rate
  */
  uint16_t configRegister;
  int16_t  convRate;
  for (uint8_t i = 0; i < _DeviceCount; i++)  // Loop for each device found
  {
    if (deviceNumber == UINT8_MAX || deviceNumber % _DeviceCount == i)  // If device needs setting
    {
      readInafromEEPROM(i);  // Load EEPROM values to ina structure
      configRegister = readWord(INA_CONFIGURATION_REGISTER, ina.address);  // Get current register
      switch (ina.type) {
        case INA219:
          if (convTime >= 68100)
            convRate = 15;
          else if (convTime >= 34050)
            convRate = 14;
          else if (convTime >= 17020)
            convRate = 13;
          else if (convTime >= 8510)
            convRate = 12;
          else if (convTime >= 4260)
            convRate = 11;
          else if (convTime >= 2130)
            convRate = 10;
          else if (convTime >= 1060)
            convRate = 9;
          else if (convTime >= 532)
            convRate = 8;
          else if (convTime >= 276)
            convRate = 2;
          else if (convTime >= 148)
            convRate = 1;
          else
            convRate = 0;
          configRegister &= ~INA219_CONFIG_BADC_MASK;  // zero out the averages part
          configRegister |= convRate << 7;             // shift in the BADC averages
          break;
        case INA226:
        case INA230:
        case INA231:
        case INA3221_0:
        case INA3221_1:
        case INA3221_2:
        case INA260:
          if (convTime >= 8244)
            convRate = 7;
          else if (convTime >= 4156)
            convRate = 6;
          else if (convTime >= 2116)
            convRate = 5;
          else if (convTime >= 1100)
            convRate = 4;
          else if (convTime >= 588)
            convRate = 3;
          else if (convTime >= 332)
            convRate = 2;
          else if (convTime >= 204)
            convRate = 1;
          else
            convRate = 0;
          if (ina.type == INA226 || ina.type == INA3221_0 || ina.type == INA3221_1 ||
              ina.type == INA3221_2) {
            configRegister &= ~INA226_CONFIG_BADC_MASK;  // zero out the averages part
            configRegister |= convRate << 6;             // shift in averages
          } else {
            configRegister &= ~INA260_CONFIG_BADC_MASK;  // zero out the averages part
            configRegister |= convRate << 7;             // shift in the averages
          }                                              // of if-then an INA226 or INA260
          break;
      }  // of switch type
      writeWord(INA_CONFIGURATION_REGISTER, configRegister,
                ina.address);  // Save new value to device
    }                          // of if this device needs to be set
  }                            // for-next each device loop
}  // of method setBusConversion()
void INA_Class::setShuntConversion(const uint32_t convTime, const uint8_t deviceNumber) {
  /*! @brief     specifies the conversion rate in microseconds, rounded to the nearest valid value
      @details   INA devices can have a conversion rate of up to 68100 microseconds
      @param[in] convTime Conversion time in microseconds. Out-of-Range values are set to the
                 closest valid value
      @param[in] deviceNumber to return the device name for[optional] When specified, only that
                 specified device number gets changed, otherwise all devices are set to the same
                 averaging rate
                 */
  int16_t configRegister, convRate;
  for (uint8_t i = 0; i < _DeviceCount; i++) {  // Loop for each device found
    if (deviceNumber == UINT8_MAX ||
        deviceNumber % _DeviceCount == i)  // If this device needs setting
    {
      readInafromEEPROM(i);  // Load EEPROM to ina structure
      configRegister = readWord(INA_CONFIGURATION_REGISTER, ina.address);  // Get register contents
      switch (ina.type) {
        case INA219:
          if (convTime >= 68100)
            convRate = 15;
          else if (convTime >= 34050)
            convRate = 14;
          else if (convTime >= 17020)
            convRate = 13;
          else if (convTime >= 8510)
            convRate = 12;
          else if (convTime >= 4260)
            convRate = 11;
          else if (convTime >= 2130)
            convRate = 10;
          else if (convTime >= 1060)
            convRate = 9;
          else if (convTime >= 532)
            convRate = 8;
          else if (convTime >= 276)
            convRate = 2;
          else if (convTime >= 148)
            convRate = 1;
          else
            convRate = 0;
          configRegister &= ~INA219_CONFIG_SADC_MASK;  // zero out the averages part
          configRegister |= convRate << 3;             // shift in the SADC averages
          break;
        case INA226:
        case INA230:
        case INA231:
        case INA3221_0:
        case INA3221_1:
        case INA3221_2:
        case INA260:
          if (convTime >= 8244)
            convRate = 7;
          else if (convTime >= 4156)
            convRate = 6;
          else if (convTime >= 2116)
            convRate = 5;
          else if (convTime >= 1100)
            convRate = 4;
          else if (convTime >= 588)
            convRate = 3;
          else if (convTime >= 332)
            convRate = 2;
          else if (convTime >= 204)
            convRate = 1;
          else
            convRate = 0;
          if (ina.type == INA226 || ina.type == INA3221_0 || ina.type == INA3221_1 ||
              ina.type == INA3221_2) {
            configRegister &= ~INA226_CONFIG_SADC_MASK;  // zero out the averages part
          } else {
            configRegister &= ~INA260_CONFIG_SADC_MASK;  // zero out the averages part
          }                                 // of if-then-else either INA226/INA3221 or a INA260
          configRegister |= convRate << 3;  // shift in the averages to register
          break;
      }  // of switch type
      writeWord(INA_CONFIGURATION_REGISTER, configRegister,
                ina.address);  // Save new value to device
    }                          // of if this device needs to be set
  }                            // for-next each device loop
}  // of method setShuntConversion()
const char *INA_Class::getDeviceName(const uint8_t deviceNumber) {
  /*! @brief     returns character buffer with the name of the device specified in the input param
      @details   See function definition for list of possible return values
      @param[in] deviceNumber to return the device name of
      @return    device name */
  if (deviceNumber > _DeviceCount) return ("");
  readInafromEEPROM(deviceNumber);  // Load EEPROM to ina structure
  switch (ina.type) {
    case INA219: return ("INA219");
    case INA226: return ("INA226");
    case INA228: return ("INA228");
    case INA230: return ("INA230");
    case INA231: return ("INA231");
    case INA260: return ("INA260");
    case INA3221_0:
    case INA3221_1:
    case INA3221_2: return ("INA3221");
    default: return ("UNKNOWN");
  }  // of switch type
}  // of method getDeviceName()
uint8_t INA_Class::getDeviceType(const uint8_t deviceNumber) {
  /*! @brief     returns uint8_t type of the device specified in the input param
      @details   See function definition for list of possible return values
      @param[in] deviceNumber to return the device name of
      @return    device name */
  if (deviceNumber > _DeviceCount) return INA_UNKNOWN;
  readInafromEEPROM(deviceNumber);  // Load EEPROM to ina structure
  return ina.type;
}  // of method getDeviceType()
uint8_t INA_Class::getDeviceAddress(const uint8_t deviceNumber) {
  /*! @brief     returns a I2C address of the device specified in the input parameter
      @details   Return the I2C address of the specified device, if number is out of range return 0
      @param[in] deviceNumber to return the device name of
      @return    I2C address of the device. Returns 0 if value is out-of-range
      */
  if (deviceNumber > _DeviceCount) return 0;
  readInafromEEPROM(deviceNumber);  // Load EEPROM to ina structure
  return (ina.address);
}  // of method getDeviceAddress()
uint16_t INA_Class::getBusMilliVolts(const uint8_t deviceNumber) {
  /*! @brief     returns the bus voltage in millivolts
      @details   The converted millivolt value is returned and if the device is in triggered mode
                 the next conversion is started
      @param[in] deviceNumber to return the device bus millivolts for
      @return uint16_t unsigned integer for the bus millivoltage */
  uint32_t busVoltage = getBusRaw(deviceNumber);  // Get raw voltage from device
  if (ina.type == INA228) {
    // The accuracy is 20bits and 195.3125uv is the LSB
    busVoltage = (uint64_t)busVoltage * 1953125 / 10000000;  // conversion to get mV
  } else {
    busVoltage = busVoltage * ina.busVoltage_LSB / 100;  // conversion to get mV
  }                                                      // if-then-else an INA228
  return (busVoltage);
}  // of method getBusMilliVolts()
uint32_t INA_Class::getBusRaw(const uint8_t deviceNumber) {
  /*! @brief     returns the raw unconverted bus voltage reading from the device
      @details   The raw measured value is returned and if the device is in triggered mode the next
                 conversion is started
      @param[in] deviceNumber to return the raw device bus voltage reading
      @return    Raw bus measurement */
  readInafromEEPROM(deviceNumber);  // Load EEPROM from EEPROM
  uint32_t raw{0};                  // define the return variable
  if (ina.type == INA228) {
    raw = read3Bytes(ina.busVoltageRegister, ina.address);  // Get the raw value from register
    raw = raw >> 4;
  } else {
    raw = readWord(ina.busVoltageRegister, ina.address);  // Get the raw value from register
    if (ina.type == INA3221_0 || ina.type == INA3221_1 || ina.type == INA3221_2 ||
        ina.type == INA219) {
      raw = raw >> 3;  // INA219 & INA3221 - the 3 LSB unused, so shift right
    }                  // of if-then an INA219 or INA3221
  }                    // if-then a 3byte bus voltage buffer
  if (!bitRead(ina.operatingMode, 2) && bitRead(ina.operatingMode, 1))  // Triggered & bus active
  {
    int16_t configRegister =
        readWord(INA_CONFIGURATION_REGISTER, ina.address);               // Get current value
    writeWord(INA_CONFIGURATION_REGISTER, configRegister, ina.address);  // Write to trigger next
  }  // of if-then triggered mode enabled
  return (raw);
}  // of method getBusRaw()
int32_t INA_Class::getShuntMicroVolts(const uint8_t deviceNumber) {
  /*! @brief     returns the shunt reading converted to microvolts
      @details   The computed microvolts value is returned and if the device is in triggered mode
                 the next conversion is started
      @param[in] deviceNumber to return the value for
      @return    int32_t signed integer for the shunt microvolts
      */
  int32_t shuntVoltage = getShuntRaw(deviceNumber);
  if (ina.type == INA260)  // INA260 has a built-in shunt
  {
    int32_t busMicroAmps = getBusMicroAmps(deviceNumber);  // Get the amps on the bus from device
    shuntVoltage         = busMicroAmps / 200;             // 2mOhm resistor, convert with Ohm's law
  } else {
    if (ina.type == INA228) {
      shuntVoltage = shuntVoltage * ina.shuntVoltage_LSB / 10;  // Convert to microvolts
    } else {
      shuntVoltage = shuntVoltage * ina.shuntVoltage_LSB / 10;  // Convert to microvolts
    }  // if-then a INA228 with 20 bit accuracy
  }    // of if-then-else an INA260
  return (shuntVoltage);
}  // of method getShuntMicroVolts()
int32_t INA_Class::getShuntRaw(const uint8_t deviceNumber) {
  /*! @brief     Returns the raw shunt reading
      @details   The raw reading is returned and if the device is in triggered mode the next
                 conversion is started
      @param[in] deviceNumber to return the value for
      @return    Raw shunt reading */
  int32_t raw;
  readInafromEEPROM(deviceNumber);  // Load EEPROM to ina structure
  if (ina.type == INA260)           // INA260 has a built-in shunt
  {
    int32_t busMicroAmps = getBusMicroAmps(deviceNumber);  // Get the amps on the bus
    raw                  = busMicroAmps / 200 / 1000;      // 2mOhm resistor, apply Ohm's law
  } else {
    if (ina.type == INA228)  // INA228 has 24 bit accuracy
    {
      raw = read3Bytes(ina.shuntVoltageRegister, ina.address);  // Get the raw value from register
      // The number is two's complement, so if negative we need to pad when shifting //
      if (raw & 0x800000) {
        raw = (raw >> 4) | 0xFFF00000;  // first 12 bits are "1"
      } else {
        raw = raw >> 4;
      }  // if-then negative
    } else {
      raw = readWord(ina.shuntVoltageRegister, ina.address);  // Get the raw value from register
    }                                                         // if-then a 24 bit register
    if (ina.type == INA3221_0 || ina.type == INA3221_1 ||
        ina.type == INA3221_2)  // Doesn't use 3 LSB
    {
      raw = raw >> 3;  // shift over 3 bits, datatype is "int" so shifts in sign bits
    }                  // of if-then we need to shift INA3221 reading over
  }                    // of if-then-else an INA260 with inbuilt shunt
  if (!bitRead(ina.operatingMode, 2) && bitRead(ina.operatingMode, 0))  // Triggered & shunt active
  {
    int16_t configRegister = readWord(INA_CONFIGURATION_REGISTER, ina.address);  // Get current reg
    writeWord(INA_CONFIGURATION_REGISTER, configRegister, ina.address);  // Write to trigger next
  }  // of if-then triggered mode enabled
  return (raw);
}  // of method getShuntMicroVolts()
int32_t INA_Class::getBusMicroAmps(const uint8_t deviceNumber) {
  /*! @brief     Returns the computed microamps measured on the bus for the specified device
      @details   The computed reading is returned and if the device is in triggered mode the next
                 conversion is started
      @param[in] deviceNumber to return the value for
      @return    int32_t signed integer for computed microamps on the bus */
  readInafromEEPROM(deviceNumber);  // Load EEPROM to ina structure
  int32_t microAmps = 0;
  if (ina.type == INA3221_0 || ina.type == INA3221_1 ||
      ina.type == INA3221_2)  // Doesn't compute Amps
  {
    microAmps =
        (int64_t)getShuntMicroVolts(deviceNumber) * ((int64_t)1000000 / (int64_t)ina.microOhmR);
  } else {
    microAmps = (int64_t)readWord(ina.currentRegister, ina.address) * (int64_t)ina.current_LSB /
                (int64_t)1000;
  }  // of if-then-else an INA3221
  return (microAmps);
}  // of method getBusMicroAmps()
int64_t INA_Class::getBusMicroWatts(const uint8_t deviceNumber) {
  /*!
  @brief     returns the computed microwatts measured on the bus for the specified device
  @details   The computed reading is returned and if the device is in triggered mode the next
             conversion is started
  @param[in] deviceNumber to return the value for
  @return    int64_t signed integer for computed microwatts on the bus
  */
  int64_t microWatts = 0;
  readInafromEEPROM(deviceNumber);  // Load EEPROM to ina structure
  if (ina.type == INA3221_0 || ina.type == INA3221_1 ||
      ina.type == INA3221_2)  // Doesn't compute Amps
  {
    microWatts =
        ((int64_t)getShuntMicroVolts(deviceNumber) * (int64_t)1000000 / (int64_t)ina.microOhmR) *
        (int64_t)getBusMilliVolts(deviceNumber) / (int64_t)1000;
  } else {
    microWatts =
        (int64_t)readWord(INA_POWER_REGISTER, ina.address) * (int64_t)ina.power_LSB / (int64_t)1000;
    if (getShuntRaw(deviceNumber) < 0) microWatts *= -1;  // Invert if negative voltage
  }                                                       // of if-then-else an INA3221
  return (microWatts);
}  // of method getBusMicroWatts()
void INA_Class::reset(const uint8_t deviceNumber) {
  /*! @brief     performs a software reset for the specified device
      @details   If no device is specified, then all devices are reset
      @param[in] deviceNumber to reset */
  for (uint8_t i = 0; i < _DeviceCount; i++)  // Loop for each device found
  {
    if (deviceNumber == UINT8_MAX ||
        deviceNumber % _DeviceCount == i)  // If this device needs setting
    {
      readInafromEEPROM(i);  // Load EEPROM to ina structure
      writeWord(INA_CONFIGURATION_REGISTER, INA_RESET_DEVICE, ina.address);  // Set MSB  to reset
      initDevice(i);                                                         // re-initialize device
    }  // of if this device needs to be set
  }    // for-next each device loop
}  // of method reset
void INA_Class::setMode(const uint8_t mode, const uint8_t deviceNumber) {
  /*!
  @brief     sets the operating mode from the list given in enum type "ina_Mode" for a device
  @details   If no device is specified, then all devices are set to the given mode
  @param[in] mode Mode (see "ina_Mode" enumerated type for list of valid values
  @param[in] deviceNumber to reset (Optional, when not set then all devices are mode changed)
  */
  int16_t configRegister;
  for (uint8_t i = 0; i < _DeviceCount; i++)  // Loop for each device found
  {
    if (deviceNumber == UINT8_MAX ||
        deviceNumber % _DeviceCount == i)  // If this device needs setting
    {
      readInafromEEPROM(i);  // Load EEPROM to ina structure
      configRegister = readWord(INA_CONFIGURATION_REGISTER, ina.address);  // Get current config
      configRegister &= ~INA_CONFIG_MODE_MASK;                             // zero out  mode bits
      ina.operatingMode = 0b00000111 & mode;                               // Mask off unused bits
      writeInatoEEPROM(i);                                                 // Store back to EEPROM
      configRegister |= ina.operatingMode;                                 // shift mode settings
      writeWord(INA_CONFIGURATION_REGISTER, configRegister, ina.address);  // Save new value
    }  // if-then this device needs to be set
  }    // for-next each device loop
}  // of method setMode()
bool INA_Class::conversionFinished(const uint8_t deviceNumber) {
  /*!
  @brief     Returns whether or not the conversion has completed
  @details   The device's conversion ready bit is read and returned. "true" denotes finished
             conversion.
  @param[in] deviceNumber to check
  */
  if (_DeviceCount == 0) return false;             // Return finished if invalid device. Issue #65
  readInafromEEPROM(deviceNumber % _DeviceCount);  // Load EEPROM to ina structure
  uint16_t cvBits = 0;
  switch (ina.type) {
    case INA219:
      cvBits = readWord(INA_BUS_VOLTAGE_REGISTER, ina.address) & 2;  // Bit 2 set denotes ready
      readWord(INA_POWER_REGISTER, ina.address);                     // Resets the "ready" bit
      break;
    case INA226:
    case INA230:
    case INA231:
    case INA260: cvBits = readWord(INA_MASK_ENABLE_REGISTER, ina.address) & (uint16_t)8; break;
    case INA3221_0:
    case INA3221_1:
    case INA3221_2: cvBits = readWord(INA3221_MASK_REGISTER, ina.address) & (uint16_t)1; break;
    default: cvBits = 1;
  }  // of switch type
  if (cvBits != 0)
    return (true);
  else
    return (false);
}  // of method "conversionFinished()"
void INA_Class::waitForConversion(const uint8_t deviceNumber) {
  /*!
  @brief     will not return until the conversion for the specified device is finished
  @details   if no device number is specified it will wait until all devices have finished their
             current conversion. If the conversion has completed already then the flag (and
             interrupt pin, if activated) is also reset.
  @param[in] deviceNumber to reset (Optional, when not set all devices have their mode changed)
  */
  uint16_t cvBits = 0;
  for (uint8_t i = 0; i < _DeviceCount; i++)  // Loop for each device found
  {
    if (deviceNumber == UINT8_MAX ||
        deviceNumber % _DeviceCount == i)  // If this device needs setting
    {
      readInafromEEPROM(i);  // Load EEPROM to ina structure
      cvBits = 0;
      while (cvBits == 0)  // Loop until the value is set
      {
        switch (ina.type) {
          case INA219:
            cvBits =
                readWord(INA_BUS_VOLTAGE_REGISTER, ina.address) & 2;  // Bit 2 set denotes ready
            readWord(INA_POWER_REGISTER, ina.address);                // Resets the "ready" bit
            break;
          case INA226:
          case INA230:
          case INA231:
          case INA260:
            cvBits = readWord(INA_MASK_ENABLE_REGISTER, ina.address) & (uint16_t)8;
            break;
          case INA3221_0:
          case INA3221_1:
          case INA3221_2:
            cvBits = readWord(INA3221_MASK_REGISTER, ina.address) & (uint16_t)1;
            break;
          default: cvBits = 1;
        }  // of switch type
      }    // of while the conversion hasn't finished
    }      // of if this device needs to be set
  }        // for-next each device loop
}  // of method waitForConversion()
bool INA_Class::alertOnConversion(const bool alertState, const uint8_t deviceNumber) {
  /*!
  @brief     configures the INA devices which support this functionality to pull the ALERT pin low
             when a conversion is complete
  @details   This call is ignored and returns false when called for an invalid device as the INA219
             doesn't have this pin it won't work for that device.
  @param[in] alertState Boolean true or false to denote the requested setting
  @param[in] deviceNumber to reset (Optional, when not set all devices have their mode changed)
  @return    Returns "true" on success, otherwise false
  */
  uint16_t alertRegister;
  bool     returnCode = false;                // Assume the worst
  for (uint8_t i = 0; i < _DeviceCount; i++)  // Loop for each device found
  {
    if (deviceNumber == UINT8_MAX || deviceNumber == i)  // If this device needs setting
    {
      readInafromEEPROM(i);  // Load EEPROM to ina structure
      switch (ina.type) {
        case INA226:
        case INA230:
        case INA231:
        case INA260:
          alertRegister = readWord(INA_MASK_ENABLE_REGISTER, ina.address);      // Get register
          alertRegister &= INA_ALERT_MASK;                                      // Mask off all bits
          if (alertState) bitSet(alertRegister, INA_ALERT_CONVERSION_RDY_BIT);  // Turn on the bit
          writeWord(INA_MASK_ENABLE_REGISTER, alertRegister, ina.address);      // Write back
          returnCode = true;
          break;
        default: returnCode = false;
      }  // of switch type
    }    // of if this device needs to be set
  }      // for-next each device loop
  return (returnCode);
}  // of method AlertOnConversion
bool INA_Class::alertOnShuntOverVoltage(const bool alertState, const int32_t milliVolts,
                                        const uint8_t deviceNumber) {
  /*!
  @brief     configures the INA devices which support this functionality to pull the ALERT pin low
             when the shunt current exceeds the value given in the parameter in millivolts
  @details   This call is ignored and returns false when called for an invalid device
  @param[in] alertState Boolean true or false to denote the requested setting
  @param[in] milliVolts alert level at which to trigger the alarm
  @param[in] deviceNumber to reset (Optional, when not set all devices have their mode changed)
  @return    Returns "true" on success, otherwise false
  */
  uint16_t alertRegister;
  bool     returnCode = false;                // Assume the worst
  for (uint8_t i = 0; i < _DeviceCount; i++)  // Loop for each device found
  {
    if (deviceNumber == UINT8_MAX || deviceNumber == i)  // If this device needs to be processed
    {
      readInafromEEPROM(i);  // Load EEPROM to ina structure
      switch (ina.type) {
        case INA226:
        case INA230:
        case INA231:
          alertRegister = readWord(INA_MASK_ENABLE_REGISTER, ina.address);  // Get current register
          alertRegister &= INA_ALERT_MASK;                                  // Mask off all bits
          if (alertState)  // If true, then also set threshold
          {
            bitSet(alertRegister, INA_ALERT_SHUNT_OVER_VOLT_BIT);           // Turn on the bit
            uint16_t threshold = milliVolts * 1000 / ina.shuntVoltage_LSB;  // Compute using LSB
            writeWord(INA_ALERT_LIMIT_REGISTER, threshold, ina.address);    // Write register
          }  // of if we are setting a value
          writeWord(INA_MASK_ENABLE_REGISTER, alertRegister, ina.address);  // Write register back
          returnCode = true;
          break;
        default: returnCode = false;
      }  // of switch type
    }    // of if this device needs to be set
  }      // for-next each device loop
  return (returnCode);
}  // of method AlertOnShuntOverVoltage
bool INA_Class::alertOnShuntUnderVoltage(const bool alertState, const int32_t milliVolts,
                                         const uint8_t deviceNumber) {
  /*!
  @brief     configures the INA devices which support this functionality to pull the ALERT pin low
             when the shunt current goes below the value given in the parameter in millivolts
  @details   This call is ignored and returns false when called for an invalid device
  @param[in] alertState Boolean true or false to denote the requested setting
  @param[in] milliVolts alert level at which to trigger the alarm
  @param[in] deviceNumber to reset (Optional, when not set all devices have their alert changed)
  @return    Returns "true" on success, otherwise false */
  uint16_t alertRegister;
  bool     returnCode = true;
  for (uint8_t i = 0; i < _DeviceCount; i++)  // Loop for each device found
  {
    if (deviceNumber == UINT8_MAX ||
        deviceNumber % _DeviceCount == i)  // If this device needs setting
    {
      readInafromEEPROM(i);  // Load EEPROM to ina structure
      switch (ina.type) {
        case INA226:
        case INA230:
        case INA231:
          alertRegister = readWord(INA_MASK_ENABLE_REGISTER, ina.address);  // Get current register
          alertRegister &= INA_ALERT_MASK;                                  // Mask off all bits
          if (alertState)                                                   // Also set threshold
          {
            bitSet(alertRegister, INA_ALERT_SHUNT_UNDER_VOLT_BIT);          // Turn on the bit
            uint16_t threshold = milliVolts * 1000 / ina.shuntVoltage_LSB;  // Compute using LSB
            writeWord(INA_ALERT_LIMIT_REGISTER, threshold, ina.address);    // Write register
          }  // of if we are setting a value
          writeWord(INA_MASK_ENABLE_REGISTER, alertRegister, ina.address);  // Write register back
          break;
        default: returnCode = false;
      }  // of switch type
    }    // of if this device needs to be set
  }      // for-next each device loop
  return (returnCode);
}  // of method AlertOnShuntUnderVoltage
bool INA_Class::alertOnBusOverVoltage(const bool alertState, const int32_t milliVolts,
                                      const uint8_t deviceNumber) {
  /*!
  @brief     configures the INA devices which support this functionality to pull the ALERT pin low
             when the bus voltage goes above the value given in the parameter in millivolts
  @details   This call is ignored and returns false when called for an invalid device
  @param[in] alertState Boolean true or false to denote the requested setting
  @param[in] milliVolts alert level at which to trigger the alarm
  @param[in] deviceNumber to reset (Optional, when not set all devices have their alert changed)
  @return    Returns "true" on success, otherwise false
  */
  uint16_t alertRegister;
  bool     returnCode = true;
  for (uint8_t i = 0; i < _DeviceCount; i++)  // Loop for each device found
  {
    if (deviceNumber == UINT8_MAX ||
        deviceNumber % _DeviceCount == i)  // If this device needs setting
    {
      readInafromEEPROM(i);  // Load EEPROM to ina structure
      switch (ina.type) {
        case INA226:  // Devices that have an alert pin
        case INA230:
        case INA231:
        case INA260:
          alertRegister =
              readWord(INA_MASK_ENABLE_REGISTER, ina.address);  // Get the current register
          alertRegister &= INA_ALERT_MASK;                      // Mask off all bits
          if (alertState)                                       // Also set threshold
          {
            bitSet(alertRegister, INA_ALERT_BUS_OVER_VOLT_BIT);           // Turn on the bit
            uint16_t threshold = milliVolts * 100 / ina.busVoltage_LSB;   // Compute using LSB val
            writeWord(INA_ALERT_LIMIT_REGISTER, threshold, ina.address);  // Write register
          }  // of if we are setting a value
          writeWord(INA_MASK_ENABLE_REGISTER, alertRegister, ina.address);  // Write register back
          break;
        default: returnCode = false;
      }  // of switch type
    }    // of if this device needs to be set
  }      // for-next each device loop
  return (returnCode);
}  // of method AlertOnBusOverVoltageConversion
bool INA_Class::alertOnBusUnderVoltage(const bool alertState, const int32_t milliVolts,
                                       const uint8_t deviceNumber) {
  /*!
  @brief     configures the INA devices which support this functionality to pull the ALERT pin
             low when the bus current goes above the value given in the parameter in millivolts.
  @details   This call is ignored and returns false when called for an invalid device
  @param[in] alertState Boolean true or false to denote the requested setting
  @param[in] milliVolts alert level at which to trigger the alarm
  @param[in] deviceNumber to reset (Optional, when not set then all devices have their alert
  changed)
  @return    Returns "true" on success, otherwise false */
  uint16_t alertRegister;
  bool     returnCode = true;                 // Assume success
  for (uint8_t i = 0; i < _DeviceCount; i++)  // Loop for each device found
  {
    if (deviceNumber == UINT8_MAX ||
        deviceNumber % _DeviceCount == i)  // If this device needs setting
    {
      readInafromEEPROM(i);  // Load EEPROM to ina structure
      switch (ina.type) {
        case INA226:  // Devices that have an alert pin
        case INA230:
        case INA231:
        case INA260:
          alertRegister = readWord(INA_MASK_ENABLE_REGISTER, ina.address);  // Get current register
          alertRegister &= INA_ALERT_MASK;                                  // Mask off all bits
          if (alertState)                                                   // Also set threshold
          {
            bitSet(alertRegister, INA_ALERT_BUS_UNDER_VOLT_BIT);          // Turn on the bit
            uint16_t threshold = milliVolts * 100 / ina.busVoltage_LSB;   // Compute using LSB val
            writeWord(INA_ALERT_LIMIT_REGISTER, threshold, ina.address);  // Write register
          }  // of if we are setting a value
          writeWord(INA_MASK_ENABLE_REGISTER, alertRegister, ina.address);  // Write register back
          break;
        default: returnCode = false;
      }  // of switch type
    }    // of if this device needs to be set
  }      // for-next each device loop
  return (returnCode);
}  // of method AlertOnBusUnderVoltage
bool INA_Class::alertOnPowerOverLimit(const bool alertState, const int32_t milliAmps,
                                      const uint8_t deviceNumber) {
  /*!
  @brief     configures the INA devices which support this functionality to pull the ALERT pin
             low when the power exceeds the value set in the parameter in milliamps
  @details   This call is ignored and returns false when called for an invalid device
  @param[in] alertState Boolean true or false to denote the requested setting
  @param[in] milliAmps alert level at which to trigger the alarm
  @param[in] deviceNumber to reset (Optional, when not set all devices have their alert changed)
  @return    Returns "true" on success, otherwise false
  */
  uint16_t alertRegister;
  bool     returnCode = true;                 // assume success
  for (uint8_t i = 0; i < _DeviceCount; i++)  // Loop for each device found
  {
    if (deviceNumber == UINT8_MAX ||
        deviceNumber % _DeviceCount == i)  // If this device needs setting
    {
      readInafromEEPROM(i);  // Load EEPROM to ina structure
      switch (ina.type) {
        case INA226:
        case INA230:
        case INA231:
        case INA260:  // Devices with alert pin
          alertRegister = readWord(INA_MASK_ENABLE_REGISTER, ina.address);  // Get current register
          alertRegister &= INA_ALERT_MASK;                                  // Mask off all bits
          if (alertState)                                                   // Also set threshold
          {
            bitSet(alertRegister, INA_ALERT_POWER_OVER_WATT_BIT);         // Turn on the bit
            uint16_t threshold = milliAmps * 1000000 / ina.power_LSB;     // Compute using LSB val
            writeWord(INA_ALERT_LIMIT_REGISTER, threshold, ina.address);  // Write register
          }  // of if we are setting a value
          writeWord(INA_MASK_ENABLE_REGISTER, alertRegister, ina.address);  // Write register back
          break;
        default: returnCode = false;
      }  // of switch type
    }    // of if this device needs to be set
  }      // for-next each device loop
  return (returnCode);
}  // of method AlertOnPowerOverLimit
void INA_Class::setAveraging(const uint16_t averages, const uint8_t deviceNumber) {
  /*!
  @brief     sets the hardware averaging for one or all devices
  @details   Out-of-Range averaging is brought down to the highest allowed value
  @param[in] averages Number of  averages to set (0-128)
  @param[in] deviceNumber to reset (Optional, when not set all devices have their averaging changed)
  */
  uint16_t averageIndex;
  int16_t  configRegister;
  for (uint8_t i = 0; i < _DeviceCount; i++)  // Loop for each device found
  {
    if (deviceNumber == UINT8_MAX ||
        deviceNumber % _DeviceCount == i)  // If this device needs setting
    {
      readInafromEEPROM(i);                                                // Load EEPROM to struct
      configRegister = readWord(INA_CONFIGURATION_REGISTER, ina.address);  // Get current register
      switch (ina.type) {
        case INA219:
          if (averages >= 128)
            averageIndex = 15;
          else if (averages >= 64)
            averageIndex = 14;
          else if (averages >= 32)
            averageIndex = 13;
          else if (averages >= 16)
            averageIndex = 12;
          else if (averages >= 8)
            averageIndex = 11;
          else if (averages >= 4)
            averageIndex = 10;
          else if (averages >= 2)
            averageIndex = 9;
          else
            averageIndex = 8;
          configRegister &= ~INA219_CONFIG_AVG_MASK;  // zero out the averages part
          configRegister |= averageIndex << 3;        // shift in the SADC averages
          configRegister |= averageIndex << 7;        // shift in the BADC averages
          break;
        case INA226:
        case INA230:
        case INA231:
        case INA3221_0:
        case INA3221_1:
        case INA3221_2:
        case INA260:
          if (averages >= 1024)
            averageIndex = 7;
          else if (averages >= 512)
            averageIndex = 6;
          else if (averages >= 256)
            averageIndex = 5;
          else if (averages >= 128)
            averageIndex = 4;
          else if (averages >= 64)
            averageIndex = 3;
          else if (averages >= 16)
            averageIndex = 2;
          else if (averages >= 4)
            averageIndex = 1;
          else
            averageIndex = 0;
          configRegister &= ~INA226_CONFIG_AVG_MASK;  // zero out the averages part
          configRegister |= averageIndex << 9;        // shift in the averages to reg
          break;
      }                                                                    // of switch type
      writeWord(INA_CONFIGURATION_REGISTER, configRegister, ina.address);  // Save new value
    }  // of if this device needs to be set
  }    // for-next each device loop
}  // of method setAveraging()
