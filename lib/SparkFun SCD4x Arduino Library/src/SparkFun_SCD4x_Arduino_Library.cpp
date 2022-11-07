/*
  This is a library written for the SCD4x family of CO2 sensors
  SparkFun sells these at its website: www.sparkfun.com
  Do you like this library? Help support SparkFun. Buy a board!
  https://www.sparkfun.com/products/18365

  Written by Paul Clark @ SparkFun Electronics, June 2nd, 2021

  The SCD41 measures CO2 from 400ppm to 5000ppm with an accuracy of +/- 40ppm + 5% of reading

  This library handles the initialization of the SCD4x and outputs
  CO2 levels, relative humidty, and temperature.

  https://github.com/sparkfun/SparkFun_SCD4x_Arduino_Library

  Development environment specifics:
  Arduino IDE 1.8.13

  SparkFun code, firmware, and software is released under the MIT License.
  Please see LICENSE.md for more details.
*/

#include "SparkFun_SCD4x_Arduino_Library.h"

SCD4x::SCD4x(scd4x_sensor_type_e sensorType)
{
  // Constructor
  _sensorType = sensorType;
}

//Initialize the Serial port
#ifdef USE_TEENSY3_I2C_LIB
bool SCD4x::begin(i2c_t3 &wirePort, bool measBegin, bool autoCalibrate, bool skipStopPeriodicMeasurements)
#else
bool SCD4x::begin(TwoWire &wirePort, bool measBegin, bool autoCalibrate, bool skipStopPeriodicMeasurements)
#endif
{
  _i2cPort = &wirePort; //Grab which port the user wants us to use

  bool success = true;

  //If periodic measurements are already running, getSerialNumber will fail...
  //To be safe, let's stop period measurements before we do anything else
  //The user can override this by setting skipStopPeriodicMeasurements to true
  if (skipStopPeriodicMeasurements == false)
    success &= stopPeriodicMeasurement(); // Delays for 500ms...

  char serialNumber[13]; // Serial number is 12 digits plus trailing NULL
  success &= getSerialNumber(serialNumber); // Read the serial number. Return false if the CRC check fails.
  if (success == false)
    return (false);

  #if SCD4x_ENABLE_DEBUGLOG
  if (_printDebug == true)
  {
    _debugPort->print(F("SCD40::begin: got serial number 0x"));
    _debugPort->println(serialNumber);
  }
  #endif // if SCD4x_ENABLE_DEBUGLOG

  if (autoCalibrate == true) // Must be done before periodic measurements are started
  {
    success &= setAutomaticSelfCalibrationEnabled(true);
    success &= (getAutomaticSelfCalibrationEnabled() == true);
  }
  else
  {
    success &= setAutomaticSelfCalibrationEnabled(false);
    success &= (getAutomaticSelfCalibrationEnabled() == false);
  }

  if (measBegin == true)
  {
    success &= startPeriodicMeasurement();
  }

  return (success);
}

//Calling this function with nothing sets the debug port to Serial
//You can also call it with other streams like Serial1, SerialUSB, etc.
void SCD4x::enableDebugging(Stream &debugPort)
{
  #if SCD4x_ENABLE_DEBUGLOG
  _debugPort = &debugPort;
  _printDebug = true;
  #endif // if SCD4x_ENABLE_DEBUGLOG
}

//Start periodic measurements. See 3.5.1
//signal update interval is 5 seconds.
bool SCD4x::startPeriodicMeasurement(void)
{
  if (periodicMeasurementsAreRunning)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("SCD4x::startPeriodicMeasurement: periodic measurements are already running"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (true); //Maybe this should be false?
  }

  bool success = sendCommand(SCD4x_COMMAND_START_PERIODIC_MEASUREMENT);
  if (success)
    periodicMeasurementsAreRunning = true;
  return (success);
}

