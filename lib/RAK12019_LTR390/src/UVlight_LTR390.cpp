/** Based on Adafruit LTR390 Arduino Library **/
/** Changed to work with RAKWireless RAK12019 UV Light Sensor **/

/*!
 *  @file Adafruit_LTR390.cpp
 *
 *  @mainpage Adafruit LTR390 I2C UV and Light Sensor
 *
 *  @section intro_sec Introduction
 *
 *      I2C Driver for the LTR390 I2C UV and Light sensor
 *
 *      This is a library for the Adafruit LTR390 breakout:
 *      https://www.adafruit.com/product/4831
 *
 *      Adafruit invests time and resources providing this open source code,
 *  please support Adafruit and open-source hardware by purchasing products from
 *      Adafruit!
 *
 *  @section author Author
 *
 *  Limor Fried (Adafruit Industries)
 *
 *      @section license License
 *
 *      BSD (see license.txt)
 *
 *      @section  HISTORY
 *
 *     v1.0 - First release
 *
 * Modifications:
 * 2021-12 RAKWireless: Make it work with the RAK12019 sensor
 * 2022-03-28 tonhuisman: Format source using Uncrustify, use f postfix on float constants, add optional doReset argument to init(),
 *                        inhibit unused constructors, add virtual destructor
 */

#include "UVlight_LTR390.h"

// UVlight_LTR390::UVlight_LTR390(int addr) {
//   _wire = &Wire;
//   i2cAddress = addr;
// }

UVlight_LTR390::UVlight_LTR390() {
  _wire      = &Wire;
  i2cAddress = LTR390_ADDRESS;
}

// UVlight_LTR390::UVlight_LTR390(TwoWire *w, int addr) {
//   _wire = w;
//   i2cAddress = addr;
// }

// UVlight_LTR390::UVlight_LTR390(TwoWire *w) {
//   _wire = w;
//   i2cAddress = LTR390_ADDRESS;
// }

/*!
    @brief  LTR390 Initialization
    @returns True on success, Fail on false.
 */
bool UVlight_LTR390::init(bool doReset) {
  ltrId = readRegister(LTR390_PART_ID);

  // check part ID!
  if ((ltrId  >> 4) != 0x0B) {
    return false;
  }

  // OK now we can do a soft reset
  // Reset disabled for RAK12019 sensor

  if (doReset && !reset()) {
    return false;
  }

  // main screen turn on
  enable(true);

  if (!enabled()) {
    return false;
  }

  // RAK12019 specific settings, default in plugin
  // setGain(LTR390_GAIN_3);                 // set gain
  // setResolution(LTR390_RESOLUTION_18BIT); // set resolution
  return true;
}

/*!
    @brief  Perform a soft reset with 10ms delay.
    @returns True on success (reset bit was cleared post-write)
 */
bool UVlight_LTR390::reset(void) {
  uint8_t readData = readRegister(LTR390_MAIN_CTRL);

  readData |= B00010000;

  writeRegister(LTR390_MAIN_CTRL, readData);
  delay(10);
  readData = readRegister(LTR390_MAIN_CTRL);

  if (readData != 0) {
    return false;
  }
  return true;
}

/*!
    @brief  Checks if new data is available in data register
    @returns True on new data available
 */
bool UVlight_LTR390::newDataAvailable(void) {
  uint8_t readStatus = readRegister(LTR390_MAIN_STATUS);

  readStatus >>= 3;
  readStatus  &= 1;
  return readStatus;
}

/*!
    @brief  Read 3-bytes out of ambient data register, does not check if data is new!
    @returns Up to 20 bits, right shifted into a 32 bit int
 */
uint32_t UVlight_LTR390::readALS(void) {
  // uint8_t readLsb = readRegister(LTR390_ALSDATA_LSB);
  // uint8_t readMsb = readRegister(LTR390_ALSDATA_MSB);
  // uint8_t readHsb = readRegister(LTR390_ALSDATA_HSB);

  // readHsb &= 0x0F;
  // uint32_t readData = (readHsb << 16) | (readMsb << 8) | readLsb;
  uint32_t readData = read3Register(LTR390_ALSDATA_LSB);

  return readData;
}

/*!
    @brief  Read 3-bytes out of UV data register, does not check if data is new!
    @returns Up to 20 bits, right shifted into a 32 bit int
 */
