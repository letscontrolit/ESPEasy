/*!
 * @file Adafruit_HMC5883_U.cpp
 *
 * @mainpage Adafruit HMC5883 Unified Library
 *
 * @section intro_sec Introduction
 *
 * This is a library for the HMC5883 magnentometer/compass
 *
 * Designed specifically to work with the Adafruit HMC5883 Breakout
 * http://www.adafruit.com/products/1746
 *
 * These displays use I2C to communicate, 2 pins are required to interface.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit andopen-source hardware by purchasing products
 * from Adafruit!
 *
 * @section author Author
 *
 * Written by Kevin Townsend for Adafruit Industries.
 *
 * @section license License
 *
 * BSD license, all text above must be included in any redistribution
 */

#include "Arduino.h"

#include <Wire.h>

#include <limits.h>

#include "Adafruit_HMC5883_U.h"

static float _hmc5883_Gauss_LSB_XY = 1100.0F; // Varies with gain
static float _hmc5883_Gauss_LSB_Z = 980.0F;   // Varies with gain

/***************************************************************************
 MAGNETOMETER
 ***************************************************************************/
/***************************************************************************
 PRIVATE FUNCTIONS
 ***************************************************************************/

/**************************************************************************/
/*!
    @brief  Abstract away platform differences in Arduino wire library
*/
/**************************************************************************/
void Adafruit_HMC5883_Unified::write8(byte address, byte reg, byte value) {
  Wire.beginTransmission(address);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)value);
  Wire.endTransmission();
}

/**************************************************************************/
/*!
    @brief  Abstract away platform differences in Arduino wire library
*/
/**************************************************************************/
byte Adafruit_HMC5883_Unified::read8(byte address, byte reg) {
  byte value;

  Wire.beginTransmission(address);
  Wire.write((uint8_t)reg);
  Wire.endTransmission();
  Wire.requestFrom(address, (byte)1);
  value = Wire.read();
  Wire.endTransmission();

  return value;
}

/**************************************************************************/
/*!
    @brief  Reads the raw data from the sensor
*/
/**************************************************************************/
void Adafruit_HMC5883_Unified::read() {
  // Read the magnetometer
  Wire.beginTransmission((byte)HMC5883_ADDRESS_MAG);
  Wire.write(HMC5883_REGISTER_MAG_OUT_X_H_M);
  Wire.endTransmission();
  Wire.requestFrom((byte)HMC5883_ADDRESS_MAG, (byte)6);

  uint8_t xhi = 0;
  uint8_t xlo = 0;
  uint8_t zhi = 0;
  uint8_t zlo = 0;
  uint8_t yhi = 0;
  uint8_t ylo = 0;
  if (Wire.available() == 6) {
    // Note high before low (different than accel)
    xhi = Wire.read();
    xlo = Wire.read();
    zhi = Wire.read();
    zlo = Wire.read();
    yhi = Wire.read();
    ylo = Wire.read();
  }

  // Shift values to create properly formed integer (low byte first)
  _magData.x = (int16_t)(xlo | ((int16_t)xhi << 8));
  _magData.y = (int16_t)(ylo | ((int16_t)yhi << 8));
  _magData.z = (int16_t)(zlo | ((int16_t)zhi << 8));

  // ToDo: Calculate orientation
  _magData.orientation = 0.0f;
}

/***************************************************************************
 CONSTRUCTOR
 ***************************************************************************/

/**************************************************************************/
/*!
    @brief  Instantiates a new Adafruit_HMC5883 class
*/
/**************************************************************************/
Adafruit_HMC5883_Unified::Adafruit_HMC5883_Unified(int32_t sensorID) {
  _sensorID = sensorID;
}

/***************************************************************************
 PUBLIC FUNCTIONS
 ***************************************************************************/

