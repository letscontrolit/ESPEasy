///////////////////////////////////////////////////////////////////////////////////////////////////
// P167 device class for IKEA Vindstyrka SEN54 and Sensirion SEN5x temperature, humidity and air quality sensors
// See datasheet https://sensirion.com/media/documents/6791EFA0/62A1F68F/Sensirion_Datasheet_Environmental_Node_SEN5x.pdf
// and info about extra request https://sensirion.com/media/documents/2B6FC1F3/6409E74A/PS_AN_Read_RHT_VOC_and_NOx_RAW_signals_D1.pdf
// Based upon code from Rob Tillaart, Viktor Balint, https://github.com/RobTillaart/SHT2x
// Rewritten and adapted for ESPeasy by andibaciu and tonhuisman
// changelog in _P167_Vindstyrka.ino
//////////////////////////////////////////////////////////////////////////////////////////////////

#include "../PluginStructs/P167_data_struct.h"
#include "../ESPEasyCore/ESPEasyGPIO.h"
#include "../Helpers/CRC_functions.h"

#include <GPIO_Direct_Access.h>

#ifdef USES_P167


# define P167_START_MEAS                           0x0021 // Start measurement command
# define P167_START_MEAS_RHT_GAS                   0x0037 // Start measurement RHT/Gas command
# define P167_STOP_MEAS                            0x0104 // Stop measurement command
# define P167_READ_DATA_RDY_FLAG                   0x0202 // Read Data Ready Flag command
# define P167_READ_MEAS                            0x03C4 // Read measurement command
# define P167_R_W_TEMP_COMP_PARAM                  0x60B2 // Read/Write Temperature Compensation Parameters command
# define P167_R_W_TWARM_START_PARAM                0x60C6 // Read/Write Warm Start Parameters command
# define P167_R_W_VOC_ALG_PARAM                    0x60D0 // Read/Write VOC Algorithm Tuning Parameters command
# define P167_R_W_NOX_ALG_PARAM                    0x60E1 // Read/Write NOx Algorithm Tuning Parameters command
# define P167_R_W_RH_T_ACC_Mode                    0x60F7 // Read/Write RH/T Acceleration Mode command
# define P167_R_W_VOC_ALG_STATE                    0x6181 // Read/Write VOC Algorithm State command
# define P167_START_FAN_CLEAN                      0x5607 // Start fan cleaning command
# define P167_R_W_AUTOCLEN_PARAM                   0x8004 // Read/Write Autocleaning Interval Parameters command
# define P167_READ_PROD_NAME                       0xD014 // Read Product Name command
# define P167_READ_SERIAL_NO                       0xD033 // Read Serial Number command
# define P167_READ_FIRM_VER                        0xD100 // Read Firmware Version command
# define P167_READ_DEVICE_STATUS                   0xD206 // Read Device Status command
# define P167_CLEAR_DEVICE_STATUS                  0xD210 // Clear Device Status command
# define P167_RESET_DEVICE                         0xD304 // Reset Device command
# define P167_READ_RAW_MEAS                        0x03D2 // Read relative humidity and temperature
                                                          // which are not compensated for temperature offset, and the
                                                          // VOC and NOx raw signals (proportional to the logarithm of the
                                                          // resistance of the MOX layer). It returns 4x2 bytes (+ 1 CRC
                                                          // byte each) command (see second datasheet fron header for more info)
# define P167_READ_RAW_MYS_MEAS                    0x03F5 // Read relative humidity and temperature and MYSTERY word (probably signed offset
                                                          // temperature)


# define P167_START_MEAS_DELAY                     50     // Timeout value for start measurement command [ms]
# define P167_START_MEAS_RHT_GAS_DELAY             50     // Timeout value for start measurement RHT/Gas command [ms]
# define P167_STOP_MEAS_DELAY                      200    // Timeout value for start measurement command [ms]
# define P167_READ_DATA_RDY_FLAG_DELAY             20     // Timeout value for read data ready flag command [ms]
# define P167_READ_MEAS_DELAY                      20     // Timeout value for read measurement command [ms]
# define P167_R_W_TEMP_COMP_PARAM_DELAY            20     // Timeout value for read/write temperature compensation parameters command [ms]
# define P167_R_W_WARM_START_PARAM_DELAY           20     // Timeout value for read/write warm start parameters command [ms]
# define P167_R_W_VOC_ALG_PARAM_DELAY              20     // Timeout value for read/write VOC algorithm tuning parameters command [ms]
# define P167_R_W_NOX_ALG_PARAM_DELAY              20     // Timeout value for read/write NOx algorithm tuning parameters command [ms]
# define P167_R_W_RH_T_ACC_MODE_DELAY              20     // Timeout value for read/write RH/T acceleration mode command [ms]
# define P167_R_W_VOC_ALG_STATE_DELAY              20     // Timeout value for read/write VOC algorithm State command [ms]
# define P167_START_FAN_CLEAN_DELAY                20     // Timeout value for start fan cleaning command [ms]
# define P167_R_W_AUTOCLEN_PARAM_DELAY             20     // Timeout value for read/write autoclean interval parameters command [ms]
# define P167_READ_PROD_NAME_DELAY                 20     // Timeout value for read product name command [ms]
# define P167_READ_SERIAL_NO_DELAY                 20     // Timeout value for read serial number command [ms]
# define P167_READ_FIRM_VER_DELAY                  20     // Timeout value for read firmware version command [ms]
# define P167_READ_DEVICE_STATUS_DELAY             20     // Timeout value for read device status command [ms]
# define P167_CLEAR_DEVICE_STATUS_DELAY            20     // Timeout value for clear device status command [ms]
# define P167_RESET_DEVICE_DELAY                   100    // Timeout value for reset device command [ms]
# define P167_READ_RAW_MEAS_DELAY                  20     // Timeout value for read raw temp and humidity command [ms]

