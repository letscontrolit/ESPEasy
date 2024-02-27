///////////////////////////////////////////////////////////////////////////////////////////////////
// P167 device class for IKEA Vindstyrka SEN54 temperature , humidity and air quality sensors 
// See datasheet https://sensirion.com/media/documents/6791EFA0/62A1F68F/Sensirion_Datasheet_Environmental_Node_SEN5x.pdf
// and info about extra request https://sensirion.com/media/documents/2B6FC1F3/6409E74A/PS_AN_Read_RHT_VOC_and_NOx_RAW_signals_D1.pdf
// Based upon code from Rob Tillaart, Viktor Balint, https://github.com/RobTillaart/SHT2x
// Rewritten and adapted for ESPeasy by andibaciu
// 2023-06-20 Initial version by andibaciu
//////////////////////////////////////////////////////////////////////////////////////////////////

#include "../PluginStructs/P167_data_struct.h"
#include "../ESPEasyCore/ESPEasyGPIO.h"

#include <GPIO_Direct_Access.h>

  #ifndef CORE_POST_3_0_0
    #ifdef ESP8266
      #define IRAM_ATTR ICACHE_RAM_ATTR
    #endif
  #endif

#ifdef USES_P167


#define P167_START_MEAS                           0x0021  // Start measurement command
#define P167_START_MEAS_RHT_GAS                   0x0037  // Start measurement RHT/Gas command
#define P167_STOP_MEAS                            0x0104  // Stop measurement command
#define P167_READ_DATA_RDY_FLAG                   0x0202  // Read Data Ready Flag command
#define P167_READ_MEAS                            0x03C4  // Read measurement command
#define P167_R_W_TEMP_COMP_PARAM                  0x60B2  // Read/Write Temperature Compensation Parameters command
#define P167_R_W_TWARM_START_PARAM                0x60C6  // Read/Write Warm Start Parameters command
#define P167_R_W_VOC_ALG_PARAM                    0x60D0  // Read/Write VOC Algorithm Tuning Parameters command
#define P167_R_W_NOX_ALG_PARAM                    0x60E1  // Read/Write NOx Algorithm Tuning Parameters command
#define P167_R_W_RH_T_ACC_Mode                    0x60F7  // Read/Write RH/T Acceleration Mode command
#define P167_R_W_VOC_ALG_STATE                    0x6181  // Read/Write VOC Algorithm State command
#define P167_START_FAN_CLEAN                      0x5607  // Start fan cleaning command
#define P167_R_W_AUTOCLEN_PARAM                   0x8004  // Read/Write Autocleaning Interval Parameters command
#define P167_READ_PROD_NAME                       0xD014  // Read Product Name command
#define P167_READ_SERIAL_NO                       0xD033  // Read Serial Number command
#define P167_READ_FIRM_VER                        0xD100  // Read Firmware Version command
#define P167_READ_DEVICE_STATUS                   0xD206  // Read Device Status command
#define P167_CLEAR_DEVICE_STATUS                  0xD210  // Clear Device Status command
#define P167_RESET_DEVICE                         0xD304  // Reset Device command
#define P167_READ_RAW_MEAS                        0x03D2  // Read relative humidity and temperature
                                                          // which are not compensated for temperature offset, and the
                                                          // VOC and NOx raw signals (proportional to the logarithm of the
                                                          // resistance of the MOX layer). It returns 4x2 bytes (+ 1 CRC
                                                          // byte each) command (see second datasheet fron header for more info)
#define P167_READ_RAW_MYS_MEAS                    0x03F5  // Read relative humidity and temperature and MYSTERY word (probably signed offset temperature)


#define P167_START_MEAS_DELAY                     50  // Timeout value for start measurement command [ms]
#define P167_START_MEAS_RHT_GAS_DELAY             50  // Timeout value for start measurement RHT/Gas command [ms]
#define P167_STOP_MEAS_DELAY                      200 // Timeout value for start measurement command [ms]
#define P167_READ_DATA_RDY_FLAG_DELAY             20  // Timeout value for read data ready flag command [ms]
#define P167_READ_MEAS_DELAY                      20  // Timeout value for read measurement command [ms]
#define P167_R_W_TEMP_COMP_PARAM_DELAY            20  // Timeout value for read/write temperature compensation parameters command [ms]
#define P167_R_W_WARM_START_PARAM_DELAY           20  // Timeout value for read/write warm start parameters command [ms]
#define P167_R_W_VOC_ALG_PARAM_DELAY              20  // Timeout value for read/write VOC algorithm tuning parameters command [ms]
#define P167_R_W_NOX_ALG_PARAM_DELAY              20  // Timeout value for read/write NOx algorithm tuning parameters command [ms]
#define P167_R_W_RH_T_ACC_MODE_DELAY              20  // Timeout value for read/write RH/T acceleration mode command [ms]
#define P167_R_W_VOC_ALG_STATE_DELAY              20  // Timeout value for read/write VOC algorithm State command [ms]
#define P167_START_FAN_CLEAN_DELAY                20  // Timeout value for start fan cleaning command [ms]
#define P167_R_W_AUTOCLEN_PARAM_DELAY             20  // Timeout value for read/write autoclean interval parameters command [ms]
#define P167_READ_PROD_NAME_DELAY                 20  // Timeout value for read product name command [ms]
#define P167_READ_SERIAL_NO_DELAY                 20  // Timeout value for read serial number command [ms]
#define P167_READ_FIRM_VER_DELAY                  20  // Timeout value for read firmware version command [ms]
#define P167_READ_DEVICE_STATUS_DELAY             20  // Timeout value for read device status command [ms]
#define P167_CLEAR_DEVICE_STATUS_DELAY            20  // Timeout value for clear device status command [ms]
#define P167_RESET_DEVICE_DELAY                   100 // Timeout value for reset device command [ms]
#define P167_READ_RAW_MEAS_DELAY                  20  // Timeout value for read raw temp and humidity command [ms]


