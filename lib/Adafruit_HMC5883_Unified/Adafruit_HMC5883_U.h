/*!
 * @file Adafruit_HMC5883_U.h
 */
#ifndef __HMC5883_H__
#define __HMC5883_H__

#if (ARDUINO >= 100)
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Adafruit_Sensor.h>

#ifdef __AVR_ATtiny85__
#include "TinyWireM.h"
#define Wire TinyWireM
#else
#include <Wire.h>
#endif

/*!
 * @brief I2C address/bits
 */
#define HMC5883_ADDRESS_MAG (0x3C >> 1) // 0011110x

/*!
 @brief Registers
 */
typedef enum {
  HMC5883_REGISTER_MAG_CRA_REG_M = 0x00,
  HMC5883_REGISTER_MAG_CRB_REG_M = 0x01,
  HMC5883_REGISTER_MAG_MR_REG_M = 0x02,
  HMC5883_REGISTER_MAG_OUT_X_H_M = 0x03,
  HMC5883_REGISTER_MAG_OUT_X_L_M = 0x04,
  HMC5883_REGISTER_MAG_OUT_Z_H_M = 0x05,
  HMC5883_REGISTER_MAG_OUT_Z_L_M = 0x06,
  HMC5883_REGISTER_MAG_OUT_Y_H_M = 0x07,
  HMC5883_REGISTER_MAG_OUT_Y_L_M = 0x08,
  HMC5883_REGISTER_MAG_SR_REG_Mg = 0x09,
  HMC5883_REGISTER_MAG_IRA_REG_M = 0x0A,
  HMC5883_REGISTER_MAG_IRB_REG_M = 0x0B,
  HMC5883_REGISTER_MAG_IRC_REG_M = 0x0C,
  HMC5883_REGISTER_MAG_TEMP_OUT_H_M = 0x31,
  HMC5883_REGISTER_MAG_TEMP_OUT_L_M = 0x32
} hmc5883MagRegisters_t;

/*!
 * @brief Magnetometer gain settings
 */
typedef enum {
  HMC5883_MAGGAIN_1_3 = 0x20, // +/- 1.3
  HMC5883_MAGGAIN_1_9 = 0x40, // +/- 1.9
  HMC5883_MAGGAIN_2_5 = 0x60, // +/- 2.5
  HMC5883_MAGGAIN_4_0 = 0x80, // +/- 4.0
  HMC5883_MAGGAIN_4_7 = 0xA0, // +/- 4.7
  HMC5883_MAGGAIN_5_6 = 0xC0, // +/- 5.6
  HMC5883_MAGGAIN_8_1 = 0xE0  // +/- 8.1
} hmc5883MagGain;

/*!
 * @brief Internal magnetometer data type
 */
typedef struct hmc5883MagData_s {
  float x;           //!< Magnetometer x value
  float y;           //!< Magnetometer y value
  float z;           //!< Magnetometer z value
  float orientation; //!< Magnetometer orientation
} hmc5883MagData;

/*!
 * @brief Chip ID
 */
#define HMC5883_ID (0b11010100)

//! Unified sensor driver for the magnetometer ///
class Adafruit_HMC5883_Unified : public Adafruit_Sensor {
public:
  /*!
   * @param sensorID sensor ID, -1 by default
   */
  Adafruit_HMC5883_Unified(int32_t sensorID = -1);

  bool begin(void); //!< @return Returns whether connection was successful
  void setMagGain(hmc5883MagGain gain); //!< @param gain Desired magnetic gain
  bool
  getEvent(sensors_event_t *); //!< @return Returns the most recent sensor event
  void getSensor(sensor_t *);

private:
  hmc5883MagGain _magGain;
  hmc5883MagData _magData; // Last read magnetometer data will be available here
  int32_t _sensorID;

  void write8(byte address, byte reg, byte value);
  byte read8(byte address, byte reg);
  void read(void);
};

#endif
