///////////////////////////////////////////////////////////////////////////////////////////////////
// Plugin data structure for P164 "GASES - ENS16x (TVOC, eCO2)"
// Plugin for ENS160 & ENS161 TVOC and eCO2 sensor with I2C interface from ScioSense
// Based upon: https://github.com/sciosense/ENS160_driver
// For documentation see 
// https://www.sciosense.com/wp-content/uploads/documents/SC-001224-DS-9-ENS160-Datasheet.pdf
//
// Based upon: https://github.com/sciosense/ENS160_driver
// MIT License for the original code referenced above
// Copyright (c) 2020 Sciosense
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// 2023 By flashmark
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "../PluginStructs/P164_data_struct.h"

#ifdef USES_P164

// A curious delay inserted in the original code [ms] 
#define ENS160_BOOTING          10
// Max time for device to react on a reset [ms]
#define ENS160_MAXBOOTING       2000

// ENS160 registers for version V0
#define ENS160_REG_PART_ID      0x00    // 2 byte register for part identification
#define ENS160_REG_OPMODE       0x10    // Operation mode register
#define ENS160_REG_CONFIG       0x11    // Pin configuration register
#define ENS160_REG_COMMAND      0x12    // Additional system commands
#define ENS160_REG_TEMP_IN      0x13    // Host ambient temperature information
#define ENS160_REG_RH_IN        0x15    // Host relative humidity information
#define ENS160_REG_DATA_STATUS  0x20    // Operating mode status readback
#define ENS160_REG_DATA_AQI     0x21    // Air quality according to index according to UBA
#define ENS160_REG_DATA_TVOC    0x22    // Equivalent TVOC concentartion (ppb)
#define ENS160_REG_DATA_ECO2    0x24		// Equivalent CO2 concentration (ppm)
#define ENS160_REG_DATA_AQI_S   0x26    // Relative Air Quality Index according to ScioSense [ENS161 only]
#define ENS160_REG_DATA_BL      0x28    // Undocumented
#define ENS160_REG_DATA_T       0x30    // Temperature used in calculations
#define ENS160_REG_DATA_RH      0x32    // Relative humidity used in calculations
#define ENS160_REG_DATA_MISR    0x38    //Data integrity field (optional)
#define ENS160_REG_GPR_WRITE_0  0x40    // General purpose write registers [8 bytes]
#define ENS160_REG_GPR_WRITE_1  (ENS160_REG_GPR_WRITE_0 + 1)
#define ENS160_REG_GPR_WRITE_2  (ENS160_REG_GPR_WRITE_0 + 2)
#define ENS160_REG_GPR_WRITE_3  (ENS160_REG_GPR_WRITE_0 + 3)
#define ENS160_REG_GPR_WRITE_4  (ENS160_REG_GPR_WRITE_0 + 4)
#define ENS160_REG_GPR_WRITE_5  (ENS160_REG_GPR_WRITE_0 + 5)
#define ENS160_REG_GPR_WRITE_6  (ENS160_REG_GPR_WRITE_0 + 6)
#define ENS160_REG_GPR_WRITE_7  (ENS160_REG_GPR_WRITE_0 + 7)
#define ENS160_REG_GPR_READ_0   0x48    // General purpose read registers [8 bytes]
#define ENS160_REG_GPR_READ_1   (ENS160_REG_GPR_READ_0 + 1)
#define ENS160_REG_GPR_READ_2   (ENS160_REG_GPR_READ_0 + 2)
#define ENS160_REG_GPR_READ_3   (ENS160_REG_GPR_READ_0 + 3)
#define ENS160_REG_GPR_READ_4   (ENS160_REG_GPR_READ_0 + 4)
#define ENS160_REG_GPR_READ_5   (ENS160_REG_GPR_READ_0 + 5)
#define ENS160_REG_GPR_READ_6   (ENS160_REG_GPR_READ_0 + 6)
#define ENS160_REG_GPR_READ_7   (ENS160_REG_GPR_READ_0 + 7)

// ENS160_REG_PART_ID values for Chip ID 
#define ENS160_PARTID           0x0160  // ENS160
#define ENS161_PARTID           0x0161  // ENS161

//ENS160 COMMAND register values
#define ENS160_COMMAND_NOP          0x00    // NOP, No operation
#define ENS160_COMMAND_CLRGPR       0xCC    // CLRGRP, Clears GPR Read Registers
#define ENS160_COMMAND_GET_APPVER   0x0E    // GET_APPVER, Get firmware version
#define ENS160_COMMAND_SETTH        0x02    // Not specified in datasheet
#define ENS160_COMMAND_SETSEQ       0xC2    // Not specified in datasheet

