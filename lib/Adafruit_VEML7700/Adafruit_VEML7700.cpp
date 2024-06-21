/*!
 *  @file Adafruit_VEML7700.cpp
 *
 *  @mainpage Adafruit VEML7700 I2C Lux Sensor
 *
 *  @section intro_sec Introduction
 *
 * 	I2C Driver for the VEML7700 I2C Lux sensor
 *
 * 	This is a library for the Adafruit VEML7700 breakout:
 * 	http://www.adafruit.com/
 *
 * 	Adafruit invests time and resources providing this open source code,
 *  please support Adafruit and open-source hardware by purchasing products from
 * 	Adafruit!
 *
 *  @section author Author
 *
 *  Limor Fried (Adafruit Industries)
 *
 * 	@section license License
 *
 * 	BSD (see license.txt)
 *
 * 	@section  HISTORY
 *
 *     v1.0 - First release
 */

#include "Adafruit_VEML7700.h"

/*!
 *    @brief  Instantiates a new VEML7700 class
 */
Adafruit_VEML7700::Adafruit_VEML7700(void) {}

/*!
 *    @brief  Sets up the hardware for talking to the VEML7700
 *    @param  theWire An optional pointer to an I2C interface
 *    @return True if initialization was successful, otherwise false.
 */
bool Adafruit_VEML7700::begin(int8_t i2cAddr, TwoWire *theWire) {
  i2c_dev = new Adafruit_I2CDevice(i2cAddr, theWire);

  if (!i2c_dev->begin()) {
    return false;
  }

  ALS_Config =
      new Adafruit_I2CRegister(i2c_dev, VEML7700_ALS_CONFIG, 2, LSBFIRST);
  ALS_HighThreshold = new Adafruit_I2CRegister(
      i2c_dev, VEML7700_ALS_THREHOLD_HIGH, 2, LSBFIRST);
  ALS_LowThreshold =
      new Adafruit_I2CRegister(i2c_dev, VEML7700_ALS_THREHOLD_LOW, 2, LSBFIRST);
  Power_Saving =
      new Adafruit_I2CRegister(i2c_dev, VEML7700_ALS_POWER_SAVE, 2, LSBFIRST);
  ALS_Data = new Adafruit_I2CRegister(i2c_dev, VEML7700_ALS_DATA, 2, LSBFIRST);
  White_Data =
      new Adafruit_I2CRegister(i2c_dev, VEML7700_WHITE_DATA, 2, LSBFIRST);
  Interrupt_Status =
      new Adafruit_I2CRegister(i2c_dev, VEML7700_INTERRUPTSTATUS, 2, LSBFIRST);

  ALS_Shutdown =
      new Adafruit_I2CRegisterBits(ALS_Config, 1, 0); // # bits, bit_shift
  ALS_Interrupt_Enable = new Adafruit_I2CRegisterBits(ALS_Config, 1, 1);
  ALS_Persistence = new Adafruit_I2CRegisterBits(ALS_Config, 2, 4);
  ALS_Integration_Time = new Adafruit_I2CRegisterBits(ALS_Config, 4, 6);
  ALS_Gain = new Adafruit_I2CRegisterBits(ALS_Config, 2, 11);
  PowerSave_Enable = new Adafruit_I2CRegisterBits(Power_Saving, 1, 0);
  PowerSave_Mode = new Adafruit_I2CRegisterBits(Power_Saving, 2, 1);

  enable(false);
  interruptEnable(false);
  setPersistence(VEML7700_PERS_1);
  setGain(VEML7700_GAIN_1_8);
  setIntegrationTime(VEML7700_IT_100MS);
  powerSaveEnable(false);
  enable(true);

  lastRead = millis();

  return true;
}

/*!
 *    @brief Read the calibrated lux value. See app note lux table on page 5
 *    @param method Lux comptation method to use. One of
 *    @returns Floating point Lux data
 */
float Adafruit_VEML7700::readLux(luxMethod method) {
  bool wait = true;
  switch (method) {
  case VEML_LUX_NORMAL_NOWAIT:
    wait = false;
    VEML7700_FALLTHROUGH
  case VEML_LUX_NORMAL:
    return computeLux(readALS(wait));
  case VEML_LUX_CORRECTED_NOWAIT:
    wait = false;
    VEML7700_FALLTHROUGH
  case VEML_LUX_CORRECTED:
    return computeLux(readALS(wait), true);
  case VEML_LUX_AUTO:
    return autoLux();
  default:
    return -1;
  }
}

