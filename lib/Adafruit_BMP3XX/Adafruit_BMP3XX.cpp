/*!
 * @file Adafruit_BMP3XX.cpp
 *
 * @mainpage Adafruit BMP3XX temperature & barometric pressure sensor driver
 *
 * @section intro_sec Introduction
 *
 * This is the documentation for Adafruit's BMP3XX driver for the
 * Arduino platform.  It is designed specifically to work with the
 * Adafruit BMP388 breakout: https://www.adafruit.com/products/3966
 *
 * These sensors use I2C or SPI to communicate
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * @section author Author
 *
 * Written by Ladyada for Adafruit Industries.
 *
 * @section license License
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */

#include "Adafruit_BMP3XX.h"
#include "Arduino.h"

//#define BMP3XX_DEBUG

Adafruit_I2CDevice *g_i2c_dev = NULL; ///< Global I2C interface pointer
Adafruit_SPIDevice *g_spi_dev = NULL; ///< Global SPI interface pointer

// Our hardware interface functions
static int8_t i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len,
                        void *intf_ptr);
static int8_t i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len,
                       void *intf_ptr);
static int8_t spi_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len,
                       void *intf_ptr);
static int8_t spi_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len,
                        void *intf_ptr);
static void delay_usec(uint32_t us, void *intf_ptr);
static int8_t validate_trimming_param(struct bmp3_dev *dev);
static int8_t cal_crc(uint8_t seed, uint8_t data);

/***************************************************************************
 PUBLIC FUNCTIONS
 ***************************************************************************/

/**************************************************************************/
/*!
    @brief  Instantiates sensor
*/
/**************************************************************************/
Adafruit_BMP3XX::Adafruit_BMP3XX(void) {
  _meas_end = 0;
  _filterEnabled = _tempOSEnabled = _presOSEnabled = false;
}

/**************************************************************************/
/*!
    @brief Initializes the sensor

    Hardware ss initialized, verifies it is in the I2C or SPI bus, then reads
    calibration data in preparation for sensor reads.

    @param  addr Optional parameter for the I2C address of BMP3. Default is 0x77
    @param  theWire Optional parameter for the I2C device we will use. Default
   is "Wire"
    @return True on sensor initialization success. False on failure.
*/
/**************************************************************************/
bool Adafruit_BMP3XX::begin_I2C(uint8_t addr, TwoWire *theWire) {
  if (i2c_dev)
    delete i2c_dev;
  if (spi_dev)
    delete spi_dev;
  spi_dev = NULL;

  g_i2c_dev = i2c_dev = new Adafruit_I2CDevice(addr, theWire);

  // verify i2c address was found
  if (!i2c_dev->begin()) {
    return false;
  }

  the_sensor.chip_id = addr;
  the_sensor.intf = BMP3_I2C_INTF;
  the_sensor.read = &i2c_read;
  the_sensor.write = &i2c_write;
  the_sensor.intf_ptr = g_i2c_dev;
  the_sensor.dummy_byte = 0;

  return _init();
}

/*!
 *    @brief  Sets up the hardware and initializes hardware SPI
 *    @param  cs_pin The arduino pin # connected to chip select
 *    @param  theSPI The SPI object to be used for SPI connections.
 *    @return True if initialization was successful, otherwise false.
 */
bool Adafruit_BMP3XX::begin_SPI(uint8_t cs_pin, SPIClass *theSPI) {
  if (i2c_dev)
    delete i2c_dev;
  if (spi_dev)
    delete spi_dev;
  i2c_dev = NULL;

  g_spi_dev = spi_dev =
      new Adafruit_SPIDevice(cs_pin,
                             BMP3XX_DEFAULT_SPIFREQ, // frequency
                             SPI_BITORDER_MSBFIRST,  // bit order
                             SPI_MODE0,              // data mode
                             theSPI);

  if (!spi_dev->begin()) {
    return false;
  }

  the_sensor.chip_id = cs_pin;
  the_sensor.intf = BMP3_SPI_INTF;
  the_sensor.read = &spi_read;
  the_sensor.write = &spi_write;
  the_sensor.intf_ptr = g_spi_dev;
  the_sensor.dummy_byte = 1;

  return _init();
}