# define P167_MAX_RETRY                            250    // Give up after amount of retries befoe going to error


const __FlashStringHelper* toString(P167_model model) {
  switch (model) {
    case P167_model::Vindstyrka: return F("IKEA Vindstyrka");
    case P167_model::SEN54: return F("Sensirion SEN54");
    case P167_model::SEN55: return F("Sensirion SEN55");
  }
  return F("");
}

/// @brief
/// @param query
/// @return
const __FlashStringHelper* P167_getQueryString(uint8_t query) {
  switch (query) {
    case 0: return F("Temperature (C)");
    case 1: return F("Humidity (% RH)");
    case 2: return F("tVOC (VOC index)");
    case 3: return F("NOx (NOx index)");
    case 4: return F("PM 1.0 (ug/m3)");
    case 5: return F("PM 2.5 (ug/m3)");
    case 6: return F("PM 4.0 (ug/m3)");
    case 7: return F("PM 10.0 (ug/m3)");
    case 8: return F("DewPoint (C)");
  }
  return F("");
}

/// @brief
/// @param query
/// @return
const __FlashStringHelper* P167_getQueryValueString(uint8_t query) {
  switch (query) {
    case 0: return F("Temperature");
    case 1: return F("Humidity");
    case 2: return F("tVOC");
    case 3: return F("NOx");
    case 4: return F("PM1p0");
    case 5: return F("PM2p5");
    case 6: return F("PM4p0");
    case 7: return F("PM10p0");
    case 8: return F("DewPoint");
  }
  return F("");
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  PUBLIC
//
P167_data_struct::P167_data_struct() {
  //
}

P167_data_struct::~P167_data_struct() {
  //
}

// Initialize/setup device properties
// Must be called at least once before oP167::Wairperating the device
bool P167_data_struct::setupDevice(uint8_t i2caddr) {
  _i2caddr = i2caddr;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, strformat(F("SEN5x: Setup with address= 0x%02x"), _i2caddr));
  }
  return true;
}

bool P167_data_struct::setupMonPin(int16_t monpin) {
  if (validGpio(monpin)) {
    _monpin = monpin;
    pinMode(_monpin, INPUT_PULLUP); // declare monitoring pin as input with pullup's
    attachInterruptArg(digitalPinToInterrupt(_monpin),
                       reinterpret_cast<void (*)(void *)>(Plugin_167_interrupt),
                       this,
                       RISING);

    # ifdef PLUGIN_167_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO, strformat(F("SEN5x: Setup I2C SCL monpin= %d"), _monpin));
    }
    # endif // ifdef PLUGIN_167_DEBUG
    return true;
  }
  return false;
}

void P167_data_struct::disableInterrupt_monpin(void) {
  detachInterrupt(digitalPinToInterrupt(_monpin));
}