/*!
 *    @brief Read the raw ALS data
 *    @param wait If false (default), read out measurement with no delay. If
 * true, wait as need based on integration time before reading out measurement
 * results.
 *    @returns 16-bit data value from the ALS register
 */
uint16_t Adafruit_VEML7700::readALS(bool wait) {
  if (wait)
    readWait();
  lastRead = millis();
  return ALS_Data->read();
}

/*!
 *    @brief Read the raw white light data
 *    @param wait If false (default), read out measurement with no delay. If
 * true, wait as need based on integration time before reading out measurement
 * results.
 *    @returns 16-bit data value from the WHITE register
 */
uint16_t Adafruit_VEML7700::readWhite(bool wait) {
  if (wait)
    readWait();
  lastRead = millis();
  return White_Data->read();
}

/*!
 *    @brief Enable or disable the sensor
 *    @param enable The flag to enable/disable
 */
void Adafruit_VEML7700::enable(bool enable) {
  ALS_Shutdown->write(!enable);
  // From app note:
  //   '''
  //   When activating the sensor, set bit 0 of the command register
  //   to “0” with a wait time of 2.5 ms before the first measurement
  //   is needed, allowing for the correct start of the signal
  //   processor and oscillator.
  //   '''
  if (enable)
    delay(5); // doubling 2.5ms spec to be sure
}

/*!
 *    @brief Ask if the interrupt is enabled
 *    @returns True if enabled, false otherwise
 */
bool Adafruit_VEML7700::enabled(void) { return !ALS_Shutdown->read(); }

/*!
 *    @brief Enable or disable the interrupt
 *    @param enable The flag to enable/disable
 */
void Adafruit_VEML7700::interruptEnable(bool enable) {
  ALS_Interrupt_Enable->write(enable);
}

/*!
 *    @brief Ask if the interrupt is enabled
 *    @returns True if enabled, false otherwise
 */
bool Adafruit_VEML7700::interruptEnabled(void) {
  return ALS_Interrupt_Enable->read();
}

/*!
 *    @brief Set the ALS IRQ persistence setting
 *    @param pers Persistence constant, can be VEML7700_PERS_1, VEML7700_PERS_2,
 *    VEML7700_PERS_4 or VEML7700_PERS_8
 */
void Adafruit_VEML7700::setPersistence(uint8_t pers) {
  ALS_Persistence->write(pers);
}

/*!
 *    @brief Get the ALS IRQ persistence setting
 *    @returns Persistence constant, can be VEML7700_PERS_1, VEML7700_PERS_2,
 *    VEML7700_PERS_4 or VEML7700_PERS_8
 */
uint8_t Adafruit_VEML7700::getPersistence(void) {
  return ALS_Persistence->read();
}

/*!
 *    @brief Set ALS integration time
 *    @param it Can be VEML7700_IT_100MS, VEML7700_IT_200MS, VEML7700_IT_400MS,
 *    VEML7700_IT_800MS, VEML7700_IT_50MS or VEML7700_IT_25MS
 *    @param wait Waits to insure old integration time cycle has completed. This
 * is a blocking delay. If disabled by passing false, user code must insure a
 * new reading is not done before old integration cycle completes.
 */
void Adafruit_VEML7700::setIntegrationTime(uint8_t it, bool wait) {
  // save current integration time
  int flushDelay = wait ? getIntegrationTimeValue() : 0;
  // set new integration time
  ALS_Integration_Time->write(it);
  // pause old integration time to insure sensor cycle has completed
  delay(flushDelay);
  // reset counter
  lastRead = millis();
}

/*!
 *    @brief Get ALS integration time setting
 *    @returns IT index, can be VEML7700_IT_100MS, VEML7700_IT_200MS,
 * VEML7700_IT_400MS, VEML7700_IT_800MS, VEML7700_IT_50MS or VEML7700_IT_25MS
 */