#define P167_MAX_RETRY                            250  // Give up after amount of retries befoe going to error 


#define SCL_MONITOR_PIN                           13  //pin13 as monitor scl i2c


//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  PUBLIC
//
P167_data_struct::P167_data_struct()
{
  _errCount                 = 0;

  _Temperature              = 0.0;
  _rawTemperature           = 0.0;
  _mysTemperature           = 0.0;
  _Humidity                 = 0.0;
  _rawHumidity              = 0.0;
  _mysHumidity              = 0.0;
  _DewPoint                 = 0.0;
  
  _tVOC                     = 0.0;
  _rawtVOC                  = 0.0;
  _NOx                      = 0.0;
  _rawNOx                   = 0.0;
  _mysOffset                = 0.0;
  _PM1p0                    = 0.0;
  _PM2p5                    = 0.0;
  _PM4p0                    = 0.0;
  _PM10p0                   = 0.0;

  _readingerrcode           = VIND_ERR_NO_ERROR;
  _readingerrcount          = 0;
  _readingsuccesscount      = 0;

  _model                    = 0;
  _i2caddr                  = 0;
  _monpin                   = 0;
 
  _state                    = P167_state::Uninitialized;
  _eid_productname          = F("");
  _eid_serialnumber         = F("");
  _firmware                 = 0;
  _last_action_started      = 0;
  _userreg                  = 0;
}


P167_data_struct::~P167_data_struct()
{
  //
}

// Initialize/setup device properties
// Must be called at least once before oP167::Wairperating the device
bool P167_data_struct::setupDevice(uint8_t i2caddr)
{
  _i2caddr = i2caddr;

#ifdef PLUGIN_167_DEBUG  
  if (loglevelActiveFor(LOG_LEVEL_INFO)) 
  {
    String log = F("SEN5x: Setup with address= ");
    log += formatToHex(_i2caddr);
    addLog(LOG_LEVEL_INFO, log);
  }
#endif
  return true;
}


bool P167_data_struct::setupMonPin(uint8_t monpin)
{
  if (validGpio(monpin))
  {
    _monpin = monpin;
    pinMode(_monpin, INPUT_PULLUP);             //declare monitoring pin as input with pullup's
    //attachInterruptArg(digitalPinToInterrupt(_monpin), reinterpret_cast<void (*)(void *)>(checkPin), this, CHANGE);
    //enableInterrupt_monpin();

    #ifdef PLUGIN_167_DEBUG  
    if (loglevelActiveFor(LOG_LEVEL_INFO)) 
    {
      String log = F("SEN5x: Setup I2C SCL monpin= ");
      log += _monpin;
      addLog(LOG_LEVEL_INFO, log);
    }
    #endif
    return true;
  }
  else
    return false;
}


void P167_data_struct::enableInterrupt_monpin(void)
{
  //attachInterruptArg(digitalPinToInterrupt(_monpin), reinterpret_cast<void (*)(void *)>(checkPin), this, CHANGE);
}


void P167_data_struct::disableInterrupt_monpin(void)
{
  //detachInterrupt(digitalPinToInterrupt(_monpin));
}

