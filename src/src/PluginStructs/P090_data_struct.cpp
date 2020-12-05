#include "../PluginStructs/P090_data_struct.h"

#ifdef USES_P090

#include "../Globals/I2Cdev.h"

// Register addresses
# define CSS811_STATUS          0x00
# define CSS811_MEAS_MODE       0x01
# define CSS811_ALG_RESULT_DATA 0x02
# define CSS811_RAW_DATA        0x03
# define CSS811_ENV_DATA        0x05
# define CSS811_NTC             0x06
# define CSS811_THRESHOLDS      0x10
# define CSS811_BASELINE        0x11
# define CSS811_HW_ID           0x20
# define CSS811_HW_VERSION      0x21
# define CSS811_FW_BOOT_VERSION 0x23
# define CSS811_FW_APP_VERSION  0x24
# define CSS811_ERROR_ID        0xE0
# define CSS811_APP_START       0xF4
# define CSS811_SW_RESET        0xFF

// ****************************************************************************//
//
//  LIS3DHCore functions
//
//  For I2C, construct LIS3DHCore myIMU(<address>);
//
//  Default <address> is 0x5B.
//
// ****************************************************************************//
CCS811Core::CCS811Core(uint8_t inputArg) : I2CAddress(inputArg)
{}

void CCS811Core::setAddress(uint8_t address)
{
  I2CAddress = address;
}

CCS811Core::status CCS811Core::beginCore(void)
{
  CCS811Core::status returnError = SENSOR_SUCCESS;

  // Wire.begin(); // not necessary

    # ifdef __AVR__
    # else
    # endif

    # ifdef __MK20DX256__
    # else
    # endif

    # ifdef ARDUINO_ARCH_ESP8266
    # else
    # endif

  // Spin for a few ms
  volatile uint8_t temp = 0;

  for (uint16_t i = 0; i < 10000; i++)
  {
    temp++;
  }

  while (Wire.available()) // Clear wire as a precaution
  {
    Wire.read();
  }

  // Check the ID register to determine if the operation was a success.
  uint8_t readCheck;
  readCheck   = 0;
  returnError = readRegister(CSS811_HW_ID, &readCheck);

  if (returnError != SENSOR_SUCCESS)
  {
    return returnError;
  }

  if (readCheck != 0x81)
  {
    returnError = SENSOR_ID_ERROR;
  }

  return returnError;
} // CCS811Core::beginCore

// ****************************************************************************//
//
//  ReadRegister
//
//  Parameters:
//    offset -- register to read
//    *outputPointer -- Pass &variable (address of) to save read data to
//
// ****************************************************************************//
CCS811Core::status CCS811Core::readRegister(uint8_t offset, uint8_t *outputPointer)
{
  bool wire_status = false;

  *outputPointer = I2C_read8_reg(I2CAddress, offset, &wire_status);

  if (wire_status) {
    return SENSOR_SUCCESS;
  }
  return SENSOR_I2C_ERROR;
}

// ****************************************************************************//
//
//  writeRegister
//
//  Parameters:
//    offset -- register to write
//    dataToWrite -- 8 bit data to write to register
//
// ****************************************************************************//
CCS811Core::status CCS811Core::writeRegister(uint8_t offset, uint8_t dataToWrite)
{
  if (I2C_write8_reg(I2CAddress, offset, dataToWrite)) {
    return SENSOR_SUCCESS;
  }
  return SENSOR_I2C_ERROR;
}

// ****************************************************************************//
//
//  multiReadRegister
//
//  Parameters:
//    offset -- register to read
//    *inputPointer -- Pass &variable (base address of) to save read data to
//    length -- number of bytes to read
//
//  Note:  Does not know if the target memory space is an array or not, or
//    if there is the array is big enough.  if the variable passed is only
//    two bytes long and 3 bytes are requested, this will over-write some
//    other memory!
//
// ****************************************************************************//
CCS811Core::status CCS811Core::multiWriteRegister(uint8_t offset, uint8_t *inputPointer, uint8_t length)
{
  CCS811Core::status returnError = SENSOR_SUCCESS;

  // define pointer that will point to the external space
  uint8_t i = 0;

  // Set the address
  Wire.beginTransmission(I2CAddress);
  Wire.write(offset);

  while (i < length)           // send data bytes
  {
    Wire.write(*inputPointer); // receive a byte as character
    inputPointer++;
    i++;
  }

  if (Wire.endTransmission() != 0)
  {
    returnError = SENSOR_I2C_ERROR;
  }

  return returnError;
}

