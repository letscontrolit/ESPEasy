#include "../PluginStructs/P014_data_struct.h"

#ifdef USES_P014


P014_data_struct::P014_data_struct() {
  state         = P014_state::Uninitialized; // Force device setup next time
  last_measurement_time = 0;
}


bool P014_data_struct::startInit(uint8_t i2caddr)
{
  state = P014_state::Uninitialized;

  uint8_t ret = I2C_wakeup(i2caddr);
  if (ret){
    if (loglevelActiveFor(LOG_LEVEL_ERROR)){
      String log = F("SI70xx : Not available at address: ");
      log += String(i2caddr, HEX);
      addLog(LOG_LEVEL_ERROR, log);
    }
    return false;
  }

  //reset the device
  if (!softReset(i2caddr)){
    if (loglevelActiveFor(LOG_LEVEL_ERROR)){
      String log = F("SI70xx : Not able to reset: ");
      log += String(i2caddr, HEX);
      addLog(LOG_LEVEL_ERROR, log);
    }
    return false;
  }

  return true;
}


bool P014_data_struct::finalizeInit(uint8_t i2caddr, uint8_t resolution)
{
  //read the device ID and revision
  //SHT devices do not have this capability so we are continuing if not available 
  int8_t ret = readSerialNumber(i2caddr); //this will also set the device ID
  if (ret){
      if (loglevelActiveFor(LOG_LEVEL_ERROR)){
          String log = F("SI70xx : Not able to read SN: addr=0x");
          log += String(i2caddr, HEX);
          log += F(" err=");
          log += String(ret, DEC);
          addLog(LOG_LEVEL_ERROR, log);
      }
    //return false;
  }

  //at this point we know the chip_id
  if (loglevelActiveFor(LOG_LEVEL_INFO)){
    String log = F("P014: chip_id=");
    log += String(chip_id, DEC);
    addLog(LOG_LEVEL_INFO, log);
  }
  if (chip_id == CHIP_ID_SI7013){
    if (!I2C_write8_reg(i2caddr,SI7013_WRITE_REG2, SI7013_REG2_DEFAULT )){
      return false;
    }
    if (!enablePowerForADC(i2caddr)){
      return false;
    }
  }

  // Set the resolution we want
  if(!setResolution(i2caddr,resolution)){
    return false;
  }
  
  //SI7013 has ADC and we need to start the first measurement
  if (chip_id == CHIP_ID_SI7013){
    if (!requestADC(i2caddr))
      return false;
  }

  //we are now initialized
  return true;
}