uint8_t Adafruit_VEML7700::getIntegrationTime(void) {
  return ALS_Integration_Time->read();
}

/*!
 *    @brief Get ALS integration time value
 *    @returns ALS integration time in milliseconds
 */
int Adafruit_VEML7700::getIntegrationTimeValue(void) {
  switch (getIntegrationTime()) {
  case VEML7700_IT_25MS:
    return 25;
  case VEML7700_IT_50MS:
    return 50;
  case VEML7700_IT_100MS:
    return 100;
  case VEML7700_IT_200MS:
    return 200;
  case VEML7700_IT_400MS:
    return 400;
  case VEML7700_IT_800MS:
    return 800;
  default:
    return -1;
  }
}

/*!
 *    @brief Set ALS gain
 *    @param gain Can be VEML7700_GAIN_1, VEML7700_GAIN_2, VEML7700_GAIN_1_8 or
 * VEML7700_GAIN_1_4
 */
void Adafruit_VEML7700::setGain(uint8_t gain) {
  ALS_Gain->write(gain);
  lastRead = millis(); // reset
}

/*!
 *    @brief Get ALS gain setting
 *    @returns Gain index, can be VEML7700_GAIN_1, VEML7700_GAIN_2,
 * VEML7700_GAIN_1_8 or VEML7700_GAIN_1_4
 */
uint8_t Adafruit_VEML7700::getGain(void) { return ALS_Gain->read(); }

/*!
 *    @brief Get ALS gain value
 *    @returns Actual gain value as float
 */
float Adafruit_VEML7700::getGainValue(void) {
  switch (getGain()) {
  case VEML7700_GAIN_1_8:
    return 0.125;
  case VEML7700_GAIN_1_4:
    return 0.25;
  case VEML7700_GAIN_1:
    return 1;
  case VEML7700_GAIN_2:
    return 2;
  default:
    return -1;
  }
}

/*!
 *    @brief Enable power save mode
 *    @param enable True if power save should be enabled
 */
void Adafruit_VEML7700::powerSaveEnable(bool enable) {
  PowerSave_Enable->write(enable);
}

/*!
 *    @brief Check if power save mode is enabled
 *    @returns True if power save is enabled
 */
bool Adafruit_VEML7700::powerSaveEnabled(void) {
  return PowerSave_Enable->read();
}

/*!
 *    @brief Assign the power save register data
 *    @param mode The 16-bit data to write to VEML7700_ALS_POWER_SAVE
 */
void Adafruit_VEML7700::setPowerSaveMode(uint8_t mode) {
  PowerSave_Mode->write(mode);
}

/*!
 *    @brief  Retrieve the power save register data
 *    @return 16-bit data from VEML7700_ALS_POWER_SAVE
 */
uint8_t Adafruit_VEML7700::getPowerSaveMode(void) {
  return PowerSave_Mode->read();
}

/*!
 *    @brief Assign the low threshold register data
 *    @param value The 16-bit data to write to VEML7700_ALS_THREHOLD_LOW
 */
void Adafruit_VEML7700::setLowThreshold(uint16_t value) {
  ALS_LowThreshold->write(value);
}

/*!
 *    @brief  Retrieve the low threshold register data
 *    @return 16-bit data from VEML7700_ALS_THREHOLD_LOW
 */
uint16_t Adafruit_VEML7700::getLowThreshold(void) {
  return ALS_LowThreshold->read();
}

/*!
 *    @brief Assign the high threshold register data
 *    @param value The 16-bit data to write to VEML7700_ALS_THREHOLD_HIGH
 */
void Adafruit_VEML7700::setHighThreshold(uint16_t value) {
  ALS_HighThreshold->write(value);
}

/*!
 *    @brief  Retrieve the high threshold register data
 *    @return 16-bit data from VEML7700_ALS_THREHOLD_HIGH
 */
uint16_t Adafruit_VEML7700::getHighThreshold(void) {
  return ALS_HighThreshold->read();
}

/*!
 *    @brief  Retrieve the interrupt status register data
 *    @return 16-bit data from VEML7700_INTERRUPTSTATUS
 */
uint16_t Adafruit_VEML7700::interruptStatus(void) {
  return Interrupt_Status->read();
}

