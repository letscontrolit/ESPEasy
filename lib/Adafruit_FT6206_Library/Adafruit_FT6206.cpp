/*!
 * @file Adafruit_FT6206.cpp
 *
 * @mainpage Adafruit FT2606 Library
 *
 * @section intro_sec Introduction
 *
 * This is a library for the Adafruit Capacitive Touch Screens
 *
 * ----> http://www.adafruit.com/products/1947
 *
 * Adjustment: To avoid a conflict with the XPT2048 library, TS_Point struct is renamed to FT_Point
 * 
 * Check out the links above for our tutorials and wiring diagrams
 * This chipset uses I2C to communicate
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * @section author Author
 *
 * Written by Limor Fried/Ladyada for Adafruit Industries.
 *
 * @section license License

 * MIT license, all text above must be included in any redistribution
 */

#include "Arduino.h"
#include <Adafruit_FT6206.h>
#include <Wire.h>

#if defined(__SAM3X8E__)
#define Wire Wire1
#endif

//#define FT6206_DEBUG
//#define I2C_DEBUG

/**************************************************************************/
/*!
    @brief  Instantiates a new FT6206 class
*/
/**************************************************************************/
// I2C, no address adjustments or pins
Adafruit_FT6206::Adafruit_FT6206() { touches = 0; }

/**************************************************************************/
/*!
    @brief  Setups the I2C interface and hardware, identifies if chip is found
    @param  thresh Optional threshhold-for-touch value, default is
   FT6206_DEFAULT_THRESSHOLD but you can try changing it if your screen is
   too/not sensitive.
    @returns True if an FT6206 is found, false on any failure
*/
/**************************************************************************/
boolean Adafruit_FT6206::begin(uint8_t thresh) {
  // Wire.begin();

#ifdef FT6206_DEBUG
  Serial.print("Vend ID: 0x");
  Serial.println(readRegister8(FT62XX_REG_VENDID), HEX);
  Serial.print("Chip ID: 0x");
  Serial.println(readRegister8(FT62XX_REG_CHIPID), HEX);
  Serial.print("Firm V: ");
  Serial.println(readRegister8(FT62XX_REG_FIRMVERS));
  Serial.print("Point Rate Hz: ");
  Serial.println(readRegister8(FT62XX_REG_POINTRATE));
  Serial.print("Thresh: ");
  Serial.println(readRegister8(FT62XX_REG_THRESHHOLD));

  // dump all registers
  for (int16_t i = 0; i < 0x10; i++) {
    Serial.print("I2C $");
    Serial.print(i, HEX);
    Serial.print(" = 0x");
    Serial.println(readRegister8(i), HEX);
  }
#endif

  // change threshhold to be higher/lower
  writeRegister8(FT62XX_REG_THRESHHOLD, thresh);

  if (readRegister8(FT62XX_REG_VENDID) != FT62XX_VENDID) {
    return false;
  }
  uint8_t id = readRegister8(FT62XX_REG_CHIPID);
  if ((id != FT6206_CHIPID) && (id != FT6236_CHIPID) &&
      (id != FT6236U_CHIPID)) {
    return false;
  }

  return true;
}

/**************************************************************************/
/*!
    @brief  Determines if there are any touches detected
    @returns Number of touches detected, can be 0, 1 or 2
*/
/**************************************************************************/
uint8_t Adafruit_FT6206::touched(void) {
  uint8_t n = readRegister8(FT62XX_REG_NUMTOUCHES);
  if (n > 2) {
    n = 0;
  }
  return n;
}

/**************************************************************************/
/*!
    @brief  Queries the chip and retrieves a point data
    @param  n The # index (0 or 1) to the points we can detect. In theory we can
   detect 2 points but we've found that you should only use this for
   single-touch since the two points cant share the same half of the screen.
    @returns {@link FT_Point} object that has the x and y coordinets set. If the
   z coordinate is 0 it means the point is not touched. If z is 1, it is
   currently touched.
*/
/**************************************************************************/
FT_Point Adafruit_FT6206::getPoint(uint8_t n) {
  readData();
  if ((touches == 0) || (n > 1)) {
    return FT_Point(0, 0, 0);
  } else {
    return FT_Point(touchX[n], touchY[n], 1);
  }
}

/************ lower level i/o **************/