//The main state machine
//Only perform the measurements with big interval to prevent the sensor from warming up.
//returns true when the state changes and false otherwise
bool P014_data_struct::update(uint8_t i2caddr, uint8_t resolution, uint8_t filter_power) {

  switch(state){
    case P014_state::Uninitialized:
        //make sure we do not try to initialize too often
        //someone could enable the plugin but does not have the hardware connected
        if (!timeOutReached(last_measurement_time + errCount*SI70xx_INIT_DELAY)) {
          return false;
        }
        last_measurement_time = millis();
        //we have to stop trying after a while
        if (errCount>P014_MAX_RETRY){
            state = P014_state::Error;
            return true;
        }

        if (I2C_wakeup(i2caddr)!=0){
          if (loglevelActiveFor(LOG_LEVEL_ERROR)){
            String log = F("SI70xx : Not available at address: ");
            log += String(i2caddr, HEX);
            addLog(LOG_LEVEL_ERROR, log);
          }
          errCount++;
          return false;
        }

        if (startInit(i2caddr)){
          state                 = P014_state::Wait_for_reset;
          return true;
        }

        errCount++;
        return false;
    //break;

    case P014_state::Wait_for_reset:
      //we need to wait for the chip to reset
      if (!timeOutReached(last_measurement_time + SI70xx_RESET_DELAY)) {
        return false;
      }

      if (!finalizeInit(i2caddr,resolution)){
        state = P014_state::Uninitialized;
        errCount++;
        return true;
      }

      errCount = 0;
      state = P014_state::Initialized;
      return true;
    //break;      

    case P014_state::Initialized:
      if (chip_id == CHIP_ID_SI7013){
        //we need to wait for the read of ADC
        if (!timeOutReached(last_measurement_time + SI70xx_MEASUREMENT_DELAY)) {
          return false;
        }

        if ( !readADC(i2caddr,filter_power)){
          return false;
        }
        
        adc = adc << filter_power; //this is the first measurement
      } 
      last_measurement_time = millis();
      state = P014_state::Ready;
      return true;
    //break;

    case P014_state::Ready:
        if (!I2C_write8(i2caddr,SI70xx_CMD_MEASURE_HUM)) { //measure humidity and temperature at the same time
          addLog(LOG_LEVEL_ERROR,F("SI70xx: startConv Failed!"));
          return false;
        }
      
        last_measurement_time = millis();
        state = P014_state::Wait_for_humidity_samples;
      return true;
    //break;

    case P014_state::Wait_for_humidity_samples:
        //make sure we wait for the measurement to complete
        if (!timeOutReached(last_measurement_time + SI70xx_MEASUREMENT_DELAY)) {
          return false;
        }

        if (!readHumidity(i2caddr, resolution)) {
          state = P014_state::Ready; //we go back to request the reading again
          return true;
        }
        
        last_measurement_time = millis();
  
        if (chip_id == CHIP_ID_HTU21D){
          if (!I2C_write8(i2caddr,SI70xx_CMD_MEASURE_TEMP)) { //HTU21D can't read temperature from humidity measurement
            addLog(LOG_LEVEL_ERROR,F("SI70xx: startConv Failed!"));
            return false;
          }
          state = P014_state::Wait_for_temperature_samples;
          return false;
        }

        if (!readTemperatureFromHumidity(i2caddr,resolution)){
          state = P014_state::Ready; //we go back to request the reading again
          return true;
        }

        if (chip_id == CHIP_ID_SI7013){
          if(!enablePowerForADC(i2caddr)){
            state = P014_state::Ready; //we go back to request the reading again
            return true;
          }
          state = P014_state::RequestADC;
        }else{
          state = P014_state::New_Values_Available;
        }
        return true;
    //break;

    case P014_state::Wait_for_temperature_samples:
        if (!timeOutReached(last_measurement_time + SI70xx_MEASUREMENT_DELAY)) {
          return false;
        }

        if (!readTemperature(i2caddr,resolution)){
          state = P014_state::Ready; //we go back to request the reading again
          return true;
        }

        state = P014_state::New_Values_Available;
        return true;

    case P014_state::RequestADC:
        //make sure we wait for the power to stabilize
        if (!timeOutReached(last_measurement_time + SI70xx_MEASUREMENT_DELAY)) {
          return false;
        }

        if (!requestADC(i2caddr)){
          state = P014_state::Ready; //we go back to request the reading again
          return true;
        }
        
        state = P014_state::Wait_for_adc_samples;
        return true;
    //break;

    case P014_state::Wait_for_adc_samples:
        if (!readADC(i2caddr, filter_power)){
          state = P014_state::RequestADC; //we go back to request the reading again
          return false;
        }
        state = P014_state::New_Values_Available;
        return true;
    //break;

    case P014_state::Error:
    case P014_state::New_Values_Available:
      //this state is used outside so all we need is to stay here
      return false;
    break;


    default: //anything not defined above sends the device for initialization
      state = P014_state::Uninitialized;
      return true;
  } 

  return false;
}


 

//--------------- Supporting Functions -------------------


/* ======================================================================
   Function: checkCRC
   Purpose : check the CRC of received data
   Input   : value read from sensor
   Output  : CRC read from sensor
   Comments: 0 if okay
   ====================================================================== */