//Stop periodic measurements. See 3.5.3
//Stop periodic measurement to change the sensor configuration or to save power.
//Note that the sensor will only respond to other commands after waiting 500 ms after issuing
//the stop_periodic_measurement command.
#ifdef USE_TEENSY3_I2C_LIB
bool SCD4x::stopPeriodicMeasurement(uint16_t delayMillis, i2c_t3 &wirePort)
#else
bool SCD4x::stopPeriodicMeasurement(uint16_t delayMillis, TwoWire &wirePort)
#endif
{
  uint8_t i2cResult;
  if (_i2cPort != NULL) // If the sensor has been begun (_i2cPort is not NULL) then _i2cPort is used
  {
    _i2cPort->beginTransmission(SCD4x_ADDRESS);
    _i2cPort->write(SCD4x_COMMAND_STOP_PERIODIC_MEASUREMENT >> 8);   //MSB
    _i2cPort->write(SCD4x_COMMAND_STOP_PERIODIC_MEASUREMENT & 0xFF); //LSB
    i2cResult = _i2cPort->endTransmission();
  }
  else
  {
    // If the sensor has not been begun (_i2cPort is NULL) then wirePort is used (which will default to Wire)
    wirePort.beginTransmission(SCD4x_ADDRESS);
    wirePort.write(SCD4x_COMMAND_STOP_PERIODIC_MEASUREMENT >> 8);   //MSB
    wirePort.write(SCD4x_COMMAND_STOP_PERIODIC_MEASUREMENT & 0xFF); //LSB
    i2cResult = wirePort.endTransmission();
  }

  if (i2cResult == 0)
  {
    periodicMeasurementsAreRunning = false;
    if (delayMillis > 0)
      delay(delayMillis);
    return(true);
  }

  #if SCD4x_ENABLE_DEBUGLOG
  if (_printDebug == true)
  {
    _debugPort->print(F("SCD4x::stopPeriodicMeasurement: I2C error: "));
    _debugPort->println(i2cResult);
  }
  #endif // if SCD4x_ENABLE_DEBUGLOG
  return (false);
}

//Get 9 bytes from SCD4x. See 3.5.2
//Updates global variables with floats
//Returns true if data is read successfully
//Read sensor output. The measurement data can only be read out once per signal update interval as the
//buffer is emptied upon read-out. If no data is available in the buffer, the sensor returns a NACK.
//To avoid a NACK response, the get_data_ready_status can be issued to check data status
//(see chapter 3.8.2 for further details).
bool SCD4x::readMeasurement(void)
{
  //Verify we have data from the sensor
  if (getDataReadyStatus() == false)
    return (false);

  scd4x_unsigned16Bytes_t tempCO2;
  tempCO2.unsigned16 = 0;
  scd4x_unsigned16Bytes_t  tempHumidity;
  tempHumidity.unsigned16 = 0;
  scd4x_unsigned16Bytes_t  tempTemperature;
  tempTemperature.unsigned16 = 0;

  _i2cPort->beginTransmission(SCD4x_ADDRESS);
  _i2cPort->write(SCD4x_COMMAND_READ_MEASUREMENT >> 8);   //MSB
  _i2cPort->write(SCD4x_COMMAND_READ_MEASUREMENT & 0xFF); //LSB
  if (_i2cPort->endTransmission() != 0)
    return (false); //Sensor did not ACK

  delay(1); //Datasheet specifies this

  #if SCD4x_ENABLE_DEBUGLOG
  uint8_t receivedBytes = (uint8_t)
  #endif // if SCD4x_ENABLE_DEBUGLOG
  _i2cPort->requestFrom((uint8_t)SCD4x_ADDRESS, (uint8_t)9);
  bool error = false;
  if (_i2cPort->available())
  {
    byte bytesToCrc[2];
    for (byte x = 0; x < 9; x++)
    {
      byte incoming = _i2cPort->read();

      switch (x)
      {
      case 0:
      case 1:
        tempCO2.bytes[x == 0 ? 1 : 0] = incoming; // Store the two CO2 bytes in little-endian format
        bytesToCrc[x] = incoming; // Calculate the CRC on the two CO2 bytes in the order they arrive
        break;
      case 3:
      case 4:
        tempTemperature.bytes[x == 3 ? 1 : 0] = incoming; // Store the two T bytes in little-endian format
        bytesToCrc[x % 3] = incoming; // Calculate the CRC on the two T bytes in the order they arrive
        break;
      case 6:
      case 7:
        tempHumidity.bytes[x == 6 ? 1 : 0] = incoming; // Store the two RH bytes in little-endian format
        bytesToCrc[x % 3] = incoming; // Calculate the CRC on the two RH bytes in the order they arrive
        break;
      default: // x == 2, 5, 8
        //Validate CRC
        uint8_t foundCrc = computeCRC8(bytesToCrc, 2); // Calculate what the CRC should be for these two bytes
        if (foundCrc != incoming) // Does this match the CRC byte from the sensor?
        {
          #if SCD4x_ENABLE_DEBUGLOG
          if (_printDebug == true)
          {
            _debugPort->print(F("SCD4x::readMeasurement: found CRC in byte "));
            _debugPort->print(x);
            _debugPort->print(F(", expected 0x"));
            _debugPort->print(foundCrc, HEX);
            _debugPort->print(F(", got 0x"));
            _debugPort->println(incoming, HEX);
          }
          #endif // if SCD4x_ENABLE_DEBUGLOG
          error = true;
        }
        break;
      }
    }
  }
  else
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->print(F("SCD4x::readMeasurement: no SCD4x data found from I2C, I2C claims we should receive "));
      _debugPort->print(receivedBytes);
      _debugPort->println(F(" bytes"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }

  if (error)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
      _debugPort->println(F("SCD4x::readMeasurement: encountered error reading SCD4x data."));
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }
  //Now copy the int16s into their associated floats
  co2 = (float)tempCO2.unsigned16;
  temperature = -45 + (((float)tempTemperature.unsigned16) * 175 / 65536);
  humidity = ((float)tempHumidity.unsigned16) * 100 / 65536;

  //Mark our global variables as fresh
  co2HasBeenReported = false;
  humidityHasBeenReported = false;
  temperatureHasBeenReported = false;

  return (true); //Success! New data available in globals.
}