// Initialize/setup device properties
// Must be called at least once before operating the device
bool P167_data_struct::setupModel(P167_model model) {
  _model = model;

  # ifdef PLUGIN_167_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, strformat(F("SEN5x: Setup model= %s"), String(toString(_model)).c_str()));
  }
  # endif // ifdef PLUGIN_167_DEBUG
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Evaluate FSM for data acquisition
// This is a state machine that is evaluated step by step by calling update() repetatively
// NOTE: Function is expected to run as critical section w.r.t. other provided functions
//       This is typically met in ESPeasy plugin context when called from within the plugin
bool P167_data_struct::update() {
  bool stable = false; // signals when a stable state is reached

  # ifdef PLUGIN_167_DEBUG
  P167_state oldState = _state;
  # endif // ifdef PLUGIN_167_DEBUG


  if (!statusMonitoring) {
    return stable;
  }


  switch (_state) {
    case P167_state::Uninitialized:

      // we have to stop trying after a while
      if (_errCount > P167_MAX_RETRY) {
        _state = P167_state::Error;
        stable = true;
      } else if (I2C_wakeup(_i2caddr) != 0) { // Try to access the I2C device
        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          addLog(LOG_LEVEL_ERROR, strformat(F("SEN5x: Not found at I2C address: 0x%02x"), _i2caddr));
        }
        _errCount++;
      } else if (_model == P167_model::Vindstyrka) {            // sensor is Vindstyrka and don't need to be reset
        _errCount = 0;                                          // Device is reachable and initialized, reset error counter

        if (writeCmd(P167_READ_FIRM_VER)) {                     // Issue a reset command
          _state               = P167_state::Read_firm_version; // Will take <20ms according to datasheet
          _last_action_started = millis();
        }
      }
      else if ((_model == P167_model::SEN54) || (_model == P167_model::SEN55)) {
        _errCount = 0;                                       // Device is reachable and initialized, reset error counter

        if (writeCmd(P167_RESET_DEVICE)) {                   // Issue a reset command
          _state               = P167_state::Wait_for_reset; // Will take <20ms according to datasheet
          _last_action_started = millis();
        }
      }
      break;

    case P167_state::Wait_for_reset:

      if (timeOutReached(_last_action_started + P167_RESET_DEVICE_DELAY)) { // we need to wait for the chip to reset
        if (I2C_wakeup(_i2caddr) != 0) {
          _errCount++;
          _state = P167_state::Uninitialized;                               // Retry
        } else {
          _errCount = 0;                                                    // Device is reachable and initialized, reset error counter

          if (writeCmd(P167_READ_FIRM_VER)) {
            _state               = P167_state::Read_firm_version;           // Will take <20ms according to datasheet
            _last_action_started = millis();
          }
        }
      }
      break;

    case P167_state::Read_firm_version:

      if (timeOutReached(_last_action_started + P167_READ_FIRM_VER_DELAY)) {
        // Start read flag
        if (!getFirmwareVersion()) {
          _errCount++;
          _state = P167_state::Uninitialized; // Retry
        } else if (!writeCmd(P167_READ_PROD_NAME)) {
          _errCount++;
          _state = P167_state::Uninitialized; // Retry
        } else {
          _last_action_started = millis();
          _state               = P167_state::Read_prod_name;
        }
      }
      break;

    case P167_state::Read_prod_name:

      if (timeOutReached(_last_action_started + P167_READ_PROD_NAME_DELAY)) {
        // Start read flag
        if (!getProductName()) {
          _errCount++;
          _state = P167_state::Uninitialized; // Retry
        } else if (!writeCmd(P167_READ_SERIAL_NO)) {
          _errCount++;
          _state = P167_state::Uninitialized; // Retry
        } else {
          _last_action_started = millis();
          _state               = P167_state::Read_serial_no;
        }
      }
      break;

    case P167_state::Read_serial_no:

      if (timeOutReached(_last_action_started + P167_READ_SERIAL_NO_DELAY)) {
        // Start read flag
        if (!getSerialNumber()) {
          _errCount++;
          _state = P167_state::Uninitialized;        // Retry
        } else if (!writeCmd(P167_READ_SERIAL_NO)) { // Read serialno again?
          _errCount++;
          _state = P167_state::Uninitialized;        // Retry
        } else {
          _last_action_started = millis();
          _state               = P167_state::Initialized;
        }
      }
      break;

    case P167_state::Write_user_reg:
      _state = P167_state::Initialized;
      break;

    case P167_state::Initialized:

      // Trigger the first read cycle automatically on regular SEN5x
      if (_model != P167_model::Vindstyrka) {
        _state = P167_state::Ready;
      }
      break;

    case P167_state::Ready:

      // Ready to execute a measurement cycle, for Vindstyrka we're eavesdropping so no command needed?
      if ((_model == P167_model::Vindstyrka) ||  (_model == P167_model::SEN54) || (_model == P167_model::SEN55)) {
        // Start measuring data
        if (!writeCmd(P167_START_MEAS)) {
          _errCount++;
          _state = P167_state::Uninitialized; // Retry
        } else {
          _last_action_started = millis();
          _state               = P167_state::Wait_for_start_meas;
        }
      }
      break;

    case P167_state::Wait_for_start_meas:

      if (timeOutReached(_last_action_started + P167_START_MEAS_DELAY)) {
        // Start read flag
        if (!writeCmd(P167_READ_DATA_RDY_FLAG)) {
          _errCount++;
          _state = P167_state::Uninitialized; // Retry
        } else {
          _last_action_started = millis();
          _state               = P167_state::Wait_for_read_flag;
        }
      }
      break;

    case P167_state::Wait_for_read_flag:

      if (timeOutReached(_last_action_started + P167_READ_DATA_RDY_FLAG_DELAY)) {
        if (readDataRdyFlag()) {
          // Ready to execute a measurement cycle
          if (!writeCmd(P167_READ_MEAS)) {
            _errCount++;
            _state = P167_state::Uninitialized; // Retry
          } else {
            _last_action_started = millis();
            _state               = P167_state::Wait_for_read_meas;
          }
        } else { // Ready Flag NOT ok, so send again Start Measurement
          // Start measuring data
          if (!writeCmd(P167_START_MEAS)) {
            _errCount++;
            _state = P167_state::Uninitialized; // Retry
          } else {
            _last_action_started = millis();
            _state               = P167_state::Wait_for_start_meas;
          }
        }
      }
      break;

    case P167_state::Wait_for_read_meas:

      if (timeOutReached(_last_action_started + P167_READ_MEAS_DELAY)) {
        if (!readMeasValue()) { // Read the previously measured temperature
          _errCount++;

          // _state = P167_state::Uninitialized; // Lost connection
          _state = P167_state::cmdSTARTmeas;
        } else {
          if (!writeCmd(P167_READ_RAW_MEAS)) {
            _errCount++;
            _state = P167_state::Uninitialized; // Retry
          } else {
            _last_action_started = millis();
            _state               = P167_state::Wait_for_read_raw_meas;
          }
        }
      }
      break;

    case P167_state::Wait_for_read_raw_meas:

      // make sure we wait for the measurement to complete
      if (timeOutReached(_last_action_started + P167_READ_RAW_MEAS_DELAY)) {
        if (!readMeasRawValue()) {
          _errCount++;

          // _state = P167_state::Uninitialized; // Lost connection
          _state = P167_state::cmdSTARTmeas;
        } else {
          if (!writeCmd(P167_READ_RAW_MYS_MEAS)) {
            _errCount++;
            _state = P167_state::Uninitialized; // Retry
          } else {
            _last_action_started = millis();
            _state               = P167_state::Wait_for_read_raw_MYS_meas;
          }
        }
      }
      break;

    case P167_state::Wait_for_read_raw_MYS_meas:

      // make sure we wait for the measurement to complete
      if (timeOutReached(_last_action_started + P167_READ_RAW_MEAS_DELAY)) {
        if (!readMeasRawMYSValue()) {
          _errCount++;

          // _state = P167_state::Uninitialized; // Lost connection
          _state = P167_state::cmdSTARTmeas;
        } else {
          if (!writeCmd(P167_READ_DEVICE_STATUS)) {
            _errCount++;
            _state = P167_state::Uninitialized; // Retry
          } else {
            _last_action_started = millis();
            _state               = P167_state::Wait_for_read_status;
          }
          calculateValue();
          stable = true;
        }
      }
      break;

    case P167_state::Wait_for_read_status:

      // make sure we wait for the measurement to complete
      if (timeOutReached(_last_action_started + P167_READ_DEVICE_STATUS_DELAY)) {
        if (!readDeviceStatus()) {
          _errCount++;

          _state = P167_state::cmdSTARTmeas;
        } else {
          _last_action_started = millis();
          _state               = P167_state::cmdSTARTmeas;
          stable               = true;
        }
      }
      break;

    case P167_state::cmdSTARTmeas:

      // Start measuring data
      if (_model == P167_model::Vindstyrka) {
        if (!writeCmd(P167_START_MEAS)) {
          _errCount++;
          _state = P167_state::Uninitialized; // Retry
        } else {
          _last_action_started = millis();
          _state               = P167_state::IDLE;
        }
      } else {
        _state = P167_state::IDLE;
      }
      break;

    case P167_state::IDLE:
      stepMonitoring      = 1;
      startMonitoringFlag = false;

      if (!_errmeas && !_errmeasraw && !_errmeasrawmys) {
        _state = P167_state::New_Values_Available;
      }
      stable = true;
      break;

    case P167_state::Error:
    case P167_state::New_Values_Available:
      // this state is used outside so all we need is to stay here
      stable = true;
      break;

      // Missing states (enum values) to be checked by the compiler
  } // switch

  # ifdef PLUGIN_167_DEBUG

  if (_state != oldState) {
    if (loglevelActiveFor(LOG_LEVEL_INFO) && _enableLogging) {
      addLog(LOG_LEVEL_INFO, strformat(F("SEN5x: State transition %d-->%d"), static_cast<int>(oldState), static_cast<int>(_state)));
    }
  }
  # endif // ifdef PLUGIN_167_DEBUG
  return stable;
}

