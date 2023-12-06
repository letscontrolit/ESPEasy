#include "../PluginStructs/P164_data_struct.h"

#ifdef USES_P164
#include "../Pluginstructs/P164_ENS160.h"
//#include "math.h"

#define  P164_ENS160_DEBUG

// Use a state machine to avoid blocking the CPU while waiting for the response
#define ENS160_STATE_INITIAL        0 // Device is in an unknown state, typically after reset
#define ENS160_STATE_ERROR          1 // Device is in an error state
#define ENS160_STATE_RESETTING      2 // Waiting for response after reset
#define ENS160_STATE_IDLE           3 // Device is brought into IDLE mode
#define ENS160_STATE_DEEPSLEEP      4 // Device is brought into DEEPSLEEP mode
#define ENS160_STATE_OPERATIONAL    5 // Device is brought into OPERATIONAL mode


///////////////////////////////////////////////////////////////////////////////
// Constructor                                                               //
///////////////////////////////////////////////////////////////////////////////
P164_data_struct::P164_data_struct(struct EventStruct *event) :
  i2cAddress(P164_I2C_ADDR)
{
  Serial.println(F("ENS160: Constructor"));
  Serial.print("ENS160: I2C address =");
  Serial.println(i2cAddress, HEX);
}

///////////////////////////////////////////////////////////////////////////////
// Initialization of the connected device                                    //
///////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::begin()
{
  if (!start(i2cAddress)) {
    Serial.println(F("ENS160: P164_data_struct::begin() ***FAILED***"));
    return false;
  }
  setMode(ENS160_OPMODE_STD);
  initialized = true;
  Serial.println(F("ENS160: P164_data_struct::begin() success"));
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Fetch the processed device values as stored in the software object        //
///////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::read(float& tvoc, float& eco2)
{
  if (measure()) {
    return false;
  }

  tvoc = getTVOC();
  eco2 = geteCO2();

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Implementation of plugin PLUGIN_WEBFORM_LOAD call                         //
// Note: this is not a class function, only data from event is available     //
///////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::webformLoad(struct EventStruct *event)
{
  bool  found = false; // A chip has responded at the I2C address
  uint8 reg0, reg1;
  uint16_t chipID = 0;

  addRowLabel(F("Detected Sensor Type"));

  chipID = I2C_read16_LE_reg(P164_I2C_ADDR, ENS160_REG_PART_ID, &found);

  if (!found) {
    addHtml(F("No device found"));
  } else if (chipID == ENS160_PARTID) {
    addHtml(F("ENS160"));
  } else if (chipID == ENS161_PARTID) {
    addHtml(F("ENS161"));
  } else {
    addHtmlInt(chipID);
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
/// Implementation of plugin PLUGIN_WEBFORM_SAVE call                        //
// Note: this is not a class function, only data from event is available     //
///////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::webformSave(struct EventStruct *event)
{
  P164_I2C_ADDR = getFormItemInt(F("i2c_addr"));
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Implementation of plugin PLUGIN_TEN_PER_SECOND call                       //
///////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::tenPerSecond(struct EventStruct *event)
{
  return evaluateState();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//                     **** The sensor handling code ****                    //
// This is based upon XXXXX code on github                                   //
// The code is heavily adapted to fit the ESPEasy structures                 //
///////////////////////////////////////////////////////////////////////////////

/*
   For documentation see https://www.sciosense.com/wp-content/uploads/documents/SC-001224-DS-9-ENS160-Datasheet.pdf
   based on application note "ENS160 Software Integration.pdf" rev 0.01
   original code from Github TODO:
 */

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void printState(int state);

///////////////////////////////////////////////////////
// Evaluate the statemachine and determine next step //
///////////////////////////////////////////////////////
bool P164_data_struct::evaluateState()
{
  bool success  = true;
  int  newState = this->_state; // Determine next state

  switch (this->_state) {
    case ENS160_STATE_INITIAL:
      // Waiting for external call to begin()
      this->_available = false;
      success          = true;
      break;
    case ENS160_STATE_ERROR:
      // Stay here until external retry attempt
      this->_available = false;
      success          = true;
      break;
    case ENS160_STATE_RESETTING:
      this->_available = false;

      // Once device has rebooted read some stuff from it and move to idle
      if (timePassedSince(this->_lastChange) > ENS160_BOOTING) { // Wait until device has rebooted
        this->checkPartID();
        this->writeMode(ENS160_OPMODE_IDLE);
        this->clearCommand();
        this->getFirmware();
        this->getStatus();
        newState = ENS160_STATE_IDLE;
      }
      break;
    case ENS160_STATE_IDLE:
      // Set device into desired operation mode as requested through _opmode
      this->_available = true;

      switch (this->_opmode) {
        case ENS160_OPMODE_STD:
          this->writeMode(ENS160_OPMODE_STD);
          newState = ENS160_STATE_OPERATIONAL;
          break;
        case ENS160_OPMODE_LP:
          this->writeMode(ENS160_OPMODE_LP);
          newState = ENS160_STATE_OPERATIONAL;
          break;
        case ENS160_OPMODE_ULP:
          this->writeMode(ENS160_OPMODE_ULP);
          newState = ENS160_STATE_OPERATIONAL;
          break;
        case ENS160_OPMODE_RESET:
          this->writeMode(ENS160_OPMODE_RESET);
          this->_opmode = ENS160_OPMODE_IDLE; // Prevent reset loop
          newState      = ENS160_STATE_RESETTING;
          break;
      }
      break;
    case ENS160_STATE_DEEPSLEEP:
      // Device is put to DEEPSLEEP mode. If requested move to another mode. But alsways through IDLE
      this->_available = true;

      if (this->_opmode != ENS160_OPMODE_DEEP_SLEEP) {
        this->writeMode(ENS160_OPMODE_IDLE); // Move through Idle state
        newState = ENS160_STATE_IDLE;
      }
      break;
    case ENS160_STATE_OPERATIONAL:
      // Device is in one of the operational modes
      this->_available = true;

      switch (this->_opmode) {
        case ENS160_OPMODE_DEEP_SLEEP:
        case ENS160_STATE_IDLE:
        case ENS160_OPMODE_RESET:
          this->writeMode(ENS160_OPMODE_IDLE); // Move through Idle state
          newState = ENS160_STATE_IDLE;
          break;
      }
      break;
    default:
      // Unplanned state, force into error state
      newState = ENS160_STATE_ERROR;
      break;
  }

  if (newState != this->_state) {
    this->_state      = newState;
    this->_lastChange = millis();
        #ifdef P164_ENS160_DEBUG
    Serial.print(F("ENS16x: State transition:"));
    printState(newState);
    Serial.print(F("; opmode "));
    Serial.print(this->_opmode);
    Serial.println(F("."));
        #endif // ifdef P164_ENS160_DEBUG
  }
  else {
     #ifdef P164_ENS160_DEBUG
     //   Serial.print(F("ENS16x: state "));
     //   printState(newState);
     //   Serial.print(F(" opmode "));
     //   Serial.println(this->_opmode);
     #endif
  }

  return success;
}

///////////////////////////////////////////////////////////////////
// Init I2C communication, resets ENS160 and checks its PART_ID. //
// Returns false on I2C problems or wrong PART_ID.               //
///////////////////////////////////////////////////////////////////
bool P164_data_struct::start(uint8_t slaveaddr)
{
  uint8_t result = 0;

  // Initialize internal bookkeeping;
  this->_state      = ENS160_STATE_INITIAL; // Assume nothing, start clean
  this->_lastChange = millis();             // Bookmark last state change as now
  this->_available  = false;
  this->_opmode     = ENS160_OPMODE_STD;

  // Set IO pin levels
  if (this->_ADDR > 0) {
    pinMode(this->_ADDR, OUTPUT);   // ADDR is input pin for device
    digitalWrite(this->_ADDR, LOW); // Set it identify ENS160_I2CADDR_0
  }

  if (this->_nINT > 0) {
    pinMode(this->_nINT, INPUT_PULLUP); // INT is open drain output pin for the device
  }

  if (this->_nCS > 0) {
    pinMode(this->_nCS, OUTPUT);    // CS is input for the device
    digitalWrite(this->_nCS, HIGH); // Must be HIGH for I2C operation
  }

  result = this->writeMode(ENS160_OPMODE_RESET);

    #ifdef P164_ENS160_DEBUG
  Serial.print(F("ENS16x: reset() result: "));
  Serial.println(result == 0 ? F("ok") : F("nok"));
    #endif // ifdef P164_ENS160_DEBUG
  this->moveToState(ENS160_STATE_RESETTING); // Go to next state RESETTING

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Initialize definition of custom mode with <n> steps                       //
// TODO: Undocumented feature, to be reworked for ESPEasy version            //
///////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::initCustomMode(uint16_t stepNum) {
  uint8_t result;

  if (stepNum > 0) {
    this->_stepCount = stepNum;
    result           = this->writeMode(ENS160_OPMODE_IDLE);
    result           = this->clearCommand();
    result           = P164_data_struct::write8(i2cAddress, ENS160_REG_COMMAND, ENS160_COMMAND_SETSEQ);
  } else {
    result = 1;
  }

  // TODO check if delay is needed
  // delay(ENS160_BOOTING);                   // Wait to boot after reset

  return result == 0;
}

///////////////////////////////////////////////////////////////////////////////
// Add a step to custom measurement profile with definition of duration      //
// enabled data acquisition and temperature for each hotplate                //
// TODO: Undocumented feature, to be reworked for ESPEasy version            //
///////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::addCustomStep(uint16_t time, bool measureHP0, bool measureHP1,
                                     bool measureHP2, bool measureHP3, uint16_t tempHP0,
                                     uint16_t tempHP1, uint16_t tempHP2, uint16_t tempHP3) {
  uint8_t seq_ack;
  uint8_t temp;

    #ifdef P164_ENS160_DEBUG
  Serial.print("setCustomMode() write step ");
  Serial.println(this->_stepCount);
    #endif // ifdef P164_ENS160_DEBUG

  // TODO check if delay is needed
  // delay(ENS160_BOOTING);                   // Wait to boot after reset

  temp = (uint8_t)(((time / 24) - 1) << 6);

  if (measureHP0) { temp = temp | 0x20; }

  if (measureHP1) { temp = temp | 0x10; }

  if (measureHP2) { temp = temp | 0x8; }

  if (measureHP3) { temp = temp | 0x4; }
  P164_data_struct::write8(i2cAddress, ENS160_REG_GPR_WRITE_0, temp);

  temp = (uint8_t)(((time / 24) - 1) >> 2);
  P164_data_struct::write8(i2cAddress, ENS160_REG_GPR_WRITE_1, temp);

  P164_data_struct::write8(i2cAddress, ENS160_REG_GPR_WRITE_2, (uint8_t)(tempHP0 / 2));
  P164_data_struct::write8(i2cAddress, ENS160_REG_GPR_WRITE_3, (uint8_t)(tempHP1 / 2));
  P164_data_struct::write8(i2cAddress, ENS160_REG_GPR_WRITE_4, (uint8_t)(tempHP2 / 2));
  P164_data_struct::write8(i2cAddress, ENS160_REG_GPR_WRITE_5, (uint8_t)(tempHP3 / 2));

  P164_data_struct::write8(i2cAddress, ENS160_REG_GPR_WRITE_6, (uint8_t)(this->_stepCount - 1));

  if (this->_stepCount == 1) {
    P164_data_struct::write8(i2cAddress, ENS160_REG_GPR_WRITE_7, 128);
  } else {
    P164_data_struct::write8(i2cAddress, ENS160_REG_GPR_WRITE_7, 0);
  }

  // TODO check if delay is needed
  // delay(ENS160_BOOTING);

  seq_ack = P164_data_struct::read8(i2cAddress, ENS160_REG_GPR_READ_7);

  // TODO check if delay is needed
  // delay(ENS160_BOOTING);                   // Wait to boot after reset

  if ((ENS160_SEQ_ACK_COMPLETE | this->_stepCount) != seq_ack) {
    this->_stepCount = this->_stepCount - 1;
    return 0;
  } else {
    return 1;
  }
}

///////////////////////////////////////////////////////////////////////////
// Perform prediction measurement and store result in internal variables //
// Return: true if data is fresh (first reading of new data)             //
///////////////////////////////////////////////////////////////////////////
bool P164_data_struct::measure() {
  uint8_t i2cbuf[8];
  uint8_t status;
  bool    newData = false;

    #ifdef P164_ENS160_DEBUG
  Serial.println(F("ENS16x: Start measurement"));
    #endif // ifdef P164_ENS160_DEBUG

  if (this->_state == ENS160_STATE_OPERATIONAL)  {
    // Check if new data is aquired
    status = P164_data_struct::read8(i2cAddress, ENS160_REG_DATA_STATUS);

    // Read predictions
    if (IS_NEWDAT(status)) {
      newData = true;
      P164_data_struct::read(i2cAddress, ENS160_REG_DATA_AQI, i2cbuf, 7);
      _data_aqi  = i2cbuf[0];
      _data_tvoc = i2cbuf[1] | ((uint16_t)i2cbuf[2] << 8);
      _data_eco2 = i2cbuf[3] | ((uint16_t)i2cbuf[4] << 8);

      if (_revENS16x > 0) {
        _data_aqi500 = ((uint16_t)i2cbuf[5]) | ((uint16_t)i2cbuf[6] << 8);
      }
      else {
        _data_aqi500 = 0;
      }
    }
  }

    #ifdef P164_ENS160_DEBUG
  Serial.print(F("ENS16x: measure: aqi= "));
  Serial.print(_data_aqi);
  Serial.print(F(" tvoc= "));
  Serial.print(_data_tvoc);
  Serial.print(F(" eco2= "));
  Serial.print(_data_eco2);
  Serial.println(F("."));
    #endif // ifdef P164_ENS160_DEBUG

  return newData;
}

////////////////////////////////////////////////////////////////////
// Perfrom raw measurement and store result in internal variables //
////////////////////////////////////////////////////////////////////
bool P164_data_struct::measureRaw() {
  uint8_t i2cbuf[8];
  uint8_t status;
  bool    newData = false;

  // Set default status for early bail out
    #ifdef P164_ENS160_DEBUG
  Serial.println("ENS16x: Start measurement");
    #endif // ifdef P164_ENS160_DEBUG

  if (this->_state == ENS160_STATE_OPERATIONAL)  {
    status = P164_data_struct::read8(i2cAddress, ENS160_REG_DATA_STATUS);

    if (IS_NEWGPR(status)) {
      newData = true;

      // Read raw resistance values
      P164_data_struct::read(i2cAddress, ENS160_REG_GPR_READ_0, i2cbuf, 8);
      _hp0_rs = CONVERT_RS_RAW2OHMS_F((uint32_t)(i2cbuf[0] | ((uint16_t)i2cbuf[1] << 8)));
      _hp1_rs = CONVERT_RS_RAW2OHMS_F((uint32_t)(i2cbuf[2] | ((uint16_t)i2cbuf[3] << 8)));
      _hp2_rs = CONVERT_RS_RAW2OHMS_F((uint32_t)(i2cbuf[4] | ((uint16_t)i2cbuf[5] << 8)));
      _hp3_rs = CONVERT_RS_RAW2OHMS_F((uint32_t)(i2cbuf[6] | ((uint16_t)i2cbuf[7] << 8)));

      // Read baselines
      P164_data_struct::read(i2cAddress, ENS160_REG_DATA_BL, i2cbuf, 8);
      _hp0_bl = CONVERT_RS_RAW2OHMS_F((uint32_t)(i2cbuf[0] | ((uint16_t)i2cbuf[1] << 8)));
      _hp1_bl = CONVERT_RS_RAW2OHMS_F((uint32_t)(i2cbuf[2] | ((uint16_t)i2cbuf[3] << 8)));
      _hp2_bl = CONVERT_RS_RAW2OHMS_F((uint32_t)(i2cbuf[4] | ((uint16_t)i2cbuf[5] << 8)));
      _hp3_bl = CONVERT_RS_RAW2OHMS_F((uint32_t)(i2cbuf[6] | ((uint16_t)i2cbuf[7] << 8)));

      P164_data_struct::read(i2cAddress, ENS160_REG_DATA_MISR, i2cbuf, 1);
      _misr = i2cbuf[0];
    }
  }

  return newData;
}

///////////////////////////////////////////////////////////////////////////////
// Writes t (degC) and h (%rh) to ENV_DATA. Returns false on I2C problems.   //
///////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::set_envdata(float t, float h) {
  uint16_t t_data  = (uint16_t)((t + 273.15f) * 64.0f);
  uint16_t rh_data = (uint16_t)(h * 512.0f);

  return this->set_envdata210(t_data, rh_data);
}

///////////////////////////////////////////////////////////////////////////////
// Writes t and h (in ENS210 format) to ENV_DATA. Returns false on I2C problems.
///////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::set_envdata210(uint16_t t, uint16_t h) {
  uint8_t trh_in[4];           // Buffer for I2C registers TEMP_IN + RH_IN

  // temp = (uint16_t)((t + 273.15f) * 64.0f);
  trh_in[0] = t & 0xff;        // TEMP_IN LSB
  trh_in[1] = (t >> 8) & 0xff; // TEMP_IN MSB

  // temp = (uint16_t)(h * 512.0f);
  trh_in[2] = h & 0xff;        // RH_IN LSB
  trh_in[3] = (h >> 8) & 0xff; // RH_IN MSB

  uint8_t result = P164_data_struct::write(i2cAddress, ENS160_REG_TEMP_IN, trh_in, 4);

  return result;
}

///////////////////////////////////////////////////////////////////////////////
// Helper function to enter a new state                                      //
///////////////////////////////////////////////////////////////////////////////
void P164_data_struct::moveToState(int newState)
{
  this->_state      = newState; // Enter the new state
  this->_lastChange = millis(); // Mark time of transition
  this->evaluateState();        // Check if we can already move on
}

///////////////////////////////////////////////////////////////////////////////
// Read firmware revision                                                    //
// Precondition: Device MODE is IDLE                                         //
///////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::getFirmware() {
  uint8_t i2cbuf[3];
  uint8_t result;

  result = P164_data_struct::write8(i2cAddress, ENS160_REG_COMMAND, ENS160_COMMAND_GET_APPVER);

  if (result == 0) {
    result = P164_data_struct::read(i2cAddress, ENS160_REG_GPR_READ_4, i2cbuf, 3);
  }

  if (result == 0)  {
    this->_fw_ver_major = i2cbuf[0];
    this->_fw_ver_minor = i2cbuf[1];
    this->_fw_ver_build = i2cbuf[2];
  }
  else {
    this->_fw_ver_major = 0;
    this->_fw_ver_minor = 0;
    this->_fw_ver_build = 0;
  }

    #ifdef P164_ENS160_DEBUG
  Serial.print(F("ENS16x: getFirmware() result: "));
  Serial.print(result == 0 ? "ok," : "nok,");
  Serial.print(this->_fw_ver_major);
  Serial.print(F(","));
  Serial.println(this->_fw_ver_minor);
  Serial.print(F(","));
  Serial.println(this->_fw_ver_build);
    #endif // ifdef P164_ENS160_DEBUG

  return result == 0;
}

//////////////////////////////////
// Set operation mode of sensor //
//////////////////////////////////
bool P164_data_struct::setMode(uint8_t mode) {
  bool result = false;

  // LP only valid for rev>0
  if (!(mode == ENS160_OPMODE_LP) and (_revENS16x == 0)) {
    this->_opmode = mode;
    result        = true;
  }

    #ifdef P164_ENS160_DEBUG
  Serial.print(F("ENS16x: setMode("));
  Serial.print(mode);
  Serial.println(F(")"));
    #endif // ifdef P164_ENS160_DEBUG

  return result;
}

///////////////////////////////////////////////////////////////////////////////
// Write to opmode register of the device                                    //
///////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::writeMode(uint8_t mode) {
  uint8_t result;

  result = P164_data_struct::write8(i2cAddress, ENS160_REG_OPMODE, mode);

    #ifdef P164_ENS160_DEBUG
  Serial.print(F("ENS16x: writeMode() activate result: "));
  Serial.println(result == 0 ? F("ok") : F("nok"));
    #endif // ifdef P164_ENS160_DEBUG

  return result == 0;
}

///////////////////////////////////////////////////////////////////////////////
// Read the part ID from ENS160 device and check for validity                //
///////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::checkPartID(void) {
  uint8_t  i2cbuf[2]; // Buffer for returned I2C data
  uint16_t part_id;   // Resulting PartID
  bool     result = false;

  P164_data_struct::read(i2cAddress, ENS160_REG_PART_ID, i2cbuf, 2);
  part_id = i2cbuf[0] | ((uint16_t)i2cbuf[1] << 8);

  if (part_id == ENS160_PARTID) {
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
  Serial.print(F("ENS16x: checkPartID() result: "));

  if (part_id == ENS160_PARTID) { Serial.println(F("ENS160 ok")); }
  else if (part_id == ENS161_PARTID) { Serial.println(F("ENS161 ok")); }
  else { Serial.println(F("nok")); }
    #endif // ifdef P164_ENS160_DEBUG

  return result;
}

///////////////////////////////////////////////////////////////////////////////
// Clear any pending command in device                                       //
///////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::clearCommand(void) {
  uint8_t status;
  uint8_t result;

  result = P164_data_struct::write8(i2cAddress, ENS160_REG_COMMAND, ENS160_COMMAND_NOP);
  result = P164_data_struct::write8(i2cAddress, ENS160_REG_COMMAND, ENS160_COMMAND_CLRGPR);
    #ifdef P164_ENS160_DEBUG
  Serial.print(F("ENS16x: clearCommand() result: "));
  Serial.println(result == 0 ? "ok" : "nok");
    #endif // ifdef P164_ENS160_DEBUG

  return result == 0;
}