//Returns the latest available CO2 level
//If the current level has already been reported, trigger a new read
uint16_t SCD4x::getCO2(void)
{
  if (co2HasBeenReported == true) //Trigger a new read
    readMeasurement();            //Pull in new co2, humidity, and temp into global vars

  co2HasBeenReported = true;

  return (uint16_t)co2; //Cut off decimal as co2 is 0 to 10,000
}

//Returns the latest available humidity
//If the current level has already been reported, trigger a new read
float SCD4x::getHumidity(void)
{
  if (humidityHasBeenReported == true) //Trigger a new read
    readMeasurement();                 //Pull in new co2, humidity, and temp into global vars

  humidityHasBeenReported = true;

  return humidity;
}

//Returns the latest available temperature
//If the current level has already been reported, trigger a new read
float SCD4x::getTemperature(void)
{
  if (temperatureHasBeenReported == true) //Trigger a new read
    readMeasurement();                    //Pull in new co2, humidity, and temp into global vars

  temperatureHasBeenReported = true;

  return temperature;
}

//Set the temperature offset (C). See 3.6.1
//Max command duration: 1ms
//The user can set delayMillis to zero f they want the function to return immediately.
//The temperature offset has no influence on the SCD4x CO2 accuracy.
//Setting the temperature offset of the SCD4x inside the customer device correctly allows the user
//to leverage the RH and T output signal.
bool SCD4x::setTemperatureOffset(float offset, uint16_t delayMillis)
{
  if (periodicMeasurementsAreRunning)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("SCD4x::setTemperatureOffset: periodic measurements are running. Aborting"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }

  if (offset < 0)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("SCD4x::setTemperatureOffset: offset must be >= 0C"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }
  if (offset >= 175)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("SCD4x::setTemperatureOffset: offset must be < 175C"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }
  uint16_t offsetWord = (uint16_t)(offset * 65536 / 175); // Toffset [°C] * 2^16 / 175
  bool success = sendCommand(SCD4x_COMMAND_SET_TEMPERATURE_OFFSET, offsetWord);
  if (delayMillis > 0)
    delay(delayMillis);
  return (success);
}

//Get the temperature offset. See 3.6.2
float SCD4x::getTemperatureOffset(void)
{
 if (periodicMeasurementsAreRunning)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("SCD4x::getTemperatureOffset: periodic measurements are running. Returning 0.0"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (0.0);
  }

  float offset;
  #if SCD4x_ENABLE_DEBUGLOG
  bool success = 
  #endif // if SCD4x_ENABLE_DEBUGLOG
  getTemperatureOffset(&offset);
  #if SCD4x_ENABLE_DEBUGLOG
  if ((success == false) && (_printDebug == true))
  {
    _debugPort->println(F("SCD4x::getTemperatureOffset: failed to read offset. Returning 0.0"));
  }
  #endif // if SCD4x_ENABLE_DEBUGLOG
  return (offset);
}

//Get the temperature offset. See 3.6.2
bool SCD4x::getTemperatureOffset(float *offset)
{
  if (periodicMeasurementsAreRunning)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("SCD4x::getTemperatureOffset: periodic measurements are running. Aborting"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }

  uint16_t offsetWord = 0; // offset will be zero if readRegister fails
  bool success = readRegister(SCD4x_COMMAND_GET_TEMPERATURE_OFFSET, &offsetWord, 1);
  *offset = ((float)offsetWord) * 175.0 / 65535.0;
  return (success);
}