/*!
 *    @brief  Sets up the hardware and initializes software SPI
 *    @param  cs_pin The arduino pin # connected to chip select
 *    @param  sck_pin The arduino pin # connected to SPI clock
 *    @param  miso_pin The arduino pin # connected to SPI MISO
 *    @param  mosi_pin The arduino pin # connected to SPI MOSI
 *    @return True if initialization was successful, otherwise false.
 */
bool Adafruit_BMP3XX::begin_SPI(int8_t cs_pin, int8_t sck_pin, int8_t miso_pin,
                                int8_t mosi_pin) {
  if (i2c_dev)
    delete i2c_dev;
  if (spi_dev)
    delete spi_dev;
  i2c_dev = NULL;

  g_spi_dev = spi_dev =
      new Adafruit_SPIDevice(cs_pin, sck_pin, miso_pin, mosi_pin,
                             BMP3XX_DEFAULT_SPIFREQ, // frequency
                             SPI_BITORDER_MSBFIRST,  // bit order
                             SPI_MODE0);             // data mode

  if (!spi_dev->begin()) {
    return false;
  }

  the_sensor.chip_id = cs_pin;
  the_sensor.intf = BMP3_SPI_INTF;
  the_sensor.read = &spi_read;
  the_sensor.write = &spi_write;
  the_sensor.intf_ptr = g_spi_dev;
  the_sensor.dummy_byte = 1;

  return _init();
}

bool Adafruit_BMP3XX::_init(void) {
  g_i2c_dev = i2c_dev;
  g_spi_dev = spi_dev;
  the_sensor.delay_us = delay_usec;
  int8_t rslt = BMP3_OK;

  /* Reset the sensor */
  rslt = bmp3_soft_reset(&the_sensor);
#ifdef BMP3XX_DEBUG
  Serial.print("Reset result: ");
  Serial.println(rslt);
#endif
  if (rslt != BMP3_OK)
    return false;

  rslt = bmp3_init(&the_sensor);
#ifdef BMP3XX_DEBUG
  Serial.print("Init result: ");
  Serial.println(rslt);
#endif

  rslt = validate_trimming_param(&the_sensor);
#ifdef BMP3XX_DEBUG
  Serial.print("Valtrim result: ");
  Serial.println(rslt);
#endif

  if (rslt != BMP3_OK)
    return false;

#ifdef BMP3XX_DEBUG
  Serial.print("T1 = ");
  Serial.println(the_sensor.calib_data.reg_calib_data.par_t1);
  Serial.print("T2 = ");
  Serial.println(the_sensor.calib_data.reg_calib_data.par_t2);
  Serial.print("T3 = ");
  Serial.println(the_sensor.calib_data.reg_calib_data.par_t3);
  Serial.print("P1 = ");
  Serial.println(the_sensor.calib_data.reg_calib_data.par_p1);
  Serial.print("P2 = ");
  Serial.println(the_sensor.calib_data.reg_calib_data.par_p2);
  Serial.print("P3 = ");
  Serial.println(the_sensor.calib_data.reg_calib_data.par_p3);
  Serial.print("P4 = ");
  Serial.println(the_sensor.calib_data.reg_calib_data.par_p4);
  Serial.print("P5 = ");
  Serial.println(the_sensor.calib_data.reg_calib_data.par_p5);
  Serial.print("P6 = ");
  Serial.println(the_sensor.calib_data.reg_calib_data.par_p6);
  Serial.print("P7 = ");
  Serial.println(the_sensor.calib_data.reg_calib_data.par_p7);
  Serial.print("P8 = ");
  Serial.println(the_sensor.calib_data.reg_calib_data.par_p8);
  Serial.print("P9 = ");
  Serial.println(the_sensor.calib_data.reg_calib_data.par_p9);
  Serial.print("P10 = ");
  Serial.println(the_sensor.calib_data.reg_calib_data.par_p10);
  Serial.print("P11 = ");
  Serial.println(the_sensor.calib_data.reg_calib_data.par_p11);
  // Serial.print("T lin = ");
  // Serial.println(the_sensor.calib_data.reg_calib_data.t_lin);
#endif

  setTemperatureOversampling(BMP3_NO_OVERSAMPLING);
  setPressureOversampling(BMP3_NO_OVERSAMPLING);
  setIIRFilterCoeff(BMP3_IIR_FILTER_DISABLE);
  setOutputDataRate(BMP3_ODR_25_HZ);

  // don't do anything till we request a reading
  the_sensor.settings.op_mode = BMP3_MODE_FORCED;

  return true;
}