///////////////////////////////////////////////////////////////////////////////
// Read status register from device                                          //
///////////////////////////////////////////////////////////////////////////////
bool P164_data_struct::getStatus()
{
  this->_statusReg = P164_data_struct::read8(i2cAddress, ENS160_REG_DATA_STATUS);
    #ifdef P164_ENS160_DEBUG
  Serial.print(F("ENS16x: Status register: 0x"));
  Serial.println(this->_statusReg, HEX);
    #endif // ifdef P164_ENS160_DEBUG
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Helper function to display the current state in readable format           //
///////////////////////////////////////////////////////////////////////////////
void printState(int state) {
    #ifdef P164_ENS160_DEBUG

  switch (state) {
    case ENS160_STATE_INITIAL:      Serial.print(F("initial")); break;
    case ENS160_STATE_ERROR:        Serial.print(F("error")); break;
    case ENS160_STATE_RESETTING:    Serial.print(F("resetting")); break;
    case ENS160_STATE_IDLE:         Serial.print(F("idle")); break;
    case ENS160_STATE_DEEPSLEEP:    Serial.print(F("deepsleep")); break;
    case ENS160_STATE_OPERATIONAL:  Serial.print(F("operational")); break;
    default:                        Serial.print(F("***ERROR***")); break;
  }
    #endif // ifdef P164_ENS160_DEBUG
}

///////////////////////////////////////////////////////////////////////////////
// TODO: Refactor I2C to use the ESPEasy standard                            //
///////////////////////////////////////////////////////////////////////////////

/**************************************************************************/

uint8_t P164_data_struct::read8(uint8_t addr, byte reg) {
  uint8_t ret;

  (void)P164_data_struct::read(addr, reg, &ret, 1);

  return ret;
}

bool P164_data_struct::read(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t num) {
  uint8_t pos    = 0;
  bool    result = true;

  #ifdef P164_ENS160_DEBUG
    Serial.print(F("ENS16x: I2C read address: 0x"));
    Serial.print(addr, HEX);
    Serial.print(F(", register: 0x"));
    Serial.print(reg,  HEX);
    Serial.print(F(" data:"));
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
      Serial.print(" 0x");
      Serial.print(buf[pos], HEX);
            #endif // ifdef P164_ENS160_DEBUG
      pos++;
    }
  }
    #ifdef P164_ENS160_DEBUG
      Serial.println(".");
    #endif // ifdef P164_ENS160_DEBUG

  return result;
}

/**************************************************************************/

bool P164_data_struct::write8(uint8_t addr, byte reg, byte value) {
  return P164_data_struct::write(addr, reg, &value, 1);
}

bool P164_data_struct::write(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t num) {
  uint8_t result;

  #ifdef P164_ENS160_DEBUG
    Serial.print(F("ENS16x: I2C write address: 0x"));
    Serial.print(addr, HEX);
    Serial.print(F(", register: 0x"));
    Serial.print(reg,  HEX);
    Serial.print(F(",  value:"));

    for (int i = 0; i < num; i++) {
      Serial.print(F(" 0x"));
      Serial.print(buf[i], HEX);
    }
  Serial.println();
  #endif // ifdef P164_ENS160_DEBUG

  Wire.beginTransmission((uint8_t)addr);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t *)buf, num);
  result = Wire.endTransmission();
  return result;
}

/**************************************************************************/

#endif // ifdef USES_P164