uint32_t UVlight_LTR390::readUVS(void) {
  // readLsb = readRegister(LTR390_UVSDATA_LSB);
  // readMsb = readRegister(LTR390_UVSDATA_MSB);
  // readHsb = readRegister(LTR390_UVSDATA_HSB);

  // readHsb &= 0x0F;
  // uint32_t readData = (readHsb << 16) | (readMsb << 8) | readLsb;
  uint32_t readData = read3Register(LTR390_UVSDATA_LSB);

  return readData;
}

/*!
    @brief  get lux data,LUX is calculated according to the formula
    @returns the ambient light data ,unit lux.
 */
float UVlight_LTR390::getLUX() {
  uint32_t raw            = readALS();
  uint8_t  _gain          = (uint8_t)(getGain());
  uint8_t  readResolution = (uint8_t)(getResolution());
  float    lux            = 0.6f * (float)(raw) / (gain_Buf[_gain] * resolution_Buf[readResolution]) * (WFAC);

  return lux;
}

/*!
    @brief  get UVI data,UVI is calculated according to the formula
    @returns the ultraviolet light data,unit uw/cm2.
 */
float UVlight_LTR390::getUVI() {
  uint32_t raw            = readUVS();
  uint8_t  _gain          = (uint8_t)(getGain());
  uint8_t  readResolution = (uint8_t)(getResolution());
  float    uvi            = (float)(raw) /
                            ((gain_Buf[_gain] / gain_Buf[LTR390_GAIN_18]) *
                             (resolution_Buf[readResolution] / resolution_Buf[LTR390_RESOLUTION_20BIT]) *
                             (float)(LTR390_SENSITIVITY)) * (WFAC);

  return uvi;
}

/*

    @brief  Enable or disable the light sensor
    @param  en True to enable, False to disable
 */
void UVlight_LTR390::enable(bool en) {
  uint8_t readData = readRegister(LTR390_MAIN_CTRL);

  readData |= (1 << 1);
  writeRegister(LTR390_MAIN_CTRL, (uint8_t)readData);
}

/*!
    @brief  Read the enabled-bit from the sensor
    @returns True if enabled
 */
bool UVlight_LTR390::enabled(void) {
  uint8_t readData = readRegister(LTR390_MAIN_CTRL);

  readData >>= 1;
  readData  &= 1;
  return readData;
}

/*!
 *  @brief  Set the sensor mode to EITHER ambient (LTR390_MODE_ALS) or UV
 * (LTR390_MODE_UVS)
 *  @param  mode The desired mode - LTR390_MODE_UVS or LTR390_MODE_ALS
 */
void UVlight_LTR390::setMode(ltr390_mode_t mode) {
  uint8_t readData = readRegister(LTR390_MAIN_CTRL);

  readData &= 0xF7;
  readData |= ((uint8_t)mode << 3);
  writeRegister(LTR390_MAIN_CTRL, (uint8_t)readData);
}

/*!
 *  @brief  get the sensor's mode
 *  @returns The current mode - LTR390_MODE_UVS or LTR390_MODE_ALS
 */
ltr390_mode_t UVlight_LTR390::getMode(void) {
  uint8_t readData = readRegister(LTR390_MAIN_CTRL);

  readData >>= 3;
  readData  &= 1;
  return (ltr390_mode_t)readData;
}

/*!
 *  @brief  Set the sensor gain
 *  @param  gain The desired gain: LTR390_GAIN_1, LTR390_GAIN_3, LTR390_GAIN_6
 *  LTR390_GAIN_9 or LTR390_GAIN_18
 */
void UVlight_LTR390::setGain(ltr390_gain_t gain) {
  writeRegister(LTR390_GAIN, (uint8_t)gain);
}

/*!
 *  @brief  Get the sensor's gain
 *  @returns gain The current gain: LTR390_GAIN_1, LTR390_GAIN_3, LTR390_GAIN_6
 *  LTR390_GAIN_9 or LTR390_GAIN_18
 */
ltr390_gain_t UVlight_LTR390::getGain(void) {
  uint8_t readData = readRegister(LTR390_GAIN);

  readData &= 7;
  return (ltr390_gain_t)readData;
}

