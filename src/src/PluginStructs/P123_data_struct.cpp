#include "../PluginStructs/P123_data_struct.h"

#ifdef USES_P123

// the main constructor
P123_data_struct::P123_data_struct() :
  last_hum_val(0.0f),
  last_temp_val(0.0f),
  last_measurement(0),
  state(P123_state::Uninitialized) {}

//check if the plugin is initialized
bool P123_data_struct::initialized() const {
  return state != P123_state::Uninitialized;
}


//The main state machine
//Only perform the measurements with big interval to prevent the sensor from warming up.
bool P123_data_struct::update(uint8_t i2caddr, uint8_t resolution, uint8_t filter_power) {
  const unsigned long current_time = millis();
  int8_t ret=0;
  switch(state){
    case P123_state::Uninitialized:
        if ((ret=begin(i2caddr,resolution))!=0) {
          String log =F("SI7013: begin Failed! ret=");
          log+= String(ret,HEX);
          addLog(LOG_LEVEL_ERROR,log);
          return false;
        }

        state            = P123_state::Ready;
        last_measurement = 0;
        if ( (ret=readADC(i2caddr,filter_power)) !=0 ){
          String log = F("SI7013: readADC Failed! ret=");
          log+= String(ret,HEX);
          addLog(LOG_LEVEL_ERROR,log);
        }
        last_adc_val = last_adc_val << filter_power; //this is the first measurement 
    break;

    case P123_state::Ready:
        last_measurement = current_time;
        if ( (ret=startConv(i2caddr, SI7013_MEASURE_HUM,resolution)) !=0) { //measure humidity and temperature at the same time
          String log = F("SI7013: startConv Failed! ret=");
          log+= String(ret,HEX);
          addLog(LOG_LEVEL_ERROR,log);
        }
        state = P123_state::Wait_for_HUM;
    break;

    case P123_state::Error:
        if ((ret=softReset(i2caddr))!=0){
          String log= F("SI7013: softReset Failed! Err=");
          log += String(ret,HEX);
          addLog(LOG_LEVEL_ERROR,log);
          return false;
        }
        state = P123_state::Ready;
    break;

    case P123_state::Wait_for_HUM:
        //make sure we wait for the measurement to complete
        if (!timeOutReached(last_measurement + SI7013_MEASURMENT_DELAY)) {
          return false;
        }

        if ((ret=readHumidity(i2caddr, resolution))!=0) {
          String log= F("SI7013: readHumidity Failed! Err=");
          log += String(ret,HEX);
          addLog(LOG_LEVEL_ERROR,log);
          state = P123_state::Error;
          return false;
        }
        last_measurement = current_time;
        //last_temp_val    = readTemperature();
        //last_hum_val     = readHumidity();
        //last_adc_val     = readADC();
        if ((ret=requestTemperature(i2caddr))!=0){
          String log= F("SI7013: requestTemperature Failed! Err=");
          log += String(ret,HEX);
          addLog(LOG_LEVEL_ERROR,log);
          state = P123_state::Error;
          return false;
        }
        state = P123_state::Wait_for_Temp;
    break;

    case P123_state::Wait_for_Temp:
        if ((ret=readTemperature(i2caddr, resolution))!=0){
          String log= F("SI7013: readTemperature Failed! Err=");
          log += String(ret,HEX);
          addLog(LOG_LEVEL_ERROR,log);
          state = P123_state::Error;
          return false;
        }

        if ((ret=requestADC(i2caddr))!=0){
          String log= F("SI7013: requestADC Failed! Err=");
          log += String(ret,HEX);
          addLog(LOG_LEVEL_ERROR,log);
          state = P123_state::Error;
          return false;
        }
        state = P123_state::Wait_for_ADC;
    break;

    case P123_state::Wait_for_ADC:
        if ((ret=readADC(i2caddr, filter_power))!=0){
          String log= F("SI7013: readADC Failed! Err=");
          log += String(ret,HEX);
          addLog(LOG_LEVEL_ERROR,log);
          state = P123_state::Error;
          return false;
        }
        state = P123_state::New_values_available;
    break;

    default: //anything not defined above sends the device for initialization
      state = P123_state::Uninitialized;
  }  
  return true;
}


