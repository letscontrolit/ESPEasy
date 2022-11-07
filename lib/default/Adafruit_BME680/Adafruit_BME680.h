/*!
 * @file Adafruit_BME680.h
 *
 * Adafruit BME680 temperature, humidity, barometric pressure and gas sensor
 * driver
 *
 * This is the documentation for Adafruit's BME680 driver for the
 * Arduino platform.  It is designed specifically to work with the
 * Adafruit BME680 breakout: https://www.adafruit.com/products/3660
 *
 * These sensors use I2C to communicate, 2 pins (SCL+SDA) are required
 * to interface with the breakout.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * Written by Ladyada for Adafruit Industries.
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */

#ifndef __BME680_H__
#define __BME680_H__

#include "Arduino.h"

#include "bme680.h"
#include <Adafruit_Sensor.h>
#include <SPI.h>
#include <Wire.h>

#define BME680_DEFAULT_ADDRESS (0x77)    ///< The default I2C address
#define BME680_DEFAULT_SPIFREQ (1000000) ///< The default SPI Clock speed

/*! Adafruit_BME680 Class for both I2C and SPI usage.
 *  Wraps the Bosch library for Arduino usage
 */
class Adafruit_BME680 {
public:
  /** Value returned by remainingReadingMillis indicating no asynchronous
   * reading has been initiated by beginReading. **/
  static constexpr int reading_not_started = -1;
  /** Value returned by remainingReadingMillis indicating asynchronous reading
   * is complete and calling endReading will not block. **/
  static constexpr int reading_complete = 0;

  Adafruit_BME680(TwoWire *theWire = &Wire);
  Adafruit_BME680(int8_t cspin, SPIClass *theSPI = &SPI);
  Adafruit_BME680(int8_t cspin, int8_t mosipin, int8_t misopin, int8_t sckpin);

  bool begin(uint8_t addr = BME680_DEFAULT_ADDRESS, bool initSettings = true);
  float readTemperature();
  float readPressure();
  float readHumidity();
  uint32_t readGas();
  float readAltitude(float seaLevel);

  bool setTemperatureOversampling(uint8_t os);
  bool setPressureOversampling(uint8_t os);
  bool setHumidityOversampling(uint8_t os);
  bool setIIRFilterSize(uint8_t fs);
  bool setGasHeater(uint16_t heaterTemp, uint16_t heaterTime);

  // Perform a reading in blocking mode.
  bool performReading();

  /*! @brief Begin an asynchronous reading.
   *  @return When the reading would be ready as absolute time in millis().
   */
  unsigned long beginReading();

  /*! @brief  End an asynchronous reading.
   *          If the asynchronous reading is still in progress, block until it
   * ends. If no asynchronous reading has started, this is equivalent to
   * performReading().
   *  @return Whether success.
   */
  bool endReading();

  /*! @brief  Get remaining time for an asynchronous reading.
   *          If the asynchronous reading is still in progress, how many millis
   * until its completion. If the asynchronous reading is completed, 0. If no
   * asynchronous reading has started, -1 or
   * Adafruit_BME680::reading_not_started. Does not block.
   *  @return Remaining millis until endReading will not block if invoked.
   */
  int remainingReadingMillis();

  /** Temperature (Celsius) assigned after calling performReading() or
   * endReading() **/
  float temperature;
  /** Pressure (Pascals) assigned after calling performReading() or endReading()
   * **/
  uint32_t pressure;
  /** Humidity (RH %) assigned after calling performReading() or endReading()
   * **/
  float humidity;
  /** Gas resistor (ohms) assigned after calling performReading() or
   * endReading() **/
  uint32_t gas_resistance;

private:
  bool _filterEnabled, _tempEnabled, _humEnabled, _presEnabled, _gasEnabled;
  uint8_t _i2caddr;
  int32_t _sensorID;
  int8_t _cs;
  unsigned long _meas_start;
  uint16_t _meas_period;

  uint8_t spixfer(uint8_t x);

  struct bme680_dev gas_sensor;
};

#endif
