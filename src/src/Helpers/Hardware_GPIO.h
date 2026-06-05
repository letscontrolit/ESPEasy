#ifndef HELPERS_HARDWARE_GPIO_H
#define HELPERS_HARDWARE_GPIO_H


#include "../../ESPEasy_common.h"

#ifdef ESP32
# include "soc/soc_caps.h"
# include <esp32-hal-periman.h>
#endif

// ********************************************************************************
// Get info of a specific GPIO pin.
// ********************************************************************************
// return true when pin can be used.
bool getGpioInfo(int   gpio,
                 int & pinnr,
                 bool& input,
                 bool& output,
                 bool& warning);


// For ESP32, see: https://github.com/espressif/arduino-esp32/blob/9d84c78cf2d44911639530e54a9dbb9ee9164f6c/cores/esp32/esp32-hal.h#L64-L75
// BOOT_PIN is defined for all ESP32 chips.
bool isBootModePin(int gpio);

bool isBootStrapPin(int gpio);

bool getGpioPullResistor(int   gpio,
                         bool& hasPullUp,
                         bool& hasPullDown);

bool validGpio(int gpio);

bool isSerialConsolePin(int gpio);


#ifdef ESP32

// Get ADC related info for a given GPIO pin
// @param gpio_pin   GPIO pin number
// @param adc        Number of ADC unit (0 == Hall effect)
// @param ch         Channel number on ADC unit
// @param t          index of touch pad ID
bool getADC_gpio_info(int  gpio_pin,
                      int& adc,
                      int& ch,
                      int& t);
#if SOC_TOUCH_SENSOR_SUPPORTED
int  touchPinToGpio(int touch_pin);
#endif

#if SOC_DAC_SUPPORTED
bool getDAC_gpio_info(int  gpio_pin,
                      int& dac);
#endif

bool getPeriman_gpio_info(int  gpio_pin,
    peripheral_bus_type_t& bus_type,
    String& gpio_str,
    String& typeName,int& bus_number, int& bus_channel);

#endif // ifdef ESP32


#endif // ifndef HELPERS_HARDWARE_GPIO_H