/* ======================================================================
Function: Plugin_123_si7013_begin
Purpose : read the user register from the sensor
Input   : user register value filled by function
Output  : 0 if okay
        : -1 device not available at the expected address
        : 
Comments: -
====================================================================== */
int8_t P123_data_struct::begin(uint8_t i2caddr, uint8_t resolution)
{
  int8_t ret;



  Wire.beginTransmission(i2caddr);
  if (Wire.endTransmission()){
    addLog(LOG_LEVEL_ERROR,F("SI7013: Device not available!")); 

    Wire.requestFrom(i2caddr, 10u);
    while(Wire.available()>=1) {
      Wire.read();
    }
    
  

    return -1; // device not available at the expected address
  }


  softReset(i2caddr);
  uint8_t reg;
  if ( (ret=readRegister(i2caddr,SI7013_READ_REG1, &reg))!=0){
    addLog(LOG_LEVEL_ERROR,F("SI7013: Can't read SI7013_READ_REG1!"));
    return -2;
  }

  if (reg != 0x3A){
    addLog(LOG_LEVEL_ERROR,F("SI7013: SI7013_READ_REG1 does not match default value"));
    return -3; //invalid register1 value after reset
  }

  readSerialNumber(i2caddr);
  readRevision(i2caddr);


  // Set the resolution we want
  
  if ( (ret = setResolution(i2caddr, resolution)!=0))  {
    String log = F("SI7013 : Res=0x");
    log += String(resolution,HEX);
    log += F(" => Error 0x");
    log += String(ret,HEX);
    addLog(LOG_LEVEL_ERROR,log);
    ret = -4;
  }

  return ret;
}

/* ======================================================================
Function: Plugin_123_si7013_checkCRC
Purpose : check the CRC of received data
Input   : value read from sensor
Output  : CRC read from sensor
Comments: 0 if okay
====================================================================== */
uint8_t P123_data_struct::checkCRC(uint16_t data, uint8_t check)
{
  uint32_t remainder, divisor;

  //Pad with 8 bits because we have to add in the check value
  remainder = (uint32_t)data << 8;

  // From: http://www.nongnu.org/avr-libc/user-manual/group__util__crc.html
  // POLYNOMIAL = 0x0131 = x^8 + x^5 + x^4 + 1 : http://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks
  // 0x988000 is the 0x0131 polynomial shifted to farthest left of three bytes
  divisor = (uint32_t) 0x988000;

  // Add the check value
  remainder |= check;

  // Operate on only 16 positions of max 24.
  // The remaining 8 are our remainder and should be zero when we're done.
  for (uint8_t i = 0 ; i < 16 ; i++) {
    //Check if there is a one in the left position
    if( remainder & (uint32_t)1<<(23 - i) )
      remainder ^= divisor;

    //Rotate the divisor max 16 times so that we have 8 bits left of a remainder
    divisor >>= 1;
  }
  return ((uint8_t) remainder);
}

/* ======================================================================
Function: si7013_readRegister
Purpose : read the user register from the sensor
Input   : user register value filled by function
Output  : 0 if okay
Comments: -
====================================================================== */
int8_t P123_data_struct::readRegister(uint8_t i2caddr, const uint8_t reg, uint8_t * value)
{
  
  // Request user register
  Wire.beginTransmission(i2caddr);
  Wire.write(reg);
  Wire.endTransmission();

  // request 1 byte result
  Wire.requestFrom(i2caddr, 1u);
  if (Wire.available()>=1) {
      *value = Wire.read();
      return 0;
  }

  return 0;
}

/* ======================================================================
Function: Plugin_123_si7013_startConv
Purpose : return temperature or humidity measured
Input   : data type SI7013_READ_HUM or SI7013_READ_TEMP
          current config resolution
Output  : 0 if okay
Comments: internal values of temp and rh are set
====================================================================== */
int8_t P123_data_struct::startConv(uint8_t i2caddr, uint8_t datatype, uint8_t resolution)
{
  //long data;
  //uint16_t raw ;
  //uint8_t checksum,tmp;



  //Request a reading
  Wire.beginTransmission(i2caddr);
  Wire.write(datatype);
  Wire.endTransmission();

  // Tried clock streching and looping until no NACK from SI7021 to know
  // when conversion's done. None have worked so far !!!
  // I fade up, I'm waiting maximum conversion time + 1ms, this works !!
  // I increased these value to add HTU21D compatibility
  // Max for SI7021 is 3/5/7/12 ms
  // max for HTU21D is 7/13/25/50 ms

  // Martinus modification 2016-01-07:
  // My test sample was still not working with 11 bit
  // So to be more safe, we add 5 ms to each and use 8,10,13,21 ms
  // But for ESP Easy, I think it does not matter at all...

  // Martinus is correct there was a bug Measure HUM need
  // hum+temp delay because it also measure temp

  /*
  if (resolution == SI7013_RESOLUTION_11T_11RH)
    tmp = 7;
  else if (resolution == SI7013_RESOLUTION_12T_08RH)
    tmp = 13;
  else if (resolution == SI7013_RESOLUTION_13T_10RH)
    tmp = 25;
  else
    tmp = 30;

  // Humidity fire also temp measurment so delay
  // need to be increased by 2 if no Hold Master
  if (datatype == SI7013_MEASURE_HUM)
    tmp *=2;

  if (datatype != SI7013_READ_TEMP_FROM_HUM)
      delay(tmp);

  
  // Wait for data to become available, device will NACK during conversion
  tmp = 0;
  do
  {
    // Request device
    Wire.beginTransmission(SI7021_I2C_ADDRESS);
    //Wire.write(SI7021_READ_REG);
    error = Wire.endTransmission(true);
    delay(1);
  }
  // always use time out in loop to avoid potential lockup (here 12ms max)
  // https://www.silabs.com/Support%20Documents/TechnicalDocs/Si7021-A20.pdf page 5
  while(error!=0 && tmp++<=12 );
  */



  return 0;
}