//Set the sensor altitude (metres above sea level). See 3.6.3
//Max command duration: 1ms
//The user can set delayMillis to zero if they want the function to return immediately.
//Reading and writing of the sensor altitude must be done while the SCD4x is in idle mode.
//Typically, the sensor altitude is set once after device installation. To save the setting to the EEPROM,
//the persist setting (see chapter 3.9.1) command must be issued.
//Per default, the sensor altitude is set to 0 meter above sea-level.
bool SCD4x::setSensorAltitude(uint16_t altitude, uint16_t delayMillis)
{
  if (periodicMeasurementsAreRunning)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("SCD4x::setSensorAltitude: periodic measurements are running. Aborting"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }

  bool success = sendCommand(SCD4x_COMMAND_SET_SENSOR_ALTITUDE, altitude);
  if (delayMillis > 0)
    delay(delayMillis);
  return (success);
}

//Get the sensor altitude. See 3.6.4
uint16_t SCD4x::getSensorAltitude(void)
{
  if (periodicMeasurementsAreRunning)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("SCD4x::getSensorAltitude: periodic measurements are running. Returning 0"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (0);
  }

  uint16_t altitude = 0;
  #if SCD4x_ENABLE_DEBUGLOG
  bool success = 
  #endif // if SCD4x_ENABLE_DEBUGLOG
  getSensorAltitude(&altitude);
  #if SCD4x_ENABLE_DEBUGLOG
  if ((success == false) && (_printDebug == true))
  {
    _debugPort->println(F("SCD4x::getSensorAltitude: failed to read altitude. Returning 0"));
  }
  #endif // if SCD4x_ENABLE_DEBUGLOG
  return (altitude);
}

//Get the sensor altitude. See 3.6.4
bool SCD4x::getSensorAltitude(uint16_t *altitude)
{
  if (periodicMeasurementsAreRunning)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("SCD4x::getSensorAltitude: periodic measurements are running. Aborting"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }

  return (readRegister(SCD4x_COMMAND_GET_SENSOR_ALTITUDE, altitude, 1));
}

//Set the ambient pressure (Pa). See 3.6.5
//Max command duration: 1ms
//The user can set delayMillis to zero if they want the function to return immediately.
//The set_ambient_pressure command can be sent during periodic measurements to enable continuous pressure compensation.
//setAmbientPressure overrides setSensorAltitude
bool SCD4x::setAmbientPressure(float pressure, uint16_t delayMillis)
{
  if (pressure < 0)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("SCD4x::setAmbientPressure: pressure must be >= 0 Pa"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }
  if (pressure > 6553500)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("SCD4x::setAmbientPressure: pressure must be <= 6553500 Pa"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }
  uint16_t pressureWord = (uint16_t)(pressure / 100);
  bool success = sendCommand(SCD4x_COMMAND_SET_AMBIENT_PRESSURE, pressureWord);
  if (delayMillis > 0)
    delay(delayMillis);
  return (success);
}

//Perform forced recalibration. See 3.7.1
float SCD4x::performForcedRecalibration(uint16_t concentration)
{
  if (periodicMeasurementsAreRunning)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("SCD4x::performForcedRecalibration: periodic measurements are running. Returning 0.0"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (0.0);
  }

  float correction = 0.0;
  #if SCD4x_ENABLE_DEBUGLOG
  bool success = 
  #endif // if SCD4x_ENABLE_DEBUGLOG
  performForcedRecalibration(concentration, &correction);
  #if SCD4x_ENABLE_DEBUGLOG
  if ((success == false) && (_printDebug == true))
  {
    _debugPort->println(F("SCD4x::performForcedRecalibration: FRC failed"));
  }
  #endif // if SCD4x_ENABLE_DEBUGLOG
  return (correction);
}