/**************************************************************************/
/*!
    @brief  Reads the bulk of data from captouch chip. Fill in {@link touches},
   {@link touchX}, {@link touchY} and {@link touchID} with results
*/
/**************************************************************************/
void Adafruit_FT6206::readData(void) {

  uint8_t i2cdat[16];
  Wire.beginTransmission(FT62XX_ADDR);
  Wire.write((byte)0);
  Wire.endTransmission();

  Wire.requestFrom((byte)FT62XX_ADDR, (byte)16);
  for (uint8_t i = 0; i < 16; i++)
    i2cdat[i] = Wire.read();

#ifdef FT6206_DEBUG
  for (int16_t i = 0; i < 16; i++) {
    Serial.print("I2C $");
    Serial.print(i, HEX);
    Serial.print(" = 0x");
    Serial.println(i2cdat[i], HEX);
  }
#endif

  touches = i2cdat[0x02];
  if ((touches > 2) || (touches == 0)) {
    touches = 0;
  }

#ifdef FT6206_DEBUG
  Serial.print("# Touches: ");
  Serial.println(touches);

  for (uint8_t i = 0; i < 16; i++) {
    Serial.print("0x");
    Serial.print(i2cdat[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  if (i2cdat[0x01] != 0x00) {
    Serial.print("Gesture #");
    Serial.println(i2cdat[0x01]);
  }
#endif

  for (uint8_t i = 0; i < 2; i++) {
    touchX[i] = i2cdat[0x03 + i * 6] & 0x0F;
    touchX[i] <<= 8;
    touchX[i] |= i2cdat[0x04 + i * 6];
    touchY[i] = i2cdat[0x05 + i * 6] & 0x0F;
    touchY[i] <<= 8;
    touchY[i] |= i2cdat[0x06 + i * 6];
    touchID[i] = i2cdat[0x05 + i * 6] >> 4;
  }

#ifdef FT6206_DEBUG
  Serial.println();
  for (uint8_t i = 0; i < touches; i++) {
    Serial.print("ID #");
    Serial.print(touchID[i]);
    Serial.print("\t(");
    Serial.print(touchX[i]);
    Serial.print(", ");
    Serial.print(touchY[i]);
    Serial.print(") ");
  }
  Serial.println();
#endif
}

uint8_t Adafruit_FT6206::readRegister8(uint8_t reg) {
  uint8_t x;
  // use i2c
  Wire.beginTransmission(FT62XX_ADDR);
  Wire.write((byte)reg);
  Wire.endTransmission();

  Wire.requestFrom((byte)FT62XX_ADDR, (byte)1);
  x = Wire.read();

#ifdef I2C_DEBUG
  Serial.print("$");
  Serial.print(reg, HEX);
  Serial.print(": 0x");
  Serial.println(x, HEX);
#endif

  return x;
}

void Adafruit_FT6206::writeRegister8(uint8_t reg, uint8_t val) {
  // use i2c
  Wire.beginTransmission(FT62XX_ADDR);
  Wire.write((byte)reg);
  Wire.write((byte)val);
  Wire.endTransmission();
}

/*

// DONT DO THIS - REALLY - IT DOESNT WORK
void Adafruit_FT6206::autoCalibrate(void) {
 writeRegister8(FT06_REG_MODE, FT6206_REG_FACTORYMODE);
 delay(100);
 //Serial.println("Calibrating...");
 writeRegister8(FT6206_REG_CALIBRATE, 4);
 delay(300);
 for (uint8_t i = 0; i < 100; i++) {
   uint8_t temp;
   temp = readRegister8(FT6206_REG_MODE);
   Serial.println(temp, HEX);
   //return to normal mode, calibration finish
   if (0x0 == ((temp & 0x70) >> 4))
     break;
 }
 delay(200);
 //Serial.println("Calibrated");
 delay(300);
 writeRegister8(FT6206_REG_MODE, FT6206_REG_FACTORYMODE);
 delay(100);
 writeRegister8(FT6206_REG_CALIBRATE, 5);
 delay(300);
 writeRegister8(FT6206_REG_MODE, FT6206_REG_WORKMODE);
 delay(300);
}
*/

/****************/

/**************************************************************************/
/*!
    @brief  Instantiates a new FT6206 class with x, y and z set to 0 by default
*/
/**************************************************************************/
FT_Point::FT_Point(void) { x = y = z = 0; }

/**************************************************************************/
/*!
    @brief  Instantiates a new FT6206 class with x, y and z set by params.
    @param  _x The X coordinate
    @param  _y The Y coordinate
    @param  _z The Z coordinate
*/
/**************************************************************************/

FT_Point::FT_Point(int16_t _x, int16_t _y, int16_t _z) {
  x = _x;
  y = _y;
  z = _z;
}

/**************************************************************************/
/*!
    @brief  Simple == comparator for two FT_Point objects
    @returns True if x, y and z are the same for both points, False otherwise.
*/
/**************************************************************************/
bool FT_Point::operator==(FT_Point p1) {
  return ((p1.x == x) && (p1.y == y) && (p1.z == z));
}

/**************************************************************************/
/*!
    @brief  Simple != comparator for two FT_Point objects
    @returns False if x, y and z are the same for both points, True otherwise.
*/
/**************************************************************************/
bool FT_Point::operator!=(FT_Point p1) {
  return ((p1.x != x) || (p1.y != y) || (p1.z != z));
}