// ****************************************************************************//
//
//  Main user class -- wrapper for the core class + maths
//
//  Construct with same rules as the core ( uint8_t busType, uint8_t inputArg )
//
// ****************************************************************************//
CCS811::CCS811(uint8_t inputArg) : CCS811Core(inputArg)
{
  refResistance = 10000;
  resistance    = 0;
  _temperature  = 0;
  tVOC          = 0;
  CO2           = 0;
}

// ****************************************************************************//
//
//  Begin
//
//  This starts the lower level begin, then applies settings
//
// ****************************************************************************//
CCS811Core::status CCS811::begin(void)
{
  uint8_t data[4] = { 0x11, 0xE5, 0x72, 0x8A };    // Reset key

  CCS811Core::status returnError = SENSOR_SUCCESS; // Default error state

  // restart the core
  returnError = beginCore();

  if (returnError != SENSOR_SUCCESS)
  {
    return returnError;
  }

  // Reset the device
  multiWriteRegister(CSS811_SW_RESET, data, 4);
  delay(1);

  if (checkForStatusError() == true)
  {
    return SENSOR_INTERNAL_ERROR;
  }

  if (appValid() == false)
  {
    return SENSOR_INTERNAL_ERROR;
  }

  // Write 0 bytes to this register to start app
  Wire.beginTransmission(I2CAddress);
  Wire.write(CSS811_APP_START);

  if (Wire.endTransmission() != 0)
  {
    return SENSOR_I2C_ERROR;
  }

  delay(200);

  // returnError = setDriveMode(1); //Read every second
  //    Serial.println();

  return returnError;
} // CCS811::begin

// ****************************************************************************//
//
//  Sensor functions
//
// ****************************************************************************//
// Updates the total voltatile organic compounds (TVOC) in parts per billion (PPB)
// and the CO2 value
// Returns nothing
CCS811Core::status CCS811::readAlgorithmResults(void)
{
  I2Cdata_bytes data(4, CSS811_ALG_RESULT_DATA);
  bool allDataRead = I2C_read_bytes(I2CAddress, data);

  if (!allDataRead) {
    return SENSOR_I2C_ERROR;
  }

  // Data ordered:
  // co2MSB, co2LSB, tvocMSB, tvocLSB

  CO2  = ((uint16_t)data[CSS811_ALG_RESULT_DATA + 0] << 8) | data[CSS811_ALG_RESULT_DATA + 1];
  tVOC = ((uint16_t)data[CSS811_ALG_RESULT_DATA + 2] << 8) | data[CSS811_ALG_RESULT_DATA + 3];
  return SENSOR_SUCCESS;
}

// Checks to see if error bit is set
bool CCS811::checkForStatusError(void)
{
  uint8_t value;

  // return the status bit
  readRegister(CSS811_STATUS, &value);
  return value & (1 << 0);
}

// Checks to see if DATA_READ flag is set in the status register
bool CCS811::dataAvailable(void)
{
  uint8_t value;

  CCS811Core::status returnError = readRegister(CSS811_STATUS, &value);

  if (returnError != SENSOR_SUCCESS)
  {
    return false;
  }
  else
  {
    return value & (1 << 3);
  }
}

// Checks to see if APP_VALID flag is set in the status register
bool CCS811::appValid(void)
{
  uint8_t value;

  CCS811Core::status returnError = readRegister(CSS811_STATUS, &value);

  if (returnError != SENSOR_SUCCESS)
  {
    return false;
  }
  else
  {
    return value & (1 << 4);
  }
}

