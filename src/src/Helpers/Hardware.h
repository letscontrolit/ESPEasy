#ifndef HELPERS_HARDWARE_H
#define HELPERS_HARDWARE_H

#include <Arduino.h>

#include "../DataStructs/GpioFactorySettingsStruct.h"
#include "../DataStructs/PinMode.h"
#include "../DataTypes/DeviceModel.h"
#include "../DataTypes/PluginID.h"
#include "../DataTypes/TaskIndex.h"

#include "../Globals/ResetFactoryDefaultPref.h"

#include "../../ESPEasy_common.h"

#ifdef FEATURE_SD
#include <SD.h>
#endif

/********************************************************************************************\
 * Initialize specific hardware settings (only global ones, others are set through devices)
 \*********************************************************************************************/
void hardwareInit();

void initI2C();

void I2CSelectHighClockSpeed();
void I2CSelectLowClockSpeed();
void I2CSelect_Max100kHz_ClockSpeed();
void I2CSelectClockSpeed(uint32_t clockFreq);
void I2CForceResetBus_swap_pins(uint8_t address);
void I2CBegin(int8_t sda, int8_t scl, uint32_t clockFreq);

#ifdef FEATURE_I2CMULTIPLEXER
bool isI2CMultiplexerEnabled();

void I2CMultiplexerSelectByTaskIndex(taskIndex_t taskIndex);
void I2CMultiplexerSelect(uint8_t i);

void I2CMultiplexerOff();

void SetI2CMultiplexer(uint8_t toWrite);

uint8_t I2CMultiplexerMaxChannels();

void I2CMultiplexerReset();

bool I2CMultiplexerPortSelectedForTask(taskIndex_t taskIndex);
#endif

void checkResetFactoryPin();

#ifdef ESP8266
extern int lastADCvalue; // Keep track of last ADC value as it cannot be read while WiFi is connecting
#endif
int espeasy_analogRead(int pin);

#ifdef ESP32
int espeasy_analogRead(int pin, bool readAsTouch);
#endif


/********************************************************************************************\
   Hardware information
 \*********************************************************************************************/
uint32_t getFlashChipId();

uint32_t getFlashRealSizeInBytes();

uint32_t getFlashChipSpeed();

bool    puyaSupport();

uint8_t getFlashChipVendorId();

bool    flashChipVendorPuya();

// Last 24 bit of MAC address as integer, to be used in rules.
uint32_t getChipId();

uint8_t getChipCores();

const __FlashStringHelper * getChipModel();

uint8_t getChipRevision();

uint32_t getSketchSize();

uint32_t getFreeSketchSpace();


/********************************************************************************************\
   PSRAM support
 \*********************************************************************************************/
#ifdef ESP32

// this function is a replacement for `psramFound()`.
// `psramFound()` can return true even if no PSRAM is actually installed
// This new version also checks `esp_spiram_is_initialized` to know if the PSRAM is initialized
// Original Tasmota: 
// https://github.com/arendst/Tasmota/blob/1e6b78a957be538cf494f0e2dc49060d1cb0fe8b/tasmota/support_esp.ino#L470
bool FoundPSRAM();

// new function to check whether PSRAM is present and supported (i.e. required pacthes are present)
bool UsePSRAM();

/*
 * ESP32 v1 and v2 needs some special patches to use PSRAM.
 * Original function used from Tasmota: 
 * https://github.com/arendst/Tasmota/blob/1e6b78a957be538cf494f0e2dc49060d1cb0fe8b/tasmota/support_esp.ino#L762
 * 
 * If using ESP32 v1, please add: `-mfix-esp32-psram-cache-issue -lc-psram-workaround -lm-psram-workaround`
 *
 * This function returns true if the chip supports PSRAM natively (v3) or if the
 * patches are present.
 */
bool CanUsePSRAM();

#endif  // ESP32

/*********************************************************************************************\
 * High entropy hardware random generator
 * Thanks to DigitalAlchemist
\*********************************************************************************************/
// Based on code from https://raw.githubusercontent.com/espressif/esp-idf/master/components/esp32/hw_random.c
uint32_t HwRandom();


/********************************************************************************************\
   Boot information
 \*********************************************************************************************/
void readBootCause();


/********************************************************************************************\
   Hardware specific configurations
 \*********************************************************************************************/
const __FlashStringHelper * getDeviceModelBrandString(DeviceModel model);

String getDeviceModelString(DeviceModel model);

bool modelMatchingFlashSize(DeviceModel model);

void setFactoryDefault(DeviceModel model);

/********************************************************************************************\
   Add pre defined plugins and rules.
 \*********************************************************************************************/
void addSwitchPlugin(taskIndex_t taskIndex, int gpio, const String& name, bool activeLow);

void addPredefinedPlugins(const GpioFactorySettingsStruct& gpio_settings);

void addButtonRelayRule(uint8_t buttonNumber, int relay_gpio);

void addPredefinedRules(const GpioFactorySettingsStruct& gpio_settings);

// ********************************************************************************
// Get info of a specific GPIO pin.
// ********************************************************************************
// return true when pin can be used.
bool getGpioInfo(int gpio, int& pinnr, bool& input, bool& output, bool& warning);

bool getGpioPullResistor(int gpio, bool& hasPullUp, bool& hasPullDown);

bool validGpio(int gpio);


#ifdef ESP32

// Get ADC related info for a given GPIO pin
// @param gpio_pin   GPIO pin number
// @param adc        Number of ADC unit (0 == Hall effect)
// @param ch         Channel number on ADC unit
// @param t          index of touch pad ID
bool getADC_gpio_info(int gpio_pin, int& adc, int& ch, int& t);
int touchPinToGpio(int touch_pin);

#endif

// ********************************************************************************
// Manage PWM state of GPIO pins.
// ********************************************************************************
void initAnalogWrite();
#if defined(ESP32)
extern int8_t ledChannelPin[16];
extern uint32_t ledChannelFreq[16];

int8_t attachLedChannel(int pin, uint32_t frequency = 0);
void detachLedChannel(int pin);
uint32_t analogWriteESP32(int pin,
                          int value,
                          uint32_t frequency = 0);
#endif // if defined(ESP32)

// Duty cycle 0..100%
bool set_Gpio_PWM_pct(int gpio, float dutyCycle_f, uint32_t frequency = 0);

bool set_Gpio_PWM(int gpio, uint32_t dutyCycle, uint32_t frequency = 0);
bool set_Gpio_PWM(int gpio, uint32_t dutyCycle, uint32_t fadeDuration_ms, uint32_t& frequency, uint32_t& key);


// ********************************************************************************
// change of device: cleanup old device and reset default settings
// ********************************************************************************
void setTaskDevice_to_TaskIndex(pluginID_t taskdevicenumber, taskIndex_t taskIndex);

// ********************************************************************************
// Initialize task with some default values applicable for almost all tasks
// ********************************************************************************
void setBasicTaskValues(taskIndex_t taskIndex, unsigned long taskdevicetimer,
                        bool enabled, const String& name, int pin1, int pin2, int pin3);

#endif // HELPERS_HARDWARE_H