// Initialize/setup device properties
// Must be called at least once before oP167::Wairperating the device
bool P167_data_struct::setupModel(uint8_t model)
{
  _model = model;

#ifdef PLUGIN_167_DEBUG  
  if (loglevelActiveFor(LOG_LEVEL_INFO)) 
  {
    String log = F("SEN5x: Setup model= ");
    log += String(_model);
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
bool P167_data_struct::update()
{
  bool stable = false;   // signals when a stable state is reached
#ifdef PLUGIN_167_DEBUG  
  P167_state oldState = _state;
#endif


  if(statusMonitoring == false)
    return stable;


  switch(_state)    
  {
    case P167_state::Uninitialized:
      //we have to stop trying after a while
      if (_errCount>P167_MAX_RETRY)
      {
          _state = P167_state::Error;
          stable = true;
      }        
      else if (I2C_wakeup(_i2caddr) != 0)  // Try to access the I2C device
      {
        if (loglevelActiveFor(LOG_LEVEL_ERROR))
        {
          String log = F("SEN5x : Not found at I2C address: ");
          log += String(_i2caddr, HEX);
          addLog(LOG_LEVEL_ERROR, log);
        }
        _errCount++;
      } 
      else if (_model==0 )                  //sensor is Vindstyrka and d'ont need to be reset
      {
        _errCount = 0;   // Device is reachable and initialized, reset error counter
        if (writeCmd(P167_READ_FIRM_VER))  // Issue a reset command
        {
          _state  = P167_state::Read_firm_version;  // Will take <20ms according to datasheet
          _last_action_started = millis();
        }
      }
      else if (_model==1)
      {
        _errCount = 0;   // Device is reachable and initialized, reset error counter
        if (writeCmd(P167_RESET_DEVICE))  // Issue a reset command
        {
          _state  = P167_state::Wait_for_reset;  // Will take <20ms according to datasheet
          _last_action_started = millis();
        }
      }
    break;

    case P167_state::Wait_for_reset:
      if (timeOutReached(_last_action_started + P167_RESET_DEVICE_DELAY)) //we need to wait for the chip to reset
      {
        if (I2C_wakeup(_i2caddr) != 0)
        {
          _errCount++;
          _state = P167_state::Uninitialized;  // Retry
        }
        else
        {
          _errCount = 0;   // Device is reachable and initialized, reset error counter
          if (writeCmd(P167_READ_FIRM_VER))
          {
            _state  = P167_state::Read_firm_version;  // Will take <20ms according to datasheet
            _last_action_started = millis();
          }
        }
      }
    break;

    case P167_state::Read_firm_version:
      if (timeOutReached(_last_action_started + P167_READ_FIRM_VER_DELAY)) 
      {
        // Start read flag
        if (!getFirmwareVersion())
        {
          _errCount++;
          _state = P167_state::Uninitialized;  // Retry
        }
        else if(!writeCmd(P167_READ_PROD_NAME)) 
        {
          _errCount++;
          _state = P167_state::Uninitialized;  // Retry
        }
        else
        {
          _last_action_started = millis();
          _state = P167_state::Read_prod_name;
        }
      }
    break;  

    case P167_state::Read_prod_name:
      if (timeOutReached(_last_action_started + P167_READ_PROD_NAME_DELAY)) 
      {
        // Start read flag
        if (!getProductName())
        {
          _errCount++;
          _state = P167_state::Uninitialized;  // Retry
        }
        else if(!writeCmd(P167_READ_SERIAL_NO)) 
        {
          _errCount++;
          _state = P167_state::Uninitialized;  // Retry
        }
        else
        {
          _last_action_started = millis();
          _state = P167_state::Read_serial_no;
        }
      }
    break;

    case P167_state::Read_serial_no:
      if (timeOutReached(_last_action_started + P167_READ_SERIAL_NO_DELAY)) 
      {
        // Start read flag
        if (!getSerialNumber())
        {
          _errCount++;
          _state = P167_state::Uninitialized;  // Retry
        }
        else if(!writeCmd(P167_READ_SERIAL_NO)) 
        {
          _errCount++;
          _state = P167_state::Uninitialized;  // Retry
        }
        else
        {
          _last_action_started = millis();
          _state = P167_state::Initialized;
        }
      }
    break; 

    case P167_state::Write_user_reg:
      _state = P167_state::Initialized;
    break;

    case P167_state::Initialized:
      // For now trigger the first read cycle automatically
      //_state = P167_state::Ready;
    break;

    case P167_state::Ready:
      // Ready to execute a measurement cycle
      if( _model==0 || _model==1 || _model==2)
      {
        // Start measuring data
        if (!writeCmd(P167_START_MEAS))
        {
          _errCount++;
          _state = P167_state::Uninitialized;  // Retry
        }
        else
        {
          _last_action_started = millis();
          _state = P167_state::Wait_for_start_meas;
        }
      }
    break;

    case P167_state::Wait_for_start_meas:
      if (timeOutReached(_last_action_started + P167_START_MEAS_DELAY)) 
      {
        // Start read flag
        if (!writeCmd(P167_READ_DATA_RDY_FLAG))
        {
          _errCount++;
          _state = P167_state::Uninitialized;  // Retry
        }
        else
        {
          _last_action_started = millis();
          _state = P167_state::Wait_for_read_flag;
        }
      }
    break;

    case P167_state::Wait_for_read_flag:
      if (timeOutReached(_last_action_started + P167_READ_DATA_RDY_FLAG_DELAY)) 
      {
        if(readDataRdyFlag())
        {
          // Ready to execute a measurement cycle
          if (!writeCmd(P167_READ_MEAS))
          {
            _errCount++;
            _state = P167_state::Uninitialized;  // Retry
          }
          else
          {
            _last_action_started = millis();
            _state = P167_state::Wait_for_read_meas;
          }
        }
        else  //Ready Flag NOT ok, so send again Start Measurement
        {
          // Start measuring data
          if (!writeCmd(P167_START_MEAS))
          {
            _errCount++;
            _state = P167_state::Uninitialized;  // Retry
          }
          else
          {
            _last_action_started = millis();
            _state = P167_state::Wait_for_start_meas;
          }
        }
      }
    break;

   case P167_state::Wait_for_read_meas:
      if (timeOutReached(_last_action_started + P167_READ_MEAS_DELAY)) 
      {
        if (!readMeasValue())    // Read the previously measured temperature
        {
          _errCount++;
          //_state = P167_state::Uninitialized; // Lost connection
          _state = P167_state::cmdSTARTmeas;
        }
        else
        {
          if (!writeCmd(P167_READ_RAW_MEAS))
          {
            _errCount++;
            _state = P167_state::Uninitialized;  // Retry
          }
          else
          {
            _last_action_started = millis();
            _state = P167_state::Wait_for_read_raw_meas;
          }
        }
      }
    break;

    case P167_state::Wait_for_read_raw_meas:
      //make sure we wait for the measurement to complete
      if (timeOutReached(_last_action_started + P167_READ_RAW_MEAS_DELAY)) 
      {
        if (!readMeasRawValue()) 
        {
          _errCount++;
          //_state = P167_state::Uninitialized; // Lost connection
          _state = P167_state::cmdSTARTmeas;
        }
        else
        {
          if (!writeCmd(P167_READ_RAW_MYS_MEAS))
          {
            _errCount++;
            _state = P167_state::Uninitialized;  // Retry
          }
          else
          {
            _last_action_started = millis();
            _state = P167_state::Wait_for_read_raw_MYS_meas;
          }
        }
      }
    break;

    case P167_state::Wait_for_read_raw_MYS_meas:
      //make sure we wait for the measurement to complete
      if (timeOutReached(_last_action_started + P167_READ_RAW_MEAS_DELAY)) 
      {
        if (!readMeasRawMYSValue()) 
        {
          _errCount++;
          //_state = P167_state::Uninitialized; // Lost connection
          _state = P167_state::cmdSTARTmeas;
        }
        else
        {
          _last_action_started = millis();
          _state = P167_state::cmdSTARTmeas;
          calculateValue();
          stable = true;
        }
      }
    break;

    case P167_state::cmdSTARTmeas:
      // Start measuring data
      if (!writeCmd(P167_START_MEAS))
      {
        _errCount++;
        _state = P167_state::Uninitialized;  // Retry
      }
      else
      {
        _last_action_started = millis();
        _state = P167_state::IDLE;
      }
    break;

    case P167_state::IDLE:
      stepMonitoring = 1;
      startMonitoringFlag = false;
      if(!_errmeas && !_errmeasraw && !_errmeasrawmys)
        _state = P167_state::New_Values_Available;
      stable = true;
    break;

    case P167_state::Error:
    case P167_state::New_Values_Available:
      //this state is used outside so all we need is to stay here
      stable = true;
    break;

    //Missing states (enum values) to be checked by the compiler

  } // switch

#ifdef PLUGIN_167_DEBUG
  if (_state != oldState)
  {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) 
    {
      String log = F("SEN5x : *** state transition ");
      log += String((int)oldState);
      log += F("-->");
      log += String((int)_state);
      addLog(LOG_LEVEL_INFO, log);
    }
  }
#endif
  return stable;
}


bool P167_data_struct::monitorSCL()
{
  if(_model==0)
  {
    if(startMonitoringFlag)
    {
      
      if(stepMonitoring==1)
      {
        lastSCLLowTransitionMonitoringTime = monpinLastTransitionTime/1000;
        if(millis() - lastSCLLowTransitionMonitoringTime < 100)
        {
          statusMonitoring = false;
          return true;
        }
        else
        {
          lastSCLLowTransitionMonitoringTime = monpinLastTransitionTime/1000;
          statusMonitoring = true;
          stepMonitoring++;
        }
      }

      if(stepMonitoring==2)
      {
        if(millis() - lastSCLLowTransitionMonitoringTime < 100)
        {
          lastSCLLowTransitionMonitoringTime = monpinLastTransitionTime/1000;
          statusMonitoring = false;
          stepMonitoring = 1;
          return true;
        }
        else if(millis() - lastSCLLowTransitionMonitoringTime > 700)
        {
          statusMonitoring = false;
          stepMonitoring = 1;
          startMonitoringFlag = false;

          //if _state not finish reading process then start from begining
          if(_state >= P167_state::Wait_for_read_meas && _state < P167_state::New_Values_Available)
          {
            _state = P167_state::Ready;
          }
          return true;
        }
        else
        {
          //processing
        }
      }
    }
    monpinValuelast = monpinValue;
  }

  if(_model == 1 || _model == 2)
  {
    statusMonitoring = true;
    startMonitoringFlag = false;
    stepMonitoring = 0;
  }

  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Returns the I2C connection state
// Note: based upon the FSM state without actual accessing the device
bool P167_data_struct::isConnected() const
{
  switch (_state)
  {
    case  P167_state::Initialized:
    case  P167_state::Ready:
    case  P167_state::Wait_for_start_meas:
    case  P167_state::Wait_for_read_flag:
    case  P167_state::Wait_for_read_meas:
    case  P167_state::Wait_for_read_raw_meas:
    case  P167_state::Wait_for_read_raw_MYS_meas:
    case  P167_state::cmdSTARTmeas:
    case  P167_state::New_Values_Available:
    case  P167_state::Read_firm_version:
    case  P167_state::Read_prod_name:
    case  P167_state::Read_serial_no:
    case  P167_state::Write_user_reg:
    case  P167_state::IDLE:
      return true;
      break;
    case  P167_state::Uninitialized:
    case  P167_state::Error:
    case  P167_state::Wait_for_reset:
      return false;
      break;

    //Missing states (enum values) to be checked by the compiler
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Returns if the device communication is in error
// Note: based upon the FSM state without actual accessing the device
bool P167_data_struct::inError() const
{
  return _state == P167_state::Error;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Returns if new acquired values are available
bool P167_data_struct::newValues() const
{
  return _state == P167_state::New_Values_Available;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Restart the FSM used to access the device
bool P167_data_struct::reset()
{
  startMonitoringFlag = true;
  stepMonitoring = 1;
  _state = P167_state::Uninitialized;
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Start a new measurement cycle
bool P167_data_struct::startMeasurements()
{
  if ((_state == P167_state::New_Values_Available) || (_state == P167_state::Initialized)  || (_state == P167_state::IDLE))
  {
    _state = P167_state::Ready;
  }
  startMonitoringFlag = true;
  stepMonitoring = 1;
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Get the electronic idenfification data store in the device
// Note: The data is read from the device during initialization 
bool P167_data_struct::getEID(String &eid_productname, String &eid_serialnumber, uint8_t &firmware) const
{
  eid_productname = _eid_productname;
  eid_serialnumber = _eid_serialnumber;
  firmware = _firmware;
  return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
// Return the previously measured raw humidity data [bits]
float P167_data_struct::getRequestedValue(uint8_t request) const
{
  //float requested_value=0;
  switch(request)
  {
    case 0:   return (float) _TemperatureX;
    case 1:   return (float) _HumidityX;
    case 2:   return (float) _PM2p5;
    case 3:   return (float) _tVOC;
    case 4:   return (float) _PM1p0;
    case 5:   return (float) _PM4p0;
    case 6:   return (float) _PM10p0;
    case 7:   return (float) _DewPoint;
  }
  return -1;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  PROTECTED
//
//////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t P167_data_struct::crc8(const uint8_t *data, uint8_t len)
{
  // CRC-8 formula from page 14 of SHT spec pdf
  // Sensirion_Humidity_Sensors_SHT2x_CRC_Calculation.pdf
  const uint8_t POLY = 0x31;
  uint8_t crc = 0xFF;

  for (uint8_t j = 0; j<len; j++)
  {
    crc ^= *data++;

    for (uint8_t i = 8; i; --i)
    {
      //crc = (crc & 0x80) ? (crc << 1) ^ POLY : (crc << 1);

      if(crc & 0x80) 
      {
        crc = (crc << 1) ^ POLY;
      } 
      else 
      {
        crc = (crc << 1);
      }
    }
    
  }
  return crc;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//bool P167_data_struct::writeCmd(uint8_t cmd)
//{
//  return I2C_write8(_i2caddr, cmd);
//}

//////////////////////////////////////////////////////////////////////////////////////////////////
bool P167_data_struct::writeCmd(uint16_t cmd)
{
  Wire.beginTransmission(_i2caddr);
  Wire.write((uint8_t)(cmd >> 8));
  Wire.write((uint8_t)cmd);
  return Wire.endTransmission() == 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
bool P167_data_struct::writeCmd(uint16_t cmd, uint8_t value)
{
  Wire.beginTransmission(_i2caddr);
  Wire.write((uint8_t)(cmd >> 8));
  Wire.write((uint8_t)cmd);
  Wire.write((uint8_t)value);
  return Wire.endTransmission() == 0;
}



bool P167_data_struct::writeCmd(uint16_t cmd, uint8_t length, uint8_t *buffer)
{
  Wire.beginTransmission(_i2caddr);
  Wire.write((uint8_t)(cmd >> 8));
  Wire.write((uint8_t)cmd);
  for (int i = 0; i < length; i++) {
    Wire.write(*(buffer + i));
  }
  return Wire.endTransmission() == 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
bool P167_data_struct::readBytes(uint8_t n, uint8_t *val, uint8_t maxDuration)
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
// Read data ready flag from device
bool P167_data_struct::readDataRdyFlag()
{
  uint8_t value=0;
  uint8_t buffer[3];

  if (!readBytes(3, (uint8_t*) &buffer[0], P167_READ_DATA_RDY_FLAG_DELAY))
  {
    return false;
  }
  if (crc8(&buffer[0], 2) == buffer[2])
  {
    value += buffer[1];
  }
  return value;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
// Read measurement values results from device
bool P167_data_struct::readMeasValue()
{
  uint16_t value=0;
  uint8_t buffer[24];

  _errmeas = false;
  if (!readBytes(24, (uint8_t*) &buffer[0], P167_READ_MEAS_DELAY))
  {
    _errmeas = true;
    return false;
  }

  String log = F("SEN5x : ***meas value ");
  for(int xx=0; xx<24;xx++)
  {
    log += String((int)buffer[xx]);
    log += F(" ");
  }

  if((buffer[0] == 0xFF && buffer[1] == 0xFF) || (buffer[3] == 0xFF && buffer[4] == 0xFF) || (buffer[6] == 0xFF && buffer[7] == 0xFF) || (buffer[9] == 0xFF && buffer[10] == 0xFF) || (buffer[12] == 0xFF && buffer[13] == 0xFF) || (buffer[15] == 0xFF && buffer[16] == 0xFF) || (buffer[18] == 0xFF && buffer[19] == 0xFF) || (buffer[21] == 0xFF && buffer[22] == 0xFF))
  {
    log += F("- error");
    addLog(LOG_LEVEL_INFO, log);
    _errmeas = true;
    _readingerrcount++;
    return false;
  }
  else
  {
    if ((crc8(&buffer[0], 2) == buffer[2]) && (buffer[0] != 0xFF || buffer[1] != 0xFF))
    {
      value  = buffer[0] << 8;
      value += buffer[1];
      _PM1p0 = (float)value/10;
    }
    else
    {
      _errmeas = true;
    }
    if ((crc8(&buffer[3], 2) == buffer[5]) && (buffer[3] != 0xFF || buffer[4] != 0xFF))
    {
      value  = buffer[3] << 8;
      value += buffer[4];
      _PM2p5 = (float)value/10;
    }
    else
    {
      _errmeas = true;
    }
    if ((crc8(&buffer[6], 2) == buffer[8]) && (buffer[6] != 0xFF || buffer[7] != 0xFF))
    {
      value  = buffer[6] << 8;
      value += buffer[7];
      _PM4p0 = (float)value/10;
    }
    else
    {
      _errmeas = true;
    }
    if ((crc8(&buffer[9], 2) == buffer[11]) && (buffer[9] != 0xFF || buffer[10] != 0xFF))
    {
      value  = buffer[9] << 8;
      value += buffer[10];
      _PM10p0 = (float)value/10;
    }
    else
    {
      _errmeas = true;
    }
    if ((crc8(&buffer[12], 2) == buffer[14]) && (buffer[12] != 0xFF || buffer[13] != 0xFF))
    {
      value  = buffer[12] << 8;
      value += buffer[13];
      _Humidity = (float)value/100;
    }
    else
    {
      _errmeas = true;
    }
    if ((crc8(&buffer[15], 2) == buffer[17]) && (buffer[15] != 0xFF || buffer[16] != 0xFF))
    {
      value  = buffer[15] << 8;
      value += buffer[16];
      _Temperature = (float)value/200;
    }
    else
    {
      _errmeas = true;
    }
    if ((crc8(&buffer[18], 2) == buffer[20]) && (buffer[18] != 0xFF || buffer[19] != 0xFF))
    {
      value  = buffer[18] << 8;
      value += buffer[19];
      _tVOC = (float)value/10;
    }
    else
    {
      _errmeas = true;
    }
    if ((crc8(&buffer[21], 2) == buffer[23]) && (buffer[21] != 0xFF || buffer[22] != 0xFF))
    {
      value  = buffer[21] << 8;
      value += buffer[22];
      _NOx = (float)value/10;
    }
    else
    {
      _errmeas = true;
    }

    if(_errmeas == true)
    {
      log += F("- crc error");
      _readingerrcount++;
    }
    else
    {
      log += F("- pass");
      _readingsuccesscount++;
    }
    addLog(LOG_LEVEL_INFO, log);
    return !_errmeas;
  }

  return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
// Read measurement values results from device
bool P167_data_struct::readMeasRawValue()
{
  uint16_t value=0;
  uint8_t buffer[12];

  _errmeasraw = false;
  if (!readBytes(12, (uint8_t*) &buffer[0], P167_READ_RAW_MEAS_DELAY))
  {
    _errmeasraw = true;
    return false;
  }

  String log = F("SEN5x : ***meas RAW value ");
  for(int xx=0; xx<12;xx++)
  {
    log += String((int)buffer[xx]);
    log += F(" ");
  }

  if((buffer[0] == 0xFF && buffer[1] == 0xFF) || (buffer[3] == 0xFF && buffer[4] == 0xFF) || (buffer[6] == 0xFF && buffer[7] == 0xFF))// || (buffer_raw[9] == 0xFF && buffer_raw[10] == 0xFF))
  {
    log += F("- error");
    addLog(LOG_LEVEL_INFO, log);
    _errmeasraw = true;
    _readingerrcount++;
    return false;
  }
  else
  {
    if ((crc8(&buffer[0], 2) == buffer[2]) && (buffer[0] != 0xFF || buffer[1] != 0xFF))
    {
      value  = buffer[0] << 8;
      value += buffer[1];
      _rawHumidity = (float)value/100.0;
    }
    else
    {
      _errmeasraw = true;
    }
    if ((crc8(&buffer[3], 2) == buffer[5]) && (buffer[3] != 0xFF || buffer[4] != 0xFF))
    {
      value  = buffer[3] << 8;
      value += buffer[4];
      _rawTemperature = (float)value/200.0;
    }
    else
    {
      _errmeasraw = true;
    }
    if ((crc8(&buffer[6], 2) == buffer[8]) && (buffer[6] != 0xFF || buffer[7] != 0xFF))
    {
      value  = buffer[6] << 8;
      value += buffer[7];
      _rawtVOC = (float)value/10.0;
    }
    else
    {
      _errmeasraw = true;
    }
    if ((crc8(&buffer[9], 2) == buffer[11]))
    {
      value  = buffer[9] << 8;
      value += buffer[10];
      _rawNOx = (float)value/10.0;
    }
    else
    {
      _errmeasraw = true;
    }

    if(_errmeasraw == true)
    {
      log += F("- crc error");
      _readingerrcount++;
    }
    else
    {
      log += F("- pass");
      _readingsuccesscount++;
    }
    addLog(LOG_LEVEL_INFO, log);
    return !_errmeasraw;
  }

  
  return true;
}



//////////////////////////////////////////////////////////////////////////////////////////////////
// Read measurement values results from device
bool P167_data_struct::readMeasRawMYSValue()
{
  uint16_t value=0;
  int16_t valuesign=0;
  uint8_t buffer[9];

  _errmeasrawmys = false;
  if (!readBytes(9, (uint8_t*) &buffer[0], P167_READ_RAW_MEAS_DELAY))
  {
    _errmeasrawmys = true;
    return false;
  }

  String log = F("SEN5x : ***meas MYS value ");
  for(int xx=0; xx<9;xx++)
  {
    log += String((int)buffer[xx]);
    log += F(" ");
  }

  if((buffer[0] == 0xFF && buffer[1] == 0xFF) || (buffer[3] == 0xFF && buffer[4] == 0xFF) || (buffer[6] == 0xFF && buffer[7] == 0xFF))
  {
    log += F("- error");
    addLog(LOG_LEVEL_INFO, log);
    _errmeasrawmys = true;
    _readingerrcount++;
    return false;
  }
  else
  {
    if ((crc8(&buffer[0], 2) == buffer[2]) && (buffer[0] != 0xFF || buffer[1] != 0xFF))
    {
      value  = buffer[0] << 8;
      value += buffer[1];
      _mysHumidity = (float)value/100.0;
    }
    else
    {
      _errmeasrawmys = true;
    }
    if ((crc8(&buffer[3], 2) == buffer[5]) && (buffer[3] != 0xFF || buffer[4] != 0xFF))
    {
      value  = buffer[3] << 8;
      value += buffer[4];
      _mysTemperature = (float)value/200.0;
    }
    else
    {
      _errmeasrawmys = true;
    }
    if ((crc8(&buffer[6], 2) == buffer[8]) && (buffer[6] != 0xFF || buffer[7] != 0xFF))
    {
      valuesign  = buffer[6] << 8;
      valuesign += buffer[7];
      _mysOffset = (float)valuesign/200.0;
    }
    else
    {
      _errmeasrawmys = true;
    }

    if(_errmeasrawmys == true)
    {
      log += F("- crc error");
      _readingerrcount++;
    }
    else
    {
      log += F("- pass");
      _readingsuccesscount++;
    }
    addLog(LOG_LEVEL_INFO, log);
    return !_errmeasrawmys;
  }

  
  return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//  Calculate DewPoint, F Temp, F HUM
bool P167_data_struct::calculateValue()
{
  float lnval;
  float rapval;
  float aval = 17.62;
  float bval = 243.12;
  float Dp;
  float eeval;
  //_TemperatureX = _mysTemperature + _mysOffset - (_mysOffset<0.0?(_mysOffset*(-1.0)):_mysOffset)/2;
  _TemperatureX = _mysTemperature + 1.5 * _mysOffset;

  lnval = logf(_mysHumidity/100.0);
  rapval = (aval * _mysTemperature)/(bval+_mysTemperature);
  Dp = (bval*(lnval+rapval))/(aval-lnval-rapval);
  _DewPoint = Dp;

  eeval = expf((aval*Dp)/(bval+Dp))/expf((aval*_TemperatureX)/(bval+_TemperatureX));

  _HumidityX = eeval*100.0;
  if(_HumidityX < 0.0)
    _HumidityX = 0.0;
  if(_HumidityX > 100.0)
    _HumidityX = 100.0;

  return true;
}

#ifndef LIMIT_BUILD_SIZE
//////////////////////////////////////////////////////////////////////////////////////////////////
//  Retrieve SEN5x identification code
//  Sensirion_SEN5x
bool P167_data_struct::getProductName()
{
  String prodname=F("");
  uint8_t buffer[48];
  //writeCmd(P167_READ_PROD_NAME);
  if (!readBytes(48, (uint8_t *) buffer, P167_READ_PROD_NAME_DELAY))
  {
    return false;
  }
  for (uint8_t i = 1; i <= 16; i++)
  {
    if(crc8(&buffer[i*3-3], 2) == buffer[i*3-1])
    {
      if (buffer[i*3-3] < 32)
        break;
      prodname+=char(buffer[i*3-3]);
      if (buffer[i*3-2] < 32)
        break;
      prodname+=char(buffer[i*3-2]);
    }
  }
  _eid_productname=prodname;

  String log = F("SEN5x : ***Product name: ");
  log += prodname;
  addLog(LOG_LEVEL_INFO, log);

  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//  Retrieve SEN54 Serial Number
bool P167_data_struct::getSerialNumber()
{
  String serno=F("");
  uint8_t buffer[48];
  //writeCmd(P167_READ_SERIAL_NO);
  if (!readBytes(48, (uint8_t *) buffer, P167_READ_SERIAL_NO_DELAY))
  {
    return false;
  }
  for (uint8_t i = 1; i <= 16; i++)
  {
    if(crc8(&buffer[i*3-3], 2) == buffer[i*3-1])
    {
      if (buffer[i*3-3] < 32)
        break;
      serno+=char(buffer[i*3-3]);
      if (buffer[i*3-2] < 32)
        break;
      serno+=char(buffer[i*3-2]);
    }
  }
  _eid_serialnumber=serno;

  String log = F("SEN5x : ***Serial number: ");
  log += serno;
  addLog(LOG_LEVEL_INFO, log);

  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Retrieve SEN54 Firmware version from device
bool P167_data_struct::getFirmwareVersion()
{
  uint8_t version = 0;
  uint8_t read_data[3];
  //writeCmd(P167_READ_FIRM_VER);
  if (!readBytes(3, (uint8_t *) &read_data, P167_READ_FIRM_VER_DELAY))
  {
    return false;
  }
  if( read_data[2] == crc8(&read_data[0],2) )
  {
    version=read_data[0];
  }
  else
  {
    version=0;
  }
  _firmware=version;
  return true;
}
#endif


uint16_t P167_data_struct::getErrCode(bool _clear) 
{
  uint16_t _tmp = _readingerrcode;
  if (_clear == true)
    clearErrCode();
  return (_tmp);
}

uint16_t P167_data_struct::getErrCount(bool _clear) 
{
  uint16_t _tmp = _readingerrcount;
  if (_clear == true)
    clearErrCount();
  return (_tmp);
}

uint16_t P167_data_struct::getSuccCount(bool _clear) 
{
  uint16_t _tmp = _readingsuccesscount;
  if (_clear == true)
    clearSuccCount();
  return (_tmp);
}

void P167_data_struct::clearErrCode() 
{
  _readingerrcode = VIND_ERR_NO_ERROR;
}

void P167_data_struct::clearErrCount() 
{
  _readingerrcount = 0;
}

void P167_data_struct::clearSuccCount() 
{
  _readingsuccesscount = 0;
}


void IRAM_ATTR P167_data_struct::checkPin_interrupt()
{
  //ISR_noInterrupts(); // s0170071: avoid nested interrups due to bouncing.

  monpinValue++;
  monpinLastTransitionTime = getMicros64();
  // Mark pin value changed
  monpinChanged = false;
  if(monpinValue!=monpinValuelast)
    monpinChanged = true;

  //ISR_interrupts(); // enable interrupts again.
}

#endif // USES_P167