uint8_t CCS811::getErrorRegister(void)
{
  uint8_t value;

  CCS811Core::status returnError = readRegister(CSS811_ERROR_ID, &value);

  if (returnError != SENSOR_SUCCESS)
  {
    return 0xFF;
  }
  else
  {
    return value; // Send all errors in the event of communication error
  }
}

// Returns the baseline value
// Used for telling sensor what 'clean' air is
// You must put the sensor in clean air and record this value
uint16_t CCS811::getBaseline(void)
{
  return I2C_read16_reg(I2CAddress, CSS811_BASELINE);
}

CCS811Core::status CCS811::setBaseline(uint16_t input)
{
  if (I2C_write16_reg(I2CAddress, CSS811_BASELINE, input)) {
    return SENSOR_SUCCESS;
  }
  return SENSOR_I2C_ERROR;
}

// Enable the nINT signal
CCS811Core::status CCS811::enableInterrupts(void)
{
  uint8_t value;
  CCS811Core::status returnError = readRegister(CSS811_MEAS_MODE, &value); // Read what's currently there

  if (returnError != SENSOR_SUCCESS)
  {
    return returnError;
  }

  //    Serial.println(value, HEX);
  value |= (1 << 3); // Set INTERRUPT bit
  writeRegister(CSS811_MEAS_MODE, value);

  //    Serial.println(value, HEX);
  return returnError;
}

// Disable the nINT signal
CCS811Core::status CCS811::disableInterrupts(void)
{
  uint8_t value;

  CCS811Core::status returnError = readRegister(CSS811_MEAS_MODE, &value); // Read what's currently there

  if (returnError != SENSOR_SUCCESS)
  {
    return returnError;
  }

  value      &= ~(1 << 3); // Clear INTERRUPT bit
  returnError = writeRegister(CSS811_MEAS_MODE, value);
  return returnError;
}

// Mode 0 = Idle
// Mode 1 = read every 1s
// Mode 2 = every 10s
// Mode 3 = every 60s
// Mode 4 = RAW mode
CCS811Core::status CCS811::setDriveMode(uint8_t mode)
{
  if (mode > 4)
  {
    mode = 4; // sanitize input
  }

  uint8_t value;
  CCS811Core::status returnError = readRegister(CSS811_MEAS_MODE, &value); // Read what's currently there

  if (returnError != SENSOR_SUCCESS)
  {
    return returnError;
  }

  value      &= ~(0b00000111 << 4); // Clear DRIVE_MODE bits
  value      |= (mode << 4);        // Mask in mode
  returnError = writeRegister(CSS811_MEAS_MODE, value);
  return returnError;
}

// Given a temp and humidity, write this data to the CSS811 for better compensation
// This function expects the humidity and temp to come in as floats
CCS811Core::status CCS811::setEnvironmentalData(float relativeHumidity, float temperature)
{
  // Check for invalid temperatures
  if ((temperature < -25) || (temperature > 50))
  {
    return SENSOR_GENERIC_ERROR;
  }

  // Check for invalid humidity
  if ((relativeHumidity < 0) || (relativeHumidity > 100))
  {
    return SENSOR_GENERIC_ERROR;
  }

  uint32_t rH   = relativeHumidity * 1000; // 42.348 becomes 42348
  uint32_t temp = temperature * 1000;      // 23.2 becomes 23200

  byte envData[4];

  // Split value into 7-bit integer and 9-bit fractional
  envData[0] = ((rH % 1000) / 100) > 7 ? (rH / 1000 + 1) << 1 : (rH / 1000) << 1;
  envData[1] = 0; // CCS811 only supports increments of 0.5 so bits 7-0 will always be zero

  if ((((rH % 1000) / 100) > 2) && (((rH % 1000) / 100) < 8))
  {
    envData[0] |= 1; // Set 9th bit of fractional to indicate 0.5%
  }

  temp += 25000;     // Add the 25C offset
  // Split value into 7-bit integer and 9-bit fractional
  envData[2] = ((temp % 1000) / 100) > 7 ? (temp / 1000 + 1) << 1 : (temp / 1000) << 1;
  envData[3] = 0;

  if ((((temp % 1000) / 100) > 2) && (((temp % 1000) / 100) < 8))
  {
    envData[2] |= 1; // Set 9th bit of fractional to indicate 0.5C
  }

  CCS811Core::status returnError = multiWriteRegister(CSS811_ENV_DATA, envData, 4);

  return returnError;
} // CCS811::setEnvironmentalData