int8_t P123_data_struct::readHumidity(uint8_t i2caddr, uint8_t resolution)
{
    uint16_t raw;
    uint8_t bytes = Wire.requestFrom(i2caddr, 3u); //asking to read 3 bytes  
    if ( bytes < 3 ) {
      return -1;
    }

    // Comes back in three bytes, data(MSB) / data(LSB) / Checksum
    raw  = ((uint16_t) Wire.read()) << 8;
    raw |= Wire.read();
    uint8_t checksum = Wire.read();


    // Check CRC of data received
    if(checkCRC(raw, checksum) != 0) {
      addLog(LOG_LEVEL_ERROR,F("SI7013 : checksum error!"));
      return -1;
    }

    // Convert raw value to Humidity percent
    // pm-cz: it is possible to enable decimal places for humidity as well by multiplying the value in formula by 100
    uint16_t data = ((1250 * (long)raw) >> 16) - 60;

    // Datasheet says doing this check
    if (data>1000) data = 1000;
    if (data<0)   data = 0;

    //pm-cz: Let us make sure we have enough precision due to ADC bits
    if (resolution == SI7013_RESOLUTION_12T_08RH) {
      data = (data + 5) / 10;
      data *= 10;
    }
    // save value
    last_hum_val = data/10.0f;

  return 0;
}


int8_t P123_data_struct::requestTemperature(uint8_t i2caddr){
  
  // Temperature
  //Request a reading
  Wire.beginTransmission(i2caddr);
  Wire.write(SI7013_READ_TEMP_FROM_HUM);
  Wire.endTransmission();

  return 0;
}

int8_t P123_data_struct::readTemperature(uint8_t i2caddr, uint8_t resolution){
  
  // Temperature
  //Request a reading
  //Wire.beginTransmission(i2caddr);
  //Wire.write(SI7013_READ_TEMP_FROM_HUM);
  //Wire.endTransmission();

  
  //delay(10);
  
  uint8_t bytes = Wire.requestFrom(i2caddr, 2u); //asking to read 2 bytes  
  if ( bytes < 2 ) {
    return -1;
  }

  // Comes back in three bytes, data(MSB) / data(LSB) / Checksum
  uint16_t   raw  = ((uint16_t) Wire.read()) << 8;
  raw |= Wire.read();

  // Convert value to Temperature (*100)
  // for 23.45C value will be 2345
  int16_t data =  ((17572 * (long)raw) >> 16) - 4685;

  
  // pm-cz: We should probably check for precision here as well
  if (resolution != SI7013_RESOLUTION_14T_12RH) {
    if (data > 0) {
      data = (data + 5) / 10;
    } else {
      data = (data - 5) / 10;
    }
    data *= 10;
  }
  

  // save value
  last_temp_val =  data / 100.0f;
  
  return 0;
}