bool P167_data_struct::monitorSCL() {
  if (_model == P167_model::Vindstyrka) {
    if (startMonitoringFlag) {
      if (stepMonitoring == 1) {
        lastSCLLowTransitionMonitoringTime = monpinLastTransitionTime / 1000;

        if (millis() - lastSCLLowTransitionMonitoringTime < 100) {
          statusMonitoring = false;
          return true;
        } else {
          lastSCLLowTransitionMonitoringTime = monpinLastTransitionTime / 1000;
          statusMonitoring                   = true;
          stepMonitoring++;
        }
      }

      if (stepMonitoring == 2) {
        if (millis() - lastSCLLowTransitionMonitoringTime < 100) {
          lastSCLLowTransitionMonitoringTime = monpinLastTransitionTime / 1000;
          statusMonitoring                   = false;
          stepMonitoring                     = 1;
          return true;
        } else if (millis() - lastSCLLowTransitionMonitoringTime > 700) {
          statusMonitoring    = false;
          stepMonitoring      = 1;
          startMonitoringFlag = false;

          // if _state not finish reading process then start from begining
          if ((_state >= P167_state::Wait_for_read_meas) && (_state < P167_state::New_Values_Available)) {
            _state = P167_state::Ready;
          }
          return true;
        } else {
          // processing
        }
      }
    }
    monpinValuelast = monpinValue;
  }

  if ((_model == P167_model::SEN54) || (_model == P167_model::SEN55)) {
    statusMonitoring    = true;
    startMonitoringFlag = false;
    stepMonitoring      = 0;
  }

  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Returns the I2C connection state
// Note: based upon the FSM state without actual accessing the device
bool P167_data_struct::isConnected() const {
  switch (_state) {
    case  P167_state::Initialized:
    case  P167_state::Ready:
    case  P167_state::Wait_for_start_meas:
    case  P167_state::Wait_for_read_flag:
    case  P167_state::Wait_for_read_meas:
    case  P167_state::Wait_for_read_raw_meas:
    case  P167_state::Wait_for_read_raw_MYS_meas:
    case  P167_state::Wait_for_read_status:
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

      // Missing states (enum values) to be checked by the compiler
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Returns if the device communication is in error
// Note: based upon the FSM state without actual accessing the device
bool P167_data_struct::inError() const {
  return _state == P167_state::Error;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Returns if new acquired values are available
bool P167_data_struct::newValues() const {
  return _state == P167_state::New_Values_Available;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Restart the FSM used to access the device
bool P167_data_struct::reset() {
  startMonitoringFlag = true;
  stepMonitoring      = 1;
  _state              = P167_state::Uninitialized;
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Start a new measurement cycle
bool P167_data_struct::startMeasurements() {
  if ((_state == P167_state::New_Values_Available) ||
      (_state == P167_state::Initialized)  ||
      (_state == P167_state::IDLE)) {
    _state = P167_state::Ready;
  }
  startMonitoringFlag = true;
  stepMonitoring      = 1;
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Get the electronic idenfification data store in the device
// Note: The data is read from the device during initialization
bool P167_data_struct::getEID(String& eid_productname, String& eid_serialnumber, uint8_t& firmware) const {
  eid_productname  = _eid_productname;
  eid_serialnumber = _eid_serialnumber;
  firmware         = _firmware;
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Get the status informasion about different part of the sensor
// Note: The data is read from the device after every measurement read request
bool P167_data_struct::getStatusInfo(P167_statusinfo param) {
  switch (param) {
    case P167_statusinfo::sensor_speed:
      return _devicestatus.speed;

    case P167_statusinfo::sensor_autoclean:
      return _devicestatus.autoclean;

    case P167_statusinfo::sensor_gas:
      return _devicestatus.gas;

    case P167_statusinfo::sensor_rht:
      return _devicestatus.rht;

    case P167_statusinfo::sensor_laser:
      return _devicestatus.laser;

    case P167_statusinfo::sensor_fan:
      return _devicestatus.fan;
  }
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Return the previously measured raw humidity data [bits]
float P167_data_struct::getRequestedValue(uint8_t request) const {
  switch (request) {
    case 0:
    {
      if (_model == P167_model::Vindstyrka) {
        return _TemperatureX;
      } else {
        return _Temperature;
      }
    }
    case 1:
    {
      if (_model == P167_model::Vindstyrka) {
        return _HumidityX;
      } else {
        return _Humidity;
      }
    }
    case 2:   return _tVOC;
    case 3:   return _NOx;
    case 4:   return _PM1p0;
    case 5:   return _PM2p5;
    case 6:   return _PM4p0;
    case 7:   return _PM10p0;
    case 8:   return _DewPoint;
  }
  return -1.0f;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  PROTECTED
//
//////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////
// bool P167_data_struct::writeCmd(uint8_t cmd)
// {
//  return I2C_write8(_i2caddr, cmd);
// }

//////////////////////////////////////////////////////////////////////////////////////////////////
bool P167_data_struct::writeCmd(uint16_t cmd)
{
  Wire.beginTransmission(_i2caddr);
  Wire.write((uint8_t)(cmd >> 8));
  Wire.write((uint8_t)cmd);
  return Wire.endTransmission() == 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
bool P167_data_struct::writeCmd(uint16_t cmd, uint8_t value) {
  Wire.beginTransmission(_i2caddr);
  Wire.write((uint8_t)(cmd >> 8));
  Wire.write((uint8_t)cmd);
  Wire.write((uint8_t)value);
  return Wire.endTransmission() == 0;
}

bool P167_data_struct::writeCmd(uint16_t cmd, uint8_t length, uint8_t *buffer) {
  Wire.beginTransmission(_i2caddr);
  Wire.write((uint8_t)(cmd >> 8));
  Wire.write((uint8_t)cmd);

  for (int i = 0; i < length; ++i) {
    Wire.write(*(buffer + i));
  }
  return Wire.endTransmission() == 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
bool P167_data_struct::readBytes(uint8_t n, uint8_t *val, uint8_t maxDuration) {
  // TODO check if part can be delegated to the I2C_access libraray from ESPeasy
  Wire.requestFrom(_i2caddr, (uint8_t)n);
  uint32_t start = millis();

  while (Wire.available() < n) {
    if (timePassedSince(start) > maxDuration) {
      return false;
    }
    yield();
  }

  for (uint8_t i = 0; i < n; i++) {
    val[i] = Wire.read();
  }
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Read data ready flag from device
bool P167_data_struct::readDataRdyFlag() {
  uint8_t value = 0;
  uint8_t buffer[3];

  if (!readBytes(3, (uint8_t *)&buffer[0], P167_READ_DATA_RDY_FLAG_DELAY)) {
    return false;
  }

  if (calc_CRC8(&buffer[0], 2) == buffer[2]) {
    value += buffer[1];
  }
  return value;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Read measurement values results from device
bool P167_data_struct::readMeasValue() {
  uint16_t value     = 0;
  int16_t  valuesign = 0;
  uint8_t  buffer[24];
  bool     condition = true;

  _errmeas = false;

  if (!readBytes(24, (uint8_t *)&buffer[0], P167_READ_MEAS_DELAY)) {
    _errmeas = true;
    return false;
  }

  String log = F("SEN5x: Measured value ");

  for (int xx = 0; xx < 24; ++xx) {
    log += buffer[xx];
    log += ' ';
  }

  if ((_model == P167_model::Vindstyrka) || (_model == P167_model::SEN54)) {
    condition = (buffer[0] == 0xFF && buffer[1] == 0xFF) || (buffer[3] == 0xFF && buffer[4] == 0xFF) ||
                (buffer[6] == 0xFF && buffer[7] == 0xFF) || (buffer[9] == 0xFF && buffer[10] == 0xFF) ||
                (buffer[12] == 0xFF && buffer[13] == 0xFF) ||
                (buffer[15] == 0xFF && buffer[16] == 0xFF) || (buffer[18] == 0xFF && buffer[19] == 0xFF);
  }

  if (_model == P167_model::SEN55) {
    condition = (buffer[0] == 0xFF && buffer[1] == 0xFF) || (buffer[3] == 0xFF && buffer[4] == 0xFF) ||
                (buffer[6] == 0xFF && buffer[7] == 0xFF) || (buffer[9] == 0xFF && buffer[10] == 0xFF) ||
                (buffer[12] == 0xFF && buffer[13] == 0xFF) ||
                (buffer[15] == 0xFF && buffer[16] == 0xFF) || (buffer[18] == 0xFF && buffer[19] == 0xFF) ||
                (buffer[21] == 0xFF && buffer[22] == 0xFF);
  }

  if (condition) {
    log += F("- error");

    if (_enableLogging) {
      addLog(LOG_LEVEL_ERROR, log);
    }
    _errmeas = true;
    _readingerrcount++;
    return false;
  } else {
    for (int xx = 0; xx < 8; ++xx) {
      if ((calc_CRC8(&buffer[xx * 3], 2) == buffer[xx * 3 + 2]) && ((buffer[xx * 3] != 0xFF) || (buffer[xx * 3 + 1] != 0xFF))) {
        value      = buffer[xx * 3] << 8;
        value     += buffer[xx * 3 + 1];
        valuesign  = buffer[xx * 3] << 8;
        valuesign += buffer[xx * 3 + 1];

        if (xx == 0) {
          _PM1p0 = value / 10.0f;
        }

        if (xx == 1) {
          _PM2p5 = value / 10.0f;
        }

        if (xx == 2) {
          _PM4p0 = value / 10.0f;
        }

        if (xx == 3) {
          _PM10p0 = value / 10.0;
        }

        if (xx == 4) {
          _Humidity = valuesign / 100.0f;
        }

        if (xx == 5) {
          _Temperature = valuesign / 200.0f;
        }

        if (xx == 6) {
          _tVOC = valuesign / 10.0f;
        }

        if (xx == 7)
        {
          if (_model == P167_model::SEN55) {
            _NOx = valuesign / 10.0f;
          } else {
            _NOx = 0.0f;
          }
        }
      } else {
        _errmeasrawmys = true;
      }
    }

    if (_errmeas) {
      log += F("- crc error");
      _readingerrcount++;
    } else {
      log += F("- pass");
      _readingsuccesscount++;

      if (_enableLogging) {
        addLog(LOG_LEVEL_INFO, log);
      }
      return !_errmeas;
    }
  }

  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Read measurement values results from device
bool P167_data_struct::readMeasRawValue() {
  uint16_t value     = 0;
  int16_t  valuesign = 0;
  uint8_t  buffer[12];
  bool     condition = true;

  _errmeasraw = false;

  if (!readBytes(12, &buffer[0], P167_READ_RAW_MEAS_DELAY)) {
    _errmeasraw = true;
    return false;
  }

  String log = F("SEN5x: Measured RAW value 0x");

  for (int xx = 0; xx < 12; xx++) {
    log += strformat(F("%02x "), buffer[xx]);
  }

  if ((_model == P167_model::Vindstyrka) || (_model == P167_model::SEN54)) {
    condition = (buffer[0] == 0xFF && buffer[1] == 0xFF) || (buffer[3] == 0xFF && buffer[4] == 0xFF) ||
                (buffer[6] == 0xFF && buffer[7] == 0xFF); // || (buffer[9] == 0xFF && buffer[10] == 0xFF))
  }

  if (_model == P167_model::SEN55) {
    condition = (buffer[0] == 0xFF && buffer[1] == 0xFF) || (buffer[3] == 0xFF && buffer[4] == 0xFF) ||
                (buffer[6] == 0xFF && buffer[7] == 0xFF) || (buffer[9] == 0xFF && buffer[10] == 0xFF);
  }

  if (condition) {
    log += F("- error");

    if (_enableLogging) {
      addLog(LOG_LEVEL_ERROR, log);
    }
    _errmeasraw = true;
    _readingerrcount++;
    return false;
  } else {
    for (int xx = 0; xx < 4; ++xx) {
      if ((calc_CRC8(&buffer[xx * 3], 2) == buffer[xx * 3 + 2]) && ((buffer[xx * 3] != 0xFF) || (buffer[xx * 3 + 1] != 0xFF))) {
        value      = buffer[xx * 3] << 8;
        value     += buffer[xx * 3 + 1];
        valuesign  = buffer[xx * 3] << 8;
        valuesign += buffer[xx * 3 + 1];

        if (xx == 0) {
          _rawHumidity = valuesign / 100.0f;
        } else

        if (xx == 1) {
          _rawTemperature = valuesign / 200.0f;
        } else

        if (xx == 2) {
          _rawtVOC = value / 10.0f;
        } else

        if (xx == 3) {
          if (_model == P167_model::SEN55) {
            _rawNOx = value / 10.0f;
          } else {
            _rawNOx = 0.0f;
          }
        }
      } else {
        _errmeasrawmys = true;
      }
    }

    if (_errmeasraw) {
      log += F("- crc error");
      _readingerrcount++;
    } else {
      log += F("- pass");
      _readingsuccesscount++;
    }

    if (_enableLogging) {
      addLog(LOG_LEVEL_INFO, log);
    }
    return !_errmeasraw;
  }


  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Read measurement values results from device
bool P167_data_struct::readMeasRawMYSValue() {
  int16_t valuesign = 0;
  uint8_t buffer[9];

  _errmeasrawmys = false;

  if (!readBytes(9, (uint8_t *)&buffer[0], P167_READ_RAW_MEAS_DELAY)) {
    _errmeasrawmys = true;
    return false;
  }

  String log = F("SEN5x: Measured MYS value 0x");

  for (int xx = 0; xx < 9; ++xx) {
    log += strformat(F("%02x "), buffer[xx]);
  }

  if (((buffer[0] == 0xFF) && (buffer[1] == 0xFF)) ||
      ((buffer[3] == 0xFF) && (buffer[4] == 0xFF)) ||
      ((buffer[6] == 0xFF) && (buffer[7] == 0xFF))) {
    log += F("- error");

    if (_enableLogging) {
      addLog(LOG_LEVEL_ERROR, log);
    }
    _errmeasrawmys = true;
    _readingerrcount++;
    return false;
  } else {
    for (int xx = 0; xx < 3; ++xx) {
      if ((calc_CRC8(&buffer[xx * 3], 2) == buffer[xx * 3 + 2]) && ((buffer[xx * 3] != 0xFF) || (buffer[xx * 3 + 1] != 0xFF))) {
        valuesign  = buffer[xx * 3] << 8;
        valuesign += buffer[xx * 3 + 1];

        if (xx == 0) {
          _mysHumidity = valuesign / 100.0f;
        } else

        if (xx == 1) {
          _mysTemperature = valuesign / 200.0f;
        } else

        if (xx == 2) {
          _mysOffset = valuesign / 200.0f;
        }
      } else {
        _errmeasrawmys = true;
      }
    }

    if (_errmeasrawmys) {
      log += F("- crc error");
      _readingerrcount++;
    } else {
      log += F("- pass");
      _readingsuccesscount++;
    }

    if (_enableLogging) {
      addLog(LOG_LEVEL_INFO, log);
    }
    return !_errmeasrawmys;
  }


  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//  Calculate DewPoint, F Temp, F HUM
bool P167_data_struct::calculateValue() {
  if (_model == P167_model::Vindstyrka) {
    _TemperatureX = _mysTemperature + _mysOffset - 2.4f; // (2.4 - temperature offset because enclosure and esp8266 power disipation)


    // version formula with interpolation
    _HumidityX = _Humidity + (_TemperatureX - _Temperature) * ((_rawHumidity - _Humidity) / (_rawTemperature - _Temperature));

    _DewPoint = compute_dew_point_temp(_TemperatureX, _HumidityX);

    if (_HumidityX < 0.0f) {
      _HumidityX = 0.0f;
    }

    if (_HumidityX > 100.0f) {
      _HumidityX = 100.0f;
    }
  } else {
    _DewPoint = compute_dew_point_temp(_Temperature, _Humidity);
  }

  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//  Retrieve SEN5x identification code
//  Sensirion_SEN5x
bool P167_data_struct::getProductName() {
  String  prodname;
  uint8_t buffer[48];

  // writeCmd(P167_READ_PROD_NAME);
  if (!readBytes(48, (uint8_t *)buffer, P167_READ_PROD_NAME_DELAY)) {
    return false;
  }

  for (uint8_t i = 1; i <= 16; ++i) {
    if (calc_CRC8(&buffer[i * 3 - 3], 2) == buffer[i * 3 - 1]) {
      if (buffer[i * 3 - 3] < 32) {
        break;
      }
      prodname += char(buffer[i * 3 - 3]);

      if (buffer[i * 3 - 2] < 32) {
        break;
      }
      prodname += char(buffer[i * 3 - 2]);
    }
  }
  _eid_productname = prodname;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, concat(F("SEN5x: Product name: "), prodname));
  }

  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//  Retrieve SEN54 Serial Number
bool P167_data_struct::getSerialNumber() {
  String  serno;
  uint8_t buffer[48];

  // writeCmd(P167_READ_SERIAL_NO);
  if (!readBytes(48, (uint8_t *)buffer, P167_READ_SERIAL_NO_DELAY)) {
    return false;
  }

  for (uint8_t i = 1; i <= 16; ++i) {
    if (calc_CRC8(&buffer[i * 3 - 3], 2) == buffer[i * 3 - 1]) {
      if (buffer[i * 3 - 3] < 32) {
        break;
      }
      serno += char(buffer[i * 3 - 3]);

      if (buffer[i * 3 - 2] < 32) {
        break;
      }
      serno += char(buffer[i * 3 - 2]);
    }
  }
  _eid_serialnumber = serno;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, concat(F("SEN5x: Serial number: "), serno));
  }

  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Retrieve SEN54 Firmware version from device
bool P167_data_struct::getFirmwareVersion() {
  uint8_t version = 0;
  uint8_t read_data[3];

  // writeCmd(P167_READ_FIRM_VER);
  if (!readBytes(3, (uint8_t *)&read_data, P167_READ_FIRM_VER_DELAY)) {
    return false;
  }

  if (read_data[2] == calc_CRC8(&read_data[0], 2)) {
    version = read_data[0];
  }
  _firmware = version;

  addLog(LOG_LEVEL_INFO, strformat(F("SEN5x: Firmware version: %d"), version));

  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Retrieve SEN54 Device Status from device
bool P167_data_struct::readDeviceStatus() {
  uint32_t value = 0;
  uint8_t  bufferstatus[6];

  // writeCmd(P167_READ_FIRM_VER);
  _errdevicestatus = false;

  if (!readBytes(6, (uint8_t *)&bufferstatus, P167_READ_DEVICE_STATUS_DELAY)) {
    _errdevicestatus = true;
    return false;
  }

  String log = F("SEN5x: device status 0x");

  for (int xx = 0; xx < 6; ++xx) {
    log += strformat(F("%02x "), bufferstatus[xx]);
  }

  if ((calc_CRC8(&bufferstatus[0], 2) == bufferstatus[2]) && (calc_CRC8(&bufferstatus[3], 2) == bufferstatus[5])) {
    value             = bufferstatus[0] << 24;
    value            += bufferstatus[1] << 16;
    value            += bufferstatus[3] << 8;
    value            += bufferstatus[4] << 0;
    _devicestatus.val = value;
  } else {
    _errdevicestatus = true;
  }

  if (_errdevicestatus) {
    log += F("- crc error");
    _readingerrcount++;
  } else {
    log += strformat(F(", flags: sp:%d cln:%d gas:%d rht:%d las:%d fan:%d - pass"),
                     _devicestatus.speed,
                     _devicestatus.autoclean,
                     _devicestatus.gas,
                     _devicestatus.rht,
                     _devicestatus.laser,
                     _devicestatus.fan);
    _readingsuccesscount++;
  }

  if (_enableLogging) {
    addLog(LOG_LEVEL_INFO, log);
  }
  return !_errdevicestatus;
}

uint16_t P167_data_struct::getErrCode(bool _clear) {
  uint16_t _tmp = _readingerrcode;

  if (_clear == true) {
    clearErrCode();
  }
  return _tmp;
}

uint16_t P167_data_struct::getErrCount(bool _clear) {
  uint16_t _tmp = _readingerrcount;

  if (_clear == true) {
    clearErrCount();
  }
  return _tmp;
}

uint16_t P167_data_struct::getSuccCount(bool _clear) {
  uint16_t _tmp = _readingsuccesscount;

  if (_clear == true) {
    clearSuccCount();
  }
  return _tmp;
}

void P167_data_struct::clearErrCode() {
  _readingerrcode = VIND_ERR_NO_ERROR;
}

void P167_data_struct::clearErrCount() {
  _readingerrcount = 0;
}

void P167_data_struct::clearSuccCount() {
  _readingsuccesscount = 0;
}

void P167_data_struct::checkPin_interrupt() {
  monpinValue              = monpinValue + 1; // volatile
  monpinLastTransitionTime = getMicros64();

  // Mark pin value changed
  monpinChanged = false;

  if (monpinValue != monpinValuelast) {
    monpinChanged = true;
  }
}

void P167_data_struct::setLogging(bool logStatus) {
  _enableLogging = logStatus;
}

void P167_data_struct::startCleaning() {
  writeCmd(P167_START_FAN_CLEAN); // Don't wait for a response, as the command causes the fan to run for 10 seconds
}

// When using interrupts we have to call the library entry point
// whenever an interrupt is triggered
void P167_data_struct::Plugin_167_interrupt(P167_data_struct *self) {
  // addLog(LOG_LEVEL_ERROR, F("********* SEN5X: interrupt apear!"));
  if (self) {
    self->checkPin_interrupt();
  }
}

#endif // USES_P167