/**************************************************************************/
/*!
    @brief Performs a reading and returns the ambient temperature.
    @return Temperature in degrees Centigrade
*/
/**************************************************************************/
float Adafruit_BMP3XX::readTemperature(void) {
  performReading();
  return temperature;
}

/**************************************************************************/
/*!
    @brief Reads the chip identifier
    @return BMP3_CHIP_ID or BMP390_CHIP_ID
*/
/**************************************************************************/
uint8_t Adafruit_BMP3XX::chipID(void) { return the_sensor.chip_id; }

/**************************************************************************/
/*!
    @brief Performs a reading and returns the barometric pressure.
    @return Barometic pressure in Pascals
*/
/**************************************************************************/
float Adafruit_BMP3XX::readPressure(void) {
  performReading();
  return pressure;
}

/**************************************************************************/
/*!
    @brief Calculates the altitude (in meters).

    Reads the current atmostpheric pressure (in hPa) from the sensor and
   calculates via the provided sea-level pressure (in hPa).

    @param  seaLevel      Sea-level pressure in hPa
    @return Altitude in meters
*/
/**************************************************************************/
float Adafruit_BMP3XX::readAltitude(float seaLevel) {
  // Equation taken from BMP180 datasheet (page 16):
  //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

  // Note that using the equation from wikipedia can give bad results
  // at high altitude. See this thread for more information:
  //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064

  float atmospheric = readPressure() / 100.0F;
  return 44330.0 * (1.0 - pow(atmospheric / seaLevel, 0.1903));
}

/**************************************************************************/
/*!
    @brief Performs a full reading of all sensors in the BMP3XX.

    Assigns the internal Adafruit_BMP3XX#temperature & Adafruit_BMP3XX#pressure
   member variables

    @return True on success, False on failure
*/
/**************************************************************************/
bool Adafruit_BMP3XX::performReading(void) {
  g_i2c_dev = i2c_dev;
  g_spi_dev = spi_dev;
  int8_t rslt;
  /* Used to select the settings user needs to change */
  uint16_t settings_sel = 0;
  /* Variable used to select the sensor component */
  uint8_t sensor_comp = 0;

  /* Select the pressure and temperature sensor to be enabled */
  the_sensor.settings.temp_en = BMP3_ENABLE;
  settings_sel |= BMP3_SEL_TEMP_EN;
  sensor_comp |= BMP3_TEMP;
  if (_tempOSEnabled) {
    settings_sel |= BMP3_SEL_TEMP_OS;
  }

  the_sensor.settings.press_en = BMP3_ENABLE;
  settings_sel |= BMP3_SEL_PRESS_EN;
  sensor_comp |= BMP3_PRESS;
  if (_presOSEnabled) {
    settings_sel |= BMP3_SEL_PRESS_OS;
  }

  if (_filterEnabled) {
    settings_sel |= BMP3_SEL_IIR_FILTER;
  }

  if (_ODREnabled) {
    settings_sel |= BMP3_SEL_ODR;
  }

  // set interrupt to data ready
  // settings_sel |= BMP3_DRDY_EN_SEL | BMP3_LEVEL_SEL | BMP3_LATCH_SEL;

  /* Set the desired sensor configuration */
#ifdef BMP3XX_DEBUG
  Serial.println("Setting sensor settings");
#endif
  rslt = bmp3_set_sensor_settings(settings_sel, &the_sensor);

  if (rslt != BMP3_OK)
    return false;

  /* Set the power mode */
  the_sensor.settings.op_mode = BMP3_MODE_FORCED;
#ifdef BMP3XX_DEBUG
  Serial.println(F("Setting power mode"));
#endif
  rslt = bmp3_set_op_mode(&the_sensor);
  if (rslt != BMP3_OK)
    return false;

  /* Variable used to store the compensated data */
  struct bmp3_data data;

  /* Temperature and Pressure data are read and stored in the bmp3_data instance
   */
#ifdef BMP3XX_DEBUG
  Serial.println(F("Getting sensor data"));
#endif
  rslt = bmp3_get_sensor_data(sensor_comp, &data, &the_sensor);
  if (rslt != BMP3_OK)
    return false;

  /*
#ifdef BMP3XX_DEBUG
  Serial.println(F("Analyzing sensor data"));
#endif
  rslt = analyze_sensor_data(&data);
  if (rslt != BMP3_OK)
    return false;
    */

  /* Save the temperature and pressure data */
  temperature = data.temperature;
  pressure = data.pressure;

  return true;
}