/* ======================================================================
Function: Plugin_123_si7013_readValues
Purpose : read temperature and humidity from SI7021 sensor
Input   : current config resolution
Output  : 0 if okay
Comments: -
====================================================================== */
int8_t P123_data_struct::readValues(uint8_t i2caddr, uint8_t resolution, uint8_t filter_power)
{
  int8_t error = 0;

  // start humidity conversion
 // error |= startConv(i2caddr, SI7013_MEASURE_HUM, resolution); //measures both humidity and temperature 
 //error |= Plugin_123_si7013_startConv(i2caddr, SI7013_MEASURE_HUM_HM, resolution); //measures only humidity
  

 //the humidity is actually doing a temperature reading Too
 //error|= Plugin_014_si7021_startConv(SI7021_READ_REG)

  // start temperature conversion
  //error |= Plugin_123_si7013_startConv(i2caddr, SI7013_MEASURE_TEMP, resolution);
  
  //error |= Plugin_123_si7013_startConv(i2caddr, SI7013_READ_TEMP_FROM_HUM, resolution);

  uint16_t raw;
    uint8_t bytes = Wire.requestFrom(i2caddr, 3u); //asking to read 3 bytes  
    if ( bytes < 2 ) {
      return -1;
 
    if (bytes == 2){
      // Comes back in two bytes, data(MSB) / data(LSB)  (no Checksum)
        raw  = ((uint16_t) Wire.read()) << 8;
        raw |= Wire.read();
    }else
    {
    
    //if ( Wire.requestFrom(i2caddr, 3u) < 3 ) {
    //  return -1;
    //}

    // Comes back in three bytes, data(MSB) / data(LSB) / Checksum
    raw  = ((uint16_t) Wire.read()) << 8;
    raw |= Wire.read();
    uint8_t checksum = Wire.read();


    // Check CRC of data received
    if(checkCRC(raw, checksum) != 0) {
      addLog(LOG_LEVEL_ERROR,F("SI7013 : checksum error!"));
      return -1;
    }

  }
  // Humidity
  //if (datatype == SI7013_MEASURE_HUM || datatype == SI7013_MEASURE_HUM_HM) {
    // Convert value to Humidity percent
    // pm-cz: it is possible to enable decimal places for humidity as well by multiplying the value in formula by 100
    uint16_t data = ((1250 * (long)raw) >> 16) - 60;

    // Datasheet says doing this check
    if (data>1000) data = 1000;
    if (data<0)   data = 0;

    //pm-cz: Let us make sure we have enough precision due to ADC bits
    if (resolution == SI7013_RESOLUTION_12T_08RH) {
      data = (data + 5) / 10;
      data *= 10;
    }
    // save value
    last_hum_val = data/10.0f;

  // Temperature
  //Request a reading
  Wire.beginTransmission(i2caddr);
  Wire.write(SI7013_READ_TEMP_FROM_HUM);
  Wire.endTransmission();

  
    // Convert value to Temperature (*100)
    // for 23.45C value will be 2345
    data =  ((17572 * (long)raw) >> 16) - 4685;

    /*
    // pm-cz: We should probably check for precision here as well
    if (resolution != SI7021_RESOLUTION_14T_12RH) {
      if (data > 0) {
        data = (data + 5) / 10;
      } else {
        data = (data - 5) / 10;
      }
      data *= 10;
    }
    */

    // save value
    last_temp_val =  data / 100.0f;
  }
  
  error|= readADC(i2caddr,filter_power);

  return error;
}



int8_t P123_data_struct::requestADC(uint8_t i2caddr)
{
  
  uint8_t reg;
  int8_t error;

  //set VOUT
  // Get the current register value
  error = readRegister(i2caddr, SI7013_READ_REG2, &reg);
  if ( error == 0) {
       
      // Prepare to write to the register value
    Wire.beginTransmission(i2caddr);
    Wire.write(SI7013_WRITE_REG2);
    
    Wire.write(reg | (1+2+4+64) );//set last three bits to 1 (VIN bufered, Vref=VDD, VOUT=VDD) and No-Hold for bit 6

    Wire.endTransmission();
 
    //read adc
    Wire.beginTransmission(i2caddr);
    Wire.write(SI7013_READ_ADC);
    Wire.endTransmission();

    //delay(10); //wating for conversion to be done in the specs is mentioned 7ms in normal mode
  }
  return error;
}

int8_t P123_data_struct::readADC(uint8_t i2caddr, uint8_t filter_power)
{
  if ( Wire.requestFrom(i2caddr, 2u) < 2 ) {
    return -1;
  }

  // Comes back in two bytes, data(MSB) / data(LSB) with no Checksum
  uint16_t raw  = ((uint16_t) Wire.read()) << 8;
  raw |= Wire.read();

  //Calculate Moving average where 2^filter_power is the moving window of points
  //MA*[i]= MA*[i-1] +X[i] - MA*[i-1]/N
  last_adc_val = last_adc_val + raw - (last_adc_val>>filter_power);
  
  //set vout to gnd to not consume power
  Wire.beginTransmission(i2caddr);
    Wire.write(SI7013_WRITE_REG2);
    Wire.write(SI7013_REG2_DEFAULT);
  return (int8_t) Wire.endTransmission();
}