//Perform forced recalibration. See 3.7.1
//To successfully conduct an accurate forced recalibration, the following steps need to be carried out:
//1. Operate the SCD4x in the operation mode later used in normal sensor operation (periodic measurement,
//   low power periodic measurement or single shot) for > 3 minutes in an environment with homogenous and
//   constant CO2 concentration.
//2. Issue stop_periodic_measurement. Wait 500 ms for the stop command to complete.
//3. Subsequently issue the perform_forced_recalibration command and optionally read out the FRC correction
//   (i.e. the magnitude of the correction) after waiting for 400 ms for the command to complete.
//A return value of 0xffff indicates that the forced recalibration has failed.
bool SCD4x::performForcedRecalibration(uint16_t concentration, float *correction)
{
  if (periodicMeasurementsAreRunning)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("SCD4x::performForcedRecalibration: periodic measurements are running. Aborting"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }

  uint16_t correctionWord;

  bool success = sendCommand(SCD4x_COMMAND_PERFORM_FORCED_CALIBRATION, concentration);

  if (success == false)
    return (false);

  delay(400); //Datasheet specifies this

  #if SCD4x_ENABLE_DEBUGLOG
  uint8_t receivedBytes = (uint8_t)
  #endif // if SCD4x_ENABLE_DEBUGLOG
  _i2cPort->requestFrom((uint8_t)SCD4x_ADDRESS, (uint8_t)3);
  bool error = false;
  if (_i2cPort->available())
  {
    byte bytesToCrc[2];
    bytesToCrc[0] = _i2cPort->read();
    correctionWord = ((uint16_t)bytesToCrc[0]) << 8;
    bytesToCrc[1] = _i2cPort->read();
    correctionWord |= (uint16_t)bytesToCrc[1];
    byte incomingCrc = _i2cPort->read();
    uint8_t foundCrc = computeCRC8(bytesToCrc, 2);
    if (foundCrc != incomingCrc)
    {
      #if SCD4x_ENABLE_DEBUGLOG
      if (_printDebug == true)
      {
        _debugPort->print(F("SCD4x::performForcedRecalibration: CRC error. Expected 0x"));
        _debugPort->print(foundCrc, HEX);
        _debugPort->print(F(", got 0x"));
        _debugPort->println(incomingCrc, HEX);
      }
      #endif // if SCD4x_ENABLE_DEBUGLOG
      error = true;
    }
  }
  else
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->print(F("SCD4x::performForcedRecalibration: no SCD4x data found from I2C, I2C claims we should receive "));
      _debugPort->print(receivedBytes);
      _debugPort->println(F(" bytes"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }

  if (error)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
      _debugPort->println(F("SCD4x::performForcedRecalibration: encountered error reading SCD4x data."));
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }

  *correction = ((float)correctionWord) - 32768; // FRC correction [ppm CO2] = word[0] – 0x8000

  if (correctionWord == 0xffff) //A return value of 0xffff indicates that the forced recalibration has failed
    return (false);
  
  return (true);
}

//Enable/disable automatic self calibration. See 3.7.2
//Set the current state (enabled / disabled) of the automatic self-calibration. By default, ASC is enabled.
//To save the setting to the EEPROM, the persist_setting (see chapter 3.9.1) command must be issued.
bool SCD4x::setAutomaticSelfCalibrationEnabled(bool enabled, uint16_t delayMillis)
{
  if (periodicMeasurementsAreRunning)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("SCD4x::setAutomaticSelfCalibrationEnabled: periodic measurements are running. Aborting"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }

  uint16_t enabledWord = enabled == true ? 0x0001 : 0x0000;
  bool success = sendCommand(SCD4x_COMMAND_SET_AUTOMATIC_SELF_CALIBRATION_ENABLED, enabledWord);
  if (delayMillis > 0)
    delay(delayMillis);
  return (success);
}

//Check if automatic self calibration is enabled. See 3.7.3
bool SCD4x::getAutomaticSelfCalibrationEnabled(void)
{
  if (periodicMeasurementsAreRunning)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("SCD4x::getAutomaticSelfCalibrationEnabled: periodic measurements are running. Aborting"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }

  uint16_t enabled;
  bool success = getAutomaticSelfCalibrationEnabled(&enabled);
  if (success == false)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("getAutomaticSelfCalibrationEnabled: failed to get self calibration status. Returning false"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }
  return (enabled == 0x0001);
}

//Check if automatic self calibration is enabled. See 3.7.3
bool SCD4x::getAutomaticSelfCalibrationEnabled(uint16_t *enabled)
{
  if (periodicMeasurementsAreRunning)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("SCD4x::getAutomaticSelfCalibrationEnabled: periodic measurements are running. Aborting"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }

  return (readRegister(SCD4x_COMMAND_GET_AUTOMATIC_SELF_CALIBRATION_ENABLED, enabled, 1));
}

//Start low power periodic measurements. See 3.8.1
//Signal update interval will be 30 seconds instead of 5
bool SCD4x::startLowPowerPeriodicMeasurement(void)
{
  if (periodicMeasurementsAreRunning)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("SCD4x::startLowPowerPeriodicMeasurement: periodic measurements are running. Aborting"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }

  bool success = sendCommand(SCD4x_COMMAND_START_LOW_POWER_PERIODIC_MEASUREMENT);
  if (success)
    periodicMeasurementsAreRunning = true;
  return (success);
}