void CCS811::setRefResistance(float input)
{
  refResistance = input;
}

CCS811Core::status CCS811::readNTC(void)
{
  I2Cdata_bytes data(4, CSS811_NTC);
  bool allDataRead = I2C_read_bytes(I2CAddress, data);

  if (!allDataRead) {
    return SENSOR_I2C_ERROR;
  }

  vrefCounts = ((uint16_t)data[CSS811_NTC + 0] << 8) | data[CSS811_NTC + 1];

  // Serial.print("vrefCounts: ");
  // Serial.println(vrefCounts);
  ntcCounts = ((uint16_t)data[CSS811_NTC + 2] << 8) | data[CSS811_NTC + 3];

  // Serial.print("ntcCounts: ");
  // Serial.println(ntcCounts);
  // Serial.print("sum: ");
  // Serial.println(ntcCounts + vrefCounts);
  resistance = ((float)ntcCounts * refResistance / (float)vrefCounts);

  // Code from Milan Malesevic and Zoran Stupic, 2011,
  // Modified by Max Mayfield,
  _temperature = log((long)resistance);
  _temperature = 1  / (0.001129148f + (0.000234125f * _temperature) + (0.0000000876741f * _temperature * _temperature * _temperature));
  _temperature = _temperature - 273.15f; // Convert Kelvin to Celsius

  return SENSOR_SUCCESS;
}

uint16_t CCS811::getTVOC(void)
{
  return tVOC;
}

uint16_t CCS811::getCO2(void)
{
  return CO2;
}

float CCS811::getResistance(void)
{
  return resistance;
}

float CCS811::getTemperature(void)
{
  return _temperature;
}

// getDriverError decodes the CCS811Core::status type and prints the
// type of error to the serial terminal.
//
// Save the return value of any function of type CCS811Core::status, then pass
// to this function to see what the output was.
String CCS811::getDriverError(CCS811Core::status errorCode)
{
  switch (errorCode)
  {
    case CCS811Core::SENSOR_SUCCESS:
      return F("SUCCESS");

    case CCS811Core::SENSOR_ID_ERROR:
      return F("ID_ERROR");

    case CCS811Core::SENSOR_I2C_ERROR:
      return F("I2C_ERROR");

    case CCS811Core::SENSOR_INTERNAL_ERROR:
      return F("INTERNAL_ERROR");

    case CCS811Core::SENSOR_GENERIC_ERROR:
      return F("GENERIC_ERROR");

    default:
      return F("Unknown");
  }
}

// getSensorError gets, clears, then prints the errors
// saved within the error register.
String CCS811::getSensorError()
{
  uint8_t error = getErrorRegister();

  if (error == 0xFF)
  {
    return F("Failed to get ERROR_ID register.");
  }
  else
  {
    if (error & 1 << 5)
    {
      return F("HeaterSupply");
    }

    if (error & 1 << 4)
    {
      return F("HeaterFault");
    }

    if (error & 1 << 3)
    {
      return F("MaxResistance");
    }

    if (error & 1 << 2)
    {
      return F("MeasModeInvalid");
    }

    if (error & 1 << 1)
    {
      return F("ReadRegInvalid");
    }

    if (error & 1 << 0)
    {
      return F("MsgInvalid");
    }
  }
  return "";
}

P090_data_struct::P090_data_struct(uint8_t i2cAddr) :
  myCCS811(0x5B) // start with default, but will update later on with user settings
{
  myCCS811.setAddress(i2cAddr);
}

#endif // ifdef USES_P090