/**************************************************************************/
/*!
    @brief  Setter for Temperature oversampling
    @param  oversample Oversampling setting, can be BMP3_NO_OVERSAMPLING,
   BMP3_OVERSAMPLING_2X, BMP3_OVERSAMPLING_4X, BMP3_OVERSAMPLING_8X,
   BMP3_OVERSAMPLING_16X, BMP3_OVERSAMPLING_32X
    @return True on success, False on failure
*/
/**************************************************************************/

bool Adafruit_BMP3XX::setTemperatureOversampling(uint8_t oversample) {
  if (oversample > BMP3_OVERSAMPLING_32X)
    return false;

  the_sensor.settings.odr_filter.temp_os = oversample;

  if (oversample == BMP3_NO_OVERSAMPLING)
    _tempOSEnabled = false;
  else
    _tempOSEnabled = true;

  return true;
}

/**************************************************************************/
/*!
    @brief  Setter for Pressure oversampling
    @param  oversample Oversampling setting, can be BMP3_NO_OVERSAMPLING,
   BMP3_OVERSAMPLING_2X, BMP3_OVERSAMPLING_4X, BMP3_OVERSAMPLING_8X,
   BMP3_OVERSAMPLING_16X, BMP3_OVERSAMPLING_32X
    @return True on success, False on failure
*/
/**************************************************************************/
bool Adafruit_BMP3XX::setPressureOversampling(uint8_t oversample) {
  if (oversample > BMP3_OVERSAMPLING_32X)
    return false;

  the_sensor.settings.odr_filter.press_os = oversample;

  if (oversample == BMP3_NO_OVERSAMPLING)
    _presOSEnabled = false;
  else
    _presOSEnabled = true;

  return true;
}

/**************************************************************************/
/*!
    @brief  Setter for IIR filter coefficient
    @param filtercoeff Coefficient of the filter (in samples). Can be
   BMP3_IIR_FILTER_DISABLE (no filtering), BMP3_IIR_FILTER_COEFF_1,
   BMP3_IIR_FILTER_COEFF_3, BMP3_IIR_FILTER_COEFF_7, BMP3_IIR_FILTER_COEFF_15,
   BMP3_IIR_FILTER_COEFF_31, BMP3_IIR_FILTER_COEFF_63, BMP3_IIR_FILTER_COEFF_127
    @return True on success, False on failure

*/
/**************************************************************************/
bool Adafruit_BMP3XX::setIIRFilterCoeff(uint8_t filtercoeff) {
  if (filtercoeff > BMP3_IIR_FILTER_COEFF_127)
    return false;

  the_sensor.settings.odr_filter.iir_filter = filtercoeff;

  if (filtercoeff == BMP3_IIR_FILTER_DISABLE)
    _filterEnabled = false;
  else
    _filterEnabled = true;

  return true;
}