//Returns true when data is available. See 3.8.2
bool SCD4x::getDataReadyStatus(void)
{
  uint16_t response;
  bool success = readRegister(SCD4x_COMMAND_GET_DATA_READY_STATUS, &response, 1);

  if (success == false)
    return (false);

  //If the least significant 11 bits of word[0] are 0 → data not ready
  //else → data ready for read-out
  if ((response & 0x07ff) == 0x0000)
    return (false);
  return (true);
}

//Persist settings: copy settings (e.g. temperature offset) from RAM to EEPROM. See 3.9.1
//Configuration settings such as the temperature offset, sensor altitude and the ASC enabled/disabled parameter
//are by default stored in the volatile memory (RAM) only and will be lost after a power-cycle. The persist_settings
//command stores the current configuration in the EEPROM of the SCD4x, making them persistent across power-cycling.
//To avoid unnecessary wear of the EEPROM, the persist_settings command should only be sent when persistence is required
//and if actual changes to the configuration have been made. The EEPROM is guaranteed to endure at least 2000 write
//cycles before failure.
bool SCD4x::persistSettings(uint16_t delayMillis)
{
  if (periodicMeasurementsAreRunning)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("SCD4x::persistSettings: periodic measurements are running. Aborting"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }

  bool success = sendCommand(SCD4x_COMMAND_PERSIST_SETTINGS);
  if (delayMillis > 0)
    delay(delayMillis);
  return (success);
}

//Get 9 bytes from SCD4x. Convert 48-bit serial number to ASCII chars. See 3.9.2
//Returns true if serial number is read successfully
//Reading out the serial number can be used to identify the chip and to verify the presence of the sensor.
bool SCD4x::getSerialNumber(char *serialNumber)
{
  if (periodicMeasurementsAreRunning)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("SCD4x::getSerialNumber: periodic measurements are running. Aborting"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }

  _i2cPort->beginTransmission(SCD4x_ADDRESS);
  _i2cPort->write(SCD4x_COMMAND_GET_SERIAL_NUMBER >> 8);   //MSB
  _i2cPort->write(SCD4x_COMMAND_GET_SERIAL_NUMBER & 0xFF); //LSB
  if (_i2cPort->endTransmission() != 0)
    return (false); //Sensor did not ACK

  delay(1); //Datasheet specifies this

  #if SCD4x_ENABLE_DEBUGLOG
  uint8_t receivedBytes = (uint8_t)
  #endif // if SCD4x_ENABLE_DEBUGLOG
  _i2cPort->requestFrom((uint8_t)SCD4x_ADDRESS, (uint8_t)9);
  bool error = false;
  if (_i2cPort->available())
  {
    byte bytesToCrc[2];
    int digit = 0;
    for (byte x = 0; x < 9; x++)
    {
      byte incoming = _i2cPort->read();

      switch (x)
      {
      case 0: // The serial number arrives as: two bytes, CRC, two bytes, CRC, two bytes, CRC
      case 1:
      case 3:
      case 4:
      case 6:
      case 7:
        serialNumber[digit++] = convertHexToASCII(incoming >> 4); // Convert each nibble to ASCII
        serialNumber[digit++] = convertHexToASCII(incoming & 0x0F);
        bytesToCrc[x % 3] = incoming;
        break;
      default: // x == 2, 5, 8
        //Validate CRC
        uint8_t foundCrc = computeCRC8(bytesToCrc, 2); // Calculate what the CRC should be for these two bytes
        if (foundCrc != incoming) // Does this match the CRC byte from the sensor?
        {
          #if SCD4x_ENABLE_DEBUGLOG
          if (_printDebug == true)
          {
            _debugPort->print(F("SCD4x::readSerialNumber: found CRC in byte "));
            _debugPort->print(x);
            _debugPort->print(F(", expected 0x"));
            _debugPort->print(foundCrc, HEX);
            _debugPort->print(F(", got 0x"));
            _debugPort->println(incoming, HEX);
          }
          #endif // if SCD4x_ENABLE_DEBUGLOG
          error = true;
        }
        break;
      }
      serialNumber[digit] = 0; // NULL-terminate the string
    }
  }
  else
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->print(F("SCD4x::readSerialNumber: no SCD4x data found from I2C, I2C claims we should receive "));
      _debugPort->print(receivedBytes);
      _debugPort->println(F(" bytes"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }

  if (error)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
      _debugPort->println(F("SCD4x::readSerialNumber: encountered error reading SCD4x data."));
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }

  return (true); //Success!
}

