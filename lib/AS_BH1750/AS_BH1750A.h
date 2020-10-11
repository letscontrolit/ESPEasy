/*
 This is a (Arduino) library for the BH1750FVI Digital Light Sensor.
 
 Description:
 http://www.rohm.com/web/global/products/-/product/BH1750FVI
 
 Datasheet:
 http://rohmfs.rohm.com/en/products/databook/datasheet/ic/sensor/light/bh1750fvi-e.pdf
 
 Copyright (c) 2013 Alexander Schulz.  All right reserved.
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA	 02110-1301	 USA
 */

#ifndef AS_BH1750A_h
#define AS_BH1750A_h

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif
#include "Wire.h"

// Mögliche I2C Adressen
#define BH1750_DEFAULT_I2CADDR 0x23
#define BH1750_SECOND_I2CADDR 0x5C

// MTreg Werte
// Default
#define BH1750_MTREG_DEFAULT 69
// Sensitivity : default = 0.45
#define BH1750_MTREG_MIN 31
// Sensitivity : default = 3.68
#define BH1750_MTREG_MAX 254

// Hardware Modi
// No active state
#define BH1750_POWER_DOWN 0x00

// Wating for measurment command
#define BH1750_POWER_ON 0x01

// Reset data register value - not accepted in POWER_DOWN mode
#define BH1750_RESET 0x07

// Start measurement at 1lx resolution. Measurement time is approx 120ms.
#define BH1750_CONTINUOUS_HIGH_RES_MODE  0x10

// Start measurement at 0.5lx resolution. Measurement time is approx 120ms.
#define BH1750_CONTINUOUS_HIGH_RES_MODE_2  0x11

// Start measurement at 4lx resolution. Measurement time is approx 16ms.
#define BH1750_CONTINUOUS_LOW_RES_MODE  0x13

// Start measurement at 1lx resolution. Measurement time is approx 120ms.
// Device is automatically set to Power Down after measurement.
#define BH1750_ONE_TIME_HIGH_RES_MODE  0x20

// Start measurement at 0.5lx resolution. Measurement time is approx 120ms.
// Device is automatically set to Power Down after measurement.
#define BH1750_ONE_TIME_HIGH_RES_MODE_2  0x21

// Start measurement at 1lx resolution. Measurement time is approx 120ms.
// Device is automatically set to Power Down after measurement.
#define BH1750_ONE_TIME_LOW_RES_MODE  0x23

#define MAX_U_LONG 4294967295

/** Virtual Modi */
typedef enum
{
  RESOLUTION_LOW         = (1), /** 4lx resolution. Measurement time is approx 16ms. */  
  RESOLUTION_NORMAL      = (2), /** 1lx resolution. Measurement time is approx 120ms. */
  RESOLUTION_HIGH        = (3), /** 0,5lx resolution. Measurement time is approx 120ms. */
  RESOLUTION_AUTO_HIGH   = (99) /** 0,11-1lx resolution. Measurement time is above 250ms. */
  }  
  sensors_resolution_t;

#ifdef ESP32
typedef void (*DelayFuncPtr)(uint32_t);
#endif
#ifdef ESP8266
typedef void (*DelayFuncPtr)(unsigned long);
#endif
typedef unsigned long (*TimeFuncPtr)(void);

/**
 * BH1750 driver class.
 */
class AS_BH1750A {
public:
  /**
   * Constructor.
   * Erlaubt die I2C-Adresse des Sensors zu ändern. 
   * Standardadresse: 0x23, Alternativadresse: 0x5C. 
   * Es sind entsprechende Konstanten definiert: BH1750_DEFAULT_I2CADDR  und BH1750_SECOND_I2CADDR.
   * Bei Nichtangabe wird die Standardadresse verwendet. 
   * Um die Alternativadresse zu nutzen, muss der Sensorpin 'ADR' des Chips auf VCC gelegt werden.
   */
  AS_BH1750A(uint8_t address = BH1750_DEFAULT_I2CADDR);