/**************************************************************************/
/*!
    @brief  Setter for output data rate (ODR)
    @param odr Sample rate in Hz. Can be BMP3_ODR_200_HZ, BMP3_ODR_100_HZ,
   BMP3_ODR_50_HZ, BMP3_ODR_25_HZ, BMP3_ODR_12_5_HZ, BMP3_ODR_6_25_HZ,
   BMP3_ODR_3_1_HZ, BMP3_ODR_1_5_HZ, BMP3_ODR_0_78_HZ, BMP3_ODR_0_39_HZ,
   BMP3_ODR_0_2_HZ, BMP3_ODR_0_1_HZ, BMP3_ODR_0_05_HZ, BMP3_ODR_0_02_HZ,
   BMP3_ODR_0_01_HZ, BMP3_ODR_0_006_HZ, BMP3_ODR_0_003_HZ, or BMP3_ODR_0_001_HZ
    @return True on success, False on failure

*/
/**************************************************************************/
bool Adafruit_BMP3XX::setOutputDataRate(uint8_t odr) {
  if (odr > BMP3_ODR_0_001_HZ)
    return false;

  the_sensor.settings.odr_filter.odr = odr;

  _ODREnabled = true;

  return true;
}

/**************************************************************************/
/*!
    @brief  Reads 8 bit values over I2C
*/
/**************************************************************************/
int8_t i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len,
                void *intf_ptr) {
  // Serial.print("I2C read address 0x"); Serial.print(reg_addr, HEX);
  // Serial.print(" len "); Serial.println(len, HEX);

  if (!g_i2c_dev->write_then_read(&reg_addr, 1, reg_data, len))
    return 1;

  return 0;
}

/**************************************************************************/
/*!
    @brief  Writes 8 bit values over I2C
*/
/**************************************************************************/
int8_t i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len,
                 void *intf_ptr) {
  // Serial.print("I2C write address 0x"); Serial.print(reg_addr, HEX);
  // Serial.print(" len "); Serial.println(len, HEX);

  if (!g_i2c_dev->write((uint8_t *)reg_data, len, true, &reg_addr, 1))
    return 1;

  return 0;
}

/**************************************************************************/
/*!
    @brief  Reads 8 bit values over SPI
*/
/**************************************************************************/
static int8_t spi_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len,
                       void *intf_ptr) {
  g_spi_dev->write_then_read(&reg_addr, 1, reg_data, len, 0xFF);
  return 0;
}

/**************************************************************************/
/*!
    @brief  Writes 8 bit values over SPI
*/
/**************************************************************************/
static int8_t spi_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len,
                        void *intf_ptr) {
  g_spi_dev->write((uint8_t *)reg_data, len, &reg_addr, 1);

  return 0;
}

static void delay_usec(uint32_t us, void *intf_ptr) { delayMicroseconds(us); }

static int8_t validate_trimming_param(struct bmp3_dev *dev) {
  int8_t rslt;
  uint8_t crc = 0xFF;
  uint8_t stored_crc;
  uint8_t trim_param[21];
  uint8_t i;

  rslt = bmp3_get_regs(BMP3_REG_CALIB_DATA, trim_param, 21, dev);
  if (rslt == BMP3_OK) {
    for (i = 0; i < 21; i++) {
      crc = (uint8_t)cal_crc(crc, trim_param[i]);
    }

    crc = (crc ^ 0xFF);
    rslt = bmp3_get_regs(0x30, &stored_crc, 1, dev);
    if (stored_crc != crc) {
      rslt = -1;
    }
  }

  return rslt;
}

/*
 * @brief function to calculate CRC for the trimming parameters
 * */
static int8_t cal_crc(uint8_t seed, uint8_t data) {
  int8_t poly = 0x1D;
  int8_t var2;
  uint8_t i;

  for (i = 0; i < 8; i++) {
    if ((seed & 0x80) ^ (data & 0x80)) {
      var2 = 1;
    } else {
      var2 = 0;
    }

    seed = (seed & 0x7F) << 1;
    data = (data & 0x7F) << 1;
    seed = seed ^ (uint8_t)(poly * var2);
  }

  return (int8_t)seed;
}