// ENS160 OPMODE register values
#define ENS160_OPMODE_RESET         0xF0    // RESET 
#define ENS160_OPMODE_DEEP_SLEEP    0x00    // DEEPSLEEP
#define ENS160_OPMODE_IDLE          0x01    // IDLE
#define ENS160_OPMODE_STD           0x02    // STANDARD
#define ENS160_OPMODE_LP            0x03    // LOW POWER (ENS161 only)
#define ENS160_OPMODE_ULP           0x04    // ULTRA LOW POWER (ENS161 only)
#define ENS160_OPMODE_CUSTOM        0xC0    // Not specified in datasheet

// ENS160 unspecified bitfields?
#define ENS160_BL_CMD_START         0x02
#define ENS160_BL_CMD_ERASE_APP     0x04
#define ENS160_BL_CMD_ERASE_BLINE   0x06
#define ENS160_BL_CMD_WRITE         0x08
#define ENS160_BL_CMD_VERIFY        0x0A
#define ENS160_BL_CMD_GET_BLVER     0x0C
#define ENS160_BL_CMD_GET_APPVER    0x0E
#define ENS160_BL_CMD_EXITBL        0x12

// ENS160 unspecified bitfields?
#define ENS160_SEQ_ACK_NOTCOMPLETE  0x80
#define ENS160_SEQ_ACK_COMPLETE     0xC0

#define IS_ENS160_SEQ_ACK_NOT_COMPLETE(x)   (ENS160_SEQ_ACK_NOTCOMPLETE == (ENS160_SEQ_ACK_NOTCOMPLETE & (x)))
#define IS_ENS160_SEQ_ACK_COMPLETE(x)       (ENS160_SEQ_ACK_COMPLETE == (ENS160_SEQ_ACK_COMPLETE & (x)))

// ENS160 STATUS bitfields
#define ENS160_STATUS_STATAS        0x80    // STATAS: Indicates that an OPMODE is running
#define ENS160_STATUS_STATER        0x40    // STATER: High indicated that an error is detected
#define ENS160_STATUS_VALIDITY      0x0C    // VALIDITY FLAG
#define ENS160_STATUS_VAL_NORM      0x00    //   0: Normal operation
#define ENS160_STATUS_VAL_WARM      0x01    //   1: Warm-Up phase
#define ENS160_STATUS_VAL_NOUSE     0x02    //   2: Not used
#define ENS160_STATUS_VAL_INVAL     0x03    //   3: Invalid output
#define ENS160_STATUS_NEWDAT        0x02    // NEWDAT: 1= New data in data registers available
#define ENS160_STATUS_NEWGPR        0x01    // NEWGRP: 1= New data in GRP_READ registers available 

// Checkers for bitfields in STATUS register
#define IS_NEWDAT(x)                (ENS160_STATUS_NEWDAT == (ENS160_STATUS_NEWDAT & (x)))
#define IS_NEWGPR(x)                (ENS160_STATUS_NEWGPR == (ENS160_STATUS_NEWGPR & (x)))
#define IS_NEW_DATA_AVAILABLE(x)    (0 != ((ENS160_STATUS_NEWDAT | ENS160_STATUS_NEWGPR ) & (x)))
#define GET_STATUS_VALIDITY(x)      (((x) & ENS160_STATUS_VALIDITY) >> 2)

// TODO: add comment on this
#define CONVERT_RS_RAW2OHMS_I(x)    (1 << ((x) >> 11))
#define CONVERT_RS_RAW2OHMS_F(x)    (pow (2, (float)(x) / 2048))

// Form IDs used on the device setup page. Should be a short unique string.
#define P164_GUID_TEMP_T     "f01"
#define P164_GUID_TEMP_V     "f02"
#define P164_GUID_HUM_T      "f03"
#define P164_GUID_HUM_V      "f04"


