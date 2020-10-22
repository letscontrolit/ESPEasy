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

/********************************************************************************************\
 * Initialize specific hardware settings (only global ones, others are set through devices)
 \*********************************************************************************************/
void hardwareInit();

void initI2C();

void I2CSelectClockSpeed(bool setLowSpeed);

#ifdef FEATURE_I2CMULTIPLEXER
bool isI2CMultiplexerEnabled();

void I2CMultiplexerSelectByTaskIndex(taskIndex_t taskIndex);
void I2CMultiplexerSelect(uint8_t i);

void I2CMultiplexerOff();

void SetI2CMultiplexer(byte toWrite);

byte I2CMultiplexerMaxChannels();

void I2CMultiplexerReset();

bool I2CMultiplexerPortSelectedForTask(taskIndex_t taskIndex);
#endif

void checkResetFactoryPin();

int espeasy_analogRead(int pin);

#ifdef ESP32
int espeasy_analogRead(int pin, bool readAsTouch);
#endif


/********************************************************************************************\
   Hardware information
 \*********************************************************************************************/
uint32_t getFlashChipId();

uint32_t getFlashRealSizeInBytes();

bool    puyaSupport();

uint8_t getFlashChipVendorId();

bool    flashChipVendorPuya();

// Last 24 bit of MAC address as integer, to be used in rules.
uint32_t getChipId();

uint8_t getChipCores();

String getChipModel();

uint8_t getChipRevision();

/********************************************************************************************\
   Hardware specific configurations
 \*********************************************************************************************/
String getDeviceModelBrandString(DeviceModel model);

String getDeviceModelString(DeviceModel model);

bool modelMatchingFlashSize(DeviceModel model);

void setFactoryDefault(DeviceModel model);

/********************************************************************************************\
   Add pre defined plugins and rules.
 \*********************************************************************************************/
void addSwitchPlugin(taskIndex_t taskIndex, byte gpio, const String& name, bool activeLow);

void addPredefinedPlugins(const GpioFactorySettingsStruct& gpio_settings);

void addButtonRelayRule(byte buttonNumber, byte relay_gpio);

void addPredefinedRules(const GpioFactorySettingsStruct& gpio_settings);

// ********************************************************************************
// Get info of a specific GPIO pin.
// ********************************************************************************
// return true when pin can be used.
bool getGpioInfo(int gpio, int& pinnr, bool& input, bool& output, bool& warning);

bool getGpioPullResistor(int gpio, bool& hasPullUp, bool& hasPullDown);


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
// change of device: cleanup old device and reset default settings
// ********************************************************************************
void setTaskDevice_to_TaskIndex(pluginID_t taskdevicenumber, taskIndex_t taskIndex);

// ********************************************************************************
// Initialize task with some default values applicable for almost all tasks
// ********************************************************************************
void setBasicTaskValues(taskIndex_t taskIndex, unsigned long taskdevicetimer,
                        bool enabled, const String& name, int pin1, int pin2, int pin3);

#endif // HELPERS_HARDWARE_H

