#ifndef HELPERS_HARDWARE_H
#define HELPERS_HARDWARE_H

#include "../../ESPEasy_common.h"

#include "../DataStructs/GpioFactorySettingsStruct.h"
#include "../DataStructs/PinMode.h"
#include "../DataTypes/DeviceModel.h"
#include "../DataTypes/PluginID.h"
#include "../DataTypes/TaskIndex.h"

#include "../Globals/ResetFactoryDefaultPref.h"

#include "../Helpers/Hardware_defines.h"

#if ESP_IDF_VERSION_MAJOR >= 5
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>

#endif


/********************************************************************************************\
 * Initialize specific hardware settings (only global ones, others are set through devices)
 \*********************************************************************************************/
void hardwareInit();


void    checkResetFactoryPin();

#ifdef ESP8266
extern int lastADCvalue; // Keep track of last ADC value as it cannot be read while WiFi is connecting

int espeasy_analogRead(int pin);
#endif // ifdef ESP8266

float mapADCtoFloat(float float_value,
                             float adc1,
                             float adc2,
                             float out1,
                             float out2);


#ifdef ESP32
void                       initADC();
float                      applyADCFactoryCalibration(
    float raw_value, 
    adc_atten_t attenuation);

bool                       hasADC_factory_calibration();
const __FlashStringHelper* getADC_factory_calibration_type();

float getADC_factory_calibrated_min(adc_atten_t attenuation);
float getADC_factory_calibrated_max(adc_atten_t attenuation);

int                        getADC_num_for_gpio(int pin);
int                        getADC_num_for_gpio(int pin, int& channel);

int                        espeasy_analogRead(int  pin,
                                              bool readAsTouch = false);

int  getCPU_MaxFreqMHz();
int  getCPU_MinFreqMHz();

#endif // ifdef ESP32



/*********************************************************************************************\
* High entropy hardware random generator
* Thanks to DigitalAlchemist
\*********************************************************************************************/

// Based on code from https://raw.githubusercontent.com/espressif/esp-idf/master/components/esp32/hw_random.c
uint32_t HwRandom();

long HwRandom(long howbig);

long HwRandom(long howsmall, long howbig);


/********************************************************************************************\
   Boot information
 \*********************************************************************************************/
void readBootCause();


/********************************************************************************************\
   Hardware specific configurations
 \*********************************************************************************************/
const __FlashStringHelper* getDeviceModelBrandString(DeviceModel model);

String                     getDeviceModelString(DeviceModel model);

bool                       modelMatchingFlashSize(DeviceModel model);

void                       setFactoryDefault(DeviceModel model);

/********************************************************************************************\
   Add pre defined plugins and rules.
 \*********************************************************************************************/
void                       addSwitchPlugin(taskIndex_t   taskIndex,
                                           int           gpio,
                                           const String& name,
                                           bool          activeLow);

void addPredefinedPlugins(const GpioFactorySettingsStruct& gpio_settings);

void addButtonRelayRule(uint8_t buttonNumber,
                        int     relay_gpio);

void addPredefinedRules(const GpioFactorySettingsStruct& gpio_settings);


// ********************************************************************************
// change of device: cleanup old device and reset default settings
// ********************************************************************************
void setTaskDevice_to_TaskIndex(pluginID_t  taskdevicenumber,
                                taskIndex_t taskIndex);

// ********************************************************************************
// Initialize task with some default values applicable for almost all tasks
// ********************************************************************************
void setBasicTaskValues(taskIndex_t   taskIndex,
                        unsigned long taskdevicetimer,
                        bool          enabled,
                        const String& name,
                        const int     pins[3]);

#endif // HELPERS_HARDWARE_H