uint8_t P014_data_struct::checkCRC(uint16_t data, uint8_t check)
{
  uint32_t remainder, divisor;

  // Pad with 8 bits because we have to add in the check value
  remainder = (uint32_t)data << 8;

  // From: http://www.nongnu.org/avr-libc/user-manual/group__util__crc.html
  // POLYNOMIAL = 0x0131 = x^8 + x^5 + x^4 + 1 : http://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks
  // 0x988000 is the 0x0131 polynomial shifted to farthest left of three bytes
  divisor = (uint32_t)0x988000;

  // Add the check value
  remainder |= check;

  // Operate on only 16 positions of max 24.
  // The remaining 8 are our remainder and should be zero when we're done.
  for (uint8_t i = 0; i < 16; i++) {
    // Check if there is a one in the left position
    if (remainder & (uint32_t)1 << (23 - i)) {
      remainder ^= divisor;
    }

    // Rotate the divisor max 16 times so that we have 8 bits left of a remainder
    divisor >>= 1;
  }
  return (uint8_t)remainder;
}





/* ======================================================================
Function: setResolution
Purpose : Sets the sensor resolution to one of four levels
Input   : see #define default is SI7013_RESOLUTION_14T_12RH
Output  : 0 if okay
Comments: -
====================================================================== */
bool P014_data_struct::setResolution(uint8_t i2caddr, uint8_t resolution)
{
  bool ok;
  uint8_t reg = I2C_read8_reg(i2caddr,SI70xx_CMD_READ_REG1,&ok);
  if (ok) {
    // remove resolution bits
    reg &= SI70xx_RESOLUTION_MASK ;
    // Write the new resolution bits but clear unused before
    ok = I2C_write8_reg(i2caddr,SI70xx_CMD_WRITE_REG1, reg | ( resolution &= ~SI70xx_RESOLUTION_MASK) );
  }

  if (!ok) {
      if (loglevelActiveFor(LOG_LEVEL_ERROR)){
        String log = F("SI70xx : Unable to set resolution: ");
        log += String(i2caddr, HEX);
        addLog(LOG_LEVEL_ERROR, log);
      }
  }

  return ok;
}


inline bool P014_data_struct::softReset(uint8_t i2caddr){
  // Prepare to write to the register value
    bool ret = I2C_write8(i2caddr, SI70xx_CMD_SOFT_RESET);
    return ret;
}


#ifndef LIMIT_BUILD_SIZE
int8_t P014_data_struct::readRevision(uint8_t i2caddr) {
  Wire.beginTransmission(i2caddr);
  Wire.write((uint8_t)(SI70xx_CMD_FIRMREV >> 8));
  Wire.write((uint8_t)(SI70xx_CMD_FIRMREV & 0xFF));
  Wire.endTransmission();

  uint32_t start = millis(); // start timeout
  
  while (!timeOutReached(start + SI70xx_MEASUREMENT_DELAY)) {
    if (Wire.requestFrom(i2caddr, 2u) == 2) {
      uint8_t rev = Wire.read();
      Wire.read(); //ignore CRC

      if (loglevelActiveFor(LOG_LEVEL_INFO)){
        String log = F("SI7013 : revision=");
        log += String(rev,HEX);
        addLog(LOG_LEVEL_INFO,log);
      }
      
      return 0;
    }
    delay(2);
  }
  return -1; // Error timeout
}

#endif

/*!
 *  @brief  Reads serial number and stores It in sernum_a and sernum_b variable
 */
