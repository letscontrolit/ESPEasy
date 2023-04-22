//////////////////////////////////////////////////////////////////////////////////////////////////
// P122 device class for SHT2x temperature & humidity sensors 
// See datasheet https://www.sensirion.com/products/catalog/SHT21/
// Based upon code from Rob Tillaart, Viktor Balint, https://github.com/RobTillaart/SHT2x
// Rewritten and adapted for ESPeasy by Flashmark
// 2023-04-01 Initial version by Flashmark
//////////////////////////////////////////////////////////////////////////////////////////////////

#include "../PluginStructs/P122_data_struct.h"
#ifdef USES_P122

//  SUPPORTED SHT2x COMMANDS
#define SHT2x_GET_TEMPERATURE_NO_HOLD   0xF3  // Read temperature without holding I2C
#define SHT2x_GET_HUMIDITY_NO_HOLD      0xF5  // Read humidity without holding I2C
#define SHT2x_GET_TEMPERATURE_HOLD      0xE3  // Read temperature holding I2C during measurement
#define SHT2x_GET_HUMIDITY_HOLD         0xE5  // Read humidity holding I2C during measurement
#define SHT2x_SOFT_RESET                0xFE  // Software reset
#define SHT2x_WRITE_USER_REGISTER       0xE6  // Write the user register
#define SHT2x_READ_USER_REGISTER        0xE7  // Read the user register
#define SHT2x_GET_EIDA                  0xFA  // Reverse engineering
#define SHT2x_EIDA_ADDRESS              0x0F  // 2nd command byte to read EIDA
#define SHT2x_GET_EIDB                  0xFC  // Reverse engineering
#define SHT2x_EIDB_ADDRESS              0xC9  // 2nd commnad byte to read EIDB
#define SHT2x_GET_FIRMWARE              0x84  // Reverse engineering
#define SHT2x_FIRMWARE_ADDRESS          0xB8  // 2nd command byte to read firmware version

// Bitflags for SHT2x User Register
#define SHT2x_USRREG_RESOLUTION         0x81  // Resolution split into bits 7 and 0
#define SHT2x_USRREG_BATTERY            0x40  // Battery status flag, bit 6
#define SHT2x_USRREG_RESERVED           0x38  // Reserved bits, do not modify, bit 3,4,5
#define SHT2x_USRREG_HEATER             0x04  // Heater enable, bit 2
#define SHT2x_USRREG_OTP                0x02  // Disable OTP reload, bit 1

#define SHT2x_READ_VAL_TIME                5  // Timeout value for reading the termperature/humidity values [ms]
#define SHT2x_READ_REG_TIME               10  // Timeout value for reading a register sequence [ms]
#define SHT2x_READ_USR_TIME                5  // Timeout value for reading user register [ms]
#define P122_RESET_DELAY                  15  // delay in miliseconds for the reset to settle down [ms]
#define P122_MAX_RETRY                   250  // Give up after amount of retries befoe going to error 

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  PUBLIC
//
P122_data_struct::P122_data_struct()
{
  _errCount       = 0;
  _rawTemperature = 0;
  _rawHumidity    = 0;
  _resolution     = 0;
  _state          = P122_state::Uninitialized;
  _eida           = 0;
  _eidb           = 0;
  _firmware       = 0;
  _last_action_started = 0;
  _userreg        = 0;
}