/**************************************************************************/
/*!
    @brief  Setups the HW
*/
/**************************************************************************/
bool Adafruit_HMC5883_Unified::begin() {
  // Enable I2C
  //Wire.begin(); // This is already done by ESP Easy Core

  // Enable the magnetometer
  write8(HMC5883_ADDRESS_MAG, HMC5883_REGISTER_MAG_MR_REG_M, 0x00);

  // Set the gain to a known level
  setMagGain(HMC5883_MAGGAIN_1_3);

  return true;
}

/**************************************************************************/
/*!
    @brief  Sets the magnetometer's gain
*/
/**************************************************************************/
void Adafruit_HMC5883_Unified::setMagGain(hmc5883MagGain gain) {
  write8(HMC5883_ADDRESS_MAG, HMC5883_REGISTER_MAG_CRB_REG_M, (byte)gain);

  _magGain = gain;

  switch (gain) {
  case HMC5883_MAGGAIN_1_3:
    _hmc5883_Gauss_LSB_XY = 1100.0f;
    _hmc5883_Gauss_LSB_Z = 980.0f;
    break;
  case HMC5883_MAGGAIN_1_9:
    _hmc5883_Gauss_LSB_XY = 855.0f;
    _hmc5883_Gauss_LSB_Z = 760.0f;
    break;
  case HMC5883_MAGGAIN_2_5:
    _hmc5883_Gauss_LSB_XY = 670.0f;
    _hmc5883_Gauss_LSB_Z = 600.0f;
    break;
  case HMC5883_MAGGAIN_4_0:
    _hmc5883_Gauss_LSB_XY = 450.0f;
    _hmc5883_Gauss_LSB_Z = 400.0f;
    break;
  case HMC5883_MAGGAIN_4_7:
    _hmc5883_Gauss_LSB_XY = 400.0f;
    _hmc5883_Gauss_LSB_Z = 255.0f;
    break;
  case HMC5883_MAGGAIN_5_6:
    _hmc5883_Gauss_LSB_XY = 330.0f;
    _hmc5883_Gauss_LSB_Z = 295.0f;
    break;
  case HMC5883_MAGGAIN_8_1:
    _hmc5883_Gauss_LSB_XY = 230.0f;
    _hmc5883_Gauss_LSB_Z = 205.0f;
    break;
  }
}

/**************************************************************************/
/*!
    @brief  Gets the most recent sensor event
*/
/**************************************************************************/
bool Adafruit_HMC5883_Unified::getEvent(sensors_event_t *event) {
  /* Clear the event */
  memset(event, 0, sizeof(sensors_event_t));

  /* Read new data */
  read();

  event->version = sizeof(sensors_event_t);
  event->sensor_id = _sensorID;
  event->type = SENSOR_TYPE_MAGNETIC_FIELD;
  event->timestamp = 0;
  event->magnetic.x =
      _magData.x / _hmc5883_Gauss_LSB_XY * SENSORS_GAUSS_TO_MICROTESLA;
  event->magnetic.y =
      _magData.y / _hmc5883_Gauss_LSB_XY * SENSORS_GAUSS_TO_MICROTESLA;
  event->magnetic.z =
      _magData.z / _hmc5883_Gauss_LSB_Z * SENSORS_GAUSS_TO_MICROTESLA;

  return true;
}

/**************************************************************************/
/*!
    @brief  Gets the sensor_t data
*/
/**************************************************************************/
void Adafruit_HMC5883_Unified::getSensor(sensor_t *sensor) {
  /* Clear the sensor_t object */
  memset(sensor, 0, sizeof(sensor_t));

  /* Insert the sensor name in the fixed length char array */
  strncpy(sensor->name, "HMC5883", sizeof(sensor->name) - 1);
  sensor->name[sizeof(sensor->name) - 1] = 0;
  sensor->version = 1;
  sensor->sensor_id = _sensorID;
  sensor->type = SENSOR_TYPE_MAGNETIC_FIELD;
  sensor->min_delay = 0;
  sensor->max_value = 800.0f;  // 8 gauss == 800 microTesla
  sensor->min_value = -800.0f; // -8 gauss == -800 microTesla
  sensor->resolution = 0.2f; // 2 milligauss == 0.2 microTesla
}