int8_t P014_data_struct::readSerialNumber(uint8_t i2caddr) {
  
  bool ok = I2C_write8_reg(i2caddr,(uint8_t)(SI70xx_CMD_ID1 >> 8),(uint8_t)(SI70xx_CMD_ID1 & 0xFF));
  if (!ok) return -1; //error on sending command 
  
  if (Wire.requestFrom(i2caddr, 8u) != 8u) {
    return -2; //time out on the SNA read
  }

  uint32_t sernum_a = Wire.read(); //SNA_3
  Wire.read(); //ignore CRC
  sernum_a <<= 8;
  sernum_a |= Wire.read(); //SNA_2
  Wire.read(); //ignore CRC
  sernum_a <<= 8;
  sernum_a |= Wire.read(); //SNA_1
  Wire.read(); //ignore CRC
  sernum_a <<= 8;
  sernum_a |= Wire.read(); //SNA_0
  Wire.read(); //ignore CRC

  ok = I2C_write8_reg(i2caddr,(uint8_t)(SI70xx_CMD_ID2 >> 8),(uint8_t)(SI70xx_CMD_ID2 & 0xFF));
  if (!ok) return -3; //error on sending command 

  if (Wire.requestFrom(i2caddr, 6u) != 6u) {
    return -4; //time out on the SNA read
  }

  uint32_t sernum_b = Wire.read(); //SNB_3 (Device ID)
  chip_id = sernum_b; //we save the chip identifier
  //Wire.read(); //ignore CRC
  sernum_b <<= 8;
  sernum_b |= Wire.read(); //SNB_2
  Wire.read(); //ignore CRC
  sernum_b <<= 8;
  sernum_b |= Wire.read(); //SNB_1
  //Wire.read(); //ignore CRC
  sernum_b <<= 8;
  sernum_b |= Wire.read(); //SNB_0
  Wire.read(); //ignore CRC

  if (loglevelActiveFor(LOG_LEVEL_INFO)){
    String log = F("SI7013 : sn=");
    log += String(sernum_a,HEX);
    log += F("-");
    log += String(sernum_b,HEX);
    addLog(LOG_LEVEL_INFO,log);
  }
  return 0; //ok
}


bool P014_data_struct::enablePowerForADC(uint8_t i2caddr){
  //set VOUT
  //Get the current register value

  //we know the type of circuit used based on the i2c address 


  bool ok=false;
  uint8_t reg = I2C_read8_reg(i2caddr,SI7013_READ_REG2, &ok);

  if (!ok) {
    addLog(LOG_LEVEL_ERROR, F("SI7013: Could not read REG2!"));
    return false;
  }
    
  
  if (i2caddr == SI7013_I2C_ADDRESS_AD0_1){
    ok = I2C_write8_reg(i2caddr,SI7013_WRITE_REG2, (reg & B11111000) | (2+4+64) );//set last three bits (VIN bufered, Vref=VDD, VOUT=GND) and No-Hold for bit 6
  }else{
    ok = I2C_write8_reg(i2caddr,SI7013_WRITE_REG2,reg | (1+2+4+64) );//set last three bits to 1 (VIN bufered, Vref=VDD, VOUT=VDD) and No-Hold for bit 6
  }
  if (!ok){
    addLog(LOG_LEVEL_ERROR, F("SI7013: Could not write REG2!"));
    return false;
  }

  return true;
}

bool P014_data_struct::disablePowerForADC(uint8_t i2caddr){
  //set VOUT
  //Get the current register value

  //we know the type of circuit used based on the i2c address 


  bool ok=false;
  uint8_t reg = I2C_read8_reg(i2caddr,SI7013_READ_REG2, &ok);

  if (!ok) {
    addLog(LOG_LEVEL_ERROR, F("SI7013: Could not read REG2!"));
    return false;
  }
    
  
  if (i2caddr == SI7013_I2C_ADDRESS_AD0_1){
    ok = I2C_write8_reg(i2caddr,SI7013_WRITE_REG2,reg | (1+2+4+64) );//set last three bits to 1 (VIN bufered, Vref=VDD, VOUT=VDD) and No-Hold for bit 6
  }else{
    ok = I2C_write8_reg(i2caddr,SI7013_WRITE_REG2, (reg & B11111000) | (2+4+64) );//set last three bits (VIN bufered, Vref=VDD, VOUT=GND) and No-Hold for bit 6
  }
  if (!ok){
    addLog(LOG_LEVEL_ERROR, F("SI7013: Could not write REG2!"));
    return false;
  }

  return true;
}