  /**
   * Führt die anfängliche Initialisierung des Sensors.
   * Mögliche Parameter: 
   *  - Modus für die Sensorauflösung:
   *    -- RESOLUTION_LOW:         Physische Sensormodus mit 4 lx Auflösung. Messzeit ca. 16ms. Bereich 0-54612.
   *    -- RESOLUTION_NORMAL:      Physische Sensormodus mit 1 lx Auflösung. Messzeit ca. 120ms. Bereich 0-54612.
   *    -- RESOLUTION_HIGH:        Physische Sensormodus mit 0,5 lx Auflösung. Messzeit ca. 120ms. Bereich 0-54612.
   *                               (Die Messbereiche können durch Änderung des MTreg verschoben werden.)
   *    -- RESOLUTION_AUTO_HIGH:   Die Werte im MTreg werden je nach Helligkeit automatisch so angepasst, 
   *                               dass eine maximalmögliche Auflösung und Messbereich erziehlt werden.
   *                               Die messbaren Werte fangen von 0,11 lx und gehen bis über 100000 lx.
   *                               (ich weis nicht, wie genau die Werte in Grenzbereichen sind, 
   *                               besonders bei hohen Werte habe ich meine Zweifel.)
   *                               Auflösung im Unteren Bereich 0,13 lx, im mittleren 0,5 lx, im oberen 1-2 lx.
   *                               Die Messzeiten verlängern sich durch mehrfache Messungen und 
   *                               die Änderungen von Measurement Time (MTreg) bis max. ca. 500 ms.
   *   
   * - AutoPowerDown: true = Der Sensor wird nach der Messung in den Stromsparmodus versetzt. 
   *   Das spätere Aufwecken wird ggf. automatisch vorgenommen, braucht jedoch geringfügig mehr Zeit.
   *
   * Defaultwerte: RESOLUTION_AUTO_HIGH, true, delay()
   *
   */
  bool begin(sensors_resolution_t mode = RESOLUTION_AUTO_HIGH, bool autoPowerDown = true);

  /**
   * Erlaub eine Prüfung, ob ein (ansprechbarer) BH1750-Sensor vorhanden ist.
   */
  bool isPresent(void);

  /**
   * Liefert aktuell gemessenen Wert für Helligkeit in lux (lx).
   * Falls sich der Sensorf in Stromsparmodus befindet, wird er automatisch geweckt.
   *
   * Wurde der Sensor (noch) nicht initialisiert (begin), wird der Wert -1 geliefert.
   *
   * Mögliche Parameter: 
   *
   * - DelayFuncPtr: delay(n) Möglichkeit, eigene Delay-Funktion mitzugeben (z.B. um sleep-Modus zu verwenden).
   * 
   * Defaultwerte: delay()
   *
   */
  float readLightLevel(DelayFuncPtr fDelayPtr = &delay, TimeFuncPtr fTimePtr = &millis);

  bool startMeasurementAsync(TimeFuncPtr fTimePtr = &millis);
  bool isMeasurementReady(void);
  float readLightLevelAsync();
  
  //float checkAndReadLightLevelAsync(TimeFuncPtr fTimePtr);

  //void reset(void);
  unsigned long nextDelay(void);

  /**
   * Schickt den Sensor in Stromsparmodus.
   * Funktionier nur, wenn der Sensor bereits initialisiert wurde.
   */
  void powerDown(void);

//bool delayExpired(); // TEST

private:
  int _address;
  uint8_t _hardwareMode;

  uint8_t _MTreg;
  float _MTregFactor;

  sensors_resolution_t _virtualMode;
  bool _autoPowerDown;

  bool _valueReaded;

  bool selectResolutionMode(uint8_t mode);
  void defineMTReg(uint8_t val);
  void powerOn();
  //void reset(void);
  uint16_t readRawLevel(void);
  float convertRawValue(uint16_t raw);
  bool isInitialized();
  bool write8(uint8_t data);
  
  TimeFuncPtr _fTimePtr;
  int _stage = 0;
  unsigned long _nextDelay = 0;
  unsigned long _lastTimestamp = 0;
  float _lastResult = -100;
  bool delayExpired();
  void selectAutoMode();
  
  // TEST
  //float readLightLevel_alt(DelayFuncPtr fDelayPtr = &delay);
  unsigned long getModeDelay();
  
};

#endif