/*!
 *    @brief Determines resolution for current gain and integration time
 * settings.
 */
float Adafruit_VEML7700::getResolution(void) {
  return MAX_RES * (IT_MAX / getIntegrationTimeValue()) *
         (GAIN_MAX / getGainValue());
}

/*!
 *    @brief Copmute lux from ALS reading.
 *    @param rawALS raw ALS register value
 *    @param corrected if true, apply non-linear correction
 *    @return lux value
 */
float Adafruit_VEML7700::computeLux(uint16_t rawALS, bool corrected) {
  float lux = getResolution() * rawALS;
  if (corrected)
    lux = (((6.0135e-13 * lux - 9.3924e-9) * lux + 8.1488e-5) * lux + 1.0023) *
          lux;
  return lux;
}

void Adafruit_VEML7700::readWait(void) {
  // From app note:
  //   '''
  //   Without using the power-saving feature (PSM_EN = 0), the
  //   controller has to wait before reading out measurement results,
  //   at least for the programmed integration time. For example,
  //   for ALS_IT = 100 ms a wait time of ≥ 100 ms is needed.
  //   '''
  // Based on testing, it needs more. So doubling to be sure.

  unsigned long timeToWait = 2 * getIntegrationTimeValue(); // see above
  unsigned long timeWaited = millis() - lastRead;

  if (timeWaited < timeToWait)
    delay(timeToWait - timeWaited);
}

bool Adafruit_VEML7700::readReady(void) {
  // From app note:
  //   '''
  //   Without using the power-saving feature (PSM_EN = 0), the
  //   controller has to wait before reading out measurement results,
  //   at least for the programmed integration time. For example,
  //   for ALS_IT = 100 ms a wait time of ≥ 100 ms is needed.
  //   '''
  // Based on testing, it needs more. So doubling to be sure.

  unsigned long timeToWait = 2 * getIntegrationTimeValue(); // see above
  unsigned long timeWaited = millis() - lastRead;

  return (timeWaited >= timeToWait);
}

/*!
 *  @brief Implemenation of App Note "Designing the VEML7700 Into an
 * Application", Vishay Document Number: 84323, Fig. 24 Flow Chart. This will
 * automatically adjust gain and integration time as needed to obtain a good raw
 * count value. Additionally, a non-linear correction is applied if needed.
 */
float Adafruit_VEML7700::autoLux(void) {
  const uint8_t gains[] = {VEML7700_GAIN_1_8, VEML7700_GAIN_1_4,
                           VEML7700_GAIN_1, VEML7700_GAIN_2};
  const uint8_t intTimes[] = {VEML7700_IT_25MS,  VEML7700_IT_50MS,
                              VEML7700_IT_100MS, VEML7700_IT_200MS,
                              VEML7700_IT_400MS, VEML7700_IT_800MS};

  uint8_t gainIndex = 0;      // start with ALS gain = 1/8
  uint8_t itIndex = 2;        // start with ALS integration time = 100ms
  bool useCorrection = false; // flag for non-linear correction

  setGain(gains[gainIndex]);
  setIntegrationTime(intTimes[itIndex]);

  uint16_t ALS = readALS(true);
  // Serial.println("** AUTO LUX DEBUG **");
  // Serial.print("ALS initial = "); Serial.println(ALS);

  if (ALS <= 100) {

    // increase first gain and then integration time as needed
    // compute lux using simple linear formula
    while ((ALS <= 100) && !((gainIndex == 3) && (itIndex == 5))) {
      if (gainIndex < 3) {
        setGain(gains[++gainIndex]);
      } else if (itIndex < 5) {
        setIntegrationTime(intTimes[++itIndex]);
      }
      ALS = readALS(true);
      // Serial.print("ALS low lux = "); Serial.println(ALS);
    }

  } else {

    // decrease integration time as needed
    // compute lux using non-linear correction
    useCorrection = true;
    while ((ALS > 10000) && (itIndex > 0)) {
      setIntegrationTime(intTimes[--itIndex]);
      ALS = readALS(true);
      // Serial.print("ALS  hi lux = "); Serial.println(ALS);
    }
  }
  // Serial.println("** AUTO LUX DEBUG **");

  return computeLux(ALS, useCorrection);
}