/* ======================================================================
Function: Plugin_123_si7013_setResolution
Purpose : Sets the sensor resolution to one of four levels
Input   : see #define default is SI7013_RESOLUTION_14T_12RH
Output  : 0 if okay
Comments: -
====================================================================== */
int8_t P123_data_struct::setResolution(uint8_t i2caddr, uint8_t resolution)
{
  uint8_t reg;
  uint8_t error;

  // Get the current register value
  error = readRegister(i2caddr, SI7013_READ_REG1, &reg);
  if ( error == 0) {
    // remove resolution bits
    reg &= SI7013_RESOLUTION_MASK ;

    // Prepare to write to the register value
    Wire.beginTransmission(i2caddr);
    Wire.write(SI7013_WRITE_REG1);

    // Write the new resolution bits but clear unused before
    Wire.write(reg | ( resolution &= ~SI7013_RESOLUTION_MASK) );
    return (int8_t) Wire.endTransmission();
  }

  return error;
}


int8_t P123_data_struct::softReset(uint8_t i2caddr){
  // Prepare to write to the register value
    Wire.beginTransmission(i2caddr);
    Wire.write(SI7013_SOFT_RESET);
    Wire.endTransmission();
    delay(50);
    return 0;
}


int8_t P123_data_struct::readRevision(uint8_t i2caddr) {
  Wire.beginTransmission(i2caddr);
  Wire.write((uint8_t)(SI7013_FIRMVERS_CMD >> 8));
  Wire.write((uint8_t)(SI7013_FIRMVERS_CMD & 0xFF));
  Wire.endTransmission();

  uint32_t start = millis(); // start timeout
  while (millis() - start < SI7013_MEASURMENT_DELAY) {
    if (Wire.requestFrom(i2caddr, 2u) == 2) {
      uint8_t rev = Wire.read();
      Wire.read();

      String log = F("SI7013 : revision=");
      log += String(rev,HEX);
      addLog(LOG_LEVEL_INFO,log);
      
      return 0;
    }
    delay(2);
  }
  return -1; // Error timeout
}

/*!
 *  @brief  Reads serial number and stores It in sernum_a and sernum_b variable
 */
int8_t P123_data_struct::readSerialNumber(uint8_t i2caddr) {
  
  Wire.beginTransmission(i2caddr);
  Wire.write((uint8_t)(SI7013_ID1_CMD >> 8));
  Wire.write((uint8_t)(SI7013_ID1_CMD & 0xFF));
  Wire.endTransmission();

  bool gotData = false;
  uint32_t start = millis(); // start timeout
  while (millis() - start < SI7013_MEASURMENT_DELAY) {
    if (Wire.requestFrom(i2caddr, 8u) == 8) {
      gotData = true;
      break;
    }
    delay(2);
  }
  if (!gotData)
    return -1; // error timeout

  uint32_t sernum_a = Wire.read();
  Wire.read();
  sernum_a <<= 8;
  sernum_a |= Wire.read();
  Wire.read();
  sernum_a <<= 8;
  sernum_a |= Wire.read();
  Wire.read();
  sernum_a <<= 8;
  sernum_a |= Wire.read();
  Wire.read();

  Wire.beginTransmission(i2caddr);
  Wire.write((uint8_t)(SI7013_ID2_CMD >> 8));
  Wire.write((uint8_t)(SI7013_ID2_CMD & 0xFF));
  Wire.endTransmission();

  gotData = false;
  start = millis(); // start timeout
  while (millis() - start < SI7013_MEASURMENT_DELAY) {
    if (Wire.requestFrom(i2caddr, 8u) == 8) {
      gotData = true;
      break;
    }
    delay(2);
  }
  if (!gotData)
    return -2; // error timeout

  uint32_t sernum_b = Wire.read();
  Wire.read();
  sernum_b <<= 8;
  sernum_b |= Wire.read();
  Wire.read();
  sernum_b <<= 8;
  sernum_b |= Wire.read();
  Wire.read();
  sernum_b <<= 8;
  sernum_b |= Wire.read();
  Wire.read();

  String log = F("SI7013 : sn=");
  log += String(sernum_a,HEX);
  log += String(sernum_b,HEX);
  addLog(LOG_LEVEL_INFO,log);

  return 0;
}


#endif // ifdef USES_P123