bool P014_data_struct::requestADC(uint8_t i2caddr)
{
    //start ADC conversion
    if(!I2C_write8(i2caddr, SI7013_READ_ADC)){
      addLog(LOG_LEVEL_ERROR,F("SI7013: could not start ADC conversion"));
      return false;
    }
    //delay(10); //waiting for conversion to be done: in the specs is mentioned 7ms in normal mode
  return true;
}


bool P014_data_struct::readADC(uint8_t i2caddr, uint8_t filter_power)
{
  if ( Wire.requestFrom(i2caddr, 2u) < 2 ) {
    return false;
  }

  // Comes back in two bytes, data(MSB) / data(LSB) with no Checksum
  uint16_t raw  = ((uint16_t) Wire.read()) << 8;
  raw |= Wire.read();

  //Calculate Moving average where 2^filter_power is the moving window of points
  //MA*[i]= MA*[i-1] +X[i] - MA*[i-1]/N
  adc = adc + raw - (adc>>filter_power);
  
  //disable power for ADC
  return disablePowerForADC(i2caddr);
}



bool P014_data_struct::readHumidity(uint8_t i2caddr, uint8_t resolution)
{
    uint16_t raw;
    uint8_t bytes = Wire.requestFrom(i2caddr, 3u); //asking to read 3 bytes  
    if ( bytes < 3 ) {
      return false;
    }

    // Comes back in three bytes, data(MSB) / data(LSB) / Checksum
    raw  = ((uint16_t) Wire.read()) << 8;
    raw |= Wire.read(); 
    uint8_t checksum = Wire.read();


    // Check CRC of data received
    if(checkCRC(raw, checksum) != 0) {
      addLog(LOG_LEVEL_ERROR,F("SI70xx : checksum error!"));
      return false;
    }

    // Convert raw value to Humidity percent
    // pm-cz: it is possible to enable decimal places for humidity as well by multiplying the value in formula by 100
    int data = ((1250 * (long)raw) >> 16) - 60;

    // Datasheet says doing this check
    if (data>1000) data = 1000;
    if (data<0)   data = 0;

    //pm-cz: Let us make sure we have enough precision due to ADC bits
    if (resolution == SI70xx_RESOLUTION_12T_08RH) {
      data = (data + 5) / 10;
      data *= 10;
    }
    // save value
    humidity = data;

  return true;
}

bool P014_data_struct::readTemperature(uint8_t i2caddr, uint8_t resolution)
{
  uint16_t raw;
  uint8_t bytes = Wire.requestFrom(i2caddr, 3u); //asking to read 3 bytes  
  if ( bytes < 3 ) {
    return false;
  }

  // Comes back in three bytes, data(MSB) / data(LSB) / Checksum
  raw  = ((uint16_t) Wire.read()) << 8;
  raw |= Wire.read();
  
  uint8_t checksum = Wire.read();

  // Check CRC of data received
  if(checkCRC(raw, checksum) != 0) {
    addLog(LOG_LEVEL_ERROR,F("SI70xx : checksum error!"));
    return false;
  }

  temperature =  convertRawTemperature(raw & 0xFFFC,resolution);
  return true;
}

bool P014_data_struct::readTemperatureFromHumidity(uint8_t i2caddr, uint8_t resolution){
  // Temperature
  //Request a reading
  uint16_t   raw = I2C_read16_reg(i2caddr,SI70xx_CMD_READ_TEMP_FROM_HUM);

  // save value
  temperature =  convertRawTemperature(raw,resolution);
  
  return true;
}

int16_t P014_data_struct::convertRawTemperature(uint16_t raw, uint8_t resolution){
  // Convert raw value to Temperature (*100)
  // for 23.45C value will be 2345
  int16_t data =  ((17572 * (long)raw) >> 16) - 4685;

  // pm-cz: We should probably check for precision here as well
  if (resolution != SI70xx_RESOLUTION_14T_12RH) {
    if (data > 0) {
      data = (data + 5) / 10;
    } else {
      data = (data - 5) / 10;
    }
    data *= 10;
  }

  return data;
}

#endif // ifdef USES_P014