/*!
 *  @brief  Set the sensor resolution. Higher resolutions take longer to read!
 *  @param  res The desired resolution: LTR390_RESOLUTION_13BIT,
 *  LTR390_RESOLUTION_16BIT, LTR390_RESOLUTION_17BIT, LTR390_RESOLUTION_18BIT,
 *  LTR390_RESOLUTION_19BIT or LTR390_RESOLUTION_20BIT
 */
void UVlight_LTR390::setResolution(ltr390_resolution_t res) {
  uint8_t readData = 0;

  readData |= (res << 4);
  writeRegister(LTR390_MEAS_RATE, (uint8_t)readData);
}

/*!
 *  @brief  Get the sensor's resolution
 *  @returns The current resolution: LTR390_RESOLUTION_13BIT,
 *  LTR390_RESOLUTION_16BIT, LTR390_RESOLUTION_17BIT, LTR390_RESOLUTION_18BIT,
 *  LTR390_RESOLUTION_19BIT or LTR390_RESOLUTION_20BIT
 */
ltr390_resolution_t UVlight_LTR390::getResolution(void) {
  uint8_t readData = readRegister(LTR390_MEAS_RATE);

  readData &= 0x70;
  readData  = 7 & (readData >> 4);
  return (ltr390_resolution_t)readData;
}

/*!
    @brief  Set the interrupt output threshold range for lower and upper.
    When the sensor is below the lower, or above upper, interrupt will fire
    @param  lower The lower value to compare against the data register.
    @param  higher The higher value to compare against the data register.
 */
void UVlight_LTR390::setThresholds(uint32_t lower, uint32_t higher) {
  uint8_t readData = higher;

  writeRegister(LTR390_THRESH_UP,      readData);
  readData = higher >> 8;
  writeRegister(LTR390_THRESH_UP + 1,  readData);
  readData  = higher >> 16;
  readData &= 0x0F;
  writeRegister(LTR390_THRESH_UP + 2,  readData);
  readData = lower;
  writeRegister(LTR390_THRESH_LOW,     readData);
  readData = lower >> 8;
  writeRegister(LTR390_THRESH_LOW + 1, readData);
  readData  = lower >> 16;
  readData &= 0x0F;
  writeRegister(LTR390_THRESH_LOW + 2, readData);
}

/*!
    @brief  Configure the interrupt based on the thresholds in setThresholds()
    When the sensor is below the lower, or above upper thresh, interrupt will
   fire
    @param  enable Whether the interrupt output is enabled
    @param  source Whether to use the ALS or UVS data register to compare
    @param  persistance The number of consecutive out-of-range readings before
            we fire the IRQ. Default is 0 (each reading will fire)
 */
void UVlight_LTR390::configInterrupt(bool enable, ltr390_mode_t  source, uint8_t persistance) {
  uint8_t readData = 0;

  readData |= (enable << 2) | (1 << 4) | (source << 5);
  writeRegister(LTR390_INT_CFG, readData);

  if (persistance > 0x0F) { persistance = 0x0F; }
  uint8_t _p = 0;

  _p |= persistance << 4;
  writeRegister(LTR390_INT_PST, _p);
}

uint8_t UVlight_LTR390::writeRegister(uint8_t reg, uint8_t data) {
  _wire->beginTransmission(i2cAddress);
  _wire->write(reg);
  _wire->write(data);
  return _wire->endTransmission();
}

uint8_t UVlight_LTR390::readRegister(uint8_t reg) {
  uint8_t regValue = 0;

  _wire->beginTransmission(i2cAddress);
  _wire->write(reg);
  _wire->endTransmission();
  _wire->requestFrom(i2cAddress, 1);

  if (_wire->available()) {
    regValue = _wire->read();
  }
  return regValue;
}

uint32_t UVlight_LTR390::read3Register(uint8_t reg) {
  uint32_t regValue = 0;

  _wire->beginTransmission(i2cAddress);
  _wire->write(reg);
  _wire->endTransmission();
  _wire->requestFrom(i2cAddress, 3);

  for (uint8_t n = 0; n < 3; n++) {
    if (_wire->available()) {
      regValue += (_wire->read() << (n * 8));
    }
  }
  regValue &= 0xFFFFF; // max 20 bit
  return regValue;
}