//PRIVATE: Convert serial number digit to ASCII
char SCD4x::convertHexToASCII(uint8_t digit)
{
  if (digit <= 9)
    return (char(digit + 0x30));
  else
    return (char(digit + 0x41 - 10)); // Use upper case for A-F
}

//Perform self test. Takes 10 seconds to complete. See 3.9.3
//The perform_self_test feature can be used as an end-of-line test to check sensor functionality
//and the customer power supply to the sensor.
bool SCD4x::performSelfTest(void)
{
  if (periodicMeasurementsAreRunning)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("SCD4x::performSelfTest: periodic measurements are running. Aborting"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }

  uint16_t response;

  #if SCD4x_ENABLE_DEBUGLOG
  if (_printDebug == true)
    _debugPort->println(F("SCD4x::performSelfTest: delaying for 10 seconds..."));
  #endif // if SCD4x_ENABLE_DEBUGLOG

  bool success = readRegister(SCD4x_COMMAND_PERFORM_SELF_TEST, &response, 10000);

  #if SCD4x_ENABLE_DEBUGLOG
  if (_printDebug == true)
  {
    _debugPort->print(F("SCD4x::performSelfTest: sensor response is 0x"));
    if (response < 0x1000) _debugPort->print(F("0"));
    if (response < 0x100) _debugPort->print(F("0"));
    if (response < 0x10) _debugPort->print(F("0"));
    _debugPort->println(response, HEX);
  }
  #endif // if SCD4x_ENABLE_DEBUGLOG

  return (success && (response == 0x0000)); // word[0] = 0 → no malfunction detected
}

//Peform factory reset. See 3.9.4
//The perform_factory_reset command resets all configuration settings stored in the EEPROM
//and erases the FRC and ASC algorithm history.
bool SCD4x::performFactoryReset(uint16_t delayMillis)
{
  if (periodicMeasurementsAreRunning)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("SCD4x::performFactoryReset: periodic measurements are running. Aborting"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }

  bool success = sendCommand(SCD4x_COMMAND_PERFORM_FACTORY_RESET);
  if (delayMillis > 0)
    delay(delayMillis);
  return (success);
}

//Reinit. See 3.9.5
//The reinit command reinitializes the sensor by reloading user settings from EEPROM.
//Before sending the reinit command, the stop measurement command must be issued.
//If the reinit command does not trigger the desired re-initialization,
//a power-cycle should be applied to the SCD4x.
bool SCD4x::reInit(uint16_t delayMillis)
{
  if (periodicMeasurementsAreRunning)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("SCD4x::reInit: periodic measurements are running. Aborting"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }

  bool success = sendCommand(SCD4x_COMMAND_REINIT);
  if (delayMillis > 0)
    delay(delayMillis);
  return (success);
}

//Low Power Single Shot. See 3.10.1
//In addition to periodic measurement modes, the SCD41 features a single shot measurement mode,
//i.e. allows for on-demand measurements.
//The typical communication sequence is as follows:
//1. The sensor is powered up.
//2. The I2C master sends a single shot command and waits for the indicated max. command duration time.
//3. The I2C master reads out data with the read measurement sequence (chapter 3.5.2).
//4. Steps 2-3 are repeated as required by the application.
bool SCD4x::measureSingleShot(void)
{
  if (_sensorType != SCD4x_SENSOR_SCD41)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("SCD4x::measureSingleShot: _sensorType is not SCD4x_SENSOR_SCD41"));
      _debugPort->println(F("SCD41's need to be instantiated using: SCD4x mySensor(SCD4x_SENSOR_SCD41)"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return(false);
  }

  if (periodicMeasurementsAreRunning)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("SCD4x::measureSingleShot: periodic measurements are running. Aborting"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }

  bool success = sendCommand(SCD4x_COMMAND_MEASURE_SINGLE_SHOT);

  #if SCD4x_ENABLE_DEBUGLOG
  if (success && (_printDebug == true))
  {
    _debugPort->println(F("SCD4x::measureSingleShot: your data will be ready in five seconds"));
  }
  #endif // if SCD4x_ENABLE_DEBUGLOG

  return (success);
}