///////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////
P164_data_struct::P164_data_struct(struct EventStruct *event) :
  i2cAddress(P164_PCONFIG_I2C_ADDR)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Initialization of the connected device                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::begin()
{
  // Start connecting the device over I2C
  if (!start(i2cAddress)) {
    addLogMove(LOG_LEVEL_ERROR, F("P164: device initialization FAILED"));
    return false;
  }
  setMode(ENS160_OPMODE_STD); // For now we only support the standard acquisition mode

  #ifdef P164_ENS160_DEBUG
    addLogMove(LOG_LEVEL_DEBUG, F("P164: begin(): success"));
  #endif
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Fetch the processed device values as stored in the software object                            //
///////////////////////////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::read(float& tvoc, float& eco2, float& aqi)
{
  bool success = measure();       // Read measurement values from device
  tvoc = (float)_data_tvoc;       // Latest acquired TVOC value
  eco2 = (float)_data_eco2;       // Latest aquired eCO2 value
  aqi = (float)_data_aqi;         // Latest aquired AQI value
  return success;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Fetch the processed device values as stored in the software object using compensation         //
///////////////////////////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::read(float& tvoc, float& eco2, float& aqi, float temp, float hum)
{
  this->set_envdata(temp, hum);   // Write new compensation temp & hum to device
  bool success = measure();       // Read measurement values from device
  tvoc = (float)_data_tvoc;       // Latest acquired TVOC value
  eco2 = (float)_data_eco2;       // Latest aquired eCO2 value
  aqi = (float)_data_aqi;         // Latest aquired AQI value
  return success;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of plugin PLUGIN_WEBFORM_LOAD call                                             //
// Note: this is not a class function, only data from event is available                         //
///////////////////////////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::webformLoad(struct EventStruct *event)
{
  bool  found = false; // A chip has responded at the I2C address
  uint16_t chipID = 0;

  addRowLabel(F("Detected Sensor Type"));
  chipID = I2C_read16_LE_reg(P164_PCONFIG_I2C_ADDR, ENS160_REG_PART_ID, &found);
  if (!found) {
    addHtml(F("No device found"));
  } else if (chipID == ENS160_PARTID) {
    addHtml(F("ENS160"));
  } else if (chipID == ENS161_PARTID) {
    addHtml(F("ENS161"));
  } else {
    addHtmlInt(chipID);
  }

  P164_data_struct *P164_data = static_cast<P164_data_struct *>(getPluginTaskData(event->TaskIndex));
  if (P164_data != nullptr) {
    addHtml(F(" Firmware: "));
    addHtmlInt(P164_data->getMajorRev());
    addHtml(F("."));
    addHtmlInt(P164_data->getMinorRev());    
    addHtml(F("."));
    addHtmlInt(P164_data->getBuild());
  }

  addFormNote(F("Both Temperature and Humidity task & values are needed to enable compensation"));
  // temperature
  addRowLabel(F("Temperature Task"));
  addTaskSelect(F(P164_GUID_TEMP_T), P164_PCONFIG_TEMP_TASK);
  if (validTaskIndex(P164_PCONFIG_TEMP_TASK))
  {
    LoadTaskSettings(P164_PCONFIG_TEMP_TASK); // we need to load the values from another task for selection!
    addRowLabel(F("Temperature Value"));
    addTaskValueSelect(F(P164_GUID_TEMP_V), P164_PCONFIG_TEMP_VAL, P164_PCONFIG_TEMP_TASK);
  }
  // humidity
  addRowLabel(F("Humidity Task"));
  addTaskSelect(F(P164_GUID_HUM_T), P164_PCONFIG_HUM_TASK);
  if (validTaskIndex(P164_PCONFIG_HUM_TASK))
  {
    LoadTaskSettings(P164_PCONFIG_HUM_TASK); // we need to load the values from another task for selection!
    addRowLabel(F("Humidity Value"));
    addTaskValueSelect(F(P164_GUID_HUM_V), P164_PCONFIG_HUM_VAL, P164_PCONFIG_HUM_TASK);
  }
  LoadTaskSettings(event->TaskIndex); // we need to restore our original taskvalues!

  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Implementation of plugin PLUGIN_WEBFORM_SAVE call                                            //
// Note: this is not a class function, only data from event is available                         //
///////////////////////////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::webformSave(struct EventStruct *event)
{
  P164_PCONFIG_I2C_ADDR  = getFormItemInt(F("i2c_addr"));
  P164_PCONFIG_TEMP_TASK = getFormItemInt(F(P164_GUID_TEMP_T));
  P164_PCONFIG_TEMP_VAL  = getFormItemInt(F(P164_GUID_TEMP_V));
  P164_PCONFIG_HUM_TASK  = getFormItemInt(F(P164_GUID_HUM_T));
  P164_PCONFIG_HUM_VAL   = getFormItemInt(F(P164_GUID_HUM_V));
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of plugin PLUGIN_TEN_PER_SECOND call                                           //
///////////////////////////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::tenPerSecond(struct EventStruct *event)
{
  return evaluateState();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Set operation mode of sensor                                                                  //
// Note: This function is to set the operation mode to the plugin software structure only        //
//       The statemachine shall handle the actual programming of the OPMODE register             //
///////////////////////////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::setMode(uint8_t mode) {
  bool result = false;

  // LP only valid for rev>0
  if (!(mode == ENS160_OPMODE_LP) and (_revENS16x == 0)) {
    this->_opmode = mode;
    result        = true;
  }

  #ifdef P164_ENS160_DEBUG
    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log;
      log += F("P164: setMode(");
      log += mode;
      log += F(")");
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
  #endif // ifdef P164_ENS160_DEBUG

  return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//                     **** The sensor handling code ****                                        //
// This is based upon Sciosense code on github                                                   //
// The code is adapted to fit the ESPEasy structures and statemachine behavior is added          //
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// Helper function to display the current state in readable format                               //
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef P164_ENS160_DEBUG
String printState(int state) {
  switch (state) {
    case P164_STATE_INITIAL:      return F("initial"); break;
    case P164_STATE_ERROR:        return F("error"); break;
    case P164_STATE_RESETTING:    return F("resetting"); break;
    case P164_STATE_STARTING1:    return F("starting1"); break;
    case P164_STATE_STARTING2:    return F("starting2"); break;
    case P164_STATE_IDLE:         return F("idle"); break;
    case P164_STATE_DEEPSLEEP:    return F("deepsleep"); break;
    case P164_STATE_OPERATIONAL:  return F("operational"); break;
    default:                      return F("***ERROR***"); break;
  }
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// Evaluate the plugin statemachine and determine next step                                      //
// To prevent using delay() to wait for the device responses a statemachine is introduced        //
// Main states follow the device states according datasheet:                                     //
//   + IDLE         Enabled, waiting for commands                                                //
//   + DEEP SLEEP   Low power standby                                                            //
//   + OPERATIONAL  Active gas sensing                                                           //
// Added states to administrate plugin software status:                                          //
//   + INITIAL      Class is constructed, waiting for begin()                                    //
//   + ERROR        Communication with the device failed or other fatal error conditions         //
//   + RESETTING    Waiting for device reset to be finished                                      //
//   + STARTING1    Startup sequence waiting for previous command to be acknowledged             //
//   + STARTING2    Startup sequence waiting for previous command to be acknowledged             //
// Note that the ENS161 device has various gas sensing operation modes which all map to the      //
// same OPERATIONAL state. These are combined in the same OPMODE register in the device          //
//   - STANDARD                                                                                  //
//   - LOW POWER                                                                                 //
//   - ULTRA LOW POWER                                                                           //
// The same OPMODE register is also used to reset the device.                                    //
// Thus OPMODE register shall not be confused with this software state                           //
///////////////////////////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::evaluateState()
{
  bool success  = true;                // State transition went without problems with device
  P164_state  newState = this->_state; // Determine next state, start with current state

  switch (this->_state) {
    case P164_STATE_INITIAL:
      // Waiting for external call to begin()
      this->_available = false;
      break;
    case P164_STATE_ERROR:
      // Stay here until correct device ID is detected and status register can be read
      // If there is a proper device connected then reset it and initialize it
      this->_available = false;
      success = this->checkPartID() && this->getStatus();
      if (success){
        success = this->writeMode(ENS160_OPMODE_RESET);  // Reset the device, takes some time
        newState = P164_STATE_RESETTING;
      }
      break;
    case P164_STATE_RESETTING:
      // Device has been reset, give it some time to get accessible again
      this->_available = false;

      // Once device has rebooted check if it is a supported device at the given I2C address
      if (timePassedSince(this->_lastChange) > ENS160_BOOTING) { 
        if (this->checkPartID())
        {
          newState = P164_STATE_STARTING1;
        }
        else if (timePassedSince(this->_lastChange) > ENS160_MAXBOOTING) {
          newState = P164_STATE_ERROR;
        }
      }
      break;
    case P164_STATE_STARTING1:
      // A valid device is found, check if its status is ready to continue
      this->_available = false;
      this->getStatus();
      if (GET_STATUS_VALIDITY(this->_statusReg) == ENS160_STATUS_VAL_NORM)
      {
        this->writeMode(ENS160_OPMODE_IDLE);
        this->clearCommand();
        newState = P164_STATE_STARTING2;
      }
      break;
    case P164_STATE_STARTING2:
      this->_available = false;
      this->getStatus();
      if (GET_STATUS_VALIDITY(this->_statusReg) == ENS160_STATUS_VAL_NORM) {
        this->getFirmware();
        newState = P164_STATE_IDLE;
      }
      break;
    case P164_STATE_IDLE:
      // Set device into desired operation mode as requested through _opmode
      this->_available = true;

      switch (this->_opmode) {
        case ENS160_OPMODE_STD:
          this->writeMode(ENS160_OPMODE_STD);
          newState = P164_STATE_OPERATIONAL;
          break;
        case ENS160_OPMODE_LP:
          this->writeMode(ENS160_OPMODE_LP);
          newState = P164_STATE_OPERATIONAL;
          break;
        case ENS160_OPMODE_ULP:
          this->writeMode(ENS160_OPMODE_ULP);
          newState = P164_STATE_OPERATIONAL;
          break;
        case ENS160_OPMODE_RESET:
          this->writeMode(ENS160_OPMODE_RESET);
          this->_opmode = ENS160_OPMODE_IDLE; // Prevent reset loop
          newState      = P164_STATE_RESETTING;
          break;
      }
      break;
    case P164_STATE_DEEPSLEEP:
      // Device is put to DEEPSLEEP mode. If requested move to another mode. But alsways through IDLE
      this->_available = true;

      if (this->_opmode != ENS160_OPMODE_DEEP_SLEEP) {
        this->writeMode(ENS160_OPMODE_IDLE); // Move through Idle state
        newState = P164_STATE_IDLE;
      }
      break;
    case P164_STATE_OPERATIONAL:
      // Device is in one of the operational modes
      this->_available = true;

      switch (this->_opmode) {
        case ENS160_OPMODE_DEEP_SLEEP:
        case P164_STATE_IDLE:
        case ENS160_OPMODE_RESET:
          this->writeMode(ENS160_OPMODE_IDLE); // Move through Idle state
          newState = P164_STATE_IDLE;
          break;
      }
      break;
    default:
      // Unplanned state, force into error state
      newState = P164_STATE_ERROR;
      break;
  }

  if (newState != this->_state) {
    #ifdef P164_ENS160_DEBUG
      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        String log = F("P164: State transition ");
        log += printState(this->_state);
        log += F(" -> ");
        log += printState(newState);
        log += F("; opmode= ");
        log += this->_opmode;
        addLogMove(LOG_LEVEL_DEBUG, log);
      }
    #endif // ifdef P164_ENS160_DEBUG
    this->_state      = newState;
    this->_lastChange = millis();
  }

  return success;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Helper function to enter a new state                                                          //
// Function must be called outside evaluateState() when an action causes a state transition      //
///////////////////////////////////////////////////////////////////////////////////////////////////
void P164_data_struct::moveToState(P164_state newState)
{
  #ifdef P164_ENS160_DEBUG
    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log;
      log += F("P164: Move to state: ");
      log += printState(this->_state);
      log += F(" --> ");
      log += printState(newState);
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
  #endif // ifdef P164_ENS160_DEBUG

  this->_state      = newState; // Enter the new state
  this->_lastChange = millis(); // Mark time of transition
  this->evaluateState();        // Check if we can already move on
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Init the device:                                                                              //
// Reset ENS16x                                                                                  //
// Returns false on encountered errors                                                           //
///////////////////////////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::start(uint8_t slaveaddr)
{
  uint8_t result = 0;

  // Initialize internal bookkeeping;
  this->_state      = P164_STATE_INITIAL;   // Assume nothing, start clean
  this->_lastChange = millis();             // Bookmark last state change as now
  this->_available  = false;
  this->_opmode     = ENS160_OPMODE_STD;
  this->i2cAddress  = slaveaddr;

  result = this->writeMode(ENS160_OPMODE_RESET);  // Reset the device, takes some time
  this->moveToState(P164_STATE_RESETTING);        // Go to next state RESETTING

  #ifdef P164_ENS160_DEBUG
    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log;
      log += F("P164: start() result= ");
      log += result ? F("ok") : F("nok");
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
  #endif
  return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Perform prediction measurement and store result in internal variables                         //
// Return: true if data is fresh (first reading of new data)                                     //
///////////////////////////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::measure() {
  bool    newData = false;      // True when new data is availabl at device
  bool    is_ok;                // Dump I2C transaction status

  if (this->_state == P164_STATE_OPERATIONAL)  {
    if (this->getStatus()) {              // Check status register if new data is aquired
      if (IS_NEWDAT(this->_statusReg)) {
        newData = true;
        _data_aqi  = I2C_read8_reg(i2cAddress, ENS160_REG_DATA_AQI, &is_ok);
        _data_tvoc = I2C_read16_LE_reg(i2cAddress, ENS160_REG_DATA_TVOC, &is_ok);
        _data_eco2 = I2C_read16_LE_reg(i2cAddress, ENS160_REG_DATA_ECO2, &is_ok);
        if (_revENS16x > 0) {             // AQI500 only available for ENS161
          _data_aqi500 = I2C_read16_LE_reg(i2cAddress, ENS160_REG_DATA_AQI_S, &is_ok);
        }
        else {
          _data_aqi500 = 0;
        }
      }
    }
    else {
      // Some issues with the device connectivity, move to error state
      this->moveToState(P164_STATE_ERROR);
    }
  }

  #ifdef P164_ENS160_DEBUG
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log;
      log += F("P164: measure() state= ");
      log += printState(this->_state);
      log += F(", aqi= ");
      log += _data_aqi;
      log += F(", tvoc= ");
      log += _data_tvoc;
      log += F(", eco2= ");
      log += _data_eco2;
      log += F(", AQI500= ");
      log += _data_aqi500;
      log += F(", newdata = ");
      log += newData;
      addLogMove(LOG_LEVEL_INFO, log);
    }
  #endif // ifdef P164_ENS160_DEBUG

  return newData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Following code is used to handle custom aquisition modes as defined by Sciosense Github code  //
// Note that custom modes are not documented in the official datasheet                           //
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef P164_USE_CUSTOMMODE

///////////////////////////////////////////////////////////////////////////////////////////////////
// Initialize definition of custom mode with <n> steps                                           //
// This feature is not documented in the datasheet, but code is provided by Sciosense            //
///////////////////////////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::initCustomMode(uint16_t stepNum) {
  bool result = false;

  if (stepNum > 0) {
    this->_stepCount = stepNum;
    result           = this->writeMode(ENS160_OPMODE_IDLE);
    result           = this->clearCommand();
    result           = I2C_write8_reg(i2cAddress, ENS160_REG_COMMAND, ENS160_COMMAND_SETSEQ);
  } 
  else {
    result = false;
  }

  return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Add a step to custom measurement profile with definition of duration                          //
// enabled data acquisition and temperature for each hotplate                                    //
// This feature is not documented in the datasheet, but code is provided by Sciosense            //
///////////////////////////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::addCustomStep(uint16_t time, bool measureHP0, bool measureHP1,
                                     bool measureHP2, bool measureHP3, uint16_t tempHP0,
                                     uint16_t tempHP1, uint16_t tempHP2, uint16_t tempHP3) {
  uint8_t seq_ack;
  uint8_t temp;
  bool    is_ok;

  #ifdef P164_ENS160_DEBUG
    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      addLogMove(LOG_LEVEL_DEBUG, concat(F("setCustomMode() write step "), this->_stepCount));
    }
  #endif // ifdef P164_ENS160_DEBUG

  // TODO check if delay is needed
  // delay(ENS160_BOOTING);                   // Wait to boot after reset

  temp = (uint8_t)(((time / 24) - 1) << 6);

  if (measureHP0) { temp = temp | 0x20; }
  if (measureHP1) { temp = temp | 0x10; }
  if (measureHP2) { temp = temp | 0x8; }
  if (measureHP3) { temp = temp | 0x4; }
  I2C_write8_reg(i2cAddress, ENS160_REG_GPR_WRITE_0, temp);

  temp = (uint8_t)(((time / 24) - 1) >> 2);
  I2C_write8_reg(i2cAddress, ENS160_REG_GPR_WRITE_1, temp);

  I2C_write8_reg(i2cAddress, ENS160_REG_GPR_WRITE_2, (uint8_t)(tempHP0 / 2));
  I2C_write8_reg(i2cAddress, ENS160_REG_GPR_WRITE_3, (uint8_t)(tempHP1 / 2));
  I2C_write8_reg(i2cAddress, ENS160_REG_GPR_WRITE_4, (uint8_t)(tempHP2 / 2));
  I2C_write8_reg(i2cAddress, ENS160_REG_GPR_WRITE_5, (uint8_t)(tempHP3 / 2));

  I2C_write8_reg(i2cAddress, ENS160_REG_GPR_WRITE_6, (uint8_t)(this->_stepCount - 1));

  if (this->_stepCount == 1) {
    I2C_write8_reg(i2cAddress, ENS160_REG_GPR_WRITE_7, 128);
  } else {
    I2C_write8_reg(i2cAddress, ENS160_REG_GPR_WRITE_7, 0);
  }

  // TODO check if delay is needed
  // delay(ENS160_BOOTING);
  seq_ack = I2C_read8_reg(i2cAddress, ENS160_REG_GPR_READ_7, &is_ok);


  // TODO check if delay is needed
  // delay(ENS160_BOOTING);                   // Wait to boot after reset

  if ((ENS160_SEQ_ACK_COMPLETE | this->_stepCount) != seq_ack) {
    this->_stepCount = this->_stepCount - 1;
    return false;
  } else {
    return true;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Perfrom raw measurement and store result in internal variables                                //
///////////////////////////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::measureRaw() {
  bool    newData = false;
  bool    is_ok = true;

  if (this->_state == P164_STATE_OPERATIONAL)  {
    this->getStatus();
    if (IS_NEWGPR(this->_statusReg)) {
      newData = true;

      // Read raw resistance values
      _hp0_rs = CONVERT_RS_RAW2OHMS_F(I2C_read16_LE_reg(i2cAddress, ENS160_REG_GPR_READ_0, &is_ok));
      _hp1_rs = CONVERT_RS_RAW2OHMS_F(I2C_read16_LE_reg(i2cAddress, ENS160_REG_GPR_READ_2, &is_ok));
      _hp2_rs = CONVERT_RS_RAW2OHMS_F(I2C_read16_LE_reg(i2cAddress, ENS160_REG_GPR_READ_4, &is_ok));
      _hp3_rs = CONVERT_RS_RAW2OHMS_F(I2C_read16_LE_reg(i2cAddress, ENS160_REG_GPR_READ_6, &is_ok));

      // Read baselines
      _hp0_bl = CONVERT_RS_RAW2OHMS_F(I2C_read16_LE_reg(i2cAddress, ENS160_REG_DATA_BL+0, &is_ok));
      _hp1_bl = CONVERT_RS_RAW2OHMS_F(I2C_read16_LE_reg(i2cAddress, ENS160_REG_DATA_BL+2, &is_ok));
      _hp2_bl = CONVERT_RS_RAW2OHMS_F(I2C_read16_LE_reg(i2cAddress, ENS160_REG_DATA_BL+4, &is_ok));
      _hp3_bl = CONVERT_RS_RAW2OHMS_F(I2C_read16_LE_reg(i2cAddress, ENS160_REG_DATA_BL+6, &is_ok));

      _misr = I2C_read8_reg(i2cAddress, ENS160_REG_DATA_MISR, &is_ok);
    }
  }

  return newData;
}
#endif // P164_USE_CUSTOMMODE

///////////////////////////////////////////////////////////////////////////////////////////////////
// Writes t (degC) and h (%rh) to ENV_DATA. Returns false on I2C problems.                       //
///////////////////////////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::set_envdata(float t, float h) {
  uint16_t t_data  = (uint16_t)((t + 273.15f) * 64.0f);
  uint16_t rh_data = (uint16_t)(h * 512.0f);

  return this->set_envdata210(t_data, rh_data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Writes t and h (in ENS210 format) to ENV_DATA. Returns false on I2C problems.                 //
///////////////////////////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::set_envdata210(uint16_t t, uint16_t h) {
  uint8_t trh_in[4];           // Buffer for I2C registers TEMP_IN + RH_IN

  // temp = (uint16_t)((t + 273.15f) * 64.0f);
  trh_in[0] = t & 0xff;        // TEMP_IN LSB
  trh_in[1] = (t >> 8) & 0xff; // TEMP_IN MSB

  // temp = (uint16_t)(h * 512.0f);
  trh_in[2] = h & 0xff;        // RH_IN LSB
  trh_in[3] = (h >> 8) & 0xff; // RH_IN MSB

  uint8_t result = I2C_writeBytes_reg(i2cAddress, ENS160_REG_TEMP_IN, trh_in, 4); 

  return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Read firmware revision                                                                        //
// Precondition: Device MODE is IDLE                                                             //
// Note: This is according to original Sciosense software and matches ENS161 datasheet           //
//       The ENS160 datasheet is unclear, suggesting GRP_READ0 and GRP_READ1 registers           //
///////////////////////////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::getFirmware() {
  bool result = false;  // Build return value for function
  bool is_ok;           // Temporary flag to track status
  unsigned long ts;     // Timestamp to limit polling time

  result = I2C_write8_reg(i2cAddress, ENS160_REG_COMMAND, ENS160_COMMAND_GET_APPVER);
  
  // Wait till GRP_READ registers are available.
  is_ok = result;
  ts = millis();
  while (is_ok) {                                     // Abuse is_ok to keep polling until:
    is_ok &= this->getStatus();                       // - Device is inaccessible
    is_ok &= (! IS_NEWGPR(this->_statusReg));         // - Firmware data is available
    is_ok &= (timePassedSince(ts) < ENS160_BOOTING);  // - Timeout occured
  }
  if (! IS_NEWGPR(this->_statusReg)) {                // Check if firmware could be read
    result = false;
    addLogMove(LOG_LEVEL_ERROR, F("P164: Could not read firmware version"));
  }

  if (result)  {
    this->_fw_ver_major = I2C_read8_reg(i2cAddress, ENS160_REG_GPR_READ_4, &is_ok);
    this->_fw_ver_minor = I2C_read8_reg(i2cAddress, ENS160_REG_GPR_READ_5, &is_ok);
    this->_fw_ver_build = I2C_read8_reg(i2cAddress, ENS160_REG_GPR_READ_6, &is_ok);
  }
  else {
    this->_fw_ver_major = 0;
    this->_fw_ver_minor = 0;
    this->_fw_ver_build = 0;
  }

  #ifdef P164_ENS160_DEBUG
    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log;
      log += F("P164: getFirmware() result: ");
      log += result ? F("ok") : F("nok");
      log += F(", major=");
      log += this->_fw_ver_major;
      log += F(", minor=");
      log += this->_fw_ver_minor;
      log += F(", build=");
      log += this->_fw_ver_build;
      log += F(", registers=");
      for (int i=0; i<8; i++)
      {
        log += formatToHex_decimal(I2C_read8_reg(i2cAddress, ENS160_REG_GPR_READ_0+i, &is_ok));
        log += ", ";
      }
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
  #endif // ifdef P164_ENS160_DEBUG

  return result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Write to opmode register of the device                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::writeMode(uint8_t mode) {
  bool result;

  result = I2C_write8_reg(i2cAddress, ENS160_REG_OPMODE, mode);

    #ifdef P164_ENS160_DEBUG
      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        String log;
        log += F("P164: writeMode(");
        log += mode;
        log += F(") result: ");
        log += result ? F("ok") : F("nok");
        addLogMove(LOG_LEVEL_DEBUG, log);
      }
    #endif // ifdef P164_ENS160_DEBUG

  return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Read the part ID from ENS160 device and check for validity                                    //
///////////////////////////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::checkPartID(void) {
  uint16_t part_id;   // Resulting PartID
  bool     result = false;

  part_id = I2C_read16_LE_reg(i2cAddress, ENS160_REG_PART_ID, &result);

  if (!result) {
    this->_revENS16x = 0xFF;
    result           = false;
  }
  else if (part_id == ENS160_PARTID) {
    this->_revENS16x = 0;
    result           = true;
  }
  else if (part_id == ENS161_PARTID) {
    this->_revENS16x = 1;
    result           = true;
  }
  else {
    this->_revENS16x = 0xFF;
    result           = false;
  }

  #ifdef P164_ENS160_DEBUG
    String log;
    log += F("P164: checkPartID() result: ");
    switch (part_id) {
      case ENS160_PARTID: log += F("ENS160"); break;
      case ENS161_PARTID: log += F("ENS161"); break;
      default:            log += F("no valid part ID read"); break;
    }
    addLogMove(LOG_LEVEL_DEBUG, log);
  #endif // ifdef P164_ENS160_DEBUG

  return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Clear any pending command in device                                                           //
///////////////////////////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::clearCommand(void) {
  bool result = false;

  result = I2C_write8_reg(i2cAddress, ENS160_REG_COMMAND, ENS160_COMMAND_NOP);
  result = I2C_write8_reg(i2cAddress, ENS160_REG_COMMAND, ENS160_COMMAND_CLRGPR);
  #ifdef P164_ENS160_DEBUG
    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      addLogMove(LOG_LEVEL_DEBUG, concat(F("P164: clearCommand() result: "), result ? "ok" : "nok"));
    }
  #endif // ifdef P164_ENS160_DEBUG

  return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Read status register from device                                                              //
///////////////////////////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::getStatus()
{
  bool ret = false;

  this->_statusReg = I2C_read8_reg(i2cAddress, ENS160_REG_DATA_STATUS, &ret);
  #ifdef P164_ENS160_DEBUG
    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log;
      log += F("P164: Status register: ");
      log += formatToHex_decimal(this->_statusReg, HEX);
      log += F(", VALIDITY: ");
      log += GET_STATUS_VALIDITY(this->_statusReg);
      log += F(", STATAS: ");
      log += (this->_statusReg & ENS160_STATUS_STATAS) == ENS160_STATUS_STATAS;
      log += F(", STATER: ");
      log += (this->_statusReg & ENS160_STATUS_STATER) == ENS160_STATUS_STATER;
      log += F(", NEWDAT: ");
      log += (this->_statusReg & ENS160_STATUS_NEWDAT) == ENS160_STATUS_NEWDAT;
      log += F(", NEWGRP: ");
      log += (this->_statusReg & ENS160_STATUS_NEWGPR) == ENS160_STATUS_NEWGPR;
      log += F(", return: ");
      log += ret;
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
  #endif // ifdef P164_ENS160_DEBUG
  return ret;
}

#ifdef P164_LEGACY_CODE
///////////////////////////////////////////////////////////////////////////////////////////////////
// I2C functionality copied from Sciosense.                                                      //
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// Read a consecutive range of registers from device                                             //
// Return: boolean == true when I2C transaction was succesful                                    //
// Note: This functionality is not found in ESPEasy I2C library                                  //
///////////////////////////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::readI2C(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t num) {
  uint8_t pos    = 0;
  uint8_t result = 0;

  #ifdef P164_ENS160_DEBUG
    String log;   // Debug String concatenated in multiple sections. Keep outside local if statement
    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      log += F("P164: I2C read address: ");
      log += formatToHex_decimal(addr);
      log += F(", register: ");
      log += formatToHex_decimal(reg);
      log += F(" data:");
    }
  #endif // ifdef P164_ENS160_DEBUG

  // on arduino we need to read in 32 byte chunks
  while (pos < num) {
    uint8_t read_now = min((uint8_t)32, (uint8_t)(num - pos));
    Wire.beginTransmission((uint8_t)addr);

    Wire.write((uint8_t)reg + pos);
    result = Wire.endTransmission();
    Wire.requestFrom((uint8_t)addr, read_now);

    for (int i = 0; i < read_now; i++) {
      buf[pos] = Wire.read();
      #ifdef P164_ENS160_DEBUG
        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          log += F(" ");
          log += formatToHex_decimal(buf[pos]);
        }
      #endif // ifdef P164_ENS160_DEBUG
      pos++;
    }
  }
  #ifdef P164_ENS160_DEBUG
    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      log += F(".");
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
  #endif // ifdef P164_ENS160_DEBUG

  return result == 0;
}
#endif // P164_LEGACY_CODE

#endif // ifdef USES_P164