// Initialize/setup device properties
// Must be called at least once before oP122::Wairperating the device
bool P122_data_struct::setupDevice(uint8_t i2caddr, uint8_t resolution)
{
  _i2caddr = i2caddr;
  _resolution = resolution;

#ifdef PLUGIN_122_DEBUG  
  if (loglevelActiveFor(LOG_LEVEL_INFO)) 
  {
    String log = F("SHT2x : Setup Device with address= ");
    log += formatToHex(_i2caddr);
    log += F(" resolution= ");
    log += String(resolution);
    addLog(LOG_LEVEL_INFO, log);
  }
#endif
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Evaluate FSM for data acquisition
// This is a state machine that is evaluated step by step by calling update() repetatively
// NOTE: Function is expected to run as critical section w.r.t. other provided functions
//       This is typically met in ESPeasy plugin context when called from within the plugin
bool P122_data_struct::update()
{
  bool stable = false;   // signals when a stable state is reached
#ifdef PLUGIN_122_DEBUG  
  P122_state oldState = _state;
#endif

  switch(_state)    
  {
    case P122_state::Uninitialized:
      //we have to stop trying after a while
      if (_errCount>P122_MAX_RETRY){
          _state = P122_state::Error;
          stable = true;
      }        
      else if (I2C_wakeup(_i2caddr) != 0)  // Try to access the I2C device
      {
        if (loglevelActiveFor(LOG_LEVEL_ERROR)){
          String log = F("SHT2x : Not found at I2C address: ");
          log += String(_i2caddr, HEX);
          addLog(LOG_LEVEL_ERROR, log);
        }
        _errCount++;
      }       
      else if (writeCmd(SHT2x_SOFT_RESET))  // Issue a reset command
      {
        _state  = P122_state::Wait_for_reset;  // Will take <15ms according to datasheet
        _last_action_started = millis();
      }
    break;

    case P122_state::Wait_for_reset:
      if (timeOutReached(_last_action_started + P122_RESET_DELAY)) //we need to wait for the chip to reset
      {
        if (I2C_wakeup(_i2caddr) != 0)
        {
          _errCount++;
          _state = P122_state::Uninitialized;  // Retry
        }
        else
        {
          _errCount = 0;   // Device is reachable and initialized, reset error counter
          _state = P122_state::Read_eid;
        }
      }
    break;      

    case P122_state::Read_eid:
#ifndef LIMIT_BUILD_SIZE
      _eida = getEIDA();
      _eidb = getEIDB();
      _firmware = getFirmwareVersion();
#endif
      _state = P122_state::Write_user_reg;
    break;

    case P122_state::Write_user_reg:
      writeUserReg();
      _state = P122_state::Initialized;
    break;

    case P122_state::Initialized:
      // For now trigger the first read cycle automatically
      _state = P122_state::Ready;
    break;

    case P122_state::Ready:
      // Ready to execute a measurement cycle
      // Start measuring temperature
      if (!I2C_write8(_i2caddr, SHT2x_GET_TEMPERATURE_NO_HOLD))
      {
        _errCount++;
        _state = P122_state::Uninitialized;  // Retry
      }
      else
      {
        _last_action_started = millis();
        _state = P122_state::Wait_for_temperature_samples;
      }
    break;

   case P122_state::Wait_for_temperature_samples:
      if (timeOutReached(_last_action_started + getTempDuration())) 
      {
        if (!readValue(_rawTemperature)){    // Read the previously measured temperature
          _errCount++;
          _state = P122_state::Uninitialized; // Lost connection
        }
        else if (!requestHumidity())    // Start humidity measurement
        {
          _errCount++;
          _state = P122_state::Uninitialized; // Lost connection
        }
        else
        {
          _last_action_started = millis();
          _state = P122_state::Wait_for_humidity_samples;
        }
      }
    break;

    case P122_state::Wait_for_humidity_samples:
      //make sure we wait for the measurement to complete
      if (timeOutReached(_last_action_started + getHumDuration())) 
      {
        if (!readValue(_rawHumidity)) 
        {
          _errCount++;
          _state = P122_state::Uninitialized; // Lost connection
        }
        else
        {
          _last_action_started = millis();
          _state = P122_state::New_Values_Available;
          stable = true;
        }
      }
    break;

    case P122_state::Error:
    case P122_state::New_Values_Available:
      //this state is used outside so all we need is to stay here
      stable = true;
    break;

    //Missing states (enum values) to be checked by the compiler

  } // switch

#ifdef PLUGIN_122_DEBUG
  if (_state != oldState)
  {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) 
    {
      String log = F("SHT2x : ***state transition ");
      log += String((int)oldState);
      log += F("-->");
      log += String((int)_state);
      addLog(LOG_LEVEL_INFO, log);
    }
  }
#endif
  return stable;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Returns the I2C connection state
// Note: based upon the FSM state without actual accessing the device
bool P122_data_struct::isConnected() const
{
  switch (_state)
  {
    case  P122_state::Initialized:
    case  P122_state::Ready:
    case  P122_state::Wait_for_temperature_samples:
    case  P122_state::Wait_for_humidity_samples:
    case  P122_state::New_Values_Available:
    case  P122_state::Read_eid:
    case  P122_state::Write_user_reg:
      return true;
      break;
    case  P122_state::Uninitialized:
    case  P122_state::Error:
    case  P122_state::Wait_for_reset:
      return false;
      break;

    //Missing states (enum values) to be checked by the compiler
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Returns if the device communication is in error
// Note: based upon the FSM state without actual accessing the device
bool P122_data_struct::inError() const
{
  return _state == P122_state::Error;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Returns if new acquired values are available
bool P122_data_struct::newValues() const
{
  return _state == P122_state::New_Values_Available;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Restart the FSM used to access the device
bool P122_data_struct::reset()
{
  _state = P122_state::Uninitialized;
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Start a new measurement cycle
bool P122_data_struct::startMeasurements()
{
  if ((_state == P122_state::New_Values_Available) || (_state == P122_state::Initialized))
  {
    _state = P122_state::Ready;
  }
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Get the electronic idenfification data store in the device
// Note: The data is read from the device during initialization 
bool P122_data_struct::getEID(uint32_t &eida, uint32_t &eidb, uint8_t &firmware) const
{
  eida = _eida;
  eidb = _eidb;
  firmware = _firmware;
  return true;
}

uint8_t P122_data_struct::getUserReg() const
{
  return _userreg;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Return the previously measured temperature in [C]
float P122_data_struct::getTemperature() const
{
  return -46.85f + (175.72f / 65536.0f) * _rawTemperature;  // Datasheet par 6.2
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Return the previously measured humidity in [%RH]
float P122_data_struct::getHumidity() const
{ 
  return -6.0f + (125.0f / 65536.0f) * _rawHumidity;  // Datasheet par  6.1
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Return the previously measured raw temperature data [bits]
uint16_t P122_data_struct::getRawTemperature() const
{
  return _rawTemperature;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Return the previously measured raw humidity data [bits]
uint16_t P122_data_struct::getRawHumidity() const
{
  return _rawHumidity;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Set the resolution selection
// Resolution is one of 4 predefined settings
// Note: This will only set a new value in the plugin data struct. Reset device to load the new value
bool P122_data_struct::setResolution(uint8_t res)
{
  _resolution = res;
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Return the last set resolution
uint8_t P122_data_struct::getResolution() const
{
  return _resolution;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
// Retrieve the battery status (power supply level)
// Note: This will bypass the FSM and directly accesses the user register of the SHT2x
bool P122_data_struct::batteryOK()
{
  uint8_t userReg = 0x00;
  writeCmd(SHT2x_READ_USER_REGISTER);
  if (!readBytes(1, (uint8_t *) &userReg, SHT2x_READ_USR_TIME))
  {
    return false;
  }
  return (userReg & SHT2x_USRREG_BATTERY) == 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  PROTECTED
//
//////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t P122_data_struct::crc8(const uint8_t *data, uint8_t len)
{
  // CRC-8 formula from page 14 of SHT spec pdf
  // Sensirion_Humidity_Sensors_SHT2x_CRC_Calculation.pdf
  const uint8_t POLY = 0x31;
  uint8_t crc = 0x00;

  for (uint8_t j = len; j; --j)
  {
    crc ^= *data++;

    for (uint8_t i = 8; i; --i)
    {
      crc = (crc & 0x80) ? (crc << 1) ^ POLY : (crc << 1);
    }
  }
  return crc;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
bool P122_data_struct::writeCmd(uint8_t cmd)
{
  return I2C_write8(_i2caddr, cmd);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
bool P122_data_struct::writeCmd(uint8_t cmd, uint8_t value)
{
  return I2C_write8_reg(_i2caddr, cmd, value);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
bool P122_data_struct::readBytes(uint8_t n, uint8_t *val, uint8_t maxDuration)
{
  // TODO check if part can be delegated to the I2C_access libraray from ESPeasy
  Wire.requestFrom(_i2caddr, (uint8_t) n);
  uint32_t start = millis();
  while (Wire.available() < n)
  { 
    if (timePassedSince(start) > maxDuration)
    {
      return false;
    }
    yield();
  }

  for (uint8_t i = 0; i < n; i++)
  {
    val[i] = Wire.read();
  }
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Send Temperature measurement command to device
bool P122_data_struct::requestTemperature()
{
  return writeCmd(SHT2x_GET_TEMPERATURE_NO_HOLD);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Sent Humidity measurement command to device
bool P122_data_struct::requestHumidity()
{
  return writeCmd(SHT2x_GET_HUMIDITY_NO_HOLD);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Read temperature/humidity measurement results from device
bool P122_data_struct::readValue(uint16_t &value)
{
  uint8_t buffer[3];

  if (!readBytes(3, (uint8_t*) &buffer[0], SHT2x_READ_VAL_TIME))
  {
    return false;
  }
  if (crc8(buffer, 2) == buffer[2])
  {
    value  = buffer[0] << 8;
    value += buffer[1];
    value &= 0xFFFC;
  }
  return true;
}

#ifndef LIMIT_BUILD_SIZE
//////////////////////////////////////////////////////////////////////////////////////////////////
//  Retrieve SHT2x identification code part I
//  Sensirion_Humidity_SHT2x_Electronic_Identification_Code_V1.1.pdf
uint32_t P122_data_struct::getEIDA()
{
  uint32_t id = 0;
  uint8_t buffer[8];
  writeCmd(SHT2x_GET_EIDA, SHT2x_EIDA_ADDRESS);
  if (readBytes(8, (uint8_t *) buffer, SHT2x_READ_REG_TIME))
  {
    for (uint8_t i = 0; i < 4; i++)
    {
      id <<= 8;
      id |= buffer[i*2];  //  SNB, skip CRC's use only uneven bytes
    }
  }
  return id;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//  Retrieve SHT2x identification code part II
//  Sensirion_Humidity_SHT2x_Electronic_Identification_Code_V1.1.pdf
uint32_t P122_data_struct::getEIDB()
{
  uint32_t id = 0;
  uint8_t buffer[8];
  writeCmd(SHT2x_GET_EIDB, SHT2x_EIDB_ADDRESS);
  if (readBytes(8, (uint8_t *) buffer, SHT2x_READ_REG_TIME))
  {
    id  = buffer[0];  //  SNC_1
    id <<= 8;
    id |= buffer[1];  //  SNC_0
    id <<= 8;
    id |= buffer[3];  //  SNA_1
    id <<= 8;
    id |= buffer[4];  //  SNA_0
  }
  return id;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Retrieve SHT2x Firmware version from device
uint8_t P122_data_struct::getFirmwareVersion()
{
  uint8_t version = 0;
  writeCmd(SHT2x_GET_FIRMWARE, SHT2x_FIRMWARE_ADDRESS);
  if (!readBytes(1, (uint8_t *) &version, SHT2x_READ_REG_TIME))
  {
    version = 0;
  }
  return version;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////
// Write the user register with data stored in the plugin data struct
bool P122_data_struct::writeUserReg()
{
  uint8_t userReg = 0;

  // First fetch the current userReg value
  writeCmd(SHT2x_READ_USER_REGISTER);
  if (!readBytes(1, (uint8_t *) &userReg, SHT2x_READ_USR_TIME))
  {
    return false;
  }

  // Set selected resolution which is split into bit 7 and bit 0
  userReg &= ~SHT2x_USRREG_RESOLUTION;  //  clear old resolution and set new
  userReg &= ~SHT2x_USRREG_HEATER;  // Reset heater control bit
  userReg |= ((_resolution & 0x02) << 6);
  userReg |= (_resolution & 0x01);

  if (!writeCmd(SHT2x_WRITE_USER_REGISTER, userReg))
  {
    return false;
  }

#ifdef PLUGIN_122_DEBUG
  // Read back the register for debugging purpose only
  writeCmd(SHT2x_READ_USER_REGISTER);
  if (!readBytes(1, (uint8_t *) &userReg, SHT2x_READ_USR_TIME))
  {
    return false;
  }
  _userreg = userReg;
#endif
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

unsigned long P122_data_struct::getTempDuration()
{
  //  Datasheet table 7
  switch (_resolution)
  {
    case 0: return 85;  // 14 bit
    case 1: return 22;  // 12 bit
    case 2: return 43;  // 13 bit
    case 3: return 11;  // 11 bit
    default: return 85; // Save value?
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Calculate the minimum delay for humidity measurement (depends on the resolution)
unsigned long P122_data_struct::getHumDuration()
{
    // Datasheet table 7
  switch (_resolution)
  {
    case 0: return 29;  // 12 bit
    case 1: return  4;  //  8 bit
    case 2: return  9;  // 10 bit
    case 3: return 15;  // 11 bit
    default: return 29; // Save value?
  }
}

#endif // USES_P122