//On-demand measurement of relative humidity and temperature only.
//The sensor output is read using the read_measurement command (chapter 3.5.2).
//CO2 output is returned as 0 ppm.
bool SCD4x::measureSingleShotRHTOnly(void)
{
  if (_sensorType != SCD4x_SENSOR_SCD41)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("SCD4x::measureSingleShotRHTOnly: _sensorType is not SCD4x_SENSOR_SCD41"));
      _debugPort->println(F("SCD41's need to be instantiated using: SCD4x mySensor(SCD4x_SENSOR_SCD41)"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return(false);
  }

  if (periodicMeasurementsAreRunning)
  {
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->println(F("SCD4x::measureSingleShotRHTOnly: periodic measurements are running. Aborting"));
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
    return (false);
  }

  bool success = sendCommand(SCD4x_COMMAND_MEASURE_SINGLE_SHOT_RHT_ONLY);

  #if SCD4x_ENABLE_DEBUGLOG
  if (success && (_printDebug == true))
  {
    _debugPort->println(F("SCD4x::measureSingleShot: your data will be ready in 50ms"));
  }
  #endif // if SCD4x_ENABLE_DEBUGLOG

  return (success);
}

//Sends a command along with arguments and CRC
bool SCD4x::sendCommand(uint16_t command, uint16_t arguments)
{
  uint8_t data[2];
  data[0] = arguments >> 8;
  data[1] = arguments & 0xFF;
  uint8_t crc = computeCRC8(data, 2); //Calc CRC on the arguments only, not the command

  _i2cPort->beginTransmission(SCD4x_ADDRESS);
  _i2cPort->write(command >> 8);     //MSB
  _i2cPort->write(command & 0xFF);   //LSB
  _i2cPort->write(arguments >> 8);   //MSB
  _i2cPort->write(arguments & 0xFF); //LSB
  _i2cPort->write(crc);
  if (_i2cPort->endTransmission() != 0)
    return (false); //Sensor did not ACK

  return (true);
}

//Sends just a command, no arguments, no CRC
bool SCD4x::sendCommand(uint16_t command)
{
  _i2cPort->beginTransmission(SCD4x_ADDRESS);
  _i2cPort->write(command >> 8);   //MSB
  _i2cPort->write(command & 0xFF); //LSB
  if (_i2cPort->endTransmission() != 0)
    return (false); //Sensor did not ACK

  return (true);
}

//Gets two bytes from SCD4x plus CRC.
//Returns true if endTransmission returns zero _and_ the CRC check is valid
bool SCD4x::readRegister(uint16_t registerAddress, uint16_t *response, uint16_t delayMillis)
{
  _i2cPort->beginTransmission(SCD4x_ADDRESS);
  _i2cPort->write(registerAddress >> 8);   //MSB
  _i2cPort->write(registerAddress & 0xFF); //LSB
  if (_i2cPort->endTransmission() != 0)
    return (false); //Sensor did not ACK

  delay(delayMillis);

  _i2cPort->requestFrom((uint8_t)SCD4x_ADDRESS, (uint8_t)3); // Request data and CRC
  if (_i2cPort->available())
  {
    uint8_t data[2];
    data[0] = _i2cPort->read();
    data[1] = _i2cPort->read();
    uint8_t crc = _i2cPort->read();
    *response = (uint16_t)data[0] << 8 | data[1];
    uint8_t expectedCRC = computeCRC8(data, 2);
    if (crc == expectedCRC) // Return true if CRC check is OK
      return (true);
    #if SCD4x_ENABLE_DEBUGLOG
    if (_printDebug == true)
    {
      _debugPort->print(F("SCD4x::readRegister: CRC fail: expected 0x"));
      _debugPort->print(expectedCRC, HEX);
      _debugPort->print(F(", got 0x"));
      _debugPort->println(crc, HEX);
    }
    #endif // if SCD4x_ENABLE_DEBUGLOG
  }
  return (false);
}

//Given an array and a number of bytes, this calculate CRC8 for those bytes
//CRC is only calc'd on the data portion (two bytes) of the four bytes being sent
//From: http://www.sunshine2k.de/articles/coding/crc/understanding_crc.html
//Tested with: http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
//x^8+x^5+x^4+1 = 0x31
uint8_t SCD4x::computeCRC8(uint8_t data[], uint8_t len)
{
  uint8_t crc = 0xFF; //Init with 0xFF

  for (uint8_t x = 0; x < len; x++)
  {
    crc ^= data[x]; // XOR-in the next input byte

    for (uint8_t i = 0; i < 8; i++)
    {
      if ((crc & 0x80) != 0)
        crc = (uint8_t)((crc << 1) ^ 0x31);
      else
        crc <<= 1;
    }
  }

  return crc; //No output reflection
}
