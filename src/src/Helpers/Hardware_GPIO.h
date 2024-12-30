#ifndef HELPERS_HARDWARE_GPIO_H
#define HELPERS_HARDWARE_GPIO_H


#include "../../ESPEasy_common.h"


// ********************************************************************************
// Get info of a specific GPIO pin.
// ********************************************************************************
// return true when pin can be used.
bool getGpioInfo(int   gpio,
                 int & pinnr,
                 bool& input,
                 bool& output,
                 bool& warning);

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
int  touchPinToGpio(int touch_pin);
bool getDAC_gpio_info(int  gpio_pin,
                      int& dac);

#endif // ifdef ESP32


#endif // ifndef HELPERS_HARDWARE_GPIO_H
